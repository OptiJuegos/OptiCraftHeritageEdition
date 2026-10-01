// Console crafting menu of the Xbox port, also used on PS2 when "Console
// Crafting" is on (upstream's LegacyCraftingScreen otherwise).
#if defined(XBOX_PLATFORM) || defined(PS2_PLATFORM)
#include "XboxCraftingScreen.h"

#include <algorithm>

#include "net/minecraft/src/ControlIcon.h"
#include "LegacyCraftingGroups.h"
#include "LegacyMenuHints.h"
#include "LegacyOptionsLayout.h"
#include "LegacyOptionsPanel.h"
#include "LegacyUiTexture.h"
#include "net/minecraft/src/Tessellator.h"
#include "net/minecraft/src/UiStrings.h"
#include "net/minecraft/src/Container.h"
#include "net/minecraft/src/ContainerWorkbench.h"
#include "net/minecraft/src/CraftingManager.h"
#include "net/minecraft/src/EntityPlayerSP.h"
#include "net/minecraft/src/FontRenderer.h"
#include "net/minecraft/src/IRecipe.h"
#include "net/minecraft/src/InventoryPlayer.h"
#include "net/minecraft/src/ItemStack.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/OpenGlHelper.h"
#include "net/minecraft/src/PlayerController.h"
#include "net/minecraft/src/RenderEngine.h"
#include "net/minecraft/src/RenderHelper.h"
#include "net/minecraft/src/RenderItem.h"
#include "net/minecraft/src/ShapedRecipes.h"
#include "net/minecraft/src/ShapelessRecipes.h"
#include "net/minecraft/src/Slot.h"
#include "net/minecraft/src/SoundManager.h"
#include "platform/Input.h"
#include "platform/PlatformTuning.h"
#include "platform/RenderAPI.h"

namespace
{
constexpr int_t INVENTORY_SLOTS = 36;
constexpr int_t FEEDBACK_TICKS = 12;

// Tab order and the item drawn on each tab (ids as in 1.2.5).
struct TabInfo { int_t group; const char *name; int_t iconId; };
const TabInfo kTabs[LEGACY_CRAFT_GROUP_COUNT] = {
    { LEGACY_CRAFT_STRUCTURE,  "Structures", 45 },   // brick
    { LEGACY_CRAFT_TOOL,       "Tools",      257 },  // iron pickaxe
    { LEGACY_CRAFT_FOOD,       "Food",       297 },  // bread
    { LEGACY_CRAFT_ARMOUR,     "Armour",     307 },  // iron chestplate
    { LEGACY_CRAFT_MECHANISM,  "Mechanisms", 331 },  // redstone
    { LEGACY_CRAFT_TRANSPORT,  "Transport",  66 },   // rail
    { LEGACY_CRAFT_DECORATION, "Decoration", 38 },   // rose
};

const LegacyCraftingGroupEntry *groupEntryFor(int_t id, int_t damage)
{
    const LegacyCraftingGroupEntry *any = nullptr;
    const LegacyCraftingGroupEntry *first = nullptr;
    for (const LegacyCraftingGroupEntry &entry : s_legacyCraftingGroups)
    {
        if (entry.id != id)
            continue;
        if (entry.aux == damage)
            return &entry;
        if (entry.aux == -1 && any == nullptr)
            any = &entry;
        if (first == nullptr)
            first = &entry;
    }
    return any != nullptr ? any : first;
}

bool stackMatches(ItemStack *stack, int_t id, int_t damage)
{
    return stack != nullptr && stack->itemID == id && (damage == -1 || stack->getItemDamage() == damage);
}

void playSound(Minecraft *mc, const char *name)
{
    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX(name, 1.0f, 1.0f);
}
} // namespace

RenderItem *XboxCraftingScreen::itemRenderer = nullptr;

XboxCraftingScreen::XboxCraftingScreen(EntityPlayer *player)
    : GuiContainer(player->inventorySlots, false), gridSize(2), tab(0), column(0), lastSignature(0),
      feedbackTicks(0), feedbackOk(false), displayMode(0)
{
}

XboxCraftingScreen::XboxCraftingScreen(EntityPlayer *player, World *world, int_t x, int_t y, int_t z)
    : GuiContainer(new ContainerWorkbench(player->inventory, world, x, y, z), true), gridSize(3), tab(0),
      column(0), lastSignature(0), feedbackTicks(0), feedbackOk(false), displayMode(0)
{
}

XboxCraftingScreen::~XboxCraftingScreen()
{
    for (auto &icon : icons)
        delete icon.second;
}

ItemStack *XboxCraftingScreen::iconStack(int_t id, int_t damage)
{
    if (damage < 0)
        damage = 0;
    const int_t key = id * 65536 + damage;
    auto found = icons.find(key);
    if (found != icons.end())
        return found->second;
    ItemStack *stack = new ItemStack(id, 1, damage);
    icons[key] = stack;
    return stack;
}

