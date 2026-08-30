#include "EntityNoteFX.h"

#include "MathHelper.h"
#include "Tessellator.h"
#include "World.h"

EntityNoteFX::EntityNoteFX(World *world, double d, double d1, double d2,
                           double d3, double d4, double d5)
	: EntityNoteFX(world, d, d1, d2, d3, d4, d5, 2.0f)
{
}

EntityNoteFX::EntityNoteFX(World *world, double d, double d1, double d2,
                           double d3, double d4, double d5, float f)
	: EntityFX(world, d, d1, d2, 0.0, 0.0, 0.0)
{
	motionX *= 0.0099999997764825821;
	motionY *= 0.0099999997764825821;
	motionZ *= 0.0099999997764825821;
	motionY += 0.2;
	particleRed   = MathHelper::sin(((float)d3 + 0.0f)        * 3.1415927f * 2.0f) * 0.65f + 0.35f;
	particleGreen = MathHelper::sin(((float)d3 + 1.0f / 3.0f)  * 3.1415927f * 2.0f) * 0.65f + 0.35f;
	particleBlue  = MathHelper::sin(((float)d3 + 0.6666667f)  * 3.1415927f * 2.0f) * 0.65f + 0.35f;
	particleScale *= 0.75f;
	particleScale *= f;
	field_21065_a = particleScale;
	particleMaxAge = 6;
	noClip = false;
	particleTextureIndex = 64;
}

void EntityNoteFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	float f6 = (((float)particleAge + f) / (float)particleMaxAge) * 32.0f;
	if (f6 < 0.0f) { f6 = 0.0f; }
	if (f6 > 1.0f) { f6 = 1.0f; }
	particleScale = field_21065_a * f6;
	EntityFX::renderParticle(tessellator, f, f1, f2, f3, f4, f5);
}

void EntityNoteFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	if (particleAge++ >= particleMaxAge)
	{
		setEntityDead();
	}
	moveEntity(motionX, motionY, motionZ);
	if (posY == prevPosY)
	{
		motionX *= 1.1;
		motionZ *= 1.1;
	}
	motionX *= 0.6600000262260437;
	motionY *= 0.6600000262260437;
	motionZ *= 0.6600000262260437;
	if (onGround)
	{
		motionX *= 0.69999998807907104;
		motionZ *= 0.69999998807907104;
	}
}
