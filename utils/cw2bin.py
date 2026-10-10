#!/usr/bin/env python3
"""
cw2bin.py - convert a ClassicWorld (.cw) map into a raw level.bin block stream.

Output: exactly 256 * 64 * 256 = 4,194,304 bytes, one byte per block,
ordered X fastest, then Z, then Y:  index = (y * 256 + z) * 256 + x
(this is the same order ClassicWorld stores BlockArray in).

Usage:
    python3 cw2bin.py map.cw                 # writes level.bin
    python3 cw2bin.py map.cw -o out/level.bin
    python3 cw2bin.py map.cw --resize        # crop/pad (with air) if the map isn't 256x64x256

Pure standard library, no dependencies.
"""
import argparse
import gzip
import struct
import sys

# Target dimensions: X (width), Y (height), Z (length)
TX, TY, TZ = 256, 64, 256


# --------------------------------------------------------------------------
# Minimal NBT reader (big-endian, as used by .cw files)
# --------------------------------------------------------------------------
class NBT:
    def __init__(self, data):
        self.d = data
        self.p = 0

    def take(self, n):
        if n < 0 or self.p + n > len(self.d):
            raise ValueError("truncated or corrupt NBT data")
        b = self.d[self.p:self.p + n]
        self.p += n
        return b

    def unpack(self, fmt):
        return struct.unpack(fmt, self.take(struct.calcsize(fmt)))[0]

    def string(self):
        return self.take(self.unpack(">H")).decode("utf-8", "replace")

    def payload(self, t):
        if t == 1:  return self.unpack(">b")
        if t == 2:  return self.unpack(">h")
        if t == 3:  return self.unpack(">i")
        if t == 4:  return self.unpack(">q")
        if t == 5:  return self.unpack(">f")
        if t == 6:  return self.unpack(">d")
        if t == 7:  return self.take(self.unpack(">i"))                # byte array
        if t == 8:  return self.string()
        if t == 9:                                                      # list
            et = self.unpack(">B")
            n = self.unpack(">i")
            return [self.payload(et) for _ in range(n)]
        if t == 10:                                                     # compound
            out = {}
            while True:
                tt = self.unpack(">B")
                if tt == 0:
                    return out
                name = self.string()
                out[name] = self.payload(tt)
        if t == 11: return [self.unpack(">i") for _ in range(self.unpack(">i"))]
        if t == 12: return [self.unpack(">q") for _ in range(self.unpack(">i"))]
        raise ValueError(f"unknown NBT tag type {t}")

    def root(self):
        t = self.unpack(">B")
        if t != 10:
            raise ValueError("root tag is not a compound - not a ClassicWorld file")
        self.string()
        return self.payload(10)


def load_cw(path):
    with open(path, "rb") as f:
        raw = f.read()
    try:
        raw = gzip.decompress(raw)
    except OSError:
        pass  # tolerate an un-gzipped file
    root = NBT(raw).root()

    for key in ("X", "Y", "Z", "BlockArray"):
        if key not in root:
            raise ValueError(f"missing '{key}' tag - not a valid ClassicWorld file")

    x, y, z = root["X"], root["Y"], root["Z"]
    blocks = root["BlockArray"]
    if len(blocks) != x * y * z:
        raise ValueError(
            f"BlockArray is {len(blocks)} bytes but {x}x{y}x{z} = {x * y * z}")
    return x, y, z, blocks, root.get("Name", "")


# --------------------------------------------------------------------------
def convert(blocks, sx, sy, sz):
    """Return a TX*TY*TZ bytearray, cropping/padding with air (0) if needed."""
    if (sx, sy, sz) == (TX, TY, TZ):
        return bytearray(blocks)

    out = bytearray(TX * TY * TZ)
    cx = min(sx, TX)
    for y in range(min(sy, TY)):
        for z in range(min(sz, TZ)):
            s = (y * sz + z) * sx
            d = (y * TZ + z) * TX
            out[d:d + cx] = blocks[s:s + cx]
    return out


def main():
    ap = argparse.ArgumentParser(description="Convert a .cw map to a raw 256x64x256 level.bin")
    ap.add_argument("input", help="input .cw file")
    ap.add_argument("-o", "--output", default="level.bin", help="output file (default: level.bin)")
    ap.add_argument("--resize", action="store_true",
                    help="crop/pad with air if the map is not 256x64x256 (default: refuse)")
    args = ap.parse_args()

    try:
        sx, sy, sz, blocks, name = load_cw(args.input)
    except (OSError, ValueError, struct.error) as e:
        sys.exit(f"error: {e}")

    print(f"Loaded '{name}' ({sx}x{sy}x{sz})")

    if (sx, sy, sz) != (TX, TY, TZ):
        if not args.resize:
            sys.exit(f"error: map is {sx}x{sy}x{sz}, server needs {TX}x{TY}x{TZ}. "
                     f"Use --resize to crop/pad.")
        print(f"Warning: cropping/padding to {TX}x{TY}x{TZ}")

    out = convert(blocks, sx, sy, sz)
    assert len(out) == TX * TY * TZ

    with open(args.output, "wb") as f:
        f.write(out)
    print(f"Wrote {len(out)} bytes to {args.output}")


if __name__ == "__main__":
    main()
