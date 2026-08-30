#pragma once

#include "java/Type.h"

class LongHashMap;

// net.minecraft.src.LongHashMapEntry
class LongHashMapEntry
{
public:
	LongHashMapEntry(int_t hash_, long_t key_, void *value_, LongHashMapEntry *nextEntry_)
		: key(key_), value(value_), nextEntry(nextEntry_), hash(hash_)
	{
	}

	long_t getKey() const { return key; }
	void *getValue() const { return value; }
	bool equals(const LongHashMapEntry *other) const;
	int_t hashCode() const;

	const long_t key;
	void *value;
	LongHashMapEntry *nextEntry;
	const int_t hash;
};
