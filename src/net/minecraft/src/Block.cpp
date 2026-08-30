#include "Block.h"

#include "AxisAlignedBB.h"
#include "BlockBed.h"
#include "BlockBookshelf.h"
#include "BlockButton.h"
#include "BlockCactus.h"
#include "BlockCake.h"
#include "BlockChest.h"
#include "BlockClay.h"
#include "BlockCloth.h"
#include "BlockCrops.h"
#include "BlockDeadBush.h"
#include "BlockDetectorRail.h"
#include "BlockDirt.h"
#include "BlockDispenser.h"
#include "BlockDoor.h"
#include "BlockFarmland.h"
#include "BlockFence.h"
#include "BlockFenceGate.h"
#include "BlockPane.h"
#include "BlockWood.h"
#include "BlockStoneBrick.h"
#include "BlockEnchantmentTable.h"
#include "BlockEndPortal.h"
#include "BlockCauldron.h"
#include "BlockBrewingStand.h"
#include "BlockEndPortalFrame.h"
#include "BlockDragonEgg.h"
#include "BlockRedstoneLight.h"
#include "BlockFire.h"
#include "BlockFlower.h"
#include "BlockFlowing.h"
#include "BlockFurnace.h"
#include "BlockGlass.h"
#include "BlockGlowStone.h"
#include "BlockGrass.h"
#include "BlockGravel.h"
#include "BlockIce.h"
#include "BlockJukeBox.h"
#include "BlockLadder.h"
#include "BlockLeaves.h"
#include "BlockLever.h"
#include "BlockLockedChest.h"
#include "BlockVine.h"
#include "BlockMycelium.h"
#include "BlockMushroomCap.h"
#include "BlockLilyPad.h"
#include "BlockLog.h"
#include "BlockMobSpawner.h"
#include "BlockMushroom.h"
#include "BlockMelon.h"
#include "BlockStem.h"
#include "BlockNetherStalk.h"
#include "BlockNetherrack.h"
#include "BlockNote.h"
#include "BlockObsidian.h"
#include "BlockOre.h"
#include "BlockOreStorage.h"
#include "BlockPistonBase.h"
#include "BlockPistonExtension.h"
#include "BlockPistonMoving.h"
#include "BlockPortal.h"
#include "BlockPressurePlate.h"
#include "BlockPumpkin.h"
#include "BlockRail.h"
#include "BlockRedstoneOre.h"
#include "BlockRedstoneRepeater.h"
#include "BlockRedstoneTorch.h"
#include "BlockRedstoneWire.h"
#include "BlockReed.h"
#include "BlockSand.h"
#include "BlockSandStone.h"
#include "BlockSilverfish.h"
#include "BlockSapling.h"
#include "BlockSign.h"
#include "BlockSnow.h"
#include "BlockSnowBlock.h"
#include "BlockSoulSand.h"
#include "BlockSponge.h"
#include "BlockStairs.h"
#include "BlockStationary.h"
#include "BlockStep.h"
#include "BlockStone.h"
#include "BlockTallGrass.h"
#include "BlockTNT.h"
#include "BlockTorch.h"
#include "BlockTrapDoor.h"
#include "BlockWeb.h"
#include "BlockWorkbench.h"
#include "EntityItem.h"
#include "EntityPlayer.h"
#include "EnchantmentHelper.h"
#include "IBlockAccess.h"
#include "Item.h"
#include "ItemBlock.h"
#include "ItemMetadata.h"
#include "ItemLilyPad.h"
#include "ItemColored.h"
#include "ItemCloth.h"
#include "ItemLeaves.h"
#include "ItemLog.h"
#include "ItemPiston.h"
#include "ItemSapling.h"
#include "ItemSlab.h"
#include "ItemStack.h"
#include "Material.h"
#include "MovingObjectPosition.h"
#include "StatCollector.h"
#include "StatList.h"
#include "StepSound.h"
#include "StepSoundSand.h"
#include "StepSoundStone.h"
#include "TileEntitySign.h"
#include "Vec3D.h"
#include "World.h"
#include "java/Random.h"
#include <stdexcept>

StepSound *Block::soundPowderFootstep = nullptr;
StepSound *Block::soundWoodFootstep = nullptr;
StepSound *Block::soundGravelFootstep = nullptr;
StepSound *Block::soundGrassFootstep = nullptr;
StepSound *Block::soundStoneFootstep = nullptr;
StepSound *Block::soundMetalFootstep = nullptr;
StepSound *Block::soundGlassFootstep = nullptr;
StepSound *Block::soundClothFootstep = nullptr;
StepSound *Block::soundSandFootstep = nullptr;

Block *Block::blocksList[Block::BLOCK_REGISTRY_SIZE] = {nullptr};
bool Block::tickOnLoad[Block::BLOCK_REGISTRY_SIZE] = {false};
bool Block::opaqueCubeLookup[Block::BLOCK_REGISTRY_SIZE] = {false};
bool Block::usesDefaultFaceCullingLookup[Block::BLOCK_REGISTRY_SIZE] = {false};
bool Block::isBlockContainer[Block::BLOCK_REGISTRY_SIZE] = {false};
int_t Block::lightOpacity[Block::BLOCK_REGISTRY_SIZE] = {0};
bool Block::lightOpacityExplicit[Block::BLOCK_REGISTRY_SIZE] = {false};
bool Block::canBlockGrass[Block::BLOCK_REGISTRY_SIZE] = {false};
int_t Block::lightValue[Block::BLOCK_REGISTRY_SIZE] = {0};
bool Block::requiresSelfNotify[Block::BLOCK_REGISTRY_SIZE] = {false};
bool Block::useNeighborBrightness[Block::BLOCK_REGISTRY_SIZE] = {false};

