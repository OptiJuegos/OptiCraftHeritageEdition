#include "NBTTagShort.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagShort::NBTTagShort()
{
}

NBTTagShort::NBTTagShort(short_t word0)
{
	shortValue = word0;
}

void NBTTagShort::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeShort(dataoutput, shortValue);
}

void NBTTagShort::readTagContents(std::istream &datainput)
{
	shortValue = IOUtil::readShort(datainput);
}

byte_t NBTTagShort::getType()
{
	return 2;
}

NBTBase *NBTTagShort::copy()
{
	NBTTagShort *tag = new NBTTagShort(shortValue);
	tag->setKey(getKey());
	return tag;
}

jstring NBTTagShort::toString()
{
	return String::toString((int_t)shortValue);
}

bool NBTTagShort::equals(NBTBase *other)
{
	if (!NBTBase::equals(other)) return false;
	auto *tag = dynamic_cast<NBTTagShort *>(other);
	return tag != nullptr && shortValue == tag->shortValue;
}

int_t NBTTagShort::hashCode()
{
	return NBTBase::hashCode() ^ static_cast<int_t>(shortValue);
}
