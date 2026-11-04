# Aubépine (pattex67/Aubepine) — what this fork adds

This branch is [PokeDinner](https://git.eden-emu.dev/Franseven/PokeDinner-a-Eglantine-Fork) (master 216b5034c)
merged with [Eglantine](https://github.com/Djihads80/Eglantine) nx2 (f50099038, build 174, then
d9558f05a on 2026-11-04) plus #374, fixes taken from other forks (credited below), and our own work to
get **Star Fox** and **Yoshi and the Mysterious Book** running. Tested on Windows 11, Ryzen 7 9800X3D, Radeon RX 9070 XT (AMD driver).

The work was done with an AI assistant (Claude Opus 5.5: reverse engineering, code and testing).
Every change has been run on the games listed below, but only in short sessions.

## Patches

`patches/` holds the code commits of this branch on top of PokeDinner master, as `git format-patch`
files. `git am` of 0001 to 0015 on PokeDinner 216b5034c reproduces the branch up to the second
Eglantine merge; the branch then merges Eglantine nx2 d9558f05a (158 commits: DK Bananza frame wait
and tensor programs, Hyrule Warriors and Pokopia shader forms, buffer cache GPU-write fixes, split
batching, FSR 3.1 in place of a game's DLSS, mesh-shader fallback; the conflicts are resolved in the
merge commit, keeping our lowerings where they cover more), and 0016 to 0019 apply on top of it:

| Patch | Content |
|---|---|
| 0001 | Removes `dist/Eglantine.icns`, which collides with `dist/eglantine.icns` on Windows (as Eglantine #378). |
| 0002 | Merge of Eglantine nx2 f50099038 into PokeDinner, conflicts resolved by hand, plus: Kirby Air Riders video decoder Stop/Finalize fix, Metroid Prime 4 loose RomFS fallback, `EDEN_NX2_UNDEF_ZERO`, AMD sparse flag, AMD pipeline workers, MKW buffer downloads, `EDEN_NX2_GPU_ONLY_LAZY_MIN` (GPU-only regions under 260 MiB backed whole on AMD: fixes Mario Kart World's green tour movie), `EDEN_NX2_VDEC_INVALIDATE`, `EDEN_NX2_TAS_TRIGGER`. |
| 0003 | Ports: [Nemyu](https://gitlab.com/nemyu-emulator/nemyu) v0.1.3 syncpoint fixes (MKW 2162-0001 title abort) and stability guards, Eglantine #374, MythosOfs syncpoint id guard, [Selevet](https://github.com/sylvieistaken/selevet-eglantine) performance build 6 Kirby white-frame fixes (blit dither bias, upscale stand-in on every dispatch). |
| 0004 | **Star Fox (SDK 21.4.4)**: `hwopus` 100 opens the new hardware audio codec (`nn::codec::hwacod`) with real Opus decoding; ULDC 8/16-bit loads; TXD with a uniform-register handle (0xf6c); MythosOfs' chat/camera commands. Also the AMD occlusion-query copy without `WAIT_BIT` (from monkeefunnee, `EDEN_NX2_QUERY_NOWAIT=0` turns it off) and diagnostic switches. |
| 0005 | **Star Fox**: the Switch 2 sound mixer streams (`audsmx:u` / sysmix) now play through Eden's output instead of being drained silently (ring data at +0x1040, s16 and f32); shader lowering for SUST with any channel mask, SURED on 2D_ARRAY/3D, `SHF.R.S32.HI`, TLD4 on ARRAY_2D, PLOP3 with a second destination. |
| 0006 | **Yoshi**: SULD/SUST/SURED on texel buffers and 1D images, SUATOM with result (0x593), TLD4.B on ARRAY_2D (0xf63), IMNMX with a wide immediate, `SHF.L.HI` forms, ATOMG with bit 90, I2FP with an immediate. |
| 0007 | Shows the emulator as **Aubépine** (window title, menus, dialogs, version); hotkey ids and the Qt application name stay. |
| 0008 | **Yoshi docked**: GPU-only regions backed whole for this game (backed as written, two shaders read as zeros and it aborted after the intro), Eglantine PR #345 (page table block id overflow), TLD4.B 0x364, 0xf63 with an offset, SURED with any operation. |
| 0009 | **Shader cache**: native programs that read as incomplete (zeros, branches past the capture) are retried every 100 ms for a minute instead of being rejected for good (Yoshi docked streams shaders in). |
| 0010 | **Shader cache**: the same retries for compute programs, and any unknown opcode is retried for a minute before being rejected; the first read that looks unwritten waits up to 100 ms for the code. Yoshi docked aborted (2162-0001) after the Bowser Jr. movie: a compute program the game had not written yet was dropped, and the game read back its missing output. |
| 0011 | **Yoshi first mission**: shader lowering for constant LEA without a carry out, with a negated constant or with a high source and carry; LDG with a descriptor pair; USHF under a UPn guard; SHF.U64.HI; TLD.LZ with a uniform-register handle (0xf66). Drops 0010's 100 ms wait (it never caught a program) and throttles compute retries to one every 100 ms. The mission is still black: more instructions behind these (opcodes 906 and fae, UIADD3 with a carry, programs that use the lowering's scratch registers). |
| 0012 | **GPU memory** (from [Purebuilt/AMDEGL](https://github.com/Purebuilt/AMDEGL) f6fba4f): when the game frees GPU memory and reuses the same addresses, the SMMU range reused for them still pointed at the old pages, so the GPU could read other data (Mario Kart World freezes there). Pages whose memory changed are mapped again; `EDEN_NX2_REFRESH_DEVICE_RANGES=0` turns it off. |
| 0013 | **Clean-up**: the diagnostics added while investigating Star Fox and Yoshi are gone (hwacod raw words, image preference words, composite and draw-target details, the `EDEN_NX2_GPU_WRITTEN_LISTS` measurement switch); hwacod is no longer logged as a stub; the shader cache's retry checks are in one place. No change in behaviour. |
| 0014 | **Yoshi first mission**: I2F.F16/F32 of an immediate (0x906), LDGSTS.E.BYPASS.128 in both forms and LDGDEPBAR (0x9af), UIADD3 with a carry into UP2 and up. |
| 0015 | **Yoshi**: when the game frees GPU memory (nx2:gpu 102) and reuses it as heap, the caches' GPU writes to it are dropped first; they were copied back over Unreal's allocator blocks later and the game aborted (2162-0001) after the intro. Fewer aborts (1 in 3 runs instead of 5 in 6), not all. |
| — | **Merge of Eglantine nx2 d9558f05a** (see above). DK Bananza now waits for each frame (upstream's fix for freezes), about 28 FPS instead of 32 here; `EDEN_NX2_FRAMES_AHEAD=1` brings the old behaviour back. NVIDIA DLSS through Streamline is left off in our Windows build. |
| 0016 | **Kirby Air Riders** title crash after the merge (after [zortos293/Lichii](https://github.com/zortos293/Lichii) 12db3f599, own implementation): the buffer cache flushes the front thread needs (macro parameters, GPU-written command lists, semaphores) run on the back thread. |
| 0017 | **Kirby Air Riders races**: UIADD3 with three sources and two uniform carry outputs, FRND.F16, and shadow copies for ISETP, IADD3 with an immediate, FMUL, FADD and FFMA that use R216 and up (the registers the lowering keeps for uniform registers). |
| 0018 | **Kirby races and Yoshi's mission**: shadow copies for every plain ALU form (LOP3, IADD3, IMAD, SHF, PRMT, LEA, FMNMX, SEL, FSEL, ISETP, FSETP), IMNMX, conversions (F2F, F2I, I2F, FRND, I2FP), S2R, MOV, LDG/STG with a descriptor register and STG data in R246 and up; F2I.F16 with any rounding; a program that fails at its first instruction is retried like a partial upload. Kirby's short test run: no rejected shaders left. |
| 0019 | **Shader stalls** (after Lichii #71, own implementation): a draw or dispatch that waits on a pipeline still queued behind hundreds of builds builds it itself; the structured control flow pass compares statement order from both ends (large Kirby compute shaders translate faster). |

Patches 0004 to 0006 are this fork's own work and only touch a few files
(`lower_to_maxwell.h`, `hardware_opus_decoder_manager.*`, `nx2_audio_device_manager.h`); they should
apply to Eglantine nx2 with little or no adjustment.

## Star Fox window patch

Star Fox's SDK checks that each display texture NVN created is aligned the way its display library
wants (8 KiB); under emulation NVN places them at a 4 KiB offset, the check fails and the game crashes
on its first frame (`nn::wsi::ounce::native::NativeWindow::Acquire` on a null window). The root cause
is not fixed yet. `star-fox/load/0400E88025B1A000/fenetre-starfox/exefs/<build id>.ips` skips the two
checks in the SDK (build id 41D4DE4A3B78214AD20C896960DF0E5EA0ED595F, base and update 131072); copy
the `0400E88025B1A000` folder into Eglantine's `load` folder. The emulation itself is not affected:
the display then reads the right image with the right layout.

## Games tested on this branch

| Game | State |
|---|---|
| Mario Kart World | Title, tour movie, races; no rejected shaders; ~40-45 FPS on an RX 9070 XT. Since the d9558f05a merge, one short run in two aborted during the tour movie (an intermittent title-screen abort was already open). |
| Donkey Kong Bananza | Saves load and play; no rejected shaders; ~28 FPS in the first cave since the merge (frame wait, see above). |
| Kirby Air Riders | Menus, races and results render; the race now shows its scenery but the first race stutters while hundreds of shaders build (the next ones use the disk cache); no rejected shaders in a short run. |
| Metroid Prime 4 | 60 FPS from loose dumps (RomFS fallback). |
| Star Fox | Title, intro cutscenes, training mission, sound; no rejected shaders (needs the window patch). |
| Yoshi and the Mysterious Book | Docked: the game starts, title, intro and Bowser Jr. movie play. It can still abort (2162-0001) after the intro, about one run in three. **The first mission was still black on the last build tested in play**; patches 0017 and 0018 translate the shaders it rejected there, not tested in the mission yet. |

## Credits

Eden, Yuzu and Sudachi developers; Eglantine (Djihads80 and its contributors); PokeDinner (Franseven);
Nemyu (Uesleizito Zin); Selevet Eglantine (sylvieistaken); MythosOfs; monkeefunnee; Purebuilt (AMDEGL);
zortos293 (Lichii). Licence: GPLv3,
as Eglantine.

---

# En français

Aubépine réunit PokeDinner, Eglantine nx2 (build 174 + #374), des correctifs repris d'autres
forks (Nemyu, Selevet, MythosOfs) et notre travail pour faire tourner **Star Fox** (codec audio matériel
`hwacod`, sortie son sysmix, une quinzaine d'instructions de shader) et **Yoshi et le Livre mystérieux**
(se lance en mode TV et joue son intro ; l'écran était encore noir à la première mission au dernier
essai, les shaders refusés là sont traduits depuis mais pas encore testés en mission). Depuis le
4 novembre, la branche inclut aussi Eglantine nx2 d9558f05a, et les courses de Kirby affichent leurs
décors (la première course rame pendant la compilation des shaders). Les patchs sont dans `patches/`, le patch de fenêtre de Star Fox dans
`star-fox/` (à copier dans le dossier `load` d'Eglantine). Travail fait avec un assistant IA
(Claude Opus 5.5), testé en sessions courtes sur Windows 11 avec une Radeon RX 9070 XT.
