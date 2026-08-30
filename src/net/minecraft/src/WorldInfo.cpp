#include "WorldInfo.h"

#include "EntityPlayer.h"
#include "NBTTagCompound.h"
#include "WorldType.h"
#include "WorldSettings.h"
#include "java/System.h"

WorldInfo::WorldInfo(NBTTagCompound *nbttagcompound)
{
	WorldType::initialize();
	randomSeed = nbttagcompound->getLong("RandomSeed");
	gameType = nbttagcompound->getInteger("GameType");
	mapFeaturesEnabled = nbttagcompound->hasKey("MapFeatures") ? nbttagcompound->getBoolean("MapFeatures") : true;
	hardcore = nbttagcompound->getBoolean("hardcore");
	terrainType = WorldType::DEFAULT;
	if (nbttagcompound->hasKey("generatorName"))
	{
		terrainType = WorldType::parseWorldType(nbttagcompound->getString("generatorName"));
		if (terrainType == nullptr)
			terrainType = WorldType::DEFAULT;
		else if (terrainType->func_48626_e())
		{
			const int_t generatorVersion = nbttagcompound->hasKey("generatorVersion")
				? nbttagcompound->getInteger("generatorVersion")
				: 0;
			terrainType = terrainType->func_48629_a(generatorVersion);
		}
	}
	spawnX = nbttagcompound->getInteger("SpawnX");
	spawnY = nbttagcompound->getInteger("SpawnY");
	spawnZ = nbttagcompound->getInteger("SpawnZ");
	worldTime = nbttagcompound->getLong("Time");
	lastTimePlayed = nbttagcompound->getLong("LastPlayed");
	sizeOnDisk = nbttagcompound->getLong("SizeOnDisk");
	levelName = nbttagcompound->getString("LevelName");
	saveVersion = nbttagcompound->getInteger("version");
	rainTime = nbttagcompound->getInteger("rainTime");
	raining = nbttagcompound->getBoolean("raining");
	thunderTime = nbttagcompound->getInteger("thunderTime");
	thundering = nbttagcompound->getBoolean("thundering");
	playerTag = nullptr;
	dimension = 0;
	if (nbttagcompound->hasKey("Player"))
	{
		NBTTagCompound *sourcePlayerTag = nbttagcompound->getCompoundTag("Player");
		playerTag = static_cast<NBTTagCompound *>(sourcePlayerTag->copy());
		dimension = playerTag->getInteger("Dimension");
	}
}

WorldInfo::WorldInfo(long_t l, const jstring &s)
{
	WorldType::initialize();
	randomSeed = l;
	terrainType = WorldType::DEFAULT;
	levelName = s;
	spawnX = 0;
	spawnY = 0;
	spawnZ = 0;
	worldTime = 0;
	lastTimePlayed = 0;
	sizeOnDisk = 0;
	playerTag = nullptr;
	dimension = 0;
	saveVersion = 0;
	gameType = 0;
	mapFeaturesEnabled = true;
	hardcore = false;
	raining = false;
	rainTime = 0;
	thundering = false;
	thunderTime = 0;
}

WorldInfo::WorldInfo(WorldSettings *settings, const jstring &s)
{
	WorldType::initialize();
	randomSeed = settings != nullptr ? settings->getSeed() : 0;
	terrainType = settings != nullptr && settings->getTerrainType() != nullptr ? settings->getTerrainType() : WorldType::DEFAULT;
	gameType = settings != nullptr ? settings->getGameType() : 0;
	mapFeaturesEnabled = settings == nullptr || settings->isMapFeaturesEnabled();
	hardcore = settings != nullptr && settings->getHardcoreEnabled();
	levelName = s;
	spawnX = 0;
	spawnY = 0;
	spawnZ = 0;
	worldTime = 0;
	lastTimePlayed = 0;
	sizeOnDisk = 0;
	playerTag = nullptr;
	dimension = 0;
	saveVersion = 0;
	raining = false;
	rainTime = 0;
	thundering = false;
	thunderTime = 0;
}

WorldInfo::WorldInfo(WorldInfo *worldinfo)
{
	randomSeed = worldinfo->randomSeed;
	terrainType = worldinfo->terrainType;
	gameType = worldinfo->gameType;
	mapFeaturesEnabled = worldinfo->mapFeaturesEnabled;
	hardcore = worldinfo->hardcore;
	spawnX = worldinfo->spawnX;
	spawnY = worldinfo->spawnY;
	spawnZ = worldinfo->spawnZ;
	worldTime = worldinfo->worldTime;
	lastTimePlayed = worldinfo->lastTimePlayed;
	sizeOnDisk = worldinfo->sizeOnDisk;
	playerTag = worldinfo->playerTag != nullptr
		? static_cast<NBTTagCompound *>(worldinfo->playerTag->copy())
		: nullptr;
	dimension = worldinfo->dimension;
	levelName = worldinfo->levelName;
	saveVersion = worldinfo->saveVersion;
	rainTime = worldinfo->rainTime;
	raining = worldinfo->raining;
	thunderTime = worldinfo->thunderTime;
	thundering = worldinfo->thundering;
}

WorldInfo::~WorldInfo()
{
	delete playerTag;
	playerTag = nullptr;
}

NBTTagCompound *WorldInfo::getNBTTagCompound()
{
	NBTTagCompound *nbttagcompound = new NBTTagCompound();
	updateTagCompound(nbttagcompound, playerTag);
	return nbttagcompound;
}