Block *Block::stone = nullptr;
BlockGrass *Block::grass = nullptr;
Block *Block::dirt = nullptr;
Block *Block::cobblestone = nullptr;
Block *Block::planks = nullptr;
Block *Block::sapling = nullptr;
Block *Block::bedrock = nullptr;
Block *Block::waterMoving = nullptr;
Block *Block::waterStill = nullptr;
Block *Block::lavaMoving = nullptr;
Block *Block::lavaStill = nullptr;
Block *Block::sand = nullptr;
Block *Block::gravel = nullptr;
Block *Block::oreGold = nullptr;
Block *Block::oreIron = nullptr;
Block *Block::oreCoal = nullptr;
Block *Block::wood = nullptr;
BlockLeaves *Block::leaves = nullptr;
Block *Block::sponge = nullptr;
Block *Block::glass = nullptr;
Block *Block::oreLapis = nullptr;
Block *Block::blockLapis = nullptr;
Block *Block::dispenser = nullptr;
Block *Block::sandStone = nullptr;
Block *Block::musicBlock = nullptr;
Block *Block::blockBed = nullptr;
Block *Block::railPowered = nullptr;
Block *Block::railDetector = nullptr;
Block *Block::pistonStickyBase = nullptr;
Block *Block::web = nullptr;
BlockTallGrass *Block::tallGrass = nullptr;
BlockDeadBush *Block::deadBush = nullptr;
Block *Block::pistonBase = nullptr;
BlockPistonExtension *Block::pistonExtension = nullptr;
Block *Block::cloth = nullptr;
BlockPistonMoving *Block::pistonMoving = nullptr;
BlockFlower *Block::plantYellow = nullptr;
BlockFlower *Block::plantRed = nullptr;
BlockFlower *Block::mushroomBrown = nullptr;
BlockFlower *Block::mushroomRed = nullptr;
Block *Block::blockGold = nullptr;
Block *Block::blockSteel = nullptr;
Block *Block::stairDouble = nullptr;
Block *Block::stairSingle = nullptr;
Block *Block::brick = nullptr;
Block *Block::tnt = nullptr;
Block *Block::bookShelf = nullptr;
Block *Block::cobblestoneMossy = nullptr;
Block *Block::obsidian = nullptr;
Block *Block::torchWood = nullptr;
BlockFire *Block::fire = nullptr;
Block *Block::mobSpawner = nullptr;
Block *Block::stairCompactPlanks = nullptr;
Block *Block::chest = nullptr;
Block *Block::redstoneWire = nullptr;
Block *Block::oreDiamond = nullptr;
Block *Block::blockDiamond = nullptr;
Block *Block::workbench = nullptr;
Block *Block::crops = nullptr;
Block *Block::tilledField = nullptr;
Block *Block::stoneOvenIdle = nullptr;
Block *Block::stoneOvenActive = nullptr;
Block *Block::signPost = nullptr;
Block *Block::doorWood = nullptr;
Block *Block::ladder = nullptr;
Block *Block::rail = nullptr;
Block *Block::stairCompactCobblestone = nullptr;
Block *Block::signWall = nullptr;
Block *Block::lever = nullptr;
Block *Block::pressurePlateStone = nullptr;
Block *Block::doorSteel = nullptr;
Block *Block::pressurePlatePlanks = nullptr;
Block *Block::oreRedstone = nullptr;
Block *Block::oreRedstoneGlowing = nullptr;
Block *Block::torchRedstoneIdle = nullptr;
Block *Block::torchRedstoneActive = nullptr;
Block *Block::button = nullptr;
Block *Block::snow = nullptr;
Block *Block::ice = nullptr;
Block *Block::blockSnow = nullptr;
Block *Block::cactus = nullptr;
Block *Block::blockClay = nullptr;
Block *Block::reed = nullptr;
Block *Block::jukebox = nullptr;
Block *Block::fence = nullptr;
Block *Block::pumpkin = nullptr;
Block *Block::netherrack = nullptr;
Block *Block::slowSand = nullptr;
Block *Block::glowStone = nullptr;
BlockPortal *Block::portal = nullptr;
Block *Block::pumpkinLantern = nullptr;
Block *Block::cake = nullptr;
Block *Block::redstoneRepeaterIdle = nullptr;
Block *Block::redstoneRepeaterActive = nullptr;
Block *Block::lockedChest = nullptr;
Block *Block::trapdoor = nullptr;
BlockMushroomCap *Block::mushroomCapBrown = nullptr;
BlockMushroomCap *Block::mushroomCapRed = nullptr;
BlockVine *Block::vine = nullptr;
BlockMycelium *Block::mycelium = nullptr;
BlockLilyPad *Block::waterlily = nullptr;
Block *Block::silverfish = nullptr;
Block *Block::stoneBrick = nullptr;
Block *Block::fenceIron = nullptr;
Block *Block::thinGlass = nullptr;
Block *Block::melon = nullptr;
Block *Block::pumpkinStem = nullptr;
Block *Block::melonStem = nullptr;
Block *Block::fenceGate = nullptr;
Block *Block::stairsBrick = nullptr;
Block *Block::stairsStoneBrickSmooth = nullptr;
Block *Block::netherBrick = nullptr;
Block *Block::netherFence = nullptr;
Block *Block::stairsNetherBrick = nullptr;
Block *Block::netherStalk = nullptr;
Block *Block::enchantmentTable = nullptr;
Block *Block::brewingStand = nullptr;
Block *Block::cauldron = nullptr;
Block *Block::endPortal = nullptr;
Block *Block::endPortalFrame = nullptr;
Block *Block::whiteStone = nullptr;
Block *Block::dragonEgg = nullptr;
Block *Block::redstoneLampIdle = nullptr;
Block *Block::redstoneLampActive = nullptr;

static TileEntity *createTileEntitySign()
{
	return new TileEntitySign();
}

Block::Block(int_t i, Material *material) :
	blockID(i),
	blockConstructorCalled(true),
	enableStats(true),
	stepSound(soundPowderFootstep),
	blockParticleGravity(1.0f),
	blockMaterial(material),
	slipperiness(0.6f)
{
	if (blocksList[i] != nullptr)
	{
		throw std::runtime_error("Block slot is already occupied");
	}
	blocksList[i] = this;
	setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	opaqueCubeLookup[i] = isOpaqueCube();
	usesDefaultFaceCullingLookup[i] = usesDefaultFaceCulling();
	lightOpacity[i] = isOpaqueCube() ? 255 : 0;
	canBlockGrass[i] = !material->getCanBlockGrass();
	isBlockContainer[i] = false;
}

Block::Block(int_t i, int_t j, Material *material) :
	Block(i, material)
{
	blockIndexInTexture = j;
}

Block *Block::setRequiresSelfNotify()
{
	requiresSelfNotify[blockID] = true;
	return this;
}

void Block::initializeBlock()
{
}

Block *Block::setStepSound(StepSound *stepsound)
{
	stepSound = stepsound;
	return this;
}

Block *Block::setLightOpacity(int_t i)
{
	lightOpacity[blockID] = i;
	lightOpacityExplicit[blockID] = true;
	return this;
}

Block *Block::setLightValue(float f)
{
	lightValue[blockID] = (int_t)(15.0f * f);
	return this;
}

Block *Block::setResistance(float f)
{
	blockResistance = f * 3.0f;
	return this;
}

bool Block::isNormalCube(int_t blockId)
{
	if (blockId < 0 || blockId >= BLOCK_REGISTRY_SIZE)
		return false;
	Block *block = blocksList[blockId];
	return block != nullptr && block->blockMaterial->isOpaque() && block->renderAsNormalBlock();
}

bool Block::renderAsNormalBlock()
{
	return true;
}

bool Block::getBlocksMovement(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	return !blockMaterial->blocksMovement();
}

int_t Block::getRenderType()
{
	return 0;
}

Block *Block::setHardness(float f)
{
	blockHardness = f;
	if (blockResistance < f * 5.0f)
	{
		blockResistance = f * 5.0f;
	}
	return this;
}

Block *Block::setBlockUnbreakable()
{
	setHardness(-1.0f);
	return this;
}

float Block::getHardness()
{
	return blockHardness;
}

Block *Block::setTickOnLoad(bool flag)
{
	tickOnLoad[blockID] = flag;
	return this;
}

Block *Block::setTickRandomly(bool flag)
{
	return setTickOnLoad(flag);
}

bool Block::getTickRandomly() const
{
	return blockID >= 0 && blockID < BLOCK_REGISTRY_SIZE && tickOnLoad[blockID];
}

bool Block::hasTileEntity() const
{
	return blockID >= 0 && blockID < BLOCK_REGISTRY_SIZE && isBlockContainer[blockID];
}

void Block::setBlockBounds(float f, float f1, float f2, float f3, float f4, float f5)
{
	minX = f;
	minY = f1;
	minZ = f2;
	maxX = f3;
	maxY = f4;
	maxZ = f5;
}

float Block::getBlockBrightness(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	return iblockaccess->getBrightness(i, j, k, lightValue[blockID]);
}

int_t Block::getMixedBrightnessForBlock(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	return iblockaccess->getLightBrightnessForSkyBlocks(i, j, k, lightValue[blockID]);
}