void XboxCraftingScreen::initGui()
{
    GuiContainer::initGui();
    if (itemRenderer == nullptr)
        itemRenderer = new RenderItem();

    tabs.clear();
    for (const TabInfo &info : kTabs)
    {
        // The inventory's 2x2 grid cannot fit any armour piece.
        if (gridSize == 2 && info.group == LEGACY_CRAFT_ARMOUR)
            continue;
        tabs.push_back(info.group);
    }
    tab = std::min<int_t>(tab, static_cast<int_t>(tabs.size()) - 1);

    buildEntries();
    buildColumns();
}

// --- recipe model -------------------------------------------------------------

void XboxCraftingScreen::buildEntries()
{
    entries.clear();
    const std::vector<IRecipe *> &recipes = CraftingManager::getInstance()->getRecipeList();
    for (IRecipe *recipe : recipes)
    {
        if (recipe == nullptr || recipe->getRecipeOutput() == nullptr)
            continue;

        Entry entry;
        entry.recipe = recipe;
        entry.output = recipe->getRecipeOutput();
        entry.canMake = false;
        entry.missingCells = 0;
        for (int_t i = 0; i < 9; ++i)
        {
            entry.cellId[i] = 0;
            entry.cellDamage[i] = -1;
        }

        if (ShapedRecipes *shaped = dynamic_cast<ShapedRecipes *>(recipe))
        {
            const int_t w = shaped->getWidth();
            const int_t h = shaped->getHeight();
            if (w > gridSize || h > gridSize)
                continue;
            for (int_t row = 0; row < h; ++row)
            {
                for (int_t col = 0; col < w; ++col)
                {
                    ItemStack *item = shaped->getPatternItem(col + row * w);
                    if (item == nullptr)
                        continue;
                    entry.cellId[col + row * gridSize] = item->itemID;
                    entry.cellDamage[col + row * gridSize] = item->getItemDamage();
                }
            }
        }
        else if (ShapelessRecipes *shapeless = dynamic_cast<ShapelessRecipes *>(recipe))
        {
            const std::vector<ItemStack *> &items = shapeless->getIngredients();
            if (static_cast<int_t>(items.size()) > gridSize * gridSize)
                continue;
            for (int_t i = 0; i < static_cast<int_t>(items.size()); ++i)
            {
                entry.cellId[i] = items[i]->itemID;
                entry.cellDamage[i] = items[i]->getItemDamage();
            }
        }
        else
        {
            continue;
        }

        for (int_t i = 0; i < gridSize * gridSize; ++i)
        {
            if (entry.cellId[i] == 0)
                continue;
            bool merged = false;
            for (Ingredient &need : entry.needs)
            {
                if (need.id == entry.cellId[i] && need.damage == entry.cellDamage[i])
                {
                    ++need.count;
                    merged = true;
                    break;
                }
            }
            if (!merged)
                entry.needs.push_back({ entry.cellId[i], entry.cellDamage[i], 1 });
        }
        if (!entry.needs.empty())
            entries.push_back(entry);
    }
}

void XboxCraftingScreen::buildColumns()
{
    columns.clear();
    column = 0;
    if (tabs.empty())
        return;
    const int_t group = tabs[tab];
    for (int_t i = 0; i < static_cast<int_t>(entries.size()); ++i)
    {
        ItemStack *output = entries[i].output;
        const LegacyCraftingGroupEntry *info = groupEntryFor(output->itemID, output->getItemDamage());
        const int_t entryGroup = info != nullptr ? info->group : LEGACY_CRAFT_DECORATION;
        if (entryGroup != group)
            continue;
        const int_t baseType = info != nullptr ? info->baseType : 0;

        // Items with a base type share a column; the rest get one each.
        Column *target = nullptr;
        if (baseType != 0)
        {
            for (Column &existing : columns)
            {
                if (existing.baseType == baseType)
                {
                    target = &existing;
                    break;
                }
            }
        }
        if (target == nullptr)
        {
            columns.push_back({ baseType, {}, 0 });
            target = &columns.back();
        }
        target->entries.push_back(i);
    }
    lastSignature = 0;
    refreshAvailability();
}

int_t XboxCraftingScreen::countInInventory(int_t id, int_t damage) const
{
    int_t total = 0;
    InventoryPlayer *inventory = mc->thePlayer->inventory;
    for (int_t i = 0; i < INVENTORY_SLOTS; ++i)
    {
        if (stackMatches(inventory->mainInventory[i], id, damage))
            total += inventory->mainInventory[i]->stackSize;
    }
    return total;
}

unsigned int XboxCraftingScreen::inventorySignature() const
{
    unsigned int hash = 2166136261u;
    InventoryPlayer *inventory = mc->thePlayer->inventory;
    for (int_t i = 0; i < INVENTORY_SLOTS; ++i)
    {
        ItemStack *stack = inventory->mainInventory[i];
        const unsigned int value = stack == nullptr ? 0u :
            static_cast<unsigned int>(stack->itemID) * 31u + static_cast<unsigned int>(stack->getItemDamage()) * 7u +
            static_cast<unsigned int>(stack->stackSize);
        hash = (hash ^ value) * 16777619u;
    }
    return hash | 1u;   // never 0, the "rebuild" marker
}

