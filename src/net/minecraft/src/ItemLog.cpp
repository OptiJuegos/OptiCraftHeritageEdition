#include "ItemLog.h"
#include "Block.h"

ItemLog::ItemLog(int i)
    : ItemBlock(i) {
    setMaxDamage(0);
    setHasSubtypes(true);
}

int ItemLog::getIconFromDamage(int i) {
    return Block::wood->getBlockTextureFromSideAndMetadata(2, i);
}

int ItemLog::getPlacedBlockMetadata(int i) {
    return i;
}
