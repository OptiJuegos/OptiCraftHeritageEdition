#include "LegacyCraftingScreen.h"

#include <algorithm>
#include <vector>
#include <string>

#include "net/minecraft/src/Block.h"
#include "net/minecraft/src/BlockFlower.h"
#include "net/minecraft/src/EntityPlayerSP.h"
#include "net/minecraft/src/Item.h"
#include "net/minecraft/src/ItemStack.h"
#include "net/minecraft/src/InventoryPlayer.h"
#include "net/minecraft/src/EntityPlayer.h"
#include "net/minecraft/src/World.h"
#include "net/minecraft/src/Container.h"
#include "net/minecraft/src/ContainerWorkbench.h"
#include "net/minecraft/src/Slot.h"
#include "net/minecraft/src/SlotCrafting.h"
#include "net/minecraft/src/PlayerController.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/FontRenderer.h"
#include "net/minecraft/src/RenderEngine.h"
#include "net/minecraft/src/RenderItem.h"
#include "net/minecraft/src/RenderHelper.h"
#include "net/minecraft/src/SoundManager.h"
#include "net/minecraft/src/ControlIcon.h"
#include "net/minecraft/src/UiStrings.h"
#include "net/minecraft/src/GuiInventory.h"
#include "net/minecraft/src/KeyBinding.h"
#include "net/minecraft/src/OpenGlHelper.h"
#include "platform/Input.h"
#include "platform/PlatformConfig.h"
#include "platform/RenderAPI.h"
#include "platform/ConsoleInputClock.h"
#include "LegacyMenuHints.h"
#include "LegacySelectionCursor.h"
#include "LegacyUiTheme.h"
#include "pc/lwjgl/Keyboard.h"

#if PLATFORM_PS2
#include "ps2/input/Ps2PadKeyCodes.h"
#include "ps2/input/Ps2PadState.h"
#endif

RenderItem *LegacyCraftingScreen::itemRenderer = new RenderItem();

namespace
{

struct RecipeIngredient
{
    int_t itemId = 0;
    int_t count = 0;
    int_t itemDamage = -1;
};

struct RecipeVariant
{
    const char *name = nullptr;
    int_t resultId = 0;
    int_t resultCount = 1;
    int_t resultDamage = 0;
    bool requiresWorkbench = false;
    int_t gridWidth = 0;
    int_t gridHeight = 0;
    int_t gridItemIds[9] = {0};
    int_t gridItemDamage[9] = {0};
    int_t ingredientCount = 0;
    RecipeIngredient ingredients[6];

    ItemStack *resultStack = nullptr;
    ItemStack *gridStacks[9] = {nullptr};
};

struct RecipeGroup
{
    int_t variantCount = 0;
    RecipeVariant variants[8];
};

struct RecipeCategory
{
    const char *name = nullptr;
    int_t iconItemId = 0;
    int_t iconDamage = 0;
    int_t groupCount = 0;
    RecipeGroup groups[32];
    ItemStack *iconStack = nullptr;
};

static RecipeCategory s_categories[LegacyCraftingScreen::kCategoryCount];
static RecipeCategory s_categories2x2[LegacyCraftingScreen::kCategoryCount];
static bool s_recipesInitialized = false;

inline const RecipeCategory *getCategoriesTable(bool is2x2)
{
    return is2x2 ? s_categories2x2 : s_categories;
}

inline int_t bId(Block *b, int_t fallback)
{
    return b != nullptr ? b->blockID : fallback;
}

inline int_t iId(Item *it, int_t fallback)
{
    return it != nullptr ? it->shiftedIndex : fallback;
}

void initStaticRecipes()
{
    if (s_recipesInitialized)
        return;

    const int_t ID_STONE         = bId(Block::stone, 1);
    const int_t ID_COBBLE        = bId(Block::cobblestone, 4);
    const int_t ID_PLANKS        = bId(Block::planks, 5);
    const int_t ID_SAND          = bId(Block::sand, 12);
    const int_t ID_WOOD          = bId(Block::wood, 17);
    const int_t ID_GLASS         = bId(Block::glass, 20);
    const int_t ID_DISPENSER     = bId(Block::dispenser, 23);
    const int_t ID_SANDSTONE     = bId(Block::sandStone, 24);
    const int_t ID_NOTEBLOCK     = bId(Block::musicBlock, 25);
    const int_t ID_RAIL_POWERED  = bId(Block::railPowered, 27);
    const int_t ID_RAIL_DETECTOR = bId(Block::railDetector, 28);
    const int_t ID_STICKY_PISTON = bId(Block::pistonStickyBase, 29);
    const int_t ID_PISTON        = bId(Block::pistonBase, 33);
    const int_t ID_WOOL          = bId(Block::cloth, 35);
    const int_t ID_BROWN_MUSH    = bId(Block::mushroomBrown, 39);
    const int_t ID_RED_MUSH      = bId(Block::mushroomRed, 40);
    const int_t ID_GOLD_BLOCK    = bId(Block::blockGold, 41);
    const int_t ID_IRON_BLOCK    = bId(Block::blockSteel, 42);
    const int_t ID_SLAB          = bId(Block::stairSingle, 44);
    const int_t ID_BRICK_BLOCK   = bId(Block::brick, 45);
    const int_t ID_TNT           = bId(Block::tnt, 46);
    const int_t ID_BOOKSHELF     = bId(Block::bookShelf, 47);
    const int_t ID_TORCH         = bId(Block::torchWood, 50);
    const int_t ID_STAIR_WOOD    = bId(Block::stairCompactPlanks, 53);
    const int_t ID_CHEST         = bId(Block::chest, 54);
    const int_t ID_DIAMOND_BLOCK = bId(Block::blockDiamond, 57);
    const int_t ID_WORKBENCH     = bId(Block::workbench, 58);
    const int_t ID_FURNACE       = bId(Block::stoneOvenIdle, 61);
    const int_t ID_LADDER        = bId(Block::ladder, 65);
    const int_t ID_RAIL          = bId(Block::rail, 66);
    const int_t ID_STAIR_COBBLE  = bId(Block::stairCompactCobblestone, 67);
    const int_t ID_LEVER         = bId(Block::lever, 69);
    const int_t ID_PLATE_STONE   = bId(Block::pressurePlateStone, 70);
    const int_t ID_PLATE_WOOD    = bId(Block::pressurePlatePlanks, 72);
    const int_t ID_RED_TORCH     = bId(Block::torchRedstoneActive, 76);
    const int_t ID_BUTTON        = bId(Block::button, 77);
    const int_t ID_SNOW_BLOCK    = bId(Block::blockSnow, 80);
    const int_t ID_CLAY_BLOCK    = bId(Block::blockClay, 82);
    const int_t ID_JUKEBOX       = bId(Block::jukebox, 84);
    const int_t ID_FENCE         = bId(Block::fence, 85);
    const int_t ID_GLOWSTONE     = bId(Block::glowStone, 89);
    const int_t ID_TRAPDOOR      = bId(Block::trapdoor, 96);
    const int_t ID_STONE_BRICK   = bId(Block::stoneBrick, 98);
    const int_t ID_IRON_BARS     = bId(Block::fenceIron, 101);
    const int_t ID_GLASS_PANE    = bId(Block::thinGlass, 102);
    const int_t ID_FENCE_GATE    = bId(Block::fenceGate, 107);
    const int_t ID_STAIR_BRICK   = bId(Block::stairsBrick, 108);
    const int_t ID_STAIR_SBRICK  = bId(Block::stairsStoneBrickSmooth, 109);
    const int_t ID_NETHER_BRICK  = bId(Block::netherBrick, 112);
    const int_t ID_NETHER_FENCE  = bId(Block::netherFence, 113);

    const int_t ID_IRON_SHOVEL   = iId(Item::shovelSteel, 256);
    const int_t ID_IRON_PICKAXE  = iId(Item::pickaxeSteel, 257);
    const int_t ID_IRON_AXE      = iId(Item::axeSteel, 258);
    const int_t ID_FLINT_STEEL   = iId(Item::flintAndSteel, 259);
    const int_t ID_APPLE_RED     = iId(Item::appleRed, 260);
    const int_t ID_BOW           = iId(Item::bow, 261);
    const int_t ID_ARROW         = iId(Item::arrow, 262);
    const int_t ID_COAL          = iId(Item::coal, 263);
    const int_t ID_DIAMOND       = iId(Item::diamond, 264);
    const int_t ID_IRON_INGOT    = iId(Item::ingotIron, 265);
    const int_t ID_GOLD_INGOT    = iId(Item::ingotGold, 266);
    const int_t ID_IRON_SWORD    = iId(Item::swordSteel, 267);
    const int_t ID_WOOD_SWORD    = iId(Item::swordWood, 268);
    const int_t ID_WOOD_SHOVEL   = iId(Item::shovelWood, 269);
    const int_t ID_WOOD_PICKAXE  = iId(Item::pickaxeWood, 270);
    const int_t ID_WOOD_AXE      = iId(Item::axeWood, 271);
    const int_t ID_STONE_SWORD   = iId(Item::swordStone, 272);
    const int_t ID_STONE_SHOVEL  = iId(Item::shovelStone, 273);
    const int_t ID_STONE_PICKAXE = iId(Item::pickaxeStone, 274);
    const int_t ID_STONE_AXE     = iId(Item::axeStone, 275);
    const int_t ID_DIAM_SWORD    = iId(Item::swordDiamond, 276);
    const int_t ID_DIAM_SHOVEL   = iId(Item::shovelDiamond, 277);
    const int_t ID_DIAM_PICKAXE  = iId(Item::pickaxeDiamond, 278);
    const int_t ID_DIAM_AXE      = iId(Item::axeDiamond, 279);
    const int_t ID_STICK         = iId(Item::stick, 280);
    const int_t ID_BOWL          = iId(Item::bowlEmpty, 281);
    const int_t ID_STEW          = iId(Item::bowlSoup, 282);
    const int_t ID_GOLD_SWORD    = iId(Item::swordGold, 283);
    const int_t ID_GOLD_SHOVEL   = iId(Item::shovelGold, 284);
    const int_t ID_GOLD_PICKAXE  = iId(Item::pickaxeGold, 285);
    const int_t ID_GOLD_AXE      = iId(Item::axeGold, 286);
    const int_t ID_STRING        = iId(Item::silk, 287);
    const int_t ID_FEATHER       = iId(Item::feather, 288);
    const int_t ID_GUNPOWDER     = iId(Item::gunpowder, 289);
    const int_t ID_WOOD_HOE      = iId(Item::hoeWood, 290);
    const int_t ID_STONE_HOE     = iId(Item::hoeStone, 291);
    const int_t ID_IRON_HOE      = iId(Item::hoeSteel, 292);
    const int_t ID_DIAM_HOE      = iId(Item::hoeDiamond, 293);
    const int_t ID_GOLD_HOE      = iId(Item::hoeGold, 294);
    const int_t ID_WHEAT         = iId(Item::wheat, 296);
    const int_t ID_BREAD         = iId(Item::bread, 297);
    const int_t ID_LEATH_HELMET  = iId(Item::helmetLeather, 298);
    const int_t ID_LEATH_CHEST   = iId(Item::plateLeather, 299);
    const int_t ID_LEATH_LEGS    = iId(Item::legsLeather, 300);
    const int_t ID_LEATH_BOOTS   = iId(Item::bootsLeather, 301);
    const int_t ID_IRON_HELMET   = iId(Item::helmetSteel, 306);
    const int_t ID_IRON_CHEST    = iId(Item::plateSteel, 307);
    const int_t ID_IRON_LEGS     = iId(Item::legsSteel, 308);
    const int_t ID_IRON_BOOTS    = iId(Item::bootsSteel, 309);
    const int_t ID_DIAM_HELMET   = iId(Item::helmetDiamond, 310);
    const int_t ID_DIAM_CHEST    = iId(Item::plateDiamond, 311);
    const int_t ID_DIAM_LEGS     = iId(Item::legsDiamond, 312);
    const int_t ID_DIAM_BOOTS    = iId(Item::bootsDiamond, 313);
    const int_t ID_GOLD_HELMET   = iId(Item::helmetGold, 314);
    const int_t ID_GOLD_CHEST    = iId(Item::plateGold, 315);
    const int_t ID_GOLD_LEGS     = iId(Item::legsGold, 316);
    const int_t ID_GOLD_BOOTS    = iId(Item::bootsGold, 317);
    const int_t ID_FLINT         = iId(Item::flint, 318);
    const int_t ID_PAINTING      = iId(Item::painting, 321);
    const int_t ID_GOLD_APPLE    = iId(Item::appleGold, 322);
    const int_t ID_SIGN          = iId(Item::sign, 323);
    const int_t ID_WOOD_DOOR     = iId(Item::doorWood, 324);
    const int_t ID_BUCKET        = iId(Item::bucketEmpty, 325);
    const int_t ID_MINECART      = iId(Item::minecartEmpty, 328);
    const int_t ID_IRON_DOOR     = iId(Item::doorSteel, 330);
    const int_t ID_REDSTONE      = iId(Item::redstone, 331);
    const int_t ID_SNOWBALL      = iId(Item::snowball, 332);
    const int_t ID_BOAT          = iId(Item::boat, 333);
    const int_t ID_LEATHER       = iId(Item::leather, 334);
    const int_t ID_MILK          = iId(Item::bucketMilk, 335);
    const int_t ID_BRICK_ITEM    = iId(Item::brick, 336);
    const int_t ID_CLAY_BALL     = iId(Item::clay, 337);
    const int_t ID_REED          = iId(Item::reed, 338);
    const int_t ID_PAPER         = iId(Item::paper, 339);
    const int_t ID_BOOK          = iId(Item::book, 340);
    const int_t ID_SLIMEBALL     = iId(Item::slimeBall, 341);
    const int_t ID_CART_CHEST    = iId(Item::minecartCrate, 342);
    const int_t ID_CART_FURNACE  = iId(Item::minecartPowered, 343);
    const int_t ID_EGG           = iId(Item::egg, 344);
    const int_t ID_COMPASS       = iId(Item::compass, 345);
    const int_t ID_FISHING_ROD   = iId(Item::fishingRod, 346);
    const int_t ID_CLOCK         = iId(Item::pocketSundial, 347);
    const int_t ID_GLOW_DUST     = iId(Item::lightStoneDust, 348);
    const int_t ID_SUGAR         = iId(Item::sugar, 353);
    const int_t ID_CAKE          = iId(Item::cake, 354);
    const int_t ID_BED           = iId(Item::bed, 355);
    const int_t ID_REPEATER      = iId(Item::redstoneRepeater, 356);
    const int_t ID_MAP           = iId(Item::mapItem, 358);
    const int_t ID_SHEARS        = iId(Item::shears, 359);
    const int_t ID_GOLD_NUGGET   = iId(Item::goldNugget, 371);

    // ==========================================
    // TAB 0: Structures
    // ==========================================
    RecipeCategory &cat0 = s_categories[0];
    cat0.name = "Structures";
    cat0.iconItemId = ID_PLANKS;
    cat0.iconDamage = 0;
    cat0.groupCount = 8;
    // G0: Planks (4 variants)
    {
        RecipeGroup &g = cat0.groups[0];
        g.variantCount = 4;
        const char *names[4] = {"Oak Planks", "Spruce Planks", "Birch Planks", "Jungle Planks"};
        for (int i = 0; i < 4; ++i)
        {
            RecipeVariant &v = g.variants[i];
            v.name = names[i];
            v.resultId = ID_PLANKS;
            v.resultCount = 4;
            v.resultDamage = i;
            v.requiresWorkbench = false;
            v.gridWidth = 1; v.gridHeight = 1;
            v.gridItemIds[0] = ID_WOOD; v.gridItemDamage[0] = i;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_WOOD, 1, i};
        }
    }

    // G1: Sticks (1 variant)
    {
        RecipeGroup &g = cat0.groups[1];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Sticks";
        v.resultId = ID_STICK;
        v.resultCount = 4;
        v.requiresWorkbench = false;
        v.gridWidth = 1; v.gridHeight = 2;
        v.gridItemIds[0] = ID_PLANKS; v.gridItemIds[1] = ID_PLANKS;
        v.ingredientCount = 1;
        v.ingredients[0] = {ID_PLANKS, 2, -1};
    }

