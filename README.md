<!--
# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: 2018 yuzu Emulator Project
# SPDX-License-Identifier: GPL-2.0-or-later
-->
<!-- lang: en-GB -->
<h1 align="center">
  <br>
  <b>Aubépine</b>
  <br>
</h1>

<h4 align="center"><b>Aubépine</b> is an experimental Nintendo Switch 2 emulator, tuned on an AMD
Radeon: a fork of <a href="https://github.com/zortos293/Lichii">Lichii</a>, itself a fork of
<a href="https://github.com/Djihads80/Eglantine">Eglantine</a> (based on Eden, Yuzu and Sudachi),
with a few fixes of its own for the games we play on AMD.</h4>

<p align="center">
  <a href="#what-it-adds">What it adds</a> |
  <a href="#games-tested">Games tested</a> |
  <a href="#building">Building</a> |
  <a href="#patches">Patches</a> |
  <a href="#credits">Credits</a> |
  <a href="#license">License</a> |
  <a href="#en-français">Français</a>
</p>

## What it adds

Since 2026-11-08 Aubépine builds on **Lichii nx2** (build 117 and later), which brings the
community's work on **The Legend of Zelda: Ocarina of Time** (vegetation, fog, white flashes, the
upscaler, performance, matrix programs on `VK_KHR_cooperative_matrix` for RDNA4) and had caught up
with most of this fork's earlier additions (Star Fox's audio, the Switch 2 mouse, Mario Kart World and
DK Bananza frame modes, FSR 3, loose dumps). On top of it:

- **[Lichii PR #573](https://github.com/zortos293/Lichii/pull/573)**, AMD and split GPU thread fixes
  for Ocarina of Time measured on a Radeon RX 9070 XT: 28-30 FPS at full resolution here, 23-24 before.
- **Compiled shaders are saved** between sessions (`VK_KHR_pipeline_binary`, from CheezerWheezer's
  "AMD Superior" build): AMD's Windows driver doesn't keep them, so every game compiled its shaders
  again at each boot.

And our own commits:

- **Made on an AMD Radeon.** Developed and tested on a Radeon RX 9070 XT (AMD driver, Windows); not
  tested on NVIDIA or Intel GPUs.
- **Ocarina of Time on AMD**: the Kokiri Forest shadow map (24576x24576) gets its memory, so shadows
  and the lighting that depends on them work.
- **Kirby Air Riders on AMD**: native storage buffers keep the size packed after their pointer when
  there is one; with the upstream sizing the game lost the GPU about 90 s in.
- **Star Fox**, **Yoshi and the Mysterious Book** and **Kirby Air Riders**: shader forms that were
  still rejected.
- The emulator shows as **Aubépine**, with its own **icon** (hawthorn berries); the executable is still
  `eglantine.exe`. Mario Kart World's recommended-settings question can be answered once, and
  `EDEN_NX2_TAS_TRIGGER` helps scripted tests.

Details in [pattex/README.md](pattex/README.md).

## Games tested

Tested on Windows 11, Ryzen 7 9800X3D and Radeon RX 9070 XT, in short sessions.

| Game | State |
|---|---|
| The Legend of Zelda: Ocarina of Time | 5-minute runs from a save without a crash, no rejected shaders, no white flashes. Max Resolution and Dynamic FPS are built in and on by default: 28-30 FPS at full resolution here (turn Max Resolution off in Add-Ons for speed). |
| Kirby Air Riders | Menus and races with their scenery, no rejected shaders; the first session stutters while shaders compile, later ones reload them. |
| Donkey Kong Bananza | Saves load and play, no rejected shaders. |
| Mario Kart World | Title and races, 31-54 FPS, no rejected shaders. |
| Star Fox | Intro and training mission, no rejected shaders (needs the window patch in [pattex/star-fox](pattex/README.md#star-fox-window-patch)). |
| Yoshi and the Mysterious Book | Starts and plays its intro, no rejected shaders; closing during the game can hang. |
| Metroid Prime 4 | Ran at 60 FPS on our previous base; not run on this one yet. |

Only play games you own and dumped yourself. No keys, firmware or game files are provided here.

## Building

Follow the [General Build Guide](docs/Build.md); the default branch of this repository is `pattex`.
Prebuilt Windows x64 builds are in [Releases](https://github.com/pattex67/Aubepine/releases) (no keys,
firmware or games included).

## Patches

[`pattex/patches/`](pattex/patches) holds the commits we add on top of Lichii as `git format-patch`
files (how to apply them is in [pattex/README.md](pattex/README.md#patches));
[`pattex/patches-pokedinner/`](pattex/patches-pokedinner) keeps the 40 patches of the previous
PokeDinner-based version for reference.

## Credits

Aubépine stands on the work of the Eden, Yuzu and Sudachi developers, Eglantine (Djihads80 and its
contributors), Lichii (zortos293 and its many contributors), PokeDinner (Franseven), Nemyu (Uesleizito
Zin), Selevet Eglantine (sylvieistaken), MythosOfs, monkeefunnee and Purebuilt (AMDEGL). The fork's
own changes were made with an AI assistant (Claude Opus 5.5).

## License

Aubépine is licensed under the GPLv3 (or any later version), like Eglantine and Lichii. Refer to the
[LICENSE.txt](LICENSE.txt) file.

---

## En français

**Aubépine** est un émulateur expérimental de la Nintendo Switch 2, réglé sur une **Radeon RX 9070 XT**
(pilote AMD ; pas testé sur NVIDIA ni Intel). Depuis le 8 novembre 2026 il repose sur **Lichii**
(fork d'Eglantine), qui apporte tout le travail de la communauté sur **Zelda: Ocarina of Time**
(végétation, brouillard, flashs, performances, calcul matriciel RDNA4) et avait déjà repris l'essentiel
de nos anciens ajouts. S'y ajoutent la PR Lichii #573 (Zelda à 28-30 FPS en pleine résolution sur une
RX 9070 XT, 23-24 avant), la **sauvegarde des shaders compilés** entre deux sessions (le pilote AMD ne
les garde pas) et nos correctifs : la carte d'ombres de la forêt Kokiri sur AMD, la taille des buffers
qui évite à **Kirby** de perdre le GPU, des instructions de shader pour **Star Fox**, **Yoshi** et
**Kirby**, le nom et l'**icône** Aubépine (des baies d'aubépine), la question de Mario Kart World
mémorisée et un outil de test. Les six jeux testés (Zelda, Kirby, DK Bananza, Mario Kart World,
Star Fox, Yoshi) tournent sans shader refusé dans nos essais courts ; Zelda tourne en pleine résolution
(patch « Max Resolution » intégré, à désactiver dans les Add-Ons pour gagner des FPS). Une version compilée pour Windows est dans
les [Releases](https://github.com/pattex67/Aubepine/releases) ; pour compiler soi-même, suivre
[docs/Build.md](docs/Build.md) sur la branche `pattex`. Ne jouez qu'à des jeux que vous possédez et
avez extraits vous-même : aucun fichier de jeu, clé ou firmware n'est fourni.
