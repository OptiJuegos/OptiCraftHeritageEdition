#include "TerrainTextureManager.h"
#include "java/Arithmetic.h"

#include <algorithm>
#include <fstream>

#include "Block.h"
#include "Chunk.h"
#include "IsoImageBuffer.h"
#include "Material.h"
#include "World.h"
#include "java/BufferedImage.h"

TerrainTextureManager::TerrainTextureManager()
{
	std::fill(field_1181_a, field_1181_a + 768, 0.0f);
	std::fill(field_1180_b, field_1180_b + PREVIEW_PIXELS, 0);
	std::fill(field_1186_c, field_1186_c + PREVIEW_PIXELS, 0);
	std::fill(field_1185_d, field_1185_d + PREVIEW_PIXELS, 0);
	std::fill(field_1184_e, field_1184_e + PREVIEW_PIXELS, 0);
	std::fill(field_1183_f, field_1183_f + 34, 0);
	std::fill(field_1182_g, field_1182_g + BLOCK_TEXTURE_ENTRIES, 0);

	std::vector<int_t> ai;
	if (loadTerrainPixels(ai))
	{
		for (int_t j = 0; j < 256; j++)
		{
			int_t k = 0;
			int_t l = 0;
			int_t i1 = 0;
			int_t j1 = (j % 16) * 16;
			int_t k1 = (j / 16) * 16;
			int_t l1 = 0;
			for (int_t i2 = 0; i2 < 16; i2++)
			{
				for (int_t j2 = 0; j2 < 16; j2++)
				{
					int_t k2 = ai[j2 + j1 + (i2 + k1) * 256];
					int_t l2 = k2 >> 24 & 0xff;
					if (l2 > 128)
					{
						k += k2 >> 16 & 0xff;
						l += k2 >> 8 & 0xff;
						i1 += k2 & 0xff;
						l1++;
					}
				}
				if (l1 == 0)
					l1++;
				field_1181_a[j * 3 + 0] = (float)(k / l1);
				field_1181_a[j * 3 + 1] = (float)(l / l1);
				field_1181_a[j * 3 + 2] = (float)(i1 / l1);
			}
		}
	}

	for (int_t i = 0; i < Block::BLOCK_REGISTRY_SIZE; i++)
	{
		if (Block::blocksList[i] != nullptr)
		{
			field_1182_g[i * 3 + 0] = Block::blocksList[i]->getBlockTextureFromSide(1);
			field_1182_g[i * 3 + 1] = Block::blocksList[i]->getBlockTextureFromSide(2);
			field_1182_g[i * 3 + 2] = Block::blocksList[i]->getBlockTextureFromSide(3);
		}
	}
}