bool Block::shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	if (l == 0 && minY > 0.0) return true;
	if (l == 1 && maxY < 1.0) return true;
	if (l == 2 && minZ > 0.0) return true;
	if (l == 3 && maxZ < 1.0) return true;
	if (l == 4 && minX > 0.0) return true;
	if (l == 5 && maxX < 1.0) return true;
	return !iblockaccess->isBlockOpaqueCube(i, j, k);
}

bool Block::getIsBlockSolid(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	return iblockaccess->getBlockMaterial(i, j, k)->isSolid();
}

bool Block::isBlockSolid(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	return getIsBlockSolid(iblockaccess, i, j, k, l);
}

int_t Block::getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	return getBlockTextureFromSideAndMetadata(l, iblockaccess->getBlockMetadata(i, j, k));
}

int_t Block::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	return getBlockTextureFromSide(i);
}

int_t Block::getBlockTextureFromSide(int_t i)
{
	return blockIndexInTexture;
}

AxisAlignedBB *Block::getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	return AxisAlignedBB::getBoundingBoxFromPool((double)i + minX, (double)j + minY, (double)k + minZ, (double)i + maxX, (double)j + maxY, (double)k + maxZ);
}

void Block::getCollidingBoundingBoxes(World *world, int_t i, int_t j, int_t k, AxisAlignedBB *axisalignedbb, std::vector<AxisAlignedBB *> &arraylist)
{
	AxisAlignedBB *axisalignedbb1 = getCollisionBoundingBoxFromPool(world, i, j, k);
	if (axisalignedbb1 != nullptr && axisalignedbb->intersectsWith(axisalignedbb1))
	{
		arraylist.push_back(axisalignedbb1);
	}
}

AxisAlignedBB *Block::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	return AxisAlignedBB::getBoundingBoxFromPool((double)i + minX, (double)j + minY, (double)k + minZ, (double)i + maxX, (double)j + maxY, (double)k + maxZ);
}

bool Block::isOpaqueCube()
{
	return true;
}

bool Block::canCollideCheck(int_t i, bool flag)
{
	return isCollidable();
}

bool Block::isCollidable()
{
	return true;
}

void Block::updateTick(World *world, int_t i, int_t j, int_t k, Random &random) {}
void Block::randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random) {}
void Block::onBlockDestroyedByPlayer(World *world, int_t i, int_t j, int_t k, int_t l) {}
void Block::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) {}

int_t Block::tickRate()
{
	return 10;
}

void Block::onBlockAdded(World *world, int_t i, int_t j, int_t k) {}
void Block::onBlockRemoval(World *world, int_t i, int_t j, int_t k) {}

int_t Block::quantityDropped(Random &random)
{
	return 1;
}

int_t Block::quantityDroppedWithBonus(int_t fortune, Random &random)
{
	return quantityDropped(random);
}

int_t Block::idDropped(int_t i, Random &random)
{
	return blockID;
}

int_t Block::idDropped(int_t i, Random &random, int_t fortune)
{
	return idDropped(i, random);
}

float Block::blockStrength(EntityPlayer *entityplayer)
{
	if (blockHardness < 0.0f)
	{
		return 0.0f;
	}
	if (!entityplayer->canHarvestBlock(this))
	{
		return 1.0f / blockHardness / 100.0f;
	}
	else
	{
		return entityplayer->getCurrentPlayerStrVsBlock(this) / blockHardness / 30.0f;
	}
}

void Block::dropBlockAsItem(World *world, int_t i, int_t j, int_t k, int_t l)
{
	dropBlockAsItem(world, i, j, k, l, 0);
}

void Block::dropBlockAsItem(World *world, int_t i, int_t j, int_t k, int_t l, int_t fortune)
{
	dropBlockAsItemWithChance(world, i, j, k, l, 1.0f, fortune);
}

void Block::dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f)
{
	dropBlockAsItemWithChance(world, i, j, k, l, f, 0);
}

void Block::dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f, int_t fortune)
{
	if (world->multiplayerWorld)
	{
		return;
	}
	int_t i1 = quantityDroppedWithBonus(fortune, world->rand);
	for (int_t j1 = 0; j1 < i1; j1++)
	{
		if (world->rand.nextFloat() > f)
		{
			continue;
		}
		int_t k1 = idDropped(l, world->rand, fortune);
		if (k1 > 0)
		{
			dropBlockAsItem_do(world, i, j, k, new ItemStack(k1, 1, damageDropped(l)));
		}
	}
}

void Block::dropBlockAsItem_do(World *world, int_t i, int_t j, int_t k, ItemStack *itemstack)
{
	if (world->multiplayerWorld)
	{
		return;
	}
	float f = 0.7f;
	double d = (double)(world->rand.nextFloat() * f) + (double)(1.0f - f) * 0.5;
	double d1 = (double)(world->rand.nextFloat() * f) + (double)(1.0f - f) * 0.5;
	double d2 = (double)(world->rand.nextFloat() * f) + (double)(1.0f - f) * 0.5;
	EntityItem *entityitem = new EntityItem(world, (double)i + d, (double)j + d1, (double)k + d2, itemstack);
	entityitem->delayBeforeCanPickup = 10;
	if (!world->entityJoinedWorld(entityitem))
		delete entityitem;
}

int_t Block::damageDropped(int_t i)
{
	return 0;
}

bool Block::func_50074_q()
{
	return renderAsNormalBlock() && !hasTileEntity();
}

ItemStack *Block::createStackedBlock(int_t metadata)
{
	int_t itemMetadata = 0;
	if (blockID >= 0 && blockID < Item::ITEM_LIST_SIZE && Item::itemsList[blockID] != nullptr &&
		Item::itemsList[blockID]->getHasSubtypes())
		itemMetadata = metadata;
	return new ItemStack(blockID, 1, itemMetadata);
}


float Block::getExplosionResistance(Entity *entity)
{
	return blockResistance / 5.0f;
}

MovingObjectPosition *Block::collisionRayTrace(World *world, int_t i, int_t j, int_t k, Vec3D *vec3d, Vec3D *vec3d1)
{
	setBlockBoundsBasedOnState(world, i, j, k);
	vec3d = vec3d->addVector(-i, -j, -k);
	vec3d1 = vec3d1->addVector(-i, -j, -k);
	Vec3D *vec3d2 = vec3d->getIntermediateWithXValue(vec3d1, minX);
	Vec3D *vec3d3 = vec3d->getIntermediateWithXValue(vec3d1, maxX);
	Vec3D *vec3d4 = vec3d->getIntermediateWithYValue(vec3d1, minY);
	Vec3D *vec3d5 = vec3d->getIntermediateWithYValue(vec3d1, maxY);
	Vec3D *vec3d6 = vec3d->getIntermediateWithZValue(vec3d1, minZ);
	Vec3D *vec3d7 = vec3d->getIntermediateWithZValue(vec3d1, maxZ);
	if (!isVecInsideYZBounds(vec3d2)) vec3d2 = nullptr;
	if (!isVecInsideYZBounds(vec3d3)) vec3d3 = nullptr;
	if (!isVecInsideXZBounds(vec3d4)) vec3d4 = nullptr;
	if (!isVecInsideXZBounds(vec3d5)) vec3d5 = nullptr;
	if (!isVecInsideXYBounds(vec3d6)) vec3d6 = nullptr;
	if (!isVecInsideXYBounds(vec3d7)) vec3d7 = nullptr;
	Vec3D *vec3d8 = nullptr;
	if (vec3d2 != nullptr && (vec3d8 == nullptr || vec3d->distanceTo(vec3d2) < vec3d->distanceTo(vec3d8))) vec3d8 = vec3d2;
	if (vec3d3 != nullptr && (vec3d8 == nullptr || vec3d->distanceTo(vec3d3) < vec3d->distanceTo(vec3d8))) vec3d8 = vec3d3;
	if (vec3d4 != nullptr && (vec3d8 == nullptr || vec3d->distanceTo(vec3d4) < vec3d->distanceTo(vec3d8))) vec3d8 = vec3d4;
	if (vec3d5 != nullptr && (vec3d8 == nullptr || vec3d->distanceTo(vec3d5) < vec3d->distanceTo(vec3d8))) vec3d8 = vec3d5;
	if (vec3d6 != nullptr && (vec3d8 == nullptr || vec3d->distanceTo(vec3d6) < vec3d->distanceTo(vec3d8))) vec3d8 = vec3d6;
	if (vec3d7 != nullptr && (vec3d8 == nullptr || vec3d->distanceTo(vec3d7) < vec3d->distanceTo(vec3d8))) vec3d8 = vec3d7;
	if (vec3d8 == nullptr)
	{
		return nullptr;
	}
	byte_t byte0 = -1;
	if (vec3d8 == vec3d2) byte0 = 4;
	if (vec3d8 == vec3d3) byte0 = 5;
	if (vec3d8 == vec3d4) byte0 = 0;
	if (vec3d8 == vec3d5) byte0 = 1;
	if (vec3d8 == vec3d6) byte0 = 2;
	if (vec3d8 == vec3d7) byte0 = 3;
	return new MovingObjectPosition(i, j, k, byte0, vec3d8->addVector(i, j, k));
}

