#include "LongHashMap.h"

#include "LongHashMapEntry.h"
#include "java/Arithmetic.h"

LongHashMap::LongHashMap()
	: hashArray(16, nullptr), numHashElements(0), capacity(12), percentUseable(12.0f / 16.0f), modCount(0)
{
}

LongHashMap::~LongHashMap()
{
	clearMap();
}

int_t LongHashMap::hash(int_t value)
{
	value ^= static_cast<int_t>(static_cast<uint_t>(value) >> 20) ^
	         static_cast<int_t>(static_cast<uint_t>(value) >> 12);
	return value ^ static_cast<int_t>(static_cast<uint_t>(value) >> 7) ^
	       static_cast<int_t>(static_cast<uint_t>(value) >> 4);
}

int_t LongHashMap::getHashedKey(long_t key)
{
	const ulong_t bits = static_cast<ulong_t>(key);
	const int_t folded = static_cast<int_t>(static_cast<uint_t>(bits ^ (bits >> 32)));
	return hash(folded);
}

int_t LongHashMap::getHashCode(long_t key)
{
	return getHashedKey(key);
}

int_t LongHashMap::getHashIndex(int_t hashValue, int_t length)
{
	return hashValue & (length - 1);
}

void *LongHashMap::getValueByKey(long_t key) const
{
	LongHashMapEntry *entry = getEntry(key);
	return entry != nullptr ? entry->value : nullptr;
}

bool LongHashMap::containsItem(long_t key) const
{
	return getEntry(key) != nullptr;
}

LongHashMapEntry *LongHashMap::getEntry(long_t key) const
{
	const int_t hashValue = getHashedKey(key);
	for (LongHashMapEntry *entry = hashArray[getHashIndex(hashValue, static_cast<int_t>(hashArray.size()))];
	     entry != nullptr; entry = entry->nextEntry)
	{
		if (entry->key == key)
			return entry;
	}
	return nullptr;
}

void LongHashMap::add(long_t key, void *value)
{
	const int_t hashValue = getHashedKey(key);
	const int_t slot = getHashIndex(hashValue, static_cast<int_t>(hashArray.size()));
	for (LongHashMapEntry *entry = hashArray[slot]; entry != nullptr; entry = entry->nextEntry)
	{
		if (entry->key == key)
		{
			entry->value = value;
			return;
		}
	}

	++modCount;
	createKey(hashValue, key, value, slot);
}

void *LongHashMap::remove(long_t key)
{
	LongHashMapEntry *entry = removeKey(key);
	if (entry == nullptr)
		return nullptr;
	void *value = entry->value;
	delete entry;
	return value;
}

LongHashMapEntry *LongHashMap::removeKey(long_t key)
{
	const int_t hashValue = getHashedKey(key);
	const int_t slot = getHashIndex(hashValue, static_cast<int_t>(hashArray.size()));
	LongHashMapEntry *previous = hashArray[slot];
	for (LongHashMapEntry *entry = previous; entry != nullptr;)
	{
		LongHashMapEntry *next = entry->nextEntry;
		if (entry->key == key)
		{
			++modCount;
			--numHashElements;
			if (previous == entry)
				hashArray[slot] = next;
			else
				previous->nextEntry = next;
			return entry;
		}
		previous = entry;
		entry = next;
	}
	return nullptr;
}

void LongHashMap::clearMap()
{
	++modCount;
	for (LongHashMapEntry *&slot : hashArray)
	{
		LongHashMapEntry *entry = slot;
		while (entry != nullptr)
		{
			LongHashMapEntry *next = entry->nextEntry;
			delete entry;
			entry = next;
		}
		slot = nullptr;
	}
	numHashElements = 0;
}

void LongHashMap::resizeTable(int_t newSize)
{
	if (static_cast<int_t>(hashArray.size()) == 0x40000000)
	{
		capacity = 0x7fffffff;
		return;
	}

	std::vector<LongHashMapEntry *> newArray(static_cast<size_t>(newSize), nullptr);
	copyHashTableTo(newArray);
	hashArray = std::move(newArray);
	capacity = JavaArithmetic::floatToInt(static_cast<float>(newSize) * percentUseable);
}

void LongHashMap::copyHashTableTo(std::vector<LongHashMapEntry *> &destination)
{
	const int_t destinationSize = static_cast<int_t>(destination.size());
	for (LongHashMapEntry *&slot : hashArray)
	{
		LongHashMapEntry *entry = slot;
		slot = nullptr;
		while (entry != nullptr)
		{
			LongHashMapEntry *next = entry->nextEntry;
			const int_t destinationSlot = getHashIndex(entry->hash, destinationSize);
			entry->nextEntry = destination[static_cast<size_t>(destinationSlot)];
			destination[static_cast<size_t>(destinationSlot)] = entry;
			entry = next;
		}
	}
}

void LongHashMap::createKey(int_t hashValue, long_t key, void *value, int_t slot)
{
	hashArray[static_cast<size_t>(slot)] =
		new LongHashMapEntry(hashValue, key, value, hashArray[static_cast<size_t>(slot)]);
	if (numHashElements++ >= capacity)
		resizeTable(JavaArithmetic::intMul(static_cast<int_t>(hashArray.size()), 2));
}
