# Bundled libraries

The Miyoo Mini's panel hangs off the SigmaStar display pipeline, and no upstream
SDL2 can put pixels on it; the port needs a build carrying the `Mini` video
driver. So every SDL2 app here ships its own copy and preloads it, and these
files are ours.

## libSDL2-2.0.so.0

Built from [steward-fu/sdl2](https://github.com/steward-fu/sdl2) at commit
`0631abc8e8916db6f9bc7e2afd0c22913d092a29`, with `build/stewardfu/build.sh` in
the fork -- the upstream Docker recipe, `mini_toolchain-v1.0`, `make cfg && make
gpu && make sdl2`. It reports itself as `libSDL2-2.0.so.0.18.2`, so SDL 2.0.18.

Previous packages carried a prebuilt copy taken from the `Sonic Mania` port of
the OnionOS Ports-Collection, built from
[XK9274/sdl2_miyoo](https://github.com/XK9274/sdl2_miyoo)'s `snow-s-mania`
branch. It was replaced because that branch advertises its texture formats as

    .num_texture_formats = 2,
    .texture_formats = { [0] = SDL_PIXELFORMAT_RGB565, [2] = SDL_PIXELFORMAT_ARGB8888 },

where index `[2]` should be `[1]`. With two formats declared, SDL reads slots 0
and 1, finds RGB565 and a zero, and can offer no 32-bit format at all: every
texture this game creates is quietly converted to RGB565 on the CPU, once per
frame. The same table in this build reads `[0] = RGB565, [1] = ARGB8888`, which
is what the driver actually supports. That branch also makes `SELECT + R1` a
screen-scaling hotkey and eats the `R1` press, which collides with the map key.

Note this is an *altered* version of SDL, which its licence requires be said
plainly: the `Mini` video, render and audio drivers are third-party additions
and not part of upstream SDL. Nothing else is patched -- the library is stock
steward-fu at the commit above.

One SDL2 behaviour the game works around rather than patches: an
`SDL_PIXELFORMAT_INDEX1LSB` surface is converted most significant bit first,
mirroring every group of eight pixels. Every SDL2 release does this, SDL3 does
not, so it only appears on a device. The sprite sheet is expanded to 32-bit by
the game instead of being handed to SDL as a 1-bit surface.

## libEGL.so

The same 7 KB placeholder the previous bundle carried, from the Ports-Collection
build. `libSDL2` names `libEGL` in `DT_NEEDED` and references a handful of `egl*`
functions, but the library is configured with `--disable-video-opengl`,
`--disable-video-opengles` and `--disable-video-opengles2` and never calls them;
binding is lazy, so a placeholder that exports nothing is enough. Its licence is
not stated at the source, and it exists only to satisfy the loader. Swap it for
the device's own EGL, or for `swiftshader/build/libEGL.so` from the build above,
if that provenance is not good enough for you.

## libjson-c.so.5

json-c, from the same Ports-Collection bundle. `libSDL2`'s audio driver reads
`/appconfigs/system.json` for the system volume; this game plays no audio, but
the symbols are resolved regardless.

## Not shipped

`libSDL2_image` and `libSDL2_ttf` were in the previous bundle only because that
build named them in `DT_NEEDED`. This one does not, so they are gone, and with
them `libpng16`, `libz`, `libfreetype` and `libbz2`. Everything still needed --
`libGLESv2` and the `libmi_*` SoC libraries -- is already on the device.

    adfaba3ed88c5e5acd521384f047bc369f52578f01d0b6b39fc7591e94e93944  libEGL.so
    d0c8f1b8cffe367c283375a9475f974c551024c61f9b189f81da84ce7752ccff  libSDL2-2.0.so.0
    db7ad1f59cbac5a23aad9dd7eba87322b5921c486097c1140b24f2d82af28892  libjson-c.so.5

## Licences

`libSDL2-2.0.so.0` is Simple DirectMedia Layer, (C) 1997-2024 Sam Lantinga,
under the zlib licence -- the full text is in the port's
`licenses/LICENSE.SDL2.txt`. The corresponding source is the steward-fu commit
named above.

`libjson-c.so.5` is json-c, (C) 2009-2012 Eric Haszlakiewicz and (C) 2004-2005
Metaparadigm Pte Ltd, under the MIT licence; see `licenses/LICENSE.json-c.txt`.
