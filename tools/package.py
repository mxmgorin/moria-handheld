#!/usr/bin/env python3
"""Zips port/ into build/moria.zip, the file PortMaster installs.

zip(1) is not assumed to be present; permissions matter, so external_attr is
set explicitly rather than left to whatever the writer defaults to.
"""
import pathlib
import zipfile

ROOT = pathlib.Path(__file__).resolve().parent.parent
PORT = ROOT / "portmaster"
OUT = ROOT / "build" / "minesofmoria-portmaster.zip"

# Only what port.json lists as items; cover and screenshot are repo metadata.
ITEMS = ["Mines of Moria.sh", "minesofmoria"]
EXECUTABLE = {".aarch64", ".armhf", ".x86_64"}


def entries():
    """The launcher, the binaries and the licenses -- nothing else.

    The game writes its cache and saves into whatever directory it runs from,
    and a stray moria.cache shipped to a device would overwrite the player's
    settings, so the gamedir is filtered rather than swept up whole.
    """
    for item in ITEMS:
        path = PORT / item
        if path.is_file():
            yield path
            continue
        for entry in sorted(path.rglob("*")):
            if not entry.is_file():
                continue
            if entry.suffix in EXECUTABLE or entry.parent.name == "licenses":
                yield entry
            else:
                print(f"skipped {entry.relative_to(PORT)}")


def main():
    OUT.parent.mkdir(exist_ok=True)
    with zipfile.ZipFile(OUT, "w", zipfile.ZIP_DEFLATED) as zf:
        for path in entries():
            info = zipfile.ZipInfo(str(path.relative_to(PORT)))
            info.compress_type = zipfile.ZIP_DEFLATED
            mode = 0o755 if path.suffix in EXECUTABLE else 0o644
            info.external_attr = mode << 16
            zf.writestr(info, path.read_bytes())
    print(f"{OUT} {OUT.stat().st_size // 1024} KiB")


if __name__ == "__main__":
    main()