bool Block::isVecInsideYZBounds(Vec3D *vec3d)
{
	return vec3d != nullptr && vec3d->yCoord >= minY && vec3d->yCoord <= maxY && vec3d->zCoord >= minZ && vec3d->zCoord <= maxZ;
}

bool Block::isVecInsideXZBounds(Vec3D *vec3d)
{
	return vec3d != nullptr && vec3d->xCoord >= minX && vec3d->xCoord <= maxX && vec3d->zCoord >= minZ && vec3d->zCoord <= maxZ;
}

bool Block::isVecInsideXYBounds(Vec3D *vec3d)
{
	return vec3d != nullptr && vec3d->xCoord >= minX && vec3d->xCoord <= maxX && vec3d->yCoord >= minY && vec3d->yCoord <= maxY;
}

void Block::onBlockDestroyedByExplosion(World *world, int_t i, int_t j, int_t k) {}

int_t Block::getRenderBlockPass()
{
	return 0;
}

bool Block::canPlaceBlockOnSide(World *world, int_t i, int_t j, int_t k, int_t l)
{
	return canPlaceBlockAt(world, i, j, k);
}

bool Block::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	int_t l = world->getBlockId(i, j, k);
	return l == 0 || blocksList[l]->blockMaterial->getIsGroundCover();
}

bool Block::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	return false;
}

void Block::onEntityWalking(World *world, int_t i, int_t j, int_t k, Entity *entity) {}

void Block::onFallenUpon(World *world, int_t i, int_t j, int_t k, Entity *entity, float fallDistance) {}
void Block::onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t l) {}
void Block::onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) {}
void Block::velocityToAddToEntity(World *world, int_t i, int_t j, int_t k, Entity *entity, Vec3D *vec3d) {}
void Block::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) {}

int_t Block::getBlockColor()
{
	return 0xffffff;
}

int_t Block::getRenderColor(int_t i)
{
	return 0xffffff;
}

int_t Block::colorMultiplier(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	return 0xffffff;
}

bool Block::isPoweringTo(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	return false;
}

bool Block::canProvidePower()
{
	return false;
}

void Block::powerBlock(World *world, int_t i, int_t j, int_t k, int_t eventId, int_t eventParam)
{
	playBlock(world, i, j, k, eventId, eventParam);
}

void Block::onEntityCollidedWithBlock(World *world, int_t i, int_t j, int_t k, Entity *entity) {}

bool Block::isIndirectlyPoweringTo(World *world, int_t i, int_t j, int_t k, int_t l)
{
	return false;
}

void Block::setBlockBoundsForItemRender() {}

float Block::getAmbientOcclusionLightValue(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	return iblockaccess != nullptr && iblockaccess->isBlockNormalCube(i, j, k) ? 0.2f : 1.0f;
}

void Block::harvestBlock(World *world, EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l)
{
	if (world == nullptr || entityplayer == nullptr)
		return;

	entityplayer->addStat(StatList::mineBlockStatArray[blockID], 1);
	entityplayer->addExhaustion(0.025f);
	if (func_50074_q() && EnchantmentHelper::getSilkTouchModifier(entityplayer->inventory))
	{
		ItemStack *stack = createStackedBlock(l);
		if (stack != nullptr)
			dropBlockAsItem_do(world, i, j, k, stack);
	}
	else
	{
		const int_t fortune = EnchantmentHelper::getFortuneModifier(entityplayer->inventory);
		dropBlockAsItem(world, i, j, k, l, fortune);
	}
}

bool Block::canBlockStay(World *world, int_t i, int_t j, int_t k)
{
	return true;
}

void Block::onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving) {}

Block *Block::setBlockName(const char *s)
{
	blockName = s;
	return this;
}

jstring Block::translateBlockName()
{
	return StatCollector::translateToLocal(getBlockName() + ".name");
}

jstring Block::getBlockName()
{
	return blockName ? jstring("tile.") + blockName : "";
}

void Block::playBlock(World *world, int_t i, int_t j, int_t k, int_t l, int_t i1) {}

bool Block::getEnableStats()
{
	return enableStats;
}

Block *Block::disableStats()
{
	enableStats = false;
	return this;
}

int_t Block::getMobilityFlag()
{
	return blockMaterial->getMaterialMobility();
}

