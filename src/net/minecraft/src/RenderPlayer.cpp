#include "RenderPlayer.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "ModelBiped.h"
#include "ItemStack.h"
#include "Item.h"
#include "ItemArmor.h"
#include "Block.h"
#include "RenderBlocks.h"
#include "RenderManager.h"
#include "ItemRenderer.h"
#include "EntityLiving.h"
#include "Entity.h"
#include "EnumAction.h"
#include "FontRenderer.h"
#include "Tessellator.h"
#include "client/Minecraft.h"
#include "platform/RenderAPI.h"


#include "InventoryPlayer.h"
#include "ModelRenderer.h"
#include "MathHelper.h"

RenderPlayer::RenderPlayer() : RenderLiving(new ModelBiped(0.0f), 0.5f) {
    modelBipedMain = (ModelBiped*)mainModel;
    modelArmorChestplate = new ModelBiped(1.0f);
    modelArmor = new ModelBiped(0.5f);
    field_208_f = 0;
}

RenderPlayer::~RenderPlayer()
{
    delete modelArmorChestplate;
    delete modelArmor;
    modelArmorChestplate = nullptr;
    modelArmor = nullptr;
    renderPassModel = nullptr;
}

int RenderPlayer::setArmorModel(EntityPlayer* entityPlayer, int i, float)
{
    if (entityPlayer == nullptr || entityPlayer->inventory == nullptr || i < 0 || i > 3)
        return -1;
    ItemStack* stack = entityPlayer->inventory->armorItemInSlot(3 - i);
    if (stack == nullptr)
        return -1;
    ItemArmor* armor = dynamic_cast<ItemArmor*>(stack->getItem());
    if (armor == nullptr)
        return -1;

    static const char* const armorFilenamePrefix[] = {"cloth", "chain", "iron", "diamond", "gold"};
    if (armor->renderIndex < 0 || armor->renderIndex >= 5)
        return -1;
    std::string texture = std::string("/armor/") + armorFilenamePrefix[armor->renderIndex] + "_" + (i != 2 ? "1" : "2") + ".png";
    loadTexture(texture);

    ModelBiped* model = i != 2 ? modelArmorChestplate : modelArmor;
    model->bipedHead->showModel = i == 0;
    model->bipedHeadwear->showModel = i == 0;
    model->bipedBody->showModel = i == 1 || i == 2;
    model->bipedRightArm->showModel = i == 1;
    model->bipedLeftArm->showModel = i == 1;
    model->bipedRightLeg->showModel = i == 2 || i == 3;
    model->bipedLeftLeg->showModel = i == 2 || i == 3;
    setRenderPassModel(model);
    return stack->isItemEnchanted() ? 15 : 1;
}

int RenderPlayer::shouldRenderPass(EntityLiving* entityLiving, int i, float f)
{
    return setArmorModel(static_cast<EntityPlayer*>(entityLiving), i, f);
}

void RenderPlayer::renderPlayer(EntityPlayer* entityPlayer, double d, double d1, double d2, float f, float f1) {
    ItemStack* itemStack = entityPlayer->inventory->getCurrentItem();
    int heldItemRight = itemStack != nullptr ? 1 : 0;
    bool aimedBow = false;
    if (itemStack != nullptr && entityPlayer->getItemInUseCount() > 0) {
        EnumAction action = itemStack->getItemUseAction();
        if (action == EnumAction::block) {
            heldItemRight = 3;
        } else if (action == EnumAction::bow) {
            aimedBow = true;
        }
    }

    modelArmorChestplate->heldItemRight = modelArmor->heldItemRight = modelBipedMain->heldItemRight = heldItemRight;
    modelArmorChestplate->aimedBow = modelArmor->aimedBow = modelBipedMain->aimedBow = aimedBow;
    modelArmorChestplate->isSneak = modelArmor->isSneak = modelBipedMain->isSneak = entityPlayer->isSneaking();
    double renderY = d1 - static_cast<double>(entityPlayer->yOffset);
    if (entityPlayer->isSneaking() && dynamic_cast<EntityPlayerSP*>(entityPlayer) == nullptr) {
        renderY -= 0.125;
    }

    RenderLiving::doRenderLiving(entityPlayer, d, renderY, d2, f, f1);

    modelArmorChestplate->aimedBow = modelArmor->aimedBow = modelBipedMain->aimedBow = false;
    modelArmorChestplate->isSneak = modelArmor->isSneak = modelBipedMain->isSneak = false;
    modelArmorChestplate->heldItemRight = modelArmor->heldItemRight = modelBipedMain->heldItemRight = 0;
}

