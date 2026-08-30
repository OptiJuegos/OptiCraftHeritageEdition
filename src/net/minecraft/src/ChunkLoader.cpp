#include "platform/Log.h"
#include "ChunkLoader.h"
#include "java/Arithmetic.h"

#include <cstdio>
#include <sstream>
#include <memory>
#include <utility>

#include "World.h"
#include "WorldInfo.h"
#include "Chunk.h"
#include "NibbleArray.h"
#include "ExtendedBlockStorage.h"
#include "AnvilConverterData.h"
#include "NibbleArrayReader.h"
#include "WorldChunkManager.h"
#include "BiomeGenBase.h"
#include "NBTTagCompound.h"
#include "NBTTagList.h"
#include "CompressedStreamTools.h"
#include "Entity.h"
#include "EntityList.h"
#include "TileEntity.h"
#include "java/String.h"

// ---- base-36 helpers (Java Integer.toString(n, 36)) ----

static jstring intToBase36(int_t n)
{
	const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	bool negative = (n < 0);
	if (negative) n = -n;
	jstring result;
	do
	{
		result = jstring(1, (char)digits[n % 36]) + result;
		n /= 36;
	} while (n > 0);
	if (negative) result = "-" + result;
	return result;
}

// --------------------------------------------------------

namespace
{
	static const int_t kLegacyHeight = 128;
	static const std::size_t kLegacyBlockCount = 16u * 16u * (std::size_t)kLegacyHeight;
	static const std::size_t kLegacyNibbleCount = kLegacyBlockCount / 2u;

	enum LegacyNibblePlane
	{
		LegacyMetadata,
		LegacySkyLight,
		LegacyBlockLight
	};

	std::vector<byte_t> flattenLegacyBlocks(Chunk *chunk)
	{
		std::vector<byte_t> out(kLegacyBlockCount, 0);
		for (int_t x = 0; x < 16; ++x)
		{
			for (int_t z = 0; z < 16; ++z)
			{
				for (int_t y = 0; y < kLegacyHeight; ++y)
					out[(std::size_t)(x << 11 | z << 7 | y)] = (byte_t)(chunk->getBlockID(x, y, z) & 0xff);
			}
		}
		return out;
	}

	std::vector<byte_t> flattenLegacyNibble(Chunk *chunk, LegacyNibblePlane plane)
	{
		std::vector<byte_t> out(kLegacyNibbleCount, 0);
		for (int_t x = 0; x < 16; ++x)
		{
			for (int_t z = 0; z < 16; ++z)
			{
				for (int_t y = 0; y < kLegacyHeight; ++y)
				{
					int_t value = 0;
					if (plane == LegacyMetadata)
						value = chunk->getBlockMetadata(x, y, z);
					else if (plane == LegacySkyLight)
						value = chunk->getSavedLightValue(EnumSkyBlock::Sky, x, y, z);
					else
						value = chunk->getSavedLightValue(EnumSkyBlock::Block, x, y, z);

					const int_t index = x << 11 | z << 7 | y;
					byte_t &packed = out[(std::size_t)(index >> 1)];
					if ((index & 1) == 0)
						packed = (byte_t)((packed & 0xf0) | (value & 0xf));
					else
						packed = (byte_t)((packed & 0x0f) | ((value & 0xf) << 4));
				}
			}
		}
		return out;
	}

	void applyLegacyNibble(Chunk *chunk, const std::vector<byte_t> &bytes, LegacyNibblePlane plane)
	{
		if (chunk == nullptr || bytes.size() != kLegacyNibbleCount)
			return;
		for (int_t x = 0; x < 16; ++x)
		{
			for (int_t z = 0; z < 16; ++z)
			{
				for (int_t y = 0; y < kLegacyHeight; ++y)
				{
					const int_t index = x << 11 | z << 7 | y;
					const byte_t packed = bytes[(std::size_t)(index >> 1)];
					const int_t value = (index & 1) == 0 ? (packed & 0xf) : ((packed >> 4) & 0xf);
					if (plane == LegacyMetadata)
					{
						ExtendedBlockStorage *section = chunk->getBlockStorage(y >> 4);
						if (section != nullptr)
							section->setExtBlockMetadata(x, y & 15, z, value);
					}
					else
					{
						ExtendedBlockStorage *section = chunk->getBlockStorage(y >> 4);
						if (section == nullptr)
							continue;
						if (plane == LegacySkyLight)
							section->setExtSkylightValue(x, y & 15, z, value);
						else
							section->setExtBlocklightValue(x, y & 15, z, value);
					}
				}
			}
		}
	}
}

