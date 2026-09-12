#!/usr/bin/env python3
"""Build Egypt's first graphics-only HUD surfaces.

This is an asset-production tool, not a runtime UI renderer. It writes indexed
RGBA pixel surfaces that the game later loads and blits. At runtime the HUD is
therefore composed from graphics-on-graphics, including words and numbers.
"""

from __future__ import annotations

import os
import struct
import sys

# Palette indices. Index 0 is transparent and is used outside shaped artwork.
TRANSPARENT = 0
INK = 1
DEEP = 2
DARK = 3
BROWN = 4
BROWN_HI = 5
BRONZE = 6
GOLD = 7
GOLD_HI = 8
PALE = 9
SHADOW = 10
RED_BROWN = 11
SAND = 12
COPPER = 13
MUTED_GOLD = 14
BLACK = 15

PALETTE = [
    (0, 0, 0, 0),
    (242, 216, 163, 255),
    (42, 22, 16, 255),
    (55, 29, 20, 255),
    (72, 37, 24, 255),
    (104, 56, 31, 255),
    (132, 78, 38, 255),
    (198, 142, 68, 255),
    (235, 183, 92, 255),
    (247, 224, 173, 255),
    (37, 18, 14, 210),
    (92, 45, 28, 255),
    (191, 140, 73, 255),
    (167, 101, 47, 255),
    (218, 161, 79, 255),
    (10, 7, 6, 255),
]
PALETTE.extend([(0, 0, 0, 0)] * (256 - len(PALETTE)))


class Canvas:
    def __init__(self, width: int, height: int, fill: int = TRANSPARENT):
        self.width = width
        self.height = height
        self.pixels = bytearray([fill]) * (width * height)

    def pixel(self, x: int, y: int, colour: int) -> None:
        if 0 <= x < self.width and 0 <= y < self.height:
            self.pixels[y * self.width + x] = colour

    def rect(self, x0: int, y0: int, x1: int, y1: int, colour: int) -> None:
        x0 = max(0, x0); y0 = max(0, y0)
        x1 = min(self.width, x1); y1 = min(self.height, y1)
        if x0 >= x1 or y0 >= y1:
            return
        row = bytes([colour]) * (x1 - x0)
        for y in range(y0, y1):
            off = y * self.width + x0
            self.pixels[off:off + (x1 - x0)] = row

    def line(self, x0: int, y0: int, x1: int, y1: int, colour: int) -> None:
        dx = abs(x1 - x0)
        sx = 1 if x0 < x1 else -1
        dy = -abs(y1 - y0)
        sy = 1 if y0 < y1 else -1
        err = dx + dy
        while True:
            self.pixel(x0, y0, colour)
            if x0 == x1 and y0 == y1:
                break
            e2 = err * 2
            if e2 >= dy:
                err += dy; x0 += sx
            if e2 <= dx:
                err += dx; y0 += sy

    def circle(self, cx: int, cy: int, radius: int, colour: int) -> None:
        rr = radius * radius
        for y in range(cy - radius, cy + radius + 1):
            dy = y - cy
            span = int(max(0, rr - dy * dy) ** 0.5)
            self.rect(cx - span, y, cx + span + 1, y + 1, colour)

    def triangle(self, a, b, c, colour: int) -> None:
        pts = sorted([a, b, c], key=lambda p: p[1])
        (x0, y0), (x1, y1), (x2, y2) = pts
        if y0 == y2:
            self.rect(min(x0, x1, x2), y0, max(x0, x1, x2) + 1, y0 + 1, colour)
            return

        def edge(pa, pb, y):
            xa, ya = pa; xb, yb = pb
            if ya == yb:
                return xa
            return xa + (xb - xa) * (y - ya) // (yb - ya)

        for y in range(y0, y2 + 1):
            xa = edge((x0, y0), (x2, y2), y)
            xb = edge((x0, y0), (x1, y1), y) if y <= y1 else edge((x1, y1), (x2, y2), y)
            if xa > xb:
                xa, xb = xb, xa
            self.rect(xa, y, xb + 1, y + 1, colour)

    def frame(self, x: int, y: int, w: int, h: int, outer: int, inner: int, face: int) -> None:
        self.rect(x, y, x + w, y + h, outer)
        self.rect(x + 2, y + 2, x + w - 2, y + h - 2, inner)
        self.rect(x + 4, y + 4, x + w - 4, y + h - 4, face)
        self.line(x + 4, y + 4, x + w - 5, y + 4, GOLD_HI)
        self.line(x + 4, y + 4, x + 4, y + h - 5, GOLD)
        self.line(x + 4, y + h - 5, x + w - 5, y + h - 5, DEEP)


