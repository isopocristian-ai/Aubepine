// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2018 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <array>
#include <map>
#include <vector>
#include <cstring>

#include <fmt/ranges.h>
#include <opus.h>

#include "core/hle/service/audio/hardware_opus_decoder.h"
#include "core/hle/service/audio/hardware_opus_decoder_manager.h"
#include "core/hle/service/cmif_serialization.h"
#include "core/core.h"
#include "core/hle/kernel/k_event.h"
#include "core/memory.h"
#include "core/hle/kernel/k_readable_event.h"
#include "core/hle/service/ipc_helpers.h"
#include "core/hle/service/kernel_helpers.h"

namespace Service::Audio {

using namespace AudioCore::OpusDecoder;

namespace {

// What the client's shared I/O region holds that the service writes: the result ring, whose header
// has the service's write index at +4 and the client's read index at +0x44, followed at +0x80 by
// 64-byte entries {u16 producer, u16 instance, u16 I/O buffer, u16 sequence, u64 processing time,
// u32 output size, u32 result} (read in HardwareAudioCodecManager::UpdateProcessResult).
struct HwacodShared {
    VAddr io_address{};
    u64 io_size{};
    u64 ring_offset{};
    u32 ring_count{};
    u32 input_capacity{};
    u32 output_capacity{};
    Kernel::KEvent* event{};
};

// The object hwacod Initialize returns. 0 InitializeInstance(u32 index, the instance parameter in a
// pointer buffer: u64 kind (1 Opus, 2 multistream Opus), u32 sample rate, u32 channels, u64 frame
// time in ns); 10 Execute: the tasks in a pointer buffer, 16 bytes each {u64 header, u32 input
// size, u8 flag}, the header being producer | instance << 16 | I/O buffer << 32 | sequence << 48
// (TaskProducer). I/O buffer i holds its input at i * (input + output capacity) in the shared
// region and its output right after the input capacity (IoBufferManager::Initialize). An input is
// a big-endian u32 size, a u32 final range, then the Opus packet; the output is s16 PCM.
// Multistream instances are not decoded yet (their tasks complete with no output).
class IHardwareAudioCodecManagerSession final
    : public ServiceFramework<IHardwareAudioCodecManagerSession> {
public:
    explicit IHardwareAudioCodecManagerSession(Core::System& system_, HwacodShared shared_)
        : ServiceFramework{system_, "hwopus:hwacod:manager"}, shared{shared_} {
        // clang-format off
        static const FunctionInfo functions[] = {
            {0, &IHardwareAudioCodecManagerSession::InitializeInstance, "InitializeInstance"},
            {10, &IHardwareAudioCodecManagerSession::Execute, "Execute"},
        };
        // clang-format on
        RegisterHandlers(functions);
    }

    ~IHardwareAudioCodecManagerSession() override {
        for (auto& [index, instance] : instances) {
            if (instance.decoder != nullptr) {
                opus_decoder_destroy(instance.decoder);
            }
        }
    }

private:
    struct Instance {
        ::OpusDecoder* decoder{};
        u32 channels{};
    };

    void InitializeInstance(HLERequestContext& ctx) {
        IPC::RequestParser rp{ctx};
        const u32 index = rp.Pop<u32>();
        const auto parameter = ctx.ReadBuffer();
        std::array<u32, 6> words{};
        std::memcpy(words.data(), parameter.data(), std::min(parameter.size(), sizeof(words)));
        const u32 kind = words[0];
        const u32 sample_rate = words[2];
        const u32 channels = words[3];
        auto& instance = instances[index];
        if (instance.decoder != nullptr) {
            opus_decoder_destroy(instance.decoder);
            instance.decoder = nullptr;
        }
        instance.channels = channels;
        int error = OPUS_OK;
        if (kind == 1 && (channels == 1 || channels == 2)) {
            instance.decoder = opus_decoder_create(static_cast<opus_int32>(sample_rate),
                                                   static_cast<int>(channels), &error);
        }
        if (static u32 logged{}; logged++ < 4 || instance.decoder == nullptr) {
            LOG_INFO(Service_Audio, "hwacod instance {}: kind {} {} Hz {} channels{}", index, kind,
                     sample_rate, channels,
                     instance.decoder != nullptr ? "" : fmt::format(" (not decoded, opus {})", error));
        }
        IPC::ResponseBuilder rb{ctx, 2};
        rb.Push(ResultSuccess);
    }

    // Decodes one task's input into its I/O buffer's output; returns the output size in bytes.
    u32 Decode(u32 instance_index, u32 io, u32 input_size) {
        const auto it = instances.find(instance_index);
        if (it == instances.end() || it->second.decoder == nullptr || input_size <= 8 ||
            input_size > shared.input_capacity) {
            return 0;
        }
        auto& memory = system.ApplicationMemory();
        const VAddr input = shared.io_address +
                            u64{io} * (shared.input_capacity + shared.output_capacity);
        const VAddr output = input + shared.input_capacity;
        std::vector<u8> data(input_size);
        memory.ReadBlock(input, data.data(), data.size());
        const u32 packet_size = (u32{data[0]} << 24) | (u32{data[1]} << 16) | (u32{data[2]} << 8) | data[3];
        if (packet_size == 0 || packet_size > input_size - 8) {
            return 0;
        }
        const u32 channels = it->second.channels;
        std::vector<opus_int16> pcm(shared.output_capacity / sizeof(opus_int16));
        const int frames = opus_decode(it->second.decoder, data.data() + 8,
                                       static_cast<opus_int32>(packet_size), pcm.data(),
                                       static_cast<int>(pcm.size() / channels), 0);
        if (frames < 0) {
            if (static u32 logged{}; logged++ < 8) {
                LOG_WARNING(Service_Audio, "hwacod instance {} decode failed: opus {}", instance_index,
                            frames);
            }
            return 0;
        }
        const u32 bytes = static_cast<u32>(frames) * channels * sizeof(opus_int16);
        memory.WriteBlock(output, pcm.data(), bytes);
        return bytes;
    }

    void Execute(HLERequestContext& ctx) {
        const auto tasks = ctx.ReadBuffer();
        auto& memory = system.ApplicationMemory();
        const VAddr ring = shared.io_address + shared.ring_offset;
        u32 write = memory.Read32(ring + 4);
        for (size_t offset = 0; offset + 16 <= tasks.size(); offset += 16) {
            u64 header{};
            u32 input_size{};
            std::memcpy(&header, tasks.data() + offset, sizeof(header));
            std::memcpy(&input_size, tasks.data() + offset + 8, sizeof(input_size));
            const u32 instance = static_cast<u32>(header >> 16) & 0xFFFF;
            const u32 io = static_cast<u32>(header >> 32) & 0xFFFF;
            const u32 output_size = Decode(instance, io, input_size);
            const VAddr entry = ring + 0x80 + u64{write} * 64;
            memory.Write64(entry, header);
            memory.Write64(entry + 8, 0);
            memory.Write32(entry + 0x10, output_size);
            memory.Write32(entry + 0x14, 0);
            write = write + 1 == shared.ring_count ? 0 : write + 1;
        }
        memory.Write32(ring + 4, write);
        if (shared.event != nullptr) {
            shared.event->Signal(system.Kernel());
        }
        IPC::ResponseBuilder rb{ctx, 2};
        rb.Push(ResultSuccess);
    }

    HwacodShared shared;
    std::map<u32, Instance> instances;
};

// nn::codec::hwacod (SDK 21, Star Fox): the hardware audio codec manager, opened with hwopus 100.
// Commands and layouts read from the SDK's shim: 1 is GetWorkBufferSize(u32), 10 Initialize (the
// work memory as transfer memory, the client process, the shared I/O region and its parameters;
// replies the manager's session object, a u32 and the event the client attaches to its system
// event).
class IHardwareAudioCodecSession final : public ServiceFramework<IHardwareAudioCodecSession> {
public:
    explicit IHardwareAudioCodecSession(Core::System& system_)
        : ServiceFramework{system_, "hwopus:hwacod"}, m_context{system_, "hwopus:hwacod"} {
        // clang-format off
        static const FunctionInfo functions[] = {
            {0, &IHardwareAudioCodecSession::StubReplyZeroed, "Unknown0 (instance parameter in a pointer buffer)"},
            {1, &IHardwareAudioCodecSession::GetWorkBufferSize, "GetWorkBufferSize"},
            {10, &IHardwareAudioCodecSession::Initialize, "Initialize"},
        };
        // clang-format on
        RegisterHandlers(functions);
    }

