#include "ItemLeaves.h"
#include "Block.h"
#include "ColorizerFoliage.h"
#include "BlockLeaves.h"

ItemLeaves::ItemLeaves(int i)
    : ItemBlock(i) {
    setMaxDamage(0);
    setHasSubtypes(true);
}

int ItemLeaves::getMetadata(int i) {
    return i | 4;
}

int ItemLeaves::getIconFromDamage(int i) {
    return Block::leaves->getBlockTextureFromSideAndMetadata(0, i);
}

int ItemLeaves::getColorFromDamage(int i) {
    return getColorFromDamage(i, 0);
}

int ItemLeaves::getColorFromDamage(int i, int renderPass) {
    (void)renderPass;
    if ((i & 1) == 1) {
        return ColorizerFoliage::getFoliageColorPine();
    }
    if ((i & 2) == 2) {
        return ColorizerFoliage::getFoliageColorBirch();
    }
    return ColorizerFoliage::getFoliageColorBasic();
}
