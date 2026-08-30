#include "EntityMinecart.h"
#include "java/Arithmetic.h"
#include "DamageSource.h"
#include "DataWatcher.h"

#include "platform/Log.h"
#include <cmath>
#include "java/Math.h"
#include <iostream>
#include <vector>

#include "AxisAlignedBB.h"
#include "Block.h"
#include "BlockRail.h"
#include "EntityItem.h"
#include "EntityIronGolem.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "NBTTagList.h"
#include "Vec3D.h"
#include "World.h"

// Java field_855_j — rail metadata -> the two unit-vector endpoints of the
// curve segment relative to the rail block's center. Each entry is
// {{dx0, dy0, dz0}, {dx1, dy1, dz1}}.
const int_t EntityMinecart::railDirections[10][2][3] = {
	{{ 0,  0, -1}, { 0,  0,  1}},
	{{-1,  0,  0}, { 1,  0,  0}},
	{{-1, -1,  0}, { 1,  0,  0}},
	{{-1,  0,  0}, { 1, -1,  0}},
	{{ 0,  0, -1}, { 0, -1,  1}},
	{{ 0, -1, -1}, { 0,  0,  1}},
	{{ 0,  0,  1}, { 1,  0,  0}},
	{{ 0,  0,  1}, {-1,  0,  0}},
	{{ 0,  0, -1}, {-1,  0,  0}},
	{{ 0,  0, -1}, { 1,  0,  0}},
};

EntityMinecart::EntityMinecart(World *world)
	: Entity(world),
	  cargoItems(nullptr),
	  field_856_i(false),
	  minecartType(0),
	  fuel(0),
	  pushX(0.0),
	  pushZ(0.0),
	  serverPosTicks(0),
	  serverPosX_(0.0),
	  serverPosY_(0.0),
	  serverPosZ_(0.0),
	  serverYaw_(0.0),
	  serverPitch_(0.0),
	  velocityX(0.0),
	  velocityY(0.0),
	  velocityZ(0.0)
{
	ensureEntityInit();
	cargoItems = new ItemStack *[36]();
	for (int_t i = 0; i < 36; i++)
		cargoItems[i] = nullptr;
	preventEntitySpawning = true;
	setSize(0.98f, 0.7f);
	yOffset = height / 2.0f;
}

EntityMinecart::EntityMinecart(World *world, double d, double d1, double d2, int_t i)
	: EntityMinecart(world)
{
	setPosition(d, d1 + (double)yOffset, d2);
	motionX = 0.0;
	motionY = 0.0;
	motionZ = 0.0;
	prevPosX = d;
	prevPosY = d1;
	prevPosZ = d2;
	minecartType = i;
}

EntityMinecart::~EntityMinecart()
{
	if (cargoItems != nullptr)
	{
		for (int_t i = 0; i < 36; ++i)
		{
			delete cargoItems[i];
			cargoItems[i] = nullptr;
		}
		delete[] cargoItems;
		cargoItems = nullptr;
	}
}

bool EntityMinecart::canTriggerWalking()
{
	return false;
}

void EntityMinecart::entityInit()
{
	dataWatcher->addObject(16, (byte_t)0);
	dataWatcher->addObject(17, int_t(0));
	dataWatcher->addObject(18, int_t(1));
	dataWatcher->addObject(19, int_t(0));
}

void EntityMinecart::setDamageTaken(int_t value)
{
	dataWatcher->updateObject(19, value);
}

int_t EntityMinecart::getDamageTaken() const
{
	return dataWatcher->getWatchableObjectInt(19);
}

void EntityMinecart::setTimeSinceHit(int_t value)
{
	dataWatcher->updateObject(17, value);
}

int_t EntityMinecart::getTimeSinceHit() const
{
	return dataWatcher->getWatchableObjectInt(17);
}

void EntityMinecart::setForwardDirection(int_t value)
{
	dataWatcher->updateObject(18, value);
}

int_t EntityMinecart::getForwardDirection() const
{
	return dataWatcher->getWatchableObjectInt(18);
}

bool EntityMinecart::isMinecartPowered() const
{
	return (dataWatcher->getWatchableObjectByte(16) & 1) != 0;
}

