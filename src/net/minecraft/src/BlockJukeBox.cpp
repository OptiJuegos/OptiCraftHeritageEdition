#include "BlockJukeBox.h"
#include "Material.h"
#include "World.h"
#include "TileEntityRecordPlayer.h"
#include "EntityItem.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "TileEntity.h"

BlockJukeBox::BlockJukeBox(int_t i, int_t j) : BlockContainer(i, j, Material::wood)
{
}

int_t BlockJukeBox::getBlockTextureFromSide(int_t i)
{
	return blockIndexInTexture + (i != 1 ? 0 : 1);
}

bool BlockJukeBox::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	if (world->getBlockMetadata(i, j, k) == 0) return false;
	ejectRecord(world, i, j, k);
	return true;
}

void BlockJukeBox::insertRecord(World *world, int_t i, int_t j, int_t k, int_t recordId)
{
	if (world->multiplayerWorld)
		return;
	TileEntityRecordPlayer *tileentityrecordplayer = dynamic_cast<TileEntityRecordPlayer *>(world->getBlockTileEntity(i, j, k));
	if (tileentityrecordplayer == nullptr)
		return;
	tileentityrecordplayer->record = recordId;
	tileentityrecordplayer->onInventoryChanged();
	world->setBlockMetadataWithNotify(i, j, k, 1);
}

void BlockJukeBox::ejectRecord(World *world, int_t i, int_t j, int_t k)
{
	if (world->multiplayerWorld)
		return;
	TileEntityRecordPlayer *tileentityrecordplayer = dynamic_cast<TileEntityRecordPlayer *>(world->getBlockTileEntity(i, j, k));
	if (tileentityrecordplayer == nullptr)
		return;

	int_t recordId = tileentityrecordplayer->record;
	if (recordId == 0)
		return;

	world->playAuxSFX(1005, i, j, k, 0);
	world->playRecord(jstring(), i, j, k);
	tileentityrecordplayer->record = 0;
	tileentityrecordplayer->onInventoryChanged();
	world->setBlockMetadataWithNotify(i, j, k, 0);

	float_t spread = 0.7f;
	double x = static_cast<double>(world->rand.nextFloat() * spread) + static_cast<double>(1.0f - spread) * 0.5;
	double y = static_cast<double>(world->rand.nextFloat() * spread) + static_cast<double>(1.0f - spread) * 0.2 + 0.6;
	double z = static_cast<double>(world->rand.nextFloat() * spread) + static_cast<double>(1.0f - spread) * 0.5;
	EntityItem *entityitem = new EntityItem(world, static_cast<double>(i) + x, static_cast<double>(j) + y,
	                                        static_cast<double>(k) + z, new ItemStack(recordId, 1, 0));
	entityitem->delayBeforeCanPickup = 10;
	if (!world->spawnEntityInWorld(entityitem))
		delete entityitem;
}

void BlockJukeBox::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
	ejectRecord(world, i, j, k);
	BlockContainer::onBlockRemoval(world, i, j, k);
}

void BlockJukeBox::dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f, int_t)
{
	if (world->multiplayerWorld)
		return;
	BlockContainer::dropBlockAsItemWithChance(world, i, j, k, l, f, 0);
}

TileEntity *BlockJukeBox::getBlockEntity()
{
	return new TileEntityRecordPlayer();
}
