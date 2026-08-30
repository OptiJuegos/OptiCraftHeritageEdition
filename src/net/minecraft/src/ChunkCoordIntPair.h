#pragma once

#include <cstddef>

#include "java/Type.h"

class ChunkPosition;

// net.minecraft.src.ChunkCoordIntPair
class ChunkCoordIntPair
{
public:
	ChunkCoordIntPair(int_t i, int_t j);

	static long_t chunkXZ2Int(int_t i, int_t j);
	static ulong_t chunkXZ2Long(int_t i, int_t j);
	int_t hashCode() const;
	bool equals(const ChunkCoordIntPair *obj) const;
	int_t getCenterXPos() const;
	int_t getCenterZPos() const;
	ChunkPosition* getChunkPosition(int_t y) const;

	const int_t chunkXPos;
	const int_t chunkZPos;

	bool operator<(const ChunkCoordIntPair &o) const
	{
		if (chunkXPos != o.chunkXPos) return chunkXPos < o.chunkXPos;
		return chunkZPos < o.chunkZPos;
	}
	bool operator==(const ChunkCoordIntPair &o) const
	{
		return chunkXPos == o.chunkXPos && chunkZPos == o.chunkZPos;
	}
};

struct ChunkCoordIntPairHash
{
	std::size_t operator()(const ChunkCoordIntPair *a) const
	{
		return (std::size_t)(uint_t)a->hashCode();
	}
};

struct ChunkCoordIntPairEqual
{
	bool operator()(const ChunkCoordIntPair *a, const ChunkCoordIntPair *b) const
	{
		return a->equals(b);
	}
};

struct ChunkCoordIntPairValueHash
{
	std::uint32_t operator()(const ChunkCoordIntPair &pair) const
	{
		return static_cast<std::uint32_t>(pair.hashCode());
	}
};

struct ChunkCoordIntPairValueEqual
{
	bool operator()(const ChunkCoordIntPair &a, const ChunkCoordIntPair &b) const
	{
		return a == b;
	}
};