    // G2: Sandstone & Stone Bricks (3 variants)
    {
        RecipeGroup &g = cat0.groups[2];
        g.variantCount = 3;
        // Sandstone
        {
            RecipeVariant &v = g.variants[0];
            v.name = "Sandstone";
            v.resultId = ID_SANDSTONE;
            v.resultCount = 1;
            v.requiresWorkbench = false;
            v.gridWidth = 2; v.gridHeight = 2;
            for (int i = 0; i < 4; ++i) v.gridItemIds[i] = ID_SAND;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_SAND, 4, -1};
        }
        // Smooth Sandstone
        {
            RecipeVariant &v = g.variants[1];
            v.name = "Smooth Sandstone";
            v.resultId = ID_SANDSTONE;
            v.resultCount = 4;
            v.resultDamage = 2;
            v.requiresWorkbench = false;
            v.gridWidth = 2; v.gridHeight = 2;
            for (int i = 0; i < 4; ++i) v.gridItemIds[i] = ID_SANDSTONE;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_SANDSTONE, 4, 0};
        }
        // Stone Bricks
        {
            RecipeVariant &v = g.variants[2];
            v.name = "Stone Bricks";
            v.resultId = ID_STONE_BRICK;
            v.resultCount = 4;
            v.requiresWorkbench = false;
            v.gridWidth = 2; v.gridHeight = 2;
            for (int i = 0; i < 4; ++i) v.gridItemIds[i] = ID_STONE;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_STONE, 4, -1};
        }
    }

    // G3: Slabs (6 variants)
    {
        RecipeGroup &g = cat0.groups[3];
        g.variantCount = 6;
        const char *names[6] = {"Stone Slab", "Sandstone Slab", "Wooden Slab", "Cobblestone Slab", "Brick Slab", "Stone Brick Slab"};
        const int_t mats[6] = {ID_STONE, ID_SANDSTONE, ID_PLANKS, ID_COBBLE, ID_BRICK_BLOCK, ID_STONE_BRICK};
        for (int i = 0; i < 6; ++i)
        {
            RecipeVariant &v = g.variants[i];
            v.name = names[i];
            v.resultId = ID_SLAB;
            v.resultCount = 6;
            v.resultDamage = i;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 1;
            v.gridItemIds[0] = mats[i]; v.gridItemIds[1] = mats[i]; v.gridItemIds[2] = mats[i];
            v.ingredientCount = 1;
            v.ingredients[0] = {mats[i], 3, -1};
        }
    }

    // G4: Stairs (4 variants)
    {
        RecipeGroup &g = cat0.groups[4];
        g.variantCount = 4;
        const char *names[4] = {"Wooden Stairs", "Cobblestone Stairs", "Brick Stairs", "Stone Brick Stairs"};
        const int_t res[4] = {ID_STAIR_WOOD, ID_STAIR_COBBLE, ID_STAIR_BRICK, ID_STAIR_SBRICK};
        const int_t mats[4] = {ID_PLANKS, ID_COBBLE, ID_BRICK_BLOCK, ID_STONE_BRICK};
        for (int i = 0; i < 4; ++i)
        {
            RecipeVariant &v = g.variants[i];
            v.name = names[i];
            v.resultId = res[i];
            v.resultCount = 4;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 3;
            v.gridItemIds[0] = mats[i];
            v.gridItemIds[3] = mats[i]; v.gridItemIds[4] = mats[i];
            v.gridItemIds[6] = mats[i]; v.gridItemIds[7] = mats[i]; v.gridItemIds[8] = mats[i];
            v.ingredientCount = 1;
            v.ingredients[0] = {mats[i], 6, -1};
        }
    }

    // G5: Fences (3 variants)
    {
        RecipeGroup &g = cat0.groups[5];
        g.variantCount = 3;
        // Wood Fence
        {
            RecipeVariant &v = g.variants[0];
            v.name = "Oak Fence";
            v.resultId = ID_FENCE;
            v.resultCount = 2;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 2;
            for (int i = 0; i < 6; ++i) v.gridItemIds[i] = ID_STICK;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_STICK, 6, -1};
        }
        // Nether Fence
        {
            RecipeVariant &v = g.variants[1];
            v.name = "Nether Brick Fence";
            v.resultId = ID_NETHER_FENCE;
            v.resultCount = 6;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 2;
            for (int i = 0; i < 6; ++i) v.gridItemIds[i] = ID_NETHER_BRICK;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_NETHER_BRICK, 6, -1};
        }
        // Fence Gate
        {
            RecipeVariant &v = g.variants[2];
            v.name = "Fence Gate";
            v.resultId = ID_FENCE_GATE;
            v.resultCount = 1;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 2;
            v.gridItemIds[0] = ID_STICK; v.gridItemIds[1] = ID_PLANKS; v.gridItemIds[2] = ID_STICK;
            v.gridItemIds[3] = ID_STICK; v.gridItemIds[4] = ID_PLANKS; v.gridItemIds[5] = ID_STICK;
            v.ingredientCount = 2;
            v.ingredients[0] = {ID_STICK, 4, -1};
            v.ingredients[1] = {ID_PLANKS, 2, -1};
        }
    }

    // G6: Glass Pane & Iron Bars (2 variants)
    {
        RecipeGroup &g = cat0.groups[6];
        g.variantCount = 2;
        // Glass Pane
        {
            RecipeVariant &v = g.variants[0];
            v.name = "Glass Pane";
            v.resultId = ID_GLASS_PANE;
            v.resultCount = 16;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 2;
            for (int i = 0; i < 6; ++i) v.gridItemIds[i] = ID_GLASS;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_GLASS, 6, -1};
        }
        // Iron Bars
        {
            RecipeVariant &v = g.variants[1];
            v.name = "Iron Bars";
            v.resultId = ID_IRON_BARS;
            v.resultCount = 16;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 2;
            for (int i = 0; i < 6; ++i) v.gridItemIds[i] = ID_IRON_INGOT;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_IRON_INGOT, 6, -1};
        }
    }

    // G7: Mineral Blocks & Ingot Unpacking (8 variants)
    {
        RecipeGroup &g = cat0.groups[7];
        g.variantCount = 8;
        // 0: Iron Block
        {
            RecipeVariant &v = g.variants[0];
            v.name = "Block of Iron";
            v.resultId = ID_IRON_BLOCK;
            v.resultCount = 1;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 3;
            for (int i = 0; i < 9; ++i) v.gridItemIds[i] = ID_IRON_INGOT;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_IRON_INGOT, 9, -1};
        }
        // 1: Iron Ingots (from Block)
        {
            RecipeVariant &v = g.variants[1];
            v.name = "Iron Ingots";
            v.resultId = ID_IRON_INGOT;
            v.resultCount = 9;
            v.requiresWorkbench = false;
            v.gridWidth = 1; v.gridHeight = 1;
            v.gridItemIds[0] = ID_IRON_BLOCK;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_IRON_BLOCK, 1, -1};
        }
        // 2: Gold Block
        {
            RecipeVariant &v = g.variants[2];
            v.name = "Block of Gold";
            v.resultId = ID_GOLD_BLOCK;
            v.resultCount = 1;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 3;
            for (int i = 0; i < 9; ++i) v.gridItemIds[i] = ID_GOLD_INGOT;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_GOLD_INGOT, 9, -1};
        }
        // 3: Gold Ingots (from Block)
        {
            RecipeVariant &v = g.variants[3];
            v.name = "Gold Ingots";
            v.resultId = ID_GOLD_INGOT;
            v.resultCount = 9;
            v.requiresWorkbench = false;
            v.gridWidth = 1; v.gridHeight = 1;
            v.gridItemIds[0] = ID_GOLD_BLOCK;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_GOLD_BLOCK, 1, -1};
        }
        // 4: Diamond Block
        {
            RecipeVariant &v = g.variants[4];
            v.name = "Block of Diamond";
            v.resultId = ID_DIAMOND_BLOCK;
            v.resultCount = 1;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 3;
            for (int i = 0; i < 9; ++i) v.gridItemIds[i] = ID_DIAMOND;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_DIAMOND, 9, -1};
        }
        // 5: Diamonds (from Block)
        {
            RecipeVariant &v = g.variants[5];
            v.name = "Diamonds";
            v.resultId = ID_DIAMOND;
            v.resultCount = 9;
            v.requiresWorkbench = false;
            v.gridWidth = 1; v.gridHeight = 1;
            v.gridItemIds[0] = ID_DIAMOND_BLOCK;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_DIAMOND_BLOCK, 1, -1};
        }
        // 6: Glowstone
        {
            RecipeVariant &v = g.variants[6];
            v.name = "Glowstone";
            v.resultId = ID_GLOWSTONE;
            v.resultCount = 1;
            v.requiresWorkbench = false;
            v.gridWidth = 2; v.gridHeight = 2;
            for (int i = 0; i < 4; ++i) v.gridItemIds[i] = ID_GLOW_DUST;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_GLOW_DUST, 4, -1};
        }
        // 7: Clay Block
        {
            RecipeVariant &v = g.variants[7];
            v.name = "Clay Block";
            v.resultId = ID_CLAY_BLOCK;
            v.resultCount = 1;
            v.requiresWorkbench = false;
            v.gridWidth = 2; v.gridHeight = 2;
            for (int i = 0; i < 4; ++i) v.gridItemIds[i] = ID_CLAY_BALL;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_CLAY_BALL, 4, -1};
        }
    }


    // ==========================================
    // TAB 1: Decoration
    // ==========================================
    RecipeCategory &cat1 = s_categories[1];
    cat1.name = "Decoration";
    cat1.iconItemId = ID_PAINTING;
    cat1.iconDamage = 0;
    cat1.groupCount = 8;
    // G0: Crafting Table (1 variant)
    {
        RecipeGroup &g = cat1.groups[0];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Crafting Table";
        v.resultId = ID_WORKBENCH;
        v.resultCount = 1;
        v.requiresWorkbench = false;
        v.gridWidth = 2; v.gridHeight = 2;
        v.gridItemIds[0] = ID_PLANKS; v.gridItemIds[1] = ID_PLANKS;
        v.gridItemIds[2] = ID_PLANKS; v.gridItemIds[3] = ID_PLANKS;
        v.ingredientCount = 1;
        v.ingredients[0] = {ID_PLANKS, 4, -1};
    }

    // G1: Chest (1 variant)
    {
        RecipeGroup &g = cat1.groups[1];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Chest";
        v.resultId = ID_CHEST;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 3;
        for (int i = 0; i < 9; ++i) v.gridItemIds[i] = (i == 4) ? 0 : ID_PLANKS;
        v.ingredientCount = 1;
        v.ingredients[0] = {ID_PLANKS, 8, -1};
    }

    // G2: Furnace (1 variant)
    {
        RecipeGroup &g = cat1.groups[2];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Furnace";
        v.resultId = ID_FURNACE;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 3;
        for (int i = 0; i < 9; ++i) v.gridItemIds[i] = (i == 4) ? 0 : ID_COBBLE;
        v.ingredientCount = 1;
        v.ingredients[0] = {ID_COBBLE, 8, -1};
    }

    // G3: Bed
    {
        RecipeGroup &g = cat1.groups[3];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Bed";
        v.resultId = ID_BED;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 2;
        v.gridItemIds[0] = ID_WOOL;   v.gridItemIds[1] = ID_WOOL;   v.gridItemIds[2] = ID_WOOL;
        v.gridItemIds[3] = ID_PLANKS; v.gridItemIds[4] = ID_PLANKS; v.gridItemIds[5] = ID_PLANKS;
        v.ingredientCount = 2;
        v.ingredients[0] = {ID_WOOL, 3, -1};
        v.ingredients[1] = {ID_PLANKS, 3, -1};
    }
    // G4: Torches (2 variants)
    {
        RecipeGroup &g = cat1.groups[4];
        g.variantCount = 2;
        const char *names[2] = {"Torch (Coal)", "Torch (Charcoal)"};
        for (int i = 0; i < 2; ++i)
        {
            RecipeVariant &v = g.variants[i];
            v.name = names[i];
            v.resultId = ID_TORCH;
            v.resultCount = 4;
            v.requiresWorkbench = false;
            v.gridWidth = 1; v.gridHeight = 2;
            v.gridItemIds[0] = ID_COAL; v.gridItemDamage[0] = i;
            v.gridItemIds[1] = ID_STICK;
            v.ingredientCount = 2;
            v.ingredients[0] = {ID_COAL, 1, i};
            v.ingredients[1] = {ID_STICK, 1, -1};
        }
    }

    // G5: Ladder (1 variant)
    {
        RecipeGroup &g = cat1.groups[5];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Ladder";
        v.resultId = ID_LADDER;
        v.resultCount = 3;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 3;
        v.gridItemIds[0] = ID_STICK; v.gridItemIds[2] = ID_STICK;
        v.gridItemIds[3] = ID_STICK; v.gridItemIds[4] = ID_STICK; v.gridItemIds[5] = ID_STICK;
        v.gridItemIds[6] = ID_STICK; v.gridItemIds[8] = ID_STICK;
        v.ingredientCount = 1;
        v.ingredients[0] = {ID_STICK, 7, -1};
    }

    // G6: Bookshelf (1 variant)
    {
        RecipeGroup &g = cat1.groups[6];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Bookshelf";
        v.resultId = ID_BOOKSHELF;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 3;
        for (int i = 0; i < 3; ++i) v.gridItemIds[i] = ID_PLANKS;
        for (int i = 3; i < 6; ++i) v.gridItemIds[i] = ID_BOOK;
        for (int i = 6; i < 9; ++i) v.gridItemIds[i] = ID_PLANKS;
        v.ingredientCount = 2;
        v.ingredients[0] = {ID_PLANKS, 6, -1};
        v.ingredients[1] = {ID_BOOK, 3, -1};
    }

    // G7: Painting
    {
        RecipeGroup &g = cat1.groups[7];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Painting";
        v.resultId = ID_PAINTING;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 3;
        for (int i = 0; i < 9; ++i) v.gridItemIds[i] = (i == 4) ? ID_WOOL : ID_STICK;
        v.ingredientCount = 2;
        v.ingredients[0] = {ID_STICK, 8, -1};
        v.ingredients[1] = {ID_WOOL, 1, -1};
    }

    // ==========================================
    // TAB 2: Tools & Armor
    // ==========================================
    RecipeCategory &cat2 = s_categories[2];
    cat2.name = "Tools & Armor";
    cat2.iconItemId = ID_IRON_PICKAXE;
    cat2.iconDamage = 0;
    cat2.groupCount = 10;
    auto setup5TierTool = [&](RecipeGroup &g, const char *toolNames[5], const int_t resIds[5],
                              int_t s0, int_t s1, int_t s2, int_t s3, int_t s4, int_t s5, int_t s6, int_t s7, int_t s8,
                              int_t matCount)
    {
        g.variantCount = 5;
        const int_t mats[5] = {ID_PLANKS, ID_COBBLE, ID_IRON_INGOT, ID_GOLD_INGOT, ID_DIAMOND};
        for (int i = 0; i < 5; ++i)
        {
            RecipeVariant &v = g.variants[i];
            v.name = toolNames[i];
            v.resultId = resIds[i];
            v.resultCount = 1;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 3;
            const int_t templateGrid[9] = {s0, s1, s2, s3, s4, s5, s6, s7, s8};
            for (int k = 0; k < 9; ++k)
            {
                if (templateGrid[k] == 1) v.gridItemIds[k] = mats[i];
                else if (templateGrid[k] == 2) v.gridItemIds[k] = ID_STICK;
                else v.gridItemIds[k] = 0;
            }
            v.ingredientCount = 2;
            v.ingredients[0] = {mats[i], matCount, -1};
            v.ingredients[1] = {ID_STICK, (s7 == 2 && s4 == 2) ? 2 : 1, -1};
        }
    };
    auto setupArmor = [&](RecipeGroup &g, const char *names[4], const int_t resIds[4],
                          int_t s0, int_t s1, int_t s2, int_t s3, int_t s4, int_t s5, int_t s6, int_t s7, int_t s8,
                          int_t matCount)
    {
        g.variantCount = 4;
        const int_t mats[4] = {ID_LEATHER, ID_IRON_INGOT, ID_GOLD_INGOT, ID_DIAMOND};
        for (int i = 0; i < 4; ++i)
        {
            RecipeVariant &v = g.variants[i];
            v.name = names[i];
            v.resultId = resIds[i];
            v.resultCount = 1;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 3;
            const int_t templateGrid[9] = {s0, s1, s2, s3, s4, s5, s6, s7, s8};
            for (int k = 0; k < 9; ++k)
                v.gridItemIds[k] = (templateGrid[k] == 1) ? mats[i] : 0;
            v.ingredientCount = 1;
            v.ingredients[0] = {mats[i], matCount, -1};
        }
    };
    // G0: Pickaxes
    {
        const char *names[5] = {"Wooden Pickaxe", "Stone Pickaxe", "Iron Pickaxe", "Golden Pickaxe", "Diamond Pickaxe"};
        const int_t res[5] = {ID_WOOD_PICKAXE, ID_STONE_PICKAXE, ID_IRON_PICKAXE, ID_GOLD_PICKAXE, ID_DIAM_PICKAXE};
        setup5TierTool(cat2.groups[0], names, res, 1, 1, 1, 0, 2, 0, 0, 2, 0, 3);
    }
    // G1: Shovels
    {
        const char *names[5] = {"Wooden Shovel", "Stone Shovel", "Iron Shovel", "Golden Shovel", "Diamond Shovel"};
        const int_t res[5] = {ID_WOOD_SHOVEL, ID_STONE_SHOVEL, ID_IRON_SHOVEL, ID_GOLD_SHOVEL, ID_DIAM_SHOVEL};
        setup5TierTool(cat2.groups[1], names, res, 0, 1, 0, 0, 2, 0, 0, 2, 0, 1);
    }
    // G2: Axes
    {
        const char *names[5] = {"Wooden Axe", "Stone Axe", "Iron Axe", "Golden Axe", "Diamond Axe"};
        const int_t res[5] = {ID_WOOD_AXE, ID_STONE_AXE, ID_IRON_AXE, ID_GOLD_AXE, ID_DIAM_AXE};
        setup5TierTool(cat2.groups[2], names, res, 1, 1, 0, 1, 2, 0, 0, 2, 0, 3);
    }
    // G3: Hoes
    {
        const char *names[5] = {"Wooden Hoe", "Stone Hoe", "Iron Hoe", "Golden Hoe", "Diamond Hoe"};
        const int_t res[5] = {ID_WOOD_HOE, ID_STONE_HOE, ID_IRON_HOE, ID_GOLD_HOE, ID_DIAM_HOE};
        setup5TierTool(cat2.groups[3], names, res, 1, 1, 0, 0, 2, 0, 0, 2, 0, 2);
    }
    // G4: Swords
    {
        const char *names[5] = {"Wooden Sword", "Stone Sword", "Iron Sword", "Golden Sword", "Diamond Sword"};
        const int_t res[5] = {ID_WOOD_SWORD, ID_STONE_SWORD, ID_IRON_SWORD, ID_GOLD_SWORD, ID_DIAM_SWORD};
        setup5TierTool(cat2.groups[4], names, res, 0, 1, 0, 0, 1, 0, 0, 2, 0, 2);
    }

    // G5: Bow & Arrows (2 variants)
    {
        RecipeGroup &g = cat2.groups[5];
        g.variantCount = 2;
        // Bow
        {
            RecipeVariant &v = g.variants[0];
            v.name = "Bow";
            v.resultId = ID_BOW;
            v.resultCount = 1;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 3;
            v.gridItemIds[0] = 0;        v.gridItemIds[1] = ID_STICK; v.gridItemIds[2] = ID_STRING;
            v.gridItemIds[3] = ID_STICK; v.gridItemIds[4] = 0;        v.gridItemIds[5] = ID_STRING;
            v.gridItemIds[6] = 0;        v.gridItemIds[7] = ID_STICK; v.gridItemIds[8] = ID_STRING;
            v.ingredientCount = 2;
            v.ingredients[0] = {ID_STICK, 3, -1};
            v.ingredients[1] = {ID_STRING, 3, -1};
        }
        // Arrows
        {
            RecipeVariant &v = g.variants[1];
            v.name = "Arrows";
            v.resultId = ID_ARROW;
            v.resultCount = 4;
            v.requiresWorkbench = true;
            v.gridWidth = 1; v.gridHeight = 3;
            v.gridItemIds[0] = ID_FLINT;
            v.gridItemIds[1] = ID_STICK;
            v.gridItemIds[2] = ID_FEATHER;
            v.ingredientCount = 3;
            v.ingredients[0] = {ID_FLINT, 1, -1};
            v.ingredients[1] = {ID_STICK, 1, -1};
            v.ingredients[2] = {ID_FEATHER, 1, -1};
        }
    }


    // G6: Helmets
    {
        const char *names[4] = {"Leather Cap", "Iron Helmet", "Golden Helmet", "Diamond Helmet"};
        const int_t res[4] = {ID_LEATH_HELMET, ID_IRON_HELMET, ID_GOLD_HELMET, ID_DIAM_HELMET};
        setupArmor(cat2.groups[6], names, res, 1, 1, 1, 1, 0, 1, 0, 0, 0, 5);
    }
    // G7: Chestplates
    {
        const char *names[4] = {"Leather Tunic", "Iron Chestplate", "Golden Chestplate", "Diamond Chestplate"};
        const int_t res[4] = {ID_LEATH_CHEST, ID_IRON_CHEST, ID_GOLD_CHEST, ID_DIAM_CHEST};
        setupArmor(cat2.groups[7], names, res, 1, 0, 1, 1, 1, 1, 1, 1, 1, 8);
    }
    // G8: Leggings
    {
        const char *names[4] = {"Leather Pants", "Iron Leggings", "Golden Leggings", "Diamond Leggings"};
        const int_t res[4] = {ID_LEATH_LEGS, ID_IRON_LEGS, ID_GOLD_LEGS, ID_DIAM_LEGS};
        setupArmor(cat2.groups[8], names, res, 1, 1, 1, 1, 0, 1, 1, 0, 1, 7);
    }
    // G9: Boots
    {
        const char *names[4] = {"Leather Boots", "Iron Boots", "Golden Boots", "Diamond Boots"};
        const int_t res[4] = {ID_LEATH_BOOTS, ID_IRON_BOOTS, ID_GOLD_BOOTS, ID_DIAM_BOOTS};
        setupArmor(cat2.groups[9], names, res, 0, 0, 0, 1, 0, 1, 1, 0, 1, 4);
    }


    // ==========================================
    // TAB 3: Mechanisms
    // ==========================================
    RecipeCategory &cat3 = s_categories[3];
    cat3.name = "Mechanisms";
    cat3.iconItemId = ID_MINECART;
    cat3.iconDamage = 0;
    cat3.groupCount = 12;
    // G0: Doors & Trapdoors (3 variants)
    {
        RecipeGroup &g = cat3.groups[0];
        g.variantCount = 3;
        // Wooden Door
        {
            RecipeVariant &v = g.variants[0];
            v.name = "Wooden Door";
            v.resultId = ID_WOOD_DOOR;
            v.resultCount = 1;
            v.requiresWorkbench = true;
            v.gridWidth = 2; v.gridHeight = 3;
            v.gridItemIds[0] = ID_PLANKS; v.gridItemIds[1] = ID_PLANKS;
            v.gridItemIds[3] = ID_PLANKS; v.gridItemIds[4] = ID_PLANKS;
            v.gridItemIds[6] = ID_PLANKS; v.gridItemIds[7] = ID_PLANKS;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_PLANKS, 6, -1};
        }
        // Iron Door
        {
            RecipeVariant &v = g.variants[1];
            v.name = "Iron Door";
            v.resultId = ID_IRON_DOOR;
            v.resultCount = 1;
            v.requiresWorkbench = true;
            v.gridWidth = 2; v.gridHeight = 3;
            v.gridItemIds[0] = ID_IRON_INGOT; v.gridItemIds[1] = ID_IRON_INGOT;
            v.gridItemIds[3] = ID_IRON_INGOT; v.gridItemIds[4] = ID_IRON_INGOT;
            v.gridItemIds[6] = ID_IRON_INGOT; v.gridItemIds[7] = ID_IRON_INGOT;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_IRON_INGOT, 6, -1};
        }
        // Trapdoor
        {
            RecipeVariant &v = g.variants[2];
            v.name = "Trapdoor";
            v.resultId = ID_TRAPDOOR;
            v.resultCount = 2;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 2;
            for (int i = 0; i < 6; ++i) v.gridItemIds[i] = ID_PLANKS;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_PLANKS, 6, -1};
        }
    }

    // G1: Pressure Plates (2 variants)
    {
        RecipeGroup &g = cat3.groups[1];
        g.variantCount = 2;
        // Stone Plate
        {
            RecipeVariant &v = g.variants[0];
            v.name = "Stone Pressure Plate";
            v.resultId = ID_PLATE_STONE;
            v.resultCount = 1;
            v.requiresWorkbench = false;
            v.gridWidth = 2; v.gridHeight = 1;
            v.gridItemIds[0] = ID_STONE; v.gridItemIds[1] = ID_STONE;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_STONE, 2, -1};
        }
        // Wood Plate
        {
            RecipeVariant &v = g.variants[1];
            v.name = "Wooden Pressure Plate";
            v.resultId = ID_PLATE_WOOD;
            v.resultCount = 1;
            v.requiresWorkbench = false;
            v.gridWidth = 2; v.gridHeight = 1;
            v.gridItemIds[0] = ID_PLANKS; v.gridItemIds[1] = ID_PLANKS;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_PLANKS, 2, -1};
        }
    }
    // G2: Button
    {
        RecipeGroup &g = cat3.groups[2];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Button";
        v.resultId = ID_BUTTON;
        v.resultCount = 1;
        v.requiresWorkbench = false;
        v.gridWidth = 1; v.gridHeight = 2;
        v.gridItemIds[0] = ID_STONE; v.gridItemIds[1] = ID_STONE;
        v.ingredientCount = 1;
        v.ingredients[0] = {ID_STONE, 2, -1};
    }
    // G3: Lever
    {
        RecipeGroup &g = cat3.groups[3];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Lever";
        v.resultId = ID_LEVER;
        v.resultCount = 1;
        v.requiresWorkbench = false;
        v.gridWidth = 1; v.gridHeight = 2;
        v.gridItemIds[0] = ID_STICK; v.gridItemIds[1] = ID_COBBLE;
        v.ingredientCount = 2;
        v.ingredients[0] = {ID_STICK, 1, -1};
        v.ingredients[1] = {ID_COBBLE, 1, -1};
    }
    // G4: Redstone Torch
    {
        RecipeGroup &g = cat3.groups[4];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Redstone Torch";
        v.resultId = ID_RED_TORCH;
        v.resultCount = 1;
        v.requiresWorkbench = false;
        v.gridWidth = 1; v.gridHeight = 2;
        v.gridItemIds[0] = ID_REDSTONE; v.gridItemIds[1] = ID_STICK;
        v.ingredientCount = 2;
        v.ingredients[0] = {ID_REDSTONE, 1, -1};
        v.ingredients[1] = {ID_STICK, 1, -1};
    }
    // G5: Redstone Repeater
    {
        RecipeGroup &g = cat3.groups[5];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Redstone Repeater";
        v.resultId = ID_REPEATER;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 2;
        v.gridItemIds[0] = ID_RED_TORCH; v.gridItemIds[1] = ID_REDSTONE; v.gridItemIds[2] = ID_RED_TORCH;
        v.gridItemIds[3] = ID_STONE;     v.gridItemIds[4] = ID_STONE;    v.gridItemIds[5] = ID_STONE;
        v.ingredientCount = 3;
        v.ingredients[0] = {ID_RED_TORCH, 2, -1};
        v.ingredients[1] = {ID_REDSTONE, 1, -1};
        v.ingredients[2] = {ID_STONE, 3, -1};
    }
    // G6: Pistons (2 variants)
    {
        RecipeGroup &g = cat3.groups[6];
        g.variantCount = 2;
        // Piston
        {
            RecipeVariant &v = g.variants[0];
            v.name = "Piston";
            v.resultId = ID_PISTON;
            v.resultCount = 1;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 3;
            v.gridItemIds[0] = ID_PLANKS; v.gridItemIds[1] = ID_PLANKS;     v.gridItemIds[2] = ID_PLANKS;
            v.gridItemIds[3] = ID_COBBLE; v.gridItemIds[4] = ID_IRON_INGOT; v.gridItemIds[5] = ID_COBBLE;
            v.gridItemIds[6] = ID_COBBLE; v.gridItemIds[7] = ID_REDSTONE;   v.gridItemIds[8] = ID_COBBLE;
            v.ingredientCount = 4;
            v.ingredients[0] = {ID_PLANKS, 3, -1};
            v.ingredients[1] = {ID_COBBLE, 4, -1};
            v.ingredients[2] = {ID_IRON_INGOT, 1, -1};
            v.ingredients[3] = {ID_REDSTONE, 1, -1};
        }
        // Sticky Piston
        {
            RecipeVariant &v = g.variants[1];
            v.name = "Sticky Piston";
            v.resultId = ID_STICKY_PISTON;
            v.resultCount = 1;
            v.requiresWorkbench = false;
            v.gridWidth = 1; v.gridHeight = 2;
            v.gridItemIds[0] = ID_SLIMEBALL; v.gridItemIds[1] = ID_PISTON;
            v.ingredientCount = 2;
            v.ingredients[0] = {ID_SLIMEBALL, 1, -1};
            v.ingredients[1] = {ID_PISTON, 1, -1};
        }
    }
    // G7: Dispenser
    {
        RecipeGroup &g = cat3.groups[7];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Dispenser";
        v.resultId = ID_DISPENSER;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 3;
        v.gridItemIds[0] = ID_COBBLE; v.gridItemIds[1] = ID_COBBLE; v.gridItemIds[2] = ID_COBBLE;
        v.gridItemIds[3] = ID_COBBLE; v.gridItemIds[4] = ID_BOW;    v.gridItemIds[5] = ID_COBBLE;
        v.gridItemIds[6] = ID_COBBLE; v.gridItemIds[7] = ID_REDSTONE; v.gridItemIds[8] = ID_COBBLE;
        v.ingredientCount = 3;
        v.ingredients[0] = {ID_COBBLE, 7, -1};
        v.ingredients[1] = {ID_BOW, 1, -1};
        v.ingredients[2] = {ID_REDSTONE, 1, -1};
    }
    // G8: TNT
    {
        RecipeGroup &g = cat3.groups[8];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "TNT";
        v.resultId = ID_TNT;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 3;
        v.gridItemIds[0] = ID_GUNPOWDER; v.gridItemIds[1] = ID_SAND;      v.gridItemIds[2] = ID_GUNPOWDER;
        v.gridItemIds[3] = ID_SAND;      v.gridItemIds[4] = ID_GUNPOWDER; v.gridItemIds[5] = ID_SAND;
        v.gridItemIds[6] = ID_GUNPOWDER; v.gridItemIds[7] = ID_SAND;      v.gridItemIds[8] = ID_GUNPOWDER;
        v.ingredientCount = 2;
        v.ingredients[0] = {ID_GUNPOWDER, 5, -1};
        v.ingredients[1] = {ID_SAND, 4, -1};
    }
    // G9: Note Block & Jukebox (2 variants)
    {
        RecipeGroup &g = cat3.groups[9];
        g.variantCount = 2;
        // Note Block
        {
            RecipeVariant &v = g.variants[0];
            v.name = "Note Block";
            v.resultId = ID_NOTEBLOCK;
            v.resultCount = 1;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 3;
            for (int i = 0; i < 9; ++i) v.gridItemIds[i] = (i == 4) ? ID_REDSTONE : ID_PLANKS;
            v.ingredientCount = 2;
            v.ingredients[0] = {ID_PLANKS, 8, -1};
            v.ingredients[1] = {ID_REDSTONE, 1, -1};
        }
        // Jukebox
        {
            RecipeVariant &v = g.variants[1];
            v.name = "Jukebox";
            v.resultId = ID_JUKEBOX;
            v.resultCount = 1;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 3;
            for (int i = 0; i < 9; ++i) v.gridItemIds[i] = (i == 4) ? ID_DIAMOND : ID_PLANKS;
            v.ingredientCount = 2;
            v.ingredients[0] = {ID_PLANKS, 8, -1};
            v.ingredients[1] = {ID_DIAMOND, 1, -1};
        }
    }

        // G10: Rails (3 variants)
    {
        RecipeGroup &g = cat3.groups[10];
        g.variantCount = 3;
        // Standard Rail
        {
            RecipeVariant &v = g.variants[0];
            v.name = "Rail";
            v.resultId = ID_RAIL;
            v.resultCount = 16;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 3;
            v.gridItemIds[0] = ID_IRON_INGOT; v.gridItemIds[2] = ID_IRON_INGOT;
            v.gridItemIds[3] = ID_IRON_INGOT; v.gridItemIds[4] = ID_STICK; v.gridItemIds[5] = ID_IRON_INGOT;
            v.gridItemIds[6] = ID_IRON_INGOT; v.gridItemIds[8] = ID_IRON_INGOT;
            v.ingredientCount = 2;
            v.ingredients[0] = {ID_IRON_INGOT, 6, -1};
            v.ingredients[1] = {ID_STICK, 1, -1};
        }
        // Powered Rail
        {
            RecipeVariant &v = g.variants[1];
            v.name = "Powered Rail";
            v.resultId = ID_RAIL_POWERED;
            v.resultCount = 6;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 3;
            v.gridItemIds[0] = ID_GOLD_INGOT; v.gridItemIds[2] = ID_GOLD_INGOT;
            v.gridItemIds[3] = ID_GOLD_INGOT; v.gridItemIds[4] = ID_STICK; v.gridItemIds[5] = ID_GOLD_INGOT;
            v.gridItemIds[6] = ID_GOLD_INGOT; v.gridItemIds[7] = ID_REDSTONE; v.gridItemIds[8] = ID_GOLD_INGOT;
            v.ingredientCount = 3;
            v.ingredients[0] = {ID_GOLD_INGOT, 6, -1};
            v.ingredients[1] = {ID_STICK, 1, -1};
            v.ingredients[2] = {ID_REDSTONE, 1, -1};
        }
        // Detector Rail
        {
            RecipeVariant &v = g.variants[2];
            v.name = "Detector Rail";
            v.resultId = ID_RAIL_DETECTOR;
            v.resultCount = 6;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 3;
            v.gridItemIds[0] = ID_IRON_INGOT; v.gridItemIds[2] = ID_IRON_INGOT;
            v.gridItemIds[3] = ID_IRON_INGOT; v.gridItemIds[4] = ID_PLATE_STONE; v.gridItemIds[5] = ID_IRON_INGOT;
            v.gridItemIds[6] = ID_IRON_INGOT; v.gridItemIds[7] = ID_REDSTONE; v.gridItemIds[8] = ID_IRON_INGOT;
            v.ingredientCount = 3;
            v.ingredients[0] = {ID_IRON_INGOT, 6, -1};
            v.ingredients[1] = {ID_PLATE_STONE, 1, -1};
            v.ingredients[2] = {ID_REDSTONE, 1, -1};
        }
    }
    // G11: Minecarts (3 variants)
    {
        RecipeGroup &g = cat3.groups[11];
        g.variantCount = 3;
        // Minecart
        {
            RecipeVariant &v = g.variants[0];
            v.name = "Minecart";
            v.resultId = ID_MINECART;
            v.resultCount = 1;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 2;
            v.gridItemIds[0] = ID_IRON_INGOT; v.gridItemIds[2] = ID_IRON_INGOT;
            v.gridItemIds[3] = ID_IRON_INGOT; v.gridItemIds[4] = ID_IRON_INGOT; v.gridItemIds[5] = ID_IRON_INGOT;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_IRON_INGOT, 5, -1};
        }
        // Powered Minecart
        {
            RecipeVariant &v = g.variants[1];
            v.name = "Powered Minecart";
            v.resultId = ID_CART_FURNACE;
            v.resultCount = 1;
            v.requiresWorkbench = false;
            v.gridWidth = 1; v.gridHeight = 2;
            v.gridItemIds[0] = ID_FURNACE; v.gridItemIds[1] = ID_MINECART;
            v.ingredientCount = 2;
            v.ingredients[0] = {ID_FURNACE, 1, -1};
            v.ingredients[1] = {ID_MINECART, 1, -1};
        }
        // Storage Minecart
        {
            RecipeVariant &v = g.variants[2];
            v.name = "Storage Minecart";
            v.resultId = ID_CART_CHEST;
            v.resultCount = 1;
            v.requiresWorkbench = false;
            v.gridWidth = 1; v.gridHeight = 2;
            v.gridItemIds[0] = ID_CHEST; v.gridItemIds[1] = ID_MINECART;
            v.ingredientCount = 2;
            v.ingredients[0] = {ID_CHEST, 1, -1};
            v.ingredients[1] = {ID_MINECART, 1, -1};
        }
    }

    // ==========================================
    // TAB 4: Food & Misc
    // ==========================================
    RecipeCategory &cat4 = s_categories[4];
    cat4.name = "Food & Misc";
    cat4.iconItemId = ID_APPLE_RED;
    cat4.iconDamage = 0;
    cat4.groupCount = 15;
    // G0: Bread
    {
        RecipeGroup &g = cat4.groups[0];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Bread";
        v.resultId = ID_BREAD;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 1;
        v.gridItemIds[0] = ID_WHEAT; v.gridItemIds[1] = ID_WHEAT; v.gridItemIds[2] = ID_WHEAT;
        v.ingredientCount = 1;
        v.ingredients[0] = {ID_WHEAT, 3, -1};
    }
    // G1: Cake
    {
        RecipeGroup &g = cat4.groups[1];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Cake";
        v.resultId = ID_CAKE;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 3;
        v.gridItemIds[0] = ID_MILK;  v.gridItemIds[1] = ID_MILK;  v.gridItemIds[2] = ID_MILK;
        v.gridItemIds[3] = ID_SUGAR; v.gridItemIds[4] = ID_EGG;   v.gridItemIds[5] = ID_SUGAR;
        v.gridItemIds[6] = ID_WHEAT; v.gridItemIds[7] = ID_WHEAT; v.gridItemIds[8] = ID_WHEAT;
        v.ingredientCount = 4;
        v.ingredients[0] = {ID_MILK, 3, -1};
        v.ingredients[1] = {ID_SUGAR, 2, -1};
        v.ingredients[2] = {ID_EGG, 1, -1};
        v.ingredients[3] = {ID_WHEAT, 3, -1};
    }
    // G2: Golden Apple
    {
        RecipeGroup &g = cat4.groups[2];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Golden Apple";
        v.resultId = ID_GOLD_APPLE;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 3;
        for (int i = 0; i < 9; ++i) v.gridItemIds[i] = (i == 4) ? ID_APPLE_RED : ID_GOLD_NUGGET;
        v.ingredientCount = 2;
        v.ingredients[0] = {ID_APPLE_RED, 1, -1};
        v.ingredients[1] = {ID_GOLD_NUGGET, 8, -1};
    }
    // G3: Bowl & Mushroom Stew
    {
        RecipeGroup &g = cat4.groups[3];
        g.variantCount = 2;
        // Bowl
        {
            RecipeVariant &v = g.variants[0];
            v.name = "Bowl";
            v.resultId = ID_BOWL;
            v.resultCount = 4;
            v.requiresWorkbench = false;
            v.gridWidth = 3; v.gridHeight = 2;
            v.gridItemIds[0] = ID_PLANKS; v.gridItemIds[2] = ID_PLANKS;
            v.gridItemIds[4] = ID_PLANKS;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_PLANKS, 3, -1};
        }
        // Stew
        {
            RecipeVariant &v = g.variants[1];
            v.name = "Mushroom Stew";
            v.resultId = ID_STEW;
            v.resultCount = 1;
            v.requiresWorkbench = false;
            v.gridWidth = 2; v.gridHeight = 2;
            v.gridItemIds[0] = ID_RED_MUSH;   v.gridItemIds[1] = ID_BROWN_MUSH;
            v.gridItemIds[2] = ID_BOWL;
            v.ingredientCount = 3;
            v.ingredients[0] = {ID_BOWL, 1, -1};
            v.ingredients[1] = {ID_RED_MUSH, 1, -1};
            v.ingredients[2] = {ID_BROWN_MUSH, 1, -1};
        }
    }
    // G4: Sugar
    {
        RecipeGroup &g = cat4.groups[4];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Sugar";
        v.resultId = ID_SUGAR;
        v.resultCount = 1;
        v.requiresWorkbench = false;
        v.gridWidth = 1; v.gridHeight = 1;
        v.gridItemIds[0] = ID_REED;
        v.ingredientCount = 1;
        v.ingredients[0] = {ID_REED, 1, -1};
    }
    // G5: Flint and Steel (1 variant)
    {
        RecipeGroup &g = cat4.groups[5];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Flint and Steel";
        v.resultId = ID_FLINT_STEEL;
        v.resultCount = 1;
        v.requiresWorkbench = false;
        v.gridWidth = 2; v.gridHeight = 2;
        v.gridItemIds[0] = ID_IRON_INGOT; v.gridItemIds[3] = ID_FLINT;
        v.ingredientCount = 2;
        v.ingredients[0] = {ID_IRON_INGOT, 1, -1};
        v.ingredients[1] = {ID_FLINT, 1, -1};
    }

    // G6: Shears (1 variant)
    {
        RecipeGroup &g = cat4.groups[6];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Shears";
        v.resultId = ID_SHEARS;
        v.resultCount = 1;
        v.requiresWorkbench = false;
        v.gridWidth = 2; v.gridHeight = 2;
        v.gridItemIds[0] = ID_IRON_INGOT; v.gridItemIds[3] = ID_IRON_INGOT;
        v.ingredientCount = 1;
        v.ingredients[0] = {ID_IRON_INGOT, 2, -1};
    }

    // G7: Fishing Rod (1 variant)
    {
        RecipeGroup &g = cat4.groups[7];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Fishing Rod";
        v.resultId = ID_FISHING_ROD;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 3;
        v.gridItemIds[2] = ID_STICK;
        v.gridItemIds[4] = ID_STICK; v.gridItemIds[5] = ID_STRING;
        v.gridItemIds[6] = ID_STICK; v.gridItemIds[8] = ID_STRING;
        v.ingredientCount = 2;
        v.ingredients[0] = {ID_STICK, 3, -1};
        v.ingredients[1] = {ID_STRING, 2, -1};
    }

        // G8: Bucket
    {
        RecipeGroup &g = cat4.groups[8];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Bucket";
        v.resultId = ID_BUCKET;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 2;
        v.gridItemIds[0] = ID_IRON_INGOT; v.gridItemIds[2] = ID_IRON_INGOT;
        v.gridItemIds[4] = ID_IRON_INGOT;
        v.ingredientCount = 1;
        v.ingredients[0] = {ID_IRON_INGOT, 3, -1};
    }
    // G9: Compass
    {
        RecipeGroup &g = cat4.groups[9];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Compass";
        v.resultId = ID_COMPASS;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 3;
        v.gridItemIds[1] = ID_IRON_INGOT;
        v.gridItemIds[3] = ID_IRON_INGOT; v.gridItemIds[4] = ID_REDSTONE; v.gridItemIds[5] = ID_IRON_INGOT;
        v.gridItemIds[7] = ID_IRON_INGOT;
        v.ingredientCount = 2;
        v.ingredients[0] = {ID_IRON_INGOT, 4, -1};
        v.ingredients[1] = {ID_REDSTONE, 1, -1};
    }
    // G10: Clock
    {
        RecipeGroup &g = cat4.groups[10];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Clock";
        v.resultId = ID_CLOCK;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 3;
        v.gridItemIds[1] = ID_GOLD_INGOT;
        v.gridItemIds[3] = ID_GOLD_INGOT; v.gridItemIds[4] = ID_REDSTONE; v.gridItemIds[5] = ID_GOLD_INGOT;
        v.gridItemIds[7] = ID_GOLD_INGOT;
        v.ingredientCount = 2;
        v.ingredients[0] = {ID_GOLD_INGOT, 4, -1};
        v.ingredients[1] = {ID_REDSTONE, 1, -1};
    }
    // G11: Map
    {
        RecipeGroup &g = cat4.groups[11];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Map";
        v.resultId = ID_MAP;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 3;
        for (int i = 0; i < 9; ++i) v.gridItemIds[i] = (i == 4) ? ID_COMPASS : ID_PAPER;
        v.ingredientCount = 2;
        v.ingredients[0] = {ID_PAPER, 8, -1};
        v.ingredients[1] = {ID_COMPASS, 1, -1};
    }
    // G12: Paper & Book (2 variants)
    {
        RecipeGroup &g = cat4.groups[12];
        g.variantCount = 2;
        // Paper
        {
            RecipeVariant &v = g.variants[0];
            v.name = "Paper";
            v.resultId = ID_PAPER;
            v.resultCount = 3;
            v.requiresWorkbench = true;
            v.gridWidth = 3; v.gridHeight = 1;
            v.gridItemIds[0] = ID_REED; v.gridItemIds[1] = ID_REED; v.gridItemIds[2] = ID_REED;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_REED, 3, -1};
        }
        // Book
        {
            RecipeVariant &v = g.variants[1];
            v.name = "Book";
            v.resultId = ID_BOOK;
            v.resultCount = 1;
            v.requiresWorkbench = true;
            v.gridWidth = 1; v.gridHeight = 3;
            v.gridItemIds[0] = ID_PAPER; v.gridItemIds[1] = ID_PAPER; v.gridItemIds[2] = ID_PAPER;
            v.ingredientCount = 1;
            v.ingredients[0] = {ID_PAPER, 3, -1};
        }
    }
    // G13: Boat
    {
        RecipeGroup &g = cat4.groups[13];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Boat";
        v.resultId = ID_BOAT;
        v.resultCount = 1;
        v.requiresWorkbench = true;
        v.gridWidth = 3; v.gridHeight = 2;
        v.gridItemIds[0] = ID_PLANKS; v.gridItemIds[2] = ID_PLANKS;
        v.gridItemIds[3] = ID_PLANKS; v.gridItemIds[4] = ID_PLANKS; v.gridItemIds[5] = ID_PLANKS;
        v.ingredientCount = 1;
        v.ingredients[0] = {ID_PLANKS, 5, -1};
    }
    // G14: Wool (1 variant)
    {
        RecipeGroup &g = cat4.groups[14];
        g.variantCount = 1;
        RecipeVariant &v = g.variants[0];
        v.name = "Wool";
        v.resultId = ID_WOOL;
        v.resultCount = 1;
        v.requiresWorkbench = false;
        v.gridWidth = 2; v.gridHeight = 2;
        for (int i = 0; i < 4; ++i) v.gridItemIds[i] = ID_STRING;
        v.ingredientCount = 1;
        v.ingredients[0] = {ID_STRING, 4, -1};
    }

        // Allocate persistent ItemStack objects for zero-allocation rendering
    for (int c = 0; c < LegacyCraftingScreen::kCategoryCount; ++c)
    {
        RecipeCategory &cat = s_categories[c];
        if (cat.iconItemId > 0)
            cat.iconStack = new ItemStack(cat.iconItemId, 1, cat.iconDamage);
        for (int g = 0; g < cat.groupCount; ++g)
        {
            RecipeGroup &grp = cat.groups[g];
            for (int v = 0; v < grp.variantCount; ++v)
            {
                RecipeVariant &var = grp.variants[v];
                if (var.resultId > 0)
                    var.resultStack = new ItemStack(var.resultId, var.resultCount, var.resultDamage);
                for (int s = 0; s < 9; ++s)
                {
                    if (var.gridItemIds[s] > 0)
                        var.gridStacks[s] = new ItemStack(var.gridItemIds[s], 1, var.gridItemDamage[s] == -1 ? 0 : var.gridItemDamage[s]);
                }
            }
        }
    }

    // Precalculate s_categories2x2 with strict 2x2 non-workbench recipes
    for (int c = 0; c < LegacyCraftingScreen::kCategoryCount; ++c)
    {
        RecipeCategory &srcCat = s_categories[c];
        RecipeCategory &dstCat = s_categories2x2[c];
        dstCat.name = srcCat.name;
        dstCat.iconItemId = srcCat.iconItemId;
        dstCat.iconDamage = srcCat.iconDamage;
        dstCat.iconStack = srcCat.iconStack;
        dstCat.groupCount = 0;

        for (int g = 0; g < srcCat.groupCount; ++g)
        {
            const RecipeGroup &srcGrp = srcCat.groups[g];
            int_t validCount = 0;
            for (int v = 0; v < srcGrp.variantCount; ++v)
            {
                const RecipeVariant &var = srcGrp.variants[v];
                if (var.gridWidth <= 2 && var.gridHeight <= 2 && !var.requiresWorkbench)
                    validCount++;
            }

            if (validCount > 0)
            {
                RecipeGroup &dstGrp = dstCat.groups[dstCat.groupCount];
                dstGrp.variantCount = 0;
                for (int v = 0; v < srcGrp.variantCount; ++v)
                {
                    const RecipeVariant &var = srcGrp.variants[v];
                    if (var.gridWidth <= 2 && var.gridHeight <= 2 && !var.requiresWorkbench)
                    {
                        dstGrp.variants[dstGrp.variantCount++] = var;
                    }
                }
                dstCat.groupCount++;
            }
        }

        // Ensure 2x2 tab icon shows a valid 2x2 item
        if (dstCat.groupCount > 0 && dstCat.groups[0].variantCount > 0)
        {
            const RecipeVariant &firstVar = dstCat.groups[0].variants[0];
            dstCat.iconItemId = firstVar.resultId;
            dstCat.iconDamage = firstVar.resultDamage;
            dstCat.iconStack = firstVar.resultStack;
        }
    }

    s_recipesInitialized = true;
}

} // namespace

