#pragma once

#include "java/Type.h"

// net.minecraft.src.ChunkCoordinates
class ChunkCoordinates
{
public:
	ChunkCoordinates();
	ChunkCoordinates(int_t i, int_t j, int_t k);
	ChunkCoordinates(const ChunkCoordinates *chunkcoordinates);

	bool equals(const ChunkCoordinates *obj) const;
	int_t hashCode() const;
	int_t compareChunkCoordinate(const ChunkCoordinates *chunkcoordinates) const;
	void set(int_t i, int_t j, int_t k);
	double getSqDistanceTo(int_t i, int_t j, int_t k) const;
	double getEuclideanDistanceTo(int_t i, int_t j, int_t k) const;
	float getDistanceSquared(int_t i, int_t j, int_t k) const;
	int_t compareTo(const ChunkCoordinates *obj) const;

	int_t x = 0;
	int_t y = 0;
	int_t z = 0;
};
