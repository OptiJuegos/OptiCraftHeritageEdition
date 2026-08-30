#pragma once

#include "TileEntity.h"

class NBTTagCompound;

class TileEntityRecordPlayer : public TileEntity {
public:
    TileEntityRecordPlayer();

    void readFromNBT(NBTTagCompound* nbttagcompound) override;
    void writeToNBT(NBTTagCompound* nbttagcompound) override;

    int_t record;
};
