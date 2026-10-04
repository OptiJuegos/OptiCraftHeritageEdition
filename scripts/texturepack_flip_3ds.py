#!/usr/bin/env python3
"""Converts a Java-Edition texture pack (.zip) to the 3DS orientation.

WHY: OptiCraft's 3DS backend renders the Minecraft 3DS Edition asset
orientation natively -- it samples rows in the file's own order, where the
Java/GL convention samples them bottom-up (see the upload note in
src/3ds/render/DsTexture.cpp: "rows are stored in the file's own order...
the official ecosystem's art is stored upside down relative to Java").
That is why the 3DS assets.pak ships pre-flipped: scripts/pak_flip_mc3ds.py
--all is part of this port's deployment recipe. A user texture pack is Java
art in Java orientation, so on a 3DS every image in it samples vertically
mirrored -- grass hangs from the top of the block face, the GUI is upside
down, the font glyphs sit on their heads.

This script flips every PNG in the pack on the Y axis so the pack matches
what the 3DS expects on disk, exactly what pak_flip_mc3ds.py --all does to
a pak. A flip is its own inverse, so running the script on a converted
pack converts it back -- never run it twice on the same art.

Excluded from the flip, exactly like the pak script's blanket mode: the
CPU-side colour lookup tables (misc/grasscolor.png and friends). The game
decodes those into colour tables on the CPU (Minecraft::startGame,
CustomColorizer::loadColors) and indexes them by (row << 8) | column in the
Java row order, so their row order is semantic and a flipped
grasscolor.png makes biome colour lookups sample the wrong row (a jungle
reads the savanna row).

Everything else flips, including pack.png (the thumbnail the Texture Packs
list uploads as a texture, so it is display-side like the rest) and
mob/char.png (a texture the entity model samples with the Java UV layout).
The flip is ALWAYS Y-only: the 3DS sampler mirrors rows and never columns,
and the player model keeps the Java UV layout, so packs never gain the X
half of MC-3DS Edition's 180-degree skin rotation (the reason
pak_flip_mc3ds.py has --xflip to STRIP it).

All PNGs are rewritten by the pure-Python flipper shared with
pak_flip_mc3ds.py: palette and 1/2/4/8/16-bit depths all flip, and Adam7
interlaced images are decoded pass-by-pass and re-emitted progressive (so
nothing is ever left silently in Java orientation -- earlier builds copied
interlaced/16-bit files verbatim, which is exactly how a converted pack
ended up with vertically mirrored /mob/* entity sheets). Non-PNG entries
(pack.txt, .properties, sounds...) are copied verbatim.

Usage:
    texturepack_flip_3ds.py <in.zip> <out.zip>
        converts the pack to the 3DS orientation: every PNG Y-flipped
        except the CPU colour tables.
    texturepack_flip_3ds.py <in.zip> <out.zip> --skip PATTERN [--skip ...]
        additionally exempts entries whose (lower-cased, forward-slashed)
        name matches an fnmatch pattern -- e.g. --skip "mob/char.png".
    texturepack_flip_3ds.py <in.zip> <out.zip> --only PATTERN [--only ...]
        flips ONLY the matching entries instead of the default set; an
        explicit --only also overrides the CPU-table exclusion (name a
        colour table to repair a pack whose LUTs shipped flipped -- the
        same contract pak_flip_mc3ds.py's targeted --yflip carries).
    texturepack_flip_3ds.py <in.zip> <out.zip> --dry-run
        lists what would be flipped and exits.

Player recipe (a pack converted for the 3DS is a DIFFERENT file than the
one a PC install uses -- keep both):
    1. texturepack_flip_3ds.py JavaPack.zip JavaPack-3DS.zip
    2. copy JavaPack-3DS.zip to <SD card>/opticraft/.minecraft/texturepacks/
    3. in the game: Options > Texture Packs, pick it, Done.
"""

import fnmatch
import os
import sys
import zipfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# The PNG rewriting is shared with the pak converter so there is exactly one
# implementation of "flip a PNG in place, keeping palette and transparency":
# flip_png() also refuses what it cannot rewrite (interlaced, truncated...)
# by raising SkipPng, which this script reports and copies verbatim.
from pak_flip_mc3ds import CPU_LUT_SKIP, PNG_SIGNATURE, SkipPng, flip_png

