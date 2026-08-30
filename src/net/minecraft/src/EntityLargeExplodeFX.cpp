#include "EntityLargeExplodeFX.h"

#include "RenderEngine.h"
#include "RenderHelper.h"
#include "Tessellator.h"
#include "platform/RenderAPI.h"

EntityLargeExplodeFX::EntityLargeExplodeFX(RenderEngine *engine, World *world,
                                           double x, double y, double z,
                                           double strength, double, double)
	: EntityFX(world, x, y, z, 0.0, 0.0, 0.0),
	  age(0), maxAge(6 + rand.nextInt(4)), renderEngine(engine),
	  explosionScale(1.0f - static_cast<float>(strength) * 0.5f)
{
	particleRed = particleGreen = particleBlue = rand.nextFloat() * 0.6f + 0.4f;
}

void EntityLargeExplodeFX::renderParticle(Tessellator *tessellator, float partialTick,
                                          float rotationX, float rotationXZ, float rotationZ,
                                          float rotationYZ, float rotationXY)
{
	const int_t frame = static_cast<int_t>((static_cast<float>(age) + partialTick) * 15.0f /
	                                       static_cast<float>(maxAge));
	if (frame > 15 || renderEngine == nullptr)
		return;

	renderBindTexture(renderEngine->getTexture("/misc/explosion.png"));
	const float u0 = static_cast<float>(frame % 4) / 4.0f;
	const float u1 = u0 + 0.999f / 4.0f;
	const float v0 = static_cast<float>(frame / 4) / 4.0f;
	const float v1 = v0 + 0.999f / 4.0f;
	const float scale = 2.0f * explosionScale;
	const float x = interpolateRenderCoordinate(prevPosX, posX, interpPosX, partialTick);
	const float y = interpolateRenderCoordinate(prevPosY, posY, interpPosY, partialTick);
	const float z = interpolateRenderCoordinate(prevPosZ, posZ, interpPosZ, partialTick);

	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	renderDisable(RenderCapability::Lighting);
	RenderHelper::disableStandardItemLighting();
	tessellator->startDrawingQuads();
	tessellator->setColorRGBA_F(particleRed, particleGreen, particleBlue, 1.0f);
	tessellator->setNormal(0.0f, 1.0f, 0.0f);
	tessellator->setBrightness(240);
	tessellator->addVertexWithUV(x - rotationX * scale - rotationYZ * scale,
	                             y - rotationXZ * scale,
	                             z - rotationZ * scale - rotationXY * scale, u1, v1);
	tessellator->addVertexWithUV(x - rotationX * scale + rotationYZ * scale,
	                             y + rotationXZ * scale,
	                             z - rotationZ * scale + rotationXY * scale, u1, v0);
	tessellator->addVertexWithUV(x + rotationX * scale + rotationYZ * scale,
	                             y + rotationXZ * scale,
	                             z + rotationZ * scale + rotationXY * scale, u0, v0);
	tessellator->addVertexWithUV(x + rotationX * scale - rotationYZ * scale,
	                             y - rotationXZ * scale,
	                             z + rotationZ * scale - rotationXY * scale, u0, v1);
	tessellator->draw();
	renderPolygonOffset(0.0f, 0.0f);
	renderEnable(RenderCapability::Lighting);
}

int_t EntityLargeExplodeFX::getBrightnessForRender(float)
{
	return 0xf0f0;
}

void EntityLargeExplodeFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	++age;
	if (age == maxAge)
		setEntityDead();
}

int_t EntityLargeExplodeFX::getFXLayer()
{
	return 3;
}
