#include "TileEntityEnchantmentTable.h"

#include "EntityPlayer.h"
#include "java/Math.h"
#include "World.h"
#include "platform/PlatformTuning.h"

#include <cmath>

Random TileEntityEnchantmentTable::rand;

TileEntityEnchantmentTable::TileEntityEnchantmentTable()
	: tickCount(0), pageFlip(0.0f), pageFlipPrev(0.0f), field_40061_d(0.0f), field_40062_e(0.0f),
	  bookSpread(0.0f), bookSpreadPrev(0.0f), bookRotation2(0.0f), bookRotationPrev(0.0f), bookRotation(0.0f)
{
}

void TileEntityEnchantmentTable::updateEntity()
{
	TileEntity::updateEntity();
	bookSpreadPrev = bookSpread;
	bookRotationPrev = bookRotation2;

	const double centerX = (double)((float)xCoord + 0.5f);
	const double centerY = (double)((float)yCoord + 0.5f);
	const double centerZ = (double)((float)zCoord + 0.5f);
	EntityPlayer *player = worldObj != nullptr ? worldObj->getClosestPlayer(centerX, centerY, centerZ, 3.0) : nullptr;
	if (player != nullptr)
	{
		double dx = player->posX - centerX;
		double dz = player->posZ - centerZ;
#if PLATFORM_FLOAT_VERTEX_MATH
		bookRotation = std::atan2(static_cast<float>(dz), static_cast<float>(dx));
#else
		bookRotation = (float)JavaMath::atan2(dz, dx);
#endif
		bookSpread += 0.1f;
		if (bookSpread < 0.5f || rand.nextInt(40) == 0)
		{
			float previousTarget = field_40061_d;
			do
			{
				field_40061_d += (float)(rand.nextIntDifference(4));
			}
			while (previousTarget == field_40061_d);
		}
	}
	else
	{
		bookRotation += 0.02f;
		bookSpread -= 0.1f;
	}

	constexpr float pi = 3.14159265358979323846f;
	while (bookRotation2 >= pi)
		bookRotation2 -= pi * 2.0f;
	while (bookRotation2 < -pi)
		bookRotation2 += pi * 2.0f;
	while (bookRotation >= pi)
		bookRotation -= pi * 2.0f;
	while (bookRotation < -pi)
		bookRotation += pi * 2.0f;

	float rotationDelta;
	for (rotationDelta = bookRotation - bookRotation2; rotationDelta >= pi; rotationDelta -= pi * 2.0f)
	{
	}
	while (rotationDelta < -pi)
		rotationDelta += pi * 2.0f;
	bookRotation2 += rotationDelta * 0.4f;

	if (bookSpread < 0.0f)
		bookSpread = 0.0f;
	if (bookSpread > 1.0f)
		bookSpread = 1.0f;

	++tickCount;
	pageFlipPrev = pageFlip;
	float pageDelta = (field_40061_d - pageFlip) * 0.4f;
	constexpr float maxPageDelta = 0.2f;
	if (pageDelta < -maxPageDelta)
		pageDelta = -maxPageDelta;
	if (pageDelta > maxPageDelta)
		pageDelta = maxPageDelta;
	field_40062_e += (pageDelta - field_40062_e) * 0.9f;
	pageFlip += field_40062_e;
}
