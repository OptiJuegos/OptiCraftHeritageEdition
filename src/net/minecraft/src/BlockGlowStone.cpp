#include "BlockGlowStone.h"
#include "Item.h"
#include "java/Random.h"
#include "MathHelper.h"

BlockGlowStone::BlockGlowStone(int_t i, int_t j, Material *material)
    : Block(i, j, material)
{
}

int_t BlockGlowStone::quantityDroppedWithBonus(int_t fortune, Random &random)
{
    return MathHelper::clamp_int(quantityDropped(random) + random.nextInt(fortune + 1), 1, 4);
}

int_t BlockGlowStone::quantityDropped(Random &random)
{
    return 2 + random.nextInt(3);
}

int_t BlockGlowStone::idDropped(int_t i, Random &random)
{
    return Item::lightStoneDust->shiftedIndex;
}