LegacyCraftingScreen::LegacyCraftingScreen(InventoryPlayer *playerInventory, World *worldObj,
                                           int_t x, int_t y, int_t z, bool is2x2, EntityPlayer *player)
    : inventory(playerInventory), world(worldObj), posX(x), posY(y), posZ(z),
      is2x2Mode(is2x2), entityPlayer(player), selectedCategory(0),
      visibleCategoryCount(0), selectedVisibleTab(0),
      craftHoldTicks(0), ps2ActionReleaseLatch(true),
      navRepeatMs(0),
      guiLeft(0), guiTop(0), xSize(282), ySize(168),
      ownerPlayerIndex(-1),
      invZoneActive(false), invCursorRow(0), invCursorCol(0), grabbedSlotIndex(-1),
      m_craftingStateDirty(true), m_cachedCanCraft(false), m_cachedInventoryHash(0)
{
    initStaticRecipes();

    for (int i = 0; i < 9; ++i)
        m_cachedSlotHasIngredient[i] = true;

    for (int i = 0; i < kCategoryCount; ++i)
    {
        visibleCategoryIndices[i] = i;
        selectedGroup[i] = 0;
        scrollOffset[i] = 0;
        for (int j = 0; j < 32; ++j)
            selectedVariant[i][j] = 0;
    }

    const RecipeCategory *cats = getCategoriesTable(is2x2);
    for (int i = 0; i < kCategoryCount; ++i)
    {
        if (cats[i].groupCount > 0)
        {
            visibleCategoryIndices[visibleCategoryCount++] = i;
        }
    }
    if (visibleCategoryCount == 0)
    {
        visibleCategoryIndices[0] = 0;
        visibleCategoryCount = 1;
    }
    selectedVisibleTab = 0;
    selectedCategory = visibleCategoryIndices[0];

    if (player != nullptr && mc != nullptr && player == static_cast<EntityPlayer*>(mc->thePlayer2))
        ownerPlayerIndex = 1;

    // 3x3 workbench windows are real server windows on a remote world: the
    // server creates a ContainerWorkbench when the packet ordering lands in
    // NetClientHandler::handleOpenWindow and then stamps our container with
    // its windowId, which only works if a workbench container already lives
    // in craftingInventory -- the legacy screen never created one, so every
    // later Packet102 click fell into the player-inventory layout instead.
    // Mirror what GuiCrafting's ctor hands to GuiContainer.
    if (!is2x2Mode && worldObj != nullptr && worldObj->multiplayerWorld && player != nullptr)
    {
        mpWorkbenchContainer = new ContainerWorkbench(playerInventory, worldObj, x, y, z);
        player->craftingInventory = mpWorkbenchContainer;
    }
}

