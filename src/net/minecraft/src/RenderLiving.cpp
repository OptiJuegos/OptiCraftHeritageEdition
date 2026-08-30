#include "RenderLiving.h"
#include "ModelBase.h"
#include "EntityLiving.h"
#include "MathHelper.h"
#include "OpenGlHelper.h"
#include "RenderManager.h"
#include "Tessellator.h"
#include "FontRenderer.h"
#include "RandomMobs.h"
#include "client/Minecraft.h"
#include "platform/RenderAPI.h"



namespace
{
    void resetLivingRenderGLState()
    {
        renderMatrixMode(RenderMatrixMode::Texture);
        renderLoadIdentity();
        renderMatrixMode(RenderMatrixMode::ModelView);

        renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
        renderDepthFunc(RenderCompare::LessEqual);
        renderDepthMask(true);
        renderEnable(RenderCapability::Texture2D);
        renderEnable(RenderCapability::AlphaTest);
        renderDisable(RenderCapability::Blend);
        renderEnable(RenderCapability::CullFace);
        renderDisable(RenderCapability::RescaleNormal);
        renderDisable(RenderCapability::Normalize);
    }
}

RenderLiving::RenderLiving(ModelBase* modelbase, float f) {
    mainModel = modelbase;
    shadowSize = f;
    renderPassModel = nullptr;
}

RenderLiving::~RenderLiving()
{
    delete mainModel;
    mainModel = nullptr;
    renderPassModel = nullptr;
}

void RenderLiving::setRenderPassModel(ModelBase* modelbase) {
    renderPassModel = modelbase;
}