ChunkLoader::ChunkLoader(File *file, bool flag)
	: saveDir(file)
	, createIfNecessary(flag)
{
}

ChunkLoader::~ChunkLoader()
{
	delete saveDir;
}

File *ChunkLoader::chunkFileForXZ(int_t i, int_t j)
{
	jstring s = "c." + intToBase36(i) + "." + intToBase36(j) + ".dat";
	jstring s1 = intToBase36(i & 0x3f);
	jstring s2 = intToBase36(j & 0x3f);

	// dir1/dir2 are only needed to build the path and to mkdir() it; they must
	// not escape this function. Previously they were leaked on every single
	// chunk access (load AND save) -- three File* per call, forever, since
	// nothing ever freed them. On a freshly-explored world that is a handful of
	// allocations; on a large, already-explored save it is one leak per unique
	// chunk column ever touched in the session, compounding with how much of
	// the world has been visited.
	std::unique_ptr<File> dir1(File::open(*saveDir, s1));
	if (!dir1->exists())
	{
		if (createIfNecessary)
			dir1->mkdir();
		else
			return nullptr;
	}
	std::unique_ptr<File> dir2(File::open(*dir1, s2));
	if (!dir2->exists())
	{
		if (createIfNecessary)
			dir2->mkdir();
		else
			return nullptr;
	}
	File *file = File::open(*dir2, s);
	if (!file->exists() && !createIfNecessary)
	{
		delete file;
		return nullptr;
	}
	return file;
}

Chunk *ChunkLoader::loadChunk(World *world, int_t i, int_t j, ChunkLoadStatus *status)
{
	if (status != nullptr)
		*status = ChunkLoadStatus::Missing;
	std::unique_ptr<File> file(chunkFileForXZ(i, j));
	if (file != nullptr && file->exists())
	{
		if (status != nullptr)
			*status = ChunkLoadStatus::ReadError;
		try
		{
			std::istream *rawStream = file->toStreamIn();
			std::unique_ptr<std::istream> is(rawStream);
			std::unique_ptr<NBTTagCompound> nbttagcompound(CompressedStreamTools::readCompressed(*is));
			is.reset();

			if (!nbttagcompound || !nbttagcompound->hasKey("Level"))
			{
				MC_LOG_WARN("chunk", "Chunk file at %d,%d is missing level data, skipping\n", i, j);
				return nullptr;
			}

			NBTTagCompound *level = nbttagcompound->getCompoundTag("Level");
			if (level == nullptr || !level->hasKey("Blocks"))
			{
				MC_LOG_ERROR("chunk", "Chunk file at %d,%d is missing block data, skipping\n", i, j);
				return nullptr;
			}

			Chunk *chunk = loadChunkIntoWorldFromCompound(world, level);
			if (chunk != nullptr && !chunk->isAtLocation(i, j))
			{
				MC_LOG_ERROR("chunk", "Chunk file at %d,%d is in the wrong location; relocating in memory. "
					"(Expected %d, %d, got %d, %d)\n",
					i, j, i, j, chunk->xPosition, chunk->zPosition);
				delete chunk;
				level->setInteger("xPos", i);
				level->setInteger("zPos", j);
				chunk = loadChunkIntoWorldFromCompound(world, level);
				if (chunk == nullptr)
					return nullptr;
				chunk->setChunkModified();
			}

			if (chunk != nullptr)
			{
				chunk->remapBlocks();
				if (status != nullptr)
					*status = ChunkLoadStatus::Loaded;
			}
			return chunk;
		}
		catch (...)
		{
			MC_LOG_ERROR("chunk", "ChunkLoader::loadChunk – exception reading chunk %d,%d\n", i, j);
		}
	}
	return nullptr;
}

void ChunkLoader::saveChunk(World *world, Chunk *chunk)
{
	world->checkSessionLock();
	std::unique_ptr<File> file(chunkFileForXZ(chunk->xPosition, chunk->zPosition));
	if (file->exists())
	{
		WorldInfo *worldinfo = world->getWorldInfo();
		worldinfo->setSizeOnDisk(worldinfo->getSizeOnDisk() - file->length());
	}
	try
	{
		std::unique_ptr<File> tmpFile(File::open(*saveDir, "tmp_chunk.dat"));
		std::ostream *rawStream = tmpFile->toStreamOut();
		std::unique_ptr<std::ostream> os(rawStream);
		std::unique_ptr<NBTTagCompound> nbttagcompound(new NBTTagCompound());
		NBTTagCompound *nbttagcompound1 = new NBTTagCompound();
		nbttagcompound->setTag("Level", nbttagcompound1);
		storeChunkInCompound(chunk, world, nbttagcompound1);
		CompressedStreamTools::writeGzippedCompoundToOutputStream(nbttagcompound.get(), *os);
		os->flush();
		os.reset();
		if (file->exists())
			file->remove();
		tmpFile->renameTo(*file);
		WorldInfo *worldinfo1 = world->getWorldInfo();
		worldinfo1->setSizeOnDisk(worldinfo1->getSizeOnDisk() + file->length());
	}
	catch (...)
	{
		MC_LOG_ERROR("chunk", "ChunkLoader::saveChunk – exception saving chunk %d,%d\n",
			chunk->xPosition, chunk->zPosition);
	}
}

