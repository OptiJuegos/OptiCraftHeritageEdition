#pragma once

// DsAssetConvert.h -- converts a Java-Edition texture pack zip to the disk
// orientation this console renders, at install time.
//
// The 3DS backend samples texture rows in the file's own order (see
// src/3ds/render/DsTexture.cpp, the upload note: "rows are stored in the
// file's own order"), which is the Minecraft 3DS Edition convention --
// art stored upside down relative to Java. That is why the 3DS assets.pak
// ships pre-flipped (scripts/pak_flip_mc3ds.py --all is part of the
// deployment recipe). Anything downloaded from the wild arrives in Java
// orientation and must be converted before it is installed, exactly what
// scripts/texturepack_flip_3ds.py does offline for a pack zip:
//
//   * every display PNG flips on the Y axis ONLY -- the sampler mirrors
//     rows, never columns, and the player model keeps the Java UV
//     layout, so the X half of MC-3DS Edition's 180-degree skin rotation
//     is never introduced;
//   * the CPU-side colour lookup tables are NOT flipped: the game decodes
//     those into colour tables on the CPU (Minecraft::startGame,
//     CustomColorizer::loadColors) and indexes them by (row << 8) |
//     column in the Java row order, so their row order is semantic and a
//     flipped grasscolor.png makes biome colour lookups sample the wrong
//     row.
//
// Skins do NOT come through here: SkinManager::installCustomSkin writes
// its skins-dir files in this convention directly (both the main file and
// the derived retro/preview PNGs), so a downloaded skin needs no separate
// conversion pass.
//
// The exclusion list is kept in lockstep with the scripts: any name added
// to CPU_LUT_SKIP in scripts/pak_flip_mc3ds.py must reach this table too.

#include <string>

namespace DsAssetConvert
{

// Convert a Java-orientation texture pack zip in place: every display PNG
// Y-flipped, the CPU colour tables and every non-PNG entry copied verbatim.
// The archive is rewritten entry by entry (decode -> row-flip -> re-encode
// per PNG), so the pack on SD ends in the same convention the offline
// script produces. false + a reason when the archive cannot be read,
// rewritten or replaced; on failure the original file is left untouched.
bool convertTexturePackZip(const std::string &zipPath, std::string &outError);

} // namespace DsAssetConvert
