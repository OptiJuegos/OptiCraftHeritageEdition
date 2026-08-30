#include "NBTTagEnd.h"

NBTTagEnd::NBTTagEnd()
{
}

void NBTTagEnd::readTagContents(std::istream &datainput)
{
}

void NBTTagEnd::writeTagContents(std::ostream &dataoutput)
{
}

byte_t NBTTagEnd::getType()
{
	return 0;
}

NBTBase *NBTTagEnd::copy()
{
	NBTTagEnd *tag = new NBTTagEnd();
	tag->setKey(getKey());
	return tag;
}

jstring NBTTagEnd::toString()
{
	return "END";
}

bool NBTTagEnd::equals(NBTBase *other)
{
	return NBTBase::equals(other);
}

int_t NBTTagEnd::hashCode()
{
	return NBTBase::hashCode();
}