LegacyCraftingScreen::~LegacyCraftingScreen()
{
    // Outlive check: onGuiClosed normally releases the session container, but
    // a hard world-teardown can skip it.
    if (mpWorkbenchContainer != nullptr)
    {
        EntityPlayer *p = entityPlayer != nullptr ? entityPlayer
                          : (mc != nullptr ? static_cast<EntityPlayer*>(mc->thePlayer) : nullptr);
        if (p != nullptr && p->craftingInventory == mpWorkbenchContainer)
            p->craftingInventory = p->inventorySlots;
        delete mpWorkbenchContainer;
        mpWorkbenchContainer = nullptr;
    }
}

int LegacyCraftingScreen::getOwnerPlayerIndex() const
{
    if (ownerPlayerIndex >= 0)
        return ownerPlayerIndex;
    if (entityPlayer != nullptr && mc != nullptr && entityPlayer == static_cast<EntityPlayer*>(mc->thePlayer2))
        return 1;
    if (mc != nullptr && mc->isScreenOwnedByPlayer2())
        return 1;
    return 0;
}

void LegacyCraftingScreen::initGui()
{
    GuiScreen::initGui();
    xSize = 282;
    ySize = 168;
    guiLeft = (width - xSize) / 2;
    guiTop = (height - ySize) / 2;
    // A revived screen (see Minecraft::displayGuiScreen) must not keep a
    // strip cursor or a half-finished grab from its previous life.
    invZoneActive = false;
    invCursorRow = 0;
    invCursorCol = 0;
    grabbedSlotIndex = -1;

    // Revive path: the screen object may come back from ownedGuiScreens after
    // onGuiClosed already tore the workbench container down -- rebuild the
    // container binding, or 3x3 clicks would fall into the inventory layout.
    if (!is2x2Mode && mpWorkbenchContainer == nullptr && world != nullptr &&
        world->multiplayerWorld)
    {
        EntityPlayer *p = entityPlayer != nullptr ? entityPlayer
                          : (mc != nullptr ? static_cast<EntityPlayer*>(mc->thePlayer) : nullptr);
        if (p != nullptr)
        {
            mpWorkbenchContainer = new ContainerWorkbench(inventory, world, posX, posY, posZ);
            p->craftingInventory = mpWorkbenchContainer;
        }
    }

    visibleCategoryCount = 0;
    const RecipeCategory *cats = getCategoriesTable(is2x2Mode);
    for (int i = 0; i < kCategoryCount; ++i)
    {
        if (cats[i].groupCount > 0)
        {
            visibleCategoryIndices[visibleCategoryCount++] = i;
        }
    }
    if (visibleCategoryCount == 0)
    {
        visibleCategoryIndices[0] = 0;
        visibleCategoryCount = 1;
    }

    if (selectedVisibleTab >= visibleCategoryCount)
        selectedVisibleTab = 0;
    selectedCategory = visibleCategoryIndices[selectedVisibleTab];

    ensureSelectionVisible();
    updateCraftingState();
}