void EntityMinecart::setMinecartPowered(bool powered)
{
	byte_t value = dataWatcher->getWatchableObjectByte(16);
	dataWatcher->updateObject(16, powered ? (byte_t)(value | 1) : (byte_t)(value & 0xfe));
}

AxisAlignedBB *EntityMinecart::getCollisionBox(Entity *entity)
{
	return entity->boundingBox;
}

AxisAlignedBB *EntityMinecart::getBoundingBox()
{
	return nullptr;
}

double EntityMinecart::getMountedYOffset()
{
	return (double)height * 0.0 - 0.30000001192092896;
}

bool EntityMinecart::attackEntityFrom(Entity *entity, int_t i)
{
	if (worldObj->multiplayerWorld || isDead)
		return true;

	setForwardDirection(-getForwardDirection());
	setTimeSinceHit(10);
	setBeenAttacked();
	setDamageTaken(JavaArithmetic::intAdd(getDamageTaken(), JavaArithmetic::intMul(i, 10)));
	if (getDamageTaken() > 40)
	{
		if (riddenByEntity != nullptr)
			riddenByEntity->mountEntity(this);

		setEntityDead();
		dropItemWithOffset(Item::minecartEmpty->shiftedIndex, 1, 0.0f);

		if (minecartType == 1)
		{
			for (int_t j = 0; j < getSizeInventory(); j++)
			{
				ItemStack *itemstack = getStackInSlot(j);
				if (itemstack == nullptr)
					continue;
				float f  = rand.nextFloat() * 0.8f + 0.1f;
				float f1 = rand.nextFloat() * 0.8f + 0.1f;
				float f2 = rand.nextFloat() * 0.8f + 0.1f;
				while (itemstack->stackSize > 0)
				{
					int_t k = rand.nextInt(21) + 10;
					if (k > itemstack->stackSize)
						k = itemstack->stackSize;
					itemstack->stackSize -= k;
					EntityItem *entityitem = new EntityItem(
						worldObj,
						posX + (double)f, posY + (double)f1, posZ + (double)f2,
						new ItemStack(itemstack->itemID, k, itemstack->getItemDamage()));
					float f3 = 0.05f;
					entityitem->motionX = (float)rand.nextGaussian() * f3;
					entityitem->motionY = (float)rand.nextGaussian() * f3 + 0.2f;
					entityitem->motionZ = (float)rand.nextGaussian() * f3;
					if (!worldObj->entityJoinedWorld(entityitem))
						delete entityitem;
				}
			}
			dropItemWithOffset(Block::chest->blockID, 1, 0.0f);
		}
		else if (minecartType == 2)
		{
			dropItemWithOffset(Block::stoneOvenIdle->blockID, 1, 0.0f);
		}
	}
	return true;
}

bool EntityMinecart::attackEntityFrom(const DamageSource &source, int_t damage)
{
	return attackEntityFrom(source.getEntity(), damage);
}

void EntityMinecart::performHurtAnimation()
{
	MC_LOG_DEBUG("entity", "Animating hurt\n");
	setForwardDirection(-getForwardDirection());
	setTimeSinceHit(10);
	int_t damageTaken = getDamageTaken();
	setDamageTaken(JavaArithmetic::intAdd(damageTaken, JavaArithmetic::intMul(damageTaken, 10)));
}

bool EntityMinecart::canBeCollidedWith()
{
	return !isDead;
}

void EntityMinecart::setEntityDead()
{
	for (int_t i = 0; i < getSizeInventory(); i++)
	{
		ItemStack *itemstack = getStackInSlot(i);
		if (itemstack == nullptr)
			continue;
		float f  = rand.nextFloat() * 0.8f + 0.1f;
		float f1 = rand.nextFloat() * 0.8f + 0.1f;
		float f2 = rand.nextFloat() * 0.8f + 0.1f;
		while (itemstack->stackSize > 0)
		{
			int_t j = rand.nextInt(21) + 10;
			if (j > itemstack->stackSize)
				j = itemstack->stackSize;
			itemstack->stackSize -= j;
			ItemStack *droppedStack = new ItemStack(itemstack->itemID, j, itemstack->getItemDamage());
			if (itemstack->hasTagCompound())
			{
				NBTTagCompound *copy = dynamic_cast<NBTTagCompound *>(itemstack->getTagCompound()->copy());
				droppedStack->setTagCompound(copy);
			}
			EntityItem *entityitem = new EntityItem(
				worldObj,
				posX + (double)f, posY + (double)f1, posZ + (double)f2,
				droppedStack);
			float f3 = 0.05f;
			entityitem->motionX = (float)rand.nextGaussian() * f3;
			entityitem->motionY = (float)rand.nextGaussian() * f3 + 0.2f;
			entityitem->motionZ = (float)rand.nextGaussian() * f3;
			if (!worldObj->entityJoinedWorld(entityitem))
				delete entityitem;
		}
	}
	Entity::setEntityDead();
}

