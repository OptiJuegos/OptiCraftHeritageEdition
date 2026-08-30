#include "EntitySilverfish.h"
#include "DamageSource.h"

#include "Block.h"
#include "BlockSilverfish.h"
#include "EntityPlayer.h"
#include "MathHelper.h"
#include "World.h"
#include "java/Arithmetic.h"

EntitySilverfish::EntitySilverfish(World *world) :
	EntityMob(world),
	allySummonCooldown(0)
{
	texture = "/mob/silverfish.png";
	setSize(0.3f, 0.7f);
	moveSpeed = 0.6f;
	attackStrength = 1;
	health = 8;
}

int_t EntitySilverfish::getMaxHealth() const
{
	return 8;
}

bool EntitySilverfish::canTriggerWalking()
{
	return false;
}

Entity *EntitySilverfish::findPlayerToAttack()
{
	return worldObj->getClosestVulnerablePlayerToEntity(this, 8.0);
}

jstring EntitySilverfish::getLivingSound()
{
	return "mob.silverfish.say";
}

jstring EntitySilverfish::getHurtSound()
{
	return "mob.silverfish.hit";
}

jstring EntitySilverfish::getDeathSound()
{
	return "mob.silverfish.kill";
}

bool EntitySilverfish::attackEntityFrom(Entity *entity, int_t damage)
{
	if (allySummonCooldown <= 0 && entity != nullptr)
	{
		allySummonCooldown = 20;
	}
	return EntityMob::attackEntityFrom(entity, damage);
}

bool EntitySilverfish::attackEntityFrom(const DamageSource &source, int_t damage)
{
	if (allySummonCooldown <= 0 && source.getEntity() != nullptr)
		allySummonCooldown = 20;
	return EntityMob::attackEntityFrom(source, damage);
}

void EntitySilverfish::attackEntity(Entity *entity, float distance)
{
	if (attackTime <= 0 && distance < 1.2f && entity->boundingBox->maxY > boundingBox->minY && entity->boundingBox->minY < boundingBox->maxY)
	{
		attackTime = 20;
		entity->attackEntityFrom(DamageSource::causeMobDamage(this), attackStrength);
	}
}

void EntitySilverfish::playStepSound(int_t, int_t, int_t, int_t)
{
	worldObj->playSoundAtEntity(this, "mob.silverfish.step", 1.0f, 1.0f);
}

int_t EntitySilverfish::getDropItemId()
{
	return 0;
}

void EntitySilverfish::onUpdate()
{
	renderYawOffset = rotationYaw;
	EntityMob::onUpdate();
}

void EntitySilverfish::updatePlayerActionState()
{
	EntityMob::updatePlayerActionState();
	if (worldObj->multiplayerWorld)
	{
		return;
	}

	if (allySummonCooldown > 0)
	{
		--allySummonCooldown;
		if (allySummonCooldown == 0)
		{
			const int_t originX = MathHelper::floor_double(posX);
			const int_t originY = MathHelper::floor_double(posY);
			const int_t originZ = MathHelper::floor_double(posZ);
			bool stop = false;
			for (int_t dy = 0; !stop && dy <= 5 && dy >= -5; dy = dy <= 0 ? 1 - dy : -dy)
			{
				for (int_t dx = 0; !stop && dx <= 10 && dx >= -10; dx = dx <= 0 ? 1 - dx : -dx)
				{
					for (int_t dz = 0; !stop && dz <= 10 && dz >= -10; dz = dz <= 0 ? 1 - dz : -dz)
					{
						const int_t x = JavaArithmetic::intAdd(originX, dx);
						const int_t y = JavaArithmetic::intAdd(originY, dy);
						const int_t z = JavaArithmetic::intAdd(originZ, dz);
						if (Block::silverfish != nullptr && worldObj->getBlockId(x, y, z) == Block::silverfish->blockID)
						{
							worldObj->playAuxSFX(2001, x, y, z, Block::silverfish->blockID + (worldObj->getBlockMetadata(x, y, z) << 12));
							worldObj->setBlockWithNotify(x, y, z, 0);
							Block::silverfish->onBlockDestroyedByPlayer(worldObj, x, y, z, 0);
							if (rand.nextBoolean())
							{
								stop = true;
							}
						}
					}
				}
			}
		}
	}

	if (playerToAttack == nullptr && !hasPath())
	{
		static const int_t offsetX[6] = {0, 0, 0, 0, -1, 1};
		static const int_t offsetY[6] = {-1, 1, 0, 0, 0, 0};
		static const int_t offsetZ[6] = {0, 0, -1, 1, 0, 0};
		const int_t x = MathHelper::floor_double(posX);
		const int_t y = MathHelper::floor_double(posY + 0.5);
		const int_t z = MathHelper::floor_double(posZ);
		const int_t side = rand.nextInt(6);
		const int_t targetX = JavaArithmetic::intAdd(x, offsetX[side]);
		const int_t targetY = JavaArithmetic::intAdd(y, offsetY[side]);
		const int_t targetZ = JavaArithmetic::intAdd(z, offsetZ[side]);
		const int_t targetId = worldObj->getBlockId(targetX, targetY, targetZ);
		if (BlockSilverfish::getPosingIdByMetadata(targetId) && Block::silverfish != nullptr)
		{
			worldObj->setBlockAndMetadataWithNotify(targetX, targetY, targetZ, Block::silverfish->blockID, BlockSilverfish::getMetadataForBlockType(targetId));
			spawnExplosionParticle();
			setEntityDead();
		}
		else
		{
			updateWanderPath();
		}
	}
	else if (playerToAttack != nullptr && !hasPath())
	{
		playerToAttack = nullptr;
	}
}

float EntitySilverfish::getBlockPathWeight(int_t x, int_t y, int_t z)
{
	return worldObj->getBlockId(x, JavaArithmetic::intSub(y, 1), z) == Block::stone->blockID ? 10.0f : EntityMob::getBlockPathWeight(x, y, z);
}

bool EntitySilverfish::getCanSpawnHere()
{
	if (!EntityCreature::getCanSpawnHere())
	{
		return false;
	}
	return worldObj->getClosestPlayerToEntity(this, 5.0) == nullptr;
}

EnumCreatureAttribute EntitySilverfish::getCreatureAttribute() const
{
	return EnumCreatureAttribute::ARTHROPOD;
}
bool EntitySilverfish::isValidLightLevel()
{
	return true;
}

