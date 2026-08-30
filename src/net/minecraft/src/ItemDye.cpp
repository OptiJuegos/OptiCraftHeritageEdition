#include "ItemDye.h"
#include "java/Arithmetic.h"
#include "ItemStack.h"
#include "World.h"
#include "Block.h"
#include "BlockSapling.h"
#include "BlockCrops.h"
#include "BlockMushroom.h"
#include "BlockStem.h"
#include "BlockGrass.h"
#include "BlockTallGrass.h"
#include "BlockFlower.h"
#include "EntitySheep.h"
#include "BlockCloth.h"
#include "EntityPlayer.h"
#include "EntityLiving.h"
#include "MathHelper.h"

const std::string ItemDye::dyeColors[16] = {
    "black", "red", "green", "brown", "blue", "purple", "cyan", "silver",
    "gray", "pink", "lime", "yellow", "lightBlue", "magenta", "orange", "white"
};

const int ItemDye::worldChunkManager[16] = {
    0x1e1b1b, 0xb3312c, 0x3b511a, 0x51301a, 0x253192, 0x7b2fbe, 0x287697, 0x287697,
    0x434343, 0xd88198, 0x41cd34, 0xdecf2a, 0x6689d3, 0xc354cd, 0xeb8844, 0xf0f0f0
};

ItemDye::ItemDye(int i)
    : Item(i) {
    setHasSubtypes(true);
    setMaxDamage(0);
}

int ItemDye::getIconFromDamage(int i) {
    int j = MathHelper::clamp_int(i, 0, 15);
    return iconIndex + (j % 8) * 16 + j / 8;
}

std::string ItemDye::getItemNameIS(ItemStack* itemstack) {
    int damage = MathHelper::clamp_int(itemstack->getItemDamage(), 0, 15);
    return Item::getItemName() + "." + dyeColors[damage];
}

bool ItemDye::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    if (!entityplayer->canPlayerEdit(i, j, k)) {
        return false;
    }
    if (itemstack->getItemDamage() != 15) {
        return false;
    }

    int blockId = world->getBlockId(i, j, k);
    if (blockId == Block::sapling->blockID) {
        if (!world->multiplayerWorld) {
            static_cast<BlockSapling*>(Block::sapling)->growTree(world, i, j, k, world->rand);
            --itemstack->stackSize;
        }
        return true;
    }
    if (blockId == Block::mushroomBrown->blockID || blockId == Block::mushroomRed->blockID) {
        if (!world->multiplayerWorld &&
            static_cast<BlockMushroom*>(Block::blocksList[blockId])->fertilizeMushroom(world, i, j, k, world->rand)) {
            --itemstack->stackSize;
        }
        return true;
    }
    if (blockId == Block::melonStem->blockID || blockId == Block::pumpkinStem->blockID) {
        if (!world->multiplayerWorld) {
            static_cast<BlockStem*>(Block::blocksList[blockId])->fertilizeStem(world, i, j, k);
            --itemstack->stackSize;
        }
        return true;
    }
    if (blockId == Block::crops->blockID) {
        if (!world->multiplayerWorld) {
            static_cast<BlockCrops*>(Block::crops)->fertilize(world, i, j, k);
            --itemstack->stackSize;
        }
        return true;
    }
    if (blockId != Block::grass->blockID) {
        return false;
    }

    if (!world->multiplayerWorld) {
        --itemstack->stackSize;
        for (int attempt = 0; attempt < 128; ++attempt) {
            int x = i;
            int y = j + 1;
            int z = k;
            bool valid = true;
            for (int step = 0; step < attempt / 16; ++step) {
                x = JavaArithmetic::intAdd(x, itemRand.nextIntDifference(3));
                const int yDirection = itemRand.nextInt(3) - 1;
                const int yScale = itemRand.nextInt(3);
                y += yDirection * yScale / 2;
                z = JavaArithmetic::intAdd(z, itemRand.nextIntDifference(3));
                if (world->getBlockId(x, y - 1, z) != Block::grass->blockID || world->isBlockNormalCube(x, y, z)) {
                    valid = false;
                    break;
                }
            }
            if (!valid || world->getBlockId(x, y, z) != 0) {
                continue;
            }
            if (itemRand.nextInt(10) != 0) {
                world->setBlockAndMetadataWithNotify(x, y, z, Block::tallGrass->blockID, 1);
            } else if (itemRand.nextInt(3) != 0) {
                world->setBlockWithNotify(x, y, z, Block::plantYellow->blockID);
            } else {
                world->setBlockWithNotify(x, y, z, Block::plantRed->blockID);
            }
        }
    }
    return true;
}

void ItemDye::useItemOnEntity(ItemStack* itemstack, EntityLiving* entityliving) {
    EntitySheep* entitysheep = dynamic_cast<EntitySheep*>(entityliving);
    if (entitysheep) {
        int i = BlockCloth::getColorFromDamage(itemstack->getItemDamage());
        if (!entitysheep->getSheared() && entitysheep->getFleeceColor() != i) {
            entitysheep->setFleeceColor(i);
            itemstack->stackSize--;
        }
    }
}

void ItemDye::saddleEntity(ItemStack* itemstack, EntityLiving* entityliving) {
    useItemOnEntity(itemstack, entityliving);
}
