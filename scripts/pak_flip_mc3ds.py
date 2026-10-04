#!/usr/bin/env python3
"""Converts the texture orientation convention of assets inside an
OptiCraft assets.pak between Minecraft 3DS Edition and Java Edition.

Minecraft 3DS Edition stores its art upside down relative to Java: plain
images are vertically flipped, and its own skin files are rotated 180
degrees (the reason github.com/Cracko298/MC-3DS-Flip exists). OptiCraft
renders on the 3DS with the file's row order natively, so a pak for this
port flips every image -- skins included -- on the Y axis only. The X
half of MC-3DS Edition's skin rotation must NOT be reproduced here: the
player model keeps the Java UV layout, and the 3DS sampler only mirrors
V (row order), never U, so an X-flipped skin reads the sheet mirrored
against those UVs and every body part samples the wrong region (the face
reads the arm texture, and so on).

A flip is its own inverse, so running the script again converts back.

Usage:
    pak_flip_mc3ds.py <in.pak> <out.pak> --all
        flips EVERY PNG in the pak on the Y axis, skins included: the 3DS
        deployment recipe (run it on assetsps2.pak to build the pak this
        port ships, or on a 3DS pak to convert it back for the other
        platforms). Use it only on a pack that sits wholly in one
        convention -- flipping a mixed pack breaks the other half, and a
        flip is its own inverse: never run it twice on the same art.
    pak_flip_mc3ds.py <assets.pak> <output.pak>
        flips the default MC-3DS-derived set (see below): the targeted
        conversion of MC-3DS Edition title/skin art into the Java
        convention for the other platforms.
    pak_flip_mc3ds.py <in.pak> <out.pak> --yflip PATTERN [--yflip ...]
                                             --xyflip PATTERN [--xyflip ...]
                                              --xflip PATTERN [--xflip ...]
        flips only the entries whose pak key matches a pattern (fnmatch,
        case-insensitive, forward slashes: "assets/skins/*.png"); --xflip
        moves an MC-3DS-style 180-degree skin into the plain-Y layout
        this port needs -- and repairs a pak that was given the X half it
        must never have had:
            --xflip "assets/skins/*.png"
    ... --skip PATTERN [--skip ...]
        exempts matching entries from every flip -- for building a
        uniformly-oriented pak out of a mixed one: flip what is still in
        the other convention, skip what already arrived converted.
    ... --dry-run
        lists what would be flipped and exits

Defaults (used when no --yflip/--xyflip/--xflip/--all is given):
    --yflip  assets/legacy/title.png      the legacy menu title
    --xyflip assets/skins/*.png           every skin (full sheet, _32 and
                                          _Front thumbnails alike) as
                                          MC-3DS Edition stores it: X+Y
                                          against the Java layout

Only 8-bit non-interlaced PNGs are flipped; anything else that matches a
pattern is left untouched with a warning. Non-PNG entries are copied
verbatim, and the output pak is byte-format identical to what
scripts/make_pak.py builds.

CPU-side colour lookup tables are excluded from the blanket flips (the
default set and --all): the game decodes those into colour tables on load
(Minecraft::startGame, CustomColorizer::loadColors) and never uploads them
as textures, so their row order is semantic rather than display-side -- a
flipped grasscolor.png makes biome colour lookups sample the wrong row (a
jungle reads the savanna row). A targeted --yflip (without --all) still
overrides the exclusion, which is how a pak produced before the exclusion
existed gets repaired:

    pak_flip_mc3ds.py in.pak out.pak --yflip assets/misc/grasscolor.png \
                                     --yflip assets/misc/foliagecolor.png \
                                     --yflip assets/misc/watercolor.png
"""

import fnmatch
import os
import struct
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

MAGIC = b"MCPK"
VERSION = 1
HEADER_BYTES = 32
ENTRY_BYTES = 16
DATA_ALIGN = 64

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"

DEFAULT_YFLIP = ("assets/legacy/title.png",)
DEFAULT_XYFLIP = ("assets/skins/*.png",)

