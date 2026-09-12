#!/usr/bin/env python3
"""Install Egypt's authored graphical HUD surfaces.

The player-facing HUD is artwork, not a runtime or build-time widget drawing
exercise.  The repository stores the finished indexed pixel surfaces as
compressed text payloads so Git remains friendly to them.  This tool only
unpacks those already-authored pixels into the build directory.

It deliberately contains no font renderer, button renderer, line/rectangle
ornament generator, or other UI construction code.
"""

from __future__ import annotations

import base64
import os
from pathlib import Path
import sys
import zlib

ASSETS = {
    "hud_chrome.e8p": "assets/hud_chrome_authored.e8p.b85",
    "hud_glyphs.e8p": "assets/hud_glyphs_authored.e8p.b85",
    "tool_icons.e8p": "assets/tool_icons_authored.e8p.b85",
}


def unpack(source: Path, destination: Path) -> None:
    encoded = "".join(source.read_text(encoding="ascii").split()).encode("ascii")
    payload = zlib.decompress(base64.b85decode(encoded))
    if payload[:4] != b"E8PA":
        raise RuntimeError(f"{source} is not an Egypt E8PA graphical surface")
    destination.write_bytes(payload)


def main() -> int:
    out_dir = Path(sys.argv[1] if len(sys.argv) > 1 else "build/assets")
    out_dir.mkdir(parents=True, exist_ok=True)
    repo_root = Path(__file__).resolve().parent.parent

    for output_name, source_name in ASSETS.items():
        unpack(repo_root / source_name, out_dir / output_name)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
