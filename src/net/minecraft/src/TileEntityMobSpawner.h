#pragma once

#include "TileEntity.h"
#include "platform/PlatformTuning.h"
#include <string>

class NBTTagCompound;

class TileEntityMobSpawner : public TileEntity {
public:
    TileEntityMobSpawner();

    std::string getMobID();
    void setMobID(const std::string &s);
    bool anyPlayerInRange();

    void readFromNBT(NBTTagCompound* nbttagcompound) override;
    void writeToNBT(NBTTagCompound* nbttagcompound) override;
    void updateEntity() override;

    int_t delay;
#if PLATFORM_FLOAT_VERTEX_MATH
    float yaw;
    float yaw2;
#else
    double yaw;
    double yaw2;
#endif

private:
    void updateDelay();

    std::string mobID;
};