void RenderPlayer::drawFirstPersonHand()
{
	modelBipedMain->onGround = 0.0f;
	modelBipedMain->setRotationAngles(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0625f);
	modelBipedMain->bipedRightArm->render(0.0625f);
}

void RenderPlayer::doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) {
    renderPlayer((EntityPlayer*)entityLiving, d, d1, d2, f, f1);
}

void RenderPlayer::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderPlayer((EntityPlayer*)entity, d, d1, d2, f, f1);
}

void RenderPlayer::renderName(EntityPlayer* entityPlayer, double d, double d1, double d2) {
    if (!Minecraft::isGuiEnabled() || entityPlayer == renderManager->livingPlayer) {
        return;
    }

    float distance = entityPlayer->getDistanceToEntity(renderManager->livingPlayer);
    float maxDistance = entityPlayer->isSneaking() ? 32.0f : 64.0f;
    if (distance >= maxDistance) {
        return;
    }

    const std::string& name = entityPlayer->username;
    if (!entityPlayer->isSneaking()) {
        renderLivingLabel(entityPlayer, name, d,
                          entityPlayer->isPlayerSleeping() ? d1 - 1.5 : d1,
                          d2, 64);
        return;
    }

    FontRenderer* fontRenderer = getFontRendererFromRenderManager();
    float scale = 0.01666667f * 1.6f;
    renderPushMatrix();
    renderTranslate((float)d, (float)d1 + 2.3f, (float)d2);
    renderNormal3f(0.0f, 1.0f, 0.0f);
    renderRotate(-renderManager->playerViewY, 0.0f, 1.0f, 0.0f);
    renderRotate(renderManager->playerViewX, 1.0f, 0.0f, 0.0f);
    renderScale(-scale, -scale, scale);
    renderDisable(RenderCapability::Lighting);
    renderTranslate(0.0f, 0.25f / scale, 0.0f);
    renderDepthMask(false);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);

    Tessellator* tessellator = &Tessellator::instance;
    renderDisable(RenderCapability::Texture2D);
    tessellator->startDrawingQuads();
    int halfWidth = fontRenderer->getStringWidth(name) / 2;
    tessellator->setColorRGBA_F(0.0f, 0.0f, 0.0f, 0.25f);
    tessellator->addVertex(-halfWidth - 1, -1.0f, 0.0f);
    tessellator->addVertex(-halfWidth - 1, 8.0f, 0.0f);
    tessellator->addVertex(halfWidth + 1, 8.0f, 0.0f);
    tessellator->addVertex(halfWidth + 1, -1.0f, 0.0f);
    tessellator->draw();
    renderEnable(RenderCapability::Texture2D);
    renderDepthMask(true);
    fontRenderer->drawString(name, -fontRenderer->getStringWidth(name) / 2, 0, 0x20ffffff);
    renderEnable(RenderCapability::Lighting);
    renderDisable(RenderCapability::Blend);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderPopMatrix();
}

void RenderPlayer::passSpecialRender(EntityLiving* entityLiving, double d, double d1, double d2) {
    renderName((EntityPlayer*)entityLiving, d, d1, d2);
}

