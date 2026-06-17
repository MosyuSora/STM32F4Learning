#!/usr/bin/env python3
"""convert.py  —  把任意图片转成 frame.raw (1024x600 BGRx) """
import struct, sys
from PIL import Image

SRC = sys.argv[1] if len(sys.argv) > 1 else "test.png"

im = Image.open(SRC).resize((1024, 600), Image.LANCZOS).convert("RGBA")
raw = bytearray()
for y in range(600):
    for x in range(1024):
        r, g, b, a = im.getpixel((x, y))
        raw += struct.pack('BBBB', b, g, r, 0)

open("frame.raw", "wb").write(raw)
print(f"{SRC} -> frame.raw ({len(raw)} bytes, 1024x600x4 BGRx)")
