#include "EntityPickupFX.h"

#include "platform/RenderAPI.h"

#include "Entity.h"
#include "EntityItem.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "OpenGlHelper.h"
#include "RenderManager.h"
#include "Tessellator.h"
#include "World.h"

EntityPickupFX::EntityPickupFX(World *world, Entity *entity, Entity *entity1, float f)
	: EntityFX(world, entity->posX, entity->posY, entity->posZ, entity->motionX, entity->motionY, entity->motionZ)
{
	field_678_p = 0;
	field_677_q = 0;
	field_677_q = 3;
	field_676_r = f;
	if (entity1 != nullptr)
	{
		collectorLastPosX = entity1->lastTickPosX;
		collectorLastPosY = entity1->lastTickPosY;
		collectorLastPosZ = entity1->lastTickPosZ;
		collectorPosX = entity1->posX;
		collectorPosY = entity1->posY;
		collectorPosZ = entity1->posZ;
	}
	else
	{
		collectorLastPosX = collectorPosX = entity->posX;
		collectorLastPosY = collectorPosY = entity->posY;
		collectorLastPosZ = collectorPosZ = entity->posZ;
	}

	// The picked-up entity is deleted by the world right after the pickup (in Java the GC
	// kept it alive while this effect still referenced it for ~3 ticks). Holding the raw
	// pointer here dereferenced freed memory -> crash in RenderManager::getEntityRenderObject
	// at typeid(*entity). Render a private copy we own instead, fully decoupled from the world.
	EntityItem *srcItem = (entity->getEntityClassID() == EntityItem::CLASS_ID) ? static_cast<EntityItem*>(entity) : nullptr;
	if (srcItem != nullptr && srcItem->item != nullptr)
	{
		EntityItem *copy = new EntityItem(world, entity->posX, entity->posY, entity->posZ, srcItem->item->copy());
		copy->rotationYaw = entity->rotationYaw;
		field_675_a = copy;
		ownsRenderEntity = true;
	}
	else
	{
		// Packet22 normally targets EntityItem. Do not retain an unsupported
		// world-owned entity after the world removes it from the collect packet.
		field_675_a = nullptr;
		ownsRenderEntity = false;
	}
}

EntityPickupFX::~EntityPickupFX()
{
	if (ownsRenderEntity)
		delete field_675_a;
}

void EntityPickupFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	if (field_675_a == nullptr)
		return;

	float f6 = ((float)field_678_p + f) / (float)field_677_q;
	f6 *= f6;
	double d  = field_675_a->posX;
	double d1 = field_675_a->posY;
	double d2 = field_675_a->posZ;
	double d3 = collectorLastPosX + (collectorPosX - collectorLastPosX) * (double)f;
	double d4 = collectorLastPosY + (collectorPosY - collectorLastPosY) * (double)f + (double)field_676_r;
	double d5 = collectorLastPosZ + (collectorPosZ - collectorLastPosZ) * (double)f;
	double d6 = d  + (d3 - d ) * (double)f6;
	double d7 = d1 + (d4 - d1) * (double)f6;
	double d8 = d2 + (d5 - d2) * (double)f6;
	MathHelper::floor_double(d6);
	MathHelper::floor_double(d7 + (double)(yOffset / 2.0f));
	MathHelper::floor_double(d8);
	const int_t brightness = getBrightnessForRender(f);
	const int_t lightU = brightness % 65536;
	const int_t lightV = brightness / 65536;
	OpenGlHelper::setLightmapTextureCoords(OpenGlHelper::lightmapTexUnit, (float)lightU, (float)lightV);
	const float renderX = static_cast<float>(d6 - interpPosX);
	const float renderY = static_cast<float>(d7 - interpPosY);
	const float renderZ = static_cast<float>(d8 - interpPosZ);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	RenderManager::instance->renderEntityWithPosYaw(field_675_a, renderX, renderY, renderZ, field_675_a->rotationYaw, f);
}

void EntityPickupFX::onUpdate()
{
	field_678_p++;
	if (field_678_p == field_677_q)
	{
		setEntityDead();
	}
}

int_t EntityPickupFX::getFXLayer()
{
	return 3;
}
