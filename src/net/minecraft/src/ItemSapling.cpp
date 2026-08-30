#include "ItemSapling.h"
#include "Block.h"

ItemSapling::ItemSapling(int i)
    : ItemBlock(i) {
    setMaxDamage(0);
    setHasSubtypes(true);
}

int ItemSapling::getMetadata(int i) {
    return i;
}

int ItemSapling::getIconFromDamage(int i) {
    return Block::sapling->getBlockTextureFromSideAndMetadata(0, i);
}
