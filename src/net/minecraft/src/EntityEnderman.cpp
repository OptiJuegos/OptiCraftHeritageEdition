#include "EntityEnderman.h"
#include "java/Arithmetic.h"

#include <algorithm>

#include "AxisAlignedBB.h"
#include "Block.h"
#include "BlockFlower.h"
#include "BlockGrass.h"
#include "BlockMycelium.h"
#include "DamageSource.h"
#include "DataWatcher.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "Material.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "Vec3D.h"
#include "World.h"

std::array<bool, 4096> EntityEnderman::canCarryBlocks = {};
bool EntityEnderman::carryableBlocksInitialized = false;

EntityEnderman::EntityEnderman(World *world)
	: EntityMob(world), isAttacking(false), teleportDelay(0), stareTimer(0)
{
	ensureEntityInit();
	initializeCarryableBlocks();
	texture = "/mob/enderman.png";
	moveSpeed = 0.2f;
	attackStrength = 7;
	setSize(0.6f, 2.9f);
	stepHeight = 1.0f;
	setHealth(getMaxHealth());
}

int_t EntityEnderman::getMaxHealth() const
{
	return 40;
}

void EntityEnderman::entityInit()
{
	EntityMob::entityInit();
	if (!dataWatcher->hasObject(16))
		dataWatcher->addObject(16, static_cast<byte_t>(0));
	if (!dataWatcher->hasObject(17))
		dataWatcher->addObject(17, static_cast<byte_t>(0));
}

void EntityEnderman::writeEntityToNBT(NBTTagCompound *tag)
{
	EntityMob::writeEntityToNBT(tag);
	tag->setShort("carried", JavaArithmetic::shortFromBits(static_cast<ushort_t>(getCarried())));
	tag->setShort("carriedData", JavaArithmetic::shortFromBits(static_cast<ushort_t>(getCarryingData())));
}

void EntityEnderman::readEntityFromNBT(NBTTagCompound *tag)
{
	EntityMob::readEntityFromNBT(tag);
	setCarried(tag->getShort("carried"));
	setCarryingData(tag->getShort("carriedData"));
}

Entity *EntityEnderman::findPlayerToAttack()
{
	EntityPlayer *player = worldObj->getClosestVulnerablePlayerToEntity(this, 64.0);
	if (player != nullptr)
	{
		if (shouldAttackPlayer(player))
		{
			if (stareTimer++ == 5)
			{
				stareTimer = 0;
				return player;
			}
		}
		else
		{
			stareTimer = 0;
		}
	}
	return nullptr;
}

bool EntityEnderman::shouldAttackPlayer(EntityPlayer *player)
{
	if (player == nullptr || player->inventory == nullptr)
		return false;
	ItemStack *helmet = player->inventory->armorInventory[3];
	if (helmet != nullptr && Block::pumpkin != nullptr && helmet->itemID == Block::pumpkin->blockID)
		return false;

	Vec3D *look = player->getLook(1.0f)->normalize();
	Vec3D *towards = Vec3D::createVector(posX - player->posX,
		boundingBox->minY + static_cast<double>(height / 2.0f) - (player->posY + static_cast<double>(player->getEyeHeight())),
		posZ - player->posZ);
	const double distance = towards->lengthVector();
	if (distance <= 0.0001)
		return true;
	towards = towards->normalize();
	const double dot = look->dotProduct(towards);
	return dot > 1.0 - 0.025 / distance && player->canEntityBeSeen(this);
}

