#include "ChunkProviderFlat.h"

#include "BiomeGenBase.h"
#include "Block.h"
#include "BlockGrass.h"
#include "Chunk.h"
#include "MapGenVillage.h"
#include "World.h"
#include "WorldChunkManager.h"
#include "java/Arithmetic.h"

ChunkProviderFlat::ChunkProviderFlat(World *world, long_t seed, bool structures) :
	worldObj(world), random(seed), useStructures(structures), villageGen(new MapGenVillage(1))
{
}

ChunkProviderFlat::~ChunkProviderFlat()
{
	delete villageGen;
}

void ChunkProviderFlat::generate(std::vector<byte_t> &blocks)
{
	const int_t height = (int_t)blocks.size() / 256;
	for (int_t x = 0; x < 16; ++x)
	{
		for (int_t z = 0; z < 16; ++z)
		{
			for (int_t y = 0; y < height; ++y)
			{
				int_t blockId = 0;
				if (y == 0)
					blockId = Block::bedrock->blockID;
				else if (y <= 2)
					blockId = Block::dirt->blockID;
				else if (y == 3)
					blockId = Block::grass->blockID;
				blocks[(std::size_t)(x << 11 | z << 7 | y)] = (byte_t)blockId;
			}
		}
	}
}

Chunk *ChunkProviderFlat::prepareChunk(int_t x, int_t z)
{
	return provideChunk(x, z);
}

Chunk *ChunkProviderFlat::loadChunk(int_t x, int_t z)
{
	return provideChunk(x, z);
}

Chunk *ChunkProviderFlat::provideChunk(int_t x, int_t z)
{
	std::vector<byte_t> blocks(32768, 0);
	generate(blocks);
	Chunk *chunk = new Chunk(worldObj, blocks, x, z);

	if (useStructures)
		villageGen->generate(this, worldObj, x, z, blocks.data());

	std::vector<BiomeGenBase *> biomes;
	worldObj->getWorldChunkManager()->loadBlockGeneratorData(
		biomes, JavaArithmetic::intMul(x, 16), JavaArithmetic::intMul(z, 16), 16, 16);
	std::vector<byte_t> &biomeArray = chunk->getBiomeArray();
	for (std::size_t i = 0; i < biomeArray.size() && i < biomes.size(); ++i)
		biomeArray[i] = biomes[i] == nullptr ? 0 : (byte_t)biomes[i]->biomeID;

	chunk->generateSkylightMap();
	return chunk;
}

bool ChunkProviderFlat::chunkExists(int_t, int_t)
{
	return true;
}

void ChunkProviderFlat::populate(IChunkProvider *, int_t x, int_t z)
{
	random.setSeed(worldObj->getRandomSeed());
	const long_t oddX = JavaArithmetic::longAdd(JavaArithmetic::longMul(random.nextLong() / 2LL, 2LL), 1LL);
	const long_t oddZ = JavaArithmetic::longAdd(JavaArithmetic::longMul(random.nextLong() / 2LL, 2LL), 1LL);
	const long_t coordSeed = JavaArithmetic::longAdd(JavaArithmetic::longMul((long_t)x, oddX), JavaArithmetic::longMul((long_t)z, oddZ));
	random.setSeed(JavaArithmetic::longFromBits((ulong_t)coordSeed ^ (ulong_t)worldObj->getRandomSeed()));
	if (useStructures)
		villageGen->generateStructuresInChunk(worldObj, random, x, z);
}

bool ChunkProviderFlat::saveChunks(bool, IProgressUpdate *)
{
	return true;
}

bool ChunkProviderFlat::unload100OldestChunks()
{
	return false;
}

bool ChunkProviderFlat::canSave()
{
	return true;
}

jstring ChunkProviderFlat::makeString()
{
	return "FlatLevelSource";
}

std::vector<SpawnListEntry> *ChunkProviderFlat::getPossibleCreatures(const EnumCreatureType &type, int_t x, int_t, int_t z)
{
	BiomeGenBase *biome = worldObj->getBiomeGenForCoords(x, z);
	return biome == nullptr ? nullptr : biome->getSpawnableList(type);
}

ChunkPosition *ChunkProviderFlat::findClosestStructure(World *, const jstring &, int_t, int_t, int_t)
{
	return nullptr;
}
