#include "EntityList.h"
#include "platform/Log.h"
#include <stdexcept>

#include <iostream>

#include "SpawnerAnimals.h"
#include "EntityArrow.h"
#include "EntityBoat.h"
#include "EntityBlaze.h"
#include "EntityCaveSpider.h"
#include "EntityMagmaCube.h"
#include "EntityMooshroom.h"
#include "EntitySnowman.h"
#include "EntityChicken.h"
#include "EntityCow.h"
#include "EntityCreeper.h"
#include "EntityFallingSand.h"
#include "EntityFireball.h"
#include "EntitySmallFireball.h"
#include "EntityEnderPearl.h"
#include "EntityEnderEye.h"
#include "EntityEnderCrystal.h"
#include "EntityDragon.h"
#include "EntityEnderman.h"
#include "EntityPotion.h"
#include "EntityExpBottle.h"
#include "EntityGhast.h"
#include "EntityGiantZombie.h"
#include "EntityItem.h"
#include "EntityXPOrb.h"
#include "EntityVillager.h"
#include "EntityIronGolem.h"
#include "EntityLiving.h"
#include "EntityMinecart.h"
#include "EntityOcelot.h"
#include "EntityMob.h"
#include "EntityPainting.h"
#include "EntityPig.h"
#include "EntityPigZombie.h"
#include "EntitySheep.h"
#include "EntitySilverfish.h"
#include "EntitySkeleton.h"
#include "EntitySlime.h"
#include "EntitySnowball.h"
#include "EntitySpider.h"
#include "EntitySquid.h"
#include "EntityTNTPrimed.h"
#include "EntityWolf.h"
#include "EntityZombie.h"
#include "NBTTagCompound.h"
#include "World.h"

std::map<jstring, std::function<Entity *(World *)>> EntityList::stringToClassMapping;
std::map<std::type_index, jstring> EntityList::classToStringMapping;
std::map<int_t, std::function<Entity *(World *)>> EntityList::IDtoClassMapping;
std::map<std::type_index, int_t> EntityList::classToIDMapping;
std::map<int_t, EntityEggInfo> EntityList::entityEggs;
JavaHashSet<int_t, EntityEggIdHash, EntityEggIdEqual> EntityList::entityEggOrder;

Entity *EntityList::createEntityInWorld(jstring s, World *world)
{
    Entity *entity = nullptr;
    try
    {
        auto it = stringToClassMapping.find(s);
        if (it != stringToClassMapping.end())
        {
            entity = it->second(world);
        }
    }
    catch (std::exception &exception)
    {
        MC_LOG_ERROR("game", "%s\n", exception.what());
    }
    return entity;
}

Entity *EntityList::createEntityFromNBT(NBTTagCompound *nbttagcompound, World *world)
{
    Entity *entity = nullptr;
    try
    {
        auto it = stringToClassMapping.find(nbttagcompound->getString("id"));
        if (it != stringToClassMapping.end())
        {
            entity = it->second(world);
        }
    }
    catch (std::exception &exception)
    {
        MC_LOG_ERROR("game", "%s\n", exception.what());
    }
    if (entity != nullptr)
    {
        entity->readFromNBT(nbttagcompound);
    }
    else
    {
        MC_LOG_WARN("entity", "Skipping Entity with id %s\n", nbttagcompound->getString("id").c_str());
    }
    return entity;
}

Entity *EntityList::createEntity(int_t i, World *world)
{
    Entity *entity = nullptr;
    try
    {
        auto it = IDtoClassMapping.find(i);
        if (it != IDtoClassMapping.end())
        {
            entity = it->second(world);
        }
    }
    catch (std::exception &exception)
    {
        MC_LOG_ERROR("game", "%s\n", exception.what());
    }
    if (entity == nullptr)
    {
        MC_LOG_WARN("entity", "Skipping Entity with id %d\n", i);
    }
    return entity;
}

int_t EntityList::getEntityID(Entity *entity)
{
    // Java HashMap.get() returns null for unmapped classes; mirror that with a sentinel
    // instead of std::map::at() (which throws "invalid map<K,T> key" and crashed the game
    // whenever an unregistered entity — e.g. the player — was processed).
    auto it = classToIDMapping.find(std::type_index(typeid(*entity)));
    if (it == classToIDMapping.end())
        throw std::runtime_error("Entity class is not registered");
    return it->second;
}

jstring EntityList::getEntityString(Entity *entity)
{
    // Java returns null here for unregistered entities; Entity::addEntityID checks for that
    // (s.empty()) and skips saving them. Return empty instead of throwing from .at().
    auto it = classToStringMapping.find(std::type_index(typeid(*entity)));
    return it != classToStringMapping.end() ? it->second : jstring();
}


jstring EntityList::getStringFromID(int_t id)
{
    auto factoryIt = IDtoClassMapping.find(id);
    if (factoryIt == IDtoClassMapping.end())
        return jstring();

    for (const auto &entry : classToIDMapping)
    {
        if (entry.second == id)
        {
            auto nameIt = classToStringMapping.find(entry.first);
            return nameIt != classToStringMapping.end() ? nameIt->second : jstring();
        }
    }
    return jstring();
}

const EntityEggInfo *EntityList::getEggInfo(int_t id)
{
    auto it = entityEggs.find(id);
    return it != entityEggs.end() ? &it->second : nullptr;
}

