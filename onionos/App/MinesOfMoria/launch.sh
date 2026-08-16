#!/bin/sh
# OnionOS (Miyoo Mini Plus / Flip) launcher.
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
gamedir=$(cd "$(dirname "$0")" && pwd)

# Our SDL2 first, preloaded as every SDL2 port here does; the device carries
# what it links against (libGLESv2 and the libmi_* SoC libraries).
export LD_LIBRARY_PATH="$gamedir/lib:$sysdir/lib/parasyte:$sysdir/lib:$miyoodir/lib:/lib:/config/lib:/customer/lib"
export LD_PRELOAD="$gamedir/lib/libSDL2-2.0.so.0"
export SDL_VIDEODRIVER=Mini
export EGL_VIDEODRIVER=Mini
# This panel takes a frame only as a copy of a streaming texture; a render
# target, which is how the game composes, is dropped without an error, and the
# software renderer never reaches it at all. So the panel driver carries the
# window and MORIA_BLIT puts a software renderer behind it -- retsend's
# CanvasBlit, inside the game.
export SDL_RENDER_DRIVER="Miyoo Mini"
export MORIA_BLIT=1
# The pad arrives as key presses here, not as a joystick; MORIA_KEYPAD feeds
# them to the same bindings a PortMaster device drives.
export MORIA_KEYPAD=1

# The stock HOME is a read-only rootfs, and saves are written to the working
# directory, so both stay next to the app on the card.
export HOME="$gamedir"
mkdir -p "$gamedir/saves"
cd "$gamedir/saves" || exit 1

# Nothing in the game raises an SDL quit event here, so MENU is the way out.
"$sysdir/bin/pressMenu2Kill" minesofmoria &
"$gamedir/minesofmoria" > "$gamedir/log.txt" 2>&1
pkill -9 pressMenu2Kill