# Entries the game reads on the CPU as colour lookup tables, never as GPU
# textures: RenderEngine::readTextureImageData feeds ColorizerGrass/
# ColorizerFoliage/ColorizerWater and CustomColorizer::loadColors, which
# index a 256x256 temperature/rainfall map by (row << 8) | column in the
# Java row order. These are kept in the Java convention even in a pak whose
# displayed images all flip to the MC-3DS orientation; a targeted
# --yflip/--xyflip (without --all) can still name one explicitly.
CPU_LUT_SKIP = (
    "assets/misc/grasscolor.png",
    "assets/misc/foliagecolor.png",
    "assets/misc/watercolor.png",
    "assets/misc/watercolorx.png",
    "assets/misc/pinecolor.png",
    "assets/misc/birchcolor.png",
    "assets/misc/swampgrasscolor.png",
    "assets/misc/swampfoliagecolor.png",
    "assets/misc/redstonecolor.png",
    "assets/misc/stemcolor.png",
    "assets/misc/myceliumparticlecolor.png",
)


class SkipPng(Exception):
    """The image is a PNG this flipper cannot rewrite; leave it verbatim."""


def read_pak(path):
    """Yields (key, payload bytes) for every entry, in table order."""
    with open(path, "rb") as pak:
        header = pak.read(HEADER_BYTES)
        if len(header) != HEADER_BYTES:
            raise SystemExit("%s: too short for an MCPK header" % path)
        magic, version, count, table_offset, names_offset, names_bytes, data_align, _ = (
            struct.unpack(">4sIIIIIII", header))
        if magic != MAGIC:
            raise SystemExit("%s: not an MCPK pak (magic %r)" % (path, magic))
        if version != 1:
            raise SystemExit("%s: MCPK version %d not supported (expected 1)"
                             % (path, version))
        if data_align <= 0:
            raise SystemExit("%s: nonsensical dataAlign %d" % (path, data_align))

        pak.seek(names_offset)
        names_blob = pak.read(names_bytes)
        if len(names_blob) != names_bytes:
            raise SystemExit("%s: names region truncated" % path)

        for i in range(count):
            pak.seek(table_offset + i * ENTRY_BYTES)
            _, name_offset, data_offset, size = struct.unpack(">IIII", pak.read(ENTRY_BYTES))
            end = names_blob.find(b"\0", name_offset)
            key = names_blob[name_offset:end].decode("utf-8")
            pak.seek(data_offset)
            payload = pak.read(size)
            if len(payload) != size:
                raise SystemExit("%s: entry %s truncated" % (path, key))
            yield key, payload


def chunk_crc(chunk_type, payload):
    return zlib.crc32(chunk_type + payload) & 0xFFFFFFFF


def append_chunk(out, chunk_type, payload):
    out += struct.pack(">I", len(payload))
    out += chunk_type
    out += payload
    out += struct.pack(">I", chunk_crc(chunk_type, payload))


def parse_png(data):
    if not data.startswith(PNG_SIGNATURE):
        raise SkipPng("not a PNG")
    chunks = []
    cursor = len(PNG_SIGNATURE)
    while cursor + 8 <= len(data):
        length, = struct.unpack(">I", data[cursor:cursor + 4])
        chunk_type = data[cursor + 4:cursor + 8]
        payload = data[cursor + 8:cursor + 8 + length]
        if len(payload) != length:
            raise SkipPng("truncated chunk")
        cursor += 12 + length
        chunks.append((chunk_type, payload))
        if chunk_type == b"IEND":
            break
    if not chunks or chunks[0][0] != b"IHDR":
        raise SkipPng("missing IHDR")

    width, height, bit_depth, color_type, _, _, interlace = struct.unpack(
        ">IIBBBBB", chunks[0][1])
    # The Y flip below is pure scanline order surgery, so it works at any
    # bit depth the scanline arithmetic handles; 16-bit/channel PNGs (what
    # tools like Photoshop emit when exporting without "8 bpc") used to be
    # copied verbatim here, which shipped them into converted packs still
    # in Java orientation -- the in-game symptom was individual textures
    # (often the /mob/* entity sheets, exported by a different tool than
    # the terrain art) rendering vertically mirrored. Only X flips keep
    # the 8-bit requirement (pixel-level byte surgery, guarded below).
    if bit_depth not in (1, 2, 4, 8, 16):
        raise SkipPng("bit depth %d (1/2/4/8/16 supported)" % bit_depth)
    if interlace not in (0, 1):
        raise SkipPng("interlace method %d" % interlace)
    channels = {0: 1, 2: 3, 3: 1, 4: 2, 6: 4}.get(color_type)
    if channels is None:
        raise SkipPng("color type %d" % color_type)

    idat = bytearray()
    palette = None
    transparency = None
    for chunk_type, payload in chunks:
        if chunk_type == b"IDAT":
            idat += payload
        elif chunk_type == b"PLTE":
            palette = payload
        elif chunk_type == b"tRNS":
            transparency = payload
    if not idat:
        raise SkipPng("no IDAT")
    return width, height, bit_depth, color_type, channels, interlace, palette, transparency, bytes(idat)