    ~IHardwareAudioCodecSession() override {
        if (m_event != nullptr) {
            m_context.CloseEvent(m_event);
        }
    }

private:
    void GetWorkBufferSize(HLERequestContext& ctx) {
        IPC::RequestParser rp{ctx};
        const u32 count = rp.Pop<u32>();
        LOG_WARNING(Service_Audio, "(STUBBED) hwacod GetWorkBufferSize count={}, returning 0x1000", count);
        IPC::ResponseBuilder rb{ctx, 3};
        rb.Push(ResultSuccess);
        rb.Push<u32>(0x1000);
    }

    void Initialize(HLERequestContext& ctx) {
        const auto cmd = ctx.CommandBuffer();
        std::string words;
        for (u32 i = 0; i < 48; ++i) {
            words += fmt::format(" {:08x}", cmd[i]);
        }
        LOG_WARNING(Service_Audio, "(STUBBED) hwacod Initialize:{}", words);
        if (m_event == nullptr) {
            m_event = m_context.CreateEvent("hwopus:hwacod:Event");
        }
        // The raw data after the CMIF header: the shared memory parameter (ring offset at word 11,
        // ring entries - 1 at word 17), then the work memory size, the I/O region and its size.
        u32 start = 0;
        while (start < 40 && cmd[start] != 0x49434653) {
            ++start;
        }
        const u32* const raw = &cmd[start + 4];
        HwacodShared shared{
            .io_address = VAddr{raw[26]} | (VAddr{raw[27]} << 32),
            .io_size = raw[28],
            .ring_offset = raw[11],
            .ring_count = raw[17] + 1,
            .input_capacity = raw[15],
            .output_capacity = raw[16],
            .event = m_event,
        };
        LOG_WARNING(Service_Audio, "hwacod I/O region {:#x} size {:#x}, result ring at +{:#x} ({} entries)",
                    shared.io_address, shared.io_size, shared.ring_offset, shared.ring_count);
        IPC::ResponseBuilder rb{ctx, 3, 1, 1};
        rb.Push(ResultSuccess);
        rb.Push<u32>(0);
        rb.PushCopyObjects(ctx, m_event->GetReadableEvent());
        rb.PushIpcInterface<IHardwareAudioCodecManagerSession>(ctx, system, shared);
    }