def write_e8pa(path: str, canvas: Canvas) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as out:
        out.write(b"E8PA")
        out.write(struct.pack("<HH", canvas.width, canvas.height))
        for r, g, b, a in PALETTE:
            out.write(bytes((r, g, b, a)))
        out.write(canvas.pixels)


# 5x7 source masks are used only here, during asset production. The executable
# never receives these masks and never renders a font. It receives a completed
# graphical glyph atlas.
FONT = {
    ' ':[0,0,0,0,0,0,0], 'A':[14,17,17,31,17,17,17], 'B':[30,17,17,30,17,17,30],
    'C':[14,17,16,16,16,17,14], 'D':[30,17,17,17,17,17,30], 'E':[31,16,16,30,16,16,31],
    'F':[31,16,16,30,16,16,16], 'G':[14,17,16,23,17,17,14], 'H':[17,17,17,31,17,17,17],
    'I':[14,4,4,4,4,4,14], 'J':[1,1,1,1,17,17,14], 'K':[17,18,20,24,20,18,17],
    'L':[16,16,16,16,16,16,31], 'M':[17,27,21,21,17,17,17], 'N':[17,25,21,19,17,17,17],
    'O':[14,17,17,17,17,17,14], 'P':[30,17,17,30,16,16,16], 'Q':[14,17,17,17,21,18,13],
    'R':[30,17,17,30,20,18,17], 'S':[15,16,16,14,1,1,30], 'T':[31,4,4,4,4,4,4],
    'U':[17,17,17,17,17,17,14], 'V':[17,17,17,17,17,10,4], 'W':[17,17,17,21,21,21,10],
    'X':[17,17,10,4,10,17,17], 'Y':[17,17,10,4,4,4,4], 'Z':[31,1,2,4,8,16,31],
    '0':[14,17,19,21,25,17,14], '1':[4,12,4,4,4,4,14], '2':[14,17,1,2,4,8,31],
    '3':[30,1,1,14,1,1,30], '4':[2,6,10,18,31,2,2], '5':[31,16,16,30,1,1,30],
    '6':[14,16,16,30,17,17,14], '7':[31,1,2,4,8,8,8], '8':[14,17,17,14,17,17,14],
    '9':[14,17,17,15,1,1,14], ':':[0,12,12,0,12,12,0], '-':[0,0,0,31,0,0,0],
    '/':[1,2,2,4,8,8,16], ',':[0,0,0,0,0,4,8], '.':[0,0,0,0,0,12,12],
    '?':[14,17,1,2,4,0,4], '+':[0,4,4,31,4,4,0],
}

GLYPHS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 :-/,.?+"


def draw_glyph(canvas: Canvas, ch: str, x: int, y: int, scale: int) -> None:
    rows = FONT.get(ch, FONT['?'])
    # Dark offset and warm face are baked into the artwork itself.
    for ry, bits in enumerate(rows):
        for rx in range(5):
            if not (bits >> (4 - rx)) & 1:
                continue
            px = x + rx * scale
            py = y + ry * scale
            canvas.rect(px + 1, py + 1, px + scale + 1, py + scale + 1, SHADOW)
            canvas.rect(px, py, px + scale, py + scale, PALE)
            # Large glyphs get tiny serif pixels so they read as illustrated UI,
            # not enlarged terminal cells.
            if scale >= 3 and (ry == 0 or ry == 6):
                canvas.pixel(px - 1, py + scale - 1, GOLD_HI)
                canvas.pixel(px + scale, py + scale - 1, GOLD_HI)


def build_glyphs() -> Canvas:
    large_w, large_h = 18, 26
    small_w, small_h = 10, 15
    canvas = Canvas(len(GLYPHS) * large_w, large_h + small_h)
    for i, ch in enumerate(GLYPHS):
        if ch != ' ':
            draw_glyph(canvas, ch, i * large_w + 2, 2, 3)
            draw_glyph(canvas, ch, i * small_w + 2, large_h + 4, 1)
    return canvas


def draw_medallion(c: Canvas, cx: int, cy: int, icon: int, active: bool) -> None:
    c.circle(cx + 1, cy + 2, 14, DEEP)
    c.circle(cx, cy, 14, GOLD_HI if active else GOLD)
    c.circle(cx, cy, 12, BRONZE)
    c.circle(cx, cy, 9, BROWN_HI if active else BROWN)
    c.line(cx - 6, cy - 10, cx + 5, cy - 10, GOLD_HI)
    if icon == 0:
        c.rect(cx - 5, cy - 7, cx - 2, cy + 7, SHADOW)
        c.rect(cx + 2, cy - 7, cx + 5, cy + 7, SHADOW)
        c.rect(cx - 5, cy - 8, cx - 3, cy + 6, PALE)
        c.rect(cx + 2, cy - 8, cx + 4, cy + 6, PALE)
    else:
        total = icon * 6 + (icon - 1)
        sx = cx - total // 2
        for _ in range(icon):
            c.triangle((sx + 1, cy - 6), (sx + 1, cy + 6), (sx + 7, cy), SHADOW)
            c.triangle((sx, cy - 7), (sx, cy + 5), (sx + 6, cy - 1), PALE)
            sx += 7


