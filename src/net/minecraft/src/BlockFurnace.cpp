#include "BlockFurnace.h"
#include "Material.h"
#include "Block.h"
#include "World.h"
#include "IBlockAccess.h"
#include "TileEntityFurnace.h"
#include "TileEntity.h"
#include "EntityPlayer.h"
#include "EntityLiving.h"
#include "MathHelper.h"
#include "IInventory.h"
#include "ItemStack.h"
#include "EntityItem.h"

bool BlockFurnace::keepFurnaceInventory = false;

BlockFurnace::BlockFurnace(int_t i, bool flag) : BlockContainer(i, Material::rock)
{
    isActive = flag;
    blockIndexInTexture = 45;
}

int_t BlockFurnace::idDropped(int_t i, Random &random)
{
    return Block::stoneOvenIdle->blockID;
}

void BlockFurnace::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
    // keepFurnaceInventory is only set while updateFurnaceBlockState swaps the idle
    // and active block ids, and that function re-installs the *existing*
    // TileEntityFurnace afterwards.  The tile entity BlockContainer::onBlockAdded
    // allocates here is therefore redundant: Java let the GC swallow that orphan once
    // the original was re-registered, but in C++ it owns memory, reaches the chunk
    // slot first and makes Chunk::setChunkBlockTileEntity delete the original - which
    // the open ContainerFurnace/GuiFurnace and their Slots still point at.  The next
    // EntityPlayer::onUpdate then runs craftingInventory->isUsableByPlayer(), which
    // tail-calls canInteractWith() through the freed vtable (Wii: "Exception (ISI)
    // occurred!" with PC set to ASCII bytes from whatever reused the block).
    if (keepFurnaceInventory)
        Block::onBlockAdded(world, i, j, k);
    else
        BlockContainer::onBlockAdded(world, i, j, k);
    setDefaultDirection(world, i, j, k);
}

void BlockFurnace::setDefaultDirection(World *world, int_t i, int_t j, int_t k)
{
    if (world->multiplayerWorld) return;
    int_t l  = world->getBlockId(i, j, k - 1);
    int_t i1 = world->getBlockId(i, j, k + 1);
    int_t j1 = world->getBlockId(i - 1, j, k);
    int_t k1 = world->getBlockId(i + 1, j, k);
    byte_t byte0 = 3;
    if (Block::opaqueCubeLookup[l] && !Block::opaqueCubeLookup[i1])  byte0 = 3;
    if (Block::opaqueCubeLookup[i1] && !Block::opaqueCubeLookup[l])  byte0 = 2;
    if (Block::opaqueCubeLookup[j1] && !Block::opaqueCubeLookup[k1]) byte0 = 5;
    if (Block::opaqueCubeLookup[k1] && !Block::opaqueCubeLookup[j1]) byte0 = 4;
    world->setBlockMetadataWithNotify(i, j, k, byte0);
}

int_t BlockFurnace::getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    if (l == 1) return blockIndexInTexture + 17;
    if (l == 0) return blockIndexInTexture + 17;
    int_t i1 = iblockaccess->getBlockMetadata(i, j, k);
    if (l != i1) return blockIndexInTexture;
    if (isActive) return blockIndexInTexture + 16;
    return blockIndexInTexture - 1;
}

void BlockFurnace::randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    if (!isActive) return;
    int_t l  = world->getBlockMetadata(i, j, k);
    float f  = (float)i + 0.5f;
    float f1 = (float)j + 0.0f + (random.nextFloat() * 6.0f) / 16.0f;
    float f2 = (float)k + 0.5f;
    float f3 = 0.52f;
    float f4 = random.nextFloat() * 0.6f - 0.3f;
    if (l == 4)
    {
        world->spawnParticle("smoke", f - f3, f1, f2 + f4, 0.0, 0.0, 0.0);
        world->spawnParticle("flame", f - f3, f1, f2 + f4, 0.0, 0.0, 0.0);
    }
    else if (l == 5)
    {
        world->spawnParticle("smoke", f + f3, f1, f2 + f4, 0.0, 0.0, 0.0);
        world->spawnParticle("flame", f + f3, f1, f2 + f4, 0.0, 0.0, 0.0);
    }
    else if (l == 2)
    {
        world->spawnParticle("smoke", f + f4, f1, f2 - f3, 0.0, 0.0, 0.0);
        world->spawnParticle("flame", f + f4, f1, f2 - f3, 0.0, 0.0, 0.0);
    }
    else if (l == 3)
    {
        world->spawnParticle("smoke", f + f4, f1, f2 + f3, 0.0, 0.0, 0.0);
        world->spawnParticle("flame", f + f4, f1, f2 + f3, 0.0, 0.0, 0.0);
    }
}

