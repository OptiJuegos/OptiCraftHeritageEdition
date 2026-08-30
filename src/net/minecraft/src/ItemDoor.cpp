#include "ItemDoor.h"
#include "ItemStack.h"
#include "Material.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "MathHelper.h"
#include "World.h"

ItemDoor::ItemDoor(int i, Material* material)
    : Item(i), doorMaterial(material) {
    maxStackSize = 1;
}

bool ItemDoor::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    if (l != 1) {
        return false;
    }

    ++j;
    Block* block = doorMaterial == Material::wood ? Block::doorWood : Block::doorSteel;
    if (!entityplayer->canPlayerEdit(i, j, k) || !entityplayer->canPlayerEdit(i, j + 1, k)) {
        return false;
    }
    if (!block->canPlaceBlockAt(world, i, j, k)) {
        return false;
    }

    int direction = MathHelper::floor_double((double)(((entityplayer->rotationYaw + 180.0f) * 4.0f) / 360.0f) - 0.5) & 3;
    placeDoorBlock(world, i, j, k, direction, block);
    --itemstack->stackSize;
    return true;
}

void ItemDoor::placeDoorBlock(World* world, int i, int j, int k, int direction, Block* block) {
    int offsetX = 0;
    int offsetZ = 0;
    if (direction == 0) offsetZ = 1;
    if (direction == 1) offsetX = -1;
    if (direction == 2) offsetZ = -1;
    if (direction == 3) offsetX = 1;

    int leftSolid = (world->isBlockNormalCube(i - offsetX, j, k - offsetZ) ? 1 : 0)
                  + (world->isBlockNormalCube(i - offsetX, j + 1, k - offsetZ) ? 1 : 0);
    int rightSolid = (world->isBlockNormalCube(i + offsetX, j, k + offsetZ) ? 1 : 0)
                   + (world->isBlockNormalCube(i + offsetX, j + 1, k + offsetZ) ? 1 : 0);
    bool leftDoor = world->getBlockId(i - offsetX, j, k - offsetZ) == block->blockID
                 || world->getBlockId(i - offsetX, j + 1, k - offsetZ) == block->blockID;
    bool rightDoor = world->getBlockId(i + offsetX, j, k + offsetZ) == block->blockID
                  || world->getBlockId(i + offsetX, j + 1, k + offsetZ) == block->blockID;
    bool hingeRight = (leftDoor && !rightDoor) || (rightSolid > leftSolid);

    world->editingBlocks = true;
    world->setBlockAndMetadataWithNotify(i, j, k, block->blockID, direction);
    world->setBlockAndMetadataWithNotify(i, j + 1, k, block->blockID, 8 | (hingeRight ? 1 : 0));
    world->editingBlocks = false;
    world->notifyBlocksOfNeighborChange(i, j, k, block->blockID);
    world->notifyBlocksOfNeighborChange(i, j + 1, k, block->blockID);
}