    KernelHelpers::ServiceContext m_context;
    Kernel::KEvent* m_event{};
};

} // Anonymous namespace

IHardwareOpusDecoderManager::IHardwareOpusDecoderManager(Core::System& system_)
    : ServiceFramework{system_, "hwopus"}, system{system_}, impl{system} {
    // clang-format off
    static const FunctionInfo functions[] = {
        {0, D<&IHardwareOpusDecoderManager::OpenHardwareOpusDecoder>, "OpenHardwareOpusDecoder"},
        {1, D<&IHardwareOpusDecoderManager::GetWorkBufferSize>, "GetWorkBufferSize"},
        {2, D<&IHardwareOpusDecoderManager::OpenHardwareOpusDecoderForMultiStream>, "OpenOpusDecoderForMultiStream"},
        {3, D<&IHardwareOpusDecoderManager::GetWorkBufferSizeForMultiStream>, "GetWorkBufferSizeForMultiStream"},
        {4, D<&IHardwareOpusDecoderManager::OpenHardwareOpusDecoderEx>, "OpenHardwareOpusDecoderEx"},
        {5, D<&IHardwareOpusDecoderManager::GetWorkBufferSizeEx>, "GetWorkBufferSizeEx"},
        {6, D<&IHardwareOpusDecoderManager::OpenHardwareOpusDecoderForMultiStreamEx>, "OpenHardwareOpusDecoderForMultiStreamEx"},
        {7, D<&IHardwareOpusDecoderManager::GetWorkBufferSizeForMultiStreamEx>, "GetWorkBufferSizeForMultiStreamEx"},
        {8, D<&IHardwareOpusDecoderManager::GetWorkBufferSizeExEx>, "GetWorkBufferSizeExEx"},
        {9, D<&IHardwareOpusDecoderManager::GetWorkBufferSizeForMultiStreamExEx>, "GetWorkBufferSizeForMultiStreamExEx"},
        // SDK 21: the session of nn::codec::hwacod, the hardware audio codec manager (Star Fox).
        {100, &IHardwareOpusDecoderManager::OpenHardwareAudioCodecSession, "OpenHardwareAudioCodecSession (guessed)"},
    };
    // clang-format on
    RegisterHandlers(functions);
}

IHardwareOpusDecoderManager::~IHardwareOpusDecoderManager() = default;

Result IHardwareOpusDecoderManager::OpenHardwareOpusDecoder(
    Out<SharedPointer<IHardwareOpusDecoder>> out_decoder, OpusParameters params, u32 tmem_size,
    InCopyHandle<Kernel::KTransferMemory> tmem_handle) {
    LOG_DEBUG(Service_Audio, "sample_rate {} channel_count {} transfer_memory_size {:#x}",
              params.sample_rate, params.channel_count, tmem_size);

    auto decoder{std::make_shared<IHardwareOpusDecoder>(system, impl.GetHardwareOpus())};
    OpusParametersEx ex{
        .sample_rate = params.sample_rate,
        .channel_count = params.channel_count,
        .use_large_frame_size = false,
    };
    R_TRY(decoder->Initialize(ex, tmem_handle.Get(), tmem_size));

    *out_decoder = decoder;
    R_SUCCEED();
}

Result IHardwareOpusDecoderManager::GetWorkBufferSize(Out<u32> out_size, OpusParameters params) {
    R_TRY(impl.GetWorkBufferSize(params, *out_size));
    LOG_DEBUG(Service_Audio, "sample_rate {} channel_count {} -- returned size {:#x}",
              params.sample_rate, params.channel_count, *out_size);
    R_SUCCEED();
}

Result IHardwareOpusDecoderManager::OpenHardwareOpusDecoderForMultiStream(
    Out<SharedPointer<IHardwareOpusDecoder>> out_decoder,
    InLargeData<OpusMultiStreamParameters, BufferAttr_HipcPointer> params, u32 tmem_size,
    InCopyHandle<Kernel::KTransferMemory> tmem_handle) {
    LOG_DEBUG(Service_Audio,
              "sample_rate {} channel_count {} total_stream_count {} stereo_stream_count {} "
              "transfer_memory_size {:#x}",
              params->sample_rate, params->channel_count, params->total_stream_count,
              params->stereo_stream_count, tmem_size);

    auto decoder{std::make_shared<IHardwareOpusDecoder>(system, impl.GetHardwareOpus())};

    OpusMultiStreamParametersEx ex{
        .sample_rate = params->sample_rate,
        .channel_count = params->channel_count,
        .total_stream_count = params->total_stream_count,
        .stereo_stream_count = params->stereo_stream_count,
        .use_large_frame_size = false,
        .mappings{},
    };
    std::memcpy(ex.mappings.data(), params->mappings.data(), sizeof(params->mappings));
    R_TRY(decoder->Initialize(ex, tmem_handle.Get(), tmem_size));

    *out_decoder = decoder;
    R_SUCCEED();
}

Result IHardwareOpusDecoderManager::GetWorkBufferSizeForMultiStream(
    Out<u32> out_size, InLargeData<OpusMultiStreamParameters, BufferAttr_HipcPointer> params) {
    R_TRY(impl.GetWorkBufferSizeForMultiStream(*params, *out_size));
    LOG_DEBUG(Service_Audio, "size {:#x}", *out_size);
    R_SUCCEED();
}

Result IHardwareOpusDecoderManager::OpenHardwareOpusDecoderEx(
    Out<SharedPointer<IHardwareOpusDecoder>> out_decoder, OpusParametersEx params, u32 tmem_size,
    InCopyHandle<Kernel::KTransferMemory> tmem_handle) {
    LOG_DEBUG(Service_Audio, "sample_rate {} channel_count {} transfer_memory_size {:#x}",
              params.sample_rate, params.channel_count, tmem_size);

    auto decoder{std::make_shared<IHardwareOpusDecoder>(system, impl.GetHardwareOpus())};
    R_TRY(decoder->Initialize(params, tmem_handle.Get(), tmem_size));

    *out_decoder = decoder;
    R_SUCCEED();
}

Result IHardwareOpusDecoderManager::GetWorkBufferSizeEx(Out<u32> out_size,
                                                        OpusParametersEx params) {
    R_TRY(impl.GetWorkBufferSizeEx(params, *out_size));
    LOG_DEBUG(Service_Audio, "size {:#x}", *out_size);
    R_SUCCEED();
}

Result IHardwareOpusDecoderManager::OpenHardwareOpusDecoderForMultiStreamEx(
    Out<SharedPointer<IHardwareOpusDecoder>> out_decoder,
    InLargeData<OpusMultiStreamParametersEx, BufferAttr_HipcPointer> params, u32 tmem_size,
    InCopyHandle<Kernel::KTransferMemory> tmem_handle) {
    LOG_DEBUG(Service_Audio,
              "sample_rate {} channel_count {} total_stream_count {} stereo_stream_count {} "
              "use_large_frame_size {}"
              "transfer_memory_size {:#x}",
              params->sample_rate, params->channel_count, params->total_stream_count,
              params->stereo_stream_count, params->use_large_frame_size, tmem_size);

    auto decoder{std::make_shared<IHardwareOpusDecoder>(system, impl.GetHardwareOpus())};

    R_TRY(decoder->Initialize(*params, tmem_handle.Get(), tmem_size));

    *out_decoder = decoder;
    R_SUCCEED();
}

Result IHardwareOpusDecoderManager::GetWorkBufferSizeForMultiStreamEx(
    Out<u32> out_size, InLargeData<OpusMultiStreamParametersEx, BufferAttr_HipcPointer> params) {
    R_TRY(impl.GetWorkBufferSizeForMultiStreamEx(*params, *out_size));
    LOG_DEBUG(Service_Audio,
              "sample_rate {} channel_count {} total_stream_count {} stereo_stream_count {} "
              "use_large_frame_size {} -- returned size {:#x}",
              params->sample_rate, params->channel_count, params->total_stream_count,
              params->stereo_stream_count, params->use_large_frame_size, *out_size);
    R_SUCCEED();
}

Result IHardwareOpusDecoderManager::GetWorkBufferSizeExEx(Out<u32> out_size,
                                                          OpusParametersEx params) {
    R_TRY(impl.GetWorkBufferSizeExEx(params, *out_size));
    LOG_DEBUG(Service_Audio, "size {:#x}", *out_size);
    R_SUCCEED();
}

Result IHardwareOpusDecoderManager::GetWorkBufferSizeForMultiStreamExEx(
    Out<u32> out_size, InLargeData<OpusMultiStreamParametersEx, BufferAttr_HipcPointer> params) {
    R_TRY(impl.GetWorkBufferSizeForMultiStreamExEx(*params, *out_size));
    LOG_DEBUG(Service_Audio, "size {:#x}", *out_size);
    R_SUCCEED();
}

void IHardwareOpusDecoderManager::OpenHardwareAudioCodecSession(HLERequestContext& ctx) {
    LOG_WARNING(Service_Audio, "(STUBBED) hwopus 100, returning a hwacod session");
    IPC::ResponseBuilder rb{ctx, 2, 0, 1};
    rb.Push(ResultSuccess);
    rb.PushIpcInterface<IHardwareAudioCodecSession>(ctx, system);
}

} // namespace Service::Audio
