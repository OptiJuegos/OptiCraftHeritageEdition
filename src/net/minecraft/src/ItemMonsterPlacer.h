#pragma once

#include "Item.h"

// net.minecraft.src.ItemMonsterPlacer
class ItemMonsterPlacer : public Item
{
public:
    explicit ItemMonsterPlacer(int id);
    std::string getItemDisplayName(ItemStack *stack) override;
    int getColorFromDamage(int damage, int renderPass) override;
    bool func_46058_c() override;
    int func_46057_a(int damage, int renderPass) override;
    bool onItemUse(ItemStack *stack, EntityPlayer *player, World *world, int x, int y, int z, int side) override;

    static bool spawnCreature(World *world, int_t entityId, double x, double y, double z);
};
