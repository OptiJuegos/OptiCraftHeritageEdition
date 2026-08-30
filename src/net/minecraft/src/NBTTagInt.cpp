#include "NBTTagInt.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagInt::NBTTagInt()
{
}

NBTTagInt::NBTTagInt(int_t i)
{
	intValue = i;
}

void NBTTagInt::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeInt(dataoutput, intValue);
}

void NBTTagInt::readTagContents(std::istream &datainput)
{
	intValue = IOUtil::readInt(datainput);
}

byte_t NBTTagInt::getType()
{
	return 3;
}

NBTBase *NBTTagInt::copy()
{
	NBTTagInt *tag = new NBTTagInt(intValue);
	tag->setKey(getKey());
	return tag;
}

jstring NBTTagInt::toString()
{
	return String::toString(intValue);
}

bool NBTTagInt::equals(NBTBase *other)
{
	if (!NBTBase::equals(other)) return false;
	auto *tag = dynamic_cast<NBTTagInt *>(other);
	return tag != nullptr && intValue == tag->intValue;
}

int_t NBTTagInt::hashCode()
{
	return NBTBase::hashCode() ^ intValue;
}
