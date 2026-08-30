#pragma once

#include "java/Type.h"
#include "java/String.h"

// net.minecraft.src.WorldType
class WorldType
{
public:
    static constexpr int_t WORLD_TYPE_COUNT = 16;
    static WorldType *worldTypes[WORLD_TYPE_COUNT];
    static WorldType *DEFAULT;
    static WorldType *FLAT;
    static WorldType *DEFAULT_1_1;

    static void initialize();
    static void cleanup();

    jstring func_48628_a() const;
    jstring getTranslateName() const;
    int_t getGeneratorVersion() const;
    WorldType *func_48629_a(int_t generatorVersion);
    bool getCanBeCreated() const;
    bool func_48626_e() const;

    static WorldType *parseWorldType(const jstring &name);

private:
    WorldType(int_t id, const jstring &name, int_t generatorVersion = 0);

    WorldType *setCanBeCreated(bool value);
    WorldType *func_48631_f();

    int_t worldTypeId;
    jstring worldType;
    int_t generatorVersion;
    bool canBeCreated;
    bool field_48638_h;
};
