#pragma once

#include "java/Type.h"

// net.minecraft.src.EntityEggInfo
class EntityEggInfo
{
public:
    EntityEggInfo(int_t spawnedId, int_t primary, int_t secondary)
        : spawnedID(spawnedId), primaryColor(primary), secondaryColor(secondary)
    {
    }

    int_t spawnedID;
    int_t primaryColor;
    int_t secondaryColor;
};