def build_chrome() -> Canvas:
    c = Canvas(1024, 168)

    # 64x72 repeating carved HUD background tile.
    bands = [BROWN_HI, BROWN_HI, BROWN, BROWN, DARK, DARK]
    for y in range(65):
        c.rect(0, y, 64, y + 1, bands[min(len(bands)-1, y * len(bands) // 65)])
    for ox in (4, 25, 46):
        c.line(ox + 2, 8, ox + 8, 5, MUTED_GOLD)
        c.line(ox + 8, 5, ox + 14, 8, MUTED_GOLD)
        c.line(ox + 14, 8, ox + 8, 11, BRONZE)
        c.line(ox + 8, 11, ox + 2, 8, BRONZE)
        c.line(ox + 8, 11, ox + 8, 25, DEEP)
        c.circle(ox + 8, 28, 3, COPPER)
        c.line(ox + 1, 37, ox + 15, 37, BRONZE)
        c.line(ox + 3, 41, ox + 13, 41, DEEP)
    c.rect(0, 65, 64, 72, DEEP)
    c.rect(0, 65, 64, 66, GOLD)
    for x in range(0, 64, 8):
        c.triangle((x, 68), (x + 4, 66), (x + 8, 68), BRONZE)
        c.triangle((x, 68), (x + 8, 68), (x + 4, 71), BRONZE)

    # Four complete control-cluster graphics: paused, x1, x2, x4.
    for state in range(4):
        x0 = 64 + state * 180
        c.triangle((x0, 2), (x0 + 174, 2), (x0 + 158, 49), DARK)
        c.triangle((x0, 2), (x0 + 158, 49), (x0, 49), DARK)
        c.line(x0 + 1, 48, x0 + 157, 48, GOLD)
        c.line(x0 + 174, 2, x0 + 158, 49, GOLD_HI)
        for k in range(5):
            c.line(x0 + 146, 8 + k * 2, x0 + 169 - k * 3, 5 + k * 5, BRONZE)
        for icon, cx in enumerate((24, 57, 90, 123)):
            draw_medallion(c, x0 + cx, 23, icon, icon == state)
        c.frame(x0 + 140, 10, 26, 26, DEEP, BRONZE, DARK)

    # Central date cartouche is artwork, including the wings and scarab.
    x0, y0 = 0, 72
    for side in (-1, 1):
        root = x0 + 160 + side * 108
        for k in range(6):
            yy = y0 + 9 + k * 3
            length = 42 - k * 5
            c.line(root, y0 + 19, root + side * length, yy, GOLD)
            c.line(root, y0 + 20, root + side * (length - 3), yy + 2, BRONZE)
    c.frame(x0 + 52, y0 + 4, 216, 36, DEEP, GOLD, RED_BROWN)
    c.circle(x0 + 160, y0 + 10, 7, BRONZE)
    c.triangle((x0 + 153, y0 + 10), (x0 + 140, y0 + 5), (x0 + 145, y0 + 14), COPPER)
    c.triangle((x0 + 167, y0 + 10), (x0 + 180, y0 + 5), (x0 + 175, y0 + 14), COPPER)
    c.line(x0 + 160, y0 + 2, x0 + 160, y0 + 18, GOLD_HI)

    def menu_plaque(px: int, py: int, hover: bool) -> None:
        c.frame(px + 8, py + 4, 184, 36, DEEP, GOLD_HI if hover else GOLD, BROWN)
        for cx in (px + 22, px + 178):
            c.line(cx, py + 31, cx, py + 12, COPPER)
            c.triangle((cx, py + 12), (cx - 6, py + 22), (cx, py + 19), GOLD)
            c.triangle((cx, py + 12), (cx + 6, py + 22), (cx, py + 19), GOLD)

    menu_plaque(800, 72, False)
    menu_plaque(800, 116, True)
    return c


def main() -> int:
    output_dir = sys.argv[1] if len(sys.argv) > 1 else "build/assets"
    os.makedirs(output_dir, exist_ok=True)
    write_e8pa(os.path.join(output_dir, "hud_chrome.e8p"), build_chrome())
    write_e8pa(os.path.join(output_dir, "hud_glyphs.e8p"), build_glyphs())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
