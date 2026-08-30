#include "IntHashMap.h"

#include "IntHashMapEntry.h"

#include "java/Arithmetic.h"
IntHashMap::IntHashMap()
	: slots(16, nullptr), count(0), threshold(12), growFactor(12.0f / 16.0f), versionStamp(0)
{
}

IntHashMap::~IntHashMap()
{
	clearMap();
}

int_t IntHashMap::computeHash(int_t value)
{
	value ^= static_cast<int_t>(static_cast<uint_t>(value) >> 20) ^
	         static_cast<int_t>(static_cast<uint_t>(value) >> 12);
	return value ^ static_cast<int_t>(static_cast<uint_t>(value) >> 7) ^
	       static_cast<int_t>(static_cast<uint_t>(value) >> 4);
}

int_t IntHashMap::getSlotIndex(int_t hash, int_t slotCount)
{
	return hash & (slotCount - 1);
}

int_t IntHashMap::getHash(int_t value)
{
	return computeHash(value);
}

void *IntHashMap::lookup(int_t key) const
{
	IntHashMapEntry *entry = lookupEntry(key);
	return entry != nullptr ? entry->valueEntry : nullptr;
}

bool IntHashMap::containsItem(int_t key) const
{
	return lookupEntry(key) != nullptr;
}

IntHashMapEntry *IntHashMap::lookupEntry(int_t key) const
{
	const int_t hash = computeHash(key);
	for (IntHashMapEntry *entry = slots[getSlotIndex(hash, static_cast<int_t>(slots.size()))];
	     entry != nullptr; entry = entry->nextEntry)
	{
		if (entry->hashEntry == key)
			return entry;
	}
	return nullptr;
}

void IntHashMap::addKey(int_t key, void *value)
{
	keySet.insert(key);
	const int_t hash = computeHash(key);
	const int_t slot = getSlotIndex(hash, static_cast<int_t>(slots.size()));
	for (IntHashMapEntry *entry = slots[slot]; entry != nullptr; entry = entry->nextEntry)
	{
		if (entry->hashEntry == key)
			entry->valueEntry = value;
	}

	++versionStamp;
	insert(hash, key, value, slot);
}

void *IntHashMap::removeObject(int_t key)
{
	keySet.erase(key);
	IntHashMapEntry *entry = removeEntry(key);
	if (entry == nullptr)
		return nullptr;
	void *value = entry->valueEntry;
	delete entry;
	return value;
}

IntHashMapEntry *IntHashMap::removeEntry(int_t key)
{
	const int_t hash = computeHash(key);
	const int_t slot = getSlotIndex(hash, static_cast<int_t>(slots.size()));
	IntHashMapEntry *previous = slots[slot];
	for (IntHashMapEntry *entry = previous; entry != nullptr;)
	{
		IntHashMapEntry *next = entry->nextEntry;
		if (entry->hashEntry == key)
		{
			++versionStamp;
			--count;
			if (previous == entry)
				slots[slot] = next;
			else
				previous->nextEntry = next;
			return entry;
		}
		previous = entry;
		entry = next;
	}
	return nullptr;
}

void IntHashMap::clearMap()
{
	++versionStamp;
	for (IntHashMapEntry *&slot : slots)
	{
		IntHashMapEntry *entry = slot;
		while (entry != nullptr)
		{
			IntHashMapEntry *next = entry->nextEntry;
			delete entry;
			entry = next;
		}
		slot = nullptr;
	}
	count = 0;
	keySet.clear();
}

void IntHashMap::grow(int_t newSize)
{
	if (static_cast<int_t>(slots.size()) == 0x40000000)
	{
		threshold = 0x7fffffff;
		return;
	}

	std::vector<IntHashMapEntry *> newSlots(static_cast<size_t>(newSize), nullptr);
	copyTo(newSlots);
	slots = std::move(newSlots);
	threshold = JavaArithmetic::floatToInt(static_cast<float>(newSize) * growFactor);
}

void IntHashMap::copyTo(std::vector<IntHashMapEntry *> &destination)
{
	const int_t destinationSize = static_cast<int_t>(destination.size());
	for (IntHashMapEntry *&slot : slots)
	{
		IntHashMapEntry *entry = slot;
		slot = nullptr;
		while (entry != nullptr)
		{
			IntHashMapEntry *next = entry->nextEntry;
			const int_t destinationSlot = getSlotIndex(entry->slotHash, destinationSize);
			entry->nextEntry = destination[static_cast<size_t>(destinationSlot)];
			destination[static_cast<size_t>(destinationSlot)] = entry;
			entry = next;
		}
	}
}

void IntHashMap::insert(int_t slotHash, int_t key, void *value, int_t slot)
{
	slots[static_cast<size_t>(slot)] =
		new IntHashMapEntry(slotHash, key, value, slots[static_cast<size_t>(slot)]);
	if (count++ >= threshold)
		grow(JavaArithmetic::intMul(static_cast<int_t>(slots.size()), 2));
}
