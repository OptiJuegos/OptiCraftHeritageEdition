#include "PlayerControllerCreative.h"

#include "Block.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "Minecraft.h"
#include "PlayerCapabilities.h"
#include "Session.h"
#include "World.h"

PlayerControllerCreative::PlayerControllerCreative(Minecraft *minecraft)
    : PlayerController(minecraft), blockHitDelay(0)
{
    field_1064_b = true;
}

void PlayerControllerCreative::enableAbilities(EntityPlayer *player)
{
    if (player == nullptr)
        return;
    player->capabilities.allowFlying = true;
    player->capabilities.isCreativeMode = true;
    player->capabilities.disableDamage = true;
}

void PlayerControllerCreative::disableAbilities(EntityPlayer *player)
{
    if (player == nullptr)
        return;
    player->capabilities.allowFlying = false;
    player->capabilities.isFlying = false;
    player->capabilities.isCreativeMode = false;
    player->capabilities.disableDamage = false;
}

void PlayerControllerCreative::initializePlayer(EntityPlayer *player)
{
    enableAbilities(player);
    if (player == nullptr || player->inventory == nullptr)
        return;
    for (int_t slot = 0; slot < 9 && slot < (int_t)Session::registeredBlocksList.size(); ++slot)
    {
        if (player->inventory->mainInventory[slot] == nullptr && Session::registeredBlocksList[(std::size_t)slot] != nullptr)
            player->inventory->mainInventory[slot] = new ItemStack(Session::registeredBlocksList[(std::size_t)slot]);
    }
}

void PlayerControllerCreative::clickBlockCreative(Minecraft *minecraft, PlayerController *controller, int_t x, int_t y, int_t z, int_t side)
{
    if (minecraft == nullptr || minecraft->theWorld == nullptr || minecraft->thePlayer == nullptr || controller == nullptr)
        return;
    if (!minecraft->theWorld->extinguishFire(minecraft->thePlayer, x, y, z, side))
        controller->sendBlockRemoved(x, y, z, side);
}

bool PlayerControllerCreative::sendPlaceBlock(EntityPlayer *player, World *world, ItemStack *itemstack, int_t x, int_t y, int_t z, int_t side)
{
    if (world == nullptr || player == nullptr)
        return false;
    int_t blockId = world->getBlockId(x, y, z);
    if (blockId > 0 && blockId < Block::BLOCK_REGISTRY_SIZE && Block::blocksList[blockId] != nullptr &&
        Block::blocksList[blockId]->blockActivated(world, x, y, z, player))
        return true;
    if (itemstack == nullptr)
        return false;
    int_t damage = itemstack->getItemDamage();
    int_t count = itemstack->stackSize;
    bool result = itemstack->useItem(player, world, x, y, z, side);
    itemstack->setItemDamage(damage);
    itemstack->stackSize = count;
    return result;
}

void PlayerControllerCreative::clickBlock(int_t x, int_t y, int_t z, int_t side)
{
    clickBlockCreative(mc, this, x, y, z, side);
    blockHitDelay = 5;
}

void PlayerControllerCreative::sendBlockRemoving(int_t x, int_t y, int_t z, int_t side)
{
    --blockHitDelay;
    if (blockHitDelay <= 0)
    {
        blockHitDelay = 5;
        clickBlockCreative(mc, this, x, y, z, side);
    }
}

void PlayerControllerCreative::resetBlockRemoving()
{
}

bool PlayerControllerCreative::shouldDrawHUD()
{
    return false;
}

void PlayerControllerCreative::onWorldChanged(World *world)
{
    PlayerController::onWorldChanged(world);
}

float PlayerControllerCreative::getBlockReachDistance()
{
    return 5.0f;
}

bool PlayerControllerCreative::isNotCreative()
{
    return false;
}

bool PlayerControllerCreative::isInCreativeMode()
{
    return true;
}

bool PlayerControllerCreative::extendedReach()
{
    return true;
}
