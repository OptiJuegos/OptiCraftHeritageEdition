#include "EntityArrow.h"
#include "java/Math.h"
#include "java/Arithmetic.h"

#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "AxisAlignedBB.h"
#include "Block.h"
#include "DamageSource.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "MovingObjectPosition.h"
#include "NBTTagCompound.h"
#include "Vec3D.h"
#include "World.h"
#include "platform/PlatformTuning.h"

EntityArrow::EntityArrow(World *world)
	: Entity(world), xTile(-1), yTile(-1), zTile(-1), inTile(0), inData(0), inGround(false),
	  doesArrowBelongToPlayer(false), arrowShake(0), arrowCritical(false), owner(nullptr), ticksInGround(0),
	  ticksInAir(0), damage(2.0), knockbackStrength(0), shootingEntityId(-1)
{
	ensureEntityInit();
	setSize(0.5f, 0.5f);
}

EntityArrow::EntityArrow(World *world, double x, double y, double z)
	: EntityArrow(world)
{
	setPosition(x, y, z);
	yOffset = 0.0f;
}

EntityArrow::EntityArrow(World *world, EntityLiving *shooter)
	: EntityArrow(world, shooter, 1.0f)
{
}

EntityArrow::EntityArrow(World *world, EntityLiving *shooter, float velocity)
	: EntityArrow(world)
{
	if (shooter == nullptr)
		return;

	setShootingEntity(shooter);
	doesArrowBelongToPlayer = shooter->isPlayer();
	setLocationAndAngles(shooter->posX, shooter->posY + (double)shooter->getEyeHeight(), shooter->posZ,
		shooter->rotationYaw, shooter->rotationPitch);
	posX -= (double)(MathHelper::cos(rotationYaw / 180.0f * (float)M_PI) * 0.16f);
	posY -= static_cast<double>(0.1f);
	posZ -= (double)(MathHelper::sin(rotationYaw / 180.0f * (float)M_PI) * 0.16f);
	setPosition(posX, posY, posZ);
	yOffset = 0.0f;
	motionX = (double)(-MathHelper::sin(rotationYaw / 180.0f * (float)M_PI) * MathHelper::cos(rotationPitch / 180.0f * (float)M_PI));
	motionZ = (double)(MathHelper::cos(rotationYaw / 180.0f * (float)M_PI) * MathHelper::cos(rotationPitch / 180.0f * (float)M_PI));
	motionY = (double)(-MathHelper::sin(rotationPitch / 180.0f * (float)M_PI));
	setArrowHeading(motionX, motionY, motionZ, velocity * 1.5f, 1.0f);
}

EntityArrow::EntityArrow(World *world, EntityLiving *shooter, EntityLiving *target, float speed, float spread)
	: EntityArrow(world)
{
	setSize(0.6f, 1.8f);
	if (shooter == nullptr || target == nullptr)
		return;

	setShootingEntity(shooter);
	doesArrowBelongToPlayer = shooter->isPlayer();
	posY = shooter->posY + (double)shooter->getEyeHeight() - static_cast<double>(0.1f);
	const double dx = target->posX - shooter->posX;
	const double dy = target->posY + (double)target->getEyeHeight() - static_cast<double>(0.7f) - posY;
	const double dz = target->posZ - shooter->posZ;
	const double horizontal = (double)MathHelper::sqrt_double(dx * dx + dz * dz);
	if (horizontal < 1.0e-7)
		return;

#if PLATFORM_FLOAT_ENTITY_CORE_MATH
	const float yaw = std::atan2(static_cast<float>(dz), static_cast<float>(dx)) * 180.0f / static_cast<float>(M_PI) - 90.0f;
	const float pitch = -std::atan2(static_cast<float>(dy), static_cast<float>(horizontal)) * 180.0f / static_cast<float>(M_PI);
#else
	const float yaw = (float)(JavaMath::atan2(dz, dx) * 180.0 / (double)(float)M_PI) - 90.0f;
	const float pitch = (float)(-(JavaMath::atan2(dy, horizontal) * 180.0 / (double)(float)M_PI));
#endif
	const double normX = dx / horizontal;
	const double normZ = dz / horizontal;
	setLocationAndAngles(shooter->posX + normX, posY, shooter->posZ + normZ, yaw, pitch);
	yOffset = 0.0f;
	const float arc = (float)horizontal * 0.2f;
	setArrowHeading(dx, dy + (double)arc, dz, speed, spread);
}

