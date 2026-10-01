#pragma once

#include <map>
#include <string>
#include <vector>

#include "net/minecraft/src/GuiContainer.h"

class EntityPlayer;
class IRecipe;
class ItemStack;
class RenderItem;
class World;

// Console-style crafting menu: recipes sorted into tabs, one column per kind of
// item (the wood/stone/iron/... versions stacked in it) and the ingredients of
// the selected recipe, with the missing ones in red.
//
// It is only a front end. Pressing A lays the recipe out in the real crafting
// grid of the open container (the 2x2 of the player's inventory or the 3x3 of
// a workbench) and takes the result through the same window clicks the normal
// screen sends, so single player and servers see an ordinary crafting action.
class XboxCraftingScreen : public GuiContainer
{
public:
    // 2x2: crafts in the player's own inventory container (not owned).
    explicit XboxCraftingScreen(EntityPlayer *player);
    // 3x3: a workbench at (x, y, z).
    XboxCraftingScreen(EntityPlayer *player, World *world, int_t x, int_t y, int_t z);
    ~XboxCraftingScreen() override;

    void initGui() override;
    void updateScreen() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
    bool usesSpecializedMenuNavigation() const override { return true; }
    bool allowsPlatformPointerInput() const override { return false; }

protected:
    void drawGuiContainerBackgroundLayer(float_t partialTick) override;
    void mouseClicked(int_t x, int_t y, int_t button) override;
    void mouseMovedOrUp(int_t x, int_t y, int_t button) override;
    void keyTyped(char_t c, int_t key) override;

private:
    struct Ingredient
    {
        int_t id;
        int_t damage;   // -1 = any
        int_t count;
    };

    struct Entry
    {
        IRecipe *recipe;
        ItemStack *output;
        int_t cellId[9];      // pattern laid out from the top-left, 0 = empty
        int_t cellDamage[9];
        std::vector<Ingredient> needs;
        bool canMake;
        unsigned short missingCells;   // bit per grid cell short of items
    };

    struct Column
    {
        int_t baseType;
        std::vector<int_t> entries;
        int_t selected;
    };

    void buildEntries();
    void buildColumns();
    void refreshAvailability();
    unsigned int inventorySignature() const;
    int_t countInInventory(int_t id, int_t damage) const;

    Entry *selectedEntry();
    void moveColumn(int_t delta);
    void moveTier(int_t delta);
    void moveTab(int_t delta);
    void craftSelected();

    void click(int_t slot, int_t button, bool shift);
    int_t findInventorySlot(int_t id, int_t damage) const;
    void clearGrid();

    // One display stack per (id, damage), kept for the life of the screen so
    // drawing allocates nothing per frame.
    ItemStack *iconStack(int_t id, int_t damage);

    // Legacy panel pieces, in screen units (t1/t2 = the 3 px / 6 px bevels).
    void drawRaised(int_t x0, int_t y0, int_t x1, int_t y1, int_t t1, int_t t2, int_t light, int_t fill,
        bool openBottom);
    void drawSunken(int_t x0, int_t y0, int_t x1, int_t y1, int_t t1, int_t fill);
    void drawHighlight(int_t x0, int_t y0, int_t x1, int_t y1, int_t t1);
    void drawAtlasIcon(int_t texture, int_t u, int_t v, int_t size, int_t x, int_t y, int_t w, int_t h);
    void drawItem(ItemStack *stack, int_t x, int_t y, int_t size, bool overlay);
    void drawDarkCentred(const std::string &text, int_t centreX, int_t y, int_t maxW);
    void drawHints();

    int_t gridSize;
    std::vector<int_t> tabs;
    int_t tab;
    int_t column;
    std::vector<Entry> entries;
    std::vector<Column> columns;
    unsigned int lastSignature;
    int_t feedbackTicks;
    bool feedbackOk;
    int_t displayMode;   // 0 inventory, 1 ingredients (X toggles, as in Legacy)
    std::map<int_t, ItemStack *> icons;

    static RenderItem *itemRenderer;
};
