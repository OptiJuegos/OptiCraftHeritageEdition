#include "BiomeGenBase.h"

#include "BiomeDecorator.h"

#include "BiomeGenBeach.h"
#include "BiomeGenHills.h"
#include "BiomeGenJungle.h"
#include "BiomeGenMushroomIsland.h"
#include "BiomeGenOcean.h"
#include "BiomeGenPlains.h"
#include "BiomeGenRiver.h"
#include "BiomeGenSnow.h"

#include "BiomeGenDesert.h"
#include "BiomeGenEnd.h"
#include "BiomeGenForest.h"
#include "BiomeGenHell.h"
#include "BiomeGenSwamp.h"
#include "BiomeGenTaiga.h"
#include "Block.h"
#include "BlockGrass.h"
#include "BlockTallGrass.h"
#include "ColorizerFoliage.h"
#include "ColorizerGrass.h"
#include "EntityChicken.h"
#include "EntityCow.h"
#include "EntityCreeper.h"
#include "EntityEnderman.h"
#include "EntityPig.h"
#include "EntitySheep.h"
#include "EntitySkeleton.h"
#include "EntitySlime.h"
#include "EntitySpider.h"
#include "EntitySquid.h"
#include "EntityZombie.h"
#include "MathHelper.h"
#include "WorldGenBigTree.h"
#include "WorldGenForest.h"
#include "WorldGenTallGrass.h"
#include "WorldGenSwamp.h"
#include "WorldGenTrees.h"
#include "WorldGenerator.h"
#include "platform/PlatformConfig.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <typeindex>

BiomeGenBase *BiomeGenBase::biomeList[BiomeGenBase::BIOME_REGISTRY_SIZE] = {};
BiomeGenBase *BiomeGenBase::ocean = nullptr;
BiomeGenBase *BiomeGenBase::plains = nullptr;
BiomeGenBase *BiomeGenBase::desert = nullptr;
BiomeGenBase *BiomeGenBase::extremeHills = nullptr;
BiomeGenBase *BiomeGenBase::forest = nullptr;
BiomeGenBase *BiomeGenBase::taiga = nullptr;
BiomeGenBase *BiomeGenBase::swampland = nullptr;
BiomeGenBase *BiomeGenBase::river = nullptr;
BiomeGenBase *BiomeGenBase::hell = nullptr;
BiomeGenBase *BiomeGenBase::sky = nullptr;
BiomeGenBase *BiomeGenBase::frozenOcean = nullptr;
BiomeGenBase *BiomeGenBase::frozenRiver = nullptr;
BiomeGenBase *BiomeGenBase::icePlains = nullptr;
BiomeGenBase *BiomeGenBase::iceMountains = nullptr;
BiomeGenBase *BiomeGenBase::mushroomIsland = nullptr;
BiomeGenBase *BiomeGenBase::mushroomIslandShore = nullptr;
BiomeGenBase *BiomeGenBase::beach = nullptr;
BiomeGenBase *BiomeGenBase::desertHills = nullptr;
BiomeGenBase *BiomeGenBase::forestHills = nullptr;
BiomeGenBase *BiomeGenBase::taigaHills = nullptr;
BiomeGenBase *BiomeGenBase::extremeHillsEdge = nullptr;
BiomeGenBase *BiomeGenBase::jungle = nullptr;
BiomeGenBase *BiomeGenBase::jungleHills = nullptr;


BiomeGenBase::BiomeGenBase()
    : color(0),
      topBlock((byte_t)Block::grass->blockID),
      fillerBlock((byte_t)Block::dirt->blockID),
      texturePackSelected(5169201),
      minHeight(0.1f),
      maxHeight(0.3f),
      temperature(0.5f),
      rainfall(0.5f),
      waterColorMultiplier(0xffffff),
      biomeID(-1),
      biomeDecorator(nullptr),
      worldGenTrees(new WorldGenTrees(false)),
      worldGenBigTree(new WorldGenBigTree(false)),
      worldGenForest(new WorldGenForest(false)),
      worldGenSwamp(new WorldGenSwamp()),
      cachedBiomeColorIndex(0),
      enableSnow(false),
      enableRain(true)
{
    updateBiomeColorIndex();
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntitySheep)), 12, 4, 4));
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntityPig)), 10, 4, 4));
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntityChicken)), 10, 4, 4));
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntityCow)), 8, 4, 4));
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntitySpider)), 10, 4, 4));
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntityZombie)), 10, 4, 4));
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntitySkeleton)), 10, 4, 4));
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntityCreeper)), 10, 4, 4));
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntitySlime)), 10, 4, 4));
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntityEnderman)), 1, 1, 4));
    spawnableWaterCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntitySquid)), 10, 4, 4));
    biomeDecorator = createBiomeDecorator();
}