def unfilter_scanlines(raw, stride, height, filter_bpp):
    """PNG unfiltering works on bytes, not pixels: filter_bpp is the number
    of bytes one filtering unit spans (max(1, bits_per_pixel // 8)), so
    sub-byte depths -- 1-bit palette fonts among them -- unfilter with the
    same arithmetic as 8-bit images; only the stride differs."""
    rows = []
    previous = bytearray(stride)
    cursor = 0
    for _ in range(height):
        if cursor + 1 + stride > len(raw):
            raise SkipPng("image data short")
        filter_type = raw[cursor]
        scanline = bytearray(raw[cursor + 1:cursor + 1 + stride])
        cursor += 1 + stride
        if filter_type == 0:
            pass
        elif filter_type == 1:  # Sub
            for i in range(filter_bpp, stride):
                scanline[i] = (scanline[i] + scanline[i - filter_bpp]) & 0xFF
        elif filter_type == 2:  # Up
            for i in range(stride):
                scanline[i] = (scanline[i] + previous[i]) & 0xFF
        elif filter_type == 3:  # Average
            for i in range(stride):
                left = scanline[i - filter_bpp] if i >= filter_bpp else 0
                scanline[i] = (scanline[i] + ((left + previous[i]) >> 1)) & 0xFF
        elif filter_type == 4:  # Paeth
            for i in range(stride):
                left = scanline[i - filter_bpp] if i >= filter_bpp else 0
                up_left = previous[i - filter_bpp] if i >= filter_bpp else 0
                predictor = left + previous[i] - up_left
                pa = abs(predictor - left)
                pb = abs(predictor - previous[i])
                pc = abs(predictor - up_left)
                if pa <= pb and pa <= pc:
                    predictor = left
                elif pb <= pc:
                    predictor = previous[i]
                else:
                    predictor = up_left
                scanline[i] = (scanline[i] + predictor) & 0xFF
        else:
            raise SkipPng("filter type %d" % filter_type)
        rows.append(bytes(scanline))
        previous = scanline
    return rows


# Adam7 de-interlacing. Interlaced PNGs arrive out of handfuls of classic
# texture tools; refusing them left exactly those entries in Java
# orientation inside "converted" packs -- the in-game symptom was single
# textures (often /mob/* entity sheets exported with different tool
# settings than the terrain art) rendering vertically mirrored on the
# console. Pure-Python decode: each Adam7 pass is its own little filtered
# image; we unfilter it with the stock routine and scatter its pixels
# into a full-size unfiltered row-major buffer.

ADAM7_PASSES = (
    (0, 0, 8, 8),
    (4, 0, 8, 8),
    (0, 4, 8, 8),
    (2, 0, 4, 8),
    (0, 2, 4, 4),
    (1, 0, 2, 4),
    (0, 1, 2, 2),
)


def get_packed_pixel(row, px, bit_depth, channels):
    """Extract one pixel from a packed scanline. Sub-byte depths are
    big-endian-packed per the PNG spec; 8/16-bit are byte-aligned."""
    if bit_depth == 8:
        start = px * channels
        return row[start:start + channels]
    if bit_depth == 16:
        start = px * channels * 2
        return row[start:start + channels * 2]
    # sub-byte: bit_depth in (1, 2, 4) and channels == 1 (palette/gray)
    bit_ofs = px * bit_depth
    byte_ofs = bit_ofs // 8
    shift = 8 - bit_depth - (bit_ofs % 8)
    mask = (1 << bit_depth) - 1
    return bytes([(row[byte_ofs] >> shift) & mask])


def put_packed_pixel(row, px, pixel, bit_depth, channels):
    if bit_depth == 8:
        start = px * channels
        row[start:start + channels] = pixel
        return
    if bit_depth == 16:
        start = px * channels * 2
        row[start:start + channels * 2] = pixel
        return
    bit_ofs = px * bit_depth
    byte_ofs = bit_ofs // 8
    shift = 8 - bit_depth - (bit_ofs % 8)
    mask = ((1 << bit_depth) - 1) << shift
    row[byte_ofs] = (row[byte_ofs] & ~mask) | ((pixel[0] << shift) & mask)


def interlace_pass_dims(width, height, x0, y0, dx, dy):
    pw = (width - x0 + dx - 1) // dx if width > x0 else 0
    ph = (height - y0 + dy - 1) // dy if height > y0 else 0
    return pw, ph


