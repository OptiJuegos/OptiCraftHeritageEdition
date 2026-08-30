#pragma once

#include "PlayerController.h"

class NetClientHandler;

// net.minecraft.src.PlayerControllerMP
class PlayerControllerMP : public PlayerController
{
public:
	PlayerControllerMP(Minecraft *minecraft, NetClientHandler *netclienthandler);
	~PlayerControllerMP() override;

	void setCreative(bool creative);
	bool shouldDrawHUD() override;
	bool func_35642_f() override;
	void flipPlayer(EntityPlayer *entityplayer) override;
	bool sendBlockRemoved(int_t i, int_t j, int_t k, int_t l) override;
	void clickBlock(int_t i, int_t j, int_t k, int_t l) override;
	void resetBlockRemoving() override;
	void sendBlockRemoving(int_t i, int_t j, int_t k, int_t l) override;
	void setPartialTime(float f) override;
	float getBlockReachDistance() override;
	void onWorldChanged(World *world) override;
	void updateController() override;
	bool sendPlaceBlock(EntityPlayer *entityplayer, World *world, ItemStack *itemstack, int_t i, int_t j, int_t k, int_t l) override;
	bool sendUseItem(EntityPlayer *entityplayer, World *world, ItemStack *itemstack) override;
	void onStoppedUsingItem(EntityPlayer *entityplayer) override;
	EntityPlayer *createPlayer(World *world) override;
	void attackEntity(EntityPlayer *entityplayer, Entity *entity) override;
	void interactWithEntity(EntityPlayer *entityplayer, Entity *entity) override;
	ItemStack *handleWindowClick(int_t i, int_t j, int_t k, bool flag, EntityPlayer *entityplayer) override;
	void sendEnchantPacket(int_t windowId, int_t enchantment) override;
	void closeWindow(int_t i, EntityPlayer *entityplayer) override;
	void sendSlotPacket(ItemStack *itemstack, int_t slot) override;
	void sendPacketDropItem(ItemStack *itemstack) override;
	bool isNotCreative() override;
	bool isInCreativeMode() override;
	bool extendedReach() override;

private:
	void syncCurrentPlayItem();

	int_t currentBlockX;
	int_t currentBlockY;
	int_t currentblockZ;
	float curBlockDamageMP;
	float prevBlockDamageMP;
	float field_9441_h;
	int_t blockHitDelay;
	bool isHittingBlock;
	bool creativeMode;
	NetClientHandler *netClientHandler;
	int_t currentPlayerItem;
};
