#pragma once

#include "java/Type.h"
#include "java/String.h"

class Entity;
class TileEntity;
class EntityPlayer;

// net.minecraft.src.IWorldAccess
class IWorldAccess
{
public:
	virtual ~IWorldAccess() = default;
	virtual void markBlockAndNeighborsNeedsUpdate(int_t i, int_t j, int_t k) = 0;
	virtual void markBlockRangeNeedsUpdate(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1) = 0;
	virtual void onChunkPublished(int_t chunkX, int_t chunkZ) { (void)chunkX; (void)chunkZ; }
	virtual void playSound(const jstring &s, double d, double d1, double d2, float f, float f1) = 0;
	virtual void spawnParticle(const jstring &s, double d, double d1, double d2, double d3, double d4, double d5) = 0;
	virtual void obtainEntitySkin(Entity *entity) = 0;
	virtual void releaseEntitySkin(Entity *entity) = 0;
	virtual void updateAllRenderers() = 0;
	virtual void playRecord(const jstring &s, int_t i, int_t j, int_t k) = 0;
	virtual void doNothingWithTileEntity(int_t i, int_t j, int_t k, TileEntity *tileentity) = 0;
	virtual void playAuxSFX(EntityPlayer *player, int_t eventId, int_t x, int_t y, int_t z, int_t data) = 0;
};
