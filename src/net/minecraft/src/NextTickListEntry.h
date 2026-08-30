#pragma once

#include <cstddef>

#include "java/Type.h"

// net.minecraft.src.NextTickListEntry
class NextTickListEntry
{
public:
	NextTickListEntry(int_t i, int_t j, int_t k, int_t l);

	bool equals(const NextTickListEntry *nextticklistentry) const;
	int_t hashCode() const;
	NextTickListEntry *setScheduledTime(long_t l);
	int_t comparer(const NextTickListEntry *nextticklistentry) const;
	int_t compareTo(const NextTickListEntry *obj) const;

private:
	static long_t nextTickEntryID;

public:
	int_t xCoord;
	int_t yCoord;
	int_t zCoord;
	int_t blockID;
	long_t scheduledTime = 0;

private:
	long_t tickEntryID;
};

// Java TreeSet ordering (compareTo) -> std::set strict-weak-ordering.
struct NextTickListEntryComparator
{
	bool operator()(const NextTickListEntry *a, const NextTickListEntry *b) const
	{
		return a->comparer(b) < 0;
	}
};

// Java HashSet (hashCode/equals) -> std::unordered_set.
struct NextTickListEntryHash
{
	std::size_t operator()(const NextTickListEntry *a) const
	{
		return (std::size_t)a->hashCode();
	}
};

struct NextTickListEntryEqual
{
	bool operator()(const NextTickListEntry *a, const NextTickListEntry *b) const
	{
		return a->equals(b);
	}
};
