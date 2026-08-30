#include "TileEntityRecordPlayer.h"
#include "NBTTagCompound.h"

TileEntityRecordPlayer::TileEntityRecordPlayer()
    : record(0)
{
}

void TileEntityRecordPlayer::readFromNBT(NBTTagCompound* nbttagcompound) {
    TileEntity::readFromNBT(nbttagcompound);
    record = nbttagcompound->getInteger("Record");
}

void TileEntityRecordPlayer::writeToNBT(NBTTagCompound* nbttagcompound) {
    TileEntity::writeToNBT(nbttagcompound);
    if (record > 0) {
        nbttagcompound->setInteger("Record", record);
    }
}
