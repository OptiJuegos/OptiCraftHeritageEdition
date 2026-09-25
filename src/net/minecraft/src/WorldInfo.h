#pragma once

#include <string>
#include <vector>
#include "java/Type.h"
#include "java/String.h"

class EntityPlayer;
class NBTTagCompound;
class WorldType;
class WorldSettings;

// net.minecraft.src.WorldInfo
class WorldInfo
{
public:
	WorldInfo(NBTTagCompound *nbttagcompound);
	WorldInfo(long_t l, const jstring &s);
	WorldInfo(WorldSettings *settings, const jstring &s);
	WorldInfo(WorldInfo *worldinfo);
	~WorldInfo();

	NBTTagCompound *getNBTTagCompound();
	NBTTagCompound *getNBTTagCompoundWithPlayer(const std::vector<EntityPlayer *> &list);
	NBTTagCompound *getNBTTagCompoundWithPlayers(const std::vector<EntityPlayer *> &list);

private:
	void updateTagCompound(NBTTagCompound *nbttagcompound, NBTTagCompound *nbttagcompound1);

public:
	long_t getRandomSeed();
	long_t getSeed();
	int_t getSpawnX();
	int_t getSpawnY();
	int_t getSpawnZ();
	long_t getWorldTime();
	long_t getSizeOnDisk();
	NBTTagCompound *getPlayerNBTTagCompound();
	int_t getDimension();
	void setSpawnX(int_t i);
	void setSpawnY(int_t i);
	void setSpawnZ(int_t i);
	void setWorldTime(long_t l);
	void setSizeOnDisk(long_t l);
	void setPlayerNBTTagCompound(NBTTagCompound *nbttagcompound);
	void setSpawn(int_t i, int_t j, int_t k);
	void setSpawnPosition(int_t i, int_t j, int_t k);
	jstring getWorldName();
	void setWorldName(const jstring &s);
	int_t getSaveVersion();
	void setSaveVersion(int_t i);
	long_t getLastTimePlayed();
	bool getThundering();
	bool isThundering();
	void setThundering(bool flag);
	int_t getThunderTime();
	void setThunderTime(int_t i);
	bool getRaining();
	bool isRaining();
	void setRaining(bool flag);
	int_t getRainTime();
	void setRainTime(int_t i);
	int_t getGameType();
	void setGameType(int_t gameType);
	bool isMapFeaturesEnabled();
	bool isHardcoreModeEnabled();
	WorldType *getTerrainType();
	void setTerrainType(WorldType *type);
	bool isLimitedWorld() const;
	void setLimitedWorld(bool flag);

private:
	long_t randomSeed;
	WorldType *terrainType;
	int_t spawnX;
	int_t spawnY;
	int_t spawnZ;
	long_t worldTime;
	long_t lastTimePlayed;
	long_t sizeOnDisk;
	NBTTagCompound *playerTag;
	int_t dimension;
	jstring levelName;
	int_t saveVersion;
	int_t gameType;
	bool mapFeaturesEnabled;
	bool hardcore;
	bool raining;
	int_t rainTime;
	bool thundering;
	int_t thunderTime;
	bool limitedWorld;
};
