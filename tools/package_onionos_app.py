#!/usr/bin/env python3
"""Zips onionos/ into build/minesofmoria-onionos-app.zip.

The archive mirrors the SD card, so it unzips at the card root and the app lands
in App/MinesOfMoria/ where MainUI looks for it. The armhf binary is installed
under its plain name, matching what launch.sh runs.
"""
import pathlib
import zipfile

ROOT = pathlib.Path(__file__).resolve().parent.parent
CARD = ROOT / "onionos"
APP = "App/MinesOfMoria"
BINARY = ROOT / "build" / "minesofmoria.armhf"
OUT = ROOT / "build" / "minesofmoria-onionos-app.zip"

EXECUTABLE = {"launch.sh"}


def main():
    if not BINARY.is_file():
        raise SystemExit(f"{BINARY} missing -- run tools/build.sh armhf")

    OUT.parent.mkdir(exist_ok=True)
    with zipfile.ZipFile(OUT, "w", zipfile.ZIP_DEFLATED) as zf:
        for path in sorted(p for p in (CARD / "App").rglob("*") if p.is_file()):
            name = str(path.relative_to(CARD))
            info = zipfile.ZipInfo(name)
            info.compress_type = zipfile.ZIP_DEFLATED
            mode = 0o755 if path.name in EXECUTABLE else 0o644
            info.external_attr = mode << 16
            zf.writestr(info, path.read_bytes())

        # The card README sits beside the app, not at the card root.
        readme = zipfile.ZipInfo(f"{APP}/README.md")
        readme.compress_type = zipfile.ZIP_DEFLATED
        readme.external_attr = 0o644 << 16
        zf.writestr(readme, (CARD / "README.md").read_bytes())

        binary = zipfile.ZipInfo(f"{APP}/minesofmoria")
        binary.compress_type = zipfile.ZIP_DEFLATED
        binary.external_attr = 0o755 << 16
        zf.writestr(binary, BINARY.read_bytes())

    print(f"{OUT} {OUT.stat().st_size // 1024} KiB")


if __name__ == "__main__":
    main()
