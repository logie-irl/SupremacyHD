# Supremacy HD for macOS

A native macOS port of **Supremacy: Your Will Be Done** (Probe Software /
Virgin Mastertronic, 1990), the Amiga strategy game, for **macOS 27.x**
(tested on Apple Silicon).

The game is a translation of the original Amiga program into C, running on
[SDL 3](https://www.libsdl.org/). It plays exactly like the Amiga original as
the WHDLoad install (V1.1) runs it: the same rules, the same random numbers,
the same screens, sound and music, including the intro, the four victory
sequences and the defeat sequence. On top of that it adds modern display
options, hi-res pictures, optional bug fixes and a trainer.

There are three apps, built from the same source:

- **Supremacy HD.app**: the hi-res game with **no game files inside**. The
  launcher asks for your game files the first time (BROWSE or EXTRACT), so
  this is the one you can share.
- **SupremacyHD.app**: the same, with your own game files inside (for you only).
- **SuperMac.app**: the game as the original looked, with your game files inside.

> **You need your own copy of the game.** This repository contains none of
> the original's code, graphics, sound, music or text (the program icon,
> converted from the game's icon, is the only exception; see
> [Licence](#licence)). The game reads everything else from your copy.
> See [Getting the game data](#getting-the-game-data).

## Contents

- [Quick start](#quick-start)
- [Getting the game data](#getting-the-game-data)
- [Playing](#playing)
- [Display options](#display-options)
- [Supremacy HD (hi-res pictures)](#supremacy-hd-hi-res-pictures)
- [Trainer](#trainer)
- [Bug fixes](#bug-fixes)
- [Building](#building)
- [What the port does](#what-the-port-does)
- [Testing](#testing)
- [Project layout](#project-layout)
- [Licence](#licence)
- [Acknowledgements](#acknowledgements)

## Quick start

Install [Homebrew](https://brew.sh/), then in Terminal:

```sh
brew install cmake sdl3
git clone https://github.com/logie-irl/SupremacyHD.git
cd SupremacyHD
```

Put your extracted game files in a folder called `SupremacyData` inside the
repository (see [Getting the game data](#getting-the-game-data)), then build
either app:

```sh
sh tools/make_hd_app.sh                    # SupremacyHD.app (hi-res pictures)
```

or

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
sh tools/make_macos_app.sh --with-data SupremacyData build SuperMac.app
```

Drag the app to Applications and open it. The game files are inside the app,
so it needs nothing else. **An app with the game's files inside is for your
own use: never share or publish it.**

## Getting the game data

The game needs the twelve data files of the Amiga version (`GAM1`, `GAM2`,
`INT1`, `INT2`, `Main`, `WIN0` to `WIN3`, `WIN.`, `WINM` and `DIEM`). The
launcher can extract them for you from either of these:

- the **WHDLoad install archive** `Supremacy_v1.1_1384.lha` (or a `.zip` of
  the installed folder);
- the **two original disks** as IPF images (SPS release 1384).

Plain ADF disk images of cracked releases do **not** work: their files are
packed or patched.

If you build an app without the game files inside, the launcher asks for
them the first time: press **EXTRACT FROM WHDLOAD .LHA/.ZIP OR .IPF DISKS**
and pick your archive or disk images, or **BROWSE** to a folder that already
holds the files.

## Playing

Open the app. The **launcher** comes first, with the thinking man from the
intro behind the options (taken from your own game files). Press
**PLAY NEW GAME**, or **LOAD SAVED GAME** to carry on a saved game.

### Launcher settings

| Setting | What it does |
|---|---|
| Intro | On: the original title, music and opponent selection. Off: start straight away against the opponent chosen below it. |
| Game speed | Normal (the original speed, 47 game passes a second) up to twice as fast. |
| Window size | Whole multiples of 320 x 200, as large as your screen allows. |
| Full screen | Start in full screen. |
| Display | Pixel, Smooth or 4:3 (see [Display options](#display-options)). |
| HD pictures | SupremacyHD only: hi-res artwork on or off. |
| Scanlines | CRT-style scanlines. |
| Trainer | The cheat keys (see [Trainer](#trainer)). |
| Bug fixes | Fix the original game's bugs (see [Bug fixes](#bug-fixes)). Off by default. |
| Replay a session log | Play a recorded session again, then carry on playing. |

### Controls

The mouse does everything, as in the original: the **left button** selects,
the **right button** leaves a screen. Ship and planet names are typed on the
keyboard (letters, digits, space and `- = ; , . /`). **Delete** erases,
**Return** accepts and **Esc** cancels. Game time stops while you type a name.

On a MacBook, hold **Fn** for the F keys.

| Key | Action |
|---|---|
| F11 or Option+Return | Window or full screen |
| F12 | Display: Pixel, Smooth, 4:3 |
| F10 | Scanlines on or off |
| F8 | Trainer on or off |
| F5, F6, F7, F9 | Trainer cheats (see [Trainer](#trainer)) |
| Close the window | Quit (save first from the in-game disk menu) |

### Saved games, settings and logs

Everything is kept in your Library, so the app itself is never changed:

- **SuperMac:** `~/Library/Application Support/Supremacy/`
- **SupremacyHD:** `~/Library/Application Support/SupremacyHD/`

That folder holds:

- `supremacy.cfg`, the launcher settings;
- `save/SSG1` to `SSG4`, the in-game disk menu's four save slots (the same
  format as the WHDLoad version's saves);
- `supremacy.log`, a record of the last session: the opponent, the random
  seed and every click and key press. **Replay a session log** plays it back
  exactly, which makes a problem easy to reproduce.

### Command line

The program inside the app can also be started from Terminal:

```
supremacy <data folder> [options]
  --opponent N         0-3: skip the intro and play against this opponent
  --seed S             the game's random seed (skips the intro)
  --no-intro           start the game at once
  --speed P            game passes per second (default 47)
  --scale N            window size: N x 320 by N x 200
  --fullscreen         start in full screen
  --display N          0 pixel (default), 1 smooth, 2 4:3
  --scanlines          CRT-style scanlines
  --trainer            the cheat keys work (F8 switches)
  --fix-bugs           fix the original game's bugs
  --no-hires           ignore hi-res pictures
  --export-pictures D  write the nine original pictures to folder D, then quit
  --log FILE           where to write the session log
  --replay FILE        play a session log again, then carry on
  --ending N           only play an end sequence: 0-3 a victory, -1 the defeat
```

## Display options

| Mode | What you see |
|---|---|
| **Pixel** | Sharp square pixels, scaled by whole numbers: the original look on a modern screen. |
| **Smooth** | Fills the window at any size without uneven pixels, and stays sharp. |
| **4:3** | The Amiga monitor's shape. Its pixels were taller than wide, so the picture is shown as 320 x 240 and the window is sized to match. |

**Scanlines** darken the lower part of each of the 200 picture lines, like a
CRT. In Pixel mode they need a window of 2x or larger; in Smooth and 4:3 they
always show.

F12 cycles the display modes and F10 switches scanlines during the game.

## Supremacy HD (hi-res pictures)

SupremacyHD draws the game's nine full-screen pictures (main screen,
statistics, shipyard, fleet, troops, cargo, surface, combat and title) in high
resolution. Wherever the screen still shows the picture, the hi-res version
is drawn. The text, icons, planet pictures and pointer that the game draws on
top stay exactly as the game draws them.

**Built in:** with nothing else installed, SupremacyHD upscales the pictures
itself from your game files, the first time each screen is shown: pixel-art
edge smoothing (Scale2x, three times), then softened, scaled to 1280 x 800,
sharpened and given a little more colour. The **HD PICTURES** switch in the
launcher turns it off for the original pixels.

**Your own pack (optional):** for an even more detailed look you can make a
**hi-res pack** with an AI upscaler. A pack is a folder called `HiRes` with
`main.png`, `statistics.png`, `shipyard.png`, `fleet.png`, `troops.png`,
`cargo.png`, `surface.png`, `combat.png` and `title.png` (any size; 1280 x 800
or larger in the 8:5 shape works best). A pack replaces the built-in
pictures.

1. Write the original pictures to `HiRes-original/`:

   ```sh
   sh tools/upscale_pictures.sh
   ```

   If `realesrgan-ncnn-vulkan` is installed, the script also upscales them
   into `HiRes/` and you can skip step 2.

2. Install the free [Upscayl](https://upscayl.org) app. Open the
   `HiRes-original` folder for batch upscaling, choose an illustration
   (digital art) model, 4x scale and PNG, and save into a new `HiRes` folder.
   The file names must stay `main.png`, `statistics.png` and so on. If
   Upscayl added a suffix, this removes it:

   ```sh
   cd HiRes && for f in *_upscayl_*.png; do mv "$f" "${f%%_upscayl_*}.png"; done; cd ..
   ```

3. Build the app again, with the pack inside:

   ```sh
   sh tools/make_hd_app.sh
   ```

Instead of rebuilding, you can also put the `HiRes` folder in
`~/Library/Application Support/SupremacyHD/HiRes`. `tools/make_hd_app.sh
--data DIR --hires DIR` takes other folders. Packs are made from the game's
artwork: keep them, and any app containing them, for your own use.

## Trainer

Off by default. Switch it on in the launcher or with **F8** during the game;
the window title says when it is on. Cheats are never applied during a
replay.

| Key | Effect |
|---|---|
| F5 | +100,000 credits on each of your planets |
| F6 | Food, minerals, fuels and energy filled to 30,000 on each of your planets |
| F7 | Population doubled (up to 30,000) and morale set to 100 |
| F9 | Peace on or off: the enemy sends no attacks, and enemy forces already on your planets are cleared when you switch it on. Turning the trainer off also ends peace |

## Bug fixes

Switch on **Bug fixes** in the launcher (or start with `--fix-bugs`) to fix
the original game's bugs. It is off by default, so the game plays exactly as
the original did. Bug fixes are always off while a session log is replayed,
so recorded logs stay exact. Saved games work with either setting. Each fix
is marked in the source (`craft.c`, `combat.c`, `enemy.c`, `events.c` and
`day.c`).

| Where | The bug | With the fix |
|---|---|---|
| Craft: launch, scrap, format | When a craft is in none of a planet's dock slots, slot 3 is cleared anyway, throwing out whatever is docked there | A slot is cleared only if it holds the craft |
| Craft: launch | Launching a nuclear craft such as a solar satellite takes 100 fuel it does not have, so its tank wraps to 65,436 | No fuel is taken from nuclear craft |
| Craft: launch, dock | Slots are used on the planet shown on the fleet screen, not on the craft's own planet | The craft's own planet is used |
| Craft: stop | STOP on an atmosphere processor that is formatting leaves it set to format, so the planet is formatted although the processor never arrives | STOP does nothing for a processor that is formatting |
| Craft: transit | A craft in transit with 0 days left is never processed again, so such a format never finishes | It arrives on that pass |
| Cargo: unload everything | The 30,000 limit is tested on one planet but the cargo goes to another; if that one is full, the loop never ends | The limit is tested on the planet unloaded onto |
| Combat | After a conquest, the +7 training bonus goes to platoons on the planet on show instead of the winning platoons | It goes to the winners |
| Combat | A conquest made as the combat screen opens writes the owner, name and captured craft to the wrong planet | It writes to the conquered planet |
| Enemy attack | The enemy's target search reads one record past the end of the planet table | The search stops at the last planet |
| Events: ships destroyed | The planet's dock slot is not cleared, so it keeps pointing at a destroyed craft | The slot is cleared |
| War tension | If the enemy owns every planet, the display reads past the end of its table and starts a stray sound | The last real entry is used |
| Events: aracno-locusts | The event tests for the year 1014, which never comes, so it never happens | It happens in 2019 (the original's intended year is unknown) |

A few other oddities are left as the original game plays them, because they
look like design choices rather than bugs.

### How ground combat is worked out

Once a day per planet (`ground_planet` in `src/core/combat.c`), for the
platoons stationed there:

- Your strength is the sum over those platoons of
  `troops + (troops x training / 72) x (suit + weapon + aggression + 1)`.
  Training runs 0-100 and only grows while a platoon is not stationed.
- The enemy's strength is its force on the planet x (2 + aggression) / 100
  (at least 1). Its force grows each day (by 4, 3 or 1 depending on the
  planet type, up to 12,437), except on the enemy base.
- If your strength is not greater than the enemy's, your platoons there are
  wiped out and the planet is lost. Otherwise your troops are scaled to
  `(yours - enemy) / yours` of their number, and the enemy force falls by 2%
  of your strength; when it runs out the planet is yours.

## Building

You need macOS 27.x, the Xcode command-line tools (`xcode-select --install`),
[Homebrew](https://brew.sh/), and then `brew install cmake sdl3`.

| Command | Result |
|---|---|
| `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build` | The game program, `build/supremacy` |
| `sh tools/make_macos_app.sh` | `Supremacy.app`, asking for the game data on first start |
| `sh tools/make_macos_app.sh --with-data SupremacyData build SuperMac.app` | `SuperMac.app` with your game files inside |
| `sh tools/make_hd_app.sh` | `SupremacyHD.app` with your game files (and `HiRes` pack, if there is one) inside |
| `sh tools/make_hd_app.sh --no-data` | `Supremacy HD.app` with no game files inside: the launcher asks for them (shareable) |

The app's name follows its file name. Each app carries its own copy of SDL 3
and is ad-hoc signed, which is enough to run it on the Mac that built it. If
macOS refuses to open it the first time, right-click the app, choose
**Open**, then **Open** again.

`make_macos_app.sh` also takes `--with-hires DIR` to put a pack in any app.
SupremacyHD is the same source built with `-DSUP_HD=ON` (into `build-hd`).

## What the port does

The game logic, screens and sound are translated routine by routine from the
original 68000 program and checked against the original running in an
emulator. Everything that touched the Amiga hardware directly has been
replaced:

- **The game logic** (daily simulation, economy, combat, the enemy, events,
  craft and cargo) is portable C. The random-number generator is reproduced
  exactly, so a game started with the same seed plays out the same way as on
  an Amiga.
- **The game state** is kept in the original's 7,000-byte big-endian block,
  so saved games are compatible with the WHDLoad version.
- **The WHDLoad version's behaviour**, including its two fixes and its
  removal of the original's disk checks (the copy protection).
- **Display:** the original 320 x 200, 32-colour screen, including its
  per-line colour changes (the copper), is rebuilt in memory each frame and
  drawn with SDL 3, in the display modes above.
- **Input:** the mouse is mapped to the original's 320 x 200 coordinates, and
  quick clicks between two game passes are never lost.
- **Sound:** the Amiga's four-channel Paula chip is emulated and mixed to
  stereo at 44.1 kHz, driven by the original music and effects players.
- **Timing:** the game runs at the original's measured speed of 47 passes a
  second on any Mac, with faster speeds in the launcher.
- **Files:** a launcher, a built-in extractor for the WHDLoad `.lha`, `.zip`
  and the original IPF disks, saved games as files, and session logs with
  exact replays.
- **No original data in the program:** tables, texts, pictures and sounds
  are read from your copy at start-up.

## Testing

`tools/fuzz` holds tests that build without SDL and run with the address and
undefined-behaviour sanitizers:

| Program | What it checks |
|---|---|
| `fuzz_game` | Plays whole games with random input and reports freezes and memory errors. |
| `test_evwait` | The one in-game event that waits for a YES/NO answer, from every screen it can start on. |
| `test_shows` | The intro, the five end sequences and their music. |
| `fuzz_extract` | The extractor, fed thousands of damaged archives and disk images. |

```sh
FUZZ_CC=clang sh tools/fuzz/build_fuzz.sh
mkdir -p build-fuzz/logs
./build-fuzz/fuzz_game SupremacyData build-fuzz/logs 10 100000 1
```

Each program describes its arguments at the top of its source file.

## Project layout

| Path | Contents |
|---|---|
| `src/core` | The game logic: state, daily simulation, economy, craft, combat, enemy, events, and the trainer cheats. No platform code. |
| `src/ui` | The screens, text and graphics routines, the intro and the end sequences. |
| `src/sound` | The music and effects players and the Paula emulation. |
| `src/assets` | Loading and unpacking the original data files. |
| `src/platform` | The SDL 3 front end, the launcher, the extractor and the hi-res pictures. |
| `assets/icons` | The app icon. |
| `tools` | The app build scripts, the picture upscaling script and the tests. |

Comments in the code give the original's 68000 addresses (for example
`$4748`) for the routines and data they correspond to.

## Licence

The source code in this repository is released under the
[MIT Licence](LICENSE).

Supremacy itself (its code, graphics, sound, music and text) remains the
property of its copyright holders and is **not** covered by this licence.
This project is not affiliated with or endorsed by them. The icon in
`assets/icons` is converted from the game icon of the WHDLoad install and is
likewise not covered by the MIT Licence. Apps built with the game's files or
hi-res pictures inside contain that copyrighted material: keep them for your
own use.

## Acknowledgements

- *Supremacy: Your Will Be Done* © 1990 Virgin Mastertronic, published by
  Melbourne House. System software by David Perry, graphics by Nick Bruty,
  design and concept by Probe Software.
- This macOS port is built on the C translation of the game by
  [derfsss](https://github.com/derfsss/Supremacy) (MIT Licence).
- The Supremacy WHDLoad install (V1.1) by Dark Angel, whose behaviour the
  game follows, and [WHDLoad](https://www.whdload.de/) by Bert Jahn.
- The [Software Preservation Society](http://www.softpres.org/), whose IPF
  disk images preserve the original disks.
- [SDL 3](https://www.libsdl.org/) (zlib licence), for graphics, sound and
  input.
- The LZSS method used to unpack the game files is Haruhiko Okumura's; the
  `-lh5-` method of LHA is Haruyasu Yoshizaki's. The built-in picture
  upscaler uses Scale2x by Andrea Mazzoleni.
- [Upscayl](https://upscayl.org) and
  [Real-ESRGAN](https://github.com/xinntao/Real-ESRGAN), suggested for making
  hi-res packs.
- [Claude Code](https://www.anthropic.com/claude-code) (Anthropic), the AI
  assistant used throughout the development of this port.
