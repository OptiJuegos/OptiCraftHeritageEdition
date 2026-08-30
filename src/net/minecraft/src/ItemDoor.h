#pragma once

#include "Item.h"

class Material;
class Block;
class World;

class ItemDoor : public Item {
public:
    ItemDoor(int id, Material* material);

    bool onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) override;
    static void placeDoorBlock(World* world, int i, int j, int k, int direction, Block* block);

private:
    Material* doorMaterial;
};
