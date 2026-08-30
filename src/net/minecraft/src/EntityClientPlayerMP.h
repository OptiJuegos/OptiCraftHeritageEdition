#pragma once

#include "EntityPlayerSP.h"

class NetClientHandler;
class EntityItem;

// net.minecraft.src.EntityClientPlayerMP
class EntityClientPlayerMP : public EntityPlayerSP
{
public:
	EntityClientPlayerMP(Minecraft *minecraft, World *world, Session *session, NetClientHandler *netclienthandler);

	static constexpr int_t CLASS_ID = 16;
	int_t getEntityClassID() const override { return CLASS_ID; }

	bool attackEntityFrom(Entity *entity, int_t i) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
	void heal(int_t i) override;
	void onUpdate() override;
	void sendMotionUpdates();
	void dropCurrentItem() override;
	EntityItem *dropOneItem() override;
	void joinEntityItemWithWorld(EntityItem *entityitem) override;
	void sendChatMessage(const std::string &s) override;
	void swingItem() override;
	void respawnPlayer() override;
	void damageEntity(int_t i) override;
	void damageEntity(const DamageSource &source, int_t damage) override;
	void closeScreen() override;
	void setHealth(int_t i) override;
	void addStat(StatBase *statbase, int_t i) override;
	void incrementStat(StatBase *statbase, int_t i);
	void addServerStatistic(StatBase *statbase, int_t i);
	void func_50009_aI() override;

public:
	NetClientHandler *sendQueue;

private:
	void sendInventoryChanged();

	int_t field_9380_bx;
	bool field_21093_bH;
	double oldPosX;
	double field_9378_bz;
	double oldPosY;
	double oldPosZ;
	float oldRotationYaw;
	float oldRotationPitch;
	bool field_9382_bF;
	bool wasSprinting;
	bool wasSneaking;
	int_t field_12242_bI;
};
