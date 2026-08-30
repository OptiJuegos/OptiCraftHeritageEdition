#include "BlockNote.h"
#include "Material.h"
#include "java/Math.h"
#include "Block.h"
#include "World.h"
#include "TileEntityNote.h"
#include "EntityPlayer.h"
#include "TileEntity.h"
#include <cmath>

BlockNote::BlockNote(int_t i) : BlockContainer(i, 74, Material::wood)
{
}

int_t BlockNote::getBlockTextureFromSide(int_t i) { return blockIndexInTexture; }

void BlockNote::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (l > 0)
	{
		bool flag = world->isBlockGettingPowered(i, j, k);
		TileEntityNote *tileentitynote = dynamic_cast<TileEntityNote *>(world->getBlockTileEntity(i, j, k));
		if (tileentitynote != nullptr && tileentitynote->previousRedstoneState != flag)
		{
			if (flag)
			{
				tileentitynote->triggerNote(world, i, j, k);
			}
			tileentitynote->previousRedstoneState = flag;
		}
	}
}

bool BlockNote::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	if (world->multiplayerWorld) return true;
	TileEntityNote *tileentitynote = dynamic_cast<TileEntityNote *>(world->getBlockTileEntity(i, j, k));
	if (tileentitynote != nullptr)
	{
		tileentitynote->changePitch();
		tileentitynote->triggerNote(world, i, j, k);
	}
	return true;
}

void BlockNote::onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	if (world->multiplayerWorld) return;
	TileEntityNote *tileentitynote = dynamic_cast<TileEntityNote *>(world->getBlockTileEntity(i, j, k));
	if (tileentitynote != nullptr)
		tileentitynote->triggerNote(world, i, j, k);
}

TileEntity *BlockNote::getBlockEntity()
{
	return new TileEntityNote();
}

void BlockNote::playBlock(World *world, int_t i, int_t j, int_t k, int_t l, int_t i1)
{
	float f = (float)JavaMath::pow(2.0, (double)(i1 - 12) / 12.0);
	jstring s = "harp";
	if (l == 1) s = "bd";
	if (l == 2) s = "snare";
	if (l == 3) s = "hat";
	if (l == 4) s = "bassattack";
	world->playSoundEffect((double)i + 0.5, (double)j + 0.5, (double)k + 0.5, jstring("note.") + s, 3.0f, f);
	world->spawnParticle("note", (double)i + 0.5, (double)j + 1.2, (double)k + 0.5, (double)i1 / 24.0, 0.0, 0.0);
}
