#include "EntityPainting.h"
#include "java/Arithmetic.h"
#include "DamageSource.h"

#include <vector>

#include "AxisAlignedBB.h"
#include "EnumArt.h"
#include "java/String.h"
#include "Item.h"
#include "ItemStack.h"
#include "Material.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "EntityItem.h"

EntityPainting::EntityPainting(World *world)
	: Entity(world)
{
	ensureEntityInit();
	field_695_c = 0;
	direction = 0;
	yOffset = 0.0f;
	setSize(0.5f, 0.5f);
}

EntityPainting::EntityPainting(World *world, int_t i, int_t j, int_t k, int_t l)
	: EntityPainting(world)
{
	xPosition = i;
	yPosition = j;
	zPosition = k;
	std::vector<EnumArt *> arraylist;
	const EnumArt **aenumart = EnumArt::values;
	int i1 = EnumArt::valuesCount;
	for (int j1 = 0; j1 < i1; j1++)
	{
		const EnumArt *enumart = aenumart[j1];
		art = const_cast<EnumArt *>(enumart);
		setDirection(l);
		if (onValidSurface())
		{
			arraylist.push_back(const_cast<EnumArt *>(enumart));
		}
	}
	if (arraylist.size() > 0)
	{
		art = arraylist[rand.nextInt(arraylist.size())];
	}
	setDirection(l);
}

EntityPainting::EntityPainting(World *world, int_t i, int_t j, int_t k, int_t l, jstring s)
	: EntityPainting(world)
{
	xPosition = i;
	yPosition = j;
	zPosition = k;
	const EnumArt **aenumart = EnumArt::values;
	int i1 = EnumArt::valuesCount;
	int j1 = 0;
	do
	{
		if (j1 >= i1)
		{
			break;
		}
		const EnumArt *enumart = aenumart[j1];
		if (enumart->title == s)
		{
			art = const_cast<EnumArt*>(enumart);
			break;
		}
		j1++;
	} while (true);
	setDirection(l);
}

void EntityPainting::entityInit()
{
}

void EntityPainting::setDirection(int_t i)
{
	direction = i;
	prevRotationYaw = rotationYaw = (float)(i * 90);
	float f = (float)art->sizeX;
	float f1 = (float)art->sizeY;
	float f2 = (float)art->sizeX;
	if (i == 0 || i == 2)
	{
		f2 = 0.5f;
	}
	else
	{
		f = 0.5f;
	}
	f /= 32.0f;
	f1 /= 32.0f;
	f2 /= 32.0f;
	float f3 = (float)xPosition + 0.5f;
	float f4 = (float)yPosition + 0.5f;
	float f5 = (float)zPosition + 0.5f;
	float f6 = 0.5625f;
	if (i == 0)
	{
		f5 -= f6;
	}
	if (i == 1)
	{
		f3 -= f6;
	}
	if (i == 2)
	{
		f5 += f6;
	}
	if (i == 3)
	{
		f3 += f6;
	}
	if (i == 0)
	{
		f3 -= getOffsetForSize(art->sizeX);
	}
	if (i == 1)
	{
		f5 += getOffsetForSize(art->sizeX);
	}
	if (i == 2)
	{
		f3 += getOffsetForSize(art->sizeX);
	}
	if (i == 3)
	{
		f5 -= getOffsetForSize(art->sizeX);
	}
	f4 += getOffsetForSize(art->sizeY);
	setPosition((double)f3, (double)f4, (double)f5);
	float f7 = -0.00625f;
	boundingBox->setBounds((double)(f3 - f - f7), (double)(f4 - f1 - f7), (double)(f5 - f2 - f7), (double)(f3 + f + f7), (double)(f4 + f1 + f7), (double)(f5 + f2 + f7));
}

float EntityPainting::getOffsetForSize(int_t i)
{
	if (i == 32)
	{
		return 0.5f;
	}
	return i != 64 ? 0.0f : 0.5f;
}

void EntityPainting::dropPaintingItem()
{
	EntityItem *item = new EntityItem(worldObj, posX, posY, posZ, new ItemStack(Item::painting));
	if (!worldObj->entityJoinedWorld(item))
		delete item;
}