void XboxCraftingScreen::refreshAvailability()
{
    const unsigned int signature = inventorySignature();
    if (signature == lastSignature)
        return;
    lastSignature = signature;

    for (const Column &col : columns)
    {
        for (int_t index : col.entries)
        {
            Entry &entry = entries[index];
            entry.canMake = true;
            entry.missingCells = 0;
            for (const Ingredient &need : entry.needs)
            {
                int_t missing = need.count - countInInventory(need.id, need.damage);
                if (missing <= 0)
                    continue;
                entry.canMake = false;
                // Mark the last `missing` cells holding this ingredient.
                for (int_t cell = gridSize * gridSize - 1; cell >= 0 && missing > 0; --cell)
                {
                    if (entry.cellId[cell] == need.id && entry.cellDamage[cell] == need.damage)
                    {
                        entry.missingCells |= static_cast<unsigned short>(1u << cell);
                        --missing;
                    }
                }
            }
        }
    }
}

XboxCraftingScreen::Entry *XboxCraftingScreen::selectedEntry()
{
    if (column < 0 || column >= static_cast<int_t>(columns.size()))
        return nullptr;
    const Column &col = columns[column];
    if (col.entries.empty())
        return nullptr;
    return &entries[col.entries[std::min<int_t>(col.selected, static_cast<int_t>(col.entries.size()) - 1)]];
}

// --- input --------------------------------------------------------------------

void XboxCraftingScreen::moveColumn(int_t delta)
{
    if (columns.empty())
        return;
    const int_t count = static_cast<int_t>(columns.size());
    column = (column + delta + count) % count;
    playSound(mc, "random.focus");
}

void XboxCraftingScreen::moveTier(int_t delta)
{
    if (column < 0 || column >= static_cast<int_t>(columns.size()))
        return;
    Column &col = columns[column];
    const int_t count = static_cast<int_t>(col.entries.size());
    if (count <= 1)
        return;
    col.selected = (col.selected + delta + count) % count;
    playSound(mc, "random.focus");
}

void XboxCraftingScreen::moveTab(int_t delta)
{
    const int_t count = static_cast<int_t>(tabs.size());
    if (count <= 0)
        return;
    tab = (tab + delta + count) % count;
    buildColumns();
    playSound(mc, "random.scroll");
}

void XboxCraftingScreen::updateScreen()
{
    GuiContainer::updateScreen();
    if (mc->currentScreen != this || mc->thePlayer == nullptr)
        return;
    if (feedbackTicks > 0)
        --feedbackTicks;
    refreshAvailability();

    const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
    if ((pad.pressed & (PLATFORM_TEXT_CLOSE | PLATFORM_TEXT_SHIFT)) != 0)
    {
        playSound(mc, "random.back");
        mc->thePlayer->closeScreen();
        return;
    }
    if ((pad.pressed & PLATFORM_TEXT_PREV_PAGE) != 0)
        moveTab(-1);
    if ((pad.pressed & PLATFORM_TEXT_NEXT_PAGE) != 0)
        moveTab(1);
    if ((pad.pressed & PLATFORM_TEXT_LEFT) != 0)
        moveColumn(-1);
    if ((pad.pressed & PLATFORM_TEXT_RIGHT) != 0)
        moveColumn(1);
    if ((pad.pressed & PLATFORM_TEXT_UP) != 0)
        moveTier(-1);
    if ((pad.pressed & PLATFORM_TEXT_DOWN) != 0)
        moveTier(1);
    if ((pad.pressed & PLATFORM_TEXT_TYPE) != 0)
        craftSelected();
    if ((pad.pressed & PLATFORM_TEXT_BACK) != 0)
    {
        // X: the lower right panel shows the inventory or the ingredient list.
        displayMode ^= 1;
        playSound(mc, "random.focus");
    }
}

void XboxCraftingScreen::mouseClicked(int_t, int_t, int_t)
{
}

void XboxCraftingScreen::mouseMovedOrUp(int_t, int_t, int_t)
{
}

void XboxCraftingScreen::keyTyped(char_t c, int_t key)
{
    // Keyboard (PC): Esc / inventory key close, as in any container.
    GuiContainer::keyTyped(c, key);
}

// --- crafting through the container --------------------------------------------

void XboxCraftingScreen::click(int_t slot, int_t button, bool shift)
{
    delete mc->playerController->windowClick(inventorySlots->windowId, slot, button, shift, mc->thePlayer);
}