void EntityArrow::entityInit()
{
}

void EntityArrow::setShootingEntity(EntityLiving *shooter)
{
	owner = shooter;
	shootingEntityId = shooter != nullptr ? shooter->entityId : -1;
}

EntityLiving *EntityArrow::getShootingEntity()
{
	if (worldObj == nullptr || shootingEntityId < 0)
	{
		owner = nullptr;
		return nullptr;
	}

	Entity *entity = worldObj->getEntityByID(shootingEntityId);
	owner = entity != nullptr && entity->isLiving() ? static_cast<EntityLiving *>(entity) : nullptr;
	return owner;
}

void EntityArrow::setArrowHeading(double x, double y, double z, float velocity, float inaccuracy)
{
	const float length = MathHelper::sqrt_double(x * x + y * y + z * z);
	if (length <= 0.0f)
		return;
	x /= (double)length;
	y /= (double)length;
	z /= (double)length;
	x += rand.nextGaussian() * static_cast<double>(0.0075f) * (double)inaccuracy;
	y += rand.nextGaussian() * static_cast<double>(0.0075f) * (double)inaccuracy;
	z += rand.nextGaussian() * static_cast<double>(0.0075f) * (double)inaccuracy;
	x *= (double)velocity;
	y *= (double)velocity;
	z *= (double)velocity;
	motionX = x;
	motionY = y;
	motionZ = z;
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
	const float fx = static_cast<float>(x);
	const float fy = static_cast<float>(y);
	const float fz = static_cast<float>(z);
	const float horizontal = MathHelper::sqrt_float(fx * fx + fz * fz);
	prevRotationYaw = rotationYaw = std::atan2(fx, fz) * 180.0f / static_cast<float>(M_PI);
	prevRotationPitch = rotationPitch = std::atan2(fy, horizontal) * 180.0f / static_cast<float>(M_PI);
#else
	const float horizontal = MathHelper::sqrt_double(x * x + z * z);
	prevRotationYaw = rotationYaw = (float)(JavaMath::atan2(x, z) * 180.0 / (double)(float)M_PI);
	prevRotationPitch = rotationPitch = (float)(JavaMath::atan2(y, (double)horizontal) * 180.0 / (double)(float)M_PI);
#endif
	ticksInGround = 0;
}

void EntityArrow::setVelocity(double x, double y, double z)
{
	motionX = x;
	motionY = y;
	motionZ = z;
	if (prevRotationPitch == 0.0f && prevRotationYaw == 0.0f)
	{
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
		const float fx = static_cast<float>(x);
		const float fy = static_cast<float>(y);
		const float fz = static_cast<float>(z);
		const float horizontal = MathHelper::sqrt_float(fx * fx + fz * fz);
		prevRotationYaw = rotationYaw = std::atan2(fx, fz) * 180.0f / static_cast<float>(M_PI);
		prevRotationPitch = rotationPitch = std::atan2(fy, horizontal) * 180.0f / static_cast<float>(M_PI);
#else
		const float horizontal = MathHelper::sqrt_double(x * x + z * z);
		prevRotationYaw = rotationYaw = (float)(JavaMath::atan2(x, z) * 180.0 / (double)(float)M_PI);
		prevRotationPitch = rotationPitch = (float)(JavaMath::atan2(y, (double)horizontal) * 180.0 / (double)(float)M_PI);
#endif
		setLocationAndAngles(posX, posY, posZ, rotationYaw, rotationPitch);
		ticksInGround = 0;
	}
}

