#ifndef RENDERPLAYER_H
#define RENDERPLAYER_H

#include "RenderLiving.h"

class EntityPlayer;
class ModelBiped;

class RenderPlayer : public RenderLiving {
public:
    RenderPlayer();
    ~RenderPlayer() override;

    void renderPlayer(EntityPlayer* entityPlayer, double d, double d1, double d2, float f, float f1);

    virtual void doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) override;

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;
	void drawFirstPersonHand();

protected:
    void renderName(EntityPlayer* entityPlayer, double d, double d1, double d2);

    void passSpecialRender(EntityLiving* entityLiving, double d, double d1, double d2) override;

    int setArmorModel(EntityPlayer* entityPlayer, int i, float f);

    int shouldRenderPass(EntityLiving* entityLiving, int i, float f) override;

    virtual void renderSpecials(EntityPlayer* entityPlayer, float f);

    virtual void renderEquippedItems(EntityLiving* entityLiving, float f) override;

    virtual void preRenderCallback(EntityLiving* entityLiving, float f) override;

    void translateEntity(EntityLiving* entityLiving, double d, double d1, double d2) override;
    void rotateCorpse(EntityLiving* entityLiving, float f, float f1, float f2) override;

private:
    ModelBiped* modelBipedMain;
    ModelBiped* modelArmorChestplate;
    ModelBiped* modelArmor;
    int field_208_f;
};

#endif
