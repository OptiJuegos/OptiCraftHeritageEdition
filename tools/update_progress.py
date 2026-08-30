#!/usr/bin/env python3
# Regenerates McBetaCpp/PROGRESS.md by scanning the decomp vs the ported dir.
import os

base = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))            # McBetaCpp
root = os.path.abspath(os.path.join(base, ".."))                                  # project root
srcjava = os.path.join(root, "mc beta 1.7.3 decomp", "src")
tgt = os.path.join(base, "src", "net", "minecraft", "src")

alljava = sorted(f[:-5] for f in os.listdir(srcjava) if f.endswith(".java"))
files = os.listdir(tgt)
doneh = set(f[:-2] for f in files if f.endswith(".h"))
donecpp = set(f[:-4] for f in files if f.endswith(".cpp"))

hdr_only = {"IBlockAccess", "EnumMovingObjectType"}  # interfaces / enums w/ no cpp
partial = {"Block", "World"}                          # header only so far

def status(n):
    if n in partial:
        return "wip"
    if n in hdr_only and n in doneh:
        return "done"
    if n in doneh and n in donecpp:
        return "done"
    if n in doneh:
        return "wip"
    return "todo"

PREFIXES = ["BlockEntity", "Block", "ItemStack", "Item", "TileEntity", "Entity",
            "NBT", "GuiScreen", "Gui", "RenderEngine", "Render", "Model",
            "WorldChunkManager", "World", "Chunk", "MapColor", "Map", "Stat",
            "Packet", "NetworkManager", "NetServer", "NetClient", "Tessellator",
            "Texture", "Tex", "Sound", "Inventory", "Container", "Slot",
            "Crafting", "Material", "Enum", "StepSound", "Math", "Vec",
            "AxisAlignedBB", "MovingObjectPosition", "Path"]

def cat(n):
    for p in PREFIXES:
        if n.startswith(p):
            return p
    return "Misc"

from collections import defaultdict
groups = defaultdict(list)
for n in alljava:
    groups[cat(n)].append(n)

total = len(alljava)
done = sum(1 for n in alljava if status(n) == "done")
wip = sum(1 for n in alljava if status(n) == "wip")
todo = sum(1 for n in alljava if status(n) == "todo")
mark = {"done": "[x]", "wip": "[~]", "todo": "[ ]"}

order = ["Material", "StepSound", "Math", "Vec", "AxisAlignedBB",
         "MovingObjectPosition", "Enum", "Block", "Item", "ItemStack",
         "TileEntity", "Entity", "NBT", "World", "Chunk", "Container", "Slot",
         "Inventory", "Crafting", "Gui", "Render", "Model", "Tessellator",
         "Tex", "Texture", "Map", "Stat", "Sound", "Packet", "Misc"]
ordered = [g for g in order if g in groups] + [g for g in sorted(groups) if g not in order]

L = ["# McBetaCpp port progress", "",
     "1:1 port of `mc beta 1.7.3 decomp/src/*.java` -> `McBetaCpp/src/net/minecraft/src/` "
     "(flat, same names as Java incl `field_*`/`func_*`, NO stubs).", "",
     "**Total %d | done %d | wip %d | todo %d**  (%d%% done)" % (total, done, wip, todo, done * 100 // total), "",
     "Legend: `[x]` ported (.h+.cpp or .h-only interface) | `[~]` partial | `[ ]` not started", "",
     "Conventions + dependency-ordered next steps: see `src/net/minecraft/src/PORTING_PROGRESS.md`.",
     "Regenerate this file: `python tools/update_progress.py`.", ""]
for g in ordered:
    items = groups[g]
    d = sum(1 for n in items if status(n) == "done")
    L.append("## %s  (%d/%d)" % (g, d, len(items)))
    for n in items:
        L.append("- %s %s" % (mark[status(n)], n))
    L.append("")

open(os.path.join(base, "PROGRESS.md"), "w", encoding="utf-8").write("\n".join(L))
print("PROGRESS.md updated: total=%d done=%d wip=%d todo=%d" % (total, done, wip, todo))
