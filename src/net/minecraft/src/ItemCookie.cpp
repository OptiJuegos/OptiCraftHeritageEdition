#include "ItemCookie.h"

ItemCookie::ItemCookie(int id, int healAmount, bool isWolfsFavoriteMeat, int maxStack) :
	ItemFood(id, healAmount, 0.1f, isWolfsFavoriteMeat)
{
	maxStackSize = maxStack;
}
