#include "MCHashEntry.h"
#include "MCHash.h"

int_t MCHashEntry::hashCode() const
{
    return MCHash::getHash(hashEntry);
}
