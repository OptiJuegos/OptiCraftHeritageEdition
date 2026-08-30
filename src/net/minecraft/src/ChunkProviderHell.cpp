#include "java/Math.h"
#include "ChunkProviderHell.h"
#include "java/Arithmetic.h"

#include <cmath>
#include <stdexcept>

#include "World.h"
#include "WorldChunkManager.h"
#include "Chunk.h"
#include "Block.h"
#include "MapGenCavesHell.h"
#include "MapGenNetherBridge.h"
#include "WorldGenHellLava.h"
#include "WorldGenFire.h"
#include "WorldGenGlowStone1.h"
#include "WorldGenGlowStone2.h"
#include "WorldGenFlowers.h"
#include "BlockSand.h"
#include "BlockFlower.h"
#include "BiomeGenBase.h"
#include "EnumCreatureType.h"

ChunkProviderHell::ChunkProviderHell(World *world, long_t l)
	: hellRNG(l)
	, field_4169_i(hellRNG, 16)
	, field_4168_j(hellRNG, 16)
	, field_4167_k(hellRNG, 8)
	, field_4166_l(hellRNG, 4)
	, field_4165_m(hellRNG, 4)
	, field_4177_a(hellRNG, 10)
	, field_4176_b(hellRNG, 16)
	, worldObj(world)
	, caveGenerator(nullptr)
	, netherBridgeGenerator(nullptr)
{
	field_4162_p.resize(256);
	field_4161_q.resize(256);
	field_4160_r.resize(256);
	caveGenerator = new MapGenCavesHell();
	netherBridgeGenerator = new MapGenNetherBridge();
}

ChunkProviderHell::~ChunkProviderHell()
{
	delete caveGenerator;
	delete netherBridgeGenerator;
}

void ChunkProviderHell::generateHellTerrain(int_t i, int_t j, byte_t *abyte0)
{
	const int_t byte0 = 4;
	const int_t byte1 = 32;
	int_t k = byte0 + 1;
	const int_t byte2 = 17;
	int_t l = byte0 + 1;
	field_4163_o = generateNoiseField(field_4163_o, i * byte0, 0, j * byte0, k, byte2, l);

	for (int_t i1 = 0; i1 < byte0; i1++)
	{
		for (int_t j1 = 0; j1 < byte0; j1++)
		{
			for (int_t k1 = 0; k1 < 16; k1++)
			{
				const terrain_noise_real_t d = static_cast<terrain_noise_real_t>(0.125f);
				terrain_noise_real_t d1 = field_4163_o[((i1 + 0) * l + (j1 + 0)) * byte2 + (k1 + 0)];
				terrain_noise_real_t d2 = field_4163_o[((i1 + 0) * l + (j1 + 1)) * byte2 + (k1 + 0)];
				terrain_noise_real_t d3 = field_4163_o[((i1 + 1) * l + (j1 + 0)) * byte2 + (k1 + 0)];
				terrain_noise_real_t d4 = field_4163_o[((i1 + 1) * l + (j1 + 1)) * byte2 + (k1 + 0)];
				terrain_noise_real_t d5 = (field_4163_o[((i1 + 0) * l + (j1 + 0)) * byte2 + (k1 + 1)] - d1) * d;
				terrain_noise_real_t d6 = (field_4163_o[((i1 + 0) * l + (j1 + 1)) * byte2 + (k1 + 1)] - d2) * d;
				terrain_noise_real_t d7 = (field_4163_o[((i1 + 1) * l + (j1 + 0)) * byte2 + (k1 + 1)] - d3) * d;
				terrain_noise_real_t d8 = (field_4163_o[((i1 + 1) * l + (j1 + 1)) * byte2 + (k1 + 1)] - d4) * d;

				for (int_t l1 = 0; l1 < 8; l1++)
				{
					const terrain_noise_real_t d9 = static_cast<terrain_noise_real_t>(0.25f);
					terrain_noise_real_t d10 = d1;
					terrain_noise_real_t d11 = d2;
					const terrain_noise_real_t d12 = (d3 - d1) * d9;
					const terrain_noise_real_t d13 = (d4 - d2) * d9;

					for (int_t i2 = 0; i2 < 4; i2++)
					{
						int_t j2 = (i2 + i1 * 4) << 11 | (0 + j1 * 4) << 7 | (k1 * 8 + l1);
						const int_t stride = 128;
						const terrain_noise_real_t d14 = static_cast<terrain_noise_real_t>(0.25f);
						terrain_noise_real_t d15 = d10;
						const terrain_noise_real_t d16 = (d11 - d10) * d14;

						for (int_t k2 = 0; k2 < 4; k2++)
						{
							int_t l2 = 0;
							if (k1 * 8 + l1 < byte1)
								l2 = Block::lavaStill->blockID;
							if (d15 > static_cast<terrain_noise_real_t>(0.0f))
								l2 = Block::netherrack->blockID;
							abyte0[j2] = (byte_t)l2;
							j2 += stride;
							d15 += d16;
						}
						d10 += d12;
						d11 += d13;
					}
					d1 += d5;
					d2 += d6;
					d3 += d7;
					d4 += d8;
				}
			}
		}
	}
}

