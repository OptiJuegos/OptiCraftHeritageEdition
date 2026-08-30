#include "TileEntityChestRenderer.h"

#include <cmath>

#include "BlockChest.h"
#include "ModelChest.h"
#include "ModelRenderer.h"
#include "ModelLargeChest.h"
#include "TileEntityChest.h"
#include "platform/RenderAPI.h"

TileEntityChestRenderer::TileEntityChestRenderer()
	: chestModel(new ModelChest()), largeChestModel(new ModelLargeChest())
{
}

TileEntityChestRenderer::~TileEntityChestRenderer()
{
	delete chestModel;
	delete largeChestModel;
}

void TileEntityChestRenderer::renderTileEntityChestAt(TileEntityChest *chest, double x, double y, double z, float partialTick)
{
	if (chest == nullptr)
		return;

	int_t metadata = 0;
	if (chest->worldObj != nullptr)
	{
		Block *block = chest->getBlockType();
		metadata = chest->getBlockMetadata();
		if (block != nullptr && metadata == 0)
		{
			BlockChest *blockChest = dynamic_cast<BlockChest *>(block);
			if (blockChest != nullptr)
				blockChest->unifyAdjacentChests(chest->worldObj, chest->xCoord, chest->yCoord, chest->zCoord);
			metadata = chest->getBlockMetadata();
		}
		chest->checkForAdjacentChests();
	}

	if (chest->adjacentChestZNeg != nullptr || chest->adjacentChestXNeg != nullptr)
		return;

	ModelChest *model;
	if (chest->adjacentChestXPos == nullptr && chest->adjacentChestZPos == nullptr)
	{
		model = chestModel;
		bindTextureByName("/item/chest.png");
	}
	else
	{
		model = largeChestModel;
		bindTextureByName("/item/largechest.png");
	}

	renderPushMatrix();
	renderEnable(RenderCapability::RescaleNormal);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderTranslate((float)x, (float)y + 1.0f, (float)z + 1.0f);
	renderScale(1.0f, -1.0f, -1.0f);
	renderTranslate(0.5f, 0.5f, 0.5f);

	float rotation = 0.0f;
	if (metadata == 2)
		rotation = 180.0f;
	if (metadata == 4)
		rotation = 90.0f;
	if (metadata == 5)
		rotation = -90.0f;

	if (metadata == 2 && chest->adjacentChestXPos != nullptr)
		renderTranslate(1.0f, 0.0f, 0.0f);
	if (metadata == 5 && chest->adjacentChestZPos != nullptr)
		renderTranslate(0.0f, 0.0f, -1.0f);

	renderRotate(rotation, 0.0f, 1.0f, 0.0f);
	renderTranslate(-0.5f, -0.5f, -0.5f);

	float lid = chest->prevLidAngle + (chest->lidAngle - chest->prevLidAngle) * partialTick;
	if (chest->adjacentChestZNeg != nullptr)
	{
		float adjacent = chest->adjacentChestZNeg->prevLidAngle +
		                 (chest->adjacentChestZNeg->lidAngle - chest->adjacentChestZNeg->prevLidAngle) * partialTick;
		if (adjacent > lid)
			lid = adjacent;
	}
	if (chest->adjacentChestXNeg != nullptr)
	{
		float adjacent = chest->adjacentChestXNeg->prevLidAngle +
		                 (chest->adjacentChestXNeg->lidAngle - chest->adjacentChestXNeg->prevLidAngle) * partialTick;
		if (adjacent > lid)
			lid = adjacent;
	}

	lid = 1.0f - lid;
	lid = 1.0f - lid * lid * lid;
	model->chestLid->rotateAngleX = -(lid * 3.14159265358979323846f / 2.0f);
	model->renderAll();

	renderDisable(RenderCapability::RescaleNormal);
	renderPopMatrix();
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void TileEntityChestRenderer::renderTileEntityAt(TileEntity *tileentity, double x, double y, double z, float partialTick)
{
	renderTileEntityChestAt(dynamic_cast<TileEntityChest *>(tileentity), x, y, z, partialTick);
}
