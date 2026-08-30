#include "EffectRenderer.h"
#include "ActiveRenderInfo.h"

#include <algorithm>
#include <sstream>

#include "Entity.h"
#include "RenderHelper.h"
#include "EntityFX.h"
#include "EntityDiggingFX.h"
#include "Block.h"
#include "MathHelper.h"
#include "RenderEngine.h"
#include "Tessellator.h"
#include "World.h"
#include "platform/RenderAPI.h"

EffectRenderer::EffectRenderer(World *world, RenderEngine *renderengine)
    : worldObj(nullptr), renderer(renderengine), rand()
{
    if (world != nullptr)
        worldObj = world;

    for (int i = 0; i < 4; i++)
        fxLayers[i].clear();
}

EffectRenderer::~EffectRenderer() = default;

void EffectRenderer::addEffect(EntityFX *entityfx)
{
	if (entityfx == nullptr)
		return;
    int_t i = entityfx->getFXLayer();
    if ((int_t)fxLayers[i].size() >= 4000)
        fxLayers[i].erase(fxLayers[i].begin());
	fxLayers[i].emplace_back(entityfx);
}

void EffectRenderer::updateEffects()
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < (int)fxLayers[i].size(); j++)
        {
			EntityFX *entityfx = fxLayers[i][j].get();
            entityfx->onUpdate();
            if (entityfx->isDead)
            {
                fxLayers[i].erase(fxLayers[i].begin() + j);
                j--;
            }
        }
    }
}

void EffectRenderer::renderParticles(Entity *entity, float_t f)
{
    const float_t f1 = ActiveRenderInfo::rotationX;
    const float_t f2 = ActiveRenderInfo::rotationZ;
    const float_t f3 = ActiveRenderInfo::rotationYZ;
    const float_t f4 = ActiveRenderInfo::rotationXY;
    const float_t f5 = ActiveRenderInfo::rotationXZ;

    EntityFX::interpPosX = entity->lastTickPosX + (entity->posX - entity->lastTickPosX) * (double)f;
    EntityFX::interpPosY = entity->lastTickPosY + (entity->posY - entity->lastTickPosY) * (double)f;
    EntityFX::interpPosZ = entity->lastTickPosZ + (entity->posZ - entity->lastTickPosZ) * (double)f;

    for (int i = 0; i < 3; i++)
    {
        if (fxLayers[i].empty())
            continue;

        int_t j = 0;
        if (i == 0) j = renderer->getTexture("/particles.png");
        if (i == 1) j = renderer->getTexture("/terrain.png");
        if (i == 2) j = renderer->getTexture("/gui/items.png");

        renderBindTexture(j);

        Tessellator *tessellator = &Tessellator::instance;
        renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        tessellator->startDrawingQuads();

        for (int k = 0; k < (int)fxLayers[i].size(); k++)
        {
			EntityFX *entityfx = fxLayers[i][k].get();
            tessellator->setBrightness(entityfx->getBrightnessForRender(f));
            entityfx->renderParticle(tessellator, f, f1, f5, f2, f3, f4);
        }

        tessellator->draw();
    }
}

void EffectRenderer::renderLitParticles(Entity *entity, float_t f)
{
    const float_t f1 = MathHelper::cos(entity->rotationYaw * (3.1415927f / 180.0f));
    const float_t f2 = MathHelper::sin(entity->rotationYaw * (3.1415927f / 180.0f));
    const float_t f3 = -f2 * MathHelper::sin(entity->rotationPitch * (3.1415927f / 180.0f));
    const float_t f4 = f1 * MathHelper::sin(entity->rotationPitch * (3.1415927f / 180.0f));
    const float_t f5 = MathHelper::cos(entity->rotationPitch * (3.1415927f / 180.0f));
    const int_t layer = 3;
    if (fxLayers[layer].empty())
        return;

    Tessellator *tessellator = &Tessellator::instance;
    for (int i = 0; i < (int)fxLayers[layer].size(); i++)
    {
		EntityFX *entityfx = fxLayers[layer][i].get();
        tessellator->setBrightness(entityfx->getBrightnessForRender(f));
        entityfx->renderParticle(tessellator, f, f1, f5, f2, f3, f4);
    }
}

void EffectRenderer::clearEffects(World *world)
{
    worldObj = world;
    for (int i = 0; i < 4; i++)
        fxLayers[i].clear();
}

