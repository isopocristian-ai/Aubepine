# Aubépine (pattex67/Aubepine) — what this fork adds

Since 2026-11-08 this branch is [Lichii](https://github.com/zortos293/Lichii) nx2 (an Eglantine fork
with the community's Ocarina of Time, AMD and performance work; `aae0b063f7`, build 117 and later, then
merged again on 2026-11-09 at `3dae119aea`, build 125 and later)
with [Lichii PR #573](https://github.com/zortos293/Lichii/pull/573) (AMD and split GPU thread fixes for
Ocarina of Time, measured on the same Radeon RX 9070 XT), the image store bounds checks of
[CheezerWheezer's "AMD Superior" build](https://github.com/Tempus3214/Lichii) (as ported to Fritchii by
Frizti), fixes for Yoshi and the
Mysterious Book taken from [Orchid](https://github.com/MythosOfs/Orchid), Satsuma, Nemyu and a Lichii
branch, and commits of ours (all listed below). Lichii had already caught up with most of what this fork carried before (Star Fox's hardware
audio codec and sound mixer, the Switch 2 mouse, the Mario Kart World and DK Bananza frame modes,
FSR 3, loose ExeFS dumps), and it is far ahead on Ocarina of Time; our commits keep what it still
lacked for our games on an AMD Radeon.

Tested on Windows 11, Ryzen 7 9800X3D, Radeon RX 9070 XT (AMD driver), in short sessions. The work
was done with an AI assistant (Claude Opus 5.5: reverse engineering, code and testing).

## Patches

`patches/` holds the commits we add as `git format-patch` files: `git am` of 0001 to 0008 on Lichii
nx2 `92f0212408`, then merging Lichii nx2 `aae0b063f7` and PR #573 (`a2178a6371`), then 0009 to 0022,
then merging Lichii nx2 `3dae119aea`, then 0023 to 0025, reproduces this branch's code. That last
merge drops 0009 and 0010, which Lichii now does its own way (its shader cache v2, #1036, and #792),
keeps 0004's 1.5 GiB rule next to Lichii's #880, and keeps 0013's guard on image downloads next to
Lichii's stale-image clear (#818); the merge commit in the branch history has the exact resolution.

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
| 0009 | *(Dropped in the merge with Lichii build 125: Lichii's shader cache v2, #1036, saves programs and the Vulkan pipeline cache per game, and leaves AMD's own driver cache on; Kirby Air Riders' second boot took 40 s here, its first 200 s.)* **Compiled shaders are saved** (`VK_KHR_pipeline_binary`, by CheezerWheezer, from the "AMD Superior" build): AMD's Windows driver keeps no compiled pipelines between sessions, so every game compiled them again at each boot. They are now stored per game (`cache/shader/<title id>/vulkan_binaries.*.bin`) and loaded on later runs (Kirby Air Riders: 2113 pipelines at 0.14 ms each). Setting: *Graphics > Advanced > Save compiled shaders*; skipped on NVIDIA. |
| 0010 | *(Dropped in the merge with Lichii build 125, replaced by Lichii #792.)* Build: when GitHub times out on a CI package's checksum and the package is already in the CPM cache, configuring goes on with that package. |
| 0011 | **Kirby Air Riders**: SHF in a geometry program's exit envelope; one of its geometry programs was rejected. |
| 0012 | **Icon**: Aubépine's hawthorn berries for the executable, the window and the About dialog. |
| 0013 | Vulkan: texture uploads and copies wait for every stage that uses them (MythosOfs, [Orchid](https://github.com/MythosOfs/Orchid) `cc04f48b3d`, found with Khronos sync validation on Yoshi's chapter scene). |
| 0014 | Texture cache: a layer with no guest bytes behind it unswizzles as zeros instead of reading past an empty span (the texture part of Orchid `f2eb04babd`; its buffer cache part is already in Lichii as #397). |
| 0015 | **Yoshi, docked**: its GPU-only regions are backed whole (backed as written, its shaders read as zeros after the intro movie and the game aborted, 2162-0001); destroying GPU memory (`nx2:gpu` 102) drops the caches' state for it as an unmap, so nothing old is copied over the heap the game puts there. From our previous base. |
| 0016 | **Yoshi**: a rejected native program is read again every 250 ms and lowered again when its bytes changed (zortos293's opt-in recheck from a Lichii branch, `46a184c4e3`, on by default for Yoshi; `EDEN_NX2_REJECT_RECHECK=1` for every game, `=0` off); TLD4.B `0x364` and uniform-handle gathers with an offset (a purple screen on the way to the chapter page). |
| 0017 | **Yoshi, first mission**: LDG through a descriptor pair (`0x381`) and I2F.F16 of an immediate (`0x906`); the two compute programs behind the mission's black screen. |
| 0018 | **Yoshi**: DMA copies, DMA clears and inline uploads over scattered device pages go page by page (found by SkiierCo/Satsuma `8400fe0f9b`, as Nemyu `9ff295b316`): Yoshi moves its shader code with the DMA engine over such ranges, and copied as one run they read the wrong pages, so its programs read as zeros (Lichii issues #347, #755). IMAD with a negated constant-buffer addend (three book-page compute programs). |
| 0019 | **Kirby Air Riders**: TEX with its handle in a uniform register (`0xf60`, and `0xb60`) on registers above the reserved range goes through shadow copies; four of its menu compute programs were rejected. |
| 0020 | **Resolution scaling**: the helper that scales images whose format has no blit support (D24S8 on AMD, and MSAA images) read the image it was writing, so the scaled copy came out empty (a bug since yuzu); a new scaled image is moved to its working layout first. Kirby Air Riders' races were black with checkerboards at 2x. |
| 0021 | Resolution scaling: 2D images with a mip chain can be scaled (first attempt for the grid fixed by 0022; on its own it blacked out Mario Kart World's races at 2x). |
| 0022 | **Kirby Air Riders at 2x**: a mip chain is scaled only while its level 0 is rendered with a depth buffer that scales. The game renders its race into such chains with its depth buffer bound; kept at 1x, they forced that depth down and back up every frame, which keeps one depth sample in four, and its main pass (depth test EQUAL) drew one pixel in four: a grid of dots and dark characters. `EDEN_NX2_RESCALE_MIPS=0` turns this off. Only acts when resolution scaling is on. |
| 0023 | **AMD**: the ASTC decoder and the unswizzle shaders check every image store against the image's edge (by Frizti and CheezerWheezer: "AMD Superior" build 7 `132cc46c9e`, as ported to Fritchii, QuincyForbes/Lichii `c739320e5b`). Edge blocks of images whose size isn't a multiple of the block size, and workgroups rounded past the image, stored texels outside it; NVIDIA drops those, AMD wrote them into other images (black blocks, a crosshatch) and could lose the GPU. |
| 0024 | Build: renames a local that hides another in `nx2_stutter_trace.cpp` (MSVC turns C4456 into an error). |
| 0025 | **Kirby Air Riders at 2x, updated game**: an image the mip chain rule of 0022 lets scale no longer stops the render targets that alias it from scaling. The updated game renders its race at a dynamic 1712x960 inside 2048x1152 mip chains, and the grid of dots stayed there. |

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
| The Legend of Zelda: Ocarina of Time | 5-minute runs from a save without a crash, no rejected shaders, no white flashes; checked again after 0013-0018 (30-31 FPS). On Lichii build 125 (0023-0025), a 6-minute run from the save: about 58 frames per second on average, 60 most of the time, with three stutters of 0.6 to 1.5 s, no rejected shaders (GPU accuracy Low, DMA Unsafe). Lichii's built-in FPS Unlock (#655) now takes Dynamic FPS's place and keeps the game's speed; it and Max Resolution are add-ons on by default: the game renders at full resolution (28-31 FPS here with Dynamic FPS since PR #573, 23-24 before); turn Max Resolution off in Add-Ons for speed. Its matrix programs run on `VK_KHR_cooperative_matrix` on RDNA4. The second run reloaded its 707 compiled shaders. Outdoor daytime views not checked on this branch yet. |
| Kirby Air Riders | 4 minutes: menus and races with their scenery, no rejected shaders (four menu compute programs before 0019), no GPU loss with patch 0008. At 2x resolution its races render since 0020 and 0022 (black with checkerboards, then a grid of dots with dark characters before), and in the updated game since 0025: two races at 2x on Lichii build 125, no grid. The first session after an update of the emulator compiles its shaders again (about 200 s), the next ones boot in about 40 s. From the second session on, its compiled shaders are reloaded instead of compiled again. Still seen at 1x and 2x: Kirby's model is dark on the vehicle select, and the character preview is magenta for a moment. Closing the emulator during a race can hang. |
| Donkey Kong Bananza | Saves load and play, no rejected shaders. |
| Mario Kart World | Title and races, 31-54 FPS, no rejected shaders; checked again at 2x after 0022 (menus, course flyovers and a race start render). |
| Star Fox | Intro and training, 33-34 FPS, no rejected shaders (needs the window patch). |
| Yoshi and the Mysterious Book | Docked, played by hand: intro, chapter page, the first mission (black before 0017) and a second level render, about 10 minutes in one session; no compute program rejected after 0018. One vertex program on the title screen still reads as zeros (three draws skipped, nothing visible). A book page's illustration that was missing before 0018 was not checked again. Closing the emulator during the game can hang. |
| Metroid Prime 4 | Not run on this branch yet. |

After the merge with Lichii build 125 (2026-11-09), only Ocarina of Time and Kirby Air Riders were run
again; the other rows describe the build before it.

## Credits

Eden, Yuzu and Sudachi developers; Eglantine (Djihads80 and its contributors); Lichii (zortos293 and
its many contributors: Duck912, Jonas, Kajcsi, Inc0rporate, kraibse, chuppatricky and others);
PokeDinner (Franseven); Nemyu (Uesleizito Zin); Selevet Eglantine (sylvieistaken); MythosOfs
(Orchid); SkiierCo (Satsuma); CheezerWheezer; Frizti (Fritchii); monkeefunnee; Purebuilt (AMDEGL). Licence: GPLv3, as Eglantine and Lichii.

---

# En français

Depuis le 8 novembre 2026, Aubépine repose sur **Lichii** nx2 (build 125 et suivants depuis le 9), avec la PR
Lichii #573 (correctifs AMD pour Zelda, mesurés sur la même RX 9070 XT : 28-30 FPS en pleine
résolution au lieu de 23-24) et la vérification des bords d'image dans le décodage ASTC de la build
« AMD Superior » (reprise par Frizti dans Fritchii : sur AMD, des blocs noirs puis la perte du GPU).
La sauvegarde des shaders compilés est désormais celle de Lichii (cache v2), et Zelda tourne vers
60 FPS avec le patch « FPS Unlock » intégré à Lichii. Nos commits ajoutent ce qui manquait encore
pour nos jeux sur une Radeon : la carte d'ombres de la forêt Kokiri avec de la mémoire, des
instructions de shader (Star Fox, Yoshi, Kirby), la taille des buffers natifs qui évite à Kirby de
perdre le GPU, le nom et l'**icône** Aubépine (des baies d'aubépine), la question de Mario Kart World
mémorisée et notre outil de test. **Yoshi et le Livre mystérieux** passe en mode TV : la première mission
n'est plus noire et les shaders du jeu ne sont plus lus vides (copies du GPU sur des pages dispersées,
trouvé par Satsuma ; correctifs d'Orchid, de Nemyu, de Lichii et les nôtres, patchs 0013 à 0018). **Kirby Air Riders** s'affiche en résolution 2x : la copie agrandie du tampon de profondeur n'est plus vide (décor noir avant) et la course n'est plus dessinée sur un pixel sur quatre (maillage de points et Kirby trop sombre avant ; patchs 0019 à 0022), aussi dans le jeu mis à jour (patch 0025). Les anciens patchs (base PokeDinner) restent dans `patches-pokedinner/`, le patch de
fenêtre de Star Fox dans `star-fox/` (à copier dans le dossier `load`). Testé en sessions courtes sur
Windows 11 avec une Radeon RX 9070 XT ; travail fait avec un assistant IA (Claude Opus 5.5).