void TerrainTextureManager::renderIsoImage(IsoImageBuffer *isoimagebuffer)
{
	World *world = isoimagebuffer->worldObj;
	if (world == nullptr)
	{
		isoimagebuffer->field_1351_f = true;
		isoimagebuffer->field_1352_e = true;
		return;
	}
	int_t i = isoimagebuffer->field_1354_c * 16;
	int_t j = isoimagebuffer->field_1353_d * 16;
	int_t k = i + 16;
	int_t l = j + 16;
	Chunk *chunk = world->getChunkFromChunkCoords(isoimagebuffer->field_1354_c, isoimagebuffer->field_1353_d);
	if (chunk->isEmptyChunk())
	{
		isoimagebuffer->field_1351_f = true;
		isoimagebuffer->field_1352_e = true;
		return;
	}
	isoimagebuffer->field_1351_f = false;
	std::fill(field_1186_c, field_1186_c + PREVIEW_PIXELS, 0);
	std::fill(field_1185_d, field_1185_d + PREVIEW_PIXELS, 0);
	std::fill(field_1183_f, field_1183_f + 34, PREVIEW_HEIGHT);
	for (int_t i1 = l - 1; i1 >= j; i1--)
	{
		for (int_t j1 = k - 1; j1 >= i; j1--)
		{
			int_t k1 = j1 - i;
			int_t l1 = i1 - j;
			int_t i2 = k1 + l1;
			bool flag = true;
			for (int_t j2 = 0; j2 < 128; j2++)
			{
				int_t k2 = ((l1 - k1 - j2) + 160) - 16;
				if (k2 >= field_1183_f[i2] && k2 >= field_1183_f[i2 + 1])
					continue;
				Block *block = Block::blocksList[world->getBlockId(j1, j2, i1)];
				if (block == nullptr)
				{
					flag = false;
					continue;
				}
				if (block->blockMaterial == Material::water)
				{
					int_t l2 = world->getBlockId(j1, j2 + 1, i1);
					if (l2 != 0 && Block::blocksList[l2]->blockMaterial == Material::water)
						continue;
					float f1 = ((float)j2 / 127.0f) * 0.6f + 0.4f;
					float f2 = world->getLightBrightness(j1, j2 + 1, i1) * f1;
					if (k2 < 0 || k2 >= PREVIEW_HEIGHT)
						continue;
					int_t i4 = i2 + k2 * 32;
					if (i2 >= 0 && i2 <= 32 && field_1185_d[i4] <= j2)
					{
						field_1185_d[i4] = j2;
						field_1184_e[i4] = (int_t)(f2 * 127.0f);
					}
					if (i2 >= -1 && i2 <= 31 && field_1185_d[i4 + 1] <= j2)
					{
						field_1185_d[i4 + 1] = j2;
						field_1184_e[i4 + 1] = (int_t)(f2 * 127.0f);
					}
					flag = false;
					continue;
				}
				if (flag)
				{
					if (k2 < field_1183_f[i2])
						field_1183_f[i2] = k2;
					if (k2 < field_1183_f[i2 + 1])
						field_1183_f[i2 + 1] = k2;
				}
				float f = ((float)j2 / 127.0f) * 0.6f + 0.4f;
				if (k2 >= 0 && k2 < PREVIEW_HEIGHT)
				{
					int_t i3 = i2 + k2 * 32;
					int_t k3 = field_1182_g[block->blockID * 3 + 0];
					float f3 = (world->getLightBrightness(j1, j2 + 1, i1) * 0.8f + 0.2f) * f;
					int_t j4 = k3;
					if (i2 >= 0)
					{
						float f5 = f3;
						if (field_1186_c[i3] <= j2)
						{
							field_1186_c[i3] = j2;
							field_1180_b[i3] = 0xff000000 | (int_t)(field_1181_a[j4 * 3 + 0] * f5) << 16 | (int_t)(field_1181_a[j4 * 3 + 1] * f5) << 8 | (int_t)(field_1181_a[j4 * 3 + 2] * f5);
						}
					}
					if (i2 < 31)
					{
						float f6 = f3 * 0.9f;
						if (field_1186_c[i3 + 1] <= j2)
						{
							field_1186_c[i3 + 1] = j2;
							field_1180_b[i3 + 1] = 0xff000000 | (int_t)(field_1181_a[j4 * 3 + 0] * f6) << 16 | (int_t)(field_1181_a[j4 * 3 + 1] * f6) << 8 | (int_t)(field_1181_a[j4 * 3 + 2] * f6);
						}
					}
				}
				if (k2 < -1 || k2 >= PREVIEW_HEIGHT - 1)
					continue;
				int_t j3 = i2 + (k2 + 1) * 32;
				int_t l3 = field_1182_g[block->blockID * 3 + 1];
				float f4 = world->getLightBrightness(j1 - 1, j2, i1) * 0.8f + 0.2f;
				int_t k4 = field_1182_g[block->blockID * 3 + 2];
				float f7 = world->getLightBrightness(j1, j2, i1 + 1) * 0.8f + 0.2f;
				if (i2 >= 0)
				{
					float f8 = f4 * f * 0.6f;
					if (field_1186_c[j3] <= j2 - 1)
					{
						field_1186_c[j3] = j2 - 1;
						field_1180_b[j3] = 0xff000000 | (int_t)(field_1181_a[l3 * 3 + 0] * f8) << 16 | (int_t)(field_1181_a[l3 * 3 + 1] * f8) << 8 | (int_t)(field_1181_a[l3 * 3 + 2] * f8);
					}
				}
				if (i2 >= 31)
					continue;
				float f9 = f7 * 0.9f * f * 0.4f;
				if (field_1186_c[j3 + 1] <= j2 - 1)
				{
					field_1186_c[j3 + 1] = j2 - 1;
					field_1180_b[j3 + 1] = 0xff000000 | (int_t)(field_1181_a[k4 * 3 + 0] * f9) << 16 | (int_t)(field_1181_a[k4 * 3 + 1] * f9) << 8 | (int_t)(field_1181_a[k4 * 3 + 2] * f9);
				}
			}
		}
	}

	blendWaterPixels();
	if (isoimagebuffer->field_1348_a == nullptr)
		isoimagebuffer->field_1348_a = new BufferedImage(PREVIEW_WIDTH, PREVIEW_HEIGHT);

	std::vector<unsigned char> rgba(PREVIEW_PIXELS * 4);
	for (int_t idx = 0; idx < PREVIEW_PIXELS; idx++)
	{
		int_t argb = field_1180_b[idx];
		rgba[idx * 4 + 0] = (unsigned char)((argb >> 16) & 0xff);
		rgba[idx * 4 + 1] = (unsigned char)((argb >> 8) & 0xff);
		rgba[idx * 4 + 2] = (unsigned char)(argb & 0xff);
		rgba[idx * 4 + 3] = (unsigned char)((argb >> 24) & 0xff);
	}
	isoimagebuffer->field_1348_a->setRGB(0, 0, PREVIEW_WIDTH, PREVIEW_HEIGHT, rgba.data());
	isoimagebuffer->field_1352_e = true;
}

