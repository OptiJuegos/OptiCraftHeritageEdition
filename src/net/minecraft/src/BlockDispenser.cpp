#include "BlockDispenser.h"
#include "Material.h"
#include "Block.h"
#include "World.h"
#include "IBlockAccess.h"
#include "TileEntityDispenser.h"
#include "EntityPlayer.h"
#include "EntityLiving.h"
#include "ItemStack.h"
#include "Item.h"
#include "EntityArrow.h"
#include "EntityEgg.h"
#include "EntitySnowball.h"
#include "NBTTagCompound.h"
#include "ItemMonsterPlacer.h"
#include "ItemPotion.h"
#include "EntitySmallFireball.h"
#include "EntityExpBottle.h"
#include "EntityPotion.h"
#include "EntityItem.h"
#include "MathHelper.h"
#include "TileEntity.h"

BlockDispenser::BlockDispenser(int_t i) : BlockContainer(i, Material::rock)
{
    blockIndexInTexture = 45;
}

int_t BlockDispenser::tickRate() { return 4; }

int_t BlockDispenser::idDropped(int_t i, Random &random)
{
    return Block::dispenser->blockID;
}

void BlockDispenser::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
    BlockContainer::onBlockAdded(world, i, j, k);
    setDispenserDefaultDirection(world, i, j, k);
}

void BlockDispenser::setDispenserDefaultDirection(World *world, int_t i, int_t j, int_t k)
{
    if (world->multiplayerWorld) return;
    int_t l  = world->getBlockId(i, j, k - 1);
    int_t i1 = world->getBlockId(i, j, k + 1);
    int_t j1 = world->getBlockId(i - 1, j, k);
    int_t k1 = world->getBlockId(i + 1, j, k);
    byte_t byte0 = 3;
    if (Block::opaqueCubeLookup[l]  && !Block::opaqueCubeLookup[i1]) byte0 = 3;
    if (Block::opaqueCubeLookup[i1] && !Block::opaqueCubeLookup[l])  byte0 = 2;
    if (Block::opaqueCubeLookup[j1] && !Block::opaqueCubeLookup[k1]) byte0 = 5;
    if (Block::opaqueCubeLookup[k1] && !Block::opaqueCubeLookup[j1]) byte0 = 4;
    world->setBlockMetadataWithNotify(i, j, k, byte0);
}

int_t BlockDispenser::getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    if (l == 1) return blockIndexInTexture + 17;
    if (l == 0) return blockIndexInTexture + 17;
    int_t i1 = iblockaccess->getBlockMetadata(i, j, k);
    if (l != i1) return blockIndexInTexture;
    return blockIndexInTexture + 1;
}

int_t BlockDispenser::getBlockTextureFromSide(int_t i)
{
    if (i == 1) return blockIndexInTexture + 17;
    if (i == 0) return blockIndexInTexture + 17;
    if (i == 3) return blockIndexInTexture + 1;
    return blockIndexInTexture;
}

bool BlockDispenser::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    if (world->multiplayerWorld) return true;
    TileEntityDispenser *tileentitydispenser = dynamic_cast<TileEntityDispenser*>(world->getBlockTileEntity(i, j, k));
    if (tileentitydispenser != nullptr)
        entityplayer->displayGUIDispenser(tileentitydispenser);
    return true;
}

