# Aubépine (pattex67/Aubepine) — what this fork adds

Since 2026-11-08 this branch is [Lichii](https://github.com/zortos293/Lichii) nx2 (an Eglantine fork
with the community's Ocarina of Time, AMD and performance work; `aae0b063f7`, build 117 and later)
with [Lichii PR #573](https://github.com/zortos293/Lichii/pull/573) (AMD and split GPU thread fixes for
Ocarina of Time, measured on the same Radeon RX 9070 XT), the compiled-shader saving of
[CheezerWheezer's "AMD Superior" build](https://github.com/Tempus3214/Lichii), and twelve commits of
ours. Lichii had already caught up with most of what this fork carried before (Star Fox's hardware
audio codec and sound mixer, the Switch 2 mouse, the Mario Kart World and DK Bananza frame modes,
FSR 3, loose ExeFS dumps), and it is far ahead on Ocarina of Time; our commits keep what it still
lacked for our games on an AMD Radeon.

Tested on Windows 11, Ryzen 7 9800X3D, Radeon RX 9070 XT (AMD driver), in short sessions. The work
was done with an AI assistant (Claude Opus 5.5: reverse engineering, code and testing).

## Patches

`patches/` holds the commits we add as `git format-patch` files: `git am` of 0001 to 0008 on Lichii
nx2 `92f0212408`, then merging Lichii nx2 `aae0b063f7` and PR #573 (`a2178a6371`), then 0009 to 0012,
reproduces this branch's code.

| Patch | Content |
|---|---|
| 0001 | Build: renames a local that hides a member in `kepler_compute.cpp` (MSVC turns C4458 into an error). |
| 0002 | `EDEN_NX2_TAS_TRIGGER=<file>`: the TAS scripts start when that file appears (scripted tests without touching the window). |
| 0003 | Mario Kart World's "recommended settings" question can be answered once ("Don't ask again"); Lichii asks at every boot. |
| 0004 | **Vulkan allocator, AMD**: images up to 1.5 GiB get memory (the limit was 2^28 texels). Ocarina of Time's 24576x24576 Kokiri Forest shadow map was created with no memory on this base: no cast shadows, black rocks, more ground fog. |
| 0005 | **Star Fox**: SURED on 2D_ARRAY and 3D surfaces with any operation; a Star Fox compute program was rejected on this base. |
| 0006 | **Yoshi**: ALD in a geometry program's exit envelope; one of Yoshi and the Mysterious Book's geometry programs was rejected. |
| 0007 | Shows the emulator as **Aubépine** (window title, menus, dialogs, version); the executable stays `eglantine.exe`; the update checker stays off in our builds and points at this repository. Hotkey ids, the Qt application name and the `eden` data folder are unchanged. |
| 0008 | **Kirby Air Riders, AMD**: native storage buffer pointers take the size word packed after them when it looks like one, as in our previous base. Bound to their whole mapping (upstream 2e12e0def5), Kirby Air Riders lost the GPU about 90 s in. `EDEN_NX2_NATIVE_MAPPING_SIZE=1` brings the upstream sizing back. |
| 0009 | **Compiled shaders are saved** (`VK_KHR_pipeline_binary`, by CheezerWheezer, from the "AMD Superior" build): AMD's Windows driver keeps no compiled pipelines between sessions, so every game compiled them again at each boot. They are now stored per game (`cache/shader/<title id>/vulkan_binaries.*.bin`) and loaded on later runs (Kirby Air Riders: 2113 pipelines at 0.14 ms each). Setting: *Graphics > Advanced > Save compiled shaders*; skipped on NVIDIA. |
| 0010 | Build: when GitHub times out on a CI package's checksum and the package is already in the CPM cache, configuring goes on with that package. |
| 0011 | **Kirby Air Riders**: SHF in a geometry program's exit envelope; one of its geometry programs was rejected. |
| 0012 | **Icon**: Aubépine's hawthorn berries for the executable, the window and the About dialog. |

`patches-pokedinner/` keeps the 40 patches of the previous base (PokeDinner + Eglantine nx2 up to
e736dfde9e) for reference; the branch history keeps them too, as the first parent of the merge that
moved to Lichii.

## Star Fox window patch

Star Fox's SDK checks that each display texture NVN created is aligned the way its display library
wants (8 KiB); under emulation NVN places them at a 4 KiB offset, the check fails and the game crashes
on its first frame (`nn::wsi::ounce::native::NativeWindow::Acquire` on a null window). The root cause
is not fixed yet. `star-fox/load/0400E88025B1A000/fenetre-starfox/exefs/<build id>.ips` skips the two
checks in the SDK (build id 41D4DE4A3B78214AD20C896960DF0E5EA0ED595F, base and update 131072); copy
the `0400E88025B1A000` folder into the emulator's `load` folder. The emulation itself is not affected:
the display then reads the right image with the right layout.

## Games tested on this branch

Windows 11, Ryzen 7 9800X3D, Radeon RX 9070 XT, short sessions closed normally.

| Game | State |
|---|---|
| The Legend of Zelda: Ocarina of Time | Two 5-minute runs from a save without a crash, no rejected shaders, no white flashes. Lichii's built-in Max Resolution and Dynamic FPS add-ons are on by default: the game renders at full resolution, 28-30 FPS here since PR #573 (23-24 before); turn Max Resolution off in Add-Ons for speed. Its matrix programs run on `VK_KHR_cooperative_matrix` on RDNA4. The second run reloaded its 707 compiled shaders. Outdoor daytime views not checked on this branch yet. |
| Kirby Air Riders | 4 minutes: menus and a race with its scenery, no rejected shaders, no GPU loss with patch 0008. From the second session on, its compiled shaders are reloaded instead of compiled again. Closing the emulator during a race can hang. |
| Donkey Kong Bananza | Saves load and play, no rejected shaders. |
| Mario Kart World | Title and races, 31-54 FPS, no rejected shaders. |
| Star Fox | Intro and training, 33-34 FPS, no rejected shaders (needs the window patch). |
| Yoshi and the Mysterious Book | Starts and plays its intro, no rejected shaders; closing the emulator during the game can hang. Not run past the Bowser Jr. movie on this branch, where it used to abort. |
| Metroid Prime 4 | Not run on this branch yet. |

## Credits

Eden, Yuzu and Sudachi developers; Eglantine (Djihads80 and its contributors); Lichii (zortos293 and
its many contributors: Duck912, Jonas, Kajcsi, Inc0rporate, kraibse, chuppatricky and others);
PokeDinner (Franseven); Nemyu (Uesleizito Zin); Selevet Eglantine (sylvieistaken); MythosOfs;
monkeefunnee; Purebuilt (AMDEGL). Licence: GPLv3, as Eglantine and Lichii.

---

# En français

Depuis le 8 novembre 2026, Aubépine repose sur **Lichii** nx2 (build 117 et suivants), avec la PR
Lichii #573 (correctifs AMD pour Zelda, mesurés sur la même RX 9070 XT : 28-30 FPS en pleine
résolution au lieu de 23-24) et la **sauvegarde des shaders compilés** de la build « AMD Superior »
(le pilote AMD ne les garde pas d'une session à l'autre). Nos commits ajoutent ce qui manquait encore
pour nos jeux sur une Radeon : la carte d'ombres de la forêt Kokiri avec de la mémoire, des
instructions de shader (Star Fox, Yoshi, Kirby), la taille des buffers natifs qui évite à Kirby de
perdre le GPU, le nom et l'**icône** Aubépine (des baies d'aubépine), la question de Mario Kart World
mémorisée et notre outil de test. Les anciens patchs (base PokeDinner) restent dans `patches-pokedinner/`, le patch de
fenêtre de Star Fox dans `star-fox/` (à copier dans le dossier `load`). Testé en sessions courtes sur
Windows 11 avec une Radeon RX 9070 XT ; travail fait avec un assistant IA (Claude Opus 5.5).
