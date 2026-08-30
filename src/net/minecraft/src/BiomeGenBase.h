#pragma once

#include <vector>
#include "java/Type.h"
#include "java/String.h"
#include "java/Random.h"
#include "SpawnListEntry.h"
#include "EnumCreatureType.h"

class WorldGenerator;
class BiomeDecorator;
class World;

// net.minecraft.src.BiomeGenBase
class BiomeGenBase
{
public:
    static constexpr int_t BIOME_REGISTRY_SIZE = 256;
    static BiomeGenBase *biomeList[BIOME_REGISTRY_SIZE];

    // Minecraft 1.2.5 biome registry.
    static BiomeGenBase *ocean;
    static BiomeGenBase *plains;
    static BiomeGenBase *desert;
    static BiomeGenBase *extremeHills;
    static BiomeGenBase *forest;
    static BiomeGenBase *taiga;
    static BiomeGenBase *swampland;
    static BiomeGenBase *river;
    static BiomeGenBase *hell;
    static BiomeGenBase *sky;
    static BiomeGenBase *frozenOcean;
    static BiomeGenBase *frozenRiver;
    static BiomeGenBase *icePlains;
    static BiomeGenBase *iceMountains;
    static BiomeGenBase *mushroomIsland;
    static BiomeGenBase *mushroomIslandShore;
    static BiomeGenBase *beach;
    static BiomeGenBase *desertHills;
    static BiomeGenBase *forestHills;
    static BiomeGenBase *taigaHills;
    static BiomeGenBase *extremeHillsEdge;
    static BiomeGenBase *jungle;
    static BiomeGenBase *jungleHills;

    static void initialize();
    static void cleanup();

    virtual WorldGenerator *getRandomWorldGenForTrees(Random &random);
    void releaseWorldGenForTrees(WorldGenerator *generator);
    virtual WorldGenerator *func_48410_b(Random &random);
    virtual int_t getSkyColorByTemp(float temperature);
    virtual std::vector<SpawnListEntry> *getSpawnableList(const EnumCreatureType &enumcreaturetype);

    bool getEnableSnow() const { return enableSnow; }
    bool canSpawnLightningBolt() const { return !enableSnow && enableRain; }
    virtual bool isHighHumidity() const { return rainfall > 0.85f; }
    virtual float getSpawningChance() const { return 0.1f; }
    int_t getIntRainfall() const;
    int_t getIntTemperature() const;
    float getFloatRainfall() const { return rainfall; }
    float getFloatTemperature() const { return temperature; }
    virtual int_t getBiomeGrassColor() const;
    virtual int_t getBiomeFoliageColor() const;
    virtual void decorate(World *world, Random &random, int_t chunkX, int_t chunkZ);

    jstring biomeName;
    int_t color;
    byte_t topBlock;
    byte_t fillerBlock;
    int_t texturePackSelected; // field_6502_q in 1.2.5
    float minHeight;
    float maxHeight;
    float temperature;
    float rainfall;
    int_t waterColorMultiplier;
    int_t biomeID;
    BiomeDecorator *biomeDecorator;

protected:
    BiomeGenBase();
    explicit BiomeGenBase(int_t id);
    virtual ~BiomeGenBase();

    virtual BiomeDecorator *createBiomeDecorator();
    int_t getCachedBiomeColorIndex() const { return cachedBiomeColorIndex; }
    BiomeGenBase *registerBiome(int_t id);
    BiomeGenBase *setTemperatureRainfall(float temperatureValue, float rainfallValue);
    BiomeGenBase *setMinMaxHeight(float minHeightValue, float maxHeightValue);
    BiomeGenBase *setDisableRain();
    BiomeGenBase *func_50086_b();
    BiomeGenBase *setEnableSnow();
    BiomeGenBase *setBiomeName(const jstring &name);
    BiomeGenBase *func_4124_a(int_t colorValue);
    BiomeGenBase *getBiomeAtCoords(int_t colorValue);
    BiomeGenBase *setColor(int_t colorValue);

    std::vector<SpawnListEntry> spawnableMonsterList;
    std::vector<SpawnListEntry> spawnableCreatureList;
    std::vector<SpawnListEntry> spawnableWaterCreatureList;

    // Java 1.2.5 keeps these generators as persistent BiomeGenBase fields.
    // Some subclasses return these borrowed instances while others allocate a
    // generator per call, so callers must use releaseWorldGenForTrees().
    WorldGenerator *worldGenTrees;
    WorldGenerator *worldGenBigTree;
    WorldGenerator *worldGenForest;
    WorldGenerator *worldGenSwamp;

private:
    void updateBiomeColorIndex();
    int_t cachedBiomeColorIndex;
    bool enableSnow;
    bool enableRain;
};
