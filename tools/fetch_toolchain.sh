#!/bin/bash
# Fetches what the cross builds need into build/, which is disposable.
#
#   zig      supplies libc for every target at a chosen glibc version, so no
#            archived distro has to be assembled to hold the floor down.
#   libSDL2  only its symbols are needed at link time; the device loads its own,
#            or the port carries one (OnionOS). Debian's 2.26.5 rather than
#            Ubuntu's 2.0.20, which predates calls the game makes.
set -eu

ROOT="$(realpath "$(dirname "$0")/..")"
OUT="$ROOT/build"
CACHE="$OUT/download-cache"

ZIG_VERSION=0.15.2
SDL_VERSION="2.26.5+dfsg-1"
SDL_POOL=http://deb.debian.org/debian/pool/main/libs/libsdl2

mkdir -p "$CACHE" "$OUT"

if [ ! -x "$OUT/zig/zig" ]; then
  tarball="$CACHE/zig-x86_64-linux-$ZIG_VERSION.tar.xz"
  [ -f "$tarball" ] ||
    curl -fL --retry 3 -o "$tarball" \
      "https://ziglang.org/download/$ZIG_VERSION/zig-x86_64-linux-$ZIG_VERSION.tar.xz"
  rm -rf "$OUT/zig"
  mkdir -p "$OUT/zig"
  tar -xJf "$tarball" -C "$OUT/zig" --strip-components=1
fi

for deb_arch in arm64 armhf amd64; do
  deb="$CACHE/libsdl2-2.0-0_${SDL_VERSION}_${deb_arch}.deb"
  [ -f "$deb" ] ||
    curl -fL --retry 3 -o "$deb" "$SDL_POOL/$(basename "$deb")"
  ( cd "$CACHE" && ar p "$deb" data.tar.xz | tar -xJ -C "$OUT/sdl2" ) 2>/dev/null ||
    { mkdir -p "$OUT/sdl2" && ( cd "$CACHE" && ar p "$deb" data.tar.xz | tar -xJ -C "$OUT/sdl2" ); }
done

# -lSDL2 wants the development symlink, which lives in libsdl2-dev; the runtime
# package is enough for linking once it exists.
for libdir in "$OUT"/sdl2/usr/lib/*/; do
  so="$(ls "$libdir" | grep -m1 '^libSDL2-2\.0\.so\.0\.' || true)"
  [ -n "$so" ] && ln -sf "$so" "$libdir/libSDL2.so"
done

echo "toolchain ready: $("$OUT/zig/zig" version), SDL2 for $(ls "$OUT/sdl2/usr/lib" | tr '\n' ' ')"
