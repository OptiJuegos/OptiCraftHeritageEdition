#include "NextTickListEntry.h"
#include "java/Arithmetic.h"
#include <cstdint>

long_t NextTickListEntry::nextTickEntryID = 0LL;

NextTickListEntry::NextTickListEntry(int_t i, int_t j, int_t k, int_t l)
{
	tickEntryID = nextTickEntryID;
	nextTickEntryID = JavaArithmetic::longAdd(nextTickEntryID, 1LL);
	xCoord = i;
	yCoord = j;
	zCoord = k;
	blockID = l;
}

bool NextTickListEntry::equals(const NextTickListEntry *nextticklistentry) const
{
	if (nextticklistentry != nullptr)
		return xCoord == nextticklistentry->xCoord && yCoord == nextticklistentry->yCoord && zCoord == nextticklistentry->zCoord && blockID == nextticklistentry->blockID;
	else
		return false;
}

int_t NextTickListEntry::hashCode() const
{
	std::uint32_t value = static_cast<std::uint32_t>(xCoord) * 1024u * 1024u;
	value += static_cast<std::uint32_t>(zCoord) * 1024u;
	value += static_cast<std::uint32_t>(yCoord);
	value = value * 256u + static_cast<std::uint32_t>(blockID);
	return JavaArithmetic::intFromBits(value);
}

NextTickListEntry *NextTickListEntry::setScheduledTime(long_t l)
{
	scheduledTime = l;
	return this;
}

int_t NextTickListEntry::comparer(const NextTickListEntry *nextticklistentry) const
{
	if (scheduledTime < nextticklistentry->scheduledTime)
		return -1;
	if (scheduledTime > nextticklistentry->scheduledTime)
		return 1;
	if (tickEntryID < nextticklistentry->tickEntryID)
		return -1;
	return tickEntryID <= nextticklistentry->tickEntryID ? 0 : 1;
}

int_t NextTickListEntry::compareTo(const NextTickListEntry *obj) const
{
	return comparer(obj);
}