BiomeGenBase::BiomeGenBase(int_t id) : BiomeGenBase()
{
    registerBiome(id);
}

BiomeGenBase *BiomeGenBase::registerBiome(int_t id)
{
    if (id < 0 || id >= BIOME_REGISTRY_SIZE)
        throw std::out_of_range("Biome id outside registry");
    if (biomeList[id] != nullptr && biomeList[id] != this)
        throw std::runtime_error("Biome id already occupied");
    biomeID = id;
    biomeList[id] = this;
    return this;
}

BiomeGenBase *BiomeGenBase::setTemperatureRainfall(float temperatureValue, float rainfallValue)
{
    if (temperatureValue > 0.1f && temperatureValue < 0.2f)
        throw std::invalid_argument("Please avoid temperatures in the range 0.1 - 0.2 because of snow");
    temperature = temperatureValue;
    rainfall = rainfallValue;
    updateBiomeColorIndex();
    return this;
}

BiomeGenBase *BiomeGenBase::setMinMaxHeight(float minHeightValue, float maxHeightValue)
{
    minHeight = minHeightValue;
    maxHeight = maxHeightValue;
    return this;
}

BiomeGenBase *BiomeGenBase::setDisableRain()
{
    enableRain = false;
    return this;
}

BiomeGenBase *BiomeGenBase::func_50086_b()
{
    return setEnableSnow();
}

BiomeGenBase *BiomeGenBase::setEnableSnow()
{
    enableSnow = true;
    return this;
}

BiomeGenBase *BiomeGenBase::setBiomeName(const jstring &name)
{
    biomeName = name;
    return this;
}

BiomeGenBase *BiomeGenBase::func_4124_a(int_t colorValue)
{
    return getBiomeAtCoords(colorValue);
}

BiomeGenBase *BiomeGenBase::getBiomeAtCoords(int_t value)
{
    texturePackSelected = value;
    return this;
}

BiomeGenBase *BiomeGenBase::setColor(int_t value)
{
    color = value;
    return this;
}

static int_t hsbToRgb(float h, float s, float b)
{
    int_t r = 0;
    int_t g = 0;
    int_t blue = 0;
    if (s == 0.0f)
    {
        r = g = blue = (int_t)(b * 255.0f + 0.5f);
    }
    else
    {
        const float hh = (h - (float)std::floor(h)) * 6.0f;
        const int_t i = (int_t)hh;
        const float f = hh - i;
        const float p = b * (1.0f - s);
        const float q = b * (1.0f - s * f);
        const float t = b * (1.0f - s * (1.0f - f));
        float rv;
        float gv;
        float bv;
        switch (i)
        {
            case 0: rv = b; gv = t; bv = p; break;
            case 1: rv = q; gv = b; bv = p; break;
            case 2: rv = p; gv = b; bv = t; break;
            case 3: rv = p; gv = q; bv = b; break;
            case 4: rv = t; gv = p; bv = b; break;
            default: rv = b; gv = p; bv = q; break;
        }
        r = (int_t)(rv * 255.0f + 0.5f);
        g = (int_t)(gv * 255.0f + 0.5f);
        blue = (int_t)(bv * 255.0f + 0.5f);
    }
    return (int_t)0xff000000 | (r << 16) | (g << 8) | blue;
}

int_t BiomeGenBase::getSkyColorByTemp(float value)
{
    value /= 3.0f;
    value = std::max(-1.0f, std::min(1.0f, value));
    return hsbToRgb(224.0f / 360.0f - value * 0.05f, 0.5f + value * 0.1f, 1.0f);
}

WorldGenerator *BiomeGenBase::getRandomWorldGenForTrees(Random &random)
{
    return random.nextInt(10) == 0 ? worldGenBigTree : worldGenTrees;
}

void BiomeGenBase::releaseWorldGenForTrees(WorldGenerator *generator)
{
    if (generator == nullptr || generator == worldGenTrees || generator == worldGenBigTree ||
        generator == worldGenForest || generator == worldGenSwamp)
        return;
    delete generator;
}

WorldGenerator *BiomeGenBase::func_48410_b(Random &)
{
    return new WorldGenTallGrass(Block::tallGrass->blockID, 1);
}

