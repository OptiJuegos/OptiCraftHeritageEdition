# PS2 Biome-Aware Lite Terrain Design

## Goal

Make the PS2 lightweight heightmap generator reflect Minecraft 1.2.5 biome elevation and variation while keeping its terrain-shaping arithmetic in `float` and preserving the existing structure and decoration pipeline.

## Architecture

The lightweight generator remains responsible only for the initial stone, water, and ice columns. It receives a 20 by 20 biome halo for each 16 by 16 chunk, blends each column's biome height parameters with a fixed 5 by 5 kernel, and feeds the blended shape into the existing all-float noise calculation. `replaceBlocksForBiome`, caves, ravines, mineshafts, villages, strongholds, and deferred PS2 population remain unchanged.

## Behavioral Requirements

- The selective PS2 defaults enable Lite terrain and float biome, cave, and ore generation without enabling reduced decoration counts.
- Ocean and river profiles remain below sea level even at the positive extreme of the Lite noise inputs.
- Plains and deserts remain comparatively gentle so village pieces can follow the ground reliably.
- Hill biomes have more relief than plains under identical noise.
- Biome shapes are blended across a two-block halo so chunk borders use the same neighboring samples.
- Existing RNG call order is unchanged.
- Existing saves are not considered terrain-compatible; validation uses a new world.

## Non-Goals

- Converting `replaceBlocksForBiome` stone noise from `double` to `float`.
- Changing biome selection, structure eligibility, structure placement, or decoration counts.
- Matching vanilla 1.2.5 terrain block-for-block.
