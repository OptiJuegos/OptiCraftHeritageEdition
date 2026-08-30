#include "WorldGenDungeons.h"

#include "java/Arithmetic.h"
#include "Block.h"
#include "Item.h"
#include "ItemStack.h"
#include "Material.h"
#include "TileEntityChest.h"
#include "TileEntityMobSpawner.h"
#include "World.h"

bool WorldGenDungeons::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	int_t byte0 = 3;
	int_t l = JavaArithmetic::intAdd(random.nextInt(2), 2);
	int_t i1 = JavaArithmetic::intAdd(random.nextInt(2), 2);
	int_t j1 = 0;
	for (int_t k1 = JavaArithmetic::intSub(JavaArithmetic::intSub(i, l), 1); k1 <= JavaArithmetic::intAdd(JavaArithmetic::intAdd(i, l), 1); k1++)
	{
		for (int_t j2 = JavaArithmetic::intSub(j, 1); j2 <= JavaArithmetic::intAdd(JavaArithmetic::intAdd(j, byte0), 1); j2++)
		{
			for (int_t i3 = JavaArithmetic::intSub(JavaArithmetic::intSub(k, i1), 1); i3 <= JavaArithmetic::intAdd(JavaArithmetic::intAdd(k, i1), 1); i3++)
			{
				Material *material = world->getBlockMaterial(k1, j2, i3);
				if (j2 == JavaArithmetic::intSub(j, 1) && !material->isSolid())
					return false;
				if (j2 == JavaArithmetic::intAdd(JavaArithmetic::intAdd(j, byte0), 1) && !material->isSolid())
					return false;
				if ((k1 == JavaArithmetic::intSub(JavaArithmetic::intSub(i, l), 1) || k1 == JavaArithmetic::intAdd(JavaArithmetic::intAdd(i, l), 1) || i3 == JavaArithmetic::intSub(JavaArithmetic::intSub(k, i1), 1) || i3 == JavaArithmetic::intAdd(JavaArithmetic::intAdd(k, i1), 1)) && j2 == j && world->isAirBlock(k1, j2, i3) && world->isAirBlock(k1, JavaArithmetic::intAdd(j2, 1), i3))
					j1++;
			}
		}
	}
	if (j1 < 1 || j1 > 5)
		return false;
	for (int_t l1 = JavaArithmetic::intSub(JavaArithmetic::intSub(i, l), 1); l1 <= JavaArithmetic::intAdd(JavaArithmetic::intAdd(i, l), 1); l1++)
	{
		for (int_t k2 = JavaArithmetic::intAdd(j, byte0); k2 >= JavaArithmetic::intSub(j, 1); k2--)
		{
			for (int_t j3 = JavaArithmetic::intSub(JavaArithmetic::intSub(k, i1), 1); j3 <= JavaArithmetic::intAdd(JavaArithmetic::intAdd(k, i1), 1); j3++)
			{
				if (l1 == JavaArithmetic::intSub(JavaArithmetic::intSub(i, l), 1) || k2 == JavaArithmetic::intSub(j, 1) || j3 == JavaArithmetic::intSub(JavaArithmetic::intSub(k, i1), 1) || l1 == JavaArithmetic::intAdd(JavaArithmetic::intAdd(i, l), 1) || k2 == JavaArithmetic::intAdd(JavaArithmetic::intAdd(j, byte0), 1) || j3 == JavaArithmetic::intAdd(JavaArithmetic::intAdd(k, i1), 1))
				{
					if (k2 >= 0 && !world->getBlockMaterial(l1, JavaArithmetic::intSub(k2, 1), j3)->isSolid())
					{
						world->setBlockWithNotify(l1, k2, j3, 0);
						continue;
					}
					if (!world->getBlockMaterial(l1, k2, j3)->isSolid())
						continue;
					if (k2 == JavaArithmetic::intSub(j, 1) && random.nextInt(4) != 0)
						world->setBlockWithNotify(l1, k2, j3, Block::cobblestoneMossy->blockID);
					else
						world->setBlockWithNotify(l1, k2, j3, Block::cobblestone->blockID);
				}
				else
				{
					world->setBlockWithNotify(l1, k2, j3, 0);
				}
			}
		}
	}
	for (int_t i2 = 0; i2 < 2; i2++)
	{
		bool breakLabel0 = false;
		for (int_t l2 = 0; l2 < 3 && !breakLabel0; l2++)
		{
			int_t k3 = JavaArithmetic::intSub(JavaArithmetic::intAdd(i, random.nextInt(JavaArithmetic::intAdd(JavaArithmetic::intMul(l, 2), 1))), l);
			int_t l3 = j;
			int_t i4 = JavaArithmetic::intSub(JavaArithmetic::intAdd(k, random.nextInt(JavaArithmetic::intAdd(JavaArithmetic::intMul(i1, 2), 1))), i1);
			if (!world->isAirBlock(k3, l3, i4))
				continue;
			int_t j4 = 0;
			if (world->getBlockMaterial(JavaArithmetic::intSub(k3, 1), l3, i4)->isSolid()) j4++;
			if (world->getBlockMaterial(JavaArithmetic::intAdd(k3, 1), l3, i4)->isSolid()) j4++;
			if (world->getBlockMaterial(k3, l3, JavaArithmetic::intSub(i4, 1))->isSolid()) j4++;
			if (world->getBlockMaterial(k3, l3, JavaArithmetic::intAdd(i4, 1))->isSolid()) j4++;
			if (j4 != 1)
				continue;
			world->setBlockWithNotify(k3, l3, i4, Block::chest->blockID);
			TileEntityChest *tileentitychest = dynamic_cast<TileEntityChest *>(world->getBlockTileEntity(k3, l3, i4));
			if (!tileentitychest)
				breakLabel0 = true;
			for (int_t k4 = 0; tileentitychest && k4 < 8; k4++)
			{
				ItemStack *itemstack = pickCheckLootItem(random);
				if (itemstack != nullptr)
				{
					const int_t slot = random.nextInt(tileentitychest->getSizeInventory());
					tileentitychest->setInventorySlotContents(slot, itemstack);
				}
				if (k4 >= 7)
					breakLabel0 = true;
			}
		}
	}
	world->setBlockWithNotify(i, j, k, Block::mobSpawner->blockID);
	TileEntityMobSpawner *tileentitymobspawner = dynamic_cast<TileEntityMobSpawner *>(world->getBlockTileEntity(i, j, k));
	if (tileentitymobspawner)
		tileentitymobspawner->setMobID(pickMobSpawner(random));
	return true;
}