void EntityEnderman::onLivingUpdate()
{
	if (isWet())
		attackEntityFrom(DamageSource::drown, 1);

	Entity *target = getTarget();
	isAttacking = target != nullptr;
	moveSpeed = target != nullptr ? 6.5f : 0.3f;

	if (!worldObj->multiplayerWorld)
	{
		if (getCarried() == 0)
		{
			if (rand.nextInt(20) == 0)
			{
				const int_t x = MathHelper::floor_double(posX - 2.0 + rand.nextDouble() * 4.0);
				const int_t y = MathHelper::floor_double(posY + rand.nextDouble() * 3.0);
				const int_t z = MathHelper::floor_double(posZ - 2.0 + rand.nextDouble() * 4.0);
				const int_t blockId = worldObj->getBlockId(x, y, z);
				if (blockId >= 0 && blockId < static_cast<int_t>(canCarryBlocks.size()) && canCarryBlocks[blockId])
				{
					setCarried(blockId);
					setCarryingData(worldObj->getBlockMetadata(x, y, z));
					worldObj->setBlockWithNotify(x, y, z, 0);
				}
			}
		}
		else if (rand.nextInt(2000) == 0)
		{
			const int_t x = MathHelper::floor_double(posX - 1.0 + rand.nextDouble() * 2.0);
			const int_t y = MathHelper::floor_double(posY + rand.nextDouble() * 2.0);
			const int_t z = MathHelper::floor_double(posZ - 1.0 + rand.nextDouble() * 2.0);
			const int_t currentId = worldObj->getBlockId(x, y, z);
			const int_t belowId = worldObj->getBlockId(x, y - 1, z);
			if (currentId == 0 && belowId > 0 && belowId < Block::BLOCK_REGISTRY_SIZE && Block::blocksList[belowId] != nullptr &&
				Block::blocksList[belowId]->renderAsNormalBlock())
			{
				worldObj->setBlockAndMetadataWithNotify(x, y, z, getCarried(), getCarryingData());
				setCarried(0);
			}
		}
	}

	for (int_t i = 0; i < 2; ++i)
	{
		const double particleX = posX + (rand.nextDouble() - 0.5) * static_cast<double>(width);
		const double particleY = posY + rand.nextDouble() * static_cast<double>(height) - 0.25;
		const double particleZ = posZ + (rand.nextDouble() - 0.5) * static_cast<double>(width);
		const double motionX = (rand.nextDouble() - 0.5) * 2.0;
		const double motionY = -rand.nextDouble();
		const double motionZ = (rand.nextDouble() - 0.5) * 2.0;
		worldObj->spawnParticle("portal", particleX, particleY, particleZ, motionX, motionY, motionZ);
	}

	if (worldObj->isDaytime() && !worldObj->multiplayerWorld)
	{
		const float brightness = getEntityBrightness(1.0f);
		if (brightness > 0.5f && worldObj->canBlockSeeTheSky(MathHelper::floor_double(posX), MathHelper::floor_double(posY),
			MathHelper::floor_double(posZ)) && rand.nextFloat() * 30.0f < (brightness - 0.4f) * 2.0f)
		{
			setTarget(nullptr);
			teleportRandomly();
		}
	}

	if (isWet())
	{
		setTarget(nullptr);
		teleportRandomly();
	}

	isJumping = false;
	target = getTarget();
	if (target != nullptr)
		faceEntity(target, 100.0f, 100.0f);

	if (!worldObj->multiplayerWorld && isEntityAlive())
	{
		target = getTarget();
		if (target != nullptr)
		{
			EntityPlayer *player = dynamic_cast<EntityPlayer *>(target);
			if (player != nullptr && shouldAttackPlayer(player))
			{
				moveStrafing = moveForward = 0.0f;
				moveSpeed = 0.0f;
				if (target->getDistanceSqToEntity(this) < 16.0)
					teleportRandomly();
				teleportDelay = 0;
			}
			else if (target->getDistanceSqToEntity(this) > 256.0 && teleportDelay++ >= 30 && teleportToEntity(target))
			{
				teleportDelay = 0;
			}
		}
		else
		{
			teleportDelay = 0;
		}
	}

	EntityMob::onLivingUpdate();
}

bool EntityEnderman::teleportRandomly()
{
	const double x = posX + (rand.nextDouble() - 0.5) * 64.0;
	const double y = posY + static_cast<double>(rand.nextInt(64) - 32);
	const double z = posZ + (rand.nextDouble() - 0.5) * 64.0;
	return teleportTo(x, y, z);
}

bool EntityEnderman::teleportToEntity(Entity *entity)
{
	if (entity == nullptr)
		return false;
	Vec3D *direction = Vec3D::createVector(posX - entity->posX,
		boundingBox->minY + static_cast<double>(height / 2.0f) - entity->posY + static_cast<double>(entity->getEyeHeight()),
		posZ - entity->posZ)->normalize();
	const double distance = 16.0;
	const double x = posX + (rand.nextDouble() - 0.5) * 8.0 - direction->xCoord * distance;
	const double y = posY + static_cast<double>(rand.nextInt(16) - 8) - direction->yCoord * distance;
	const double z = posZ + (rand.nextDouble() - 0.5) * 8.0 - direction->zCoord * distance;
	return teleportTo(x, y, z);
}