int_t XboxCraftingScreen::findInventorySlot(int_t id, int_t damage) const
{
    // The player inventory is always the last 36 slots of both containers.
    const int_t count = static_cast<int_t>(inventorySlots->slots.size());
    for (int_t slot = count - INVENTORY_SLOTS; slot < count; ++slot)
    {
        if (stackMatches(inventorySlots->slots[slot]->getStack(), id, damage))
            return slot;
    }
    return -1;
}

void XboxCraftingScreen::clearGrid()
{
    for (int_t cell = 0; cell < gridSize * gridSize; ++cell)
    {
        if (inventorySlots->slots[1 + cell]->getHasStack())
            click(1 + cell, 0, true);
    }
}

void XboxCraftingScreen::craftSelected()
{
    Entry *entry = selectedEntry();
    InventoryPlayer *inventory = mc->thePlayer->inventory;
    if (entry == nullptr || !entry->canMake || inventory->getItemStack() != nullptr)
    {
        feedbackOk = false;
        feedbackTicks = FEEDBACK_TICKS;
        return;
    }

    // Lay the recipe out cell by cell: pick the stack up, drop one item on the
    // cell (right click), put the rest back where it came from.
    bool placed = true;
    for (int_t cell = 0; cell < gridSize * gridSize && placed; ++cell)
    {
        if (entry->cellId[cell] == 0)
            continue;
        const int_t from = findInventorySlot(entry->cellId[cell], entry->cellDamage[cell]);
        if (from < 0)
        {
            placed = false;
            break;
        }
        click(from, 0, false);
        click(1 + cell, 1, false);
        if (inventory->getItemStack() != nullptr)
            click(from, 0, false);
        if (inventory->getItemStack() != nullptr)
            placed = false;
    }

    ItemStack *result = inventorySlots->slots[0]->getStack();
    const bool ok = placed && result != nullptr && result->itemID == entry->output->itemID;
    if (ok)
        click(0, 0, true);   // shift-click: the result goes into the inventory
    clearGrid();             // leftovers (empty buckets...) or an aborted layout

    feedbackOk = ok && !inventorySlots->slots[0]->getHasStack();
    feedbackTicks = FEEDBACK_TICKS;
    if (feedbackOk)
        playSound(mc, "random.pop");
    lastSignature = 0;
    refreshAvailability();
}

// --- drawing -------------------------------------------------------------------
//
// Geometry follows the Legacy Console Edition crafting scene at 1280x720
// (xuiscene_craftingpanel_3x3/2x2): every number below is in those pixels,
// relative to the panel's top-left corner, and is scaled to the GUI when drawn.
// The panel art is drawn from its colours (black outline, white/grey bevel,
// #C6C6C6 body, #8B8B8B slots) instead of shipping the 689x490 textures.

namespace
{
struct SceneLayout
{
    int_t panelW;
    int_t slotX0;       // first craftable slot box (54 px pitch, y = 140)
    int_t slotCount;
    int_t nameX, nameW; // selected item name, centred, y = 288
    int_t gridX, gridY, gridStep, gridCell;
    int_t arrowX, arrowY;
    int_t outX, outY;   // 72x72 result box
    int_t leftX0, leftX1, rightX0, rightX1;   // the two lower recesses, y 283..475
    int_t invX, invY, hotbarY, labelX, labelW;
    int_t ingX, ingY;
};

const SceneLayout kLayout3x3 = { 689, 21, 12, 21, 281, 23, 332, 46, 46, 175, 382, 229, 364,
                                 15, 308, 313, 674, 351, 324, 436, 321, 344, 333, 321 };
const SceneLayout kLayout2x2 = { 591, 26, 10, 16, 246, 29, 343, 48, 48, 137, 373, 179, 353,
                                 15, 265, 270, 576, 278, 325, 437, 274, 298, 288, 324 };

constexpr int_t SCENE_H = 490;
constexpr int_t TAB_W = 98;
constexpr int_t TAB_H = 66;
constexpr int_t ROW_Y = 140;
constexpr int_t SLOT_PITCH = 54;
constexpr int_t RECESS_Y0 = 283;
constexpr int_t RECESS_Y1 = 475;

constexpr int_t C_BLACK = 0xff000000;
constexpr int_t C_WHITE = 0xffffffff;
constexpr int_t C_BODY = 0xffc6c6c6;
constexpr int_t C_SHADOW = 0xff555555;
constexpr int_t C_TAB = 0xff9e9e9e;
constexpr int_t C_TAB_LIGHT = 0xffcccccc;
constexpr int_t C_SLOT_DARK = 0xff373737;
constexpr int_t C_SLOT = 0xff8b8b8b;
constexpr int_t C_SLOT_RED = 0xffd13838;
constexpr int_t C_RECESS = 0xffa6a6a6;
constexpr int_t C_TEXT = 0x404040;

LegacyUiTexture s_iconsTexture("/legacy/crafting_icons.png");
}

namespace
{
struct Scene
{
    const SceneLayout *layout;
    float_t k;
    int_t ox, oy;
    int_t t1, t2;