void EntityArrow::onUpdate()
{
	Entity::onUpdate();
	EntityLiving *shooter = getShootingEntity();

	if (prevRotationPitch == 0.0f && prevRotationYaw == 0.0f)
	{
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
		const float fx = static_cast<float>(motionX);
		const float fy = static_cast<float>(motionY);
		const float fz = static_cast<float>(motionZ);
		const float horizontal = MathHelper::sqrt_float(fx * fx + fz * fz);
		prevRotationYaw = rotationYaw = std::atan2(fx, fz) * 180.0f / static_cast<float>(M_PI);
		prevRotationPitch = rotationPitch = std::atan2(fy, horizontal) * 180.0f / static_cast<float>(M_PI);
#else
		const float horizontal = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ);
		prevRotationYaw = rotationYaw = (float)(JavaMath::atan2(motionX, motionZ) * 180.0 / (double)(float)M_PI);
		prevRotationPitch = rotationPitch = (float)(JavaMath::atan2(motionY, (double)horizontal) * 180.0 / (double)(float)M_PI);
#endif
	}

	int_t blockId = worldObj->getBlockId(xTile, yTile, zTile);
	if (blockId > 0 && blockId < Block::BLOCK_REGISTRY_SIZE && Block::blocksList[blockId] != nullptr)
	{
		Block::blocksList[blockId]->setBlockBoundsBasedOnState(worldObj, xTile, yTile, zTile);
		AxisAlignedBB *box = Block::blocksList[blockId]->getCollisionBoundingBoxFromPool(worldObj, xTile, yTile, zTile);
		if (box != nullptr && box->isVecInside(Vec3D::createVector(posX, posY, posZ)))
			inGround = true;
	}

	if (arrowShake > 0)
		--arrowShake;

	if (inGround)
	{
		blockId = worldObj->getBlockId(xTile, yTile, zTile);
		const int_t metadata = worldObj->getBlockMetadata(xTile, yTile, zTile);
		if (blockId == inTile && metadata == inData)
		{
			++ticksInGround;
			if (ticksInGround == 1200)
				setEntityDead();
			return;
		}

		inGround = false;
		motionX *= (double)(rand.nextFloat() * 0.2f);
		motionY *= (double)(rand.nextFloat() * 0.2f);
		motionZ *= (double)(rand.nextFloat() * 0.2f);
		ticksInGround = 0;
		ticksInAir = 0;
	}
	else
	{
		++ticksInAir;
	}

	Vec3D *start = Vec3D::createVector(posX, posY, posZ);
	Vec3D *end = Vec3D::createVector(posX + motionX, posY + motionY, posZ + motionZ);
	MovingObjectPosition *hit = worldObj->rayTraceBlocks(start, end, false, true);
	start = Vec3D::createVector(posX, posY, posZ);
	end = Vec3D::createVector(posX + motionX, posY + motionY, posZ + motionZ);
	if (hit != nullptr)
		end = Vec3D::createVector(hit->hitVec->xCoord, hit->hitVec->yCoord, hit->hitVec->zCoord);

	Entity *closestEntity = nullptr;
	double closestDistance = 0.0;
	std::vector<Entity *> &entities = worldObj->getEntitiesWithinAABBExcludingEntity(
		this, boundingBox->addCoord(motionX, motionY, motionZ)->expand(1.0, 1.0, 1.0));
	for (Entity *candidate : entities)
	{
		if (candidate == nullptr || !candidate->canBeCollidedWith() || (candidate == shooter && ticksInAir < 5))
			continue;
		AxisAlignedBB *expanded = candidate->boundingBox->expand(static_cast<double>(0.3f), static_cast<double>(0.3f), static_cast<double>(0.3f));
		MovingObjectPosition *intercept = expanded->calculateIntercept(start, end);
		if (intercept == nullptr)
			continue;
		const double distance = start->distanceTo(intercept->hitVec);
		if (distance < closestDistance || closestDistance == 0.0)
		{
			closestEntity = candidate;
			closestDistance = distance;
		}
		delete intercept;
	}

	if (closestEntity != nullptr)
	{
		delete hit;
		hit = new MovingObjectPosition(closestEntity);
	}

	if (hit != nullptr)
	{
		if (hit->entityHit != nullptr)
		{
			const float speed = MathHelper::sqrt_double(motionX * motionX + motionY * motionY + motionZ * motionZ);
			int_t hitDamage = JavaArithmetic::doubleToInt(std::ceil(static_cast<double>(speed) * damage));
			if (arrowCritical)
				hitDamage += rand.nextInt(hitDamage / 2 + 2);

			DamageSource source = DamageSource::causeArrowDamage(this, shooter != nullptr ? static_cast<Entity *>(shooter) : this);
			if (isBurning())
				hit->entityHit->setFire(5);

			if (hit->entityHit->attackEntityFrom(source, hitDamage))
			{
				if (hit->entityHit->isLiving())
				{
					EntityLiving *living = static_cast<EntityLiving *>(hit->entityHit);
					++living->arrowHitTempCounter;
					if (knockbackStrength > 0)
					{
						const float horizontal = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ);
						if (horizontal > 0.0f)
							hit->entityHit->addVelocity(motionX * (double)knockbackStrength * static_cast<double>(0.6f) / (double)horizontal, 0.1,
								motionZ * (double)knockbackStrength * static_cast<double>(0.6f) / (double)horizontal);
					}
				}
				worldObj->playSoundAtEntity(this, "random.bowhit", 1.0f, 1.2f / (rand.nextFloat() * 0.2f + 0.9f));
				setEntityDead();
			}
			else
			{
				motionX *= static_cast<double>(-0.1f);
				motionY *= static_cast<double>(-0.1f);
				motionZ *= static_cast<double>(-0.1f);
				rotationYaw += 180.0f;
				prevRotationYaw += 180.0f;
				ticksInAir = 0;
			}
		}
		else
		{
			xTile = hit->blockX;
			yTile = hit->blockY;
			zTile = hit->blockZ;
			inTile = worldObj->getBlockId(xTile, yTile, zTile);
			inData = worldObj->getBlockMetadata(xTile, yTile, zTile);
			motionX = (double)(float)(hit->hitVec->xCoord - posX);
			motionY = (double)(float)(hit->hitVec->yCoord - posY);
			motionZ = (double)(float)(hit->hitVec->zCoord - posZ);
			const float speed = MathHelper::sqrt_double(motionX * motionX + motionY * motionY + motionZ * motionZ);
			if (speed > 0.0f)
			{
				posX -= motionX / (double)speed * static_cast<double>(0.05f);
				posY -= motionY / (double)speed * static_cast<double>(0.05f);
				posZ -= motionZ / (double)speed * static_cast<double>(0.05f);
			}
			worldObj->playSoundAtEntity(this, "random.bowhit", 1.0f, 1.2f / (rand.nextFloat() * 0.2f + 0.9f));
			inGround = true;
			arrowShake = 7;
			arrowCritical = false;
		}
		delete hit;
	}

	if (arrowCritical)
	{
		for (int_t i = 0; i < 4; ++i)
			worldObj->spawnParticle("crit", posX + motionX * (double)i / 4.0, posY + motionY * (double)i / 4.0,
				posZ + motionZ * (double)i / 4.0, -motionX, -motionY + 0.2, -motionZ);
	}

	posX += motionX;
	posY += motionY;
	posZ += motionZ;
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
	const float motionXf = static_cast<float>(motionX);
	const float motionYf = static_cast<float>(motionY);
	const float motionZf = static_cast<float>(motionZ);
	const float horizontalSpeed = MathHelper::sqrt_float(motionXf * motionXf + motionZf * motionZf);
	rotationYaw = std::atan2(motionXf, motionZf) * 180.0f / static_cast<float>(M_PI);
	for (rotationPitch = std::atan2(motionYf, horizontalSpeed) * 180.0f / static_cast<float>(M_PI);
#else
	const float horizontalSpeed = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ);
	rotationYaw = (float)(JavaMath::atan2(motionX, motionZ) * 180.0 / (double)(float)M_PI);
	for (rotationPitch = (float)(JavaMath::atan2(motionY, (double)horizontalSpeed) * 180.0 / (double)(float)M_PI);
#endif
		rotationPitch - prevRotationPitch < -180.0f; prevRotationPitch -= 360.0f) {}
	while (rotationPitch - prevRotationPitch >= 180.0f) prevRotationPitch += 360.0f;
	while (rotationYaw - prevRotationYaw < -180.0f) prevRotationYaw -= 360.0f;
	while (rotationYaw - prevRotationYaw >= 180.0f) prevRotationYaw += 360.0f;
	rotationPitch = prevRotationPitch + (rotationPitch - prevRotationPitch) * 0.2f;
	rotationYaw = prevRotationYaw + (rotationYaw - prevRotationYaw) * 0.2f;

	float drag = 0.99f;
	const float gravity = 0.05f;
	if (isInWater())
	{
		for (int_t i = 0; i < 4; ++i)
			worldObj->spawnParticle("bubble", posX - motionX * 0.25, posY - motionY * 0.25, posZ - motionZ * 0.25,
				motionX, motionY, motionZ);
		drag = 0.8f;
	}
	motionX *= (double)drag;
	motionY *= (double)drag;
	motionZ *= (double)drag;
	motionY -= (double)gravity;
	setPosition(posX, posY, posZ);
}

