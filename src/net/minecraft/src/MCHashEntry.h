#pragma once

#include "java/Type.h"

class MCHash;

// net.minecraft.src.MCHashEntry
class MCHashEntry
{
public:
	MCHashEntry(int_t slotHash_, int_t hashEntry_, void *valueEntry_, MCHashEntry *next)
		: slotHash(slotHash_), hashEntry(hashEntry_), valueEntry(valueEntry_), nextEntry(next) {}

	int_t getHash() const { return hashEntry; }
	void *getValue() const { return valueEntry; }

	bool equals(const MCHashEntry *other) const
	{
		if (!other) return false;
		return hashEntry == other->hashEntry && valueEntry == other->valueEntry;
	}

	int_t hashCode() const;

	const int_t hashEntry;
	void *valueEntry;
	MCHashEntry *nextEntry;
	const int_t slotHash;
};