void ChunkProviderHell::replaceHellBlocks(int_t i, int_t j, byte_t *abyte0)
{
	const int_t byte0 = 64;
#if PLATFORM_CONSOLE_LOW
	const terrain_coord_real_t d = static_cast<terrain_coord_real_t>(0.03125f);
	const terrain_coord_real_t blockX = static_cast<terrain_coord_real_t>(JavaArithmetic::intMul(i, 16));
	const terrain_coord_real_t blockZ = static_cast<terrain_coord_real_t>(JavaArithmetic::intMul(j, 16));
	field_4162_p = field_4166_l.generateNoiseOctavesFloat(field_4162_p, blockX, blockZ, 0.0f, 16, 16, 1, d, d, 1.0f);
	field_4161_q = field_4166_l.generateNoiseOctavesFloat(field_4161_q, blockX, 109.0f, blockZ, 16, 1, 16, d, 1.0f, d);
	field_4160_r = field_4165_m.generateNoiseOctavesFloat(field_4160_r, blockX, blockZ, 0.0f, 16, 16, 1, d * 2.0f, d * 2.0f, d * 2.0f);
#else
	double d = 0.03125;
	field_4162_p = field_4166_l.generateNoiseOctaves(field_4162_p, (double)JavaArithmetic::intMul(i, 16), (double)JavaArithmetic::intMul(j, 16), 0.0, 16, 16, 1, d, d, 1.0);
	field_4161_q = field_4166_l.generateNoiseOctaves(field_4161_q, (double)JavaArithmetic::intMul(i, 16), 109.0, (double)JavaArithmetic::intMul(j, 16), 16, 1, 16, d, 1.0, d);
	field_4160_r = field_4165_m.generateNoiseOctaves(field_4160_r, (double)JavaArithmetic::intMul(i, 16), (double)JavaArithmetic::intMul(j, 16), 0.0, 16, 16, 1, d * 2.0, d * 2.0, d * 2.0);
#endif

	for (int_t k = 0; k < 16; k++)
	{
		for (int_t l = 0; l < 16; l++)
		{
#if PLATFORM_CONSOLE_LOW
			const std::size_t surfaceIndex = static_cast<std::size_t>(k + l * 16);
			const terrain_noise_real_t soulSandRandom = static_cast<terrain_noise_real_t>(hellRNG.nextDoubleFloat());
			const terrain_noise_real_t gravelRandom = static_cast<terrain_noise_real_t>(hellRNG.nextDoubleFloat());
			const terrain_noise_real_t depthRandom = static_cast<terrain_noise_real_t>(hellRNG.nextDoubleFloat());
			bool flag = field_4162_p[surfaceIndex]
				+ soulSandRandom * static_cast<terrain_noise_real_t>(0.2f) > static_cast<terrain_noise_real_t>(0.0f);
			bool flag1 = field_4161_q[surfaceIndex]
				+ gravelRandom * static_cast<terrain_noise_real_t>(0.2f) > static_cast<terrain_noise_real_t>(0.0f);
			int_t i1 = JavaArithmetic::floatToInt(
				field_4160_r[surfaceIndex] / static_cast<terrain_noise_real_t>(3.0f)
				+ static_cast<terrain_noise_real_t>(3.0f)
				+ depthRandom * static_cast<terrain_noise_real_t>(0.25f));
#else
			bool flag  = field_4162_p[k + l * 16] + hellRNG.nextDouble() * 0.2 > 0.0;
			bool flag1 = field_4161_q[k + l * 16] + hellRNG.nextDouble() * 0.2 > 0.0;
			int_t i1 = JavaArithmetic::doubleToInt(
				field_4160_r[k + l * 16] / 3.0 + 3.0 + hellRNG.nextDouble() * 0.25);
#endif
			int_t j1 = -1;
			byte_t byte1 = (byte_t)Block::netherrack->blockID;
			byte_t byte2 = (byte_t)Block::netherrack->blockID;

			for (int_t k1 = 127; k1 >= 0; k1--)
			{
				int_t l1 = (l * 16 + k) * 128 + k1;
#if PLATFORM_CONSOLE_LOW
				if (k1 >= 127 - hellRNG.nextInt5())
#else
				if (k1 >= 127 - hellRNG.nextInt(5))
#endif
				{
					abyte0[l1] = (byte_t)Block::bedrock->blockID;
					continue;
				}
#if PLATFORM_CONSOLE_LOW
				if (k1 <= hellRNG.nextInt5())
#else
				if (k1 <= 0 + hellRNG.nextInt(5))
#endif
				{
					abyte0[l1] = (byte_t)Block::bedrock->blockID;
					continue;
				}
				byte_t byte3 = abyte0[l1];
				if (byte3 == 0) { j1 = -1; continue; }
				if (byte3 != (byte_t)Block::netherrack->blockID) continue;

				if (j1 == -1)
				{
					if (i1 <= 0)
					{
						byte1 = 0;
						byte2 = (byte_t)Block::netherrack->blockID;
					}
					else if (k1 >= byte0 - 4 && k1 <= byte0 + 1)
					{
						byte1 = (byte_t)Block::netherrack->blockID;
						byte2 = (byte_t)Block::netherrack->blockID;
						if (flag1) { byte1 = (byte_t)Block::gravel->blockID; }
						if (flag1) { byte2 = (byte_t)Block::netherrack->blockID; }
						if (flag)  { byte1 = (byte_t)Block::slowSand->blockID; }
						if (flag)  { byte2 = (byte_t)Block::slowSand->blockID; }
					}
					if (k1 < byte0 && byte1 == 0)
						byte1 = (byte_t)Block::lavaStill->blockID;
					j1 = i1;
					if (k1 >= byte0 - 1)
						abyte0[l1] = byte1;
					else
						abyte0[l1] = byte2;
					continue;
				}
				if (j1 > 0)
				{
					j1--;
					abyte0[l1] = byte2;
				}
			}
		}
	}
}

