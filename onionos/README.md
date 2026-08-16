# Mines of Moria for OnionOS

Koeneke's 1983 roguelike, by way of Umoria and [Rufe.org's rebuild of
it](https://github.com/RufeDotOrg/moria_at). Descend the mines level by level and
kill the Balrog at the bottom; die and the character is gone.

## Install

Two packages, both unzipped into the root of the SD card. They differ only in
where the game appears; either one is enough.

`minesofmoria-onionos-app.zip` lands in `App/MinesOfMoria/` and shows up under
Apps.
Saves go to `App/MinesOfMoria/saves`, the last run's log to
`App/MinesOfMoria/log.txt`.

`minesofmoria-onionos-port.zip` lands in `Roms/PORTS/Games/MinesOfMoria/` with
its shortcut in `Roms/PORTS/Shortcuts/`, and shows up under Ports. Onion's own
`launch_standalone.sh` runs it from the game directory, so saves sit there rather
than in a `saves/` subdirectory, beside `log.txt`.

Installed together they are two separate copies, each with its own SDL2 and its
own saves; nothing is shared between them.

## Credits

The game is GPL-3; the source for this binary is the `handheld` branch of
<https://github.com/mxmgorin/moria_at>, and the build stamps its own commit into
the executable. `licenses/` carries the texts, and `lib/README.md` says where the
bundled SDL2 came from and under what terms.