int_t BlockFurnace::getBlockTextureFromSide(int_t i)
{
    if (i == 1) return blockIndexInTexture + 17;
    if (i == 0) return blockIndexInTexture + 17;
    if (i == 3) return blockIndexInTexture - 1;
    return blockIndexInTexture;
}

bool BlockFurnace::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    if (world->multiplayerWorld) return true;
    TileEntityFurnace *tileentityfurnace = (TileEntityFurnace*)world->getBlockTileEntity(i, j, k);
    // Java threw an NPE here and the game caught it on the next tick; the C++
    // path builds ContainerFurnace with a null IInventory and the first
    // Slot::getStack() call dereferences it. Refusing the interaction leaves the
    // block usable once its tile entity is back.
    if (tileentityfurnace == nullptr) return true;
    entityplayer->displayGUIFurnace(tileentityfurnace);
    return true;
}

void BlockFurnace::updateFurnaceBlockState(bool flag, World *world, int_t i, int_t j, int_t k)
{
    int_t l = world->getBlockMetadata(i, j, k);
    TileEntity *tileentity = world->getBlockTileEntity(i, j, k);
    keepFurnaceInventory = true;
    if (flag)
        world->setBlockWithNotify(i, j, k, Block::stoneOvenActive->blockID);
    else
        world->setBlockWithNotify(i, j, k, Block::stoneOvenIdle->blockID);
    keepFurnaceInventory = false;
    world->setBlockMetadataWithNotify(i, j, k, l);
    if (tileentity == nullptr) return;
    tileentity->validate();
    world->setBlockTileEntity(i, j, k, tileentity);
}

TileEntity *BlockFurnace::getBlockEntity()
{
    return new TileEntityFurnace();
}

void BlockFurnace::onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving)
{
    int_t l = MathHelper::floor_double((double)((entityliving->rotationYaw * 4.0f) / 360.0f) + 0.5) & 3;
    if (l == 0) world->setBlockMetadataWithNotify(i, j, k, 2);
    if (l == 1) world->setBlockMetadataWithNotify(i, j, k, 5);
    if (l == 2) world->setBlockMetadataWithNotify(i, j, k, 3);
    if (l == 3) world->setBlockMetadataWithNotify(i, j, k, 4);
}

void BlockFurnace::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
    if (!keepFurnaceInventory)
    {
        TileEntityFurnace *tileentityfurnace = (TileEntityFurnace*)world->getBlockTileEntity(i, j, k);
        if (tileentityfurnace == nullptr)
        {
            BlockContainer::onBlockRemoval(world, i, j, k);
            return;
        }
        for (int_t l = 0; l < tileentityfurnace->getSizeInventory(); l++)
        {
            ItemStack *itemstack = tileentityfurnace->getStackInSlot(l);
            if (itemstack == nullptr) continue;
            float f  = furnaceRand.nextFloat() * 0.8f + 0.1f;
            float f1 = furnaceRand.nextFloat() * 0.8f + 0.1f;
            float f2 = furnaceRand.nextFloat() * 0.8f + 0.1f;
            while (itemstack->stackSize > 0)
            {
                int_t i1 = furnaceRand.nextInt(21) + 10;
                if (i1 > itemstack->stackSize) i1 = itemstack->stackSize;
                itemstack->stackSize -= i1;
                EntityItem *entityitem = new EntityItem(world,
                    (float)i + f, (float)j + f1, (float)k + f2,
                    new ItemStack(itemstack->itemID, i1, itemstack->getItemDamage()));
                float f3 = 0.05f;
                entityitem->motionX = (float)furnaceRand.nextGaussian() * f3;
                entityitem->motionY = (float)furnaceRand.nextGaussian() * f3 + 0.2f;
                entityitem->motionZ = (float)furnaceRand.nextGaussian() * f3;
                if (!world->entityJoinedWorld(entityitem))
                    delete entityitem;
            }
        }
    }
    BlockContainer::onBlockRemoval(world, i, j, k);
}
