#pragma once

#include "ItemFood.h"

class ItemCookie : public ItemFood {
public:
    ItemCookie(int id, int healAmount, bool isWolfsFavoriteMeat, int maxStack);
};