def adam7_deinterlace(raw, width, height, bit_depth, channels):
    """Decode an interlaced=1 PNG pixel stream into full packed scanlines."""
    bpp_bits = bit_depth * channels
    filter_bpp = max(1, bpp_bits // 8)
    stride = (width * bpp_bits + 7) // 8
    rows = [bytearray(stride) for _ in range(height)]

    cursor = 0
    for x0, y0, dx, dy in ADAM7_PASSES:
        pw, ph = interlace_pass_dims(width, height, x0, y0, dx, dy)
        if pw == 0 or ph == 0:
            continue
        pass_stride = (pw * bpp_bits + 7) // 8
        pass_bytes = ph * (1 + pass_stride)
        if cursor + pass_bytes > len(raw):
            raise SkipPng("interlaced data short")
        pass_rows = unfilter_scanlines(raw[cursor:cursor + pass_bytes],
                                       pass_stride, ph, filter_bpp)
        cursor += pass_bytes
        for py, pass_row in enumerate(pass_rows):
            target_row = rows[y0 + py * dy]
            for ppx in range(pw):
                pixel = get_packed_pixel(pass_row, ppx, bit_depth, channels)
                put_packed_pixel(target_row, x0 + ppx * dx, pixel, bit_depth, channels)
    if cursor != len(raw):
        raise SkipPng("interlaced data trailing bytes")
    return [bytes(r) for r in rows]


def flip_png(data, flip_x, flip_y, crop_top_half=False):
    """Returns a new PNG with the requested axis flips applied. Adam7
    interlaced input is decoded pass-by-pass first and re-emitted
    progressive (the game's loader only cares about row content).

    crop_top_half keeps only the top half of the source rows (the ignored
    bottom is a modern pack's overlay/second-layer area): applied BEFORE any
    flip, since it addresses the source's own row order. Only ever enabled
    by texturepack_flip_3ds.py for the legacy 64x32 mob/item sheets a square
    (64x64+) modern sheet would otherwise break on this game -- see
    LEGACY_SLIM_SHEETS there."""
    width, height, bit_depth, color_type, channels, interlace, palette, transparency, idat = parse_png(data)
    stride = (width * bit_depth * channels + 7) // 8
    filter_bpp = max(1, (bit_depth * channels) // 8)
    if interlace != 0:
        rows = adam7_deinterlace(zlib.decompress(idat), width, height,
                                 bit_depth, channels)
    else:
        rows = unfilter_scanlines(zlib.decompress(idat), stride, height, filter_bpp)

    if crop_top_half and width == height and width % 64 == 0:
        # Only square 64x64/128x128... modern sheets carry the legacy
        # layout in the top half; anything else is left untouched (the
        # normal flips below still apply).
        rows = rows[: height // 2]
        height //= 2

    if flip_y:
        # Scanline order only: no pixel decoding needed, so every bit depth
        # and palette combination flips vertically here.
        rows.reverse()
    if flip_x:
        if bit_depth != 8:
            # Pixel-exact X flipping would need repacking sub-byte rows;
            # nothing in the MC-3DS set needs it (skins are 8-bit).
            raise SkipPng("X flip needs 8-bit pixels (got %d)" % bit_depth)
        row_pixels = width * channels
        flipped = []
        for row in rows:
            pixels = [row[i:i + channels] for i in range(0, row_pixels, channels)]
            pixels.reverse()
            flipped.append(b"".join(pixels))
        rows = flipped

    out = bytearray(PNG_SIGNATURE)
    ihdr = struct.pack(">IIBBBBB", width, height, bit_depth, color_type, 0, 0, 0)
    append_chunk(out, b"IHDR", ihdr)
    if palette is not None:
        append_chunk(out, b"PLTE", palette)
    if transparency is not None:
        append_chunk(out, b"tRNS", transparency)
    idat_bytes = bytearray()
    for row in rows:
        idat_bytes.append(0)  # filter type None for every scanline
        idat_bytes += row
    append_chunk(out, b"IDAT", zlib.compress(bytes(idat_bytes)))
    append_chunk(out, b"IEND", b"")
    return bytes(out)


def fnv1a32(text):
    value = 0x811C9DC5
    for byte in text.encode("utf-8"):
        value ^= byte
        value = (value * 0x01000193) & 0xFFFFFFFF
    return value


def align(value, alignment):
    return (value + alignment - 1) // alignment * alignment


def write_pak(entries, output):
    """entries: list of (key, payload). Same layout as make_pak.py."""
    names = bytearray()
    records = []
    for key, payload in entries:
        name_offset = len(names)
        names += key.encode("utf-8") + b"\0"
        records.append([fnv1a32(key.lower()), name_offset, 0, len(payload), key, payload])
    records.sort(key=lambda record: (record[0], record[4]))

    table_offset = HEADER_BYTES
    names_offset = table_offset + len(records) * ENTRY_BYTES
    data_offset = align(names_offset + len(names), DATA_ALIGN)

    cursor = data_offset
    for record in records:
        record[2] = cursor
        cursor = align(cursor + record[3], DATA_ALIGN)

    with open(output, "wb") as out:
        out.write(struct.pack(">4sIIIIIII", MAGIC, VERSION, len(records), table_offset,
                              names_offset, len(names), DATA_ALIGN, 0))
        for hash_value, name_offset, offset, size, _, _ in records:
            out.write(struct.pack(">IIII", hash_value, name_offset, offset, size))
        out.write(bytes(names))
        out.write(b"\0" * (data_offset - out.tell()))
        for _, _, offset, _, _, payload in records:
            out.write(b"\0" * (offset - out.tell()))
            out.write(payload)
            out.write(b"\0" * (align(offset + len(payload), DATA_ALIGN)
                               - (offset + len(payload))))


def main():
    args = sys.argv[1:]
    dry_run = False
    flip_all = False
    yflip = []
    xyflip = []
    xflip = []
    skip = []
    positional = []
    i = 0
    while i < len(args):
        arg = args[i]
        if arg == "--yflip":
            yflip.append(args[i + 1])
            i += 2
        elif arg == "--xyflip":
            xyflip.append(args[i + 1])
            i += 2
        elif arg == "--xflip":
            xflip.append(args[i + 1])
            i += 2
        elif arg == "--skip":
            skip.append(args[i + 1])
            i += 2
        elif arg == "--dry-run":
            dry_run = True
            i += 1
        elif arg == "--all":
            flip_all = True
            i += 1
        else:
            positional.append(arg)
            i += 1
    if len(positional) != 2:
        raise SystemExit(__doc__)
    pak_path, out_path = positional

    if flip_all:
        # Every PNG -- skins included -- gets the plain Y flip: this
        # target's sampler compensates row order (V) only, and the player
        # model keeps the Java UV layout, so skins must not gain the X
        # half of MC-3DS Edition's rotation. An explicit --xyflip/--xflip
        # still applies on top for the patterns it names (X wins over Y
        # in the loop below). The CPU-side colour tables never join the
        # blanket flip.
        skip = list(CPU_LUT_SKIP) + skip
        yflip.insert(0, "*")
    elif not yflip and not xyflip and not xflip:
        skip = list(CPU_LUT_SKIP) + skip
        yflip = list(DEFAULT_YFLIP)
        xyflip = list(DEFAULT_XYFLIP)

    def match(key, patterns):
        lowered = key.lower()
        return any(fnmatch.fnmatch(lowered, pattern.lower()) for pattern in patterns)

    entries = []
    flipped = 0
    skipped = 0
    for key, payload in read_pak(pak_path):
        exempt = match(key, skip)
        hit_xy = not exempt and match(key, xyflip)
        hit_x = not exempt and match(key, xflip)
        hit_y = not exempt and match(key, yflip)
        flip_x = hit_xy or hit_x
        flip_y = hit_xy or hit_y
        if exempt and (match(key, yflip) or match(key, xyflip) or match(key, xflip)):
            print("skipped %s (excluded from flip)" % key)
        elif (flip_x or flip_y) and not dry_run:
            try:
                payload = flip_png(payload, flip_x, flip_y)
                flipped += 1
                print("flipped %s (%s)" % (key, "X+Y" if flip_x and flip_y else "X" if flip_x else "Y"))
            except SkipPng as reason:
                skipped += 1
                print("LEFT AS-IS %s (%s)" % (key, reason))
        elif flip_x or flip_y:
            print("would flip %s (%s)" % (key, "X+Y" if flip_x and flip_y else "X" if flip_x else "Y"))
        entries.append((key, payload))

    if dry_run:
        print("dry run: %d entries inspected, nothing written" % len(entries))
        return

    write_pak(entries, out_path)
    print("wrote %s: %d entries, %d flipped, %d left as-is"
          % (out_path, len(entries), flipped, skipped))


if __name__ == "__main__":
    main()