void EntityMinecart::onUpdate()
{
	if (getTimeSinceHit() > 0)
		setTimeSinceHit(getTimeSinceHit() - 1);
	if (getDamageTaken() > 0)
		setDamageTaken(getDamageTaken() - 1);

	if (posY < -64.0)
		kill();

	if (isMinecartPowered() && rand.nextInt(4) == 0)
		worldObj->spawnParticle("largesmoke", posX, posY + 0.8, posZ, 0.0, 0.0, 0.0);

	if (worldObj->multiplayerWorld)
	{
		if (serverPosTicks > 0)
		{
			double d  = posX + (serverPosX_ - posX) / (double)serverPosTicks;
		double d1 = posY + (serverPosY_ - posY) / (double)serverPosTicks;
		double d3 = posZ + (serverPosZ_ - posZ) / (double)serverPosTicks;
		double d4;
		for (d4 = serverYaw_ - (double)rotationYaw; d4 <  -180.0; d4 += 360.0) {}
		for (;                                       d4 >=  180.0; d4 -= 360.0) {}
		rotationYaw   += (float)(d4 / (double)serverPosTicks);
		rotationPitch += (float)((serverPitch_ - (double)rotationPitch) / (double)serverPosTicks);
			serverPosTicks--;
			setPosition(d, d1, d3);
			setRotation(rotationYaw, rotationPitch);
		}
		else
		{
			setPosition(posX, posY, posZ);
			setRotation(rotationYaw, rotationPitch);
		}
		return;
	}

	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	motionY -= 0.039999999105930328;
	int_t i = MathHelper::floor_double(posX);
	int_t j = MathHelper::floor_double(posY);
	int_t k = MathHelper::floor_double(posZ);
	if (BlockRail::isRailBlockAt(worldObj, i, j - 1, k))
		j--;

	double d2 = 0.40000000000000002;
	bool   flag = false;
	double d5 = 0.0078125;
	int_t  l = worldObj->getBlockId(i, j, k);

	if (BlockRail::isRailBlock(l))
	{
		Vec3D *vec3d = getRailPosition(posX, posY, posZ);
		int_t  i1 = worldObj->getBlockMetadata(i, j, k);
		posY = (double)j;
		bool flag1 = false;
		bool flag2 = false;
		if (l == Block::railPowered->blockID)
		{
			flag1 = (i1 & 8) != 0;
			flag2 = !flag1;
		}
		if (static_cast<BlockRail *>(Block::blocksList[l])->getIsPowered())
			i1 &= 7;

		if (i1 >= 2 && i1 <= 5)
			posY = (double)(j + 1);

		if (i1 == 2) motionX -= d5;
		if (i1 == 3) motionX += d5;
		if (i1 == 4) motionZ += d5;
		if (i1 == 5) motionZ -= d5;

		const int_t (&ai)[2][3] = railDirections[i1];
		double d9  = (double)(ai[1][0] - ai[0][0]);
		double d10 = (double)(ai[1][2] - ai[0][2]);
		double d11 = JavaMath::sqrt(d9 * d9 + d10 * d10);
		double d12 = motionX * d9 + motionZ * d10;
		if (d12 < 0.0)
		{
			d9  = -d9;
			d10 = -d10;
		}
		double d13 = JavaMath::sqrt(motionX * motionX + motionZ * motionZ);
		motionX = (d13 * d9)  / d11;
		motionZ = (d13 * d10) / d11;

		if (flag2)
		{
			double d16 = JavaMath::sqrt(motionX * motionX + motionZ * motionZ);
			if (d16 < 0.029999999999999999)
			{
				motionX *= 0.0;
				motionY *= 0.0;
				motionZ *= 0.0;
			}
			else
			{
				motionX *= 0.5;
				motionY *= 0.0;
				motionZ *= 0.5;
			}
		}

		double d17 = 0.0;
		double d18 = (double)i + 0.5 + (double)ai[0][0] * 0.5;
		double d19 = (double)k + 0.5 + (double)ai[0][2] * 0.5;
		double d20 = (double)i + 0.5 + (double)ai[1][0] * 0.5;
		double d21 = (double)k + 0.5 + (double)ai[1][2] * 0.5;
		d9  = d20 - d18;
		d10 = d21 - d19;

		if (d9 == 0.0)
		{
			posX = (double)i + 0.5;
			d17  = posZ - (double)k;
		}
		else if (d10 == 0.0)
		{
			posZ = (double)k + 0.5;
			d17  = posX - (double)i;
		}
		else
		{
			double d22 = posX - d18;
			double d24 = posZ - d19;
			d17 = (d22 * d9 + d24 * d10) * 2.0;
		}

		posX = d18 + d9  * d17;
		posZ = d19 + d10 * d17;
		setPosition(posX, posY + (double)yOffset, posZ);

		double d23 = motionX;
		double d25 = motionZ;
		if (riddenByEntity != nullptr)
		{
			d23 *= 0.75;
			d25 *= 0.75;
		}
		if (d23 < -d2) d23 = -d2;
		if (d23 >  d2) d23 =  d2;
		if (d25 < -d2) d25 = -d2;
		if (d25 >  d2) d25 =  d2;
		moveEntity(d23, 0.0, d25);

		if (ai[0][1] != 0 &&
		    MathHelper::floor_double(posX) - i == ai[0][0] &&
		    MathHelper::floor_double(posZ) - k == ai[0][2])
		{
			setPosition(posX, posY + (double)ai[0][1], posZ);
		}
		else if (ai[1][1] != 0 &&
		         MathHelper::floor_double(posX) - i == ai[1][0] &&
		         MathHelper::floor_double(posZ) - k == ai[1][2])
		{
			setPosition(posX, posY + (double)ai[1][1], posZ);
		}

		if (riddenByEntity != nullptr)
		{
			motionX *= 0.99699997901916504;
			motionY *= 0.0;
			motionZ *= 0.99699997901916504;
		}
		else
		{
			if (minecartType == 2)
			{
				double d27 = MathHelper::sqrt_double(pushX * pushX + pushZ * pushZ);
				if (d27 > 0.01)
				{
					flag = true;
					pushX /= d27;
					pushZ /= d27;
					double d29 = 0.040000000000000001;
					motionX *= 0.80000001192092896;
					motionY *= 0.0;
					motionZ *= 0.80000001192092896;
					motionX += pushX * d29;
					motionZ += pushZ * d29;
				}
				else
				{
					motionX *= 0.89999997615814209;
					motionY *= 0.0;
					motionZ *= 0.89999997615814209;
				}
			}
			motionX *= 0.95999997854232788;
			motionY *= 0.0;
			motionZ *= 0.95999997854232788;
		}

		Vec3D *vec3d1 = getRailPosition(posX, posY, posZ);
		if (vec3d1 != nullptr && vec3d != nullptr)
		{
			double d28 = (vec3d->yCoord - vec3d1->yCoord) * 0.050000000000000003;
			double d14 = JavaMath::sqrt(motionX * motionX + motionZ * motionZ);
			if (d14 > 0.0)
			{
				motionX = (motionX / d14) * (d14 + d28);
				motionZ = (motionZ / d14) * (d14 + d28);
			}
			setPosition(posX, vec3d1->yCoord, posZ);
		}

		int_t k1 = MathHelper::floor_double(posX);
		int_t l1 = MathHelper::floor_double(posZ);
		if (k1 != i || l1 != k)
		{
			double d15 = JavaMath::sqrt(motionX * motionX + motionZ * motionZ);
			motionX = d15 * (double)(k1 - i);
			motionZ = d15 * (double)(l1 - k);
		}

		if (minecartType == 2)
		{
			double d30 = MathHelper::sqrt_double(pushX * pushX + pushZ * pushZ);
			if (d30 > 0.01 && motionX * motionX + motionZ * motionZ > 0.001)
			{
				pushX /= d30;
				pushZ /= d30;
				if (pushX * motionX + pushZ * motionZ < 0.0)
				{
					pushX = 0.0;
					pushZ = 0.0;
				}
				else
				{
					pushX = motionX;
					pushZ = motionZ;
				}
			}
		}

		if (flag1)
		{
			double d31 = JavaMath::sqrt(motionX * motionX + motionZ * motionZ);
			if (d31 > 0.01)
			{
				double d32 = 0.059999999999999998;
				motionX += (motionX / d31) * d32;
				motionZ += (motionZ / d31) * d32;
			}
			else if (i1 == 1)
			{
				if      (worldObj->isBlockNormalCube(i - 1, j, k)) motionX =  0.02;
				else if (worldObj->isBlockNormalCube(i + 1, j, k)) motionX = -0.02;
			}
			else if (i1 == 0)
			{
				if      (worldObj->isBlockNormalCube(i, j, k - 1)) motionZ =  0.02;
				else if (worldObj->isBlockNormalCube(i, j, k + 1)) motionZ = -0.02;
			}
		}
	}
	else
	{
		if (motionX < -d2) motionX = -d2;
		if (motionX >  d2) motionX =  d2;
		if (motionZ < -d2) motionZ = -d2;
		if (motionZ >  d2) motionZ =  d2;
		if (onGround)
		{
			motionX *= 0.5;
			motionY *= 0.5;
			motionZ *= 0.5;
		}
		moveEntity(motionX, motionY, motionZ);
		if (!onGround)
		{
			motionX *= 0.94999998807907104;
			motionY *= 0.94999998807907104;
			motionZ *= 0.94999998807907104;
		}
	}

	rotationPitch = 0.0f;
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
	const float d6 = static_cast<float>(prevPosX - posX);
	const float d7 = static_cast<float>(prevPosZ - posZ);
	if (d6 * d6 + d7 * d7 > 0.001f)
	{
		rotationYaw = (std::atan2(d7, d6) * 180.0f) / 3.1415927f;
		if (field_856_i)
			rotationYaw += 180.0f;
	}

	float d8;
	for (d8 = rotationYaw - prevRotationYaw; d8 >= 180.0f; d8 -= 360.0f) {}
	for (; d8 < -180.0f; d8 += 360.0f) {}
#else
	double d6 = prevPosX - posX;
	double d7 = prevPosZ - posZ;
	if (d6 * d6 + d7 * d7 > 0.001)
	{
		rotationYaw = (float)((JavaMath::atan2(d7, d6) * 180.0) / 3.1415926535897931);
		if (field_856_i)
			rotationYaw += 180.0f;
	}

	double d8;
	for (d8 = rotationYaw - prevRotationYaw; d8 >= 180.0; d8 -= 360.0) {}
	for (; d8 < -180.0; d8 += 360.0) {}
#endif
	if (d8 < -170.0f || d8 >= 170.0f)
	{
		rotationYaw += 180.0f;
		field_856_i = !field_856_i;
	}
	setRotation(rotationYaw, rotationPitch);

	std::vector<Entity *> &list = worldObj->getEntitiesWithinAABBExcludingEntity(
		this, boundingBox->expand(0.20000000298023224, 0.0, 0.20000000298023224));
	if (!list.empty())
	{
		for (Entity *entity : list)
		{
			if (entity != riddenByEntity && entity->canBePushed() && dynamic_cast<EntityMinecart*>(entity) != nullptr)
				entity->applyEntityCollision(this);
		}
	}

	if (riddenByEntity != nullptr && riddenByEntity->isDead)
	{
		if (riddenByEntity->ridingEntity == this)
			riddenByEntity->ridingEntity = nullptr;
		riddenByEntity = nullptr;
	}

	if (fuel > 0)
		--fuel;
	if (fuel <= 0)
		pushX = pushZ = 0.0;
	setMinecartPowered(fuel > 0);
}