void ChunkLoader::storeChunkInCompound(Chunk *chunk, World *world, NBTTagCompound *nbttagcompound)
{
	world->checkSessionLock();
	nbttagcompound->setInteger("xPos", chunk->xPosition);
	nbttagcompound->setInteger("zPos", chunk->zPosition);
	nbttagcompound->setLong("LastUpdate", world->getWorldTime());
	nbttagcompound->setByteArray("Blocks", flattenLegacyBlocks(chunk));
	nbttagcompound->setByteArray("Data", flattenLegacyNibble(chunk, LegacyMetadata));
	nbttagcompound->setByteArray("SkyLight", flattenLegacyNibble(chunk, LegacySkyLight));
	nbttagcompound->setByteArray("BlockLight", flattenLegacyNibble(chunk, LegacyBlockLight));
	std::vector<byte_t> legacyHeightMap(256u, 0);
	for (int_t column = 0; column < 256; ++column)
		legacyHeightMap[(std::size_t)column] = (byte_t)std::min(chunk->heightMap[column], 255);
	nbttagcompound->setByteArray("HeightMap", legacyHeightMap);
	nbttagcompound->setBoolean("TerrainPopulated", chunk->isTerrainPopulated);

	chunk->hasEntities = false;
	NBTTagList *nbttaglist = new NBTTagList();
	for (int_t i = 0; i < Chunk::SECTION_COUNT; i++)
	{
		for (Entity *entity : chunk->entities[i])
		{
			if (entity == nullptr || !world->isLoadedEntityPointer(entity))
				continue;
			if (!entity->addedToChunk || entity->chunkCoordX != chunk->xPosition ||
			    entity->chunkCoordZ != chunk->zPosition || entity->chunkCoordY != i)
				continue;
			chunk->hasEntities = true;
			NBTTagCompound *nbttagcompound1 = new NBTTagCompound();
			if (entity->addEntityID(nbttagcompound1))
				nbttaglist->setTag(nbttagcompound1);
			else
				delete nbttagcompound1;
		}
	}
	nbttagcompound->setTag("Entities", nbttaglist);

	NBTTagList *nbttaglist1 = new NBTTagList();
	for (const ChunkPosition &position : chunk->chunkTileEntityOrder.valuesInIterationOrder())
	{
		auto it = chunk->chunkTileEntityMap.find(position);
		if (it == chunk->chunkTileEntityMap.end())
			continue;
		TileEntity *tileentity = it->second;
		if (tileentity == nullptr || !world->isLoadedTileEntityPointer(tileentity))
			continue;
		NBTTagCompound *nbttagcompound2 = new NBTTagCompound();
		tileentity->writeToNBT(nbttagcompound2);
		nbttaglist1->setTag(nbttagcompound2);
	}
	nbttagcompound->setTag("TileEntities", nbttaglist1);
}

