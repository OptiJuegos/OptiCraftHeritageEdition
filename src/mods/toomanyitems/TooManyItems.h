#pragma once

#include "java/Type.h"
#include <vector>
#include <string>

class Minecraft;
class GuiContainer;
class ItemStack;
class EntityPlayer;
class World;
class FontRenderer;

// TooManyItems (TMI) para OptiCraft Heritage Edition (PS2 / PC / Wii)
class TooManyItems
{
public:
    static void init();
    static bool isEnabled();
    static void setEnabled(bool enabled);
    static void toggle();

    static bool isDeleteMode();
    static void setDeleteMode(bool del);

    static void draw(GuiContainer *gui, int_t mouseX, int_t mouseY);
    static bool mouseClicked(GuiContainer *gui, int_t mouseX, int_t mouseY, int_t button);
    static bool keyTyped(char_t c, int_t key);
    static void updateInput(GuiContainer *gui);

    // Cheats y Utilidades
    static void setTime(World *world, int64_t time);
    static void toggleRain(World *world);
    static void toggleCreative(Minecraft *mc);
    static void healPlayer(EntityPlayer *player);
    static void saveInventory(EntityPlayer *player, int slot);
    static void loadInventory(EntityPlayer *player, int slot);

private:
    static bool s_enabled;
    static bool s_deleteMode;
    static int_t s_currentPage;
    static int_t s_totalPages;
    static std::vector<ItemStack*> s_items;
    static bool s_initialized;

    // Guardado de inventario (hasta 4 slots de save)
    static std::vector<ItemStack*> s_savedStates[4];
    static bool s_stateSaved[4];

    static void populateItems();
};
