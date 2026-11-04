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
- **Yoshi and the Mysterious Book** starts in docked mode and plays its intro (surface operations on
  texel buffers, `SUATOM`, array gathers, a few integer forms, its GPU-only memory backed whole, and
  streamed-in shaders, compute ones included, retried until their code has arrived). The first
  mission was still black on the last build tested in play; the shaders it rejected are translated
  since, not tested there yet.
- **Kirby Air Riders**: menus, races and results render, and races now show their scenery (Selevet's
  white-frame fixes, Lichii's title-crash and shader-stall fixes, many new shader forms); no rejected
  shaders in a short run. The first race stutters while its shaders build.
- **Mario Kart World**: Nemyu's fixes for the 2162-0001 title-screen abort and its stability guards,
  and the tour movie fixed on AMD GPUs.
- GPU memory reused at the same addresses is mapped again when its pages changed (from
  [AMDEGL](https://github.com/Purebuilt/AMDEGL); fixes stale reads behind Mario Kart World freezes).
- Up to date with Eglantine nx2 d9558f05a (2026-11-04): DK Bananza frame wait, Hyrule Warriors and
  Pokopia shader forms, buffer cache fixes, FSR 3.1 in place of a game's DLSS.
- Metroid Prime 4 loose RomFS dumps load, and several other fixes listed in
  [pattex/README.md](pattex/README.md).

## Games tested

Tested on Windows 11, Ryzen 7 9800X3D and Radeon RX 9070 XT, in short sessions.

| Game | State |
|---|---|
| Star Fox | Title, intro cutscenes, training mission, sound; no rejected shaders (needs the window patch). |
| Yoshi and the Mysterious Book | Docked: the game starts, title, intro and Bowser Jr. movie play; it can still abort (2162-0001) after the intro, about one run in three. **The first mission was still black on the last build tested in play**; its rejected shaders are translated since (patches 0017-0018), not tested in the mission yet. |
| Mario Kart World | Title, tour movie, races at ~40-45 FPS; no rejected shaders. One short run in two aborted during the tour movie since the latest Eglantine merge. |
| Donkey Kong Bananza | Saves load and play; no rejected shaders; ~28 FPS since Eglantine's frame wait (`EDEN_NX2_FRAMES_AHEAD=1` for the old behaviour). |
| Kirby Air Riders | Menus, races and results; races show their scenery, the first one stutters while shaders build; no rejected shaders in a short run. |
| Metroid Prime 4 | 60 FPS from loose dumps. |

Only play games you own and dumped yourself. No keys, firmware or game files are provided here.

## Building

Follow the [General Build Guide](docs/Build.md); the default branch of this repository is `pattex`.
A prebuilt Windows x64 build is in [Releases](https://github.com/pattex67/Aubepine/releases)
(no keys, firmware or games included).

## Patches

[`pattex/patches/`](pattex/patches) holds every change of this fork as `git format-patch` files on top
of PokeDinner, with what each one contains in [pattex/README.md](pattex/README.md). The Star Fox and
Yoshi work (patches 0004 to 0006) touches only a few files and should apply to Eglantine with little
adjustment.

## Credits

Aubépine stands on the work of the Eden, Yuzu and Sudachi developers, Eglantine (Djihads80 and its
contributors), PokeDinner (Franseven), Nemyu (Uesleizito Zin), Selevet Eglantine (sylvieistaken),
MythosOfs, monkeefunnee, Purebuilt (AMDEGL) and zortos293 (Lichii). The fork's own changes were made with an AI assistant (Claude Opus 5.5).

## License

Aubépine is licensed under the GPLv3 (or any later version), like Eglantine. Refer to the
[LICENSE.txt](LICENSE.txt) file.

---

## En français

**Aubépine** est un émulateur expérimental de la Nintendo Switch 2, fork d'Eglantine et de PokeDinner.
Il regroupe des correctifs de plusieurs forks et ajoute son propre travail : **Star Fox** démarre et se
joue (codec audio matériel du SDK 21, son du nouveau mixeur, nouvelles instructions de shader ; il faut
encore un petit patch de jeu, voir [pattex/README.md](pattex/README.md)), **Yoshi et le Livre
mystérieux** se lance en mode TV et joue son intro (il peut encore planter après, environ une fois sur
trois) ; **l'écran était encore noir à la première mission** au dernier essai, ses shaders refusés sont
traduits depuis mais pas encore testés en mission. Les courses de **Kirby** affichent maintenant leurs
décors (la première rame pendant la compilation des shaders), et Mario Kart World, DK Bananza et
Metroid Prime 4 profitent des correctifs regroupés. À jour avec Eglantine nx2 d9558f05a (4 novembre). Testé sur Windows 11 avec une Radeon RX 9070 XT. Une version
compilée pour Windows est dans les [Releases](https://github.com/pattex67/Aubepine/releases) ; pour
compiler soi-même, suivre [docs/Build.md](docs/Build.md) sur la branche `pattex`. Ne jouez qu'à des jeux que vous
possédez et avez extraits vous-même : aucun fichier de jeu, clé ou firmware n'est fourni.