void RenderLiving::doRenderLiving(EntityLiving* entityliving, double d, double d1, double d2, float f, float f1) {
    renderPushMatrix();
    renderDisable(RenderCapability::CullFace);
    mainModel->onGround = getSwingProgress(entityliving, f1);
    if (renderPassModel != nullptr) {
        renderPassModel->onGround = mainModel->onGround;
    }
    mainModel->isRiding = entityliving->isRiding();
    if (renderPassModel != nullptr) {
        renderPassModel->isRiding = mainModel->isRiding;
    }
    mainModel->isChild = entityliving->isChild();
    if (renderPassModel != nullptr) {
        renderPassModel->isChild = mainModel->isChild;
    }

    try {
        auto interpolateRotation = [](float previous, float current, float partialTick) {
            float delta = current - previous;
            while (delta < -180.0f) {
                delta += 360.0f;
            }
            while (delta >= 180.0f) {
                delta -= 360.0f;
            }
            return previous + partialTick * delta;
        };

        float bodyYaw = interpolateRotation(entityliving->prevRenderYawOffset, entityliving->renderYawOffset, f1);
        float headYaw = interpolateRotation(entityliving->prevRotationYawHead, entityliving->rotationYawHead, f1);
        float pitch = entityliving->prevRotationPitch + (entityliving->rotationPitch - entityliving->prevRotationPitch) * f1;
        translateEntity(entityliving, d, d1, d2);
        float age = getTicksExisted(entityliving, f1);
        rotateCorpse(entityliving, age, bodyYaw, f1);
        float scale = 0.0625f;
        renderEnable(RenderCapability::RescaleNormal);
        renderScale(-1.0f, -1.0f, 1.0f);
        preRenderCallback(entityliving, f1);
        renderTranslate(0.0f, -24.0f * scale - 0.0078125f, 0.0f);
        float limbAmount = entityliving->field_705_Q + (entityliving->field_704_R - entityliving->field_705_Q) * f1;
        float limbSwing = entityliving->field_703_S - entityliving->field_704_R * (1.0f - f1);
        if (entityliving->isChild()) {
            limbSwing *= 3.0f;
        }
        if (limbAmount > 1.0f) {
            limbAmount = 1.0f;
        }

        renderEnable(RenderCapability::AlphaTest);
        mainModel->setLivingAnimations(entityliving, limbSwing, limbAmount, f1);
        renderModel(entityliving, limbSwing, limbAmount, age, headYaw - bodyYaw, pitch, scale);

        for (int pass = 0; pass < 4; ++pass) {
            int passResult = shouldRenderPass(entityliving, pass, f1);
            if (passResult > 0 && renderPassModel != nullptr) {
                renderPassModel->setLivingAnimations(entityliving, limbSwing, limbAmount, f1);
                renderPassModel->render(entityliving, limbSwing, limbAmount, age, headYaw - bodyYaw, pitch, scale);

                if (passResult == 15) {
                    float glintTime = static_cast<float>(entityliving->ticksExisted) + f1;
                    loadTexture("%blur%/misc/glint.png");
                    renderEnable(RenderCapability::Blend);
                    renderColor4f(0.5f, 0.5f, 0.5f, 1.0f);
                    renderDepthFunc(RenderCompare::Equal);
                    renderDepthMask(false);

                    for (int glintPass = 0; glintPass < 2; ++glintPass) {
                        renderDisable(RenderCapability::Lighting);
                        const float tint = 0.76f;
                        renderColor4f(0.5f * tint, 0.25f * tint, 0.8f * tint, 1.0f);
                        renderBlendFunc(RenderBlendFactor::SrcColor, RenderBlendFactor::One);
                        renderMatrixMode(RenderMatrixMode::Texture);
                        renderLoadIdentity();
                        float offset = glintTime * (0.001f + static_cast<float>(glintPass) * 0.003f) * 20.0f;
                        const float textureScale = 1.0f / 3.0f;
                        renderScale(textureScale, textureScale, textureScale);
                        renderRotate(30.0f - static_cast<float>(glintPass) * 60.0f, 0.0f, 0.0f, 1.0f);
                        renderTranslate(0.0f, offset, 0.0f);
                        renderMatrixMode(RenderMatrixMode::ModelView);
                        renderPassModel->render(entityliving, limbSwing, limbAmount, age, headYaw - bodyYaw, pitch, scale);
                    }

                    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                    renderMatrixMode(RenderMatrixMode::Texture);
                    renderDepthMask(true);
                    renderLoadIdentity();
                    renderMatrixMode(RenderMatrixMode::ModelView);
                    renderEnable(RenderCapability::Lighting);
                    renderDisable(RenderCapability::Blend);
                    renderDepthFunc(RenderCompare::LessEqual);
                }

                renderDisable(RenderCapability::Blend);
                renderEnable(RenderCapability::AlphaTest);
            }
        }

        renderEquippedItems(entityliving, f1);
        float brightness = entityliving->getEntityBrightness(f1);
        int colorMultiplier = getColorMultiplier(entityliving, brightness, f1);

        OpenGlHelper::setActiveTexture(OpenGlHelper::lightmapTexUnit);
        renderDisable(RenderCapability::Texture2D);
        OpenGlHelper::setActiveTexture(OpenGlHelper::defaultTexUnit);

        if ((colorMultiplier >> 24 & 0xff) > 0 || entityliving->hurtTime > 0 || entityliving->deathTime > 0) {
            renderDisable(RenderCapability::Texture2D);
            renderDisable(RenderCapability::AlphaTest);
            renderEnable(RenderCapability::Blend);
            renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
            renderDepthFunc(RenderCompare::Equal);

            if (entityliving->hurtTime > 0 || entityliving->deathTime > 0) {
                renderColor4f(brightness, 0.0f, 0.0f, 0.4f);
                mainModel->render(entityliving, limbSwing, limbAmount, age, headYaw - bodyYaw, pitch, scale);
                for (int pass = 0; pass < 4; ++pass) {
                    if (inheritRenderPass(entityliving, pass, f1) >= 0 && renderPassModel != nullptr) {
                        renderColor4f(brightness, 0.0f, 0.0f, 0.4f);
                        renderPassModel->render(entityliving, limbSwing, limbAmount, age, headYaw - bodyYaw, pitch, scale);
                    }
                }
            }

            if ((colorMultiplier >> 24 & 0xff) > 0) {
                float red = static_cast<float>(colorMultiplier >> 16 & 0xff) / 255.0f;
                float green = static_cast<float>(colorMultiplier >> 8 & 0xff) / 255.0f;
                float blue = static_cast<float>(colorMultiplier & 0xff) / 255.0f;
                float alpha = static_cast<float>(colorMultiplier >> 24 & 0xff) / 255.0f;
                renderColor4f(red, green, blue, alpha);
                mainModel->render(entityliving, limbSwing, limbAmount, age, headYaw - bodyYaw, pitch, scale);
                for (int pass = 0; pass < 4; ++pass) {
                    if (inheritRenderPass(entityliving, pass, f1) >= 0 && renderPassModel != nullptr) {
                        renderColor4f(red, green, blue, alpha);
                        renderPassModel->render(entityliving, limbSwing, limbAmount, age, headYaw - bodyYaw, pitch, scale);
                    }
                }
            }

            renderDepthFunc(RenderCompare::LessEqual);
            renderDisable(RenderCapability::Blend);
            renderEnable(RenderCapability::AlphaTest);
            renderEnable(RenderCapability::Texture2D);
        }

        renderDisable(RenderCapability::RescaleNormal);
    }
    catch (...) {
        // Keep renderer state recoverable even if one entity renderer fails.
    }

    OpenGlHelper::setActiveTexture(OpenGlHelper::lightmapTexUnit);
    renderEnable(RenderCapability::Texture2D);
    OpenGlHelper::setActiveTexture(OpenGlHelper::defaultTexUnit);
    renderEnable(RenderCapability::CullFace);
    renderPopMatrix();
    resetLivingRenderGLState();
    passSpecialRender(entityliving, d, d1, d2);
    resetLivingRenderGLState();
}

