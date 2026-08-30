#pragma once

#include "java/Type.h"
#include "java/String.h"
#include "java/HashSet.h"
#include <functional>
#include <map>
#include <typeindex>
#include <vector>
#include "EntityEggInfo.h"

class World;
class Entity;
class NBTTagCompound;

struct EntityEggIdHash
{
    int_t operator()(int_t value) const { return value; }
};

struct EntityEggIdEqual
{
    bool operator()(int_t lhs, int_t rhs) const { return lhs == rhs; }
};

// net.minecraft.src.EntityList
class EntityList
{
public:
    static Entity *createEntityInWorld(jstring s, World *world);
    static Entity *createEntityByName(jstring s, World *world) { return createEntityInWorld(s, world); }
    static Entity *createEntityFromNBT(NBTTagCompound *nbttagcompound, World *world);
    static Entity *createEntity(int_t i, World *world);
    static Entity *createEntityByID(int_t i, World *world) { return createEntity(i, world); }
    static int_t getEntityID(Entity *entity);
    static jstring getEntityString(Entity *entity);
    static jstring getStringFromID(int_t id);
    static const EntityEggInfo *getEggInfo(int_t id);
    static std::vector<int_t> getEntityEggIDs();

    // Java addMapping(Class, String, int) -> C++ template; factory replaces reflection
    template<typename T>
    static void addMapping(jstring s, int_t i)
    {
        auto factory = [](World *w) -> Entity * { return new T(w); };
        stringToClassMapping[s] = factory;
        classToStringMapping[std::type_index(typeid(T))] = s;
        IDtoClassMapping[i] = factory;
        classToIDMapping[std::type_index(typeid(T))] = i;
    }

    template<typename T>
    static void addMapping(jstring s, int_t i, int_t primaryColor, int_t secondaryColor)
	{
		addMapping<T>(s, i);
		entityEggs.insert_or_assign(i, EntityEggInfo(i, primaryColor, secondaryColor));
		entityEggOrder.add(i);
	}

private:
    static std::map<int_t, EntityEggInfo> entityEggs;
    static JavaHashSet<int_t, EntityEggIdHash, EntityEggIdEqual> entityEggOrder;
    static std::map<jstring, std::function<Entity *(World *)>> stringToClassMapping;
    static std::map<std::type_index, jstring> classToStringMapping;
    static std::map<int_t, std::function<Entity *(World *)>> IDtoClassMapping;
    static std::map<std::type_index, int_t> classToIDMapping;
};
