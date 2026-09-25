#include "client/Ps2SplitScreen.h"

#ifdef PS2_PLATFORM

#include "client/Minecraft.h"
#include "net/minecraft/src/EntityPlayerSP.h"
#include "net/minecraft/src/World.h"
#include "net/minecraft/src/WorldInfo.h"
#include "net/minecraft/src/WorldProvider.h"
#include "net/minecraft/src/Session.h"
#include "net/minecraft/src/MovementInputFromOptions.h"
#include "net/minecraft/src/skin/SkinManager.h"
#include "net/minecraft/src/GuiInventory.h"
#include "net/minecraft/src/GuiIngame.h"
#include "net/minecraft/src/InventoryPlayer.h"
#include "net/minecraft/src/FoodStats.h"
#include "net/minecraft/src/ChunkCoordinates.h"
#include "net/minecraft/src/PlayerController.h"
#include "net/minecraft/src/SoundManager.h"
#include "net/minecraft/src/StringTranslate.h"
#include "net/minecraft/src/MovingObjectPosition.h"
#include "platform/Input.h"
#include "ps2/input/Ps2PadState.h"

#include <cmath>

namespace Ps2SplitScreen
{

static constexpr float SPLITSCREEN_MAX_DIST = 32.0f;
static Session s_sessionP2("Player 2", "");

static bool isSpanishLanguage()
{
    StringTranslate *tr = StringTranslate::getInstance();
    return (tr != nullptr && tr->getCurrentLanguage().rfind("es_", 0) == 0);
}

void joinPlayer2(Minecraft *mc)
{
    if (mc == nullptr || mc->theWorld == nullptr || mc->thePlayer == nullptr || mc->thePlayer2 != nullptr)
        return;

    if (mc->theWorld->multiplayerWorld)
        return;

    WorldInfo *worldInfo = mc->theWorld->getWorldInfo();
    if (worldInfo == nullptr || !worldInfo->isLimitedWorld())
    {
        if (mc->ingameGUI != nullptr)
        {
            if (isSpanishLanguage())
                mc->ingameGUI->addChatMessage("\xc2\xa7" "c[Pantalla Dividida] Solo disponible en mundos Clasicos (256x256).");
            else
                mc->ingameGUI->addChatMessage("\xc2\xa7" "c[Split-Screen] Only available in Classic (256x256) worlds.");
        }
        return;
    }

    mc->thePlayerOne = mc->thePlayer;

    EntityPlayerSP *p2 = new EntityPlayerSP(mc, mc->theWorld, &s_sessionP2, mc->theWorld->worldProvider->worldType);
    delete p2->movementInput;
    p2->movementInput = new MovementInputFromOptions(mc->gameSettings, 1);

    const std::string skinP2 = SkinManager::getPlayer2SkinTexture();
    if (!skinP2.empty())
    {
        p2->skinUrl = "";
        p2->setEntityTexture(skinP2);
    }

    p2->setLocationAndAngles(
        mc->thePlayer->posX + 1.0,
        mc->thePlayer->posY,
        mc->thePlayer->posZ + 1.0,
        mc->thePlayer->rotationYaw,
        mc->thePlayer->rotationPitch
    );

    p2->capabilities = mc->thePlayer->capabilities;

    mc->theWorld->spawnEntityInWorld(p2);

    mc->thePlayer2 = p2;
    mc->setSplitScreenActive(true);

    if (mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.levelup", 1.0f, 1.0f);

    if (mc->ingameGUI != nullptr)
    {
        if (isSpanishLanguage())
        {
            mc->ingameGUI->addChatMessage("\xc2\xa7" "a[P2] Jugador 2 conectado!");
            mc->ingameGUI->addChatMessage("\xc2\xa7" "b[P2] [X] Saltar | [R3] Agachar | [Cuadrado] Inventario");
        }
        else
        {
            mc->ingameGUI->addChatMessage("\xc2\xa7" "a[P2] Player 2 connected!");
            mc->ingameGUI->addChatMessage("\xc2\xa7" "b[P2] [X] Jump | [R3] Sneak | [Square] Inventory");
        }
    }
}

void leavePlayer2(Minecraft *mc)
{
    if (mc == nullptr)
        return;

    if (mc->thePlayer2 != nullptr)
    {
        if (mc->theWorld != nullptr)
            mc->theWorld->detachEntityForWorldChange(mc->thePlayer2);
        delete mc->thePlayer2;
        mc->thePlayer2 = nullptr;
    }
    delete mc->objectMouseOver2;
    mc->objectMouseOver2 = nullptr;

    if (mc->thePlayerOne != nullptr)
        mc->thePlayer = mc->thePlayerOne;

    mc->setSplitScreenActive(false);
    mc->setScreenOwnedByPlayer2(false);
    ps2SetMenuPad(0);
    ps2SetMenuOwnerPad(-1);
}

void tick(Minecraft *mc)
{
    if (mc == nullptr)
        return;

    const Ps2PadSnapshot &pad2 = ps2PadGetSnapshot(1);
    if (!pad2.connected)
        return;

    static unsigned short s_prevTickHeld = 0;
    const unsigned short tickPressed = static_cast<unsigned short>(pad2.held & ~s_prevTickHeld);
    const unsigned short pad2Released = static_cast<unsigned short>(s_prevTickHeld & ~pad2.held);
    s_prevTickHeld = pad2.held;

    if (mc->thePlayer2 == nullptr)
    {
        if ((tickPressed & PS2_PAD_START) && mc->theWorld != nullptr && !mc->theWorld->multiplayerWorld && mc->thePlayer != nullptr)
        {
            joinPlayer2(mc);
        }
        return;
    }

    EntityPlayerSP *p2 = mc->thePlayer2;

    // Auto-respawn if Player 2 is dead
    if (p2->isDead || p2->getHealth() <= 0)
    {
        p2->isDead = false;
        p2->deathTime = 0;
        p2->setHealth(20);
        if (p2->getFoodStats() != nullptr)
        {
            p2->getFoodStats()->setFoodLevel(20);
            p2->getFoodStats()->setFoodSaturationLevel(5.0f);
        }
        p2->clearActivePotions();
        if (mc->thePlayerOne != nullptr && !mc->thePlayerOne->isDead)
        {
            p2->setLocationAndAngles(
                mc->thePlayerOne->posX + 1.0,
                mc->thePlayerOne->posY,
                mc->thePlayerOne->posZ + 1.0,
                0.0f, 0.0f);
        }
        else if (mc->theWorld != nullptr)
        {
            ChunkCoordinates spawn = mc->theWorld->getSpawnPoint();
            p2->setLocationAndAngles(spawn.x + 0.5, spawn.y + 1.0, spawn.z + 0.5, 0.0f, 0.0f);
        }
        p2->motionX = 0.0;
        p2->motionY = 0.0;
        p2->motionZ = 0.0;
        return;
    }

    // START button (Pause menu)
    if ((tickPressed & PS2_PAD_START) && mc->currentScreen == nullptr)
    {
        mc->setScreenOwnedByPlayer2(true);
        ps2SetMenuPad(1);
        ps2SetMenuOwnerPad(1);
        mc->displayInGameMenu();
        return;
    }

    // Hotbar selection
    if (p2->inventory != nullptr)
    {
        if (tickPressed & PS2_PAD_R1)
            p2->inventory->currentItem = (p2->inventory->currentItem + 1) % 9;
        if (tickPressed & PS2_PAD_L1)
            p2->inventory->currentItem = (p2->inventory->currentItem + 8) % 9;
        if (mc->currentScreen == nullptr && (tickPressed & PS2_PAD_TRIANGLE))
            p2->dropOneItem();
    }

    // Square button: Open P2 Inventory
    if (tickPressed & PS2_PAD_SQUARE)
    {
        if (mc->currentScreen == nullptr)
        {
            mc->setScreenOwnedByPlayer2(true);
            mc->thePlayer = p2;
            ps2SetMenuPad(1);
            ps2SetMenuOwnerPad(1);
            mc->displayGuiScreen(new GuiInventory(p2));
            return;
        }
        else if (!mc->isScreenOwnedByPlayer2())
        {
            if (mc->ingameGUI != nullptr)
            {
                if (isSpanishLanguage())
                    mc->ingameGUI->addChatMessage("\xc2\xa7" "c[P2] Turno ocupado por Jugador 1");
                else
                    mc->ingameGUI->addChatMessage("\xc2\xa7" "c[P2] Menu in use by Player 1");
            }
        }
    }

    // Skip world interaction while a menu is open
    if (mc->currentScreen != nullptr)
        return;

    // Build / Attack / Use actions for Player 2
    EntityPlayerSP *p1 = mc->thePlayer;
    MovingObjectPosition *hr1 = mc->objectMouseOver;

    mc->thePlayer = p2;
    mc->objectMouseOver = mc->objectMouseOver2;

    if (pad2.held & PS2_PAD_R2)
    {
        mc->clickMouse(0, true);
    }
    else if (pad2Released & PS2_PAD_R2)
    {
        if (mc->playerController != nullptr)
            mc->playerController->resetBlockRemoving();
    }

    if (tickPressed & PS2_PAD_L2)
    {
        mc->clickMouse(1);
    }

    if (mc->currentScreen != nullptr)
    {
        mc->setScreenOwnedByPlayer2(true);
        ps2SetMenuPad(1);
        ps2SetMenuOwnerPad(1);
        // Leave mc->thePlayer as p2 so the screen operates on Player 2!
        mc->objectMouseOver = hr1;
        return;
    }

    // Restore Player 1
    mc->thePlayer = p1;
    mc->objectMouseOver = hr1;
}

void postTick(Minecraft *mc)
{
    if (mc == nullptr || !mc->isSplitScreenActive() || mc->thePlayer2 == nullptr || mc->thePlayerOne == nullptr)
        return;

    EntityPlayerSP *p1 = mc->thePlayerOne;
    EntityPlayerSP *p2 = mc->thePlayer2;
    if (p1->isDead || p2->isDead)
        return;

    double dx = p2->posX - p1->posX;
    double dz = p2->posZ - p1->posZ;
    double distSq = dx * dx + dz * dz;
    if (distSq <= (SPLITSCREEN_MAX_DIST * SPLITSCREEN_MAX_DIST))
        return;

    double dist = std::sqrt(distSq);
    if (dist < 1e-4)
        return;

    double nx = dx / dist;
    double nz = dz / dist;

    // Decompose velocity into radial component along the tether vector.
    // p2Out: outward separating velocity of P2 moving away from P1 (> 0 means moving away)
    // p1Out: outward separating velocity of P1 moving away from P2 (> 0 means moving away)
    double p2Out = p2->motionX * nx + p2->motionZ * nz;
    double p1Out = -p1->motionX * nx - p1->motionZ * nz;

    // Only cancel separating motion. Tangential velocity (smooth sliding along perimeter)
    // and inward velocity (walking back towards the partner) are completely untouched!
    // This eliminates any sticking or "glue" feeling when hitting the tether boundary.
    if (p2Out > 0.0)
    {
        p2->motionX -= p2Out * nx;
        p2->motionZ -= p2Out * nz;
    }
    if (p1Out > 0.0)
    {
        p1->motionX += p1Out * nx;
        p1->motionZ += p1Out * nz;
    }

    // Clamp player positions smoothly onto the tether boundary perimeter.
    // Using setPosition() keeps the bounding box and physics in exact sync.
    if (p2Out >= p1Out)
    {
        p2->setPosition(p1->posX + nx * SPLITSCREEN_MAX_DIST, p2->posY, p1->posZ + nz * SPLITSCREEN_MAX_DIST);
    }
    else
    {
        p1->setPosition(p2->posX - nx * SPLITSCREEN_MAX_DIST, p1->posY, p2->posZ - nz * SPLITSCREEN_MAX_DIST);
    }
}

} // namespace Ps2SplitScreen

#endif // PS2_PLATFORM