    int_t x(int_t v) const { return ox + static_cast<int_t>(v * k + 0.5f); }
    int_t y(int_t v) const { return oy + static_cast<int_t>(v * k + 0.5f); }
    int_t s(int_t v) const { return std::max<int_t>(1, static_cast<int_t>(v * k + 0.5f)); }
};
}

void XboxCraftingScreen::drawGuiContainerBackgroundLayer(float_t)
{
}

void XboxCraftingScreen::drawRaised(int_t x0, int_t y0, int_t x1, int_t y1, int_t t1, int_t t2,
    int_t light, int_t fill, bool openBottom)
{
    const int_t bottomOutline = openBottom ? 0 : t1;
    drawRect(x0, y0, x1, y1, C_BLACK);
    drawRect(x0 + t1, y0 + t1, x1 - t1, y1 - bottomOutline, light);
    drawRect(x0 + t1 + t2, y0 + t1 + t2, x1 - t1, y1 - bottomOutline, C_SHADOW);
    drawRect(x0 + t1 + t2, y0 + t1 + t2, x1 - t1 - t2, y1 - bottomOutline - (openBottom ? 0 : t2), fill);
}

void XboxCraftingScreen::drawSunken(int_t x0, int_t y0, int_t x1, int_t y1, int_t t1, int_t fill)
{
    drawRect(x0, y0, x1, y1, C_SLOT_DARK);
    drawRect(x0 + t1, y0 + t1, x1, y1, C_WHITE);
    drawRect(x0 + t1, y0 + t1, x1 - t1, y1 - t1, fill);
}

void XboxCraftingScreen::drawHighlight(int_t x0, int_t y0, int_t x1, int_t y1, int_t t1)
{
    // Craft_Highlight_L: three 3 px rings, dark / pale green / mid green.
    const int_t rings[3] = { static_cast<int_t>(0xff2a3128u), static_cast<int_t>(0xffa1b29du), static_cast<int_t>(0xff434f41u) };
    for (int_t i = 0; i < 3; ++i)
    {
        const int_t a = i * t1;
        const int_t b = a + t1;
        drawRect(x0 + a, y0 + a, x1 - a, y0 + b, rings[i]);
        drawRect(x0 + a, y1 - b, x1 - a, y1 - a, rings[i]);
        drawRect(x0 + a, y0 + b, x0 + b, y1 - b, rings[i]);
        drawRect(x1 - b, y0 + b, x1 - a, y1 - b, rings[i]);
    }
}

void XboxCraftingScreen::drawAtlasIcon(int_t texture, int_t u, int_t v, int_t size, int_t x, int_t y,
    int_t w, int_t h)
{
    if (texture < 0)
        return;
    mc->renderEngine->bindTexture(texture);
    renderEnable(RenderCapability::Texture2D);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    const float_t u0 = u / 256.0f, v0 = v / 128.0f;
    const float_t u1 = (u + size) / 256.0f, v1 = (v + size) / 128.0f;
    Tessellator *tess = &Tessellator::instance;
    tess->startDrawingQuads();
    tess->addVertexWithUV(x, y + h, zLevel, u0, v1);
    tess->addVertexWithUV(x + w, y + h, zLevel, u1, v1);
    tess->addVertexWithUV(x + w, y, zLevel, u1, v0);
    tess->addVertexWithUV(x, y, zLevel, u0, v0);
    tess->draw();
    renderDisable(RenderCapability::Blend);
}

void XboxCraftingScreen::drawItem(ItemStack *stack, int_t x, int_t y, int_t size, bool overlay)
{
    if (stack == nullptr)
        return;
    const float_t scale = size / 16.0f;
    renderPushMatrix();
    renderTranslate(static_cast<float_t>(x), static_cast<float_t>(y), 0.0f);
    renderScale(scale, scale, 1.0f);
    itemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine, stack, 0, 0);
    if (overlay)
        itemRenderer->renderItemOverlayIntoGUI(fontRenderer, mc->renderEngine, stack, 0, 0);
    renderPopMatrix();
}

void XboxCraftingScreen::drawDarkCentred(const std::string &text, int_t centreX, int_t y, int_t maxW)
{
    const std::string shown = fontRenderer->trimStringToWidth(text, std::max<int_t>(1, maxW));
    fontRenderer->drawString(shown, centreX - fontRenderer->getStringWidth(shown) / 2, y, C_TEXT);
}

void XboxCraftingScreen::drawHints()
{
    const std::string buttons[] = {"Cross", "Circle", "Square", "L1/R1"};
    const std::string actions[] = {uiText("Craft"), uiText("Back"),
        displayMode == 0 ? uiText("Ingredients") : uiText("Inventory"), uiText("Group")};
    drawControlHintRow(mc, width, legacyHintRowY(height), buttons, actions, 4);
}

