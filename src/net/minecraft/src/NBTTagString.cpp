#include "NBTTagString.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagString::NBTTagString()
{
}

NBTTagString::NBTTagString(const jstring &s)
{
	stringValue = s;
}

void NBTTagString::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeUTF(dataoutput, stringValue);
}

void NBTTagString::readTagContents(std::istream &datainput)
{
	stringValue = IOUtil::readUTF(datainput);
}

byte_t NBTTagString::getType()
{
	return 8;
}

NBTBase *NBTTagString::copy()
{
	NBTTagString *tag = new NBTTagString(stringValue);
	tag->setKey(getKey());
	return tag;
}

jstring NBTTagString::toString()
{
	return stringValue;
}

bool NBTTagString::equals(NBTBase *other)
{
	if (!NBTBase::equals(other)) return false;
	auto *tag = dynamic_cast<NBTTagString *>(other);
	return tag != nullptr && stringValue == tag->stringValue;
}

int_t NBTTagString::hashCode()
{
	return NBTBase::hashCode() ^ String::hashCode(stringValue);
}
