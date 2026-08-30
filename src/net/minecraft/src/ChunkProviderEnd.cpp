#include "ChunkProviderEnd.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "BiomeGenBase.h"
#include "Block.h"
#include "BlockSand.h"
#include "Chunk.h"
#include "MathHelper.h"
#include "World.h"
#include "WorldChunkManager.h"
#include "java/Arithmetic.h"
#include "java/Math.h"

ChunkProviderEnd::ChunkProviderEnd(World *world, long_t seed) :
	endRNG(seed), noiseGen1(endRNG, 16), noiseGen2(endRNG, 16), noiseGen3(endRNG, 8),
	noiseGen4(endRNG, 10), noiseGen5(endRNG, 16), endWorld(world)
{
}

void ChunkProviderEnd::generateTerrain(int_t chunkX, int_t chunkZ, byte_t *blocks,
                                       const std::vector<BiomeGenBase *> &)
{
	const int_t horizontalCells = 2;
	const int_t horizontalPoints = horizontalCells + 1;
	const int_t verticalPoints = 33;
	densities = initializeNoiseField(densities, JavaArithmetic::intMul(chunkX, horizontalCells), 0,
	                                 JavaArithmetic::intMul(chunkZ, horizontalCells), horizontalPoints,
	                                 verticalPoints, horizontalPoints);

	for (int_t cellX = 0; cellX < horizontalCells; ++cellX)
	{
		for (int_t cellZ = 0; cellZ < horizontalCells; ++cellZ)
		{
			for (int_t cellY = 0; cellY < 32; ++cellY)
			{
				const terrain_noise_real_t yStep = static_cast<terrain_noise_real_t>(0.25);
				terrain_noise_real_t d00 = densities[((cellX + 0) * horizontalPoints + cellZ + 0) * verticalPoints + cellY];
				terrain_noise_real_t d01 = densities[((cellX + 0) * horizontalPoints + cellZ + 1) * verticalPoints + cellY];
				terrain_noise_real_t d10 = densities[((cellX + 1) * horizontalPoints + cellZ + 0) * verticalPoints + cellY];
				terrain_noise_real_t d11 = densities[((cellX + 1) * horizontalPoints + cellZ + 1) * verticalPoints + cellY];
				const terrain_noise_real_t dy00 = (densities[((cellX + 0) * horizontalPoints + cellZ + 0) * verticalPoints + cellY + 1] - d00) * yStep;
				const terrain_noise_real_t dy01 = (densities[((cellX + 0) * horizontalPoints + cellZ + 1) * verticalPoints + cellY + 1] - d01) * yStep;
				const terrain_noise_real_t dy10 = (densities[((cellX + 1) * horizontalPoints + cellZ + 0) * verticalPoints + cellY + 1] - d10) * yStep;
				const terrain_noise_real_t dy11 = (densities[((cellX + 1) * horizontalPoints + cellZ + 1) * verticalPoints + cellY + 1] - d11) * yStep;

				for (int_t subY = 0; subY < 4; ++subY)
				{
					const terrain_noise_real_t xStep = static_cast<terrain_noise_real_t>(0.125);
					terrain_noise_real_t x0 = d00;
					terrain_noise_real_t x1 = d01;
					const terrain_noise_real_t dx0 = (d10 - d00) * xStep;
					const terrain_noise_real_t dx1 = (d11 - d01) * xStep;

					for (int_t subX = 0; subX < 8; ++subX)
					{
						int_t index = (subX + cellX * 8) << 11 | (cellZ * 8) << 7 | cellY * 4 + subY;
						const int_t zStride = 128;
						const terrain_noise_real_t zStep = static_cast<terrain_noise_real_t>(0.125);
						terrain_noise_real_t density = x0;
						const terrain_noise_real_t dz = (x1 - x0) * zStep;

						for (int_t subZ = 0; subZ < 8; ++subZ)
						{
							blocks[index] = density > static_cast<terrain_noise_real_t>(0.0) ? (byte_t)Block::whiteStone->blockID : 0;
							index += zStride;
							density += dz;
						}
						x0 += dx0;
						x1 += dx1;
					}

					d00 += dy00;
					d01 += dy01;
					d10 += dy10;
					d11 += dy11;
				}
			}
		}
	}
}

