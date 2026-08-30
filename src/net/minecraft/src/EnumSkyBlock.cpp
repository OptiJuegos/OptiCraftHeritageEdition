#include "EnumSkyBlock.h"

EnumSkyBlock::EnumSkyBlock(int_t j) :
	defaultLightValue(j)
{
}

EnumSkyBlock *const EnumSkyBlock::Sky = new EnumSkyBlock(15);
EnumSkyBlock *const EnumSkyBlock::Block = new EnumSkyBlock(0);

EnumSkyBlock *EnumSkyBlock::rotationPitch[2] = {
	Sky, Block
};