Chunk *ChunkLoader::loadChunkIntoWorldFromCompound(World *world, NBTTagCompound *nbttagcompound)
{
	static const size_t BLOCKS_SIZE = 16 * 16 * 128;
	static const size_t NIBBLE_SIZE = BLOCKS_SIZE / 2;
	static const size_t HEIGHTMAP_SIZE = 16 * 16;

	int_t i = nbttagcompound->getInteger("xPos");
	int_t j = nbttagcompound->getInteger("zPos");
	std::vector<byte_t> blocks = nbttagcompound->getByteArray("Blocks");
	if (blocks.size() != BLOCKS_SIZE)
	{
		MC_LOG_INFO("chunk", "Invalid Blocks size in chunk %d,%d: %zu bytes; expected %zu. Regenerating chunk.\n",
			i, j, blocks.size(), BLOCKS_SIZE);
		return nullptr;
	}
	Chunk *chunk = new Chunk(world, std::move(blocks), i, j);

	std::vector<byte_t> dataBytes = nbttagcompound->getByteArray("Data");
	if (dataBytes.size() == NIBBLE_SIZE)
		applyLegacyNibble(chunk, dataBytes, LegacyMetadata);
	else
		MC_LOG_INFO("chunk", "Invalid Data size in chunk %d,%d: %zu bytes; expected %zu. Resetting metadata.\n",
			i, j, dataBytes.size(), NIBBLE_SIZE);

	std::vector<byte_t> skyBytes = nbttagcompound->getByteArray("SkyLight");
	bool regenerateSkyLight = false;
	if (skyBytes.size() == NIBBLE_SIZE)
		applyLegacyNibble(chunk, skyBytes, LegacySkyLight);
	else
	{
		MC_LOG_INFO("chunk", "Invalid SkyLight size in chunk %d,%d: %zu bytes; expected %zu. Regenerating skylight.\n",
			i, j, skyBytes.size(), NIBBLE_SIZE);
		regenerateSkyLight = true;
	}

	std::vector<byte_t> blockLightBytes = nbttagcompound->getByteArray("BlockLight");
	if (blockLightBytes.size() == NIBBLE_SIZE)
		applyLegacyNibble(chunk, blockLightBytes, LegacyBlockLight);
	else
	{
		MC_LOG_INFO("chunk", "Invalid BlockLight size in chunk %d,%d: %zu bytes; expected %zu. Resetting block light.\n",
			i, j, blockLightBytes.size(), NIBBLE_SIZE);
		chunk->initBlockLight();
	}

	std::vector<byte_t> hm = nbttagcompound->getByteArray("HeightMap");
	if (hm.size() == HEIGHTMAP_SIZE)
	{
		for (std::size_t column = 0; column < HEIGHTMAP_SIZE; ++column)
			chunk->heightMap[column] = hm[column] & 0xff;
	}
	else
	{
		MC_LOG_INFO("chunk", "Invalid HeightMap size in chunk %d,%d: %zu bytes; expected %zu. Regenerating heightmap/skylight.\n",
			i, j, hm.size(), HEIGHTMAP_SIZE);
		std::fill(chunk->heightMap, chunk->heightMap + HEIGHTMAP_SIZE, 0);
		regenerateSkyLight = true;
	}

	chunk->isTerrainPopulated = nbttagcompound->getBoolean("TerrainPopulated");

	if (regenerateSkyLight)
	{
		chunk->generateSkylightMap();
	}

	NBTTagList *nbttaglist = nbttagcompound->getTagList("Entities");
	if (nbttaglist != nullptr)
	{
		for (int_t k = 0; k < nbttaglist->tagCount(); k++)
		{
			NBTTagCompound *nbttagcompound1 = static_cast<NBTTagCompound *>(nbttaglist->tagAt(k));
			Entity *entity = EntityList::createEntityFromNBT(nbttagcompound1, world);
			chunk->hasEntities = true;
			if (entity != nullptr)
				chunk->addEntity(entity);
		}
	}

	NBTTagList *nbttaglist1 = nbttagcompound->getTagList("TileEntities");
	if (nbttaglist1 != nullptr)
	{
		for (int_t l = 0; l < nbttaglist1->tagCount(); l++)
		{
			NBTTagCompound *nbttagcompound2 = static_cast<NBTTagCompound *>(nbttaglist1->tagAt(l));
			TileEntity *tileentity = TileEntity::createAndLoadEntity(nbttagcompound2);
			if (tileentity != nullptr)
				chunk->addTileEntity(tileentity);
		}
	}

	return chunk;
}


AnvilConverterData *ChunkLoader::load(NBTTagCompound *nbttagcompound)
{
	if (nbttagcompound == nullptr)
		return nullptr;

	AnvilConverterData *data = new AnvilConverterData(
		nbttagcompound->getInteger("xPos"), nbttagcompound->getInteger("zPos"));
	data->blocks = nbttagcompound->getByteArray("Blocks");
	data->data = NibbleArrayReader(nbttagcompound->getByteArray("Data"), 7);
	data->skyLight = NibbleArrayReader(nbttagcompound->getByteArray("SkyLight"), 7);
	data->blockLight = NibbleArrayReader(nbttagcompound->getByteArray("BlockLight"), 7);
	data->heightmap = nbttagcompound->getByteArray("HeightMap");
	data->terrainPopulated = nbttagcompound->getBoolean("TerrainPopulated");
	data->entities = nbttagcompound->getTagList("Entities");
	data->tileEntities = nbttagcompound->getTagList("TileEntities");
	data->tileTicks = nbttagcompound->hasKey("TileTicks") ? nbttagcompound->getTagList("TileTicks") : nullptr;

	// Old saves occasionally encoded LastUpdate with TAG_Int. NBTTagCompound's
	// typed getters return zero for a mismatched tag, so inspect the tag first.
	NBTBase *lastUpdate = nbttagcompound->getTag("LastUpdate");
	if (lastUpdate != nullptr && lastUpdate->getType() == 4)
		data->lastUpdated = nbttagcompound->getLong("LastUpdate");
	else
		data->lastUpdated = (long_t)nbttagcompound->getInteger("LastUpdate");

	return data;
}

