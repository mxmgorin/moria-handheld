#!/usr/bin/env python3
"""Zips the Ports variant of the OnionOS app into build/minesofmoria-onionos-port.zip.

Same binary and libraries as tools/package_onionos.py, laid out the way the
Ports Collection does it instead: the payload under Roms/PORTS/Games, a .port
shortcut Emu/PORTS/launch.sh executes, and box art named after it. The archive
mirrors the SD card, so it unzips at the card root.

The libraries and licenses are read out of the App tree rather than copied
beside the shortcut, so the 5.7 MB of SDL2 is in the repo once.
"""
import pathlib
import zipfile

ROOT = pathlib.Path(__file__).resolve().parent.parent
CARD = ROOT / "onionos"
APP = CARD / "App/MinesOfMoria"
GAMEDIR = "Roms/PORTS/Games/MinesOfMoria"
BINARY = ROOT / "build" / "minesofmoria.armhf"
COVER = ROOT / "portmaster" / "cover.png"
OUT = ROOT / "build" / "minesofmoria-onionos-port.zip"

# The shortcut is executed, not sourced, and Onion chmods it anyway; the binary
# has to arrive executable because nothing else sets it.
EXECUTABLE = {".port"}


def entries():
    """(archive name, mode, bytes), in card order."""
    for path in sorted(p for p in (CARD / "Roms").rglob("*") if p.is_file()):
        name = str(path.relative_to(CARD))
        mode = 0o755 if path.suffix in EXECUTABLE else 0o644
        yield name, mode, path.read_bytes()

    # Art is found by the shortcut's own name, so the two cannot drift apart.
    shortcut = next(p for p in (CARD / "Roms").rglob("*.port"))
    yield f"Roms/PORTS/Imgs/{shortcut.stem}.png", 0o644, COVER.read_bytes()

    for path in sorted(p for p in APP.rglob("*") if p.is_file()):
        if path.parent.name in ("lib", "licenses"):
            yield f"{GAMEDIR}/{path.relative_to(APP)}", 0o644, path.read_bytes()

    yield f"{GAMEDIR}/README.md", 0o644, (CARD / "README.md").read_bytes()
    yield f"{GAMEDIR}/minesofmoria", 0o755, BINARY.read_bytes()


def main():
    if not BINARY.is_file():
        raise SystemExit(f"{BINARY} missing -- run tools/build.sh armhf")

    OUT.parent.mkdir(exist_ok=True)
    with zipfile.ZipFile(OUT, "w", zipfile.ZIP_DEFLATED) as zf:
        for name, mode, blob in entries():
            info = zipfile.ZipInfo(name)
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = mode << 16
            zf.writestr(info, blob)

    print(f"{OUT} {OUT.stat().st_size // 1024} KiB")


if __name__ == "__main__":
    main()