void RenderPlayer::renderSpecials(EntityPlayer* entityPlayer, float f) {
    // Helmet blocks (pumpkins etc.) render attached to the head in Beta.
    ItemStack* helmet = entityPlayer->inventory->armorItemInSlot(3);
    if (helmet != nullptr && helmet->itemID >= 0 && helmet->itemID < 256 &&
        Block::blocksList[helmet->itemID] != nullptr) {
        renderPushMatrix();
        modelBipedMain->bipedHead->postRender(0.0625f);
        if (RenderBlocks::renderItemIn3d(Block::blocksList[helmet->itemID]->getRenderType())) {
            float scale = 0.625f;
            renderTranslate(0.0f, -0.25f, 0.0f);
            renderRotate(180.0f, 0.0f, 1.0f, 0.0f);
            renderScale(scale, -scale, scale);
        }
        renderManager->itemRenderer->renderItem(entityPlayer, helmet);
        renderPopMatrix();
    }

    // The original special ears for the deadmau5 account.
    if (entityPlayer->username == "deadmau5" &&
        loadDownloadableImageTexture(entityPlayer->skinUrl, "")) {
        for (int i = 0; i < 2; ++i) {
            float yaw = (entityPlayer->prevRotationYaw + (entityPlayer->rotationYaw - entityPlayer->prevRotationYaw) * f)
                      - (entityPlayer->prevRenderYawOffset + (entityPlayer->renderYawOffset - entityPlayer->prevRenderYawOffset) * f);
            float pitch = entityPlayer->prevRotationPitch + (entityPlayer->rotationPitch - entityPlayer->prevRotationPitch) * f;
            renderPushMatrix();
            renderRotate(yaw, 0.0f, 1.0f, 0.0f);
            renderRotate(pitch, 1.0f, 0.0f, 0.0f);
            renderTranslate(0.375f * (float)(i * 2 - 1), 0.0f, 0.0f);
            renderTranslate(0.0f, -0.375f, 0.0f);
            renderRotate(-pitch, 1.0f, 0.0f, 0.0f);
            renderRotate(-yaw, 0.0f, 1.0f, 0.0f);
            renderScale(4.0f / 3.0f, 4.0f / 3.0f, 4.0f / 3.0f);
            modelBipedMain->renderEars(0.0625f);
            renderPopMatrix();
        }
    }

    // Cape/cloak physics from Beta 1.7.3.
    if (!entityPlayer->playerCloakUrl.empty() &&
        loadDownloadableImageTexture(entityPlayer->playerCloakUrl, "")) {
        renderPushMatrix();
        renderTranslate(0.0f, 0.0f, 0.125f);
        const float dx = static_cast<float>(
            (entityPlayer->field_20066_r + (entityPlayer->field_20063_u - entityPlayer->field_20066_r) * static_cast<double>(f))
            - (entityPlayer->prevPosX + (entityPlayer->posX - entityPlayer->prevPosX) * static_cast<double>(f)));
        const float dy = static_cast<float>(
            (entityPlayer->field_20065_s + (entityPlayer->field_20062_v - entityPlayer->field_20065_s) * static_cast<double>(f))
            - (entityPlayer->prevPosY + (entityPlayer->posY - entityPlayer->prevPosY) * static_cast<double>(f)));
        const float dz = static_cast<float>(
            (entityPlayer->field_20064_t + (entityPlayer->field_20061_w - entityPlayer->field_20064_t) * static_cast<double>(f))
            - (entityPlayer->prevPosZ + (entityPlayer->posZ - entityPlayer->prevPosZ) * static_cast<double>(f)));
        const float bodyYaw = entityPlayer->prevRenderYawOffset +
            (entityPlayer->renderYawOffset - entityPlayer->prevRenderYawOffset) * f;
        const float sinYaw = MathHelper::sin((bodyYaw * 3.1415927f) / 180.0f);
        const float negCosYaw = -MathHelper::cos((bodyYaw * 3.1415927f) / 180.0f);
        float vertical = dy * 10.0f;
        if (vertical < -6.0f) vertical = -6.0f;
        if (vertical > 32.0f) vertical = 32.0f;
        float forward = (dx * sinYaw + dz * negCosYaw) * 100.0f;
        float sideways = (dx * negCosYaw - dz * sinYaw) * 100.0f;
        if (forward < 0.0f) forward = 0.0f;
        float cameraBobbing = entityPlayer->field_775_e + (entityPlayer->field_774_f - entityPlayer->field_775_e) * f;
        vertical += MathHelper::sin((entityPlayer->prevDistanceWalkedModified +
                    (entityPlayer->distanceWalkedModified - entityPlayer->prevDistanceWalkedModified) * f) * 6.0f)
                    * 32.0f * cameraBobbing;
        if (entityPlayer->isSneaking()) vertical += 25.0f;
        renderRotate(6.0f + forward / 2.0f + vertical, 1.0f, 0.0f, 0.0f);
        renderRotate(sideways / 2.0f, 0.0f, 0.0f, 1.0f);
        renderRotate(-sideways / 2.0f, 0.0f, 1.0f, 0.0f);
        renderRotate(180.0f, 0.0f, 1.0f, 0.0f);
        modelBipedMain->renderCloak(0.0625f);
        renderPopMatrix();
    }

    ItemStack* held = entityPlayer->inventory->getCurrentItem();
    ItemStack fishingStick(Item::stick);
    if (held != nullptr && entityPlayer->fishEntity != nullptr) {
        held = &fishingStick;
    }

    if (held != nullptr && held->isValid()) {
        renderPushMatrix();
        modelBipedMain->bipedRightArm->postRender(0.0625f);
        renderTranslate(-0.0625f, 0.4375f, 0.0625f);

        Item* item = held->getItem();
        if (item == nullptr) {
            renderPopMatrix();
            return;
        }

        EnumAction action = EnumAction::none;
        if (entityPlayer->getItemInUseCount() > 0) {
            action = held->getItemUseAction();
        }

        if (held->itemID >= 0 && held->itemID < 256 && Block::blocksList[held->itemID] != nullptr &&
            RenderBlocks::renderItemIn3d(Block::blocksList[held->itemID]->getRenderType())) {
            float scale = 0.5f;
            renderTranslate(0.0f, 0.1875f, -0.3125f);
            scale *= 0.75f;
            renderRotate(20.0f, 1.0f, 0.0f, 0.0f);
            renderRotate(45.0f, 0.0f, 1.0f, 0.0f);
            renderScale(scale, -scale, scale);
        } else if (Item::bow != nullptr && held->itemID == Item::bow->shiftedIndex) {
            float scale = 0.625f;
            renderTranslate(0.0f, 0.125f, 0.3125f);
            renderRotate(-20.0f, 0.0f, 1.0f, 0.0f);
            renderScale(scale, -scale, scale);
            renderRotate(-100.0f, 1.0f, 0.0f, 0.0f);
            renderRotate(45.0f, 0.0f, 1.0f, 0.0f);
        } else if (item->isFull3D()) {
            float scale = 0.625f;
            if (item->shouldRotateAroundWhenRendering()) {
                renderRotate(180.0f, 0.0f, 0.0f, 1.0f);
                renderTranslate(0.0f, -0.125f, 0.0f);
            }
            if (entityPlayer->getItemInUseCount() > 0 && action == EnumAction::block) {
                renderTranslate(0.05f, 0.0f, -0.1f);
                renderRotate(-50.0f, 0.0f, 1.0f, 0.0f);
                renderRotate(-10.0f, 1.0f, 0.0f, 0.0f);
                renderRotate(-60.0f, 0.0f, 0.0f, 1.0f);
            }
            renderTranslate(0.0f, 0.1875f, 0.0f);
            renderScale(scale, -scale, scale);
            renderRotate(-100.0f, 1.0f, 0.0f, 0.0f);
            renderRotate(45.0f, 0.0f, 1.0f, 0.0f);
        } else {
            float scale = 0.375f;
            renderTranslate(0.25f, 0.1875f, -0.1875f);
            renderScale(scale, scale, scale);
            renderRotate(60.0f, 0.0f, 0.0f, 1.0f);
            renderRotate(-90.0f, 1.0f, 0.0f, 0.0f);
            renderRotate(20.0f, 0.0f, 0.0f, 1.0f);
        }

        if (item->func_46058_c()) {
            for (int renderPass = 0; renderPass <= 1; ++renderPass) {
                int color = item->getColorFromDamage(held->getItemDamage(), renderPass);
                float red = static_cast<float>(color >> 16 & 0xff) / 255.0f;
                float green = static_cast<float>(color >> 8 & 0xff) / 255.0f;
                float blue = static_cast<float>(color & 0xff) / 255.0f;
                renderColor4f(red, green, blue, 1.0f);
                renderManager->itemRenderer->renderItem(entityPlayer, held, renderPass);
            }
        } else {
            renderManager->itemRenderer->renderItem(entityPlayer, held, 0);
        }

        renderPopMatrix();
    }

}

