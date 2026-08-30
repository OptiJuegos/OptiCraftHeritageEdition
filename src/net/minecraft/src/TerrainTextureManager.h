#pragma once

#include <vector>

#include "java/Type.h"

class IsoImageBuffer;

// net.minecraft.src.TerrainTextureManager
class TerrainTextureManager
{
public:
	TerrainTextureManager();

	void renderIsoImage(IsoImageBuffer *isoimagebuffer);

private:
	void blendWaterPixels();
	bool loadTerrainPixels(std::vector<int_t> &pixels);

	static constexpr int_t PREVIEW_WIDTH = 32;
	static constexpr int_t PREVIEW_HEIGHT = 544;
	static constexpr int_t PREVIEW_PIXELS = PREVIEW_WIDTH * PREVIEW_HEIGHT;
	static constexpr int_t BLOCK_TEXTURE_ENTRIES = 4096 * 3;

	float field_1181_a[768];
	int_t field_1180_b[PREVIEW_PIXELS];
	int_t field_1186_c[PREVIEW_PIXELS];
	int_t field_1185_d[PREVIEW_PIXELS];
	int_t field_1184_e[PREVIEW_PIXELS];
	int_t field_1183_f[34];
	int_t field_1182_g[BLOCK_TEXTURE_ENTRIES];
};