Vec3D *EntityMinecart::getNextRailPosition(double d, double d1, double d2, double d3)
{
	int_t i = MathHelper::floor_double(d);
	int_t j = MathHelper::floor_double(d1);
	int_t k = MathHelper::floor_double(d2);
	if (BlockRail::isRailBlockAt(worldObj, i, j - 1, k))
		j--;
	int_t l = worldObj->getBlockId(i, j, k);
	if (!BlockRail::isRailBlock(l))
		return nullptr;

	int_t i1 = worldObj->getBlockMetadata(i, j, k);
	if (static_cast<BlockRail *>(Block::blocksList[l])->getIsPowered())
		i1 &= 7;
	d1 = (double)j;
	if (i1 >= 2 && i1 <= 5)
		d1 = (double)(j + 1);

	const int_t (&ai)[2][3] = railDirections[i1];
	double d4 = (double)(ai[1][0] - ai[0][0]);
	double d5 = (double)(ai[1][2] - ai[0][2]);
	double d6 = JavaMath::sqrt(d4 * d4 + d5 * d5);
	d4 /= d6;
	d5 /= d6;
	d  += d4 * d3;
	d2 += d5 * d3;

	if (ai[0][1] != 0 &&
	    MathHelper::floor_double(d)  - i == ai[0][0] &&
	    MathHelper::floor_double(d2) - k == ai[0][2])
	{
		d1 += (double)ai[0][1];
	}
	else if (ai[1][1] != 0 &&
	         MathHelper::floor_double(d)  - i == ai[1][0] &&
	         MathHelper::floor_double(d2) - k == ai[1][2])
	{
		d1 += (double)ai[1][1];
	}
	return getRailPosition(d, d1, d2);
}