void ChunkLoader::convertToAnvilFormat(AnvilConverterData *data, NBTTagCompound *output,
                                      WorldChunkManager *worldChunkManager)
{
	if (data == nullptr || output == nullptr || worldChunkManager == nullptr)
		return;

	output->setInteger("xPos", data->x);
	output->setInteger("zPos", data->z);
	output->setLong("LastUpdate", data->lastUpdated);

	std::vector<int_t> heightMap(data->heightmap.size(), 0);
	for (std::size_t i = 0; i < data->heightmap.size(); ++i)
		heightMap[i] = (int_t)data->heightmap[i];
	output->setIntArray("HeightMap", heightMap);
	output->setBoolean("TerrainPopulated", data->terrainPopulated);

	NBTTagList *sections = new NBTTagList();
	const std::size_t expectedBlocks = 16u * 16u * 128u;
	if (data->blocks.size() >= expectedBlocks)
	{
		for (int_t sectionIndex = 0; sectionIndex < 8; ++sectionIndex)
		{
			bool empty = true;
			for (int_t x = 0; x < 16 && empty; ++x)
			{
				for (int_t z = 0; z < 16 && empty; ++z)
				{
					for (int_t y = 0; y < 16; ++y)
					{
						const int_t legacyIndex = x << 11 | z << 7 | y + (sectionIndex << 4);
						if (data->blocks[(std::size_t)legacyIndex] != 0)
						{
							empty = false;
							break;
						}
					}
				}
			}

			if (empty)
				continue;

			std::vector<byte_t> blocks(4096, 0);
			NibbleArray metadata(4096, 4);
			NibbleArray skyLight(4096, 4);
			NibbleArray blockLight(4096, 4);

			for (int_t x = 0; x < 16; ++x)
			{
				for (int_t y = 0; y < 16; ++y)
				{
					for (int_t z = 0; z < 16; ++z)
					{
						const int_t legacyY = y + (sectionIndex << 4);
						const int_t legacyIndex = x << 11 | z << 7 | legacyY;
						const int_t sectionStorageIndex = y << 8 | z << 4 | x;
						blocks[(std::size_t)sectionStorageIndex] = data->blocks[(std::size_t)legacyIndex];
						metadata.set(x, y, z, data->data.get(x, legacyY, z));
						skyLight.set(x, y, z, data->skyLight.get(x, legacyY, z));
						blockLight.set(x, y, z, data->blockLight.get(x, legacyY, z));
					}
				}
			}

			NBTTagCompound *section = new NBTTagCompound();
			section->setByte("Y", (byte_t)sectionIndex);
			section->setByteArray("Blocks", blocks);
			section->setByteArray("Data", metadata.data);
			section->setByteArray("SkyLight", skyLight.data);
			section->setByteArray("BlockLight", blockLight.data);
			sections->appendTag(section);
		}
	}
	output->setTag("Sections", sections);

	std::vector<byte_t> biomes(256, 0);
	for (int_t x = 0; x < 16; ++x)
	{
		for (int_t z = 0; z < 16; ++z)
		{
			BiomeGenBase *biome = worldChunkManager->getBiomeGenAt(JavaArithmetic::intShl(data->x, 4) | x,
			                                                     JavaArithmetic::intShl(data->z, 4) | z);
			biomes[(std::size_t)(z << 4 | x)] = (byte_t)(biome != nullptr ? biome->biomeID & 0xff : 0);
		}
	}
	output->setByteArray("Biomes", biomes);

	// setTag takes ownership. Copy the lists because converter data borrows them
	// from the legacy Level compound that remains owner of the source tags.
	if (data->entities != nullptr)
		output->setTag("Entities", data->entities->copy());
	else
		output->setTag("Entities", new NBTTagList());
	if (data->tileEntities != nullptr)
		output->setTag("TileEntities", data->tileEntities->copy());
	else
		output->setTag("TileEntities", new NBTTagList());
	if (data->tileTicks != nullptr)
		output->setTag("TileTicks", data->tileTicks->copy());
}

void ChunkLoader::addRandomArmor()
{
}

void ChunkLoader::saveExtraData()
{
}

void ChunkLoader::saveExtraChunkData(World *world, Chunk *chunk)
{
}
