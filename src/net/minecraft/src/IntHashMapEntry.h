#pragma once

#include "java/Type.h"

class IntHashMap;

// net.minecraft.src.IntHashMapEntry
class IntHashMapEntry
{
public:
	IntHashMapEntry(int_t slotHash_, int_t hashEntry_, void *valueEntry_, IntHashMapEntry *nextEntry_)
		: hashEntry(hashEntry_), valueEntry(valueEntry_), nextEntry(nextEntry_), slotHash(slotHash_)
	{
	}

	int_t getHash() const { return hashEntry; }
	void *getValue() const { return valueEntry; }
	bool equals(const IntHashMapEntry *other) const;
	int_t hashCode() const;

	const int_t hashEntry;
	void *valueEntry;
	IntHashMapEntry *nextEntry;
	const int_t slotHash;
};
