#pragma once

#include "java/Type.h"
#include <algorithm>
#include <vector>

class Entity;

// Ownership is keyed by allocation. Network identity belongs to entityHash.
struct WorldClientEntityHash
{
	int_t operator()(Entity *entity) const
	{
		return static_cast<int_t>(static_cast<uint_t>(reinterpret_cast<std::uintptr_t>(entity) >> 4));
	}
};

struct WorldClientEntityEqual
{
	bool operator()(Entity *lhs, Entity *rhs) const { return lhs == rhs; }
};

// A hash-set retry must rotate explicitly; reinsertion only changes bucket heads.
inline void selectClientEntityRetryBatch(std::vector<Entity *> &entities,
                                        std::size_t &cursor, std::size_t budget)
{
	if (entities.empty())
	{
		cursor = 0;
		return;
	}
	cursor %= entities.size();
	std::rotate(entities.begin(), entities.begin() + cursor, entities.end());
	cursor = (cursor + std::min(budget, entities.size())) % entities.size();
	if (entities.size() > budget)
		entities.resize(budget);
}
