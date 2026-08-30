#include "NBTTagDouble.h"
#include <cstring>

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagDouble::NBTTagDouble()
{
}

NBTTagDouble::NBTTagDouble(double d)
{
	doubleValue = d;
}

void NBTTagDouble::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeDouble(dataoutput, doubleValue);
}

void NBTTagDouble::readTagContents(std::istream &datainput)
{
	doubleValue = IOUtil::readDouble(datainput);
}

byte_t NBTTagDouble::getType()
{
	return 6;
}

NBTBase *NBTTagDouble::copy()
{
	NBTTagDouble *tag = new NBTTagDouble(doubleValue);
	tag->setKey(getKey());
	return tag;
}

jstring NBTTagDouble::toString()
{
	return String::toString(doubleValue);
}

bool NBTTagDouble::equals(NBTBase *other)
{
	if (!NBTBase::equals(other)) return false;
	auto *tag = dynamic_cast<NBTTagDouble *>(other);
	return tag != nullptr && doubleValue == tag->doubleValue;
}

int_t NBTTagDouble::hashCode()
{
	ulong_t bits = 0;
	static_assert(sizeof(bits) == sizeof(doubleValue), "double bit width");
	std::memcpy(&bits, &doubleValue, sizeof(bits));
	return NBTBase::hashCode() ^ static_cast<int_t>(static_cast<uint_t>(bits ^ (bits >> 32)));
}
