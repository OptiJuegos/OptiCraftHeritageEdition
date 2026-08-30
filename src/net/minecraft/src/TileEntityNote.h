#pragma once

#include "TileEntity.h"

class World;

class TileEntityNote : public TileEntity {
public:
    TileEntityNote();

    void readFromNBT(NBTTagCompound* nbttagcompound) override;
    void writeToNBT(NBTTagCompound* nbttagcompound) override;
    void changePitch();
    void triggerNote(World* world, int i, int j, int k);

    byte_t note;
    bool previousRedstoneState;
};
