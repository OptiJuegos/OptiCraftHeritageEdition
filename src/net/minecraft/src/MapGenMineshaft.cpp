#include "MapGenMineshaft.h"

#include <algorithm>
#include <cstdlib>

#include "StructureMineshaftStart.h"
#include "java/Arithmetic.h"

bool MapGenMineshaft::canSpawnStructureAtCoords(int_t chunkX, int_t chunkZ)
{
	return rand.nextInt(100) == 0 &&
	       rand.nextInt(80) < std::max(JavaArithmetic::intAbs(chunkX), JavaArithmetic::intAbs(chunkZ));
}

StructureStart *MapGenMineshaft::getStructureStart(int_t chunkX, int_t chunkZ)
{
	return new StructureMineshaftStart(worldObj, rand, chunkX, chunkZ);
}
