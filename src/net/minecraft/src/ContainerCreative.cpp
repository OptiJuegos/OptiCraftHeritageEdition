#include "ContainerCreative.h"

#include "Block.h"
#include "BlockDeadBush.h"
#include "BlockFlower.h"
#include "BlockGrass.h"
#include "BlockLeaves.h"
#include "BlockLilyPad.h"
#include "BlockMycelium.h"
#include "BlockTallGrass.h"
#include "BlockVine.h"
#include "EntityList.h"
#include "EntityPlayer.h"
#include "GuiContainerCreative.h"
#include "InventoryBasic.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "Slot.h"

#include <cmath>

ContainerCreative::ContainerCreative(EntityPlayer *player)
{
    Block *blocks[] = {
        Block::cobblestone, Block::stone, Block::oreDiamond, Block::oreGold, Block::oreIron, Block::oreCoal,
        Block::oreLapis, Block::oreRedstone, Block::stoneBrick, Block::stoneBrick, Block::stoneBrick,
        Block::stoneBrick, Block::blockClay, Block::blockDiamond, Block::blockGold, Block::blockSteel,
        Block::bedrock, Block::blockLapis, Block::brick, Block::cobblestoneMossy, Block::stairSingle,
        Block::stairSingle, Block::stairSingle, Block::stairSingle, Block::stairSingle, Block::stairSingle,
        Block::obsidian, Block::netherrack, Block::slowSand, Block::glowStone, Block::wood, Block::wood,
        Block::wood, Block::wood, Block::leaves, Block::leaves, Block::leaves, Block::leaves, Block::dirt,
        Block::grass, Block::sand, Block::sandStone, Block::sandStone, Block::sandStone, Block::gravel,
        Block::web, Block::planks, Block::planks, Block::planks, Block::planks, Block::sapling, Block::sapling,
        Block::sapling, Block::sapling, Block::deadBush, Block::sponge, Block::ice, Block::blockSnow,
        Block::plantYellow, Block::plantRed, Block::mushroomBrown, Block::mushroomRed, Block::cactus,
        Block::melon, Block::pumpkin, Block::pumpkinLantern, Block::vine, Block::fenceIron, Block::thinGlass,
        Block::netherBrick, Block::netherFence, Block::stairsNetherBrick, Block::whiteStone, Block::mycelium,
        Block::waterlily, Block::tallGrass, Block::tallGrass, Block::chest, Block::workbench, Block::glass,
        Block::tnt, Block::bookShelf, Block::cloth, Block::cloth, Block::cloth, Block::cloth, Block::cloth,
        Block::cloth, Block::cloth, Block::cloth, Block::cloth, Block::cloth, Block::cloth, Block::cloth,
        Block::cloth, Block::cloth, Block::cloth, Block::cloth, Block::dispenser, Block::stoneOvenIdle,
        Block::musicBlock, Block::jukebox, Block::pistonStickyBase, Block::pistonBase, Block::fence,
        Block::fenceGate, Block::ladder, Block::rail, Block::railPowered, Block::railDetector, Block::torchWood,
        Block::stairCompactPlanks, Block::stairCompactCobblestone, Block::stairsBrick,
        Block::stairsStoneBrickSmooth, Block::lever, Block::pressurePlateStone, Block::pressurePlatePlanks,
        Block::torchRedstoneActive, Block::button, Block::trapdoor, Block::enchantmentTable,
        Block::redstoneLampIdle
    };

    int_t clothMeta = 0;
    int_t slabMeta = 0;
    int_t logMeta = 0;
    int_t plankMeta = 0;
    int_t saplingMeta = 0;
    int_t stoneBrickMeta = 0;
    int_t sandstoneMeta = 0;
    int_t leavesMeta = 0;
    int_t tallGrassMeta = 1;

    for (Block *block : blocks)
    {
        if (block == nullptr)
            continue;

        int_t meta = 0;
        if (block == Block::cloth) meta = clothMeta++;
        else if (block == Block::stairSingle) meta = slabMeta++;
        else if (block == Block::wood) meta = logMeta++;
        else if (block == Block::planks) meta = plankMeta++;
        else if (block == Block::sapling) meta = saplingMeta++;
        else if (block == Block::stoneBrick) meta = stoneBrickMeta++;
        else if (block == Block::sandStone) meta = sandstoneMeta++;
        else if (block == Block::tallGrass) meta = tallGrassMeta++;
        else if (block == Block::leaves) meta = leavesMeta++;
        itemList.push_back(new ItemStack(block, 1, meta));
    }

    for (int_t id = 256; id < Item::ITEM_LIST_SIZE; ++id)
    {
        Item *item = Item::itemsList[id];
        if (item != nullptr && item != Item::potion && item != Item::monsterPlacer)
            itemList.push_back(new ItemStack(item));
    }

    if (Item::dyePowder != nullptr)
    {
        for (int_t damage = 1; damage < 16; ++damage)
            itemList.push_back(new ItemStack(Item::dyePowder, 1, damage));
    }

    if (Item::monsterPlacer != nullptr)
    {
        for (int_t id : EntityList::getEntityEggIDs())
            itemList.push_back(new ItemStack(Item::monsterPlacer, 1, id));
    }

    InventoryBasic *creativeInventory = GuiContainerCreative::getInventory();
    for (int_t row = 0; row < 9; ++row)
    {
        for (int_t col = 0; col < 8; ++col)
            addSlot(new Slot(creativeInventory, col + row * 8, 8 + col * 18, 18 + row * 18));
    }

    for (int_t col = 0; col < 9; ++col)
        addSlot(new Slot(player->inventory, col, 8 + col * 18, 184));

    scrollTo(0.0f);
}

ContainerCreative::~ContainerCreative()
{
    InventoryBasic *creativeInventory = GuiContainerCreative::getInventory();
    if (creativeInventory != nullptr)
    {
        for (int_t i = 0; i < creativeInventory->getSizeInventory(); ++i)
            creativeInventory->setInventorySlotContents(i, nullptr);
    }

    for (ItemStack *stack : itemList)
        delete stack;
    itemList.clear();
}

bool ContainerCreative::isUsableByPlayer(EntityPlayer *)
{
    return true;
}

void ContainerCreative::scrollTo(float_t scroll)
{
    int_t rows = (int_t)itemList.size() / 8 - 8 + 1;
    if (rows < 0)
        rows = 0;
    int_t firstRow = (int_t)((double)(scroll * (float_t)rows) + 0.5);
    if (firstRow < 0)
        firstRow = 0;

    InventoryBasic *creativeInventory = GuiContainerCreative::getInventory();
    for (int_t row = 0; row < 9; ++row)
    {
        for (int_t col = 0; col < 8; ++col)
        {
            int_t itemIndex = col + (row + firstRow) * 8;
            creativeInventory->setInventorySlotContents(
                col + row * 8,
                itemIndex >= 0 && itemIndex < (int_t)itemList.size() ? itemList[itemIndex] : nullptr);
        }
    }
}

void ContainerCreative::retrySlotClick(int_t, int_t, bool, EntityPlayer *)
{
}