Chunk *ChunkProviderHell::prepareChunk(int_t i, int_t j)
{
	return provideChunk(i, j);
}

Chunk *ChunkProviderHell::provideChunk(int_t i, int_t j)
{
	const ulong_t chunkSeedBits = static_cast<ulong_t>(static_cast<long_t>(i)) * 0x4f9939f508ULL
	                            + static_cast<ulong_t>(static_cast<long_t>(j)) * 0x1ef1565bd5ULL;
	hellRNG.setSeed(JavaArithmetic::longFromBits(chunkSeedBits));
	std::vector<byte_t> abyte0(32768, 0);
	generateHellTerrain(i, j, abyte0.data());
	replaceHellBlocks(i, j, abyte0.data());
	caveGenerator->generate(this, worldObj, i, j, abyte0.data());
	netherBridgeGenerator->generate(this, worldObj, i, j, abyte0.data());
	Chunk *chunk = new Chunk(worldObj, abyte0, i, j);
	worldObj->getWorldChunkManager()->loadBlockGeneratorData(
		biomesForGeneration, JavaArithmetic::intMul(i, 16), JavaArithmetic::intMul(j, 16), 16, 16);
	std::vector<byte_t> &biomeArray = chunk->getBiomeArray();
	for (std::size_t index = 0; index < biomeArray.size() && index < biomesForGeneration.size(); ++index)
	{
		if (biomesForGeneration[index] != nullptr)
			biomeArray[index] = static_cast<byte_t>(biomesForGeneration[index]->biomeID);
	}
	chunk->resetRelightChecks();
	return chunk;
}