void RenderLiving::translateEntity(EntityLiving* entityliving, double d, double d1, double d2) {
    renderTranslate((float)d, (float)d1, (float)d2);
}

void RenderLiving::rotateCorpse(EntityLiving* entityliving, float f, float f1, float f2) {
    renderRotate(180.0f - f1, 0.0f, 1.0f, 0.0f);
    if (entityliving->deathTime > 0) {
        float f3 = ((((float)entityliving->deathTime + f2) - 1.0f) / 20.0f) * 1.6f;
        f3 = MathHelper::sqrt_float(f3);
        if (f3 > 1.0f) {
            f3 = 1.0f;
        }
        renderRotate(f3 * getDeathMaxRotation(entityliving), 0.0f, 0.0f, 1.0f);
    }
}

float RenderLiving::getSwingProgress(EntityLiving* entityliving, float f) {
    return entityliving->getSwingProgress(f);
}

float RenderLiving::getTicksExisted(EntityLiving* entityliving, float f) {
    return (float)entityliving->ticksExisted + f;
}

void RenderLiving::renderEquippedItems(EntityLiving* entityliving, float f) {
}

void RenderLiving::renderModel(EntityLiving* entityliving, float f, float f1, float f2, float f3, float f4, float f5)
{
    const std::string baseTexture = entityliving->getEntityTexture();
    const std::string renderTexture = RandomMobs::getTexture(entityliving, baseTexture,
        renderManager != nullptr ? renderManager->renderEngine : nullptr);
    loadDownloadableImageTexture(entityliving->skinUrl, renderTexture);
    mainModel->render(entityliving, f, f1, f2, f3, f4, f5);
}

int RenderLiving::inheritRenderPass(EntityLiving* entityliving, int i, float f) {
    return shouldRenderPass(entityliving, i, f);
}

int RenderLiving::shouldRenderPass(EntityLiving* entityliving, int i, float f) {
    return -1;
}

float RenderLiving::getDeathMaxRotation(EntityLiving* entityliving) {
    return 90.0f;
}

int RenderLiving::getColorMultiplier(EntityLiving* entityliving, float f, float f1) {
    return 0;
}

void RenderLiving::preRenderCallback(EntityLiving* entityliving, float f) {
}

void RenderLiving::passSpecialRender(EntityLiving* entityliving, double d, double d1, double d2) {
    if (Minecraft::isDebugInfoEnabled()) {
        renderLivingLabel(entityliving, std::to_string(entityliving->entityId), d, d1, d2, 64);
    }
}

void RenderLiving::renderLivingLabel(EntityLiving* entityliving, std::string s, double d, double d1, double d2, int i) {
    float f = entityliving->getDistanceToEntity(renderManager->livingPlayer);
    if (f > (float)i) {
        return;
    }
    FontRenderer* fontrenderer = getFontRendererFromRenderManager();
    float f1 = 1.6f;
    float f2 = (1.0f / 60.0f) * f1;
    renderPushMatrix();
    renderTranslate((float)d + 0.0f, (float)d1 + 2.3f, (float)d2);
    renderNormal3f(0.0f, 1.0f, 0.0f);
    renderRotate(-renderManager->playerViewY, 0.0f, 1.0f, 0.0f);
    renderRotate(renderManager->playerViewX, 1.0f, 0.0f, 0.0f);
    renderScale(-f2, -f2, f2);
    renderDisable(RenderCapability::Lighting);
    renderDepthMask(false);
    renderDisable(RenderCapability::DepthTest);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    Tessellator* tessellator = &Tessellator::instance;
    int byte0 = 0;
    if (s == "deadmau5") {
        byte0 = -10;
    }
    renderDisable(RenderCapability::Texture2D);
    tessellator->startDrawingQuads();
    int j = fontrenderer->getStringWidth(s) / 2;
    tessellator->setColorRGBA_F(0.0f, 0.0f, 0.0f, 0.25f);
    tessellator->addVertex(-j - 1, -1 + byte0, 0.0f);
    tessellator->addVertex(-j - 1, 8 + byte0, 0.0f);
    tessellator->addVertex(j + 1, 8 + byte0, 0.0f);
    tessellator->addVertex(j + 1, -1 + byte0, 0.0f);
    tessellator->draw();
    renderEnable(RenderCapability::Texture2D);
    fontrenderer->drawString(s, -fontrenderer->getStringWidth(s) / 2, byte0, 0x20ffffff);
    renderEnable(RenderCapability::DepthTest);
    renderDepthMask(true);
    fontrenderer->drawString(s, -fontrenderer->getStringWidth(s) / 2, byte0, -1);
    renderEnable(RenderCapability::Lighting);
    renderDisable(RenderCapability::Blend);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderPopMatrix();
}

void RenderLiving::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    doRenderLiving((EntityLiving*)entity, d, d1, d2, f, f1);
}