void XboxCraftingScreen::drawScreen(int_t, int_t, float_t)
{
    drawDefaultBackground();

    Scene sc;
    sc.layout = gridSize == 3 ? &kLayout3x3 : &kLayout2x2;
    const SceneLayout &L = *sc.layout;
    const int_t hintY = legacyHintRowY(height);
    sc.k = std::min<float_t>((width - 8) / static_cast<float_t>(L.panelW + 6),
        (hintY - 8) / static_cast<float_t>(SCENE_H + 5));
    sc.ox = (width - static_cast<int_t>(L.panelW * sc.k)) / 2;
    sc.oy = std::max<int_t>(4, (hintY - 4 - static_cast<int_t>(SCENE_H * sc.k)) / 2);
    sc.t1 = sc.s(3);
    sc.t2 = sc.s(6);
    const int_t t1 = sc.t1, t2 = sc.t2;

    // Body, then the tab row on top of it.
    drawRaised(sc.x(0), sc.y(TAB_H), sc.x(L.panelW), sc.y(SCENE_H), t1, t2, C_WHITE, C_BODY, false);
    const int_t tabCount = static_cast<int_t>(tabs.size());
    for (int_t i = 0; i < tabCount; ++i)
    {
        if (i != tab)
            drawRaised(sc.x(i * TAB_W), sc.y(0), sc.x(std::min<int_t>((i + 1) * TAB_W, L.panelW)), sc.y(TAB_H),
                t1, t2, C_TAB_LIGHT, C_TAB, true);
    }
    // The selected tab is taller and covers the body's top border (Tab_*.png).
    if (tab >= 0 && tab < tabCount)
        drawRaised(sc.x(tab * TAB_W - 3), sc.y(-5), sc.x(tab * TAB_W + 104), sc.y(TAB_H) + t1 + t2 + 1,
            t1, t2, C_WHITE, C_BODY, true);

    // Group name
    const TabInfo *activeInfo = nullptr;
    for (const TabInfo &info : kTabs)
    {
        if (!tabs.empty() && info.group == tabs[tab])
            activeInfo = &info;
    }
    if (activeInfo != nullptr)
        drawDarkCentred(uiText(activeInfo->name), sc.x(L.panelW / 2), sc.y(93) + (sc.s(27) - 8) / 2,
            sc.s(568));

    // Craftable slots
    const int_t columnCount = static_cast<int_t>(columns.size());
    const int_t first = std::max<int_t>(0, std::min<int_t>(column - L.slotCount / 2, columnCount - L.slotCount));
    for (int_t i = 0; i < L.slotCount; ++i)
    {
        const int_t bx = L.slotX0 + i * SLOT_PITCH;
        drawSunken(sc.x(bx), sc.y(ROW_Y), sc.x(bx + SLOT_PITCH), sc.y(ROW_Y + SLOT_PITCH), t1, C_SLOT);
    }

    // Tier scroller over the selected column, then the green highlight.
    const bool columnVisible = column >= first && column < first + L.slotCount && column < columnCount;
    int_t tierCount = columnVisible ? static_cast<int_t>(columns[column].entries.size()) : 0;
    const int_t selBoxX = L.slotX0 + (column - first) * SLOT_PITCH;
    int_t highlightY = ROW_Y;
    int_t tierSlotY[3] = { -1, -1, -1 };   // slot box y of the entries drawn in the scroller
    int_t tierEntry[3] = { -1, -1, -1 };
    if (columnVisible && tierCount > 1)
    {
        const Column &col = columns[column];
        const int_t sel = std::min<int_t>(col.selected, tierCount - 1);
        const int_t sx = selBoxX - 13;
        const int_t top = tierCount >= 3 ? ROW_Y - 102 : ROW_Y - 48;
        const int_t slots = tierCount >= 3 ? 3 : 2;
        drawRaised(sc.x(sx + 4), sc.y(top + 24), sc.x(sx + 76), sc.y(top + 48 + slots * SLOT_PITCH + 24),
            t1, t2, C_WHITE, C_BODY, false);
        for (int_t j = 0; j < slots; ++j)
        {
            const int_t by = top + 48 + j * SLOT_PITCH;
            drawSunken(sc.x(selBoxX), sc.y(by), sc.x(selBoxX + SLOT_PITCH), sc.y(by + SLOT_PITCH), t1, C_SLOT);
            tierSlotY[j] = by;
        }
        if (slots == 3)
        {
            tierEntry[0] = col.entries[(sel - 1 + tierCount) % tierCount];
            tierEntry[1] = col.entries[sel];
            tierEntry[2] = col.entries[(sel + 1) % tierCount];
        }
        else
        {
            tierEntry[0] = col.entries[0];
            tierEntry[1] = col.entries[1];
            highlightY = ROW_Y + sel * SLOT_PITCH;
        }
        // Scroll arrows
        const int_t ax = sc.x(sx + 40);
        const int_t upY = sc.y(top + 32);
        const int_t downY = sc.y(top + 48 + slots * SLOT_PITCH + 12);
        for (int_t r = 0; r < sc.s(8); ++r)
        {
            drawRect(ax - r, upY + r, ax + r + 1, upY + r + 1, 0xff414141);
            drawRect(ax - r, downY + sc.s(8) - r, ax + r + 1, downY + sc.s(8) - r + 1, 0xff414141);
        }
    }
    if (columnVisible)
        drawHighlight(sc.x(selBoxX - 9), sc.y(highlightY - 9), sc.x(selBoxX + 63), sc.y(highlightY + 63), t1);

    // Lower recesses
    drawSunken(sc.x(L.leftX0), sc.y(RECESS_Y0), sc.x(L.leftX1), sc.y(RECESS_Y1), t1, C_RECESS);
    drawSunken(sc.x(L.rightX0), sc.y(RECESS_Y0), sc.x(L.rightX1), sc.y(RECESS_Y1), t1, C_RECESS);

    Entry *entry = selectedEntry();
    for (int_t cell = 0; cell < gridSize * gridSize; ++cell)
    {
        const int_t cx = L.gridX + (cell % gridSize) * L.gridStep;
        const int_t cy = L.gridY + (cell / gridSize) * L.gridStep;
        const bool missing = entry != nullptr && (entry->missingCells & (1u << cell)) != 0;
        drawSunken(sc.x(cx), sc.y(cy), sc.x(cx + L.gridCell), sc.y(cy + L.gridCell), t1, missing ? C_SLOT_RED : C_SLOT);
    }
    drawSunken(sc.x(L.outX), sc.y(L.outY), sc.x(L.outX + 72), sc.y(L.outY + 72), t1,
        entry != nullptr && !entry->canMake ? C_SLOT_RED : C_SLOT);

    InventoryPlayer *inventory = mc->thePlayer->inventory;
    if (displayMode == 0)
    {
        for (int_t i = 0; i < INVENTORY_SLOTS; ++i)
        {
            const int_t cx = L.invX + (i % 9) * 32;
            const int_t cy = i < 9 ? L.hotbarY : L.invY + ((i - 9) / 9) * 32;
            drawSunken(sc.x(cx), sc.y(cy), sc.x(cx + 32), sc.y(cy + 32), t1, C_SLOT);
        }
    }
    else if (entry != nullptr)
    {
        for (int_t j = 0; j < static_cast<int_t>(entry->needs.size()) && j < 4; ++j)
        {
            const int_t cy = L.ingY + j * 36;
            const bool enough = countInInventory(entry->needs[j].id, entry->needs[j].damage) >= entry->needs[j].count;
            drawSunken(sc.x(L.ingX), sc.y(cy), sc.x(L.ingX + 32), sc.y(cy + 32), t1, enough ? C_SLOT : C_SLOT_RED);
        }
    }

    // Tab icons and the crafting arrow (Legacy art)
    const int_t iconTexture = s_iconsTexture.resolve(mc->renderEngine);
    for (int_t i = 0; i < tabCount; ++i)
    {
        const int_t group = tabs[i];
        const int_t iconY = i == tab ? 9 : 13;
        drawAtlasIcon(iconTexture, (group % 5) * 48, (group / 5) * 48, 48,
            sc.x(i * TAB_W + 25), sc.y(iconY), sc.s(48), sc.s(48));
    }
    drawAtlasIcon(iconTexture, 96, 48, 32, sc.x(L.arrowX), sc.y(L.arrowY), sc.s(32), sc.s(32));

    // Items
#if PLATFORM_GUI_FORCE_DEPTH_DISABLED
    renderEnable(RenderCapability::DepthTest);
    renderDepthMask(true);
    renderDepthFunc(RenderCompare::LessEqual);
#endif
    RenderHelper::enableGUIStandardItemLighting();
    renderEnable(RenderCapability::RescaleNormal);
    OpenGlHelper::setLightmapTextureCoords(OpenGlHelper::lightmapTexUnit, 240.0f, 240.0f);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    const int_t bigItem = sc.s(40);
    const int_t bigInset = sc.s(7);
    for (int_t i = 0; i < L.slotCount && first + i < columnCount; ++i)
    {
        const int_t bx = L.slotX0 + i * SLOT_PITCH;
        const Column &col = columns[first + i];
        const bool scrolled = first + i == column && tierCount >= 3;
        const int_t shown = scrolled ? std::min<int_t>(col.selected, static_cast<int_t>(col.entries.size()) - 1) : 0;
        drawItem(entries[col.entries[shown]].output, sc.x(bx) + bigInset, sc.y(ROW_Y) + bigInset, bigItem, true);
    }
    for (int_t j = 0; j < 3; ++j)
    {
        if (tierEntry[j] >= 0 && tierSlotY[j] != ROW_Y)
            drawItem(entries[tierEntry[j]].output, sc.x(selBoxX) + bigInset, sc.y(tierSlotY[j]) + bigInset, bigItem, true);
    }

    if (entry != nullptr)
    {
        const int_t cellItem = sc.s(L.gridCell - 12);
        const int_t cellInset = sc.s(6);
        for (int_t cell = 0; cell < gridSize * gridSize; ++cell)
        {
            if (entry->cellId[cell] == 0)
                continue;
            drawItem(iconStack(entry->cellId[cell], entry->cellDamage[cell]),
                sc.x(L.gridX + (cell % gridSize) * L.gridStep) + cellInset,
                sc.y(L.gridY + (cell / gridSize) * L.gridStep) + cellInset, cellItem, false);
        }
        drawItem(entry->output, sc.x(L.outX) + sc.s(12), sc.y(L.outY) + sc.s(12), sc.s(48), true);
    }

    const int_t smallItem = sc.s(26);
    const int_t smallInset = sc.s(3);
    if (displayMode == 0)
    {
        for (int_t i = 0; i < INVENTORY_SLOTS; ++i)
        {
            const int_t cx = L.invX + (i % 9) * 32;
            const int_t cy = i < 9 ? L.hotbarY : L.invY + ((i - 9) / 9) * 32;
            drawItem(inventory->mainInventory[i], sc.x(cx) + smallInset, sc.y(cy) + smallInset, smallItem, true);
        }
    }
    else if (entry != nullptr)
    {
        for (int_t j = 0; j < static_cast<int_t>(entry->needs.size()) && j < 4; ++j)
            drawItem(iconStack(entry->needs[j].id, entry->needs[j].damage), sc.x(L.ingX) + smallInset,
                sc.y(L.ingY + j * 36) + smallInset, smallItem, false);
    }

    renderDisable(RenderCapability::RescaleNormal);
    RenderHelper::disableStandardItemLighting();
    renderDisable(RenderCapability::Lighting);
    renderDisable(RenderCapability::DepthTest);

    // What cannot be made is drawn half transparent in Legacy; fade it into the slot.
    for (int_t i = 0; i < L.slotCount && first + i < columnCount; ++i)
    {
        const Column &col = columns[first + i];
        const bool scrolled = first + i == column && tierCount >= 3;
        const int_t shown = scrolled ? std::min<int_t>(col.selected, static_cast<int_t>(col.entries.size()) - 1) : 0;
        if (!entries[col.entries[shown]].canMake)
        {
            const int_t bx = sc.x(L.slotX0 + i * SLOT_PITCH) + t1;
            const int_t by = sc.y(ROW_Y) + t1;
            drawRect(bx, by, sc.x(L.slotX0 + (i + 1) * SLOT_PITCH) - t1, sc.y(ROW_Y + SLOT_PITCH) - t1, 0x908b8b8b);
        }
    }
    for (int_t j = 0; j < 3; ++j)
    {
        if (tierEntry[j] >= 0 && tierSlotY[j] != ROW_Y && !entries[tierEntry[j]].canMake)
            drawRect(sc.x(selBoxX) + t1, sc.y(tierSlotY[j]) + t1, sc.x(selBoxX + SLOT_PITCH) - t1,
                sc.y(tierSlotY[j] + SLOT_PITCH) - t1, 0x908b8b8b);
    }

    // Texts
    if (entry != nullptr)
    {
        const std::vector<std::string> info = entry->output->getItemNameandInformation();
        drawDarkCentred(info.empty() ? std::string() : info[0], sc.x(L.nameX + L.nameW / 2),
            sc.y(288) + (sc.s(27) - 8) / 2, sc.s(L.nameW));
    }
    drawDarkCentred(displayMode == 0 ? uiText("Inventory") : uiText("Ingredients"), sc.x(L.labelX + L.labelW / 2),
        sc.y(288) + (sc.s(27) - 8) / 2, sc.s(L.labelW));
    if (displayMode != 0 && entry != nullptr)
    {
        for (int_t j = 0; j < static_cast<int_t>(entry->needs.size()) && j < 4; ++j)
        {
            const Ingredient &need = entry->needs[j];
            const int_t have = countInInventory(need.id, need.damage);
            const std::vector<std::string> needInfo = iconStack(need.id, need.damage)->getItemNameandInformation();
            const std::string line = std::to_string(need.count) + " x " + (needInfo.empty() ? std::string() : needInfo[0]);
            const int_t tx = sc.x(L.ingX + 40);
            fontRenderer->drawString(fontRenderer->trimStringToWidth(line, sc.x(L.rightX1 - 8) - tx), tx,
                sc.y(L.ingY + j * 36) + (sc.s(32) - 8) / 2, have >= need.count ? C_TEXT : 0xb02020);
        }
    }
    if (columns.empty())
        drawDarkCentred(uiText("Nothing to craft here"), sc.x(L.panelW / 2), sc.y(ROW_Y) + (sc.s(54) - 8) / 2,
            sc.s(L.panelW));

    drawHints();
}

#endif // XBOX_PLATFORM || PS2_PLATFORM
