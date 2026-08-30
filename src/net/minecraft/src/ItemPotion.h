#pragma once

#include <unordered_map>
#include <vector>

#include "Item.h"
#include "PotionEffect.h"

// net.minecraft.src.ItemPotion
class ItemPotion : public Item
{
public:
    explicit ItemPotion(int id);

    const std::vector<PotionEffect> &getEffects(ItemStack *itemstack);
    const std::vector<PotionEffect> &getEffects(int damage);
    ItemStack *onFoodEaten(ItemStack *itemstack, World *world, EntityPlayer *player) override;
    int getMaxItemUseDuration(ItemStack *itemstack) override;
    EnumAction getItemUseAction(ItemStack *itemstack) override;
    ItemStack *onItemRightClick(ItemStack *itemstack, World *world, EntityPlayer *player) override;
    bool onItemUse(ItemStack *itemstack, EntityPlayer *player, World *world, int x, int y, int z, int side) override;
    int getIconFromDamage(int damage) override;
    int func_46057_a(int damage, int renderPass) override;
    int getColorFromDamage(int damage) override;
    int getColorFromDamage(int damage, int renderPass) override;
    std::string getItemDisplayName(ItemStack *itemstack) override;
    void addInformation(ItemStack *itemstack, std::vector<std::string>& information) override;
    bool func_46058_c() override;
    bool hasEffect(ItemStack *itemstack) override;
    bool isEffectInstant(int damage);

    static bool isSplash(int damage);

private:
    std::unordered_map<int, std::vector<PotionEffect>> effectCache;
};