static struct EntityListInit
{
    EntityListInit()
    {
        EntityList::addMapping<EntityArrow>("Arrow", 10);
        EntityList::addMapping<EntitySnowball>("Snowball", 11);
        EntityList::addMapping<EntityFireball>("Fireball", 12);
        EntityList::addMapping<EntitySmallFireball>("SmallFireball", 13);
        EntityList::addMapping<EntityEnderPearl>("ThrownEnderpearl", 14);
        EntityList::addMapping<EntityEnderEye>("EyeOfEnderSignal", 15);
        EntityList::addMapping<EntityPotion>("ThrownPotion", 16);
        EntityList::addMapping<EntityExpBottle>("ThrownExpBottle", 17);
        EntityList::addMapping<EntityItem>("Item", 1);
        EntityList::addMapping<EntityXPOrb>("XPOrb", 2);
        EntityList::addMapping<EntityPainting>("Painting", 9);
        EntityList::addMapping<EntityLiving>("Mob", 48);
        EntityList::addMapping<EntityMob>("Monster", 49);
        EntityList::addMapping<EntityCreeper>("Creeper", 50, 894731, 0);
        EntityList::addMapping<EntitySkeleton>("Skeleton", 51, 12698049, 4802889);
        EntityList::addMapping<EntitySpider>("Spider", 52, 3419431, 11013646);
        EntityList::addMapping<EntityGiantZombie>("Giant", 53);
        EntityList::addMapping<EntityZombie>("Zombie", 54, 44975, 7969893);
        EntityList::addMapping<EntitySlime>("Slime", 55, 5349438, 8306542);
        EntityList::addMapping<EntityGhast>("Ghast", 56, 16382457, 12369084);
        EntityList::addMapping<EntityPigZombie>("PigZombie", 57, 15373203, 5009705);
        EntityList::addMapping<EntityEnderman>("Enderman", 58, 1447446, 0);
        EntityList::addMapping<EntityCaveSpider>("CaveSpider", 59, 803406, 11013646);
        EntityList::addMapping<EntitySilverfish>("Silverfish", 60, 7237230, 3158064);
        EntityList::addMapping<EntityBlaze>("Blaze", 61, 16167425, 16775294);
        EntityList::addMapping<EntityMagmaCube>("LavaSlime", 62, 3407872, 16579584);
        EntityList::addMapping<EntityDragon>("EnderDragon", 63);
        EntityList::addMapping<EntityPig>("Pig", 90, 15771042, 14377823);
        EntityList::addMapping<EntitySheep>("Sheep", 91, 15198183, 16758197);
        EntityList::addMapping<EntityCow>("Cow", 92, 4470310, 10592673);
        EntityList::addMapping<EntityChicken>("Chicken", 93, 10592673, 16711680);
        EntityList::addMapping<EntitySquid>("Squid", 94, 2243405, 7375001);
        EntityList::addMapping<EntityWolf>("Wolf", 95, 14144467, 13545366);
        EntityList::addMapping<EntityMooshroom>("MushroomCow", 96, 10489616, 12040119);
        EntityList::addMapping<EntitySnowman>("SnowMan", 97);
        EntityList::addMapping<EntityOcelot>("Ozelot", 98, 15720061, 5653556);
        EntityList::addMapping<EntityIronGolem>("VillagerGolem", 99);
        EntityList::addMapping<EntityVillager>("Villager", 120, 5651507, 12422002);
        EntityList::addMapping<EntityEnderCrystal>("EnderCrystal", 200);
        EntityList::addMapping<EntityTNTPrimed>("PrimedTnt", 20);
        EntityList::addMapping<EntityFallingSand>("FallingSand", 21);
        EntityList::addMapping<EntityMinecart>("Minecart", 40);
        EntityList::addMapping<EntityBoat>("Boat", 41);

        // Register EntityLiving subclasses with SpawnerAnimals so performSpawning can instantiate them.
        // Java used reflection (Class.getConstructor().newInstance()); here we register lambdas.
#define SPAWN_REG(T) SpawnerAnimals_registerFactory(std::type_index(typeid(T)), [](World *w) -> EntityLiving * { return new T(w); })
        SPAWN_REG(EntityCreeper);
        SPAWN_REG(EntitySkeleton);
        SPAWN_REG(EntitySpider);
        SPAWN_REG(EntityCaveSpider);
        SPAWN_REG(EntityGiantZombie);
        SPAWN_REG(EntityZombie);
        SPAWN_REG(EntitySlime);
        SPAWN_REG(EntityGhast);
        SPAWN_REG(EntityPigZombie);
        SPAWN_REG(EntityEnderman);
        SPAWN_REG(EntitySilverfish);
        SPAWN_REG(EntityBlaze);
        SPAWN_REG(EntityMagmaCube);
        SPAWN_REG(EntityPig);
        SPAWN_REG(EntitySheep);
        SPAWN_REG(EntityCow);
        SPAWN_REG(EntityChicken);
        SPAWN_REG(EntitySquid);
        SPAWN_REG(EntityWolf);
        SPAWN_REG(EntityMooshroom);
        SPAWN_REG(EntitySnowman);
        SPAWN_REG(EntityOcelot);
        SPAWN_REG(EntityVillager);
        SPAWN_REG(EntityMob);
        SPAWN_REG(EntityLiving);
#undef SPAWN_REG
    }
} entityListInit;

std::vector<int_t> EntityList::getEntityEggIDs()
{
    return entityEggOrder.valuesInIterationOrder();
}
