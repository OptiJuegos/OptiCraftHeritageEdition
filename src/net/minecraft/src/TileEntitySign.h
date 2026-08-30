#pragma once

#include "TileEntity.h"
#include "java/String.h"

class TileEntitySign : public TileEntity {
public:
    TileEntitySign();

    void writeToNBT(NBTTagCompound* nbttagcompound) override;
    void readFromNBT(NBTTagCompound* nbttagcompound) override;

    bool isEditable() const;
    void setEditable(bool editable);

    jstring signText[4];
    int lineBeingEdited;

private:
    bool editable;
};