TerrainNoiseBuffer &ChunkProviderHell::generateNoiseField(TerrainNoiseBuffer &ad,
	int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1)
{
	if (ad.empty())
	{
		const std::size_t noiseCount = checkedNoiseVolumeSize(l, i1, j1);
		if (noiseCount == 0)
			throw std::length_error("Invalid Nether noise field size");
		ad.resize(noiseCount);
	}

#if PLATFORM_CONSOLE_LOW
	const terrain_coord_real_t d = static_cast<terrain_coord_real_t>(684.412f);
	const terrain_coord_real_t d1 = static_cast<terrain_coord_real_t>(2053.236f);
	const terrain_coord_real_t noiseX = static_cast<terrain_coord_real_t>(i);
	const terrain_coord_real_t noiseY = static_cast<terrain_coord_real_t>(j);
	const terrain_coord_real_t noiseZ = static_cast<terrain_coord_real_t>(k);
	field_4172_f = field_4177_a.generateNoiseOctavesFloat(field_4172_f, noiseX, noiseY, noiseZ, l, 1, j1, 1.0f, 0.0f, 1.0f);
	field_4171_g = field_4176_b.generateNoiseOctavesFloat(field_4171_g, noiseX, noiseY, noiseZ, l, 1, j1, 100.0f, 0.0f, 100.0f);
	field_4175_c = field_4167_k.generateNoiseOctavesFloat(field_4175_c, noiseX, noiseY, noiseZ, l, i1, j1, d / 80.0f, d1 / 60.0f, d / 80.0f);
	field_4174_d = field_4169_i.generateNoiseOctavesFloat(field_4174_d, noiseX, noiseY, noiseZ, l, i1, j1, d, d1, d);
	field_4173_e = field_4168_j.generateNoiseOctavesFloat(field_4173_e, noiseX, noiseY, noiseZ, l, i1, j1, d, d1, d);
#else
	const double d  = 684.412;
	const double d1 = 2053.236;
	field_4172_f = field_4177_a.generateNoiseOctaves(field_4172_f, (double)i, (double)j, (double)k, l, 1, j1, 1.0, 0.0, 1.0);
	field_4171_g = field_4176_b.generateNoiseOctaves(field_4171_g, (double)i, (double)j, (double)k, l, 1, j1, 100.0, 0.0, 100.0);
	field_4175_c = field_4167_k.generateNoiseOctaves(field_4175_c, (double)i, (double)j, (double)k, l, i1, j1, d / 80.0, d1 / 60.0, d / 80.0);
	field_4174_d = field_4169_i.generateNoiseOctaves(field_4174_d, (double)i, (double)j, (double)k, l, i1, j1, d, d1, d);
	field_4173_e = field_4168_j.generateNoiseOctaves(field_4173_e, (double)i, (double)j, (double)k, l, i1, j1, d, d1, d);
#endif

	int_t k1 = 0;
	int_t l1 = 0;

	std::vector<terrain_noise_real_t> ad1(i1);
	for (int_t i2 = 0; i2 < i1; i2++)
	{
#if PLATFORM_CONSOLE_LOW
		ad1[i2] = std::cos((static_cast<float>(i2) * 3.1415927f * 6.0f) / static_cast<float>(i1)) * 2.0f;
		terrain_noise_real_t d2 = static_cast<float>(i2);
#else
		ad1[i2] = JavaMath::cos(((double)i2 * 3.1415926535897931 * 6.0) / (double)i1) * 2.0;
		terrain_noise_real_t d2 = static_cast<double>(i2);
#endif
		if (i2 > i1 / 2) d2 = static_cast<terrain_noise_real_t>(i1 - 1 - i2);
		if (d2 < static_cast<terrain_noise_real_t>(4.0))
		{
			d2 = static_cast<terrain_noise_real_t>(4.0) - d2;
			ad1[i2] -= d2 * d2 * d2 * static_cast<terrain_noise_real_t>(10.0);
		}
	}

	for (int_t j2 = 0; j2 < l; j2++)
	{
		for (int_t k2 = 0; k2 < j1; k2++)
		{
			terrain_noise_real_t d3 = (field_4172_f[l1] + static_cast<terrain_noise_real_t>(256.0)) / static_cast<terrain_noise_real_t>(512.0);
			if (d3 > static_cast<terrain_noise_real_t>(1.0)) d3 = static_cast<terrain_noise_real_t>(1.0);
			terrain_noise_real_t d4 = static_cast<terrain_noise_real_t>(0.0);
			terrain_noise_real_t d5 = field_4171_g[l1] / static_cast<terrain_noise_real_t>(8000.0);
			if (d5 < static_cast<terrain_noise_real_t>(0.0)) d5 = -d5;
			d5 = d5 * static_cast<terrain_noise_real_t>(3.0) - static_cast<terrain_noise_real_t>(3.0);
			if (d5 < static_cast<terrain_noise_real_t>(0.0))
			{
				d5 /= static_cast<terrain_noise_real_t>(2.0);
				if (d5 < static_cast<terrain_noise_real_t>(-1.0)) d5 = static_cast<terrain_noise_real_t>(-1.0);
				d5 /= static_cast<terrain_noise_real_t>(1.4);
				d5 /= static_cast<terrain_noise_real_t>(2.0);
				d3 = static_cast<terrain_noise_real_t>(0.0);
			}
			else
			{
				if (d5 > static_cast<terrain_noise_real_t>(1.0)) d5 = static_cast<terrain_noise_real_t>(1.0);
				d5 /= static_cast<terrain_noise_real_t>(6.0);
			}
			d3 += static_cast<terrain_noise_real_t>(0.5);
			d5 = (d5 * static_cast<terrain_noise_real_t>(i1)) / static_cast<terrain_noise_real_t>(16.0);
			l1++;

			for (int_t l2 = 0; l2 < i1; l2++)
			{
				terrain_noise_real_t d6 = static_cast<terrain_noise_real_t>(0.0);
				terrain_noise_real_t d7 = ad1[l2];
				terrain_noise_real_t d8 = field_4174_d[k1] / static_cast<terrain_noise_real_t>(512.0);
				terrain_noise_real_t d9 = field_4173_e[k1] / static_cast<terrain_noise_real_t>(512.0);
				terrain_noise_real_t d10 = (field_4175_c[k1] / static_cast<terrain_noise_real_t>(10.0) + static_cast<terrain_noise_real_t>(1.0)) / static_cast<terrain_noise_real_t>(2.0);
				if (d10 < static_cast<terrain_noise_real_t>(0.0))       d6 = d8;
				else if (d10 > static_cast<terrain_noise_real_t>(1.0))  d6 = d9;
				else                  d6 = d8 + (d9 - d8) * d10;
				d6 -= d7;
				if (l2 > i1 - 4)
				{
					terrain_noise_real_t d11 = (float)(l2 - (i1 - 4)) / 3.0f;
					d6 = d6 * (static_cast<terrain_noise_real_t>(1.0) - d11) + static_cast<terrain_noise_real_t>(-10.0) * d11;
				}
				if (static_cast<terrain_noise_real_t>(l2) < d4)
				{
					terrain_noise_real_t d12 = (d4 - static_cast<terrain_noise_real_t>(l2)) / static_cast<terrain_noise_real_t>(4.0);
					if (d12 < static_cast<terrain_noise_real_t>(0.0)) d12 = static_cast<terrain_noise_real_t>(0.0);
					if (d12 > static_cast<terrain_noise_real_t>(1.0)) d12 = static_cast<terrain_noise_real_t>(1.0);
					d6 = d6 * (static_cast<terrain_noise_real_t>(1.0) - d12) + static_cast<terrain_noise_real_t>(-10.0) * d12;
				}
				ad[k1] = static_cast<terrain_noise_real_t>(d6);
				k1++;
			}
		}
	}
	return ad;
}

