#include "NBTTagList.h"

#include <istream>
#include <ostream>
#include <memory>
#include <stdexcept>

#include "java/IOUtil.h"

NBTTagList::NBTTagList()
{
}

NBTTagList::~NBTTagList()
{
	clearTags();
}

void NBTTagList::clearTags()
{
	for (NBTBase *tag : tagList)
		delete tag;
	tagList.clear();
	tagType = 0;
}

void NBTTagList::writeTagContents(std::ostream &dataoutput)
{
	if (tagList.size() > 0 && tagList[0] != nullptr)
		tagType = tagList[0]->getType();
	else
		tagType = 1;
	IOUtil::writeByte(dataoutput, tagType);
	IOUtil::writeInt(dataoutput, (int_t)tagList.size());
	for (int_t i = 0; i < (int_t)tagList.size(); i++)
	{
		if (tagList[i] != nullptr)
			tagList[i]->writeTagContents(dataoutput);
	}
}

void NBTTagList::readTagContents(std::istream &datainput)
{
	clearTags();

	tagType = IOUtil::readByte(datainput);
	int_t i = IOUtil::readInt(datainput);
	// A normal Beta world has at most a handful of entities/tile entities per
	// chunk.  Reject corrupt list counts rather than allocating until the Wii
	// heap fails; the surrounding chunk loader will skip that bad chunk.
	constexpr int_t kMaxNbtListEntries = 32768;
	if (i < 0 || i > kMaxNbtListEntries)
		throw std::runtime_error("Invalid NBT list length");
	if (i > 0 && (tagType <= 0 || tagType > 11))
		throw std::runtime_error("Invalid NBT list tag type");
	for (int_t j = 0; j < i; j++)
	{
		std::unique_ptr<NBTBase> nbtbase(NBTBase::createTagOfType(tagType));
		if (nbtbase == nullptr)
			throw std::runtime_error("Invalid NBT list tag type");
		nbtbase->readTagContents(datainput);
		tagList.push_back(nbtbase.release());
	}
}

byte_t NBTTagList::getType()
{
	return 9;
}

NBTBase *NBTTagList::copy()
{
	NBTTagList *tag = new NBTTagList();
	tag->setKey(getKey());
	tag->tagType = tagType;

	for (NBTBase *entry : tagList)
	{
		if (entry != nullptr)
			tag->setTag(entry->copy());
	}

	return tag;
}

jstring NBTTagList::toString()
{
	return String::toString((int_t)tagList.size()) + " entries of type " + NBTBase::getTagName(tagType);
}

void NBTTagList::setTag(NBTBase *nbtbase)
{
	if (nbtbase == nullptr)
		return;
	tagType = nbtbase->getType();
	tagList.push_back(nbtbase);
}

NBTBase *NBTTagList::tagAt(int_t i)
{
	return i >= 0 && (std::size_t)i < tagList.size() ? tagList[(std::size_t)i] : nullptr;
}

int_t NBTTagList::tagCount()
{
	return (int_t)tagList.size();
}

bool NBTTagList::equals(NBTBase *other)
{
	if (!NBTBase::equals(other)) return false;
	auto *tag = dynamic_cast<NBTTagList *>(other);
	if (tag == nullptr || tagType != tag->tagType || tagList.size() != tag->tagList.size()) return false;
	for (std::size_t i = 0; i < tagList.size(); ++i)
	{
		NBTBase *left = tagList[i];
		NBTBase *right = tag->tagList[i];
		if (left == nullptr ? right != nullptr : !left->equals(right)) return false;
	}
	return true;
}

int_t NBTTagList::hashCode()
{
	uint_t result = 1u;
	for (NBTBase *tag : tagList)
		result = result * 31u + static_cast<uint_t>(tag != nullptr ? tag->hashCode() : 0);
	return NBTBase::hashCode() ^ static_cast<int_t>(result);
}