void LegacyCraftingScreen::legacyCloseScreen()
{
    EntityPlayer *p = entityPlayer != nullptr ? entityPlayer
                      : (mc != nullptr ? static_cast<EntityPlayer*>(mc->thePlayer) : nullptr);
    // Route through the player so multiplayer sends Packet101CloseWindow and
    // resets craftingInventory -- displayGuiScreen(nullptr) alone left the
    // server's workbench window open. Singleplayer closeScreen() ends in the
    // same displayGuiScreen/closePlayerScreen call, so offline behavior is
    // unchanged.
    if (p != nullptr)
    {
        p->closeScreen();
        return;
    }
    if (mc != nullptr && mc->isSplitScreenActive())
        mc->closePlayerScreen(getOwnerPlayerIndex());
    else if (mc != nullptr)
        mc->displayGuiScreen(nullptr);
}

void LegacyCraftingScreen::onGuiClosed()
{
    // 3x3 multiplayer sessions created a real workbench container for this
    // screen (see the ctor): run its close semantics (matrix leftovers back to
    // the player) and hand the live container slot back to the inventory one,
    // exactly as GuiContainer::onGuiClosed + EntityPlayer::closeScreen do.
    if (mpWorkbenchContainer != nullptr)
    {
        EntityPlayer *p = entityPlayer != nullptr ? entityPlayer
                          : (mc != nullptr ? static_cast<EntityPlayer*>(mc->thePlayer) : nullptr);
        if (p != nullptr)
        {
            if (mc != nullptr && mc->playerController != nullptr)
                mc->playerController->closeWindow(mpWorkbenchContainer->windowId, p);
            mpWorkbenchContainer->onCraftGuiClosed(p);
            if (p->craftingInventory == mpWorkbenchContainer)
                p->craftingInventory = p->inventorySlots;
        }
        delete mpWorkbenchContainer;
        mpWorkbenchContainer = nullptr;
    }
    GuiScreen::onGuiClosed();
}

void LegacyCraftingScreen::ensureSelectionVisible()
{
    const RecipeCategory *cats = getCategoriesTable(is2x2Mode);
    const int_t cat = selectedCategory;
    if (cat < 0 || cat >= kCategoryCount) return;
    const int_t groupCount = cats[cat].groupCount;
    if (groupCount <= 0) return;

    if (selectedGroup[cat] < 0) selectedGroup[cat] = 0;
    if (selectedGroup[cat] >= groupCount) selectedGroup[cat] = groupCount - 1;

    const int_t cur = selectedGroup[cat];
    const int_t varCount = cats[cat].groups[cur].variantCount;
    if (selectedVariant[cat][cur] < 0) selectedVariant[cat][cur] = 0;
    if (selectedVariant[cat][cur] >= varCount) selectedVariant[cat][cur] = varCount - 1;

    const int_t visibleSlots = 10;
    if (cur < scrollOffset[cat])
        scrollOffset[cat] = cur;
    if (cur >= scrollOffset[cat] + visibleSlots)
        scrollOffset[cat] = cur - visibleSlots + 1;
}

void LegacyCraftingScreen::changeCategory(int dir)
{
    if (visibleCategoryCount <= 0) return;
    selectedVisibleTab = (selectedVisibleTab + dir + visibleCategoryCount) % visibleCategoryCount;
    selectedCategory = visibleCategoryIndices[selectedVisibleTab];
    ensureSelectionVisible();
    updateCraftingState();
    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
}

void LegacyCraftingScreen::changeVariant(int dir)
{
    const RecipeCategory *cats = getCategoriesTable(is2x2Mode);
    const int_t cat = selectedCategory;
    if (cat < 0 || cat >= kCategoryCount) return;
    const int_t grp = selectedGroup[cat];
    if (grp < 0 || grp >= cats[cat].groupCount) return;
    const int_t varCount = cats[cat].groups[grp].variantCount;
    if (varCount <= 1) return;

    // Wraps: the D-pad's second carousel direction became the strip handoff
    // (see handleNavigation), so the pad cycles variants on UP alone and
    // needs the wrap to reach every variant. The touch arrows above/below
    // the selected recipe use the same calls and wrap identically.
    selectedVariant[cat][grp] = (selectedVariant[cat][grp] + dir + varCount) % varCount;
    updateCraftingState();
    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
}

void LegacyCraftingScreen::handleNavigation(int dirX, int dirY)
{
    // The strip zone owns the D-pad while it is active; the carousel keeps
    // left/right and its variants ride UP (the touch arrows above/below the
    // selected recipe stay for direct access).
    if (invZoneActive)
    {
        handleInventoryZoneNavigation(dirX, dirY);
        return;
    }

    const RecipeCategory *cats = getCategoriesTable(is2x2Mode);
    const int_t cat = selectedCategory;
    if (cat < 0 || cat >= kCategoryCount) return;
    const int_t groupCount = cats[cat].groupCount;
    if (groupCount <= 0) return;

    if (dirX != 0)
    {
        selectedGroup[cat] = (selectedGroup[cat] + dirX + groupCount) % groupCount;
        ensureSelectionVisible();
        updateCraftingState();
        if (mc != nullptr && mc->sndManager != nullptr)
            mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
    }
    else if (dirY > 0)
    {
        // Up cycles variants -- with wrap, since down is no longer the
        // carousel's second direction (see below); the touch arrows above
        // and below the selected recipe stay for direct access.
        changeVariant(1);
    }
    else if (dirY < 0)
    {
        // Down leaves the recipes for the inventory strip -- the strip is
        // the console inventory and reaching it is the whole point of the
        // D-pad here (2026-09-28: "the cursor never came down to the
        // inventory" on every platform). NOTE: this screen's callers pass
        // dirY=+1 for UP (see handleSpecializedMenuInput), so DOWN is the
        // negative dirY here.
        enterInventoryZone();
    }
}

void LegacyCraftingScreen::enterInventoryZone()
{
    // Enter under the recipe cursor: the strip's top row continues the
    // carousel's spatial flow, so the recipe's x maps onto a strip column.
    constexpr int_t visibleCount = 10;
    const int_t carouselW = visibleCount * 22;
    const int_t carouselStartX = guiLeft + (xSize - carouselW) / 2;
    const int_t activeSlotX = carouselStartX + (selectedGroup[selectedCategory] - scrollOffset[selectedCategory]) * 22;
    const int_t invX = guiLeft + 104;
    int_t col = (activeSlotX + 9 - invX) / 18;
    if (col < 0) col = 0;
    if (col > 8) col = 8;

    invCursorRow = 0;
    invCursorCol = col;
    invZoneActive = true;
    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
}

void LegacyCraftingScreen::handleInventoryZoneNavigation(int dirX, int dirY)
{
    // Screen-space rows: row 0 is the strip's top row and row 3 the hotbar,
    // but this screen's callers pass dirY=+1 for UP (see the carousel's
    // handleNavigation note), so up is the positive dirY here too.
    if (dirX < 0)
    {
        if (invCursorCol > 0)
        {
            --invCursorCol;
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
        }
    }
    else if (dirX > 0)
    {
        if (invCursorCol < 8)
        {
            ++invCursorCol;
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
        }
    }
    else if (dirY > 0)
    {
        // Up from the strip's top row hands the D-pad back to the recipes.
        if (invCursorRow > 0)
        {
            --invCursorRow;
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
        }
        else
        {
            invZoneActive = false;
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
        }
    }
    else if (dirY < 0)
    {
        // Down stops at the hotbar; wrapping between the strip and the
        // carousel would make the handoff unpredictable.
        if (invCursorRow < 3)
        {
            ++invCursorRow;
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
        }
    }
}

void LegacyCraftingScreen::clickStripSlot(int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= 36 ||
        inventory == nullptr || inventory->mainInventory == nullptr)
        return;

    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);

    // Multiplayer: a local swap is invisible to the server and reverts on the
    // next window sync. First press leaves a visual grab only; the physical
    // move runs on the second press as vanilla window clicks, so nothing is
    // ever held in a cursor the legacy UI cannot draw.
    if (world != nullptr && world->multiplayerWorld)
    {
        EntityPlayer *player = entityPlayer != nullptr ? entityPlayer
                               : (mc != nullptr ? static_cast<EntityPlayer*>(mc->thePlayer) : nullptr);
        Container *container = player != nullptr ? player->craftingInventory : nullptr;
        if (grabbedSlotIndex < 0)
        {
            if (inventory->mainInventory[slotIndex] != nullptr)
                grabbedSlotIndex = slotIndex;
            return;
        }
        if (grabbedSlotIndex == slotIndex || container == nullptr || player == nullptr)
        {
            grabbedSlotIndex = -1;
            return;
        }

        // map a mainInventory index to the container slot backing it. The
        // storage region's screen positions are identical in ContainerPlayer
        // and ContainerWorkbench (hotbar i<9 -> (8+i*18,142); main rows
        // i>=9 -> (8+((i-9)%9)*18, 84+((i-9)/9)*18)), so match on those --
        // Slot::slotIndex is private and stack-pointer identity is ambiguous
        // for empty slots.
        auto slotForInvIndex = [&](int_t invIndex) -> int_t
        {
            const int_t wantX = invIndex < 9 ? 8 + invIndex * 18
                                             : 8 + ((invIndex - 9) % 9) * 18;
            const int_t wantY = invIndex < 9 ? 142
                                             : 84 + ((invIndex - 9) / 9) * 18;
            for (Slot *slot : container->slots)
            {
                if (slot != nullptr && slot->getInventory() == inventory &&
                    slot->xDisplayPosition == wantX && slot->yDisplayPosition == wantY)
                    return slot->slotNumber;
            }
            return -1;
        };

        const int_t srcSlot = slotForInvIndex(grabbedSlotIndex);
        const int_t dstSlot = slotForInvIndex(slotIndex);
        grabbedSlotIndex = -1;
        if (srcSlot < 0 || dstSlot < 0)
            return;

        // Vanilla click semantics: pick A up, click B (swap/merge/place), and
        // if B partially merged, park the remainder back where it started.
        delete mc->playerController->windowClick(container->windowId, srcSlot, 0, false, player);
        delete mc->playerController->windowClick(container->windowId, dstSlot, 0, false, player);
        if (inventory->getItemStack() != nullptr)
            delete mc->playerController->windowClick(container->windowId, srcSlot, 0, false, player);
        return;
    }

    if (grabbedSlotIndex < 0)
    {
        // Nothing held: lift this slot's stack. Lifting an empty slot is a
        // no-op -- there is nothing to move and no cursor item would show.
        if (inventory->mainInventory[slotIndex] != nullptr)
            grabbedSlotIndex = slotIndex;
    }
    else if (grabbedSlotIndex == slotIndex)
    {
        grabbedSlotIndex = -1; // same slot again: put it back down
    }
    else
    {
        // Swap in place between the two slots. No cursor item ever exists,
        // so closing the screen mid-move loses nothing and nothing can be
        // duplicated; crafting's ingredient scan reads the same array and
        // is unaffected.
        ItemStack *held = inventory->mainInventory[grabbedSlotIndex];
        inventory->mainInventory[grabbedSlotIndex] = inventory->mainInventory[slotIndex];
        inventory->mainInventory[slotIndex] = held;
        grabbedSlotIndex = -1;
    }
}

bool LegacyCraftingScreen::playerHasIngredient(int_t itemId, int_t itemDamage) const
{
    if (itemId <= 0 || inventory == nullptr || inventory->mainInventory == nullptr)
        return false;

    for (int_t i = 0; i < 36; ++i)
    {
        ItemStack *st = inventory->mainInventory[i];
        if (st != nullptr && st->isValid() && st->itemID == itemId)
        {
            if (itemDamage == -1 || st->getItemDamage() == itemDamage)
                return true;
        }
    }
    return false;
}

