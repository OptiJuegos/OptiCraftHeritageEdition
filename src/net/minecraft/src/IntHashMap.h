#pragma once

#include <unordered_set>
#include <vector>

#include "java/Type.h"

class IntHashMapEntry;

// net.minecraft.src.IntHashMap
class IntHashMap
{
public:
	IntHashMap();
	~IntHashMap();

	void *lookup(int_t key) const;
	bool containsItem(int_t key) const;
	void addKey(int_t key, void *value);
	void *removeObject(int_t key);
	void clearMap();

	const std::unordered_set<int_t> &getKeySet() const { return keySet; }

	static int_t getHash(int_t value);

private:
	static int_t computeHash(int_t value);
	static int_t getSlotIndex(int_t hash, int_t slotCount);

	IntHashMapEntry *lookupEntry(int_t key) const;
	IntHashMapEntry *removeEntry(int_t key);
	void grow(int_t newSize);
	void copyTo(std::vector<IntHashMapEntry *> &destination);
	void insert(int_t slotHash, int_t key, void *value, int_t slot);

	std::vector<IntHashMapEntry *> slots;
	int_t count;
	int_t threshold;
	const float growFactor;
	int_t versionStamp;
	std::unordered_set<int_t> keySet;
};