void EntityPainting::onUpdate()
{
	if (field_695_c++ == 100 && !worldObj->multiplayerWorld)
	{
		field_695_c = 0;
		if (!isDead && !onValidSurface())
		{
			setEntityDead();
			dropPaintingItem();
		}
	}
}

bool EntityPainting::onValidSurface()
{
	if (worldObj->getCollidingBoundingBoxes(this, boundingBox).size() > 0)
	{
		return false;
	}
	int i = art->sizeX / 16;
	int j = art->sizeY / 16;
	int k = xPosition;
	int l = yPosition;
	int i1 = zPosition;
	if (direction == 0)
	{
		k = MathHelper::floor_double(posX - (double)((float)art->sizeX / 32.0f));
	}
	if (direction == 1)
	{
		i1 = MathHelper::floor_double(posZ - (double)((float)art->sizeX / 32.0f));
	}
	if (direction == 2)
	{
		k = MathHelper::floor_double(posX - (double)((float)art->sizeX / 32.0f));
	}
	if (direction == 3)
	{
		i1 = MathHelper::floor_double(posZ - (double)((float)art->sizeX / 32.0f));
	}
	l = MathHelper::floor_double(posY - (double)((float)art->sizeY / 32.0f));
	for (int j1 = 0; j1 < i; j1++)
	{
		for (int k1 = 0; k1 < j; k1++)
		{
			Material *material;
			if (direction == 0 || direction == 2)
			{
				material = worldObj->getBlockMaterial(k + j1, l + k1, zPosition);
			}
			else
			{
				material = worldObj->getBlockMaterial(xPosition, l + k1, i1 + j1);
			}
			if (!material->isSolid())
			{
				return false;
			}
		}
	}
	const auto& list = worldObj->getEntitiesWithinAABBExcludingEntity(this, boundingBox);
	for (size_t l1 = 0; l1 < list.size(); l1++)
	{
		if (dynamic_cast<EntityPainting*>(list[l1]) != nullptr)
		{
			return false;
		}
	}
	return true;
}

bool EntityPainting::canBeCollidedWith()
{
	return true;
}

bool EntityPainting::attackEntityFrom(Entity *entity, int_t i)
{
	if (!isDead && !worldObj->multiplayerWorld)
	{
		setEntityDead();
		setBeenAttacked();
		dropPaintingItem();
	}
	return true;
}

bool EntityPainting::attackEntityFrom(const DamageSource &source, int_t damage)
{
	return attackEntityFrom(source.getEntity(), damage);
}

void EntityPainting::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	nbttagcompound->setByte("Dir", JavaArithmetic::byteFromBits(static_cast<ubyte_t>(direction)));
	nbttagcompound->setString("Motive", art->title);
	nbttagcompound->setInteger("TileX", xPosition);
	nbttagcompound->setInteger("TileY", yPosition);
	nbttagcompound->setInteger("TileZ", zPosition);
}

void EntityPainting::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	xPosition = nbttagcompound->getInteger("TileX");
	yPosition = nbttagcompound->getInteger("TileY");
	zPosition = nbttagcompound->getInteger("TileZ");
	direction = nbttagcompound->getByte("Dir");
	jstring s = nbttagcompound->getString("Motive");
	const EnumArt **aenumart = EnumArt::values;
	int i = EnumArt::valuesCount;
	for (int j = 0; j < i; j++)
	{
		const EnumArt *enumart = aenumart[j];
		if (enumart->title == s)
		{
			art = const_cast<EnumArt*>(enumart);
			break;
		}
	}
	if (art == nullptr)
	{
		art = const_cast<EnumArt *>(&EnumArt::Kebab);
	}
	setDirection(direction);
}

void EntityPainting::moveEntity(double d, double d1, double d2)
{
	if (!worldObj->multiplayerWorld && !isDead && d * d + d1 * d1 + d2 * d2 > 0.0)
	{
		setEntityDead();
		dropPaintingItem();
	}
}

void EntityPainting::addVelocity(double d, double d1, double d2)
{
	if (!worldObj->multiplayerWorld && !isDead && d * d + d1 * d1 + d2 * d2 > 0.0)
	{
		setEntityDead();
		dropPaintingItem();
	}
}
