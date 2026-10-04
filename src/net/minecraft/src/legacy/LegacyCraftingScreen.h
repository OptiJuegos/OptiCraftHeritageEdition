#pragma once

#include "net/minecraft/src/GuiScreen.h"

class InventoryPlayer;
class World;
class EntityPlayer;
class RenderItem;
class ItemStack;
class Container;
struct PlatformTextInputSnapshot;

class LegacyCraftingScreen : public GuiScreen
{
public:
    static const int_t kCategoryCount = 5;

    LegacyCraftingScreen(InventoryPlayer *playerInventory, World *world, int_t x, int_t y, int_t z,
                         bool is2x2 = false, EntityPlayer *player = nullptr);
    virtual ~LegacyCraftingScreen();

    void initGui() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
    void updateScreen() override;
    void onGuiClosed() override;
    bool doesGuiPauseGame() override { return false; }
    int getOwnerPlayerIndex() const override;
    // Pad input lives here rather than in updateScreen(): the specialized
    // hook runs first in GuiScreen::handleInput, so the consume-on-read text
    // latch is still full, and on 3DS/Wii the D-pad/A edges that the menu
    // channel also queues as keyboard events cannot double-fire a step.
    void handleSpecializedMenuInput() override;

protected:
    bool usesSpecializedMenuNavigation() const override { return true; }
    void keyTyped(char_t c, int_t key) override;
    void mouseClicked(int_t mouseX, int_t mouseY, int_t button) override;

private:
    void handleNavigation(int dirX, int dirY);
    void changeCategory(int dir);
    void changeVariant(int dir);
    void craftCurrentRecipe();
    // Multiplayer: the server is the only authority, so direct mainInventory
    // mutation (used offline) is invisible to it and gets reverted by the
    // next window sync. The legacy screen instead emulates the window clicks
    // a Java mouse user would send for the same craft -- place ingredients
    // one by one, click the result slot -- which both simulates the result
    // locally AND queues the Packet102 on the wire.
    bool craftCurrentRecipeViaContainerClicks();
    bool canCraftCurrentRecipe() const;
    void ensureSelectionVisible();
    void drawSlotRect(int_t sx, int_t sy);
    void drawTooltip(ItemStack *stack, int_t mouseX, int_t mouseY);
    bool playerHasIngredient(int_t itemId, int_t itemDamage) const;
    void updateCraftingState();
    uint32_t computeInventoryHash() const;

    bool m_craftingStateDirty;
    bool m_cachedCanCraft;
    bool m_cachedSlotHasIngredient[9];
    uint32_t m_cachedInventoryHash;

    // The strip doubles as the console inventory (with legacyUI the inventory
    // key opens this screen), so its 4x9 slot grid is navigable and the A
    // button/touch grabs a stack and swaps it with the next slot acted on.
    // Navigation is two-zoned: D-pad DOWN from the recipe carousel enters the
    // strip under the recipe cursor, UP from the strip's first row returns to
    // the recipes.
    void enterInventoryZone();
    void handleInventoryZoneNavigation(int dirX, int dirY);
    void clickStripSlot(int slotIndex);
    static int stripSlotIndex(int row, int col) { return row < 3 ? 9 + row * 9 + col : col; }

    // Close through the player (GuiContainer::keyTyped does the same) instead
    // of clearing the screen directly: the MP side of closeScreen() emits the
    // Packet101CloseWindow for a live workbench container and resets
    // craftingInventory; skipping it left the server's window open and
    // desynced every later click.
    void legacyCloseScreen();

    // Circle-pad/D-pad hold-repeat (3DS): a held navigation direction steps
    // again on the console's repeat cadence. The deadline lives here so a
    // re-opened screen never inherits a running timer.
    void legacyNavigationRepeat(const PlatformTextInputSnapshot &pad);

    InventoryPlayer *inventory;
    World *world;
    int_t posX, posY, posZ;
    bool is2x2Mode;
    EntityPlayer *entityPlayer;

    int_t selectedCategory;
    int_t visibleCategoryIndices[kCategoryCount];
    int_t visibleCategoryCount;
    int_t selectedVisibleTab;
    int_t selectedGroup[kCategoryCount];
    int_t selectedVariant[kCategoryCount][32];
    int_t scrollOffset[kCategoryCount];

    // Inventory-strip cursor state (see enterInventoryZone). grabbedSlotIndex
    // is a mainInventory index the player lifted a stack from, or -1; the
    // swap happens in place between two slots, so no cursor item ever exists
    // and there is nothing to restore if the screen closes mid-move.
    bool invZoneActive;
    int_t invCursorRow;
    int_t invCursorCol;
    int grabbedSlotIndex;

    int_t craftHoldTicks;
    bool ps2ActionReleaseLatch;
    // Hold-repeat deadline for the navigation bits (3DS -- see
    // legacyNavigationRepeat), in consoleInputNowMs time.
    int_t navRepeatMs;
    int_t guiLeft;
    int_t guiTop;
    int_t xSize;
    int_t ySize;
    int ownerPlayerIndex;
    // Multiplayer workbench (3x3) sessions run through a real server window:
    // NetClientHandler::handleOpenWindow stamps the server's windowId onto the
    // player's live container right after showing the screen, so the legacy
    // screen must own a ContainerWorkbench there just like GuiCrafting does --
    // otherwise the stamp lands on the vanilla inventory container and
    // silently desyncs every later click. P2P/singleplayer keep the pure
    // local-mutation path untouched.
    Container *mpWorkbenchContainer = nullptr;

    static RenderItem *itemRenderer;
};
