# Crash Tester

A small test application I made for my [Aroma Crash Handler Module](https://github.com/JerrySM64/aroma-crash-handler-module). It lets you trigger various kinds of crashes (DSI, ISI, Program, Alignment, broken stack) on purpose to test how the handler handles them.

## Controls

- **A**: null write (DSI)
- **X**: jump to 0 (ISI)
- **Y**: trap (Program)
- **B**: misaligned stfdu (Alignment)
- **R**: illegal instruction (Program)
- **L**: crash with messed up stack on r1
- **HOME**: quit without crashing

## Building

### With Nix (recommended)

The flake pins the devkitPPC toolchain and [wut](https://github.com/devkitPro/wut) via [devkitNix](https://github.com/bandithedoge/devkitNix) by digest - nothing needs to be installed manually.

```sh
nix build .#  # -> result/cafecrashtest.wuhb
```

For a dev shell with the toolchain (plain `make` just works inside it):

```sh
nix develop
```

### Without Nix

Requires devkitPPC and wut from [devkitPro](https://devkitpro.org/). On a system with devkitPro's [pacman setup](https://devkitpro.org/wiki/Getting_Started):

```sh
sudo (dkp-)pacman -Syu --needed wiiu-dev
export DEVKITPRO=/opt/devkitpro
export DEVKITPPC=$DEVKITPRO/devkitPPC
make
```

## Installing

Copy the built `.wuhb` to the SD card:

```
sd:/wiiu/apps/cafecrashtest/cafecrashtest.wuhb
```

It shows up in the Wii U Menu with [homebrew on menu](https://github.com/wiiu-env/homebrew_on_menu_plugin) installed. Make sure [AromaCrashHandlerModule](https://github.com/JerrySM64/aroma-crash-handler-module) is installed first - that's the whole point of this app.

## License
This application is licensed under the [MIT License](LICENSE).
