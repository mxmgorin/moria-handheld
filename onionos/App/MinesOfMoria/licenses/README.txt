Mines of Moria for OnionOS - what is in the binary and under what terms.

This package ships the armhf executable and, unlike the PortMaster one, a copy
of SDL2: the Miyoo panel needs the port carrying its own display driver and no
stock SDL2 can drive it. lib/README.md records where those binaries came from
and under what terms.

  moria_at game code          GPL-3.0-or-later    LICENSE.moria_at.txt
    Rufe.org LLC's rebuild of Umoria, itself derived from Robert Alan
    Koeneke's Moria. The repository LICENSE covers the work as a whole.

  moria_at SDL2 platform      ISC                 LICENSE.moria_at.txt applies
    Every file under platform/sdl2 carries the header
      "// Rufe.org LLC 2022-2025: ISC License"
    a more permissive grant by the same copyright holder, given inside a
    repository whose LICENSE is GPL-3. No separate ISC text is shipped
    upstream, so the GPL-3 terms above are the ones this port distributes
    under.

  puff.c                      zlib                LICENSE.puff.txt
    Mark Adler's inflate reference, compiled in via platform/sdl2/puff_stream.c.

  SDL2, headers and lib       zlib                LICENSE.SDL2.txt
    third_party/SDL2/SDL.h, a flattened SDL 2.28.5 header by Sam Lantinga, and
    the library itself under lib/, built from steward-fu's Miyoo Mini port --
    an altered version, as lib/README.md says.

  json-c                      MIT                 LICENSE.json-c.txt
    Pulled in by that SDL2 build; this game never calls it.

  Font and sprite assets      GPL-3.0-or-later    LICENSE.moria_at.txt
    platform/sdl2/asset/font_zlib.c and sprite.c hold deflate-compressed raw
    rasters -- 64 KiB of glyph bitmap and 88 KiB of sprite sheet. Neither
    carries embedded attribution of its own; both are committed to the
    upstream repository by its copyright holder and are covered by its
    LICENSE.

  App icon                    no separate grant   icon.png
    Rufe.org LLC's own store icon for this same game, from its Google Play
    listing (org.rufe.moria), scaled down. It is not in the source
    repository -- platform/sdl2/asset/icon.c is a tighter crop of the same
    drawing at lower resolution -- so nothing states terms for it beyond the
    author's own; it marks their game on a menu and is theirs to withdraw.

SOURCE
The GPL requires the source that corresponds to this binary. It is at

  https://github.com/mxmgorin/moria_at   branch: handheld

which is a fork of https://github.com/RufeDotOrg/moria_at with the handheld
changes on top. Each build stamps its own commit into the executable, and the
game shows it under "v) version detail" in the feature menu, so a binary can
always be matched to the source it came from. The port README carries the
build command.
