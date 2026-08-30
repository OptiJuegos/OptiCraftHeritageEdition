#include "RenderEnchantmentTable.h"

#include "MathHelper.h"
#include "ModelBook.h"
#include "TileEntityEnchantmentTable.h"
#include "platform/RenderAPI.h"

RenderEnchantmentTable::RenderEnchantmentTable()
	: bookModel(new ModelBook())
{
}

RenderEnchantmentTable::~RenderEnchantmentTable()
{
	delete bookModel;
}

void RenderEnchantmentTable::renderEnchantmentTableAt(TileEntityEnchantmentTable *table, double x, double y, double z, float partialTick)
{
	if (table == nullptr)
		return;

	renderPushMatrix();
	renderTranslate((float)x + 0.5f, (float)y + 12.0f / 16.0f, (float)z + 0.5f);
	float time = (float)table->tickCount + partialTick;
	renderTranslate(0.0f, 0.1f + MathHelper::sin(time * 0.1f) * 0.01f, 0.0f);

	float rotationDelta = table->bookRotation2 - table->bookRotationPrev;
	while (rotationDelta >= 3.14159265358979323846f)
		rotationDelta -= 6.28318530717958647692f;
	while (rotationDelta < -3.14159265358979323846f)
		rotationDelta += 6.28318530717958647692f;

	float rotation = table->bookRotationPrev + rotationDelta * partialTick;
	renderRotate(-rotation * 180.0f / 3.14159265358979323846f, 0.0f, 1.0f, 0.0f);
	renderRotate(80.0f, 0.0f, 0.0f, 1.0f);
	bindTextureByName("/item/book.png");

	float flipRight = table->pageFlipPrev + (table->pageFlip - table->pageFlipPrev) * partialTick + 0.25f;
	float flipLeft = table->pageFlipPrev + (table->pageFlip - table->pageFlipPrev) * partialTick + 12.0f / 16.0f;
	flipRight = (flipRight - (float)MathHelper::floor_float(flipRight)) * 1.6f - 0.3f;
	flipLeft = (flipLeft - (float)MathHelper::floor_float(flipLeft)) * 1.6f - 0.3f;
	if (flipRight < 0.0f) flipRight = 0.0f;
	if (flipRight > 1.0f) flipRight = 1.0f;
	if (flipLeft < 0.0f) flipLeft = 0.0f;
	if (flipLeft > 1.0f) flipLeft = 1.0f;
	float spread = table->bookSpreadPrev + (table->bookSpread - table->bookSpreadPrev) * partialTick;
	bookModel->render(time, flipRight, flipLeft, spread, 0.0f, 1.0f / 16.0f);
	renderPopMatrix();
}

void RenderEnchantmentTable::renderTileEntityAt(TileEntity *tileentity, double x, double y, double z, float partialTick)
{
	renderEnchantmentTableAt(dynamic_cast<TileEntityEnchantmentTable *>(tileentity), x, y, z, partialTick);
}
