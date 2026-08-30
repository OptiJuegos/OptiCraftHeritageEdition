#include "NBTTagCompound.h"
#include <typeinfo>

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

#include "NBTTagByte.h"
#include "NBTTagShort.h"
#include "NBTTagInt.h"
#include "NBTTagLong.h"
#include "NBTTagFloat.h"
#include "NBTTagDouble.h"
#include "NBTTagString.h"
#include "NBTTagByteArray.h"
#include "NBTTagIntArray.h"
#include "NBTTagList.h"

NBTTagCompound::NBTTagCompound()
{
}

NBTTagCompound::~NBTTagCompound()
{
	clearTags();
}

void NBTTagCompound::clearTags()
{
	for (auto &entry : tagMap)
	{
		delete entry.second;
		entry.second = nullptr;
	}
	tagMap.clear();
	tagOrder.clear();
}

void NBTTagCompound::writeTagContents(std::ostream &dataoutput)
{
	for (const jstring &key : tagOrder.valuesInIterationOrder())
	{
		auto it = tagMap.find(key);
		if (it != tagMap.end() && it->second != nullptr)
			NBTBase::writeTag(it->second, dataoutput);
	}
	IOUtil::writeByte(dataoutput, 0);
}

void NBTTagCompound::readTagContents(std::istream &datainput)
{
	clearTags();

	while (true)
	{
		NBTBase *nbtbase = NBTBase::readTag(datainput);
		if (nbtbase == nullptr)
			break;

		if (nbtbase->getType() == 0)
		{
			delete nbtbase;
			break;
		}

		setTag(nbtbase->getKey(), nbtbase);
	}
}

std::vector<NBTBase *> NBTTagCompound::getTags()
{
	std::vector<NBTBase *> values;
	values.reserve(tagMap.size());
	for (const jstring &key : tagOrder.valuesInIterationOrder())
	{
		auto it = tagMap.find(key);
		if (it != tagMap.end() && it->second != nullptr)
			values.push_back(it->second);
	}
	return values;
}

byte_t NBTTagCompound::getType()
{
	return 10;
}

NBTBase *NBTTagCompound::copy()
{
	NBTTagCompound *tag = new NBTTagCompound();
	tag->setKey(getKey());

	for (const jstring &key : tagOrder.valuesInIterationOrder())
	{
		auto it = tagMap.find(key);
		if (it != tagMap.end() && it->second != nullptr)
			tag->setTag(key, it->second->copy());
	}

	return tag;
}

void NBTTagCompound::setTag(const jstring &s, NBTBase *nbtbase)
{
	auto it = tagMap.find(s);
	if (it != tagMap.end())
	{
		if (it->second == nbtbase)
		{
			if (nbtbase != nullptr)
				nbtbase->setKey(s);
			return;
		}

		delete it->second;
		if (nbtbase != nullptr)
			it->second = nbtbase->setKey(s);
		else
		{
			tagMap.erase(it);
			tagOrder.remove(s);
		}
		return;
	}

	if (nbtbase != nullptr)
	{
		tagOrder.add(s);
		tagMap[s] = nbtbase->setKey(s);
	}
}

void NBTTagCompound::setByte(const jstring &s, byte_t byte0)
{
	setTag(s, new NBTTagByte(byte0));
}

void NBTTagCompound::setShort(const jstring &s, short_t word0)
{
	setTag(s, new NBTTagShort(word0));
}

void NBTTagCompound::setInteger(const jstring &s, int_t i)
{
	setTag(s, new NBTTagInt(i));
}

void NBTTagCompound::setLong(const jstring &s, long_t l)
{
	setTag(s, new NBTTagLong(l));
}

void NBTTagCompound::setFloat(const jstring &s, float f)
{
	setTag(s, new NBTTagFloat(f));
}

void NBTTagCompound::setDouble(const jstring &s, double d)
{
	setTag(s, new NBTTagDouble(d));
}

void NBTTagCompound::setString(const jstring &s, const jstring &s1)
{
	setTag(s, new NBTTagString(s1));
}

void NBTTagCompound::setByteArray(const jstring &s, const std::vector<byte_t> &abyte0)
{
	setTag(s, new NBTTagByteArray(abyte0));
}

void NBTTagCompound::setIntArray(const jstring &s, const std::vector<int_t> &values)
{
	setTag(s, new NBTTagIntArray(values));
}

void NBTTagCompound::func_48183_a(const jstring &s, const std::vector<int_t> &values)
{
	setIntArray(s, values);
}

void NBTTagCompound::setCompoundTag(const jstring &s, NBTTagCompound *nbttagcompound)
{
	setTag(s, nbttagcompound);
}

void NBTTagCompound::setBoolean(const jstring &s, bool flag)
{
	setByte(s, (byte_t)(flag ? 1 : 0));
}

bool NBTTagCompound::hasKey(const jstring &s)
{
	return tagMap.count(s) != 0;
}

byte_t NBTTagCompound::getByte(const jstring &s)
{
	auto it = tagMap.find(s);
	if (it == tagMap.end()) return 0;
	NBTTagByte *tag = dynamic_cast<NBTTagByte *>(it->second);
	if (tag == nullptr) throw std::bad_cast();
	return tag->byteValue;
}

