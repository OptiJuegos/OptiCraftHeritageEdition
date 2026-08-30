#pragma once

#include "ItemBlock.h"

class ItemLog : public ItemBlock {
public:
    ItemLog(int id);

    int getIconFromDamage(int damage) override;
    int getPlacedBlockMetadata(int damage) override;
};