bool LegacyCraftingScreen::canCraftCurrentRecipe() const
{
    const RecipeCategory *cats = getCategoriesTable(is2x2Mode);
    const int_t cat = selectedCategory;
    if (cat < 0 || cat >= kCategoryCount) return false;
    const int_t grp = selectedGroup[cat];
    if (grp < 0 || grp >= cats[cat].groupCount) return false;
    const int_t var = selectedVariant[cat][grp];
    if (var < 0 || var >= cats[cat].groups[grp].variantCount) return false;
    const RecipeVariant &recipe = cats[cat].groups[grp].variants[var];

    if (is2x2Mode && recipe.requiresWorkbench)
        return false;

    if (inventory == nullptr || inventory->mainInventory == nullptr)
        return false;

    for (int_t i = 0; i < recipe.ingredientCount; ++i)
    {
        const RecipeIngredient &ing = recipe.ingredients[i];
        if (ing.itemId <= 0 || ing.count <= 0)
            continue;
        int_t available = 0;
        for (int_t s = 0; s < 36; ++s)
        {
            ItemStack *st = inventory->mainInventory[s];
            if (st != nullptr && st->isValid() && st->itemID == ing.itemId)
            {
                if (ing.itemDamage == -1 || st->getItemDamage() == ing.itemDamage)
                    available += st->stackSize;
            }
        }
        if (available < ing.count)
            return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Multiplayer crafting
//
// The legacy screen never touches the vanilla container: offline it just
// rewrites mainInventory, which a server knows nothing about and reverts on
// the next window sync. In multiplayer we instead drive the player's live
// container through windowClick() (slot pickup / place-one / put-back, then
// a click on the result slot), exactly what GuiCrafting's mouse handlers do
// -- each click simulates locally AND emits the Packet102 the server replays
// on its own mirror of the container. The 3x3 workbench variant additionally
// installs a real ContainerWorkbench in the player's craftingInventory while
// the screen is open (see the ctor), so the windowId the server just
// announced in Packet104 (handleOpenWindow) lands on the right container and
// window sync packets resolve.
// ---------------------------------------------------------------------------

namespace
{

// One container click with the vanilla semantics (mouseButton 1 = place a
// single unit). The simulated stack copy is dropped.
void legacyContainerClick(Minecraft *mc, EntityPlayer *player, Container *container,
                          int_t slotNumber, int_t mouseButton)
{
    delete mc->playerController->windowClick(container->windowId, slotNumber,
                                             mouseButton, false, player);
}

// The container slot whose backing stack pointer is `stack` inside
// `inventory` (the player's main inventory/armor pair), or -1.
int_t legacySlotForStack(Container *container, InventoryPlayer *inventory, ItemStack *stack)
{
    if (stack == nullptr)
        return -1;
    for (Slot *slot : container->slots)
    {
        if (slot == nullptr || slot->getInventory() != inventory)
            continue;
        if (slot->getStack() == stack)
            return slot->slotNumber;
    }
    return -1;
}

// The first storage slot able to receive `stack` (mergeable partial first,
// then any empty one). -1 if the inventory is full.
int_t legacyFindDepositSlot(Container *container, InventoryPlayer *inventory, ItemStack *stack)
{
    int_t emptySlot = -1;
    for (Slot *slot : container->slots)
    {
        if (slot == nullptr || slot->getInventory() != inventory)
            continue;
        if (!slot->isItemValid(stack))
            continue; // armor slots reject non-armor
        ItemStack *st = slot->getStack();
        if (st == nullptr)
        {
            if (emptySlot < 0)
                emptySlot = slot->slotNumber;
            continue;
        }
        if (st->isValid() && st->itemID == stack->itemID &&
            (!st->getHasSubtypes() || st->getItemDamage() == stack->getItemDamage()) &&
            st->stackSize + stack->stackSize <= st->getMaxStackSize())
            return slot->slotNumber;
    }
    return emptySlot;
}

} // namespace

bool LegacyCraftingScreen::craftCurrentRecipeViaContainerClicks()
{
    const RecipeCategory *cats = getCategoriesTable(is2x2Mode);
    const int_t cat = selectedCategory;
    const int_t grp = selectedGroup[cat];
    if (grp < 0 || grp >= cats[cat].groupCount) return false;
    const int_t var = selectedVariant[cat][grp];
    if (var < 0 || var >= cats[cat].groups[grp].variantCount) return false;
    const RecipeVariant &recipe = cats[cat].groups[grp].variants[var];

    if (is2x2Mode && recipe.requiresWorkbench)
        return false;

    EntityPlayer *player = entityPlayer != nullptr ? entityPlayer
                           : (mc != nullptr ? static_cast<EntityPlayer*>(mc->thePlayer) : nullptr);
    if (player == nullptr || inventory == nullptr || inventory->mainInventory == nullptr ||
        mc == nullptr || mc->playerController == nullptr)
        return false;

    // The screen must be bound to the container the matrix lives in: the
    // player's own container for 2x2, the session workbench for 3x3 (created
    // in the ctor for the multiplayer window).
    Container *container = player->craftingInventory;
    if (container == nullptr)
        return false;
    const int_t gridW = is2x2Mode ? 2 : 3;
    const int_t matrixFirstSlot = 1; // slot 0 is the crafting result in both containers

    // A held cursor item would be dragged into the sequence; the legacy UI
    // has no cursor, but a stale one can survive a java-GUI round trip --
    // refuse rather than corrupting the exchange.
    if (inventory->getItemStack() != nullptr)
        return false;

    // Fill the recipe grid cell by cell, one unit each. The source search
    // runs fresh per placement (some recipes repeat an id across cells) --
    // the "pick up / place one / put the rest back" dance keeps every cell
    // stack at size 1 while the result drains exactly one unit each.
    for (int_t gy = 0; gy < recipe.gridHeight; ++gy)
    {
        for (int_t gx = 0; gx < recipe.gridWidth; ++gx)
        {
            const int_t wantId = recipe.gridItemIds[gy * recipe.gridWidth + gx];
            if (wantId <= 0)
                continue;
            const int_t wantDamage = recipe.gridItemDamage[gy * recipe.gridWidth + gx];

            int_t srcInvIdx = -1;
            for (int_t i = 0; i < 36; ++i)
            {
                ItemStack *st = inventory->mainInventory[i];
                if (st != nullptr && st->isValid() && st->stackSize > 0 &&
                    st->itemID == wantId && (wantDamage == -1 || st->getItemDamage() == wantDamage))
                {
                    srcInvIdx = i;
                    break;
                }
            }
            if (srcInvIdx < 0)
                return false;

            const int_t srcSlot = legacySlotForStack(container, inventory, inventory->mainInventory[srcInvIdx]);
            if (srcSlot < 0)
                return false;
            const int_t matrixSlot = matrixFirstSlot + gy * gridW + gx;

            legacyContainerClick(mc, player, container, srcSlot, 0);    // pick up
            legacyContainerClick(mc, player, container, matrixSlot, 1); // place exactly one
            if (inventory->getItemStack() != nullptr)
                legacyContainerClick(mc, player, container, srcSlot, 0); // put the rest back
        }
    }

    // Take the result (SlotCrafting consumes one unit per filled cell).
    legacyContainerClick(mc, player, container, 0, 0);

    // Deposit the held result; dropping to -999 mirrors "close the GUI with
    // an item on the cursor" when the inventory is full.
    ItemStack *held = inventory->getItemStack();
    if (held != nullptr)
    {
        const int_t dstSlot = legacyFindDepositSlot(container, inventory, held);
        if (dstSlot >= 0)
            legacyContainerClick(mc, player, container, dstSlot, 0);
        else
            legacyContainerClick(mc, player, container, -999, 0);
    }

    inventory->inventoryChanged = true;
    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.pop", 1.0f, 1.0f);
    return true;
}

uint32_t LegacyCraftingScreen::computeInventoryHash() const
{
    if (inventory == nullptr || inventory->mainInventory == nullptr)
        return 0;
    uint32_t h = 0x811c9dc5;
    for (int i = 0; i < 36; ++i)
    {
        ItemStack *st = inventory->mainInventory[i];
        if (st != nullptr && st->isValid())
        {
            h ^= static_cast<uint32_t>(st->itemID);
            h *= 0x01000193;
            h ^= static_cast<uint32_t>(st->stackSize);
            h *= 0x01000193;
            h ^= static_cast<uint32_t>(st->getItemDamage() + 1);
            h *= 0x01000193;
        }
    }
    return h;
}

void LegacyCraftingScreen::updateCraftingState()
{
    m_cachedCanCraft = canCraftCurrentRecipe();

    const RecipeCategory *cats = getCategoriesTable(is2x2Mode);
    const int_t cat = selectedCategory;
    if (cat >= 0 && cat < kCategoryCount && selectedGroup[cat] >= 0 && selectedGroup[cat] < cats[cat].groupCount)
    {
        const RecipeGroup &grp = cats[cat].groups[selectedGroup[cat]];
        const int_t var = selectedVariant[cat][selectedGroup[cat]];
        if (var >= 0 && var < grp.variantCount)
        {
            const RecipeVariant &v = grp.variants[var];
            const int slotCount = is2x2Mode ? 4 : 9;
            for (int s = 0; s < slotCount; ++s)
            {
                if (v.gridStacks[s] != nullptr)
                    m_cachedSlotHasIngredient[s] = playerHasIngredient(v.gridItemIds[s], v.gridItemDamage[s]);
                else
                    m_cachedSlotHasIngredient[s] = true;
            }
        }
    }
    m_craftingStateDirty = false;
    m_cachedInventoryHash = computeInventoryHash();
}

void LegacyCraftingScreen::craftCurrentRecipe()
{
    // On a remote world only the server's container can craft; the local
    // mainInventory rewrite used offline would be reverted by the next
    // window sync. Emulate the GUI's own click sequence instead.
    if (world != nullptr && world->multiplayerWorld)
    {
        if (!craftCurrentRecipeViaContainerClicks())
        {
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("note.bass", 1.0f, 0.8f);
        }
        return;
    }

    if (!canCraftCurrentRecipe())
    {
        if (mc != nullptr && mc->sndManager != nullptr)
            mc->sndManager->playSoundFX("note.bass", 1.0f, 0.8f);
        return;
    }

    const RecipeCategory *cats = getCategoriesTable(is2x2Mode);
    const int_t cat = selectedCategory;
    if (cat < 0 || cat >= kCategoryCount) return;
    const int_t grp = selectedGroup[cat];
    if (grp < 0 || grp >= cats[cat].groupCount) return;
    const int_t var = selectedVariant[cat][grp];
    if (var < 0 || var >= cats[cat].groups[grp].variantCount) return;
    const RecipeVariant &recipe = cats[cat].groups[grp].variants[var];

    // Deduct ingredients
    for (int_t i = 0; i < recipe.ingredientCount; ++i)
    {
        const RecipeIngredient &ing = recipe.ingredients[i];
        if (ing.itemId <= 0 || ing.count <= 0)
            continue;
        int_t toTake = ing.count;
        for (int_t s = 0; s < 36 && toTake > 0; ++s)
        {
            ItemStack *st = inventory->mainInventory[s];
            if (st != nullptr && st->isValid() && st->itemID == ing.itemId)
            {
                if (ing.itemDamage == -1 || st->getItemDamage() == ing.itemDamage)
                {
                    int_t take = std::min(st->stackSize, toTake);
                    st->stackSize -= take;
                    toTake -= take;
                    if (st->stackSize <= 0)
                    {
                        delete st;
                        inventory->mainInventory[s] = nullptr;
                    }
                }
            }
        }
    }

    // Add crafted result to inventory
    ItemStack *result = new ItemStack(recipe.resultId, recipe.resultCount, recipe.resultDamage);
    inventory->addItemStackToInventory(result);
    if (result != nullptr && result->stackSize > 0)
    {
        EntityPlayer *p = entityPlayer ? entityPlayer : (mc ? static_cast<EntityPlayer*>(mc->thePlayer) : nullptr);
        if (p != nullptr)
            p->dropPlayerItem(result);
        else
            delete result;
    }
    else
    {
        delete result;
    }
    inventory->inventoryChanged = true;
    updateCraftingState();

    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.pop", 1.0f, 1.0f);
}

void LegacyCraftingScreen::drawSlotRect(int_t sx, int_t sy)
{
    // Sunken slot with 3 rects: dark top/left, light bottom/right, slot gray fill
    drawRect(sx, sy, sx + 18, sy + 18, 0xff373737);
    drawRect(sx + 1, sy + 1, sx + 18, sy + 18, 0xffffffff);
    drawRect(sx + 1, sy + 1, sx + 17, sy + 17, 0xff8b8b8b);
}

void LegacyCraftingScreen::drawTooltip(ItemStack *stack, int_t mouseX, int_t mouseY)
{
    if (stack == nullptr || fontRenderer == nullptr)
        return;

    std::vector<std::string> lines = stack->getItemNameandInformation();
    if (lines.empty())
        return;

    int_t tooltipWidth = 0;
    for (const std::string &l : lines)
        tooltipWidth = std::max(tooltipWidth, fontRenderer->getStringWidth(l));

    int_t tx = mouseX + 12;
    int_t ty = mouseY - 12;
    if (tx + tooltipWidth + 6 > width)
        tx = width - tooltipWidth - 6;
    if (ty < 4)
        ty = 4;

    int_t tooltipHeight = 8;
    if (lines.size() > 1)
        tooltipHeight += 2 + (static_cast<int_t>(lines.size()) - 1) * 10;

    const int_t bg = static_cast<int_t>(0xf0100010u);
    drawGradientRect(tx - 3, ty - 4, tx + tooltipWidth + 3, ty - 3, bg, bg);
    drawGradientRect(tx - 3, ty + tooltipHeight + 3, tx + tooltipWidth + 3, ty + tooltipHeight + 4, bg, bg);
    drawGradientRect(tx - 3, ty - 3, tx + tooltipWidth + 3, ty + tooltipHeight + 3, bg, bg);
    drawGradientRect(tx - 4, ty - 3, tx - 3, ty + tooltipHeight + 3, bg, bg);
    drawGradientRect(tx + tooltipWidth + 3, ty - 3, tx + tooltipWidth + 4, ty + tooltipHeight + 3, bg, bg);

    const int_t borderTop = 0x505000ff;
    const int_t borderBottom = (borderTop & 0x00fefefe) >> 1 | (borderTop & static_cast<int_t>(0xff000000u));
    drawGradientRect(tx - 3, ty - 2, tx - 2, ty + tooltipHeight + 2, borderTop, borderBottom);
    drawGradientRect(tx + tooltipWidth + 2, ty - 2, tx + tooltipWidth + 3, ty + tooltipHeight + 2, borderTop, borderBottom);
    drawGradientRect(tx - 3, ty - 3, tx + tooltipWidth + 3, ty - 2, borderTop, borderTop);
    drawGradientRect(tx - 3, ty + tooltipHeight + 2, tx + tooltipWidth + 3, ty + tooltipHeight + 3, borderBottom, borderBottom);

    for (std::size_t i = 0; i < lines.size(); ++i)
    {
        fontRenderer->drawStringWithShadow(lines[i], tx, ty, 0xffffffff);
        ty += (i == 0) ? 12 : 10;
    }
}

void LegacyCraftingScreen::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    drawDefaultBackground();

    const int_t panelLeft = guiLeft;
    const int_t panelTop = guiTop + 24;
    const int_t panelRight = guiLeft + xSize;
    const int_t panelBottom = guiTop + ySize;

    if (selectedCategory < 0 || selectedCategory >= kCategoryCount)
        selectedCategory = 0;
    if (selectedVisibleTab < 0 || selectedVisibleTab >= visibleCategoryCount)
        selectedVisibleTab = 0;

    const RecipeCategory *cats = getCategoriesTable(is2x2Mode);
    const RecipeCategory &currentCat = cats[selectedCategory];

    const int_t carouselY = panelTop + 16;
    const int_t visibleCount = 10;
    const int_t carouselW = visibleCount * 22;
    const int_t carouselStartX = guiLeft + (xSize - carouselW) / 2;

    const int_t curGroup = selectedGroup[selectedCategory];
    const int_t scroll = scrollOffset[selectedCategory];

    const int_t matrixX = is2x2Mode ? (guiLeft + 12) : (guiLeft + 9);
    const int_t matrixY = is2x2Mode ? (panelTop + 68) : (panelTop + 62);
    const int_t arrowX = is2x2Mode ? (guiLeft + 54) : (guiLeft + 66);
    const int_t arrowY = is2x2Mode ? (matrixY + 10) : (matrixY + 19);
    const int_t resultX = is2x2Mode ? (guiLeft + 82) : (guiLeft + 90);
    const int_t resultY = is2x2Mode ? (matrixY + 7) : (matrixY + 16);
    const int_t statusY = panelTop + 122;

    const int_t invX = guiLeft + 114;
    const int_t invY = panelTop + 62;

    const bool hasActiveGroup = (curGroup >= 0 && curGroup < currentCat.groupCount);
    const int_t curVar = hasActiveGroup ? selectedVariant[selectedCategory][curGroup] : 0;
    const RecipeVariant *activeVariant = (hasActiveGroup && curVar >= 0 && curVar < currentCat.groups[curGroup].variantCount)
                                             ? &currentCat.groups[curGroup].variants[curVar]
                                             : nullptr;

    const bool canCraft = m_cachedCanCraft;
    ItemStack *hoveredStack = nullptr;

    int_t activeSlotX = -1;
    int_t activeSlotY = -1;
    int_t activeGroupIdx = -1;

    // =============================================================
    // PASS 1: 2D Unlit Geometry (Panels, Tabs, Slots, Textures, Labels)
    // =============================================================
    renderDisable(RenderCapability::Lighting);

    // 1.1 Main panel drop shadow and outer bevel
    drawRect(panelLeft + 2, panelTop + 2, panelRight + 2, panelBottom + 2, 0x68000000);
    drawRect(panelLeft - 1, panelTop - 1, panelRight + 1, panelBottom + 1, 0xff343434);
    drawRect(panelLeft, panelTop, panelRight, panelBottom, 0xffc6c6c6);
    drawRect(panelLeft + 2, panelTop + 1, panelRight - 2, panelTop + 2, 0xffededed);
    drawRect(panelLeft + 1, panelTop + 2, panelLeft + 2, panelBottom - 2, 0xffededed);
    drawRect(panelLeft + 2, panelBottom - 2, panelRight - 2, panelBottom - 1, 0xff858585);
    drawRect(panelRight - 2, panelTop + 2, panelRight - 1, panelBottom - 2, 0xff858585);

    // 1.2 Top Tabs backgrounds
    for (int t = 0; t < visibleCategoryCount; ++t)
    {
        const int catIdx = visibleCategoryIndices[t];
        const int_t tabX = guiLeft + 16 + t * 32;
        const bool active = (t == selectedVisibleTab);
        const int_t tabY = active ? (guiTop + 4) : (guiTop + 7);
        const int_t tabH = active ? 22 : 18;

        drawRect(tabX - 1, tabY - 1, tabX + 29, tabY + tabH, 0xff343434);
        drawRect(tabX, tabY, tabX + 28, tabY + tabH, active ? 0xffc6c6c6 : 0xff9c9c9c);
        drawRect(tabX + 1, tabY + 1, tabX + 27, tabY + 2, active ? 0xffffffff : 0xffb8b8b8);
        drawRect(tabX + 1, tabY + 2, tabX + 2, tabY + tabH, active ? 0xffffffff : 0xffb8b8b8);
        drawRect(tabX + 27, tabY + 2, tabX + 28, tabY + tabH, active ? 0xff858585 : 0xff505050);

        if (active)
            drawRect(tabX + 1, panelTop, tabX + 27, panelTop + 2, 0xffc6c6c6);
    }

    // 1.3 Tab shoulder hints & Category Title
    // Shoulder button tab hints: the buttons that ride SPACE/SHIFT in
    // handleSpecializedMenuInput(). PS2's are L1/R1, the 3DS's are L/R
    // (mapTextButtons); the Wii maps them to its controller-dependent
    // X/Z (GameCube) or -/2 (Wiimote) pair, so it keeps the generic
    // desktop legend rather than a wrong single label.
    const int_t rHintX = guiLeft + 16 + visibleCategoryCount * 32 + 4;
#if PLATFORM_PS2
    fontRenderer->drawStringWithShadow("L1", guiLeft + 4, guiTop + 9, 0xffe0e0e0);
    fontRenderer->drawStringWithShadow("R1", rHintX, guiTop + 9, 0xffe0e0e0);
#elif PLATFORM_3DS
    fontRenderer->drawStringWithShadow("L", guiLeft + 5, guiTop + 9, 0xffe0e0e0);
    fontRenderer->drawStringWithShadow("R", rHintX, guiTop + 9, 0xffe0e0e0);
#elif PLATFORM_WII
    fontRenderer->drawStringWithShadow("L", guiLeft + 6, guiTop + 9, 0xffe0e0e0);
    fontRenderer->drawStringWithShadow("R", rHintX, guiTop + 9, 0xffe0e0e0);
#else
    fontRenderer->drawStringWithShadow("Q", guiLeft + 6, guiTop + 9, 0xffe0e0e0);
    fontRenderer->drawStringWithShadow("E", rHintX, guiTop + 9, 0xffe0e0e0);
#endif

    int_t catTitleW = fontRenderer->getStringWidth(currentCat.name);
    fontRenderer->drawStringWithShadow(currentCat.name, guiLeft + (xSize - catTitleW) / 2, panelTop + 4, 0xffffff00);

    // 1.4 Carousel slot backgrounds
    for (int i = 0; i < visibleCount; ++i)
    {
        const int_t gIdx = scroll + i;
        if (gIdx >= currentCat.groupCount) break;

        const int_t gx = carouselStartX + i * 22;
        const int_t gy = carouselY;
        const bool isSelected = (gIdx == curGroup);

        drawSlotRect(gx, gy);

        if (isSelected)
        {
            activeSlotX = gx;
            activeSlotY = gy;
            activeGroupIdx = gIdx;
        }
    }

    // 1.5 Horizontal divider line
    drawRect(guiLeft + 8, panelTop + 45, guiLeft + xSize - 8, panelTop + 46, 0xff858585);
    drawRect(guiLeft + 8, panelTop + 46, guiLeft + xSize - 8, panelTop + 47, 0xffffffff);

    // 1.6 Lower Section: Grid & Labels & Result Slot
    if (activeVariant != nullptr)
    {
        fontRenderer->drawString(activeVariant->name ? activeVariant->name : "Recipe", matrixX, panelTop + 50, 0x303030);
        fontRenderer->drawString("Inventory", invX, panelTop + 50, 0x303030);

        const int rows = is2x2Mode ? 2 : 3;
        const int cols = is2x2Mode ? 2 : 3;
        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < cols; ++c)
            {
                const int slot = r * cols + c;
                const int sx = matrixX + c * 18;
                const int sy = matrixY + r * 18;
                drawSlotRect(sx, sy);

                if (activeVariant->gridStacks[slot] != nullptr && !m_cachedSlotHasIngredient[slot])
                {
                    drawRect(sx + 1, sy + 1, sx + 17, sy + 17, 0x60cc2020);
                }
            }
        }

        // Arrow texture
        int_t craftTex = mc->renderEngine->getTexture("/gui/crafting.png");
        renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        mc->renderEngine->bindTexture(craftTex);
        drawTexturedModalRect(arrowX, arrowY, 89, 35, 22, 15);

        // Result Slot frame
        drawRect(resultX, resultY, resultX + 22, resultY + 22, 0xff8b8b8b);
        drawRect(resultX, resultY, resultX + 22, resultY + 1, 0xff373737);
        drawRect(resultX, resultY, resultX + 1, resultY + 22, 0xff373737);
        drawRect(resultX + 1, resultY + 1, resultX + 21, resultY + 2, 0xff373737);
        drawRect(resultX + 1, resultY + 1, resultX + 2, resultY + 21, 0xff373737);
        drawRect(resultX + 21, resultY + 1, resultX + 22, resultY + 22, 0xffffffff);
        drawRect(resultX + 1, resultY + 21, resultX + 22, resultY + 22, 0xffffffff);

        if (!canCraft)
            drawRect(resultX + 1, resultY + 1, resultX + 21, resultY + 21, 0x60cc2020);
        else
            drawRect(resultX + 1, resultY + 1, resultX + 21, resultY + 21, 0x3020c020);

        // Status text
        if (canCraft)
        {
#if PLATFORM_PS2
            fontRenderer->drawString("Ready [Cross]", matrixX, statusY, 0x207820);
#elif PLATFORM_WII
            fontRenderer->drawString("Ready [A]", matrixX, statusY, 0x207820);
#else
            fontRenderer->drawString("Ready [Enter]", matrixX, statusY, 0x207820);
#endif
        }
        else if (is2x2Mode && activeVariant->requiresWorkbench)
        {
            fontRenderer->drawString("Needs Crafting Table", matrixX, statusY, 0x902020);
        }
        else
        {
            fontRenderer->drawString("Missing Items", matrixX, statusY, 0x902020);
        }
    }

    // 1.7 Inventory slot rectangles
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 9; ++c)
        {
            const int sx = invX + c * 18;
            const int sy = invY + r * 18 + (r == 3 ? 3 : 0);
            drawSlotRect(sx, sy);
        }
    }

    // =============================================================
    // PASS 2: 3D Item Rendering (Lighting ENABLED once!)
    // =============================================================
    RenderHelper::enableGUIStandardItemLighting();
    renderPushMatrix();
    renderTranslate(0.0f, 0.0f, 0.0f);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderEnable(RenderCapability::RescaleNormal);
    OpenGlHelper::setLightmapTextureCoords(OpenGlHelper::lightmapTexUnit, 240.0f, 240.0f);

    // 2.1 Tab icons
    for (int t = 0; t < visibleCategoryCount; ++t)
    {
        const int catIdx = visibleCategoryIndices[t];
        const int_t tabX = guiLeft + 16 + t * 32;
        const bool active = (t == selectedVisibleTab);
        const int_t tabY = active ? (guiTop + 4) : (guiTop + 7);
        if (cats[catIdx].iconStack != nullptr)
        {
            itemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine, cats[catIdx].iconStack,
                                            tabX + 6, tabY + (active ? 3 : 2));
        }
    }

    // 2.2 Carousel items
    for (int i = 0; i < visibleCount; ++i)
    {
        const int_t gIdx = scroll + i;
        if (gIdx >= currentCat.groupCount) break;

        const RecipeGroup &grp = currentCat.groups[gIdx];
        const int_t gx = carouselStartX + i * 22;
        const int_t gy = carouselY;
        const int_t varIdx = selectedVariant[selectedCategory][gIdx];

        if (grp.variants[varIdx].resultStack != nullptr)
        {
            itemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine,
                                            grp.variants[varIdx].resultStack, gx + 1, gy + 1);

            if (mouseX >= gx && mouseX < gx + 18 && mouseY >= gy && mouseY < gy + 18)
                hoveredStack = grp.variants[varIdx].resultStack;
        }
    }

    // 2.3 Crafting matrix items
    if (activeVariant != nullptr)
    {
        const int rows = is2x2Mode ? 2 : 3;
        const int cols = is2x2Mode ? 2 : 3;
        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < cols; ++c)
            {
                const int slot = r * cols + c;
                const int sx = matrixX + c * 18;
                const int sy = matrixY + r * 18;

                if (activeVariant->gridStacks[slot] != nullptr)
                {
                    itemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine, activeVariant->gridStacks[slot], sx + 1, sy + 1);

                    if (mouseX >= sx && mouseX < sx + 18 && mouseY >= sy && mouseY < sy + 18)
                        hoveredStack = activeVariant->gridStacks[slot];
                }
            }
        }

        // 2.4 Result slot item
        if (activeVariant->resultStack != nullptr)
        {
            itemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine, activeVariant->resultStack, resultX + 3, resultY + 3);
            itemRenderer->renderItemOverlayIntoGUI(fontRenderer, mc->renderEngine, activeVariant->resultStack, resultX + 3, resultY + 3);

            if (mouseX >= resultX && mouseX < resultX + 22 && mouseY >= resultY && mouseY < resultY + 22)
                hoveredStack = activeVariant->resultStack;
        }
    }

    // 2.5 Inventory items
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 9; ++c)
        {
            const int slotIndex = (r < 3) ? (9 + r * 9 + c) : c;
            const int sx = invX + c * 18;
            const int sy = invY + r * 18 + (r == 3 ? 3 : 0);

            ItemStack *st = (inventory != nullptr && inventory->mainInventory != nullptr)
                                ? inventory->mainInventory[slotIndex]
                                : nullptr;
            if (st != nullptr && st->isValid())
            {
                itemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine, st, sx + 1, sy + 1);
                itemRenderer->renderItemOverlayIntoGUI(fontRenderer, mc->renderEngine, st, sx + 1, sy + 1);

                if (mouseX >= sx && mouseX < sx + 18 && mouseY >= sy && mouseY < sy + 18)
                    hoveredStack = st;
            }

            // The lifted stack keeps a tint until it lands somewhere: the
            // grab is the strip's only "cursor item" and it lives in its own
            // slot until the swap, so it needs a marker to be findable.
            if (grabbedSlotIndex == slotIndex)
                drawRect(sx + 1, sy + 1, sx + 17, sy + 17, 0x5020a0ff);
        }
    }

    renderDisable(RenderCapability::RescaleNormal);
    RenderHelper::disableStandardItemLighting();
    renderPopMatrix();

    // =============================================================
    // PASS 3: Overlays, Missing Markers, Popup & Cursor (Unlit)
    // =============================================================
    renderDisable(RenderCapability::Lighting);

    // Missing ingredient exclamation marks
    if (activeVariant != nullptr)
    {
        const int rows = is2x2Mode ? 2 : 3;
        const int cols = is2x2Mode ? 2 : 3;
        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < cols; ++c)
            {
                const int slot = r * cols + c;
                const int sx = matrixX + c * 18;
                const int sy = matrixY + r * 18;
                if (activeVariant->gridStacks[slot] != nullptr && !m_cachedSlotHasIngredient[slot])
                {
                    fontRenderer->drawString("!", sx + 2, sy + 1, 0xffff20);
                }
            }
        }

        if (!canCraft)
            fontRenderer->drawString("!", resultX + 3, resultY + 2, 0xffff20);
    }

    // Variant dropdown popup
    if (activeGroupIdx >= 0 && activeGroupIdx < currentCat.groupCount)
    {
        const RecipeGroup &grp = currentCat.groups[activeGroupIdx];
        const int_t gx = activeSlotX;
        const int_t gy = activeSlotY;
        const int_t curVar = selectedVariant[selectedCategory][activeGroupIdx];

        if (grp.variantCount > 1)
        {
            const int_t popTop = (curVar > 0) ? (gy - 28) : (gy - 12);
            const int_t popBottom = (curVar < grp.variantCount - 1) ? (gy + 46) : (gy + 28);

            // Drop shadow
            drawRect(gx - 2, popTop + 2, gx + 22, popBottom + 4, 0x60000000);
            // Outer dark border
            drawRect(gx - 2, popTop, gx + 20, popBottom + 2, 0xff343434);
            // Inner panel background
            drawRect(gx - 1, popTop + 1, gx + 19, popBottom + 1, 0xffc0c0c0);
            // 3D bevel highlights
            drawRect(gx - 1, popTop + 1, gx + 19, popTop + 2, 0xffffffff);
            drawRect(gx - 1, popTop + 1, gx, popBottom + 1, 0xffffffff);
            drawRect(gx + 18, popTop + 1, gx + 19, popBottom + 1, 0xff606060);
            drawRect(gx - 1, popBottom, gx + 19, popBottom + 1, 0xff606060);

            // Up indicator
            fontRenderer->drawString("^", gx + 7, (curVar > 0) ? (gy - 26) : (gy - 10), curVar > 0 ? 0x202020 : 0x808080);

            // Previous variant preview
            if (curVar > 0 && grp.variants[curVar - 1].resultStack != nullptr)
            {
                drawSlotRect(gx, gy - 18);
                RenderHelper::enableGUIStandardItemLighting();
                renderPushMatrix();
                renderEnable(RenderCapability::RescaleNormal);
                itemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine,
                                                grp.variants[curVar - 1].resultStack, gx + 1, gy - 17);
                renderDisable(RenderCapability::RescaleNormal);
                RenderHelper::disableStandardItemLighting();
                renderPopMatrix();
                renderDisable(RenderCapability::Lighting);

                if (mouseX >= gx && mouseX < gx + 18 && mouseY >= gy - 18 && mouseY < gy)
                    hoveredStack = grp.variants[curVar - 1].resultStack;
            }

            // Redraw active center slot and item inside popup
            drawSlotRect(gx, gy);
            if (grp.variants[curVar].resultStack != nullptr)
            {
                RenderHelper::enableGUIStandardItemLighting();
                renderPushMatrix();
                renderEnable(RenderCapability::RescaleNormal);
                itemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine,
                                                grp.variants[curVar].resultStack, gx + 1, gy + 1);
                renderDisable(RenderCapability::RescaleNormal);
                RenderHelper::disableStandardItemLighting();
                renderPopMatrix();
                renderDisable(RenderCapability::Lighting);
            }

            // Next variant preview
            if (curVar < grp.variantCount - 1 && grp.variants[curVar + 1].resultStack != nullptr)
            {
                drawSlotRect(gx, gy + 18);
                RenderHelper::enableGUIStandardItemLighting();
                renderPushMatrix();
                renderEnable(RenderCapability::RescaleNormal);
                itemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine,
                                                grp.variants[curVar + 1].resultStack, gx + 1, gy + 19);
                renderDisable(RenderCapability::RescaleNormal);
                RenderHelper::disableStandardItemLighting();
                renderPopMatrix();
                renderDisable(RenderCapability::Lighting);

                if (mouseX >= gx && mouseX < gx + 18 && mouseY >= gy + 18 && mouseY < gy + 36)
                    hoveredStack = grp.variants[curVar + 1].resultStack;
            }

            // Down indicator
            fontRenderer->drawString("v", gx + 7, (curVar < grp.variantCount - 1) ? (gy + 37) : (gy + 19), curVar < grp.variantCount - 1 ? 0x202020 : 0x808080);
        }

        // Selection cursor centered on active recipe carousel slot, and on the
        // strip slot the pad cursor is on while that zone is active (same cursor,
        // one screen, whichever zone owns the D-pad).
        if (invZoneActive)
        {
            const int_t stripCursorY = invY + invCursorRow * 18 + (invCursorRow == 3 ? 3 : 0);
            legacyDrawSelectionCursorCentered(mc, invX + invCursorCol * 18 + 9, stripCursorY + 9, 20, zLevel + 64.0f);
        }
        else
        {
            legacyDrawSelectionCursorCentered(mc, gx + 9, gy + 9, 20, zLevel + 64.0f);
        }
    }

    // Tooltip display
    if (hoveredStack != nullptr)
        drawTooltip(hoveredStack, mouseX, mouseY);

    // Bottom Action Hints (static strings to avoid runtime heap allocation).
    // Every platform declares its arrays at function scope and draws exactly
    // once inside its own branch. An earlier merge left a second, shared
    // draw after this block: it double-drew the row on the consoles, and on
    // PS2 it referenced arrays that only existed inside the if/else blocks
    // -- out of scope there, so it broke the PS2 build outright.
