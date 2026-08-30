#include "EntitySlime.h"

#include "Chunk.h"
#include "DamageSource.h"
#include "DataWatcher.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"

EntitySlime::EntitySlime(World *world)
	: EntityLiving(world), field_40139_a(0.0f), field_768_a(0.0f), field_767_b(0.0f), slimeJumpDelay(0)
{
	ensureEntityInit();
	texture = "/mob/slime.png";
	const int_t size = 1 << rand.nextInt(3);
	yOffset = 0.0f;
	slimeJumpDelay = rand.nextInt(20) + 10;
	setSlimeSize(size);
}

void EntitySlime::entityInit()
{
	EntityLiving::entityInit();
	if (!dataWatcher->hasObject(16))
		dataWatcher->addObject(16, static_cast<byte_t>(1));
}

void EntitySlime::setSlimeSize(int_t size)
{
	if (size < 1)
		size = 1;
	dataWatcher->updateObject(16, static_cast<byte_t>(size));
	setSize(0.6f * static_cast<float>(size), 0.6f * static_cast<float>(size));
	setPosition(posX, posY, posZ);
	setHealth(getMaxHealth());
	experienceValue = size;
}

int_t EntitySlime::getMaxHealth() const
{
	const int_t size = const_cast<EntitySlime *>(this)->getSlimeSize();
	return size * size;
}

int_t EntitySlime::getSlimeSize()
{
	return dataWatcher->getWatchableObjectByte(16);
}

void EntitySlime::writeEntityToNBT(NBTTagCompound *tag)
{
	EntityLiving::writeEntityToNBT(tag);
	tag->setInteger("Size", getSlimeSize() - 1);
}

void EntitySlime::readEntityFromNBT(NBTTagCompound *tag)
{
	EntityLiving::readEntityFromNBT(tag);
	setSlimeSize(tag->getInteger("Size") + 1);
}

jstring EntitySlime::getSlimeParticle()
{
	return "slime";
}

jstring EntitySlime::getSlimeSound()
{
	return "mob.slime";
}

void EntitySlime::onUpdate()
{
	if (!worldObj->multiplayerWorld && worldObj->difficultySetting == 0 && getSlimeSize() > 0)
		isDead = true;

	field_768_a += (field_40139_a - field_768_a) * 0.5f;
	field_767_b = field_768_a;
	const bool wasOnGround = onGround;
	EntityLiving::onUpdate();

	if (onGround && !wasOnGround)
	{
		const int_t size = getSlimeSize();
		for (int_t i = 0; i < size * 8; ++i)
		{
			const float angle = rand.nextFloat() * 3.1415927f * 2.0f;
			const float radius = rand.nextFloat() * 0.5f + 0.5f;
			const float offsetX = MathHelper::sin(angle) * static_cast<float>(size) * 0.5f * radius;
			const float offsetZ = MathHelper::cos(angle) * static_cast<float>(size) * 0.5f * radius;
			worldObj->spawnParticle(getSlimeParticle(), posX + static_cast<double>(offsetX), boundingBox->minY,
				posZ + static_cast<double>(offsetZ), 0.0, 0.0, 0.0);
		}

		if (makesSoundOnLand())
			worldObj->playSoundAtEntity(this, getSlimeSound(), getSoundVolume(),
				((rand.nextFloatDifference()) * 0.2f + 1.0f) / 0.8f);

		field_40139_a = -0.5f;
	}

	updateSquish();
}

