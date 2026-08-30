#pragma once

#include <vector>
#include "java/Type.h"
#include "MCHashEntry.h"

// net.minecraft.src.MCHash
class MCHash
{
public:
	explicit MCHash(bool reuseEntries = false);
	~MCHash();

	void *lookup(int_t i);
	void addKey(int_t i, void *obj);
	void *removeObject(int_t i);
	void clearMap();
	void reserveEntries(int_t expectedEntries);

	static int_t getHash(int_t i);

private:
	static int_t computeHash(int_t i);
	static int_t getSlotIndex(int_t i, int_t j);

	void grow(int_t i);
	void copyTo(std::vector<MCHashEntry *> &dest);
	void insert(int_t slotHash, int_t key, void *obj, int_t slot);
	MCHashEntry *removeEntry(int_t i);

	std::vector<MCHashEntry *> slots;
	std::vector<MCHashEntry *> freeEntries;
	int_t count;
	int_t threshold;
	const float growFactor;
	int_t versionStamp;
	bool reuseEntries;
};
