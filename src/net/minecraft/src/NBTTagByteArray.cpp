#include "NBTTagByteArray.h"

#include <istream>
#include <ostream>
#include <stdexcept>

#include "java/IOUtil.h"

NBTTagByteArray::NBTTagByteArray()
{
}

NBTTagByteArray::NBTTagByteArray(const std::vector<byte_t> &abyte0)
{
	byteArray = abyte0;
}

void NBTTagByteArray::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeInt(dataoutput, (int_t)byteArray.size());
	if (!byteArray.empty())
		dataoutput.write(reinterpret_cast<const char *>(byteArray.data()), (std::streamsize)byteArray.size());
}

void NBTTagByteArray::readTagContents(std::istream &datainput)
{
	int_t i = IOUtil::readInt(datainput);
	// Vanilla chunk arrays are at most 32 KB and map data is 16 KB.  A megabyte
	// still leaves room for legitimate level/mod data, while rejecting a broken
	// signed length before it converts to size_t and throws std::bad_alloc.
	constexpr int_t kMaxNbtByteArrayBytes = 1024 * 1024;
	if (i < 0 || i > kMaxNbtByteArrayBytes)
		throw std::runtime_error("Invalid NBT byte-array length");
	byteArray.assign((size_t)i, 0);
	if (i > 0)
	{
		datainput.read(reinterpret_cast<char *>(byteArray.data()), (std::streamsize)i);
		if (!datainput)
			throw std::runtime_error("Truncated NBT byte array");
	}
}

byte_t NBTTagByteArray::getType()
{
	return 7;
}

NBTBase *NBTTagByteArray::copy()
{
	NBTTagByteArray *tag = new NBTTagByteArray(byteArray);
	tag->setKey(getKey());
	return tag;
}

jstring NBTTagByteArray::toString()
{
	return "[" + String::toString((int_t)byteArray.size()) + " bytes]";
}

bool NBTTagByteArray::equals(NBTBase *other)
{
	if (!NBTBase::equals(other)) return false;
	auto *tag = dynamic_cast<NBTTagByteArray *>(other);
	return tag != nullptr && byteArray == tag->byteArray;
}

int_t NBTTagByteArray::hashCode()
{
	uint_t result = 1u;
	for (byte_t value : byteArray)
		result = result * 31u + static_cast<uint_t>(static_cast<int_t>(value));
	return NBTBase::hashCode() ^ static_cast<int_t>(result);
}