void RenderPlayer::renderEquippedItems(EntityLiving* entityLiving, float f) {
    renderSpecials((EntityPlayer*)entityLiving, f);
}

void RenderPlayer::translateEntity(EntityLiving* entityLiving, double d, double d1, double d2)
{
    EntityPlayer* player = static_cast<EntityPlayer*>(entityLiving);
    if (player->isEntityAlive() && player->isPlayerSleeping())
        RenderLiving::translateEntity(player, d + player->field_22063_x, d1 + player->field_22062_y, d2 + player->field_22061_z);
    else
        RenderLiving::translateEntity(player, d, d1, d2);
}

void RenderPlayer::rotateCorpse(EntityLiving* entityLiving, float f, float f1, float f2)
{
    EntityPlayer* player = static_cast<EntityPlayer*>(entityLiving);
    if (player->isEntityAlive() && player->isPlayerSleeping())
    {
        renderRotate(player->getBedOrientationInDegrees(), 0.0f, 1.0f, 0.0f);
        renderRotate(getDeathMaxRotation(player), 0.0f, 0.0f, 1.0f);
        renderRotate(270.0f, 0.0f, 1.0f, 0.0f);
    }
    else
    {
        RenderLiving::rotateCorpse(player, f, f1, f2);
    }
}

void RenderPlayer::preRenderCallback(EntityLiving* entityLiving, float f) {
    renderScale(0.9375f, 0.9375f, 0.9375f);
}
