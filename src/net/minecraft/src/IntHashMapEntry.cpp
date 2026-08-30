#include "IntHashMapEntry.h"

#include "IntHashMap.h"

bool IntHashMapEntry::equals(const IntHashMapEntry *other) const
{
	return other != nullptr && hashEntry == other->hashEntry && valueEntry == other->valueEntry;
}

int_t IntHashMapEntry::hashCode() const
{
	return IntHashMap::getHash(hashEntry);
}
