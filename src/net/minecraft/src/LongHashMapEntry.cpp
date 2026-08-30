#include "LongHashMapEntry.h"

#include "LongHashMap.h"

bool LongHashMapEntry::equals(const LongHashMapEntry *other) const
{
	return other != nullptr && key == other->key && value == other->value;
}

int_t LongHashMapEntry::hashCode() const
{
	return LongHashMap::getHashCode(key);
}
