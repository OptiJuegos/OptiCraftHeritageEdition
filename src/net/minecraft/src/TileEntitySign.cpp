#include "TileEntitySign.h"
#include "java/String.h"
#include "NBTTagCompound.h"

TileEntitySign::TileEntitySign() {
    signText[0] = "";
    signText[1] = "";
    signText[2] = "";
    signText[3] = "";
    lineBeingEdited = -1;
    editable = true;
}

void TileEntitySign::writeToNBT(NBTTagCompound* nbttagcompound) {
    TileEntity::writeToNBT(nbttagcompound);
    nbttagcompound->setString("Text1", signText[0]);
    nbttagcompound->setString("Text2", signText[1]);
    nbttagcompound->setString("Text3", signText[2]);
    nbttagcompound->setString("Text4", signText[3]);
}

void TileEntitySign::readFromNBT(NBTTagCompound* nbttagcompound) {
    editable = false;
    TileEntity::readFromNBT(nbttagcompound);
    for (int_t i = 0; i < 4; ++i)
    {
        signText[i] = nbttagcompound->getString("Text" + std::to_string(i + 1));
        if (String::utf16Length(signText[i]) > 15)
            signText[i] = String::truncateUtf16(signText[i], 15);
    }
}


bool TileEntitySign::isEditable() const {
    return editable;
}

void TileEntitySign::setEditable(bool value) {
    editable = value;
}