void ChunkProviderEnd::replaceBlocksForBiome(int_t, int_t, byte_t *blocks,
                                             const std::vector<BiomeGenBase *> &)
{
	for (int_t x = 0; x < 16; ++x)
	{
		for (int_t z = 0; z < 16; ++z)
		{
			const int_t depth = 1;
			int_t remaining = -1;
			byte_t top = (byte_t)Block::whiteStone->blockID;
			byte_t filler = (byte_t)Block::whiteStone->blockID;
			for (int_t y = 127; y >= 0; --y)
			{
				const int_t index = (z * 16 + x) * 128 + y;
				const int_t id = blocks[index] & 255;
				if (id == 0)
				{
					remaining = -1;
				}
				else if (id == Block::stone->blockID)
				{
					if (remaining == -1)
					{
						if (depth <= 0)
						{
							top = 0;
							filler = (byte_t)Block::whiteStone->blockID;
						}
						remaining = depth;
						blocks[index] = top;
					}
					else if (remaining > 0)
					{
						--remaining;
						blocks[index] = filler;
					}
				}
			}
		}
	}
}

Chunk *ChunkProviderEnd::prepareChunk(int_t x, int_t z)
{
	return provideChunk(x, z);
}

Chunk *ChunkProviderEnd::loadChunk(int_t x, int_t z)
{
	return provideChunk(x, z);
}

Chunk *ChunkProviderEnd::provideChunk(int_t x, int_t z)
{
	const long_t seedX = JavaArithmetic::longMul((long_t)x, 341873128712LL);
	const long_t seedZ = JavaArithmetic::longMul((long_t)z, 132897987541LL);
	endRNG.setSeed(JavaArithmetic::longAdd(seedX, seedZ));

	std::vector<byte_t> blocks(32768, 0);
	endWorld->getWorldChunkManager()->loadBlockGeneratorData(
		biomesForGeneration, JavaArithmetic::intMul(x, 16), JavaArithmetic::intMul(z, 16), 16, 16);
	generateTerrain(x, z, blocks.data(), biomesForGeneration);
	replaceBlocksForBiome(x, z, blocks.data(), biomesForGeneration);

	Chunk *chunk = new Chunk(endWorld, blocks, x, z);
	std::vector<byte_t> &biomeArray = chunk->getBiomeArray();
	for (std::size_t i = 0; i < biomeArray.size() && i < biomesForGeneration.size(); ++i)
		biomeArray[i] = biomesForGeneration[i] == nullptr ? 0 : (byte_t)biomesForGeneration[i]->biomeID;
	chunk->generateSkylightMap();
	return chunk;
}

