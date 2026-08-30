#include "EntityXPOrb.h"

#include "Block.h"
#include "DamageSource.h"
#include "EntityPlayer.h"
#include "Material.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "java/Math.h"
#include "java/Arithmetic.h"

EntityXPOrb::EntityXPOrb(World *world) : Entity(world)
{
	setSize(0.25f, 0.25f);
	yOffset = height / 2.0f;
	ensureEntityInit();
}

EntityXPOrb::EntityXPOrb(World *world, double x, double y, double z, int_t value) : Entity(world), xpValue(value)
{
	setSize(0.5f, 0.5f);
	yOffset = height / 2.0f;
	setPosition(x, y, z);
	rotationYaw = static_cast<float>(Math::random() * 360.0);
	motionX = static_cast<double>(static_cast<float>(Math::random() * static_cast<double>(0.2f) - static_cast<double>(0.1f)) * 2.0f);
	motionY = static_cast<double>(static_cast<float>(Math::random() * 0.2) * 2.0f);
	motionZ = static_cast<double>(static_cast<float>(Math::random() * static_cast<double>(0.2f) - static_cast<double>(0.1f)) * 2.0f);
	ensureEntityInit();
}

bool EntityXPOrb::canTriggerWalking()
{
	return false;
}

void EntityXPOrb::entityInit()
{
}

int_t EntityXPOrb::getBrightnessForRender(float partialTick)
{
	float brightnessBoost = 0.5f;
	if (brightnessBoost < 0.0f)
		brightnessBoost = 0.0f;
	if (brightnessBoost > 1.0f)
		brightnessBoost = 1.0f;

	const int_t packed = Entity::getBrightnessForRender(partialTick);
	int_t blockLight = packed & 255;
	const int_t skyLight = (packed >> 16) & 255;
	blockLight += static_cast<int_t>(brightnessBoost * 15.0f * 16.0f);
	if (blockLight > 240)
		blockLight = 240;
	return blockLight | (skyLight << 16);
}

void EntityXPOrb::onUpdate()
{
	Entity::onUpdate();
	if (field_35126_c > 0)
		--field_35126_c;

	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	motionY -= static_cast<double>(0.03f);
	if (worldObj->getBlockMaterial(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ)) == Material::lava)
	{
		motionY = static_cast<double>(0.2f);
		motionX = static_cast<double>((rand.nextFloatDifference()) * 0.2f);
		motionZ = static_cast<double>((rand.nextFloatDifference()) * 0.2f);
		worldObj->playSoundAtEntity(this, "random.fizz", 0.4f, 2.0f + rand.nextFloat() * 0.4f);
	}

	pushOutOfBlocks(posX, (boundingBox->minY + boundingBox->maxY) / 2.0, posZ);
	const double attractionRange = 8.0;
	EntityPlayer *player = worldObj->getClosestPlayerToEntity(this, attractionRange);
	if (player != nullptr)
	{
		const double dx = (player->posX - posX) / attractionRange;
		const double dy = (player->posY + static_cast<double>(player->getEyeHeight()) - posY) / attractionRange;
		const double dz = (player->posZ - posZ) / attractionRange;
		const double distance = JavaMath::sqrt(dx * dx + dy * dy + dz * dz);
		double strength = 1.0 - distance;
		if (strength > 0.0 && distance > 0.0)
		{
			strength *= strength;
			motionX += dx / distance * strength * 0.1;
			motionY += dy / distance * strength * 0.1;
			motionZ += dz / distance * strength * 0.1;
		}
	}

	moveEntity(motionX, motionY, motionZ);
	float friction = 0.98f;
	if (onGround)
	{
		friction = 0.1f * 0.1f * 58.8f;
		const int_t blockId = worldObj->getBlockId(MathHelper::floor_double(posX), MathHelper::floor_double(boundingBox->minY) - 1, MathHelper::floor_double(posZ));
		if (blockId > 0 && blockId < Block::BLOCK_REGISTRY_SIZE && Block::blocksList[blockId] != nullptr)
			friction = Block::blocksList[blockId]->slipperiness * 0.98f;
	}

	motionX *= static_cast<double>(friction);
	motionY *= static_cast<double>(0.98f);
	motionZ *= static_cast<double>(friction);
	if (onGround)
		motionY *= static_cast<double>(-0.9f);

	++xpColor;
	++xpOrbAge;
	if (xpOrbAge >= 6000)
		setDead();
}

bool EntityXPOrb::handleWaterMovement()
{
	return worldObj->handleMaterialAcceleration(boundingBox, Material::water, this);
}

void EntityXPOrb::dealFireDamage(int_t damage)
{
	attackEntityFrom(DamageSource::inFire, damage);
}

bool EntityXPOrb::attackEntityFrom(const DamageSource &, int_t damage)
{
	setBeenAttacked();
	xpOrbHealth -= damage;
	if (xpOrbHealth <= 0)
		setDead();
	return false;
}

void EntityXPOrb::writeEntityToNBT(NBTTagCompound *compound)
{
	compound->setShort("Health", static_cast<short_t>(JavaArithmetic::byteFromBits(static_cast<ubyte_t>(xpOrbHealth))));
	compound->setShort("Age", static_cast<short_t>(xpOrbAge));
	compound->setShort("Value", static_cast<short_t>(xpValue));
}

void EntityXPOrb::readEntityFromNBT(NBTTagCompound *compound)
{
	xpOrbHealth = compound->getShort("Health") & 255;
	xpOrbAge = compound->getShort("Age");
	xpValue = compound->getShort("Value");
}

void EntityXPOrb::onCollideWithPlayer(EntityPlayer *player)
{
	if (!worldObj->multiplayerWorld && field_35126_c == 0 && player != nullptr && player->xpCooldown == 0)
	{
		player->xpCooldown = 2;
		worldObj->playSoundAtEntity(this, "random.orb", 0.1f, 0.5f * ((rand.nextFloatDifference()) * 0.7f + 1.8f));
		player->onItemPickup(this, 1);
		player->addExperience(xpValue);
		setDead();
	}
}

int_t EntityXPOrb::getXpValue() const
{
	return xpValue;
}

int_t EntityXPOrb::getTextureByXP() const
{
	return xpValue >= 2477 ? 10 : (xpValue >= 1237 ? 9 : (xpValue >= 617 ? 8 : (xpValue >= 307 ? 7 : (xpValue >= 149 ? 6 : (xpValue >= 73 ? 5 : (xpValue >= 37 ? 4 : (xpValue >= 17 ? 3 : (xpValue >= 7 ? 2 : (xpValue >= 3 ? 1 : 0)))))))));
}

int_t EntityXPOrb::getXPSplit(int_t value)
{
	return value >= 2477 ? 2477 : (value >= 1237 ? 1237 : (value >= 617 ? 617 : (value >= 307 ? 307 : (value >= 149 ? 149 : (value >= 73 ? 73 : (value >= 37 ? 37 : (value >= 17 ? 17 : (value >= 7 ? 7 : (value >= 3 ? 3 : 1)))))))));
}

bool EntityXPOrb::canAttackWithItem()
{
	return false;
}