std::vector<SpawnListEntry> *BiomeGenBase::getSpawnableList(const EnumCreatureType &type)
{
    if (type.getCreatureTag() == EnumCreatureTypeTag::monster_tag)
        return &spawnableMonsterList;
    if (type.getCreatureTag() == EnumCreatureTypeTag::creature_tag)
        return &spawnableCreatureList;
    if (type.getCreatureTag() == EnumCreatureTypeTag::waterCreature_tag)
        return &spawnableWaterCreatureList;
    return nullptr;
}

BiomeGenBase::~BiomeGenBase()
{
    delete biomeDecorator;
    biomeDecorator = nullptr;
    delete worldGenTrees;
    delete worldGenBigTree;
    delete worldGenForest;
    delete worldGenSwamp;
    worldGenTrees = nullptr;
    worldGenBigTree = nullptr;
    worldGenForest = nullptr;
    worldGenSwamp = nullptr;
}

BiomeDecorator *BiomeGenBase::createBiomeDecorator()
{
    return new BiomeDecorator(this);
}

void BiomeGenBase::decorate(World *world, Random &random, int_t chunkX, int_t chunkZ)
{
    if (biomeDecorator != nullptr)
        biomeDecorator->decorate(world, random, chunkX, chunkZ);
}

int_t BiomeGenBase::getIntRainfall() const
{
    return (int_t)(rainfall * 65536.0f);
}

int_t BiomeGenBase::getIntTemperature() const
{
    return (int_t)(temperature * 65536.0f);
}

void BiomeGenBase::updateBiomeColorIndex()
{
    const double temp = (double)MathHelper::clamp_float(temperature, 0.0f, 1.0f);
    const double rain = (double)MathHelper::clamp_float(rainfall, 0.0f, 1.0f) * temp;
    const int_t colorX = (int_t)((1.0 - temp) * 255.0);
    const int_t colorY = (int_t)((1.0 - rain) * 255.0);
    cachedBiomeColorIndex = colorY << 8 | colorX;
}

int_t BiomeGenBase::getBiomeGrassColor() const
{
#if PLATFORM_PS2
    return ColorizerGrass::getGrassColorByIndex(cachedBiomeColorIndex);
#else
    const double temp = (double)MathHelper::clamp_float(temperature, 0.0f, 1.0f);
    const double rain = (double)MathHelper::clamp_float(rainfall, 0.0f, 1.0f);
    return ColorizerGrass::getGrassColor(temp, rain);
#endif
}

int_t BiomeGenBase::getBiomeFoliageColor() const
{
#if PLATFORM_PS2
    return ColorizerFoliage::getFoliageColorByIndex(cachedBiomeColorIndex);
#else
    const double temp = (double)MathHelper::clamp_float(temperature, 0.0f, 1.0f);
    const double rain = (double)MathHelper::clamp_float(rainfall, 0.0f, 1.0f);
    return ColorizerFoliage::getFoliageColor(temp, rain);
#endif
}

