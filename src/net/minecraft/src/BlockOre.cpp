#include "BlockOre.h"
#include "Material.h"
#include "Item.h"
#include "java/Random.h"

BlockOre::BlockOre(int_t i, int_t j) : Block(i, j, Material::rock)
{
}

int_t BlockOre::idDropped(int_t i, Random &random, int_t fortune)
{
    if (blockID == Block::oreCoal->blockID)    return Item::coal->shiftedIndex;
    if (blockID == Block::oreDiamond->blockID) return Item::diamond->shiftedIndex;
    if (blockID == Block::oreLapis->blockID)   return Item::dyePowder->shiftedIndex;
    return blockID;
}

int_t BlockOre::quantityDropped(Random &random)
{
    if (blockID == Block::oreLapis->blockID)
        return 4 + random.nextInt(5);
    return 1;
}

int_t BlockOre::quantityDroppedWithBonus(int_t fortune, Random &random)
{
    if (fortune > 0 && blockID != idDropped(0, random, fortune))
    {
        int_t multiplier = random.nextInt(fortune + 2) - 1;
        if (multiplier < 0)
        {
            multiplier = 0;
        }
        return quantityDropped(random) * (multiplier + 1);
    }
    return quantityDropped(random);
}

int_t BlockOre::damageDropped(int_t i)
{
    return blockID != Block::oreLapis->blockID ? 0 : 4;
}
