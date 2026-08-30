#pragma once

#include "ItemBlock.h"

class ItemPiston : public ItemBlock {
public:
    ItemPiston(int id);

    int getMetadata(int damage) override;
};