Vec3D *EntityMinecart::getRailPosition(double d, double d1, double d2)
{
	int_t i = MathHelper::floor_double(d);
	int_t j = MathHelper::floor_double(d1);
	int_t k = MathHelper::floor_double(d2);
	if (BlockRail::isRailBlockAt(worldObj, i, j - 1, k))
		j--;
	int_t l = worldObj->getBlockId(i, j, k);
	if (!BlockRail::isRailBlock(l))
		return nullptr;

	int_t  i1 = worldObj->getBlockMetadata(i, j, k);
	d1 = (double)j;
	if (static_cast<BlockRail *>(Block::blocksList[l])->getIsPowered())
		i1 &= 7;
	if (i1 >= 2 && i1 <= 5)
		d1 = (double)(j + 1);

	const int_t (&ai)[2][3] = railDirections[i1];
	double d3 = 0.0;
	double d4 = (double)i + 0.5 + (double)ai[0][0] * 0.5;
	double d5 = (double)j + 0.5 + (double)ai[0][1] * 0.5;
	double d6 = (double)k + 0.5 + (double)ai[0][2] * 0.5;
	double d7 = (double)i + 0.5 + (double)ai[1][0] * 0.5;
	double d8 = (double)j + 0.5 + (double)ai[1][1] * 0.5;
	double d9 = (double)k + 0.5 + (double)ai[1][2] * 0.5;
	double d10 = d7 - d4;
	double d11 = (d8 - d5) * 2.0;
	double d12 = d9 - d6;

	if (d10 == 0.0)
	{
		d  = (double)i + 0.5;
		d3 = d2 - (double)k;
	}
	else if (d12 == 0.0)
	{
		d2 = (double)k + 0.5;
		d3 = d - (double)i;
	}
	else
	{
		double d13 = d  - d4;
		double d14 = d2 - d6;
		d3 = (d13 * d10 + d14 * d12) * 2.0;
	}

	d  = d4 + d10 * d3;
	d1 = d5 + d11 * d3;
	d2 = d6 + d12 * d3;

	if (d11 < 0.0) d1 += 1.0;
	if (d11 > 0.0) d1 += 0.5;

	return Vec3D::createVector(d, d1, d2);
}

