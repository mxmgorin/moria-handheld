# Mines of Moria for handhelds

Robert Alan Koeneke's 1983 Moria, via Umoria and [Rufe.org's](https://github.com/RufeDotOrg/moria_at) remake. Descend the Dwarvish mines level by level and defeat the Balrog at the bottom. Death is permanent, the dungeon is different every time, and time only advances when you take a turn.

This is a handheld-focused port for PortMaster devices and the Miyoo Mini under OnionOS, including packaging for both platforms. It sits on upstream `da8a3e5`.

## Install

### PortMaster

Unzip `minesofmoria-portmaster.zip` into the ports directory.

`portmaster/README.md` has the full button layout; `L2 + Y` shows it in game.

### OnionOS (Miyoo Mini / Plus / Flip)

Two packages, both unzipped at the root of the SD card. Either is enough.

`minesofmoria-onionos-app.zip` lands in `App/MinesOfMoria/` and appears under
Apps. `minesofmoria-onionos-port.zip` lands in `Roms/PORTS/` and appears under
Ports, next to the rest of the Ports Collection — which has to be installed. If the entry does not show up, `~Refresh roms` in
the Ports list rereads the tree.

## Build

```shell
tools/fetch_toolchain.sh                       # zig, and the SDL2 link libraries
for a in aarch64 armhf x86_64; do tools/build.sh "$a"; done
python3 tools/package.py                       # -> build/minesofmoria-portmaster.zip
```

`tools/build.sh` refuses to finish if a binary needs a glibc above 2.28, which is
the failure that otherwise only shows up on the device. CI runs these same
scripts, so a release cannot come out different from a local build.

The x86_64 binary is the one to test changes against; `MORIA_WINDOW` gives it a
handheld's geometry on a desktop, which no fullscreen window will reproduce:

```shell
MORIA_WINDOW=640x480 build/minesofmoria.x86_64
```

## Upstream

Terminal client, wizard mode, the cosmocc build and the mobile targets are
upstream's and are documented in [its
README](https://github.com/RufeDotOrg/moria_at#readme).

## What changed

| Area | Change |
| --- | --- |
| Window | Fullscreen-desktop rather than an exclusive 1920x1080 mode no panel offers. A driver that reports no display size gets a plain 640x480 window, and the layout texture is fitted to what the renderer can actually hold. |
| Screen | The canvas follows the panel instead of a fixed 16:9 and is scaled by whole panels; the touch pad and side panels are gone and the map fills the screen. Health sits in the status row, since this layout has no stat block. `n) minimum map rows` trades screen fill against how much dungeon stays visible. |
| Pad | Read from a hat, from dpad buttons or from keys, not only from the analog axes, so a device without a stick can be played at all. A held direction repeats, a held L2 opens a second command layer, and the pad can no longer be switched off when it is the only way back into the menu. |
| Miyoo | Its panel takes a frame only as a copy of a streaming texture, so the game composes into a surface and the window carries one copy per frame. Its pad arrives as key presses rather than as a joystick, and those keys feed the same bindings a PortMaster device drives. |
| Fixes | Prompts are centred on their own length rather than the buffer's, and the 1-bit sprite sheet is expanded by hand because SDL2 reads an INDEX1LSB surface most significant bit first whatever the format says. |

## Credits

Robert Alan Koeneke wrote Moria in 1983, Umoria carried it to C, and
[Rufe.org LLC](https://github.com/RufeDotOrg/moria_at) rebuilt it as `moria_at`
— which is the game these packages compile.