void Block::initialize()
{
	soundPowderFootstep = new StepSound("stone", 1.0f, 1.0f);
	soundWoodFootstep = new StepSound("wood", 1.0f, 1.0f);
	soundGravelFootstep = new StepSound("gravel", 1.0f, 1.0f);
	soundGrassFootstep = new StepSound("grass", 1.0f, 1.0f);
	soundStoneFootstep = new StepSound("stone", 1.0f, 1.0f);
	soundMetalFootstep = new StepSound("stone", 1.0f, 1.5f);
	soundGlassFootstep = new StepSoundStone("stone", 1.0f, 1.0f);
	soundClothFootstep = new StepSound("cloth", 1.0f, 1.0f);
	soundSandFootstep = new StepSoundSand("sand", 1.0f, 1.0f);

	stone = (new BlockStone(1, 1))->setHardness(1.5f)->setResistance(10.0f)->setStepSound(soundStoneFootstep)->setBlockName("stone");
	grass = static_cast<BlockGrass *>((new BlockGrass(2))->setHardness(0.6f)->setStepSound(soundGrassFootstep)->setBlockName("grass"));
	dirt = (new BlockDirt(3, 2))->setHardness(0.5f)->setStepSound(soundGravelFootstep)->setBlockName("dirt");
	cobblestone = (new Block(4, 16, Material::rock))->setHardness(2.0f)->setResistance(10.0f)->setStepSound(soundStoneFootstep)->setBlockName("stonebrick");
	planks = (new BlockWood(5))->setHardness(2.0f)->setResistance(5.0f)->setStepSound(soundWoodFootstep)->setBlockName("wood")->setRequiresSelfNotify();
	sapling = (new BlockSapling(6, 15))->setHardness(0.0f)->setStepSound(soundGrassFootstep)->setBlockName("sapling")->setRequiresSelfNotify();
	bedrock = (new Block(7, 17, Material::rock))->setBlockUnbreakable()->setResistance(6000000.0f)->setStepSound(soundStoneFootstep)->setBlockName("bedrock")->disableStats();
	waterMoving = (new BlockFlowing(8, Material::water))->setHardness(100.0f)->setLightOpacity(3)->setBlockName("water")->disableStats()->setRequiresSelfNotify();
	waterStill = (new BlockStationary(9, Material::water))->setHardness(100.0f)->setLightOpacity(3)->setBlockName("water")->disableStats()->setRequiresSelfNotify();
	lavaMoving = (new BlockFlowing(10, Material::lava))->setHardness(0.0f)->setLightValue(1.0f)->setLightOpacity(255)->setBlockName("lava")->disableStats()->setRequiresSelfNotify();
	lavaStill = (new BlockStationary(11, Material::lava))->setHardness(100.0f)->setLightValue(1.0f)->setLightOpacity(255)->setBlockName("lava")->disableStats()->setRequiresSelfNotify();
	sand = (new BlockSand(12, 18))->setHardness(0.5f)->setStepSound(soundSandFootstep)->setBlockName("sand");
	gravel = (new BlockGravel(13, 19))->setHardness(0.6f)->setStepSound(soundGravelFootstep)->setBlockName("gravel");
	oreGold = (new BlockOre(14, 32))->setHardness(3.0f)->setResistance(5.0f)->setStepSound(soundStoneFootstep)->setBlockName("oreGold");
	oreIron = (new BlockOre(15, 33))->setHardness(3.0f)->setResistance(5.0f)->setStepSound(soundStoneFootstep)->setBlockName("oreIron");
	oreCoal = (new BlockOre(16, 34))->setHardness(3.0f)->setResistance(5.0f)->setStepSound(soundStoneFootstep)->setBlockName("oreCoal");
	wood = (new BlockLog(17))->setHardness(2.0f)->setStepSound(soundWoodFootstep)->setBlockName("log")->setRequiresSelfNotify();
	leaves = static_cast<BlockLeaves *>((new BlockLeaves(18, 52))->setHardness(0.2f)->setLightOpacity(1)->setStepSound(soundGrassFootstep)->setBlockName("leaves")->setRequiresSelfNotify());
	sponge = (new BlockSponge(19))->setHardness(0.6f)->setStepSound(soundGrassFootstep)->setBlockName("sponge");
	glass = (new BlockGlass(20, 49, Material::glass, false))->setHardness(0.3f)->setStepSound(soundGlassFootstep)->setBlockName("glass");
	oreLapis = (new BlockOre(21, 160))->setHardness(3.0f)->setResistance(5.0f)->setStepSound(soundStoneFootstep)->setBlockName("oreLapis");
	blockLapis = (new Block(22, 144, Material::rock))->setHardness(3.0f)->setResistance(5.0f)->setStepSound(soundStoneFootstep)->setBlockName("blockLapis");
	dispenser = (new BlockDispenser(23))->setHardness(3.5f)->setStepSound(soundStoneFootstep)->setBlockName("dispenser")->setRequiresSelfNotify();
	sandStone = (new BlockSandStone(24))->setStepSound(soundStoneFootstep)->setHardness(0.8f)->setBlockName("sandStone")->setRequiresSelfNotify();
	musicBlock = (new BlockNote(25))->setHardness(0.8f)->setBlockName("musicBlock")->setRequiresSelfNotify();
	blockBed = (new BlockBed(26))->setHardness(0.2f)->setBlockName("bed")->disableStats()->setRequiresSelfNotify();
	railPowered = (new BlockRail(27, 179, true))->setHardness(0.7f)->setStepSound(soundMetalFootstep)->setBlockName("goldenRail")->setRequiresSelfNotify();
	railDetector = (new BlockDetectorRail(28, 195))->setHardness(0.7f)->setStepSound(soundMetalFootstep)->setBlockName("detectorRail")->setRequiresSelfNotify();
	pistonStickyBase = (new BlockPistonBase(29, 106, true))->setBlockName("pistonStickyBase")->setRequiresSelfNotify();
	web = (new BlockWeb(30, 11))->setLightOpacity(1)->setHardness(4.0f)->setBlockName("web");
	tallGrass = static_cast<BlockTallGrass *>((new BlockTallGrass(31, 39))->setHardness(0.0f)->setStepSound(soundGrassFootstep)->setBlockName("tallgrass"));
	deadBush = static_cast<BlockDeadBush *>((new BlockDeadBush(32, 55))->setHardness(0.0f)->setStepSound(soundGrassFootstep)->setBlockName("deadbush"));
	pistonBase = (new BlockPistonBase(33, 107, false))->setBlockName("pistonBase")->setRequiresSelfNotify();
	pistonExtension = static_cast<BlockPistonExtension *>((new BlockPistonExtension(34, 107))->setRequiresSelfNotify());
	cloth = (new BlockCloth())->setHardness(0.8f)->setStepSound(soundClothFootstep)->setBlockName("cloth")->setRequiresSelfNotify();
	pistonMoving = new BlockPistonMoving(36);
	plantYellow = static_cast<BlockFlower *>((new BlockFlower(37, 13))->setHardness(0.0f)->setStepSound(soundGrassFootstep)->setBlockName("flower"));
	plantRed = static_cast<BlockFlower *>((new BlockFlower(38, 12))->setHardness(0.0f)->setStepSound(soundGrassFootstep)->setBlockName("rose"));
	mushroomBrown = static_cast<BlockFlower *>((new BlockMushroom(39, 29))->setHardness(0.0f)->setStepSound(soundGrassFootstep)->setLightValue(0.125f)->setBlockName("mushroom"));
	mushroomRed = static_cast<BlockFlower *>((new BlockMushroom(40, 28))->setHardness(0.0f)->setStepSound(soundGrassFootstep)->setBlockName("mushroom"));
	blockGold = (new BlockOreStorage(41, 23))->setHardness(3.0f)->setResistance(10.0f)->setStepSound(soundMetalFootstep)->setBlockName("blockGold");
	blockSteel = (new BlockOreStorage(42, 22))->setHardness(5.0f)->setResistance(10.0f)->setStepSound(soundMetalFootstep)->setBlockName("blockIron");
	stairDouble = (new BlockStep(43, true))->setHardness(2.0f)->setResistance(10.0f)->setStepSound(soundStoneFootstep)->setBlockName("stoneSlab");
	stairSingle = (new BlockStep(44, false))->setHardness(2.0f)->setResistance(10.0f)->setStepSound(soundStoneFootstep)->setBlockName("stoneSlab");
	brick = (new Block(45, 7, Material::rock))->setHardness(2.0f)->setResistance(10.0f)->setStepSound(soundStoneFootstep)->setBlockName("brick");
	tnt = (new BlockTNT(46, 8))->setHardness(0.0f)->setStepSound(soundGrassFootstep)->setBlockName("tnt");
	bookShelf = (new BlockBookshelf(47, 35))->setHardness(1.5f)->setStepSound(soundWoodFootstep)->setBlockName("bookshelf");
	cobblestoneMossy = (new Block(48, 36, Material::rock))->setHardness(2.0f)->setResistance(10.0f)->setStepSound(soundStoneFootstep)->setBlockName("stoneMoss");
	obsidian = (new BlockObsidian(49, 37))->setHardness(50.0f)->setResistance(2000.0f)->setStepSound(soundStoneFootstep)->setBlockName("obsidian");
	torchWood = (new BlockTorch(50, 80))->setHardness(0.0f)->setLightValue(0.9375f)->setStepSound(soundWoodFootstep)->setBlockName("torch")->setRequiresSelfNotify();
	fire = static_cast<BlockFire *>((new BlockFire(51, 31))->setHardness(0.0f)->setLightValue(1.0f)->setStepSound(soundWoodFootstep)->setBlockName("fire")->disableStats());
	mobSpawner = (new BlockMobSpawner(52, 65))->setHardness(5.0f)->setStepSound(soundMetalFootstep)->setBlockName("mobSpawner")->disableStats();
	stairCompactPlanks = (new BlockStairs(53, planks))->setBlockName("stairsWood")->setRequiresSelfNotify();
	chest = (new BlockChest(54))->setHardness(2.5f)->setStepSound(soundWoodFootstep)->setBlockName("chest")->setRequiresSelfNotify();
	redstoneWire = (new BlockRedstoneWire(55, 164))->setHardness(0.0f)->setStepSound(soundPowderFootstep)->setBlockName("redstoneDust")->disableStats()->setRequiresSelfNotify();
	oreDiamond = (new BlockOre(56, 50))->setHardness(3.0f)->setResistance(5.0f)->setStepSound(soundStoneFootstep)->setBlockName("oreDiamond");
	blockDiamond = (new BlockOreStorage(57, 24))->setHardness(5.0f)->setResistance(10.0f)->setStepSound(soundMetalFootstep)->setBlockName("blockDiamond");
	workbench = (new BlockWorkbench(58))->setHardness(2.5f)->setStepSound(soundWoodFootstep)->setBlockName("workbench");
	crops = (new BlockCrops(59, 88))->setHardness(0.0f)->setStepSound(soundGrassFootstep)->setBlockName("crops")->disableStats()->setRequiresSelfNotify();
	tilledField = (new BlockFarmland(60))->setHardness(0.6f)->setStepSound(soundGravelFootstep)->setBlockName("farmland")->setRequiresSelfNotify();
	stoneOvenIdle = (new BlockFurnace(61, false))->setHardness(3.5f)->setStepSound(soundStoneFootstep)->setBlockName("furnace")->setRequiresSelfNotify();
	stoneOvenActive = (new BlockFurnace(62, true))->setHardness(3.5f)->setStepSound(soundStoneFootstep)->setLightValue(0.875f)->setBlockName("furnace")->setRequiresSelfNotify();
	signPost = (new BlockSign(63, createTileEntitySign, true))->setHardness(1.0f)->setStepSound(soundWoodFootstep)->setBlockName("sign")->disableStats()->setRequiresSelfNotify();
	doorWood = (new BlockDoor(64, Material::wood))->setHardness(3.0f)->setStepSound(soundWoodFootstep)->setBlockName("doorWood")->disableStats()->setRequiresSelfNotify();
	ladder = (new BlockLadder(65, 83))->setHardness(0.4f)->setStepSound(soundWoodFootstep)->setBlockName("ladder")->setRequiresSelfNotify();
	rail = (new BlockRail(66, 128, false))->setHardness(0.7f)->setStepSound(soundMetalFootstep)->setBlockName("rail")->setRequiresSelfNotify();
	stairCompactCobblestone = (new BlockStairs(67, cobblestone))->setBlockName("stairsStone")->setRequiresSelfNotify();
	signWall = (new BlockSign(68, createTileEntitySign, false))->setHardness(1.0f)->setStepSound(soundWoodFootstep)->setBlockName("sign")->disableStats()->setRequiresSelfNotify();
	lever = (new BlockLever(69, 96))->setHardness(0.5f)->setStepSound(soundWoodFootstep)->setBlockName("lever")->setRequiresSelfNotify();
	pressurePlateStone = (new BlockPressurePlate(70, stone->blockIndexInTexture, EnumMobType::mobs, Material::rock))->setHardness(0.5f)->setStepSound(soundStoneFootstep)->setBlockName("pressurePlate")->setRequiresSelfNotify();
	doorSteel = (new BlockDoor(71, Material::iron))->setHardness(5.0f)->setStepSound(soundMetalFootstep)->setBlockName("doorIron")->disableStats()->setRequiresSelfNotify();
	pressurePlatePlanks = (new BlockPressurePlate(72, planks->blockIndexInTexture, EnumMobType::everything, Material::wood))->setHardness(0.5f)->setStepSound(soundWoodFootstep)->setBlockName("pressurePlate")->setRequiresSelfNotify();
	oreRedstone = (new BlockRedstoneOre(73, 51, false))->setHardness(3.0f)->setResistance(5.0f)->setStepSound(soundStoneFootstep)->setBlockName("oreRedstone")->setRequiresSelfNotify();
	oreRedstoneGlowing = (new BlockRedstoneOre(74, 51, true))->setLightValue(0.625f)->setHardness(3.0f)->setResistance(5.0f)->setStepSound(soundStoneFootstep)->setBlockName("oreRedstone")->setRequiresSelfNotify();
	torchRedstoneIdle = (new BlockRedstoneTorch(75, 115, false))->setHardness(0.0f)->setStepSound(soundWoodFootstep)->setBlockName("notGate")->setRequiresSelfNotify();
	torchRedstoneActive = (new BlockRedstoneTorch(76, 99, true))->setHardness(0.0f)->setLightValue(0.5f)->setStepSound(soundWoodFootstep)->setBlockName("notGate")->setRequiresSelfNotify();
	button = (new BlockButton(77, stone->blockIndexInTexture))->setHardness(0.5f)->setStepSound(soundStoneFootstep)->setBlockName("button")->setRequiresSelfNotify();
	snow = (new BlockSnow(78, 66))->setHardness(0.1f)->setStepSound(soundClothFootstep)->setBlockName("snow")->setLightOpacity(0);
	ice = (new BlockIce(79, 67))->setHardness(0.5f)->setLightOpacity(3)->setStepSound(soundGlassFootstep)->setBlockName("ice");
	blockSnow = (new BlockSnowBlock(80, 66))->setHardness(0.2f)->setStepSound(soundClothFootstep)->setBlockName("snow");
	cactus = (new BlockCactus(81, 70))->setHardness(0.4f)->setStepSound(soundClothFootstep)->setBlockName("cactus");
	blockClay = (new BlockClay(82, 72))->setHardness(0.6f)->setStepSound(soundGravelFootstep)->setBlockName("clay");
	reed = (new BlockReed(83, 73))->setHardness(0.0f)->setStepSound(soundGrassFootstep)->setBlockName("reeds")->disableStats();
	jukebox = (new BlockJukeBox(84, 74))->setHardness(2.0f)->setResistance(10.0f)->setStepSound(soundStoneFootstep)->setBlockName("jukebox")->setRequiresSelfNotify();
	fence = (new BlockFence(85, 4))->setHardness(2.0f)->setResistance(5.0f)->setStepSound(soundWoodFootstep)->setBlockName("fence");
	pumpkin = (new BlockPumpkin(86, 102, false))->setHardness(1.0f)->setStepSound(soundWoodFootstep)->setBlockName("pumpkin")->setRequiresSelfNotify();
	netherrack = (new BlockNetherrack(87, 103))->setHardness(0.4f)->setStepSound(soundStoneFootstep)->setBlockName("hellrock");
	slowSand = (new BlockSoulSand(88, 104))->setHardness(0.5f)->setStepSound(soundSandFootstep)->setBlockName("hellsand");
	glowStone = (new BlockGlowStone(89, 105, Material::glass))->setHardness(0.3f)->setStepSound(soundGlassFootstep)->setLightValue(1.0f)->setBlockName("lightgem");
	portal = static_cast<BlockPortal *>((new BlockPortal(90, 14))->setHardness(-1.0f)->setStepSound(soundGlassFootstep)->setLightValue(0.75f)->setBlockName("portal"));
	pumpkinLantern = (new BlockPumpkin(91, 102, true))->setHardness(1.0f)->setStepSound(soundWoodFootstep)->setLightValue(1.0f)->setBlockName("litpumpkin")->setRequiresSelfNotify();
	cake = (new BlockCake(92, 121))->setHardness(0.5f)->setStepSound(soundClothFootstep)->setBlockName("cake")->disableStats()->setRequiresSelfNotify();
	redstoneRepeaterIdle = (new BlockRedstoneRepeater(93, false))->setHardness(0.0f)->setStepSound(soundWoodFootstep)->setBlockName("diode")->disableStats()->setRequiresSelfNotify();
	redstoneRepeaterActive = (new BlockRedstoneRepeater(94, true))->setHardness(0.0f)->setLightValue(0.625f)->setStepSound(soundWoodFootstep)->setBlockName("diode")->disableStats()->setRequiresSelfNotify();
	lockedChest = (new BlockLockedChest(95))->setHardness(0.0f)->setLightValue(1.0f)->setStepSound(soundWoodFootstep)->setBlockName("lockedchest")->setTickOnLoad(true)->setRequiresSelfNotify();
	trapdoor = (new BlockTrapDoor(96, Material::wood))->setHardness(3.0f)->setStepSound(soundWoodFootstep)->setBlockName("trapdoor")->disableStats()->setRequiresSelfNotify();
	silverfish = (new BlockSilverfish(97))->setHardness(12.0f / 16.0f);
	stoneBrick = (new BlockStoneBrick(98))->setHardness(1.5f)->setResistance(10.0f)->setStepSound(soundStoneFootstep)->setBlockName("stonebricksmooth");
	mushroomCapBrown = static_cast<BlockMushroomCap *>((new BlockMushroomCap(99, Material::wood, 142, 0))->setHardness(0.2f)->setStepSound(soundWoodFootstep)->setBlockName("mushroom")->setRequiresSelfNotify());
	mushroomCapRed = static_cast<BlockMushroomCap *>((new BlockMushroomCap(100, Material::wood, 142, 1))->setHardness(0.2f)->setStepSound(soundWoodFootstep)->setBlockName("mushroom")->setRequiresSelfNotify());
	fenceIron = (new BlockPane(101, 85, 85, Material::iron, true))->setHardness(5.0f)->setResistance(10.0f)->setStepSound(soundMetalFootstep)->setBlockName("fenceIron");
	thinGlass = (new BlockPane(102, 49, 148, Material::glass, false))->setHardness(0.3f)->setStepSound(soundGlassFootstep)->setBlockName("thinGlass");
	melon = (new BlockMelon(103))->setHardness(1.0f)->setStepSound(soundWoodFootstep)->setBlockName("melon");
	pumpkinStem = (new BlockStem(104, pumpkin))->setHardness(0.0f)->setStepSound(soundWoodFootstep)->setBlockName("pumpkinStem")->setRequiresSelfNotify();
	melonStem = (new BlockStem(105, melon))->setHardness(0.0f)->setStepSound(soundWoodFootstep)->setBlockName("pumpkinStem")->setRequiresSelfNotify();
	vine = static_cast<BlockVine *>((new BlockVine(106))->setHardness(0.2f)->setStepSound(soundGrassFootstep)->setBlockName("vine")->setRequiresSelfNotify());
	fenceGate = (new BlockFenceGate(107, 4))->setHardness(2.0f)->setResistance(5.0f)->setStepSound(soundWoodFootstep)->setBlockName("fenceGate")->setRequiresSelfNotify();
	stairsBrick = (new BlockStairs(108, brick))->setBlockName("stairsBrick")->setRequiresSelfNotify();
	stairsStoneBrickSmooth = (new BlockStairs(109, stoneBrick))->setBlockName("stairsStoneBrickSmooth")->setRequiresSelfNotify();
	mycelium = static_cast<BlockMycelium *>((new BlockMycelium(110))->setHardness(0.6f)->setStepSound(soundGrassFootstep)->setBlockName("mycel"));
	waterlily = static_cast<BlockLilyPad *>((new BlockLilyPad(111, 76))->setHardness(0.0f)->setStepSound(soundGrassFootstep)->setBlockName("waterlily"));
	netherBrick = (new Block(112, 224, Material::rock))->setHardness(2.0f)->setResistance(10.0f)->setStepSound(soundStoneFootstep)->setBlockName("netherBrick");
	netherFence = (new BlockFence(113, 224, Material::rock))->setHardness(2.0f)->setResistance(10.0f)->setStepSound(soundStoneFootstep)->setBlockName("netherFence");
	stairsNetherBrick = (new BlockStairs(114, netherBrick))->setBlockName("stairsNetherBrick")->setRequiresSelfNotify();
	netherStalk = (new BlockNetherStalk(115))->setBlockName("netherStalk")->setRequiresSelfNotify();
	enchantmentTable = (new BlockEnchantmentTable(116))->setHardness(5.0f)->setResistance(2000.0f)->setBlockName("enchantmentTable");
	brewingStand = (new BlockBrewingStand(117))->setHardness(0.5f)->setLightValue(2.0f / 16.0f)->setBlockName("brewingStand")->setRequiresSelfNotify();
	cauldron = (new BlockCauldron(118))->setHardness(2.0f)->setBlockName("cauldron")->setRequiresSelfNotify();
	endPortal = (new BlockEndPortal(119, Material::portal))->setHardness(-1.0f)->setResistance(6000000.0f);
	endPortalFrame = (new BlockEndPortalFrame(120))->setStepSound(soundGlassFootstep)->setLightValue(2.0f / 16.0f)->setHardness(-1.0f)->setBlockName("endPortalFrame")->setRequiresSelfNotify()->setResistance(6000000.0f);
	whiteStone = (new Block(121, 175, Material::rock))->setHardness(3.0f)->setResistance(15.0f)->setStepSound(soundStoneFootstep)->setBlockName("whiteStone");
	dragonEgg = (new BlockDragonEgg(122, 167))->setHardness(3.0f)->setResistance(15.0f)->setStepSound(soundStoneFootstep)->setLightValue(2.0f / 16.0f)->setBlockName("dragonEgg");
	redstoneLampIdle = (new BlockRedstoneLight(123, false))->setHardness(0.3f)->setStepSound(soundGlassFootstep)->setBlockName("redstoneLight");
	redstoneLampActive = (new BlockRedstoneLight(124, true))->setHardness(0.3f)->setStepSound(soundGlassFootstep)->setBlockName("redstoneLight");

	Item::itemsList[cloth->blockID] = (new ItemCloth(cloth->blockID - 256))->setItemName("cloth");
	Item::itemsList[wood->blockID] = (new ItemMetadata(wood->blockID - 256, wood))->setItemName("log");
	Item::itemsList[planks->blockID] = (new ItemMetadata(planks->blockID - 256, planks))->setItemName("wood");
	Item::itemsList[stoneBrick->blockID] = (new ItemMetadata(stoneBrick->blockID - 256, stoneBrick))->setItemName("stonebricksmooth");
	Item::itemsList[sandStone->blockID] = (new ItemMetadata(sandStone->blockID - 256, sandStone))->setItemName("sandStone");
	Item::itemsList[stairSingle->blockID] = (new ItemSlab(stairSingle->blockID - 256))->setItemName("stoneSlab");
	Item::itemsList[sapling->blockID] = (new ItemSapling(sapling->blockID - 256))->setItemName("sapling");
	Item::itemsList[leaves->blockID] = (new ItemLeaves(leaves->blockID - 256))->setItemName("leaves");
	Item::itemsList[vine->blockID] = new ItemColored(vine->blockID - 256, false);
	Item::itemsList[tallGrass->blockID] = (new ItemColored(tallGrass->blockID - 256, true))->setBlockNames({"shrub", "grass", "fern"});
	Item::itemsList[waterlily->blockID] = new ItemLilyPad(waterlily->blockID - 256);
	Item::itemsList[pistonBase->blockID] = new ItemPiston(pistonBase->blockID - 256);
	Item::itemsList[pistonStickyBase->blockID] = new ItemPiston(pistonStickyBase->blockID - 256);
	for (int_t i = 0; i < 256; i++)
	{
		if (blocksList[i] != nullptr && Item::itemsList[i] == nullptr)
		{
			Item::itemsList[i] = new ItemBlock(i - 256);
			blocksList[i]->initializeBlock();
		}
	}

	// The Block base ctor sets opaqueCubeLookup/lightOpacity from the virtual isOpaqueCube(), but in
	// C++ a virtual call from a base ctor dispatches to Block::isOpaqueCube() (always true), never the
	// subclass override (e.g. plants/flowers/torches return false) — same pitfall as Entity::entityInit.
	// Java's in-ctor virtual dispatch gets this right; C++ does not. Recompute here, now that each block
	// is fully constructed so the override resolves correctly. Preserve any explicit setLightOpacity().
	// usesDefaultFaceCullingLookup has the exact same ctor-dispatch pitfall, so it is recomputed here too.
	for (int_t i = 0; i < 256; i++)
	{
		if (blocksList[i] != nullptr)
		{
			opaqueCubeLookup[i] = blocksList[i]->isOpaqueCube();
			usesDefaultFaceCullingLookup[i] = blocksList[i]->usesDefaultFaceCulling();
			if (!lightOpacityExplicit[i])
				lightOpacity[i] = blocksList[i]->isOpaqueCube() ? 255 : 0;

			bool neighborBrightness = blocksList[i]->getRenderType() == 10;
			if (dynamic_cast<BlockStep *>(blocksList[i]) != nullptr ||
				i == tilledField->blockID || canBlockGrass[i])
				neighborBrightness = true;
			useNeighborBrightness[i] = neighborBrightness;
		}
	}

	canBlockGrass[0] = true;
	StatList::initBlockStats(); // func_25154_a (also triggers StatList::initStats internally)
}


