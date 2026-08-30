#include "NBTTagLong.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagLong::NBTTagLong()
{
}

NBTTagLong::NBTTagLong(long_t l)
{
	longValue = l;
}

void NBTTagLong::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeLong(dataoutput, longValue);
}

void NBTTagLong::readTagContents(std::istream &datainput)
{
	longValue = IOUtil::readLong(datainput);
}

byte_t NBTTagLong::getType()
{
	return 4;
}

NBTBase *NBTTagLong::copy()
{
	NBTTagLong *tag = new NBTTagLong(longValue);
	tag->setKey(getKey());
	return tag;
}

jstring NBTTagLong::toString()
{
	return String::toString(longValue);
}

bool NBTTagLong::equals(NBTBase *other)
{
	if (!NBTBase::equals(other)) return false;
	auto *tag = dynamic_cast<NBTTagLong *>(other);
	return tag != nullptr && longValue == tag->longValue;
}

int_t NBTTagLong::hashCode()
{
	const ulong_t bits = static_cast<ulong_t>(longValue);
	return NBTBase::hashCode() ^ static_cast<int_t>(static_cast<uint_t>(bits ^ (bits >> 32)));
}
