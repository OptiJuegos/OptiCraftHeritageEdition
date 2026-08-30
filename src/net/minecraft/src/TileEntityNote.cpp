#include "TileEntityNote.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "Block.h"
#include "Material.h"

TileEntityNote::TileEntityNote() {
    note = 0;
    previousRedstoneState = false;
}

void TileEntityNote::readFromNBT(NBTTagCompound* nbttagcompound) {
    TileEntity::readFromNBT(nbttagcompound);
    note = nbttagcompound->getByte("note");
    if (note < 0) {
        note = 0;
    }
    if (note > 24) {
        note = 24;
    }
}

void TileEntityNote::writeToNBT(NBTTagCompound* nbttagcompound) {
    TileEntity::writeToNBT(nbttagcompound);
    nbttagcompound->setByte("note", note);
}

void TileEntityNote::changePitch() {
    note = (note + 1) % 25;
    onInventoryChanged();
}

void TileEntityNote::triggerNote(World* world, int i, int j, int k) {
    if (world->getBlockMaterial(i, j + 1, k) == Material::air) {
        Material* material = world->getBlockMaterial(i, j - 1, k);
        char byte0 = 0;
        if (material == Material::rock) {
            byte0 = 1;
        }
        if (material == Material::sand) {
            byte0 = 2;
        }
        if (material == Material::glass) {
            byte0 = 3;
        }
        if (material == Material::wood) {
            byte0 = 4;
        }
        world->playNoteAt(i, j, k, byte0, note);
    }
}