# The pak keys carry the internal "assets/" prefix; a user pack's entries
# sit at the zip root (terrain.png, misc/grasscolor.png), but a pack built
# by unpacking a pak can carry the prefix too. Normalise both to one list.
CPU_LUT_ENTRY_NAMES = tuple(
    key[len("assets/"):] if key.startswith("assets/") else key
    for key in CPU_LUT_SKIP)

# Mob/item/armor sheets that are 64x32 in this game's (Beta 1.7.3) asset
# layout. Modern packs (MC 1.8+) ship these SQUARE (64x64, or HD 128x128):
# the old box UVs keep addressing the top half and the new overlay layers
# live in the bottom half, so a square sheet sampled by the legacy model
# reads as garbage (e.g. the zombie of the Modrinth "MC New Textures" pack,
# whose mob/zombie.png is 64x64, renders as a floating green-and-cyan
# square). Such an entry is cropped to its top half -- the overlays are not
# in this game anyway. KEEP IN LOCKSTEP with kLegacySlimSheets in
# src/3ds/assets/DsAssetConvert.cpp -- that list is the runtime counterpart;
# both mirror this one. Deliberately absent: mob/snowman.png,
# mob/villager*.png and mob/villager_golem.png are 64x64/128x128 even in the
# game's own shipped assets, so a square sheet there is already correct.
LEGACY_SLIM_SHEETS = frozenset([
    "mob/char.png", "mob/cavespider.png", "mob/chicken.png",
    "mob/cow.png", "mob/creeper.png", "mob/enderman.png",
    "mob/enderman_eyes.png", "mob/fire.png",
    "mob/ghast.png", "mob/ghast_fire.png", "mob/lava.png",
    "mob/ozelot.png", "mob/cat_black.png", "mob/cat_red.png",
    "mob/cat_siamese.png", "mob/pig.png",
    "mob/pigman.png", "mob/pigzombie.png", "mob/redcow.png",
    "mob/saddle.png", "mob/sheep.png", "mob/sheep_fur.png",
    "mob/silverfish.png", "mob/skeleton.png",
    "mob/slime.png", "mob/spider.png", "mob/spider_eyes.png",
    "mob/squid.png", "mob/wolf.png", "mob/wolf_angry.png",
    "mob/wolf_collar.png", "mob/wolf_tame.png",
    "mob/zombie.png",
    "armor/chainmail_1.png", "armor/chainmail_2.png",
    "armor/cloth_1.png", "armor/cloth_2.png",
    "armor/diamond_1.png", "armor/diamond_2.png",
    "armor/gold_1.png", "armor/gold_2.png",
    "armor/iron_1.png", "armor/iron_2.png",
    "item/boat.png", "item/book.png", "item/cart.png",
    "item/door.png", "item/sign.png",
])


def normalise_entry_name(name):
    """Lower-cased, forward-slashed, with the optional assets/ prefix
    stripped -- the form patterns and the LUT list compare against."""
    lowered = name.replace("\\", "/").lower()
    if lowered.startswith("assets/"):
        lowered = lowered[len("assets/"):]
    return lowered


def is_cpu_lut(entry_name):
    return normalise_entry_name(entry_name) in CPU_LUT_ENTRY_NAMES


def is_legacy_slim_sheet(entry_name):
    return normalise_entry_name(entry_name) in LEGACY_SLIM_SHEETS


def matches(entry_name, patterns):
    if not patterns:
        return False
    return any(fnmatch.fnmatch(normalise_entry_name(entry_name), pattern.lower())
               for pattern in patterns)