bool EntityEnderman::teleportTo(double x, double y, double z)
{
	const double oldX = posX;
	const double oldY = posY;
	const double oldZ = posZ;
	posX = x;
	posY = y;
	posZ = z;
	bool valid = false;
	const int_t blockX = MathHelper::floor_double(posX);
	int_t blockY = MathHelper::floor_double(posY);
	const int_t blockZ = MathHelper::floor_double(posZ);
	if (worldObj->blockExists(blockX, blockY, blockZ))
	{
		bool foundGround = false;
		while (!foundGround && blockY > 0)
		{
			const int_t belowId = worldObj->getBlockId(blockX, blockY - 1, blockZ);
			if (belowId > 0 && belowId < Block::BLOCK_REGISTRY_SIZE && Block::blocksList[belowId] != nullptr &&
				Block::blocksList[belowId]->blockMaterial->blocksMovement())
			{
				foundGround = true;
			}
			else
			{
				--posY;
				--blockY;
			}
		}
		if (foundGround)
		{
			setPosition(posX, posY, posZ);
			valid = worldObj->getCollidingBoundingBoxes(this, boundingBox).empty() && !worldObj->isAnyLiquid(boundingBox);
		}
	}

	if (!valid)
	{
		setPosition(oldX, oldY, oldZ);
		return false;
	}

	constexpr int_t particleCount = 128;
	for (int_t i = 0; i < particleCount; ++i)
	{
		const double t = static_cast<double>(i) / static_cast<double>(particleCount - 1);
		const float motionXParticle = (rand.nextFloat() - 0.5f) * 0.2f;
		const float motionYParticle = (rand.nextFloat() - 0.5f) * 0.2f;
		const float motionZParticle = (rand.nextFloat() - 0.5f) * 0.2f;
		const double particleX = oldX + (posX - oldX) * t + (rand.nextDouble() - 0.5) * static_cast<double>(width) * 2.0;
		const double particleY = oldY + (posY - oldY) * t + rand.nextDouble() * static_cast<double>(height);
		const double particleZ = oldZ + (posZ - oldZ) * t + (rand.nextDouble() - 0.5) * static_cast<double>(width) * 2.0;
		worldObj->spawnParticle("portal", particleX, particleY, particleZ,
			static_cast<double>(motionXParticle), static_cast<double>(motionYParticle), static_cast<double>(motionZParticle));
	}
	worldObj->playSoundEffect(oldX, oldY, oldZ, "mob.endermen.portal", 1.0f, 1.0f);
	worldObj->playSoundAtEntity(this, "mob.endermen.portal", 1.0f, 1.0f);
	return true;
}

jstring EntityEnderman::getLivingSound()
{
	return "mob.endermen.idle";
}

jstring EntityEnderman::getHurtSound()
{
	return "mob.endermen.hit";
}

jstring EntityEnderman::getDeathSound()
{
	return "mob.endermen.death";
}

int_t EntityEnderman::getDropItemId()
{
	return Item::enderPearl != nullptr ? Item::enderPearl->shiftedIndex : 0;
}

void EntityEnderman::dropFewItems(bool, int_t lootingLevel)
{
	const int_t itemId = getDropItemId();
	if (itemId <= 0)
		return;
	const int_t count = rand.nextInt(2 + lootingLevel);
	for (int_t i = 0; i < count; ++i)
		dropItem(itemId, 1);
}

void EntityEnderman::setCarried(int_t blockId)
{
	dataWatcher->updateObject(16, static_cast<byte_t>(blockId & 255));
}

int_t EntityEnderman::getCarried()
{
	return dataWatcher->getWatchableObjectByte(16) & 255;
}

void EntityEnderman::setCarryingData(int_t metadata)
{
	dataWatcher->updateObject(17, static_cast<byte_t>(metadata & 255));
}

int_t EntityEnderman::getCarryingData()
{
	return dataWatcher->getWatchableObjectByte(17) & 255;
}

bool EntityEnderman::attackEntityFrom(const DamageSource &source, int_t damage)
{
	if (source.getSourceOfDamage() != source.getEntity())
	{
		for (int_t i = 0; i < 64; ++i)
		{
			if (teleportRandomly())
				return true;
		}
		return false;
	}
	return EntityMob::attackEntityFrom(source, damage);
}

void EntityEnderman::initializeCarryableBlocks()
{
	if (carryableBlocksInitialized)
		return;
	carryableBlocksInitialized = true;
	const Block *blocks[] = {
		Block::grass, Block::dirt, Block::sand, Block::gravel, Block::plantYellow, Block::plantRed,
		Block::mushroomBrown, Block::mushroomRed, Block::tnt, Block::cactus, Block::blockClay,
		Block::pumpkin, Block::melon, Block::mycelium
	};
	for (const Block *block : blocks)
	{
		if (block != nullptr && block->blockID >= 0 && block->blockID < static_cast<int_t>(canCarryBlocks.size()))
			canCarryBlocks[block->blockID] = true;
	}
}