bool ChunkProviderHell::chunkExists(int_t i, int_t j)
{
	return true;
}

void ChunkProviderHell::populate(IChunkProvider *ichunkprovider, int_t i, int_t j)
{
	BlockSand::fallInstantly = true;
	int_t k = JavaArithmetic::intMul(i, 16);
	int_t l = JavaArithmetic::intMul(j, 16);
	netherBridgeGenerator->generateStructuresInChunk(worldObj, hellRNG, i, j);

	for (int_t i1 = 0; i1 < 8; i1++)
	{
		int_t k1 = JavaArithmetic::intAdd(JavaArithmetic::intAdd(k, hellRNG.nextInt(16)), 8);
		int_t i3 = hellRNG.nextInt(120) + 4;
		int_t k4 = JavaArithmetic::intAdd(JavaArithmetic::intAdd(l, hellRNG.nextInt(16)), 8);
		WorldGenHellLava(Block::lavaMoving->blockID).generate(worldObj, hellRNG, k1, i3, k4);
	}

	int_t j1 = hellRNG.nextInt(hellRNG.nextInt(10) + 1) + 1;
	for (int_t l1 = 0; l1 < j1; l1++)
	{
		int_t j3 = JavaArithmetic::intAdd(JavaArithmetic::intAdd(k, hellRNG.nextInt(16)), 8);
		int_t l4 = hellRNG.nextInt(120) + 4;
		int_t i6 = JavaArithmetic::intAdd(JavaArithmetic::intAdd(l, hellRNG.nextInt(16)), 8);
		WorldGenFire().generate(worldObj, hellRNG, j3, l4, i6);
	}

	j1 = hellRNG.nextInt(hellRNG.nextInt(10) + 1);
	for (int_t i2 = 0; i2 < j1; i2++)
	{
		int_t k3 = JavaArithmetic::intAdd(JavaArithmetic::intAdd(k, hellRNG.nextInt(16)), 8);
		int_t i5 = hellRNG.nextInt(120) + 4;
		int_t j6 = JavaArithmetic::intAdd(JavaArithmetic::intAdd(l, hellRNG.nextInt(16)), 8);
		WorldGenGlowStone1().generate(worldObj, hellRNG, k3, i5, j6);
	}

	for (int_t j2 = 0; j2 < 10; j2++)
	{
		int_t l3 = JavaArithmetic::intAdd(JavaArithmetic::intAdd(k, hellRNG.nextInt(16)), 8);
		int_t j5 = hellRNG.nextInt(128);
		int_t k6 = JavaArithmetic::intAdd(JavaArithmetic::intAdd(l, hellRNG.nextInt(16)), 8);
		WorldGenGlowStone2().generate(worldObj, hellRNG, l3, j5, k6);
	}

	if (hellRNG.nextInt(1) == 0)
	{
		int_t k2 = JavaArithmetic::intAdd(JavaArithmetic::intAdd(k, hellRNG.nextInt(16)), 8);
		int_t i4 = hellRNG.nextInt(128);
		int_t k5 = JavaArithmetic::intAdd(JavaArithmetic::intAdd(l, hellRNG.nextInt(16)), 8);
		WorldGenFlowers(Block::mushroomBrown->blockID).generate(worldObj, hellRNG, k2, i4, k5);
	}
	if (hellRNG.nextInt(1) == 0)
	{
		int_t l2 = JavaArithmetic::intAdd(JavaArithmetic::intAdd(k, hellRNG.nextInt(16)), 8);
		int_t j4 = hellRNG.nextInt(128);
		int_t l5 = JavaArithmetic::intAdd(JavaArithmetic::intAdd(l, hellRNG.nextInt(16)), 8);
		WorldGenFlowers(Block::mushroomRed->blockID).generate(worldObj, hellRNG, l2, j4, l5);
	}

	BlockSand::fallInstantly = false;
}

std::vector<SpawnListEntry> *ChunkProviderHell::getPossibleCreatures(const EnumCreatureType &type, int_t x, int_t y, int_t z)
{
	if (type.getCreatureTag() == EnumCreatureTypeTag::monster_tag && netherBridgeGenerator != nullptr &&
		netherBridgeGenerator->isInsideStructure(x, y, z))
		return netherBridgeGenerator->getSpawnList();

	BiomeGenBase *biome = worldObj->getBiomeGenForCoords(x, z);
	return biome == nullptr ? nullptr : biome->getSpawnableList(type);
}

ChunkPosition *ChunkProviderHell::findClosestStructure(World *, const jstring &, int_t, int_t, int_t)
{
	return nullptr;
}

bool ChunkProviderHell::saveChunks(bool flag, IProgressUpdate *iprogressupdate)
{
	return true;
}

bool ChunkProviderHell::unload100OldestChunks()
{
	return false;
}

bool ChunkProviderHell::canSave()
{
	return true;
}

jstring ChunkProviderHell::makeString()
{
	return "HellRandomLevelSource";
}