def main():
    args = sys.argv[1:]
    dry_run = False
    skip = []
    only = []
    positional = []
    i = 0
    while i < len(args):
        arg = args[i]
        if arg == "--skip":
            skip.append(args[i + 1])
            i += 2
        elif arg == "--only":
            only.append(args[i + 1])
            i += 2
        elif arg == "--dry-run":
            dry_run = True
            i += 1
        else:
            positional.append(arg)
            i += 1
    if len(positional) != 2:
        raise SystemExit(__doc__)
    in_path, out_path = positional

    if in_path.lower().endswith(".pak"):
        raise SystemExit(
            "%s is an MCPK pak; use scripts/pak_flip_mc3ds.py on paks "
            "(this tool converts .zip texture packs)" % in_path)
    if os.path.isdir(in_path):
        raise SystemExit(
            "%s is a directory; zip it first (entries at the zip root: "
            "terrain.png, gui/gui.png, pack.txt...)" % in_path)
    if not os.path.isfile(in_path):
        raise SystemExit("%s: not found" % in_path)
    if not zipfile.is_zipfile(in_path):
        raise SystemExit("%s: not a zip (a texture pack is the pack's .zip)" % in_path)

    flipped = 0
    left_as_is = 0
    copied = 0
    unflipped_display = []
    with zipfile.ZipFile(in_path, "r") as zin:
        infos = zin.infolist()
        if dry_run:
            out = None
        else:
            out = zipfile.ZipFile(out_path, "w", zipfile.ZIP_DEFLATED)
        try:
            for info in infos:
                data = zin.read(info.filename)
                entry = normalise_entry_name(info.filename)

                is_png = data.startswith(PNG_SIGNATURE)
                # --skip always exempts. The CPU colour tables are exempt
                # from the default blanket flip; a targeted --only names
                # entries explicitly and overrides that exclusion, exactly
                # like a targeted --yflip does in pak_flip_mc3ds.py (that is
                # how a pack whose LUTs shipped flipped gets repaired).
                exempt = matches(info.filename, skip)
                if not only:
                    exempt = exempt or is_cpu_lut(info.filename)
                wanted = matches(info.filename, only) if only else is_png

                if not is_png or not wanted or exempt:
                    if is_png and exempt and wanted:
                        print("skipped %s (excluded from flip)" % info.filename)
                    if out is not None:
                        info.compress_type = zipfile.ZIP_DEFLATED
                        out.writestr(info, data)
                    copied += 1
                    continue

                if dry_run:
                    print("would flip %s (Y)" % info.filename)
                    continue

                try:
                    slim = is_legacy_slim_sheet(info.filename)
                    data = flip_png(data, False, True, crop_top_half=slim)
                    flipped += 1
                    print("flipped %s (Y%s)" % (info.filename, "+crop" if slim else ""))
                except SkipPng as reason:
                    left_as_is += 1
                    unflipped_display.append("%s (%s)" % (info.filename, reason))
                    print("LEFT AS-IS %s (%s)" % (info.filename, reason))

                info.compress_type = zipfile.ZIP_DEFLATED
                out.writestr(info, data)
        finally:
            if out is not None:
                out.close()

    if dry_run:
        print("dry run: %d entries inspected, nothing written" % len(infos))
        return

    print("wrote %s: %d entries, %d flipped, %d left as-is, %d copied verbatim"
          % (out_path, len(infos), flipped, left_as_is, copied))

    # A display PNG left unflipped is a pack that renders that one texture
    # vertically mirrored on the console -- exactly the failure mode that
    # reads as "some art (often the /mob/* entity sheets) converted wrong".
    # Copying it anyway keeps the pack installable, but this must surface as
    # a loud failure, not a warning lost between progress lines: exit 1 and
    # tell the player precisely which files to re-export (Adam7 interlacing
    # is the usual cause -- Photoshop's "Interlaced" checkbox).
    if unflipped_display:
        print("")
        print("ERROR: %d PNG(s) could not be flipped and will render "
              "vertically mirrored on the 3DS:" % len(unflipped_display))
        for item in unflipped_display:
            print("  - %s" % item)
        print("re-export them non-interlaced (interlace OFF), PNG-8 or PNG-16,"
              " and run the conversion again")
        raise SystemExit(1)

    print("copy the converted zip to <SD>/opticraft/.minecraft/texturepacks/ "
          "and select it in Options > Texture Packs")


if __name__ == "__main__":
    main()
