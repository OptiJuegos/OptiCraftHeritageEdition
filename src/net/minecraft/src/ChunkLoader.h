#pragma once

#include "IChunkLoader.h"
#include "java/Type.h"
#include "java/File.h"
#include "java/String.h"

class World;
class Chunk;
class NBTTagCompound;
class AnvilConverterData;
class WorldChunkManager;

// net.minecraft.src.ChunkLoader
class ChunkLoader : public IChunkLoader
{
public:
	ChunkLoader(File *file, bool flag);
	~ChunkLoader() override;

	Chunk *loadChunk(World *world, int_t i, int_t j,
		ChunkLoadStatus *status = nullptr) override;
	void   saveChunk(World *world, Chunk *chunk) override;

	static void storeChunkInCompound(Chunk *chunk, World *world, NBTTagCompound *nbttagcompound);
	static Chunk *loadChunkIntoWorldFromCompound(World *world, NBTTagCompound *nbttagcompound);

	// Minecraft 1.2.5 McRegion -> Anvil conversion helpers. The returned
	// converter data borrows entity/tile NBT lists from the source compound.
	static AnvilConverterData *load(NBTTagCompound *nbttagcompound);
	static void convertToAnvilFormat(AnvilConverterData *data, NBTTagCompound *output,
	                                 WorldChunkManager *worldChunkManager);

	void addRandomArmor() override;   // func_814_a – no-op flush/tick
	void saveExtraData() override;
	void saveExtraChunkData(World *world, Chunk *chunk) override;

private:
	File *chunkFileForXZ(int_t i, int_t j);

	File *saveDir;
	bool  createIfNecessary;
};
