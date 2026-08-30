#include "NBTTagFloat.h"
#include <cstring>

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagFloat::NBTTagFloat()
{
}

NBTTagFloat::NBTTagFloat(float f)
{
	floatValue = f;
}

void NBTTagFloat::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeFloat(dataoutput, floatValue);
}

void NBTTagFloat::readTagContents(std::istream &datainput)
{
	floatValue = IOUtil::readFloat(datainput);
}

byte_t NBTTagFloat::getType()
{
	return 5;
}

NBTBase *NBTTagFloat::copy()
{
	NBTTagFloat *tag = new NBTTagFloat(floatValue);
	tag->setKey(getKey());
	return tag;
}

jstring NBTTagFloat::toString()
{
	return String::toString(floatValue);
}

bool NBTTagFloat::equals(NBTBase *other)
{
	if (!NBTBase::equals(other)) return false;
	auto *tag = dynamic_cast<NBTTagFloat *>(other);
	return tag != nullptr && floatValue == tag->floatValue;
}

int_t NBTTagFloat::hashCode()
{
	uint_t bits = 0;
	static_assert(sizeof(bits) == sizeof(floatValue), "float bit width");
	std::memcpy(&bits, &floatValue, sizeof(bits));
	return NBTBase::hashCode() ^ static_cast<int_t>(bits);
}
