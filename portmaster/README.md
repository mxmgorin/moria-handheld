## Notes

Mines of Moria by [Rufe.org](https://github.com/RufeDotOrg/moria_at), a rebuild of Umoria
(GPLv3), which is Robert Alan Koeneke's 1983 Moria. Built from the `portmaster`
branch of [this fork](https://github.com/mxmgorin/moria_at/tree/portmaster), four
commits on top of upstream `da8a3e5`:

- the window is created as fullscreen-desktop rather than requesting an
  exclusive 1920x1080 mode, which no handheld panel offers;
- the dpad is read from a hat or from dpad buttons, not just the analog axes,
  so devices without a stick can be played at all;
- the layout canvas follows the panel instead of being fixed at 16:9, and the
  touch pad, face buttons, stat block and minimap are dropped, so the map fills
  the screen;
- holding L2 opens a second command layer, for the commands ten buttons have no
  room for, and the buttons are laid out the way a handheld expects — A acts, B
  backs out, L1 undoes.

## Controls

### In the dungeon

| Button | Action |
|--|--|
|dpad|walk one step|
|L2 + dpad|run in that direction|
|A + dpad|walk one step|
|A|act on the square you stand on|
|B + dpad|run in that direction|
|B|rest until healed, or until something is seen or heard|
|X|use an item — opens the inventory list|
|Y|repeat the last action|
|L1|character sheet, while held|
|R1|dungeon map, while held|
|R2|look along a direction — press, then a dpad direction|
|SELECT|message history|
|START|game menu — help, options, save and quit|

A on its own reads what is under you: stairs are taken, an item is picked up, a
shop door is entered, otherwise you search the square. Diagonals are two dpad
directions at once.

### Holding L2

| Button | Action |
|--|--|
|A|use equipment|
|X|drop an item|
|Y|help — the whole layout, in game|
|L1|magnification|
|R1|locate yourself on the level map|
|SELECT|undo a turn|

### In lists and menus

| Button | Action |
|--|--|
|dpad up/down|move the selection|
|dpad left/right|switch column, where a list has one|
|A|choose the selected entry|
|B, X|back out|
|Y|choose it the other way — inspect rather than use|
|Y + dpad up/down|jump half a page|
|Y + dpad left/right|jump to the start or the end|

Renderer, magnification, colours, hand-swap and joystick can all be changed from
the game menu under `e) Extra features`.

The map is drawn to the shape of your screen, which on a wide panel means fewer
dungeon rows. `n) minimum map rows` sets a floor it will not crop below — raise
it and the map keeps its rows and gains bars down the sides instead.

## License

GPL-3.0-or-later, as upstream. `minesofmoria/licenses/` holds the texts and a
README saying what each one covers; SDL2 is not bundled, the device supplies it.
The corresponding source is the `handheld` branch linked above, and every build
stamps its own commit into the executable, shown under `v) version detail`.

## Compile

```shell
git clone -b handheld https://github.com/mxmgorin/moria_at && cd moria_at
ln -sf "$PWD/platform/sdl2" src/platform
zig cc -target aarch64-linux-gnu.2.28 \
  -std=gnu17 -O2 -DRELEASE -DNDEBUG \
  -Wno-incompatible-function-pointer-types -Wno-implicit-function-declaration \
  -Wno-implicit-int -Wno-return-type -Wno-int-conversion \
  -I. -Ithird_party/SDL2 \
  -o minesofmoria.aarch64 src/moria_at.c -lSDL2 -lm
```