TerrainNoiseBuffer &ChunkProviderEnd::initializeNoiseField(TerrainNoiseBuffer &values,
	int_t x, int_t y, int_t z, int_t sizeX, int_t sizeY, int_t sizeZ)
{
	const std::size_t requiredSize = checkedNoiseVolumeSize(sizeX, sizeY, sizeZ);
	if (values.size() != requiredSize)
		values.assign(requiredSize, 0.0);
	else
		std::fill(values.begin(), values.end(), 0.0);

#if PLATFORM_CONSOLE_LOW
	terrain_noise_real_t scaleXZ = static_cast<terrain_noise_real_t>(684.412f);
	const terrain_noise_real_t scaleY = static_cast<terrain_noise_real_t>(684.412f);
	noiseData4 = noiseGen4.getBiomeGenForCoordsFloat(noiseData4, x, z, sizeX, sizeZ, 1.121f, 1.121f, 0.5f);
	noiseData5 = noiseGen5.getBiomeGenForCoordsFloat(noiseData5, x, z, sizeX, sizeZ, 200.0f, 200.0f, 0.5f);
	scaleXZ *= static_cast<terrain_noise_real_t>(2.0f);
	const terrain_coord_real_t noiseX = static_cast<terrain_coord_real_t>(x);
	const terrain_coord_real_t noiseY = static_cast<terrain_coord_real_t>(y);
	const terrain_coord_real_t noiseZ = static_cast<terrain_coord_real_t>(z);
	noiseData1 = noiseGen3.generateNoiseOctavesFloat(noiseData1, noiseX, noiseY, noiseZ, sizeX, sizeY, sizeZ,
	                                                 scaleXZ / 80.0f, scaleY / 160.0f, scaleXZ / 80.0f);
	noiseData2 = noiseGen1.generateNoiseOctavesFloat(noiseData2, noiseX, noiseY, noiseZ, sizeX, sizeY, sizeZ,
	                                                 scaleXZ, scaleY, scaleXZ);
	noiseData3 = noiseGen2.generateNoiseOctavesFloat(noiseData3, noiseX, noiseY, noiseZ, sizeX, sizeY, sizeZ,
	                                                 scaleXZ, scaleY, scaleXZ);
#else
	terrain_noise_real_t scaleXZ = static_cast<terrain_noise_real_t>(684.412);
	const terrain_noise_real_t scaleY = static_cast<terrain_noise_real_t>(684.412);
	noiseData4 = noiseGen4.getBiomeGenForCoords(noiseData4, x, z, sizeX, sizeZ, 1.121, 1.121, 0.5);
	noiseData5 = noiseGen5.getBiomeGenForCoords(noiseData5, x, z, sizeX, sizeZ, 200.0, 200.0, 0.5);
	scaleXZ *= static_cast<terrain_noise_real_t>(2.0);
	noiseData1 = noiseGen3.generateNoiseOctaves(noiseData1, x, y, z, sizeX, sizeY, sizeZ,
	                                            scaleXZ / static_cast<terrain_noise_real_t>(80.0), scaleY / static_cast<terrain_noise_real_t>(160.0), scaleXZ / static_cast<terrain_noise_real_t>(80.0));
	noiseData2 = noiseGen1.generateNoiseOctaves(noiseData2, x, y, z, sizeX, sizeY, sizeZ,
	                                            scaleXZ, scaleY, scaleXZ);
	noiseData3 = noiseGen2.generateNoiseOctaves(noiseData3, x, y, z, sizeX, sizeY, sizeZ,
	                                            scaleXZ, scaleY, scaleXZ);
#endif

	int_t noiseIndex = 0;
	int_t columnIndex = 0;
	for (int_t localX = 0; localX < sizeX; ++localX)
	{
		for (int_t localZ = 0; localZ < sizeZ; ++localZ)
		{
			terrain_noise_real_t var16 = (noiseData4[columnIndex] + static_cast<terrain_noise_real_t>(256.0)) / static_cast<terrain_noise_real_t>(512.0);
			if (var16 > static_cast<terrain_noise_real_t>(1.0))
				var16 = static_cast<terrain_noise_real_t>(1.0);
			terrain_noise_real_t var18 = noiseData5[columnIndex] / static_cast<terrain_noise_real_t>(8000.0);
			if (var18 < static_cast<terrain_noise_real_t>(0.0))
				var18 = -var18 * static_cast<terrain_noise_real_t>(0.3);
			var18 = var18 * static_cast<terrain_noise_real_t>(3.0) - static_cast<terrain_noise_real_t>(2.0);

			const float worldX = static_cast<float>(JavaArithmetic::intAdd(localX, x));
			const float worldZ = static_cast<float>(JavaArithmetic::intAdd(localZ, z));
			float islandHeight = 100.0f - MathHelper::sqrt_float(worldX * worldX + worldZ * worldZ) * 8.0f;
			if (islandHeight > 80.0f)
				islandHeight = 80.0f;
			if (islandHeight < -100.0f)
				islandHeight = -100.0f;

			if (var18 > static_cast<terrain_noise_real_t>(1.0))
				var18 = static_cast<terrain_noise_real_t>(1.0);
			var18 /= static_cast<terrain_noise_real_t>(8.0);
			var18 = static_cast<terrain_noise_real_t>(0.0);
			if (var16 < static_cast<terrain_noise_real_t>(0.0))
				var16 = static_cast<terrain_noise_real_t>(0.0);
			var16 += static_cast<terrain_noise_real_t>(0.5);
			var18 = var18 * static_cast<terrain_noise_real_t>(sizeY) / static_cast<terrain_noise_real_t>(16.0);
			++columnIndex;
			const terrain_noise_real_t centerY = static_cast<terrain_noise_real_t>(sizeY) / static_cast<terrain_noise_real_t>(2.0);

			for (int_t localY = 0; localY < sizeY; ++localY)
			{
				terrain_noise_real_t density;
				const terrain_noise_real_t verticalDistance = std::abs((static_cast<terrain_noise_real_t>(localY) - centerY) * static_cast<terrain_noise_real_t>(8.0) / var16);
				const terrain_noise_real_t low = noiseData2[noiseIndex] / static_cast<terrain_noise_real_t>(512.0);
				const terrain_noise_real_t high = noiseData3[noiseIndex] / static_cast<terrain_noise_real_t>(512.0);
				const terrain_noise_real_t blend = (noiseData1[noiseIndex] / static_cast<terrain_noise_real_t>(10.0) + static_cast<terrain_noise_real_t>(1.0)) / static_cast<terrain_noise_real_t>(2.0);
				if (blend < static_cast<terrain_noise_real_t>(0.0))
					density = low;
				else if (blend > static_cast<terrain_noise_real_t>(1.0))
					density = high;
				else
					density = low + (high - low) * blend;
				density -= static_cast<terrain_noise_real_t>(8.0);
				density += islandHeight;

				const int_t topFade = 2;
				if (localY > sizeY / 2 - topFade)
				{
					terrain_noise_real_t fade = static_cast<terrain_noise_real_t>(static_cast<float>(localY - (sizeY / 2 - topFade)) / 64.0f);
					if (fade < static_cast<terrain_noise_real_t>(0.0)) fade = static_cast<terrain_noise_real_t>(0.0);
					if (fade > static_cast<terrain_noise_real_t>(1.0)) fade = static_cast<terrain_noise_real_t>(1.0);
					density = density * (static_cast<terrain_noise_real_t>(1.0) - fade) + static_cast<terrain_noise_real_t>(-3000.0) * fade;
				}

				if (localY < 8)
				{
					const terrain_noise_real_t fade = static_cast<terrain_noise_real_t>(static_cast<float>(8 - localY) / 7.0f);
					density = density * (static_cast<terrain_noise_real_t>(1.0) - fade) + static_cast<terrain_noise_real_t>(-30.0) * fade;
				}

				(void)verticalDistance;
				values[noiseIndex++] = density;
			}
		}
	}
	return values;
}