void EntityMinecart::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	nbttagcompound->setInteger("Type", minecartType);
	if (minecartType == 2)
	{
		nbttagcompound->setDouble("PushX", pushX);
		nbttagcompound->setDouble("PushZ", pushZ);
		nbttagcompound->setShort("Fuel", JavaArithmetic::shortFromBits(static_cast<ushort_t>(fuel)));
	}
	else if (minecartType == 1)
	{
		NBTTagList *nbttaglist = new NBTTagList();
		for (int_t i = 0; i < getSizeInventory(); i++)
		{
			if (cargoItems[i] != nullptr)
			{
				NBTTagCompound *nbttagcompound1 = new NBTTagCompound();
				nbttagcompound1->setByte("Slot", (byte_t)i);
				cargoItems[i]->writeToNBT(nbttagcompound1);
				nbttaglist->setTag(nbttagcompound1);
			}
		}
		nbttagcompound->setTag("Items", nbttaglist);
	}
}

void EntityMinecart::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	minecartType = nbttagcompound->getInteger("Type");
	if (minecartType == 2)
	{
		pushX = nbttagcompound->getDouble("PushX");
		pushZ = nbttagcompound->getDouble("PushZ");
		fuel  = nbttagcompound->getShort("Fuel");
	}
	else if (minecartType == 1)
	{
		NBTTagList *nbttaglist = nbttagcompound->getTagList("Items");
		// cargoItems is already sized 36 from the constructor; Java rebuilds it
		// here just to be safe — for us, clearing is enough.
		for (int_t i = 0; i < getSizeInventory(); i++)
		{
			delete cargoItems[i];
			cargoItems[i] = nullptr;
		}
		for (int_t i = 0; i < nbttaglist->tagCount(); i++)
		{
			NBTTagCompound *nbttagcompound1 = static_cast<NBTTagCompound *>(nbttaglist->tagAt(i));
			int_t jj = nbttagcompound1->getByte("Slot") & 0xFF;
			if (jj >= 0 && jj < getSizeInventory())
			{
				ItemStack *loaded = ItemStack::loadItemStackFromNBT(nbttagcompound1);
				delete cargoItems[jj];
				cargoItems[jj] = loaded;
			}
		}
	}
}