void BiomeGenBase::initialize()
{
    if (ocean != nullptr)
        return;

    ocean = (new BiomeGenOcean())->registerBiome(0)->setColor(112)->setBiomeName("Ocean")->setMinMaxHeight(-1.0f, 0.4f);
    plains = (new BiomeGenPlains())->registerBiome(1)->setColor(9286496)->setBiomeName("Plains")->setTemperatureRainfall(0.8f, 0.4f);
    desert = (new BiomeGenDesert())->registerBiome(2)->setColor(16421912)->setBiomeName("Desert")->setDisableRain()->setTemperatureRainfall(2.0f, 0.0f)->setMinMaxHeight(0.1f, 0.2f);
    extremeHills = (new BiomeGenHills())->registerBiome(3)->setColor(6316128)->setBiomeName("Extreme Hills")->setMinMaxHeight(0.2f, 1.3f)->setTemperatureRainfall(0.2f, 0.3f);
    forest = (new BiomeGenForest())->registerBiome(4)->setColor(353825)->setBiomeName("Forest")->getBiomeAtCoords(5159473)->setTemperatureRainfall(0.7f, 0.8f);
    taiga = (new BiomeGenTaiga())->registerBiome(5)->setColor(747097)->setBiomeName("Taiga")->getBiomeAtCoords(5159473)->setEnableSnow()->setTemperatureRainfall(0.05f, 0.8f)->setMinMaxHeight(0.1f, 0.4f);
    swampland = (new BiomeGenSwamp())->registerBiome(6)->setColor(522674)->setBiomeName("Swampland")->getBiomeAtCoords(9154376)->setMinMaxHeight(-0.2f, 0.1f)->setTemperatureRainfall(0.8f, 0.9f);
    river = (new BiomeGenRiver())->registerBiome(7)->setColor(255)->setBiomeName("River")->setMinMaxHeight(-0.5f, 0.0f);
    hell = (new BiomeGenHell())->registerBiome(8)->setColor(16711680)->setBiomeName("Hell")->setDisableRain()->setTemperatureRainfall(2.0f, 0.0f);
    sky = (new BiomeGenEnd())->registerBiome(9)->setColor(8421631)->setBiomeName("Sky")->setDisableRain();
    frozenOcean = (new BiomeGenOcean())->registerBiome(10)->setColor(9474208)->setBiomeName("FrozenOcean")->setEnableSnow()->setMinMaxHeight(-1.0f, 0.5f)->setTemperatureRainfall(0.0f, 0.5f);
    frozenRiver = (new BiomeGenRiver())->registerBiome(11)->setColor(10526975)->setBiomeName("FrozenRiver")->setEnableSnow()->setMinMaxHeight(-0.5f, 0.0f)->setTemperatureRainfall(0.0f, 0.5f);
    icePlains = (new BiomeGenSnow())->registerBiome(12)->setColor(16777215)->setBiomeName("Ice Plains")->setEnableSnow()->setTemperatureRainfall(0.0f, 0.5f);
    iceMountains = (new BiomeGenSnow())->registerBiome(13)->setColor(10526880)->setBiomeName("Ice Mountains")->setEnableSnow()->setMinMaxHeight(0.2f, 1.2f)->setTemperatureRainfall(0.0f, 0.5f);
    mushroomIsland = (new BiomeGenMushroomIsland())->registerBiome(14)->setColor(16711935)->setBiomeName("MushroomIsland")->setTemperatureRainfall(0.9f, 1.0f)->setMinMaxHeight(0.2f, 1.0f);
    mushroomIslandShore = (new BiomeGenMushroomIsland())->registerBiome(15)->setColor(10486015)->setBiomeName("MushroomIslandShore")->setTemperatureRainfall(0.9f, 1.0f)->setMinMaxHeight(-1.0f, 0.1f);
    beach = (new BiomeGenBeach())->registerBiome(16)->setColor(16440917)->setBiomeName("Beach")->setTemperatureRainfall(0.8f, 0.4f)->setMinMaxHeight(0.0f, 0.1f);
    desertHills = (new BiomeGenDesert())->registerBiome(17)->setColor(13786898)->setBiomeName("DesertHills")->setDisableRain()->setTemperatureRainfall(2.0f, 0.0f)->setMinMaxHeight(0.2f, 0.7f);
    forestHills = (new BiomeGenForest())->registerBiome(18)->setColor(2250012)->setBiomeName("ForestHills")->getBiomeAtCoords(5159473)->setTemperatureRainfall(0.7f, 0.8f)->setMinMaxHeight(0.2f, 0.6f);
    taigaHills = (new BiomeGenTaiga())->registerBiome(19)->setColor(1456435)->setBiomeName("TaigaHills")->setEnableSnow()->getBiomeAtCoords(5159473)->setTemperatureRainfall(0.05f, 0.8f)->setMinMaxHeight(0.2f, 0.7f);
    extremeHillsEdge = (new BiomeGenHills())->registerBiome(20)->setColor(7501978)->setBiomeName("Extreme Hills Edge")->setMinMaxHeight(0.2f, 0.8f)->setTemperatureRainfall(0.2f, 0.3f);
    jungle = (new BiomeGenJungle())->registerBiome(21)->setColor(5470985)->setBiomeName("Jungle")->getBiomeAtCoords(5470985)->setTemperatureRainfall(1.2f, 0.9f)->setMinMaxHeight(0.2f, 0.4f);
    jungleHills = (new BiomeGenJungle())->registerBiome(22)->setColor(2900485)->setBiomeName("JungleHills")->getBiomeAtCoords(5470985)->setTemperatureRainfall(1.2f, 0.9f)->setMinMaxHeight(1.8f, 0.2f);

}

void BiomeGenBase::cleanup()
{
    for (int_t i = 0; i < BIOME_REGISTRY_SIZE; ++i)
    {
        delete biomeList[i];
        biomeList[i] = nullptr;
    }

    ocean = plains = desert = extremeHills = forest = taiga = swampland = river = nullptr;
    hell = sky = frozenOcean = frozenRiver = icePlains = iceMountains = nullptr;
    mushroomIsland = mushroomIslandShore = beach = desertHills = forestHills = nullptr;
    taigaHills = extremeHillsEdge = jungle = jungleHills = nullptr;
}
