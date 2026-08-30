#pragma once

#include <iosfwd>

#include "java/Type.h"
#include "java/String.h"

// net.minecraft.src.NBTBase
class NBTBase
{
public:
	NBTBase();
	explicit NBTBase(const jstring &name);
	virtual ~NBTBase() = default;

	virtual void writeTagContents(std::ostream &dataoutput) = 0;
	virtual void readTagContents(std::istream &datainput) = 0;
	virtual byte_t getType() = 0;
	virtual NBTBase *copy() = 0;
	virtual jstring toString();

	void write(std::ostream &dataoutput) { writeTagContents(dataoutput); }
	void load(std::istream &datainput) { readTagContents(datainput); }
	byte_t getId() { return getType(); }
	virtual bool equals(NBTBase *other);
	virtual int_t hashCode();

	jstring getKey();
	NBTBase *setKey(const jstring &s);
	jstring getName() { return getKey(); }
	NBTBase *setName(const jstring &s) { return setKey(s); }

	static NBTBase *readTag(std::istream &datainput);
	static NBTBase *readNamedTag(std::istream &datainput) { return readTag(datainput); }
	static void writeTag(NBTBase *nbtbase, std::ostream &dataoutput);
	static void writeNamedTag(NBTBase *nbtbase, std::ostream &dataoutput) { writeTag(nbtbase, dataoutput); }
	static NBTBase *createTagOfType(byte_t byte0);
	static NBTBase *newTag(byte_t type, const jstring &name);
	static jstring getTagName(byte_t byte0);

private:
	jstring key;
};
