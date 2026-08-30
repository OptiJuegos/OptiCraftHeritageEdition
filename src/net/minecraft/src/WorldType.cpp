#include "WorldType.h"

WorldType *WorldType::worldTypes[WorldType::WORLD_TYPE_COUNT] = {};
WorldType *WorldType::DEFAULT = nullptr;
WorldType *WorldType::FLAT = nullptr;
WorldType *WorldType::DEFAULT_1_1 = nullptr;

WorldType::WorldType(int_t id, const jstring &name, int_t version)
    : worldTypeId(id),
      worldType(name),
      generatorVersion(version),
      canBeCreated(true),
      field_48638_h(false)
{
    if (id >= 0 && id < WORLD_TYPE_COUNT)
        worldTypes[id] = this;
}

void WorldType::initialize()
{
    if (DEFAULT != nullptr)
        return;

    DEFAULT = (new WorldType(0, "default", 1))->func_48631_f();
    FLAT = new WorldType(1, "flat");
    DEFAULT_1_1 = (new WorldType(8, "default_1_1", 0))->setCanBeCreated(false);
}

void WorldType::cleanup()
{
    for (int_t i = 0; i < WORLD_TYPE_COUNT; ++i)
    {
        delete worldTypes[i];
        worldTypes[i] = nullptr;
    }
    DEFAULT = nullptr;
    FLAT = nullptr;
    DEFAULT_1_1 = nullptr;
}

jstring WorldType::func_48628_a() const
{
    return worldType;
}

jstring WorldType::getTranslateName() const
{
    return jstring("generator.") + worldType;
}

int_t WorldType::getGeneratorVersion() const
{
    return generatorVersion;
}

WorldType *WorldType::func_48629_a(int_t version)
{
    return this == DEFAULT && version == 0 ? DEFAULT_1_1 : this;
}

WorldType *WorldType::setCanBeCreated(bool value)
{
    canBeCreated = value;
    return this;
}

bool WorldType::getCanBeCreated() const
{
    return canBeCreated;
}

WorldType *WorldType::func_48631_f()
{
    field_48638_h = true;
    return this;
}

bool WorldType::func_48626_e() const
{
    return field_48638_h;
}

WorldType *WorldType::parseWorldType(const jstring &name)
{
    for (int_t i = 0; i < WORLD_TYPE_COUNT; ++i)
    {
        WorldType *type = worldTypes[i];
        if (type != nullptr && String::equalsIgnoreCaseJava(type->worldType, name))
            return type;
    }
    return nullptr;
}
