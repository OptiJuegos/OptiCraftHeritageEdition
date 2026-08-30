#include "NBTBase.h"

#include <istream>
#include <ostream>
#include <stdexcept>

#include "java/IOUtil.h"

#include "NBTTagEnd.h"
#include "NBTTagByte.h"
#include "NBTTagShort.h"
#include "NBTTagInt.h"
#include "NBTTagLong.h"
#include "NBTTagFloat.h"
#include "NBTTagDouble.h"
#include "NBTTagByteArray.h"
#include "NBTTagString.h"
#include "NBTTagList.h"
#include "NBTTagCompound.h"
#include "NBTTagIntArray.h"

NBTBase::NBTBase()
{
}

NBTBase::NBTBase(const jstring &name) : key(name)
{
}

jstring NBTBase::toString()
{
	return "";
}

bool NBTBase::equals(NBTBase *other)
{
	return other != nullptr && getId() == other->getId() && getName() == other->getName();
}

int_t NBTBase::hashCode()
{
	return String::hashCode(getName()) ^ static_cast<int_t>(getId());
}

jstring NBTBase::getKey()
{
	if (key.empty())
		return "";
	else
		return key;
}

NBTBase *NBTBase::setKey(const jstring &s)
{
	key = s;
	return this;
}

NBTBase *NBTBase::readTag(std::istream &datainput)
{
	byte_t byte0 = IOUtil::readByte(datainput);
	if (byte0 == 0)
	{
		return new NBTTagEnd();
	}
	else
	{
		NBTBase *nbtbase = createTagOfType(byte0);
		if (nbtbase == nullptr)
			throw std::runtime_error("Invalid NBT tag type: " + std::to_string((int)(ubyte_t)byte0));
		nbtbase->key = IOUtil::readUTF(datainput);
		nbtbase->readTagContents(datainput);
		return nbtbase;
	}
}

void NBTBase::writeTag(NBTBase *nbtbase, std::ostream &dataoutput)
{
	IOUtil::writeByte(dataoutput, nbtbase->getType());
	if (nbtbase->getType() == 0)
	{
		return;
	}
	else
	{
		IOUtil::writeUTF(dataoutput, nbtbase->getKey());
		nbtbase->writeTagContents(dataoutput);
		return;
	}
}

NBTBase *NBTBase::createTagOfType(byte_t byte0)
{
	switch (byte0)
	{
	case 0:
		return new NBTTagEnd();
	case 1:
		return new NBTTagByte();
	case 2:
		return new NBTTagShort();
	case 3:
		return new NBTTagInt();
	case 4:
		return new NBTTagLong();
	case 5:
		return new NBTTagFloat();
	case 6:
		return new NBTTagDouble();
	case 7:
		return new NBTTagByteArray();
	case 8:
		return new NBTTagString();
	case 9:
		return new NBTTagList();
	case 10:
		return new NBTTagCompound();
	case 11:
		return new NBTTagIntArray();
	}
	return nullptr;
}

NBTBase *NBTBase::newTag(byte_t type, const jstring &name)
{
	NBTBase *tag = createTagOfType(type);
	if (tag != nullptr)
		tag->setName(name);
	return tag;
}

jstring NBTBase::getTagName(byte_t byte0)
{
	switch (byte0)
	{
	case 0:
		return "TAG_End";
	case 1:
		return "TAG_Byte";
	case 2:
		return "TAG_Short";
	case 3:
		return "TAG_Int";
	case 4:
		return "TAG_Long";
	case 5:
		return "TAG_Float";
	case 6:
		return "TAG_Double";
	case 7:
		return "TAG_Byte_Array";
	case 8:
		return "TAG_String";
	case 9:
		return "TAG_List";
	case 10:
		return "TAG_Compound";
	case 11:
		return "TAG_Int_Array";
	}
	return "UNKNOWN";
}
