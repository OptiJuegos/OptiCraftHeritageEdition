#include "ItemFood.h"

#include "EntityPlayer.h"
#include "FoodStats.h"
#include "ItemStack.h"
#include "PotionEffect.h"
#include "World.h"

ItemFood::ItemFood(int id, int heal, float saturation, bool wolfFood) :
	Item(id),
	healAmount(heal),
	saturationModifier(saturation),
	wolfsFavoriteMeat(wolfFood),
	alwaysEdible(false),
	potionId(0),
	potionDuration(0),
	potionAmplifier(0),
	potionEffectProbability(0.0f)
{
}

ItemFood::ItemFood(int id, int heal, bool wolfFood) :
	ItemFood(id, heal, 0.6f, wolfFood)
{
}

ItemStack *ItemFood::onFoodEaten(ItemStack *itemstack, World *world, EntityPlayer *entityplayer)
{
	if (itemstack == nullptr || entityplayer == nullptr || world == nullptr)
		return itemstack;

	--itemstack->stackSize;
	entityplayer->getFoodStats()->addStats(this);
	world->playSoundAtEntity(entityplayer, "random.burp", 0.5f, world->rand.nextFloat() * 0.1f + 0.9f);
	if (!world->multiplayerWorld && potionId > 0 && world->rand.nextFloat() < potionEffectProbability)
		entityplayer->addPotionEffect(new PotionEffect(potionId, potionDuration * 20, potionAmplifier));
	return itemstack;
}

int ItemFood::getMaxItemUseDuration(ItemStack *)
{
	return 32;
}

EnumAction ItemFood::getItemUseAction(ItemStack *)
{
	return EnumAction::eat;
}

ItemStack *ItemFood::onItemRightClick(ItemStack *itemstack, World *, EntityPlayer *entityplayer)
{
	if (entityplayer != nullptr && entityplayer->canEat(alwaysEdible))
		entityplayer->setItemInUse(itemstack, getMaxItemUseDuration(itemstack));
	return itemstack;
}

int ItemFood::getHealAmount() const
{
	return healAmount;
}

float ItemFood::getSaturationModifier() const
{
	return saturationModifier;
}

bool ItemFood::isWolfsFavoriteMeat() const
{
	return wolfsFavoriteMeat;
}

bool ItemFood::isWolfsFavoriteMeatItem() const
{
	return wolfsFavoriteMeat;
}

bool ItemFood::getIsWolfsFavoriteMeat() const
{
	return wolfsFavoriteMeat;
}

ItemFood *ItemFood::setAlwaysEdible()
{
	alwaysEdible = true;
	return this;
}

ItemFood *ItemFood::setPotionEffect(int id, int duration, int amplifier, float probability)
{
	potionId = id;
	potionDuration = duration;
	potionAmplifier = amplifier;
	potionEffectProbability = probability;
	return this;
}
