#pragma once

#include "PlayerController.h"

class EntityPlayer;
class ItemStack;
class Minecraft;
class World;

// net.minecraft.src.PlayerControllerCreative
class PlayerControllerCreative : public PlayerController
{
public:
    explicit PlayerControllerCreative(Minecraft *minecraft);

    static void enableAbilities(EntityPlayer *player);
    static void disableAbilities(EntityPlayer *player);
    static void clickBlockCreative(Minecraft *minecraft, PlayerController *controller, int_t x, int_t y, int_t z, int_t side);

    void initializePlayer(EntityPlayer *player) override;
    bool sendPlaceBlock(EntityPlayer *player, World *world, ItemStack *itemstack, int_t x, int_t y, int_t z, int_t side) override;
    void clickBlock(int_t x, int_t y, int_t z, int_t side) override;
    void sendBlockRemoving(int_t x, int_t y, int_t z, int_t side) override;
    void resetBlockRemoving() override;
    bool shouldDrawHUD() override;
    void onWorldChanged(World *world) override;
    float getBlockReachDistance() override;
    bool isNotCreative() override;
    bool isInCreativeMode() override;
    bool extendedReach() override;

private:
    int_t blockHitDelay;
};
