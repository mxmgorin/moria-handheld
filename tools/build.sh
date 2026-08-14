#!/bin/bash
# Builds the moria_at SDL2 client for PortMaster.
#
#   tools/build.sh aarch64   every handheld PortMaster tracks
#   tools/build.sh armhf     32-bit userlands, OnionOS among them
#   tools/build.sh x86_64    desktop CFWs and local testing
#
# zig cc holds the glibc floor down by itself, so there is no sysroot to keep in
# step with an archived distro. src/ is a checkout of the portmaster branch of
# github.com/mxmgorin/moria_at; every fix lives there as a commit and this only
# compiles. SDL2 headers come from the tree (third_party/SDL2/SDL.h, a flattened
# 2.28.5 header), so no SDL2 dev package is required either.
set -eu

ARCH="${1:-aarch64}"
ROOT="$(realpath "$(dirname "$0")/..")"
REPO="$ROOT"   # the fork is the game; sources live in $REPO/src
OUT="$ROOT/build"

# The oldest userland targeted: OnionOS is glibc 2.28, ArkOS 2.30, everything
# else newer. Above this the loader refuses the binary on the device.
GLIBC_FLOOR=2.28

. "$ROOT/tools/upstream.env"
COMMIT="$(git -C "$REPO" rev-parse HEAD)"

case "$ARCH" in
  aarch64) TRIPLE=aarch64-linux-gnu   ; LIBDIR=aarch64-linux-gnu   ;;
  armhf)   TRIPLE=arm-linux-gnueabihf ; LIBDIR=arm-linux-gnueabihf ;;
  x86_64)  TRIPLE=x86_64-linux-gnu    ; LIBDIR=x86_64-linux-gnu    ;;
  *) echo "unknown arch: $ARCH" >&2; exit 1 ;;
esac

[ -x "$OUT/zig/zig" ] || "$ROOT/tools/fetch_toolchain.sh"

# platform is a gitignored symlink upstream expects the build to place.
rm -f "$REPO/src/platform"
ln -s "$REPO/platform/sdl2" "$REPO/src/platform"

mkdir -p "$OUT"
BIN="$OUT/minesofmoria.$ARCH"

# K&R definitions and implicit int: the source predates C99 style on purpose.
"$OUT/zig/zig" cc \
  -target "$TRIPLE.$GLIBC_FLOOR" \
  -std=gnu17 -O2 -DRELEASE -DNDEBUG \
  -fno-strict-aliasing -fno-math-errno \
  -Wno-incompatible-function-pointer-types -Wno-implicit-function-declaration \
  -Wno-implicit-int -Wno-return-type -Wno-int-conversion \
  -Wno-deprecated-non-prototype \
  -I"$REPO" -I"$REPO/third_party/SDL2" \
  -o "$BIN" "$REPO/src/moria_at.c" \
  -L"$OUT/sdl2/usr/lib/$LIBDIR" -lSDL2 -lm \
  -Wl,-s \
  -Wl,--allow-shlib-undefined   # SDL2 pulls X11/wayland/alsa we never call

# Fixed-length placeholders patched in the binary, as upstream's README
# documents. versionD feeds the replay hash, so VERSION is pinned in
# upstream.env and must stay stable; the commit stamp is free to move.
python3 - "$BIN" "$VERSION" "$COMMIT" <<'EOF'
import sys
path, version, commit = sys.argv[1:4]
blob = open(path, 'rb').read()
for placeholder, value in ((b'XXXX.YYYY.ZZZZ', version), (b'AbCdEfGhIjKlMnO', commit)):
    value = value.encode()[:len(placeholder)].ljust(len(placeholder), b'0')
    assert blob.count(placeholder) == 1, f'{placeholder!r} x{blob.count(placeholder)}'
    blob = blob.replace(placeholder, value)
open(path, 'wb').write(blob)
EOF

# Fail here rather than on the device, which is how this class of port breaks.
over="$(readelf -V "$BIN" | grep -o 'GLIBC_2\.[0-9]*' | sort -uV |
  awk -F. -v floor="${GLIBC_FLOOR#2.}" '$2 > floor' | tr '\n' ' ')"
if [ -n "$over" ]; then
  echo "$BIN needs $over; the floor is GLIBC_$GLIBC_FLOOR" >&2
  exit 1
fi

cp "$BIN" "$ROOT/portmaster/minesofmoria/"

echo "built $BIN ($VERSION, ${COMMIT:0:12}, glibc <= $GLIBC_FLOOR)"
