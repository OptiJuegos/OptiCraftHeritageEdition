#include "ComponentStrongholdStairs2.h"

#include "ComponentStrongholdPortalRoom.h"

ComponentStrongholdStairs2::ComponentStrongholdStairs2(int_t, Random &random, int_t x, int_t z)
	: ComponentStrongholdStairs(0, random, x, z)
{
}

ChunkPosition *ComponentStrongholdStairs2::getCenter() const
{
	return portalRoom != nullptr ? portalRoom->getCenter() : ComponentStrongholdStairs::getCenter();
}
