#pragma once

#include <vector>

#include "NBTBase.h"

// net.minecraft.src.NBTTagList
class NBTTagList : public NBTBase
{
public:
	NBTTagList();
	~NBTTagList() override;

	void writeTagContents(std::ostream &dataoutput) override;
	void readTagContents(std::istream &datainput) override;
	byte_t getType() override;
	NBTBase *copy() override;
	bool equals(NBTBase *other) override;
	int_t hashCode() override;
	jstring toString() override;

	void setTag(NBTBase *nbtbase);
	void appendTag(NBTBase *nbtbase) { setTag(nbtbase); }
	NBTBase *tagAt(int_t i);
	int_t tagCount();

private:
	void clearTags();

	std::vector<NBTBase *> tagList;
	byte_t tagType = 0;
};