float EntityMinecart::getShadowSize()
{
	return 0.0f;
}

void EntityMinecart::applyEntityCollision(Entity *entity)
{
	if (worldObj->multiplayerWorld || entity == riddenByEntity)
		return;

	if (entity->isLiving() && !entity->isPlayer() && dynamic_cast<EntityIronGolem *>(entity) == nullptr &&
	    minecartType == 0 && motionX * motionX + motionZ * motionZ > 0.01 &&
	    riddenByEntity == nullptr && entity->ridingEntity == nullptr)
	{
		entity->mountEntity(this);
	}

	double d = entity->posX - posX;
	double d1 = entity->posZ - posZ;
	double d2 = d * d + d1 * d1;
	if (d2 < (double)1.0e-4f)
		return;

	d2 = MathHelper::sqrt_double(d2);
	d /= d2;
	d1 /= d2;
	double d3 = 1.0 / d2;
	if (d3 > 1.0)
		d3 = 1.0;
	d *= d3;
	d1 *= d3;
	d *= (double)0.1f;
	d1 *= (double)0.1f;
	d *= (double)(1.0f - entityCollisionReduction);
	d1 *= (double)(1.0f - entityCollisionReduction);
	d *= 0.5;
	d1 *= 0.5;

	if (EntityMinecart *otherCart = dynamic_cast<EntityMinecart *>(entity))
	{
		Vec3D *collisionDirection = Vec3D::createVector(entity->posX - posX, 0.0, entity->posZ - posZ)->normalize();
		Vec3D *cartDirection = Vec3D::createVector(
			(double)MathHelper::cos(rotationYaw * 3.1415927f / 180.0f), 0.0,
			(double)MathHelper::sin(rotationYaw * 3.1415927f / 180.0f))->normalize();
		if (std::fabs(collisionDirection->dotProduct(cartDirection)) < (double)0.8f)
			return;

		double d7 = entity->motionX + motionX;
		double d8 = entity->motionZ + motionZ;
		if (otherCart->minecartType == 2 && minecartType != 2)
		{
			motionX *= (double)0.2f;
			motionZ *= (double)0.2f;
			addVelocity(entity->motionX - d, 0.0, entity->motionZ - d1);
			entity->motionX *= (double)0.95f;
			entity->motionZ *= (double)0.95f;
		}
		else if (otherCart->minecartType != 2 && minecartType == 2)
		{
			entity->motionX *= (double)0.2f;
			entity->motionZ *= (double)0.2f;
			entity->addVelocity(motionX + d, 0.0, motionZ + d1);
			motionX *= (double)0.95f;
			motionZ *= (double)0.95f;
		}
		else
		{
			d7 /= 2.0;
			d8 /= 2.0;
			motionX *= (double)0.2f;
			motionZ *= (double)0.2f;
			addVelocity(d7 - d, 0.0, d8 - d1);
			entity->motionX *= (double)0.2f;
			entity->motionZ *= (double)0.2f;
			entity->addVelocity(d7 + d, 0.0, d8 + d1);
		}
	}
	else
	{
		addVelocity(-d, 0.0, -d1);
		entity->addVelocity(d / 4.0, 0.0, d1 / 4.0);
	}
}