NBTTagCompound *WorldInfo::getNBTTagCompoundWithPlayer(const std::vector<EntityPlayer *> &list)
{
	NBTTagCompound *nbttagcompound = new NBTTagCompound();
	EntityPlayer *entityplayer = nullptr;
	NBTTagCompound *nbttagcompound1 = nullptr;
	if (!list.empty())
		entityplayer = list[0];
	if (entityplayer != nullptr)
	{
		nbttagcompound1 = new NBTTagCompound();
		entityplayer->writeToNBT(nbttagcompound1);
	}
	updateTagCompound(nbttagcompound, nbttagcompound1);
	return nbttagcompound;
}

NBTTagCompound *WorldInfo::getNBTTagCompoundWithPlayers(const std::vector<EntityPlayer *> &list)
{
	return getNBTTagCompoundWithPlayer(list);
}

void WorldInfo::updateTagCompound(NBTTagCompound *nbttagcompound, NBTTagCompound *nbttagcompound1)
{
	nbttagcompound->setLong("RandomSeed", randomSeed);
	if (terrainType != nullptr)
	{
		nbttagcompound->setString("generatorName", terrainType->func_48628_a());
		nbttagcompound->setInteger("generatorVersion", terrainType->getGeneratorVersion());
	}
	nbttagcompound->setInteger("GameType", gameType);
	nbttagcompound->setBoolean("MapFeatures", mapFeaturesEnabled);
	nbttagcompound->setInteger("SpawnX", spawnX);
	nbttagcompound->setInteger("SpawnY", spawnY);
	nbttagcompound->setInteger("SpawnZ", spawnZ);
	nbttagcompound->setLong("Time", worldTime);
	nbttagcompound->setLong("SizeOnDisk", sizeOnDisk);
	nbttagcompound->setLong("LastPlayed", System::currentTimeMillis());
	nbttagcompound->setString("LevelName", levelName);
	nbttagcompound->setInteger("version", saveVersion);
	nbttagcompound->setInteger("rainTime", rainTime);
	nbttagcompound->setBoolean("raining", raining);
	nbttagcompound->setInteger("thunderTime", thunderTime);
	nbttagcompound->setBoolean("thundering", thundering);
	nbttagcompound->setBoolean("hardcore", hardcore);
	if (nbttagcompound1 != nullptr)
	{
		if (nbttagcompound1 == playerTag)
			nbttagcompound->setCompoundTag("Player", static_cast<NBTTagCompound *>(playerTag->copy()));
		else
			nbttagcompound->setCompoundTag("Player", nbttagcompound1);
	}
}

long_t WorldInfo::getRandomSeed() { return randomSeed; }
long_t WorldInfo::getSeed() { return randomSeed; }
int_t WorldInfo::getSpawnX() { return spawnX; }
int_t WorldInfo::getSpawnY() { return spawnY; }
int_t WorldInfo::getSpawnZ() { return spawnZ; }
long_t WorldInfo::getWorldTime() { return worldTime; }
long_t WorldInfo::getSizeOnDisk() { return sizeOnDisk; }
NBTTagCompound *WorldInfo::getPlayerNBTTagCompound() { return playerTag; }
int_t WorldInfo::getDimension() { return dimension; }
void WorldInfo::setSpawnX(int_t i) { spawnX = i; }
void WorldInfo::setSpawnY(int_t i) { spawnY = i; }
void WorldInfo::setSpawnZ(int_t i) { spawnZ = i; }
void WorldInfo::setWorldTime(long_t l) { worldTime = l; }
void WorldInfo::setSizeOnDisk(long_t l) { sizeOnDisk = l; }
void WorldInfo::setPlayerNBTTagCompound(NBTTagCompound *nbttagcompound)
{
	if (playerTag == nbttagcompound)
		return;
	delete playerTag;
	playerTag = nbttagcompound;
}
void WorldInfo::setSpawn(int_t i, int_t j, int_t k) { spawnX = i; spawnY = j; spawnZ = k; }
void WorldInfo::setSpawnPosition(int_t i, int_t j, int_t k) { setSpawn(i, j, k); }
jstring WorldInfo::getWorldName() { return levelName; }
void WorldInfo::setWorldName(const jstring &s) { levelName = s; }
int_t WorldInfo::getSaveVersion() { return saveVersion; }
void WorldInfo::setSaveVersion(int_t i) { saveVersion = i; }
long_t WorldInfo::getLastTimePlayed() { return lastTimePlayed; }
bool WorldInfo::getThundering() { return thundering; }
bool WorldInfo::isThundering() { return thundering; }
void WorldInfo::setThundering(bool flag) { thundering = flag; }
int_t WorldInfo::getThunderTime() { return thunderTime; }
void WorldInfo::setThunderTime(int_t i) { thunderTime = i; }
bool WorldInfo::getRaining() { return raining; }
bool WorldInfo::isRaining() { return raining; }
void WorldInfo::setRaining(bool flag) { raining = flag; }
int_t WorldInfo::getRainTime() { return rainTime; }
void WorldInfo::setRainTime(int_t i) { rainTime = i; }


int_t WorldInfo::getGameType() { return gameType; }
bool WorldInfo::isMapFeaturesEnabled() { return mapFeaturesEnabled; }
bool WorldInfo::isHardcoreModeEnabled() { return hardcore; }
WorldType *WorldInfo::getTerrainType() { return terrainType; }
void WorldInfo::setTerrainType(WorldType *type) { terrainType = type != nullptr ? type : WorldType::DEFAULT; }