void BlockDispenser::dispenseItem(World *world, int_t i, int_t j, int_t k, Random &random)
{
    int_t direction = world->getBlockMetadata(i, j, k);
    int_t xDirection = 0;
    int_t zDirection = 0;
    if (direction == 3) zDirection = 1;
    else if (direction == 2) zDirection = -1;
    else if (direction == 5) xDirection = 1;
    else xDirection = -1;

    TileEntityDispenser *dispenser = dynamic_cast<TileEntityDispenser*>(world->getBlockTileEntity(i, j, k));
    if (dispenser == nullptr)
        return;

    ItemStack *stack = dispenser->getRandomStackFromInventory();
    const double x = (double)i + (double)xDirection * 0.6 + 0.5;
    const double y = (double)j + 0.5;
    const double z = (double)k + (double)zDirection * 0.6 + 0.5;
    if (stack == nullptr)
    {
        world->isTrapdoorOpen(1001, i, j, k, 0);
        return;
    }

    bool stackTransferred = false;
    if (stack->itemID == Item::arrow->shiftedIndex)
    {
        EntityArrow *entity = new EntityArrow(world, x, y, z);
        entity->setArrowHeading(xDirection, 0.10000000149011612, zDirection, 1.1f, 6.0f);
        entity->doesArrowBelongToPlayer = true;
        if (!world->entityJoinedWorld(entity)) delete entity;
        world->isTrapdoorOpen(1002, i, j, k, 0);
    }
    else if (stack->itemID == Item::egg->shiftedIndex)
    {
        EntityEgg *entity = new EntityEgg(world, x, y, z);
        entity->setThrowableHeading(xDirection, 0.10000000149011612, zDirection, 1.1f, 6.0f);
        if (!world->entityJoinedWorld(entity)) delete entity;
        world->isTrapdoorOpen(1002, i, j, k, 0);
    }
    else if (stack->itemID == Item::snowball->shiftedIndex)
    {
        EntitySnowball *entity = new EntitySnowball(world, x, y, z);
        entity->setThrowableHeading(xDirection, 0.10000000149011612, zDirection, 1.1f, 6.0f);
        if (!world->entityJoinedWorld(entity)) delete entity;
        world->isTrapdoorOpen(1002, i, j, k, 0);
    }
    else if (stack->itemID == Item::potion->shiftedIndex && ItemPotion::isSplash(stack->getItemDamage()))
    {
        EntityPotion *entity = new EntityPotion(world, x, y, z, stack->getItemDamage());
        entity->setThrowableHeading(xDirection, 0.10000000149011612, zDirection, 1.375f, 3.0f);
        if (!world->entityJoinedWorld(entity)) delete entity;
        world->isTrapdoorOpen(1002, i, j, k, 0);
    }
    else if (stack->itemID == Item::expBottle->shiftedIndex)
    {
        EntityExpBottle *entity = new EntityExpBottle(world, x, y, z);
        entity->setThrowableHeading(xDirection, 0.10000000149011612, zDirection, 1.375f, 3.0f);
        if (!world->entityJoinedWorld(entity)) delete entity;
        world->isTrapdoorOpen(1002, i, j, k, 0);
    }
    else if (stack->itemID == Item::monsterPlacer->shiftedIndex)
    {
        ItemMonsterPlacer::spawnCreature(world, stack->getItemDamage(),
                                         x + (double)xDirection * 0.3, y - 0.3, z + (double)zDirection * 0.3);
        world->isTrapdoorOpen(1002, i, j, k, 0);
    }
    else if (stack->itemID == Item::fireballCharge->shiftedIndex)
    {
        const double motionX = (double)xDirection + random.nextGaussian() * 0.05;
        const double motionY = random.nextGaussian() * 0.05;
        const double motionZ = (double)zDirection + random.nextGaussian() * 0.05;
        EntitySmallFireball *entity = new EntitySmallFireball(world,
            x + (double)xDirection * 0.3, y, z + (double)zDirection * 0.3,
            motionX, motionY, motionZ);
        if (!world->entityJoinedWorld(entity)) delete entity;
        world->isTrapdoorOpen(1009, i, j, k, 0);
    }
    else
    {
        EntityItem *entity = new EntityItem(world, x, y - 0.3, z, stack);
        stackTransferred = true;
        const double speed = random.nextDouble() * 0.1 + 0.2;
        entity->motionX = (double)xDirection * speed;
        entity->motionY = 0.20000000298023224;
        entity->motionZ = (double)zDirection * speed;
        entity->motionX += random.nextGaussian() * 0.007499999832361937 * 6.0;
        entity->motionY += random.nextGaussian() * 0.007499999832361937 * 6.0;
        entity->motionZ += random.nextGaussian() * 0.007499999832361937 * 6.0;
        if (!world->entityJoinedWorld(entity))
        {
            // EntityItem owns stack even if insertion fails.
            delete entity;
        }
        world->isTrapdoorOpen(1000, i, j, k, 0);
    }

    if (!stackTransferred)
        delete stack;
    world->isTrapdoorOpen(2000, i, j, k, xDirection + 1 + (zDirection + 1) * 3);
}

void BlockDispenser::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
    if (l > 0 && Block::blocksList[l]->canProvidePower())
    {
        bool flag = world->isBlockIndirectlyGettingPowered(i, j, k) || world->isBlockIndirectlyGettingPowered(i, j + 1, k);
        if (flag) world->scheduleBlockUpdate(i, j, k, blockID, tickRate());
    }
}

void BlockDispenser::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    if (!world->multiplayerWorld &&
        (world->isBlockIndirectlyGettingPowered(i, j, k) || world->isBlockIndirectlyGettingPowered(i, j + 1, k)))
        dispenseItem(world, i, j, k, random);
}

TileEntity *BlockDispenser::getBlockEntity() { return new TileEntityDispenser(); }

void BlockDispenser::onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving)
{
    int_t l = MathHelper::floor_double((double)((entityliving->rotationYaw * 4.0f) / 360.0f) + 0.5) & 3;
    if (l == 0) world->setBlockMetadataWithNotify(i, j, k, 2);
    if (l == 1) world->setBlockMetadataWithNotify(i, j, k, 5);
    if (l == 2) world->setBlockMetadataWithNotify(i, j, k, 3);
    if (l == 3) world->setBlockMetadataWithNotify(i, j, k, 4);
}

void BlockDispenser::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
    TileEntityDispenser *tileentitydispenser = dynamic_cast<TileEntityDispenser*>(world->getBlockTileEntity(i, j, k));
    if (tileentitydispenser == nullptr)
    {
        BlockContainer::onBlockRemoval(world, i, j, k);
        return;
    }
    for (int_t l = 0; l < tileentitydispenser->getSizeInventory(); l++)
    {
        ItemStack *itemstack = tileentitydispenser->getStackInSlot(l);
        if (itemstack == nullptr) continue;
        float f  = random.nextFloat() * 0.8f + 0.1f;
        float f1 = random.nextFloat() * 0.8f + 0.1f;
        float f2 = random.nextFloat() * 0.8f + 0.1f;
        while (itemstack->stackSize > 0)
        {
            int_t i1 = random.nextInt(21) + 10;
            if (i1 > itemstack->stackSize) i1 = itemstack->stackSize;
            itemstack->stackSize -= i1;
            ItemStack *droppedStack = new ItemStack(itemstack->itemID, i1, itemstack->getItemDamage());
            if (itemstack->hasTagCompound())
            {
                NBTTagCompound *tagCopy = dynamic_cast<NBTTagCompound *>(itemstack->getTagCompound()->copy());
                droppedStack->setTagCompound(tagCopy);
            }
            EntityItem *entityitem = new EntityItem(world,
                (float)i + f, (float)j + f1, (float)k + f2, droppedStack);
            float f3 = 0.05f;
            entityitem->motionX = (float)random.nextGaussian() * f3;
            entityitem->motionY = (float)random.nextGaussian() * f3 + 0.2f;
            entityitem->motionZ = (float)random.nextGaussian() * f3;
            if (!world->entityJoinedWorld(entityitem))
                delete entityitem;
        }
    }
    BlockContainer::onBlockRemoval(world, i, j, k);
}