#if PLATFORM_PS2
    static const std::string buttons2x2[] = {"L1/R1", "D-Pad", "Cross", "Triangle", "Circle"};
    static const std::string actions2x2[] = {uiText("Category"), uiText("Navigate"), uiText("Craft/Move"), uiText("Inventory"), uiText("Back")};
    static const std::string buttons3x3[] = {"L1/R1", "D-Pad", "Cross", "Circle"};
    static const std::string actions3x3[] = {uiText("Category"), uiText("Navigate"), uiText("Craft/Move"), uiText("Back")};
    if (is2x2Mode)
        drawControlHintRow(mc, width, legacyHintRowY(height), buttons2x2, actions2x2, 5);
    else
        drawControlHintRow(mc, width, legacyHintRowY(height), buttons3x3, actions3x3, 4);
#elif PLATFORM_3DS || PLATFORM_WII
    static const std::string buttons[] = {"L/R", "D-Pad", "A", "B"};
    static const std::string actions[] = {uiText("Category"), uiText("Navigate"), uiText("Craft/Move"), uiText("Back")};
    drawControlHintRow(mc, width, legacyHintRowY(height), buttons, actions, 4);
#else
    static const std::string buttons[] = {"Q/E", "Arrows", "Enter", "Esc"};
    static const std::string actions[] = {uiText("Category"), uiText("Navigate"), uiText("Craft/Move"), uiText("Back")};
    drawControlHintRow(mc, width, legacyHintRowY(height), buttons, actions, 4);