void EntitySlime::updatePlayerActionState()
{
	despawnEntity();
	EntityPlayer *player = worldObj->getClosestVulnerablePlayerToEntity(this, 16.0);
	if (player != nullptr)
		faceEntity(player, 10.0f, 20.0f);

	if (onGround && slimeJumpDelay-- <= 0)
	{
		slimeJumpDelay = getJumpDelay();
		if (player != nullptr)
			slimeJumpDelay /= 3;

		isJumping = true;
		if (makesSoundOnJump())
			worldObj->playSoundAtEntity(this, getSlimeSound(), getSoundVolume(),
				((rand.nextFloatDifference()) * 0.2f + 1.0f) * 0.8f);

		field_40139_a = 1.0f;
		moveStrafing = 1.0f - rand.nextFloat() * 2.0f;
		moveForward = static_cast<float>(getSlimeSize());
	}
	else
	{
		isJumping = false;
		if (onGround)
			moveStrafing = moveForward = 0.0f;
	}
}

void EntitySlime::updateSquish()
{
	field_40139_a *= 0.6f;
}

int_t EntitySlime::getJumpDelay()
{
	return rand.nextInt(20) + 10;
}

EntitySlime *EntitySlime::createInstance()
{
	return new EntitySlime(worldObj);
}

void EntitySlime::setEntityDead()
{
	const int_t size = getSlimeSize();
	if (!worldObj->multiplayerWorld && size > 1 && getHealth() <= 0)
	{
		const int_t count = 2 + rand.nextInt(3);
		for (int_t i = 0; i < count; ++i)
		{
			const float offsetX = (static_cast<float>(i % 2) - 0.5f) * static_cast<float>(size) / 4.0f;
			const float offsetZ = (static_cast<float>(i / 2) - 0.5f) * static_cast<float>(size) / 4.0f;
			EntitySlime *slime = createInstance();
			slime->setSlimeSize(size / 2);
			slime->setLocationAndAngles(posX + static_cast<double>(offsetX), posY + 0.5,
				posZ + static_cast<double>(offsetZ), rand.nextFloat() * 360.0f, 0.0f);
			if (!worldObj->entityJoinedWorld(slime))
				delete slime;
		}
	}
	EntityLiving::setEntityDead();
}

void EntitySlime::onCollideWithPlayer(EntityPlayer *player)
{
	if (!canDamagePlayer() || player == nullptr)
		return;

	const int_t size = getSlimeSize();
	if (canEntityBeSeen(player) && static_cast<double>(getDistanceToEntity(player)) < 0.6 * static_cast<double>(size) &&
		player->attackEntityFrom(DamageSource::causeMobDamage(this), getAttackStrength()))
	{
		worldObj->playSoundAtEntity(this, "mob.slimeattack", 1.0f, (rand.nextFloatDifference()) * 0.2f + 1.0f);
	}
}

bool EntitySlime::canDamagePlayer()
{
	return getSlimeSize() > 1;
}

int_t EntitySlime::getAttackStrength()
{
	return getSlimeSize();
}

jstring EntitySlime::getHurtSound()
{
	return "mob.slime";
}

jstring EntitySlime::getDeathSound()
{
	return "mob.slime";
}

int_t EntitySlime::getDropItemId()
{
	return getSlimeSize() == 1 ? Item::slimeBall->shiftedIndex : 0;
}

bool EntitySlime::getCanSpawnHere()
{
	Chunk *chunk = worldObj->getChunkFromBlockCoords(MathHelper::floor_double(posX), MathHelper::floor_double(posZ));
	if (chunk == nullptr)
		return false;
	Random chunkRandom = chunk->getRandomWithSeed(987234911LL);
	return (getSlimeSize() == 1 || worldObj->difficultySetting > 0) && rand.nextInt(10) == 0 &&
		chunkRandom.nextInt(10) == 0 && posY < 40.0 ? EntityLiving::getCanSpawnHere() : false;
}

float EntitySlime::getSoundVolume()
{
	return 0.4f * static_cast<float>(getSlimeSize());
}

int_t EntitySlime::getVerticalFaceSpeed()
{
	return 0;
}

bool EntitySlime::makesSoundOnJump()
{
	return getSlimeSize() > 1;
}

bool EntitySlime::makesSoundOnLand()
{
	return getSlimeSize() > 2;
}
