#include "ChunkCoordIntPair.h"
#include "ChunkPosition.h"
#include "java/Arithmetic.h"

ChunkCoordIntPair::ChunkCoordIntPair(int_t i, int_t j) :
	chunkXPos(i),
	chunkZPos(j)
{
}

long_t ChunkCoordIntPair::chunkXZ2Int(int_t i, int_t j)
{
	const ulong_t bits = static_cast<ulong_t>(static_cast<uint_t>(i)) |
	                   (static_cast<ulong_t>(static_cast<uint_t>(j)) << 32);
	return JavaArithmetic::longFromBits(bits);
}

ulong_t ChunkCoordIntPair::chunkXZ2Long(int_t i, int_t j)
{
	return static_cast<ulong_t>(chunkXZ2Int(i, j));
}

int_t ChunkCoordIntPair::hashCode() const
{
	const uint_t low = static_cast<uint_t>(chunkXPos);
	const uint_t high = static_cast<uint_t>(chunkZPos);
	return JavaArithmetic::intFromBits(low ^ high);
}

bool ChunkCoordIntPair::equals(const ChunkCoordIntPair *obj) const
{
	return obj->chunkXPos == chunkXPos && obj->chunkZPos == chunkZPos;
}

int_t ChunkCoordIntPair::getCenterXPos() const
{
	return JavaArithmetic::intAdd(JavaArithmetic::intShl(chunkXPos, 4), 8);
}

int_t ChunkCoordIntPair::getCenterZPos() const
{
	return JavaArithmetic::intAdd(JavaArithmetic::intShl(chunkZPos, 4), 8);
}

ChunkPosition* ChunkCoordIntPair::getChunkPosition(int_t y) const
{
	return new ChunkPosition(getCenterXPos(), y, getCenterZPos());
}
