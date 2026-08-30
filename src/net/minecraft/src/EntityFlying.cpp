#include "EntityFlying.h"

#include "AxisAlignedBB.h"
#include "Block.h"
#include "MathHelper.h"
#include "World.h"

void EntityFlying::moveEntityWithHeading(float f, float f1)
{
	if (isInWater())
	{
		moveFlying(f, f1, 0.02f);
		moveEntity(motionX, motionY, motionZ);
		motionX *= 0.80000001192092896;
		motionY *= 0.80000001192092896;
		motionZ *= 0.80000001192092896;
	}
	else if (handleLavaMovement())
	{
		moveFlying(f, f1, 0.02f);
		moveEntity(motionX, motionY, motionZ);
		motionX *= 0.5;
		motionY *= 0.5;
		motionZ *= 0.5;
	}
	else
	{
		float f2 = 0.91f;
		if (onGround)
		{
			f2 = 546.0f * 0.1f * 0.1f * 0.1f;
			int_t i = worldObj->getBlockId(MathHelper::floor_double(posX), MathHelper::floor_double(boundingBox->minY) - 1, MathHelper::floor_double(posZ));
			if (i > 0)
			{
				f2 = Block::blocksList[i]->slipperiness * 0.91f;
			}
		}
		float f3 = 0.16277136f / (f2 * f2 * f2);
		moveFlying(f, f1, onGround ? 0.1f * f3 : 0.02f);
		f2 = 0.91f;
		if (onGround)
		{
			f2 = 546.0f * 0.1f * 0.1f * 0.1f;
			int_t j = worldObj->getBlockId(MathHelper::floor_double(posX), MathHelper::floor_double(boundingBox->minY) - 1, MathHelper::floor_double(posZ));
			if (j > 0)
			{
				f2 = Block::blocksList[j]->slipperiness * 0.91f;
			}
		}
		moveEntity(motionX, motionY, motionZ);
		motionX *= f2;
		motionY *= f2;
		motionZ *= f2;
	}
	field_705_Q = field_704_R;
	double d  = posX - prevPosX;
	double d1 = posZ - prevPosZ;
	float f4 = MathHelper::sqrt_double(d * d + d1 * d1) * 4.0f;
	if (f4 > 1.0f) { f4 = 1.0f; }
	field_704_R += (f4 - field_704_R) * 0.4f;
	field_703_S += field_704_R;
}
