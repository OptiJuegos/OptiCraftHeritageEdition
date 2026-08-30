#pragma once

#include <vector>

#include "java/Type.h"

class LongHashMapEntry;

// net.minecraft.src.LongHashMap
class LongHashMap
{
public:
	LongHashMap();
	~LongHashMap();

	int_t getNumHashElements() const { return numHashElements; }
	void *getValueByKey(long_t key) const;
	bool containsItem(long_t key) const;
	void add(long_t key, void *value);
	void *remove(long_t key);
	void clearMap();

	static int_t getHashCode(long_t key);

private:
	static int_t getHashedKey(long_t key);
	static int_t hash(int_t value);
	static int_t getHashIndex(int_t hashValue, int_t length);

	LongHashMapEntry *getEntry(long_t key) const;
	LongHashMapEntry *removeKey(long_t key);
	void resizeTable(int_t newSize);
	void copyHashTableTo(std::vector<LongHashMapEntry *> &destination);
	void createKey(int_t hashValue, long_t key, void *value, int_t slot);

	std::vector<LongHashMapEntry *> hashArray;
	int_t numHashElements;
	int_t capacity;
	const float percentUseable;
	int_t modCount;
};
