#include "NBTTagIntArray.h"

#include <istream>
#include <ostream>
#include <stdexcept>

#include "java/IOUtil.h"

NBTTagIntArray::NBTTagIntArray()
{
}

NBTTagIntArray::NBTTagIntArray(const std::vector<int_t> &values) : intArray(values)
{
}

void NBTTagIntArray::writeTagContents(std::ostream &dataoutput)
{
    IOUtil::writeInt(dataoutput, (int_t)intArray.size());
    for (int_t value : intArray)
        IOUtil::writeInt(dataoutput, value);
}

void NBTTagIntArray::readTagContents(std::istream &datainput)
{
    int_t length = IOUtil::readInt(datainput);
    constexpr int_t kMaxNbtIntArrayEntries = 1024 * 1024;
    if (length < 0 || length > kMaxNbtIntArrayEntries)
        throw std::runtime_error("Invalid NBT int-array length");

    intArray.resize((size_t)length);
    for (int_t &value : intArray)
        value = IOUtil::readInt(datainput);
}

byte_t NBTTagIntArray::getType()
{
    return 11;
}

NBTBase *NBTTagIntArray::copy()
{
    NBTTagIntArray *tag = new NBTTagIntArray(intArray);
    tag->setKey(getKey());
    return tag;
}

jstring NBTTagIntArray::toString()
{
    return "[" + String::toString((int_t)intArray.size()) + " ints]";
}

bool NBTTagIntArray::equals(NBTBase *other)
{
	if (!NBTBase::equals(other)) return false;
	auto *tag = dynamic_cast<NBTTagIntArray *>(other);
	return tag != nullptr && intArray == tag->intArray;
}

int_t NBTTagIntArray::hashCode()
{
	uint_t result = 1u;
	for (int_t value : intArray)
		result = result * 31u + static_cast<uint_t>(value);
	return NBTBase::hashCode() ^ static_cast<int_t>(result);
}