void EntityArrow::writeEntityToNBT(NBTTagCompound *compound)
{
	compound->setShort("xTile", JavaArithmetic::shortFromBits(static_cast<ushort_t>(xTile)));
	compound->setShort("yTile", JavaArithmetic::shortFromBits(static_cast<ushort_t>(yTile)));
	compound->setShort("zTile", JavaArithmetic::shortFromBits(static_cast<ushort_t>(zTile)));
	compound->setByte("inTile", JavaArithmetic::byteFromBits(static_cast<ubyte_t>(inTile)));
	compound->setByte("inData", JavaArithmetic::byteFromBits(static_cast<ubyte_t>(inData)));
	compound->setByte("shake", JavaArithmetic::byteFromBits(static_cast<ubyte_t>(arrowShake)));
	compound->setByte("inGround", (byte_t)(inGround ? 1 : 0));
	compound->setBoolean("player", doesArrowBelongToPlayer);
	compound->setDouble("damage", damage);
}

void EntityArrow::readEntityFromNBT(NBTTagCompound *compound)
{
	xTile = compound->getShort("xTile");
	yTile = compound->getShort("yTile");
	zTile = compound->getShort("zTile");
	inTile = compound->getByte("inTile") & 0xff;
	inData = compound->getByte("inData") & 0xff;
	arrowShake = compound->getByte("shake") & 0xff;
	inGround = compound->getByte("inGround") == 1;
	doesArrowBelongToPlayer = compound->getBoolean("player");
	if (compound->hasKey("damage"))
		damage = compound->getDouble("damage");
}

void EntityArrow::onCollideWithPlayer(EntityPlayer *player)
{
	if (!worldObj->multiplayerWorld && inGround && doesArrowBelongToPlayer && arrowShake <= 0)
	{
		ItemStack *pickup = new ItemStack(Item::arrow, 1);
		const bool added = player->inventory->addItemStackToInventory(pickup);
		delete pickup;
		if (added)
		{
			worldObj->playSoundAtEntity(this, "random.pop", 0.2f,
				((rand.nextFloatDifference()) * 0.7f + 1.0f) * 2.0f);
			player->onItemPickup(this, 1);
			setEntityDead();
		}
	}
}

float EntityArrow::getShadowSize()
{
	return 0.0f;
}

void EntityArrow::setDamage(double value)
{
	damage = value;
}

double EntityArrow::getDamage() const
{
	return damage;
}

void EntityArrow::setKnockbackStrength(int_t strength)
{
	knockbackStrength = strength;
}

bool EntityArrow::canAttackWithItem()
{
	return false;
}