int_t EntityMinecart::getSizeInventory()
{
	return 27;
}

ItemStack *EntityMinecart::getStackInSlot(int_t i)
{
	return i >= 0 && i < getSizeInventory() ? cargoItems[i] : nullptr;
}


ItemStack *EntityMinecart::getStackInSlotOnClosing(int_t i)
{
	if (i < 0 || i >= getSizeInventory() || cargoItems[i] == nullptr)
		return nullptr;
	ItemStack *stack = cargoItems[i];
	cargoItems[i] = nullptr;
	return stack;
}

ItemStack *EntityMinecart::decrStackSize(int_t i, int_t j)
{
	if (i < 0 || i >= getSizeInventory() || j <= 0 || cargoItems[i] == nullptr)
		return nullptr;
	if (cargoItems[i]->stackSize <= j)
	{
		ItemStack *itemstack = cargoItems[i];
		cargoItems[i] = nullptr;
		return itemstack;
	}
	ItemStack *itemstack1 = cargoItems[i]->splitStack(j);
	if (cargoItems[i]->stackSize == 0)
	{
		delete cargoItems[i];
		cargoItems[i] = nullptr;
	}
	return itemstack1;
}

void EntityMinecart::setInventorySlotContents(int_t i, ItemStack *itemstack)
{
	if (i < 0 || i >= getSizeInventory())
	{
		delete itemstack;
		return;
	}
	if (cargoItems[i] != itemstack)
		delete cargoItems[i];
	cargoItems[i] = itemstack;
	if (itemstack != nullptr && itemstack->stackSize > getInventoryStackLimit())
		itemstack->stackSize = getInventoryStackLimit();
}

std::string EntityMinecart::getInvName()
{
	return "container.minecart";
}

int_t EntityMinecart::getInventoryStackLimit()
{
	return 64;
}

void EntityMinecart::onInventoryChanged()
{
}

bool EntityMinecart::canInteractWith(EntityPlayer *entityplayer)
{
	if (isDead)
		return false;
	return entityplayer->getDistanceSqToEntity(this) <= 64.0;
}

bool EntityMinecart::interact(EntityPlayer *entityplayer)
{
	if (minecartType == 0)
	{
		if (riddenByEntity != nullptr && riddenByEntity->isPlayer() && riddenByEntity != entityplayer)
			return true;
		if (!worldObj->multiplayerWorld)
			entityplayer->mountEntity(this);
	}
	else if (minecartType == 1)
	{
		if (!worldObj->multiplayerWorld)
			entityplayer->displayGUIChest(this);
	}
	else if (minecartType == 2)
	{
		ItemStack *itemstack = entityplayer->inventory->getCurrentItem();
		if (itemstack != nullptr && itemstack->itemID == Item::coal->shiftedIndex)
		{
			if (--itemstack->stackSize == 0)
			{
				entityplayer->inventory->setInventorySlotContents(entityplayer->inventory->currentItem, nullptr);
			}

			fuel += 3600;
		}
		pushX = posX - entityplayer->posX;
		pushZ = posZ - entityplayer->posZ;
	}
	return true;
}

void EntityMinecart::setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i)
{
	serverPosX_   = d;
	serverPosY_   = d1;
	serverPosZ_   = d2;
	serverYaw_    = f;
	serverPitch_  = f1;
	serverPosTicks = i + 2;
	motionX = velocityX;
	motionY = velocityY;
	motionZ = velocityZ;
}

void EntityMinecart::setVelocity(double d, double d1, double d2)
{
	velocityX = motionX = d;
	velocityY = motionY = d1;
	velocityZ = motionZ = d2;
}
