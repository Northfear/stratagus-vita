# Stratagus port for PS Vita

## Install

Wargus/War1gus requires extracted Warcraft 1/2 data in order to play the game.

So basically you'll have to install Wargus/War1gus on your PC, extract game data from original discs or GoG version and once you have a working PC version, you'll have to copy extracted game data to your Vita.

### Steps:

1. Download and install stratagus.vpk. Extract content of stratagus_data.zip into `ux0:data/`.

    https://github.com/Northfear/stratagus-vita/releases

2. Download Wargus/War1gus and install it to your PC (Only use the versions linked below, since version mismatch may cause problems)

    https://github.com/Wargus/wargus/releases/tag/v3.1.2

    https://github.com/Wargus/war1gus/releases/tag/v3.1.2

(You may need to disable antivirus during Wargus installation, since it may break ecoding process in some cases)

3. On the first launch you will be asked for a copy of the original Warcraft 1/2. Select the path to the game or GoG installer and wait until installer extracts/encodes the data.

4. Once the data is extracted and you have a working PC installation of Wargus/War1gus, it's time to copy the game data to Vita.

If you were using portable version of Wargus/War1gus, then the extracted data should be in the installation folder. Otherwise the game data should be located at `Documents\Stratagus\data.Wargus` or `Documents\Stratagus\data.War1gus` (for Windows versions).

Copy `campaigns`, `graphics`, `maps`, `music`, `sounds` and `videos` from Wargus/War1gus data folders and paste them into `ux0:data/Wargus/` for Wargus or into `ux0:data/War1gus/` for War1gus.

Copy `scripts/wc2-config.lua` for Wargus or `scripts/wc1-config.lua` for War1gus and paste the file into `ux0:data/Wargus/scripts/`/`ux0:data/War1gus/scripts/`.

5. Start the game.

## Building

### Prerequisites
- VitaSDK
- SDL2
- SDL2-image
- SDL2-mixer
- Lua 5.1.5
- Tolua++
- Theora (optional)
- OpenMP (optional)

### Build
```
mkdir build && cd build
export SDLDIR=$VITASDK/arm-vita-eabi/
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake -DVITA=true -DENABLE_STATIC=ON -DENABLE_USEGAMEDIR=ON -DEAGER_LOAD=ON -DCMAKE_BUILD_TYPE=None
make
```

## Port info

### Controls

- Left analog stick - Cursor movement
- Right analog stick - Map scrolling
- × - Left mouse button
- ○ - Right mouse button
- □ - Attack
- △ - Stop
- D-Pad Up/Right/Down/Left - 1/2/3/4 button
- L1 - Ctrl
- R1 - Shift (also used for cursor movement speedup)
- SELECT - F10
- START - Esc

Use L1 + D-Pad to create teams (1-4) and D-Pad to select them (same as Ctrl + 1-4 on keyboard).

You can change cursor movement speed by editing `ux0:data/Wargus/wc2/preferences.lua`/`ux0:data/War1gus/wc1/preferences.lua` and changing `ControllerSpeed` option (you may need to change some game options first for this file to appear).

### Additional info

Multiplayer is not supported on PS Vita.

Game startup/loading times are around 1-2 minutes for Wargus.

Auto-cast with mages can cause pretty big slowdowns in War1gus (probably in Wargus too). No performance problems with auto healing and clerics tho.

You can turn on bilinear filtering by editing `ux0:data/Wargus/wc2/preferences.lua`/`ux0:data/War1gus/wc1/preferences.lua` and changing `BilinearFilter` option to `true`.

    _______________________________________________________________________
         _________ __                 __                               
        /   _____//  |_____________ _/  |______     ____  __ __  ______
        \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
        /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ | 
       /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
               \/                  \/          \//_____/            \/ 
    ______________________                           ______________________
                          T H E   W A R   B E G I N S
           Stratagus - A free fantasy real time strategy game engine

[![Join the chat at https://gitter.im/Wargus](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/Wargus?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

[![Discord](https://img.shields.io/discord/780082494447288340?style=flat-square&logo=discord&label=discord)](https://discord.gg/dQGxaw3QfB)

Windows: <a href="https://ci.appveyor.com/project/timfel/stratagus"><img width="100" src="https://ci.appveyor.com/api/projects/status/github/Wargus/stratagus?branch=master&svg=true"></a>

Linux & OSX: [![Build Status](https://travis-ci.org/Wargus/stratagus.svg?branch=master)](https://travis-ci.org/Wargus/stratagus)

For Mac OS X, Stratagus comes bundled in the app bundles for Wargus, Stargus, and War1gus:
  - Wargus: https://github.com/Wargus/stratagus/wiki/osx/Wargus.app.tar.gz
  - War1gus: https://github.com/Wargus/stratagus/wiki/osx/War1gus.app.tar.gz
  - Stargus: https://github.com/Wargus/stratagus/wiki/osx/Stargus.app.tar.gz

On Ubuntu/Debian, you probably want the deb packages:
  - https://launchpad.net/~stratagus/+archive/ubuntu/ppa
  - Note that the game packages you probably want are `wargus`, `war1gus`, and `stargus`

For Windows, you likely want the game installers:
  - Wargus: https://github.com/Wargus/wargus/releases
  - War1gus: https://github.com/Wargus/war1gus/releases
  - Stargus: https://github.com/Wargus/stargus/releases

If you do indeed need the Stratagus engine itself, nightly builds are available:
  - Windows Installer: https://github.com/Wargus/stratagus/releases/tag/master-builds
  - Ubuntu/Debian Packages: https://launchpad.net/~stratagus/+archive/ubuntu/ppa
  
### Contributing

If you want to contribute, there is a video that details how to set up a development environment on Windows using VSCode here: https://youtu.be/c1Zm7tt_QtQ 

Read 'doc/index.html' for general information and license information.
Read 'doc/install.html' for Stratagus installation instructions.
Read 'doc/changelog.html' for the Stratagus changelog.

The Mac and Windows builds are done on Appveyor and Travis. The Ubuntu packages
are built on Launchpad:
  - https://code.launchpad.net/~stratagus/+recipe/stratagus-github
  - https://code.launchpad.net/~stratagus/+recipe/war1gus-github
  - https://code.launchpad.net/~stratagus/+recipe/stargus-github
  - https://code.launchpad.net/~stratagus/+recipe/wargus-github
