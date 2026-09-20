#!/usr/bin/env python3
"""Reconstruct and verify Egypt's authored menu surface."""

from __future__ import annotations

import base64
import hashlib
import lzma
from pathlib import Path
import sys

EXPECTED_PARTS = 29
EXPECTED_SIZE = 35948
EXPECTED_SHA256 = "e22ea3d3689495f83b6bd31437806ff897fff530f3d8420579f142bd3a1337c9"


def main() -> int:
    repo_root = Path(__file__).resolve().parent.parent
    output = Path(sys.argv[1] if len(sys.argv) > 1 else "build/assets/menu.e16")
    parts = sorted((repo_root / "assets").glob("menu_q20_xz_*.b64"))
    if len(parts) != EXPECTED_PARTS:
        raise RuntimeError(f"expected {EXPECTED_PARTS} menu parts, found {len(parts)}")

    encoded = "".join("".join(path.read_text(encoding="ascii").split()) for path in parts)
    payload = lzma.decompress(base64.b64decode(encoded))

    if len(payload) != EXPECTED_SIZE:
        raise RuntimeError(f"menu size mismatch: {len(payload)}")
    if payload[:4] != b"EJ8A":
        raise RuntimeError("menu surface has the wrong signature")
    digest = hashlib.sha256(payload).hexdigest()
    if digest != EXPECTED_SHA256:
        raise RuntimeError(f"menu SHA-256 mismatch: {digest}")

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(payload)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