void Block::cleanup()
{
	// StepSounds
	delete soundPowderFootstep; soundPowderFootstep = nullptr;
	delete soundWoodFootstep; soundWoodFootstep = nullptr;
	delete soundGravelFootstep; soundGravelFootstep = nullptr;
	delete soundGrassFootstep; soundGrassFootstep = nullptr;
	delete soundStoneFootstep; soundStoneFootstep = nullptr;
	delete soundMetalFootstep; soundMetalFootstep = nullptr;
	delete soundGlassFootstep; soundGlassFootstep = nullptr;
	delete soundClothFootstep; soundClothFootstep = nullptr;
	delete soundSandFootstep; soundSandFootstep = nullptr;

	// Blocks (via blocksList to avoid double-free)
	for (int i = 0; i < 256; i++)
	{
		if (blocksList[i] != nullptr)
		{
			delete blocksList[i];
			blocksList[i] = nullptr;
		}
	}

	// Clear static pointers (they point into blocksList, already freed)
	stone = nullptr;
	grass = nullptr;
	dirt = nullptr;
	cobblestone = nullptr;
	planks = nullptr;
	sapling = nullptr;
	bedrock = nullptr;
	waterMoving = nullptr;
	waterStill = nullptr;
	lavaMoving = nullptr;
	lavaStill = nullptr;
	sand = nullptr;
	gravel = nullptr;
	oreGold = nullptr;
	oreIron = nullptr;
	oreCoal = nullptr;
	wood = nullptr;
	leaves = nullptr;
	sponge = nullptr;
	glass = nullptr;
	oreLapis = nullptr;
	blockLapis = nullptr;
	dispenser = nullptr;
	sandStone = nullptr;
	musicBlock = nullptr;
	blockBed = nullptr;
	railPowered = nullptr;
	railDetector = nullptr;
	pistonStickyBase = nullptr;
	web = nullptr;
	tallGrass = nullptr;
	deadBush = nullptr;
	pistonBase = nullptr;
	pistonExtension = nullptr;
	cloth = nullptr;
	pistonMoving = nullptr;
	plantYellow = nullptr;
	plantRed = nullptr;
	mushroomBrown = nullptr;
	mushroomRed = nullptr;
	blockGold = nullptr;
	blockSteel = nullptr;
	stairDouble = nullptr;
	stairSingle = nullptr;
	brick = nullptr;
	tnt = nullptr;
	bookShelf = nullptr;
	cobblestoneMossy = nullptr;
	obsidian = nullptr;
	torchWood = nullptr;
	fire = nullptr;
	mobSpawner = nullptr;
	stairCompactPlanks = nullptr;
	chest = nullptr;
	redstoneWire = nullptr;
	oreDiamond = nullptr;
	blockDiamond = nullptr;
	workbench = nullptr;
	crops = nullptr;
	tilledField = nullptr;
	stoneOvenIdle = nullptr;
	stoneOvenActive = nullptr;
	signPost = nullptr;
	doorWood = nullptr;
	ladder = nullptr;
	rail = nullptr;
	stairCompactCobblestone = nullptr;
	signWall = nullptr;
	lever = nullptr;
	pressurePlateStone = nullptr;
	doorSteel = nullptr;
	pressurePlatePlanks = nullptr;
	oreRedstone = nullptr;
	oreRedstoneGlowing = nullptr;
	torchRedstoneIdle = nullptr;
	torchRedstoneActive = nullptr;
	button = nullptr;
	snow = nullptr;
	ice = nullptr;
	blockSnow = nullptr;
	cactus = nullptr;
	blockClay = nullptr;
	reed = nullptr;
	jukebox = nullptr;
	fence = nullptr;
	pumpkin = nullptr;
	netherrack = nullptr;
	slowSand = nullptr;
	glowStone = nullptr;
	portal = nullptr;
	pumpkinLantern = nullptr;
	cake = nullptr;
	redstoneRepeaterIdle = nullptr;
	redstoneRepeaterActive = nullptr;
	lockedChest = nullptr;
	trapdoor = nullptr;
	mushroomCapBrown = nullptr;
	mushroomCapRed = nullptr;
	vine = nullptr;
	mycelium = nullptr;
	waterlily = nullptr;
	silverfish = nullptr;
	stoneBrick = nullptr;
	fenceIron = nullptr;
	thinGlass = nullptr;
	melon = nullptr;
	pumpkinStem = nullptr;
	melonStem = nullptr;
	fenceGate = nullptr;
	stairsBrick = nullptr;
	stairsStoneBrickSmooth = nullptr;
	netherBrick = nullptr;
	netherFence = nullptr;
	stairsNetherBrick = nullptr;
	netherStalk = nullptr;
	enchantmentTable = nullptr;
	brewingStand = nullptr;
	cauldron = nullptr;
	endPortal = nullptr;
	endPortalFrame = nullptr;
	whiteStone = nullptr;
	dragonEgg = nullptr;
	redstoneLampIdle = nullptr;
	redstoneLampActive = nullptr;
}

