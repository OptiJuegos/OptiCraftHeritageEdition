#include "ItemPiston.h"

ItemPiston::ItemPiston(int i)
    : ItemBlock(i) {
}

int ItemPiston::getMetadata(int i) {
    return 7;
}