void EffectRenderer::addBlockDestroyEffects(int_t i, int_t j, int_t k, int_t l, int_t i1)
{
    if (l == 0)
        return;

    Block *block = Block::blocksList[l];
    int_t j1 = 4;

    for (int k1 = 0; k1 < j1; k1++)
    {
        for (int l1 = 0; l1 < j1; l1++)
        {
            for (int i2 = 0; i2 < j1; i2++)
            {
                const float localX = (static_cast<float>(k1) + 0.5f) / static_cast<float>(j1);
                const float localY = (static_cast<float>(l1) + 0.5f) / static_cast<float>(j1);
                const float localZ = (static_cast<float>(i2) + 0.5f) / static_cast<float>(j1);
                const double d = static_cast<double>(i) + static_cast<double>(localX);
                const double d1 = static_cast<double>(j) + static_cast<double>(localY);
                const double d2 = static_cast<double>(k) + static_cast<double>(localZ);
                int_t j2 = rand.nextInt(6);
                addEffect(
                    (new EntityDiggingFX(worldObj, d, d1, d2,
                        static_cast<double>(localX - 0.5f),
                        static_cast<double>(localY - 0.5f),
                        static_cast<double>(localZ - 0.5f),
                        block, j2, i1))->applyColourMultiplier(i, j, k)
                );
            }
        }
    }
}

void EffectRenderer::addBlockHitEffects(int_t i, int_t j, int_t k, int_t l)
{
    int_t i1 = worldObj->getBlockId(i, j, k);
    if (i1 == 0)
        return;

    Block *block = Block::blocksList[i1];
    float_t f = 0.1f;

    double d  = (double)i + rand.nextDouble() * (block->maxX - block->minX - (double)(f * 2.0f)) + (double)f + block->minX;
    double d1 = (double)j + rand.nextDouble() * (block->maxY - block->minY - (double)(f * 2.0f)) + (double)f + block->minY;
    double d2 = (double)k + rand.nextDouble() * (block->maxZ - block->minZ - (double)(f * 2.0f)) + (double)f + block->minZ;

    if (l == 0) d1 = ((double)j + block->minY) - (double)f;
    if (l == 1) d1 =  (double)j + block->maxY  + (double)f;
    if (l == 2) d2 = ((double)k + block->minZ) - (double)f;
    if (l == 3) d2 =  (double)k + block->maxZ  + (double)f;
    if (l == 4) d  = ((double)i + block->minX) - (double)f;
    if (l == 5) d  =  (double)i + block->maxX  + (double)f;

    addEffect(
        (new EntityDiggingFX(worldObj, d, d1, d2, 0.0, 0.0, 0.0,
            block, l, worldObj->getBlockMetadata(i, j, k)))
            ->applyColourMultiplier(i, j, k)->multiplyVelocity(0.2f)->multiplyParticleScaleBy(0.6f)
    );
}

void EffectRenderer::renderBlockBreakParticles(Entity *entity, float f)
{
	if (fxLayers[1].empty())
		return;

	float f1 = MathHelper::cos((entity->rotationYaw * 3.1415927f) / 180.0f);
	float f2 = MathHelper::sin((entity->rotationYaw * 3.1415927f) / 180.0f);
	float f3 = -f2 * MathHelper::sin((entity->rotationPitch * 3.1415927f) / 180.0f);
	float f4 =  f1 * MathHelper::sin((entity->rotationPitch * 3.1415927f) / 180.0f);
	float f5 = MathHelper::cos((entity->rotationPitch * 3.1415927f) / 180.0f);

	RenderHelper::enableStandardItemLighting();
	renderBindTexture(renderer->getTexture("/terrain.png"));

	Tessellator *tessellator = &Tessellator::instance;
	tessellator->startDrawingQuads();

	for (int i = 0; i < (int)fxLayers[1].size(); i++)
		fxLayers[1][i]->renderParticle(tessellator, f, f1, f5, f2, f3, f4);

	tessellator->draw();
	RenderHelper::disableStandardItemLighting();
}

std::string EffectRenderer::getStatistics()
{
    std::ostringstream oss;
    oss << (fxLayers[0].size() + fxLayers[1].size() + fxLayers[2].size());
    return oss.str();
}
