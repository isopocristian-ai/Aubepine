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

<h4 align="center"><b>Aubépine</b> is an experimental Nintendo Switch 2 emulator: a fork of
<a href="https://github.com/Djihads80/Eglantine">Eglantine</a> and
<a href="https://git.eden-emu.dev/Franseven/PokeDinner-a-Eglantine-Fork">PokeDinner</a>
(both based on Eden, Yuzu and Sudachi), gathering fixes from several forks and adding its own work
on games that did not run elsewhere.</h4>

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

- **Star Fox** boots and plays (SDK 21.4.4): the new hardware audio codec (`nn::codec::hwacod`, opened
  through `hwopus` 100) with real Opus decoding, the Switch 2 sound mixer streams played through the
  audio output, and about fifteen more Ampere shader forms. It still needs a small game patch for its
  window check, see [pattex/README.md](pattex/README.md#star-fox-window-patch).
- **Yoshi and the Mysterious Book**: title and intro run at 60 FPS (surface operations on texel
  buffers, `SUATOM`, array gathers and a few integer forms), but it does not get into the game yet:
  in docked mode it stops after the intro movie (see the table below).
- **Kirby Air Riders**: menus, races and results render (Selevet's white-frame fixes and new shader
  forms); fewer rejected shaders than upstream.
- **Mario Kart World**: Nemyu's fixes for the 2162-0001 title-screen abort and its stability guards,
  and the tour movie fixed on AMD GPUs.
- Metroid Prime 4 loose RomFS dumps load, and several other fixes listed in
  [pattex/README.md](pattex/README.md).

## Games tested

Tested on Windows 11, Ryzen 7 9800X3D and Radeon RX 9070 XT, in short sessions.

| Game | State |
|---|---|
| Star Fox | Title, intro cutscenes, training mission, sound; no rejected shaders (needs the window patch). |
| Yoshi and the Mysterious Book | Title and intro cutscene at 60 FPS, but **does not get into the game yet**: in docked mode it stops right after the Bowser Jr. intro movie (abort 2162-0001; GPU-only memory backed as written makes two shaders read as zeros). Being worked on. |
| Mario Kart World | Title, tour movie, races at ~40-45 FPS; no rejected shaders. |
| Donkey Kong Bananza | Saves load and play; no rejected shaders. |
| Kirby Air Riders | Menus, races and results; race scenery still partly dark; 8 rejected shaders. |
| Metroid Prime 4 | 60 FPS from loose dumps. |

Only play games you own and dumped yourself. No keys, firmware or game files are provided here.

## Building

Follow the [General Build Guide](docs/Build.md); the default branch of this repository is `pattex`.
There are no prebuilt releases yet.

## Patches

[`pattex/patches/`](pattex/patches) holds every change of this fork as `git format-patch` files on top
of PokeDinner, with what each one contains in [pattex/README.md](pattex/README.md). The Star Fox and
Yoshi work (patches 0004 to 0006) touches only a few files and should apply to Eglantine with little
adjustment.

## Credits

Aubépine stands on the work of the Eden, Yuzu and Sudachi developers, Eglantine (Djihads80 and its
contributors), PokeDinner (Franseven), Nemyu (Uesleizito Zin), Selevet Eglantine (sylvieistaken),
MythosOfs and monkeefunnee. The fork's own changes were made with an AI assistant (Claude Opus 5.5).

## License

Aubépine is licensed under the GPLv3 (or any later version), like Eglantine. Refer to the
[LICENSE.txt](LICENSE.txt) file.

---

## En français

**Aubépine** est un émulateur expérimental de la Nintendo Switch 2, fork d'Eglantine et de PokeDinner.
Il regroupe des correctifs de plusieurs forks et ajoute son propre travail : **Star Fox** démarre et se
joue (codec audio matériel du SDK 21, son du nouveau mixeur, nouvelles instructions de shader ; il faut
encore un petit patch de jeu, voir [pattex/README.md](pattex/README.md)), **Yoshi et le Livre
mystérieux** affiche son écran titre et sa cinématique mais ne va pas encore en jeu (en mode TV il
s'arrête après la vidéo de Bowser Jr.), et Kirby, Mario Kart World, DK Bananza et Metroid
Prime 4 profitent des correctifs regroupés. Testé sur Windows 11 avec une Radeon RX 9070 XT. Pour
compiler, suivre [docs/Build.md](docs/Build.md) sur la branche `pattex`. Ne jouez qu'à des jeux que vous
possédez et avez extraits vous-même : aucun fichier de jeu, clé ou firmware n'est fourni.