bool ChunkProviderEnd::chunkExists(int_t, int_t)
{
	return true;
}

void ChunkProviderEnd::populate(IChunkProvider *, int_t x, int_t z)
{
	BlockSand::fallInstantly = true;
	const int_t blockX = JavaArithmetic::intMul(x, 16);
	const int_t blockZ = JavaArithmetic::intMul(z, 16);
	BiomeGenBase *biome = endWorld->getBiomeGenForCoords(JavaArithmetic::intAdd(blockX, 16), JavaArithmetic::intAdd(blockZ, 16));
	if (biome != nullptr)
		biome->decorate(endWorld, endWorld->rand, blockX, blockZ);
	BlockSand::fallInstantly = false;
}

bool ChunkProviderEnd::saveChunks(bool, IProgressUpdate *) { return true; }
bool ChunkProviderEnd::unload100OldestChunks() { return false; }
bool ChunkProviderEnd::canSave() { return true; }
jstring ChunkProviderEnd::makeString() { return "RandomLevelSource"; }

std::vector<SpawnListEntry> *ChunkProviderEnd::getPossibleCreatures(const EnumCreatureType &type, int_t x, int_t, int_t z)
{
	BiomeGenBase *biome = endWorld->getBiomeGenForCoords(x, z);
	return biome == nullptr ? nullptr : biome->getSpawnableList(type);
}

ChunkPosition *ChunkProviderEnd::findClosestStructure(World *, const jstring &, int_t, int_t, int_t)
{
	return nullptr;
}
