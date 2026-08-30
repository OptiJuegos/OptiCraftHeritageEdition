#include "NBTTagByte.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagByte::NBTTagByte()
{
}

NBTTagByte::NBTTagByte(byte_t byte0)
{
	byteValue = byte0;
}

void NBTTagByte::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeByte(dataoutput, byteValue);
}

void NBTTagByte::readTagContents(std::istream &datainput)
{
	byteValue = IOUtil::readByte(datainput);
}

byte_t NBTTagByte::getType()
{
	return 1;
}

NBTBase *NBTTagByte::copy()
{
	NBTTagByte *tag = new NBTTagByte(byteValue);
	tag->setKey(getKey());
	return tag;
}

jstring NBTTagByte::toString()
{
	return String::toString((int_t)byteValue);
}

bool NBTTagByte::equals(NBTBase *other)
{
	if (!NBTBase::equals(other)) return false;
	auto *tag = dynamic_cast<NBTTagByte *>(other);
	return tag != nullptr && byteValue == tag->byteValue;
}

int_t NBTTagByte::hashCode()
{
	return NBTBase::hashCode() ^ static_cast<int_t>(byteValue);
}
