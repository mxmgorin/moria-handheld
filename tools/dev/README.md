# Input test harness

Not shipped. Verifies the dpad paths without a handheld.

```
gcc -o vpad tools/dev/vpad.c
gcc -o jsdump tools/dev/jsdump.c $(sdl2-config --cflags --libs)
```

`vpad` creates a uinput gamepad with a hat, a stick and the face, shoulder and
`BTN_DPAD_*` keys, then takes commands on stdin (`h <x> <y>`, `b <code> <0|1>`,
`q`). `/dev/uinput` is `root:input`, so membership in `input` is enough.
`jsdump` prints the GUID, the counts and a live event dump — that is how you
learn whether a device reports its dpad as a hat, as buttons or as axes.

**Use a genuine SDL2, not the system one.** Where `libSDL2` is `sdl2-compat`
(SDL2 API over SDL3), joystick events never reach a windowed app and the pad
looks dead. Point at a real one instead:

```
curl -O http://deb.debian.org/debian/pool/main/libs/libsdl2/libsdl2-2.0-0_2.26.5+dfsg-1_amd64.deb
ar p libsdl2-*.deb data.tar.xz | tar -xJ
export LD_LIBRARY_PATH="$PWD/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"
```

Under Xvfb the window captures as all black regardless of renderer, so verify
game state rather than pixels: build with a `SDL_Log` of `uD.y`/`uD.x` next to
the movement branch in `src/moria_at.c` and watch the position move. Mind that
SDL drops joystick events for an unfocused window unless
`SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS=1` is set.