#endif
}

void LegacyCraftingScreen::updateScreen()
{
    GuiScreen::updateScreen();
}

#if PLATFORM_3DS
void LegacyCraftingScreen::legacyNavigationRepeat(const PlatformTextInputSnapshot &pad)
{
    // Hold-to-repeat for the navigation bits -- the D-pad's own bits, which
    // the circle pad also rides while a screen is open (DsInput's
    // menuStickNavBits feeds both channels). The 250 ms/90 ms cadence is
    // ContainerSlotNavigator's, so the carousel and the strip scroll while a
    // direction is held instead of stepping once per push.
    constexpr int kRepeatDelayMs = 250;
    constexpr int kRepeatIntervalMs = 90;

    const unsigned int heldBits =
        pad.held & (PLATFORM_TEXT_LEFT | PLATFORM_TEXT_RIGHT | PLATFORM_TEXT_UP | PLATFORM_TEXT_DOWN);
    // A fresh press steps through the edge handlers in
    // handleSpecializedMenuInput; arm the delay here and let this frame pass.
    if ((pad.pressed & heldBits) != 0)
    {
        navRepeatMs = consoleInputNowMs() + kRepeatDelayMs;
        return;
    }

    const int now = consoleInputNowMs();
    if (heldBits == 0 || now < navRepeatMs)
        return;

    int dirX = 0;
    int dirY = 0;
    if (heldBits & PLATFORM_TEXT_LEFT)       dirX = -1;
    else if (heldBits & PLATFORM_TEXT_RIGHT) dirX = 1;
    else if (heldBits & PLATFORM_TEXT_UP)    dirY = 1;
    else if (heldBits & PLATFORM_TEXT_DOWN)  dirY = -1;
    navRepeatMs = now + kRepeatIntervalMs;
    if (dirX != 0 || dirY != 0)
        handleNavigation(dirX, dirY);
}
#endif

void LegacyCraftingScreen::handleSpecializedMenuInput()
{
    // The specialized hook can be called on a screen that was replaced earlier
    // in the same frame's queue drain; a stale recipe click then would craft
    // into the wrong window.
    if (mc != nullptr && mc->currentScreen != this)
        return;

    if (inventory != nullptr)
    {
        if (inventory->inventoryChanged || m_craftingStateDirty)
        {
            inventory->inventoryChanged = false;
            updateCraftingState();
        }
        else
        {
            uint32_t h = computeInventoryHash();
            if (h != m_cachedInventoryHash)
            {
                updateCraftingState();
            }
        }
    }
    else if (m_craftingStateDirty)
    {
        updateCraftingState();
    }

#if PLATFORM_PS2
    const Ps2PadSnapshot &ps2Pad = ps2PadGetSnapshot(getOwnerPlayerIndex());
    if (ps2Pad.connected)
    {
        unsigned short pressed = ps2Pad.pressed;
        if (ps2ActionReleaseLatch)
        {
            pressed &= ~PS2_PAD_CROSS;
            if ((ps2Pad.held & PS2_PAD_CROSS) == 0)
                ps2ActionReleaseLatch = false;
        }

        if (pressed & PS2_PAD_L1) changeCategory(-1);
        if (pressed & PS2_PAD_R1) changeCategory(1);
        if (pressed & PS2_PAD_LEFT)  handleNavigation(-1, 0);
        if (pressed & PS2_PAD_RIGHT) handleNavigation(1, 0);
        if (pressed & PS2_PAD_UP)    handleNavigation(0, 1);
        if (pressed & PS2_PAD_DOWN)  handleNavigation(0, -1);

        // In the strip zone the action button moves items; on the recipes it
        // crafts.
        if (pressed & PS2_PAD_TRIANGLE)
        {
            if (is2x2Mode)
            {
                EntityPlayer *p = entityPlayer ? entityPlayer : (mc ? static_cast<EntityPlayer*>(mc->thePlayer) : nullptr);
                if (p != nullptr)
                {
                    if (mc->isSplitScreenActive())
                        mc->displayPlayerScreen(getOwnerPlayerIndex(), new GuiInventory(p));
                    else
                        mc->displayGuiScreen(new GuiInventory(p));
                    return;
                }
            }
        }

        if (pressed & PS2_PAD_CROSS)
        {
            if (invZoneActive)
                clickStripSlot(stripSlotIndex(static_cast<int>(invCursorRow), static_cast<int>(invCursorCol)));
            else
                craftCurrentRecipe();
        }

        if (pressed & (PS2_PAD_CIRCLE | PS2_PAD_SQUARE))
        {
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);
            if (mc != nullptr && mc->isSplitScreenActive())
                mc->closePlayerScreen(getOwnerPlayerIndex());
            else
                legacyCloseScreen();
            return;
        }

        // Hold-to-repeat crafting: recipes only -- a held button must not
        // rattle off swaps in the strip zone.
        if (!ps2ActionReleaseLatch && (ps2Pad.held & PS2_PAD_CROSS) != 0 && !invZoneActive)
        {
            ++craftHoldTicks;
            if (craftHoldTicks >= 10 && (craftHoldTicks % 3 == 0))
                craftCurrentRecipe();
        }
        else
        {
            craftHoldTicks = 0;
        }
        return;
    }
#endif

    const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
    if (pad.connected)
    {
        if (pad.pressed & PLATFORM_TEXT_PREV_PAGE) changeCategory(-1);
        if (pad.pressed & PLATFORM_TEXT_NEXT_PAGE) changeCategory(1);
        if (pad.pressed & PLATFORM_TEXT_LEFT)  handleNavigation(-1, 0);
        if (pad.pressed & PLATFORM_TEXT_RIGHT) handleNavigation(1, 0);
        if (pad.pressed & PLATFORM_TEXT_UP)    handleNavigation(0, 1);
        if (pad.pressed & PLATFORM_TEXT_DOWN)  handleNavigation(0, -1);
#if PLATFORM_3DS
        // Held directions repeat (D-pad and circle pad alike -- the stick
        // rides these same bits through DsInput's menu channel).
        legacyNavigationRepeat(pad);
#endif
        // Category tabs ride the same shoulders the virtual keyboard uses as
        // SPACE/SHIFT (3DS L/R, GC-pad X/Z, Wiimote MINUS/2) -- the same
        // role PS2's L1/R1 play above. Before this the tabs had no button on
        // any pad in the generic branch.
        if (pad.pressed & PLATFORM_TEXT_SPACE) changeCategory(-1);
        if (pad.pressed & PLATFORM_TEXT_SHIFT) changeCategory(1);
        // Strip zone: the action button grabs/swaps a stack; recipes: it crafts.
        if (pad.pressed & PLATFORM_TEXT_TYPE)
        {
            if (invZoneActive)
                clickStripSlot(stripSlotIndex(static_cast<int>(invCursorRow), static_cast<int>(invCursorCol)));
            else
                craftCurrentRecipe();
        }

        if (pad.pressed & PLATFORM_TEXT_CLOSE)
        {
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);
            if (mc != nullptr && mc->isSplitScreenActive())
                mc->closePlayerScreen(getOwnerPlayerIndex());
            else
                legacyCloseScreen();
            return;
        }

        if (pad.held & PLATFORM_TEXT_TYPE && !invZoneActive)
        {
            ++craftHoldTicks;
            if (craftHoldTicks >= 10 && (craftHoldTicks % 3 == 0))
                craftCurrentRecipe();
        }
        else
        {
            craftHoldTicks = 0;
        }
    }
}

void LegacyCraftingScreen::keyTyped(char_t c, int_t key)
{
    const bool isCloseKey = (key == lwjgl::Keyboard::KEY_ESCAPE) ||
        (mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->keyBindCrafting != nullptr && key == mc->gameSettings->keyBindCrafting->keyCode);
    if (isCloseKey)
    {
        if (mc != nullptr && mc->sndManager != nullptr)
            mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);
        if (mc != nullptr && mc->isSplitScreenActive())
            mc->closePlayerScreen(getOwnerPlayerIndex());
        else
            legacyCloseScreen();
        return;
    }

    // The keyboard handlers below are desktop's. On the consoles the same
    // buttons arrive through handleSpecializedMenuInput() as PLATFORM_TEXT_*
    // edges, AND the menu channel queues the D-pad/A as keyboard events that
    // end up here -- handling them twice made every D-pad step skip two slots
    // and every craft press produce two crafts (2026-09-28, 3DS). ESC stays
    // for everyone: B on 3DS, CIRCLE on PS2 and PLUS on Wii all reach this
    // screen as KEY_ESCAPE, which is the console "back" convention.
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM) && !defined(CTR_PLATFORM)
    // Toggle to Inventory if in 2x2 hand crafting mode, or close workbench if in 3x3 mode
    const bool isInventoryKey = (key == lwjgl::Keyboard::KEY_I ||
        (mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->keyBindInventory != nullptr && key == mc->gameSettings->keyBindInventory->keyCode));
    if (isInventoryKey)
    {
        if (is2x2Mode)
        {
            EntityPlayer *p = entityPlayer ? entityPlayer : (mc ? static_cast<EntityPlayer*>(mc->thePlayer) : nullptr);
            if (p != nullptr)
            {
                if (mc->isSplitScreenActive())
                    mc->displayPlayerScreen(getOwnerPlayerIndex(), new GuiInventory(p));
                else
                    mc->displayGuiScreen(new GuiInventory(p));
                return;
            }
        }
        else
        {
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);
            if (mc != nullptr && mc->isSplitScreenActive())
                mc->closePlayerScreen(getOwnerPlayerIndex());
            else
                legacyCloseScreen();
            return;
        }
    }

    if (key == lwjgl::Keyboard::KEY_Q || key == lwjgl::Keyboard::KEY_PRIOR)
    {
        changeCategory(-1);
        return;
    }
    if (key == lwjgl::Keyboard::KEY_E || key == lwjgl::Keyboard::KEY_NEXT)
    {
        changeCategory(1);
        return;
    }

    if (key == lwjgl::Keyboard::KEY_LEFT || key == lwjgl::Keyboard::KEY_A)
    {
        handleNavigation(-1, 0);
        return;
    }
    if (key == lwjgl::Keyboard::KEY_RIGHT || key == lwjgl::Keyboard::KEY_D)
    {
        handleNavigation(1, 0);
        return;
    }
    if (key == lwjgl::Keyboard::KEY_UP || key == lwjgl::Keyboard::KEY_W)
    {
        handleNavigation(0, 1);
        return;
    }
    if (key == lwjgl::Keyboard::KEY_DOWN || key == lwjgl::Keyboard::KEY_S)
    {
        handleNavigation(0, -1);
        return;
    }

    if (key == lwjgl::Keyboard::KEY_RETURN || key == lwjgl::Keyboard::KEY_SPACE)
    {
        // Desktop shares the pad's zone semantics: Enter on the strip moves
        // items, on the recipes it crafts.
        if (invZoneActive)
            clickStripSlot(stripSlotIndex(static_cast<int>(invCursorRow), static_cast<int>(invCursorCol)));
        else
            craftCurrentRecipe();
        return;
    }

#endif

    GuiScreen::keyTyped(c, key);
}

void LegacyCraftingScreen::mouseClicked(int_t mouseX, int_t mouseY, int_t button)
{
    GuiScreen::mouseClicked(mouseX, mouseY, button);

    // 1. Click on Category Tabs
    for (int t = 0; t < visibleCategoryCount; ++t)
    {
        const int_t tabX = guiLeft + 16 + t * 32;
        const int_t tabY = guiTop + 4;
        if (mouseX >= tabX && mouseX < tabX + 28 && mouseY >= tabY && mouseY < tabY + 22)
        {
            selectedVisibleTab = t;
            selectedCategory = visibleCategoryIndices[t];
            ensureSelectionVisible();
            updateCraftingState();
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            return;
        }
    }

    // 2. Click on Recipe Carousel Items
    const RecipeCategory *cats = getCategoriesTable(is2x2Mode);
    const int_t panelTop = guiTop + 24;
    const int_t carouselY = panelTop + 16;
    const int_t visibleCount = 10;
    const int_t carouselW = visibleCount * 22;
    const int_t carouselStartX = guiLeft + (xSize - carouselW) / 2;
    const int_t scroll = scrollOffset[selectedCategory];
    const RecipeCategory &currentCat = cats[selectedCategory];

    for (int i = 0; i < visibleCount; ++i)
    {
        const int_t gIdx = scroll + i;
        if (gIdx >= currentCat.groupCount) break;

        const int_t gx = carouselStartX + i * 22;
        const int_t gy = carouselY;

        // If clicking on active group's variant arrows or popups
        if (gIdx == selectedGroup[selectedCategory] && currentCat.groups[gIdx].variantCount > 1)
        {
            const RecipeGroup &grp = currentCat.groups[gIdx];
            const int_t curVar = selectedVariant[selectedCategory][gIdx];
            const int_t popTop = (curVar > 0) ? (gy - 28) : (gy - 12);
            const int_t popBottom = (curVar < grp.variantCount - 1) ? (gy + 46) : (gy + 28);

            if (mouseX >= gx - 2 && mouseX < gx + 20)
            {
                if (mouseY >= popTop && mouseY < gy)
                {
                    changeVariant(-1);
                    return;
                }
                if (mouseY >= gy + 18 && mouseY <= popBottom)
                {
                    changeVariant(1);
                    return;
                }
            }
        }

        if (mouseX >= gx && mouseX < gx + 18 && mouseY >= gy && mouseY < gy + 18)
        {
            selectedGroup[selectedCategory] = gIdx;
            ensureSelectionVisible();
            updateCraftingState();
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
            return;
        }
    }

    // 3. Click on Result Slot -> Craft
    const int_t matrixX = is2x2Mode ? (guiLeft + 12) : (guiLeft + 9);
    const int_t matrixY = is2x2Mode ? (panelTop + 68) : (panelTop + 62);
    const int_t resultX = is2x2Mode ? (guiLeft + 82) : (guiLeft + 90);
    const int_t resultY = is2x2Mode ? (matrixY + 7) : (matrixY + 16);

    if (mouseX >= resultX && mouseX < resultX + 22 && mouseY >= resultY && mouseY < resultY + 22)
    {
        craftCurrentRecipe();
        return;
    }

    // 4. Click on the inventory strip -> grab/swap. The same state the pad's
    // strip cursor uses, so touch and pad move the same stacks; the pad's
    // zone cursor itself is untouched (touch acts where it lands, as it does
    // on the tabs and recipes above).
    const int_t stripX = guiLeft + 104;
    const int_t stripY = panelTop + 60;
    if (mouseX >= stripX && mouseX < stripX + 9 * 18 && mouseY >= stripY && mouseY < stripY + 4 * 18 + 3)
    {
        int_t col = (mouseX - stripX) / 18;
        int_t row = (mouseY - stripY) / 18;
        if (col < 0) col = 0;
        if (col > 8) col = 8;
        if (row < 0) row = 0;
        if (row > 3) row = 3; // the hotbar row sits 3px lower; the gap rounds down onto it
        clickStripSlot(stripSlotIndex(static_cast<int>(row), static_cast<int>(col)));
        return;
    }
}