short_t NBTTagCompound::getShort(const jstring &s)
{
	auto it = tagMap.find(s);
	if (it == tagMap.end()) return 0;
	NBTTagShort *tag = dynamic_cast<NBTTagShort *>(it->second);
	if (tag == nullptr) throw std::bad_cast();
	return tag->shortValue;
}

int_t NBTTagCompound::getInteger(const jstring &s)
{
	auto it = tagMap.find(s);
	if (it == tagMap.end()) return 0;
	NBTTagInt *tag = dynamic_cast<NBTTagInt *>(it->second);
	if (tag == nullptr) throw std::bad_cast();
	return tag->intValue;
}

long_t NBTTagCompound::getLong(const jstring &s)
{
	auto it = tagMap.find(s);
	if (it == tagMap.end()) return 0LL;
	NBTTagLong *tag = dynamic_cast<NBTTagLong *>(it->second);
	if (tag == nullptr) throw std::bad_cast();
	return tag->longValue;
}

float NBTTagCompound::getFloat(const jstring &s)
{
	auto it = tagMap.find(s);
	if (it == tagMap.end()) return 0.0f;
	NBTTagFloat *tag = dynamic_cast<NBTTagFloat *>(it->second);
	if (tag == nullptr) throw std::bad_cast();
	return tag->floatValue;
}

double NBTTagCompound::getDouble(const jstring &s)
{
	auto it = tagMap.find(s);
	if (it == tagMap.end()) return 0.0;
	NBTTagDouble *tag = dynamic_cast<NBTTagDouble *>(it->second);
	if (tag == nullptr) throw std::bad_cast();
	return tag->doubleValue;
}

jstring NBTTagCompound::getString(const jstring &s)
{
	auto it = tagMap.find(s);
	if (it == tagMap.end()) return "";
	NBTTagString *tag = dynamic_cast<NBTTagString *>(it->second);
	if (tag == nullptr) throw std::bad_cast();
	return tag->stringValue;
}

std::vector<byte_t> NBTTagCompound::getByteArray(const jstring &s)
{
	auto it = tagMap.find(s);
	if (it == tagMap.end()) return std::vector<byte_t>();
	NBTTagByteArray *tag = dynamic_cast<NBTTagByteArray *>(it->second);
	if (tag == nullptr) throw std::bad_cast();
	return tag->byteArray;
}

std::vector<int_t> NBTTagCompound::getIntArray(const jstring &s)
{
	auto it = tagMap.find(s);
	if (it == tagMap.end()) return std::vector<int_t>();
	NBTTagIntArray *tag = dynamic_cast<NBTTagIntArray *>(it->second);
	if (tag == nullptr) throw std::bad_cast();
	return tag->intArray;
}

std::vector<int_t> NBTTagCompound::func_48182_l(const jstring &s)
{
	return getIntArray(s);
}

NBTBase *NBTTagCompound::getTag(const jstring &s)
{
	auto it = tagMap.find(s);
	return it == tagMap.end() ? nullptr : it->second;
}

NBTTagCompound *NBTTagCompound::getCompoundTag(const jstring &s)
{
	auto it = tagMap.find(s);
	if (it == tagMap.end()) return new NBTTagCompound();
	NBTTagCompound *tag = dynamic_cast<NBTTagCompound *>(it->second);
	if (tag == nullptr) throw std::bad_cast();
	return tag;
}

NBTTagList *NBTTagCompound::getTagList(const jstring &s)
{
	auto it = tagMap.find(s);
	if (it == tagMap.end()) return new NBTTagList();
	NBTTagList *tag = dynamic_cast<NBTTagList *>(it->second);
	if (tag == nullptr) throw std::bad_cast();
	return tag;
}

bool NBTTagCompound::getBoolean(const jstring &s)
{
	return getByte(s) != 0;
}

jstring NBTTagCompound::toString()
{
	return String::toString((int_t)tagMap.size()) + " entries";
}

bool NBTTagCompound::equals(NBTBase *other)
{
	if (!NBTBase::equals(other)) return false;
	auto *tag = dynamic_cast<NBTTagCompound *>(other);
	if (tag == nullptr || tagMap.size() != tag->tagMap.size()) return false;
	for (const auto &entry : tagMap)
	{
		auto it = tag->tagMap.find(entry.first);
		if (it == tag->tagMap.end()) return false;
		if (entry.second == nullptr ? it->second != nullptr : !entry.second->equals(it->second)) return false;
	}
	return true;
}

int_t NBTTagCompound::hashCode()
{
	uint_t mapHash = 0u;
	for (const auto &entry : tagMap)
	{
		const uint_t keyHash = static_cast<uint_t>(String::hashCode(entry.first));
		const uint_t valueHash = static_cast<uint_t>(entry.second != nullptr ? entry.second->hashCode() : 0);
		mapHash += keyHash ^ valueHash;
	}
	return NBTBase::hashCode() ^ static_cast<int_t>(mapHash);
}