void TerrainTextureManager::blendWaterPixels()
{
	for (int_t i = 0; i < PREVIEW_WIDTH; i++)
	{
		for (int_t j = 0; j < PREVIEW_HEIGHT; j++)
		{
			int_t k = i + j * PREVIEW_WIDTH;
			if (field_1186_c[k] == 0)
				field_1180_b[k] = 0;
			if (field_1185_d[k] <= field_1186_c[k])
				continue;
			int_t l = field_1180_b[k] >> 24 & 0xff;
			field_1180_b[k] = ((field_1180_b[k] & 0xfefefe) >> 1) + field_1184_e[k];
			if (l < 128)
				field_1180_b[k] = 0x80000000 + field_1184_e[k] * 2;
			else
				field_1180_b[k] |= 0xff000000;
		}
	}
}

bool TerrainTextureManager::loadTerrainPixels(std::vector<int_t> &pixels)
{
	std::vector<std::string> candidates = {
		"terrain.png",
		"resources/terrain.png",
		"assets/terrain.png",
		"McBetaCpp/resources/terrain.png",
		"McBetaCpp/assets/terrain.png"
	};
	for (const std::string &candidate : candidates)
	{
		std::ifstream input(candidate, std::ios::binary);
		if (!input)
			continue;
		BufferedImage image = BufferedImage::ImageIO_read(input);
		if (image.getWidth() < 256 || image.getHeight() < 256)
			return false;
		pixels.resize(0x10000);
		const unsigned char *raw = image.getRawPixels();
		for (int_t i = 0; i < 0x10000; i++)
		{
			int_t r = raw[i * 4 + 0] & 0xff;
			int_t g = raw[i * 4 + 1] & 0xff;
			int_t b = raw[i * 4 + 2] & 0xff;
			int_t a = raw[i * 4 + 3] & 0xff;
			pixels[i] = JavaArithmetic::intFromBits((static_cast<uint_t>(a) << 24) | (static_cast<uint_t>(r) << 16) | (static_cast<uint_t>(g) << 8) | static_cast<uint_t>(b));
		}
		return true;
	}
	return false;
}