ItemStack *WorldGenDungeons::pickCheckLootItem(Random &random)
{
	int_t i = random.nextInt(11);
	if (i == 0) return new ItemStack(Item::saddle);
	if (i == 1) return new ItemStack(Item::ingotIron, random.nextInt(4) + 1);
	if (i == 2) return new ItemStack(Item::bread);
	if (i == 3) return new ItemStack(Item::wheat, random.nextInt(4) + 1);
	if (i == 4) return new ItemStack(Item::gunpowder, random.nextInt(4) + 1);
	if (i == 5) return new ItemStack(Item::silk, random.nextInt(4) + 1);
	if (i == 6) return new ItemStack(Item::bucketEmpty);
	if (i == 7 && random.nextInt(100) == 0) return new ItemStack(Item::appleGold);
	if (i == 8 && random.nextInt(2) == 0) return new ItemStack(Item::redstone, random.nextInt(4) + 1);
	if (i == 9 && random.nextInt(10) == 0) return new ItemStack(Item::itemsList[Item::record13->shiftedIndex + random.nextInt(2)]);
	if (i == 10) return new ItemStack(Item::dyePowder, 1, 3);
	return nullptr;
}

std::string WorldGenDungeons::pickMobSpawner(Random &random)
{
	int_t i = random.nextInt(4);
	if (i == 0) return "Skeleton";
	if (i == 1) return "Zombie";
	if (i == 2) return "Zombie";
	if (i == 3) return "Spider";
	return "";
}
