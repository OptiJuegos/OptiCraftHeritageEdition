#pragma once

#include <vector>
#include <unordered_map>

#include "NBTBase.h"
#include "java/HashSet.h"

class NBTTagList;

// net.minecraft.src.NBTTagCompound
class NBTTagCompound : public NBTBase
{
public:
	NBTTagCompound();
	~NBTTagCompound() override;

	void writeTagContents(std::ostream &dataoutput) override;
	void readTagContents(std::istream &datainput) override;
	std::vector<NBTBase *> getTags();
	byte_t getType() override;
	NBTBase *copy() override;
	bool equals(NBTBase *other) override;
	int_t hashCode() override;

	void setTag(const jstring &s, NBTBase *nbtbase);
	void setByte(const jstring &s, byte_t byte0);
	void setShort(const jstring &s, short_t word0);
	void setInteger(const jstring &s, int_t i);
	void setLong(const jstring &s, long_t l);
	void setFloat(const jstring &s, float f);
	void setDouble(const jstring &s, double d);
	void setString(const jstring &s, const jstring &s1);
	void setByteArray(const jstring &s, const std::vector<byte_t> &abyte0);
	void setIntArray(const jstring &s, const std::vector<int_t> &values);
	void func_48183_a(const jstring &s, const std::vector<int_t> &values);
	void setCompoundTag(const jstring &s, NBTTagCompound *nbttagcompound);
	void setBoolean(const jstring &s, bool flag);

	bool hasKey(const jstring &s);
	byte_t getByte(const jstring &s);
	short_t getShort(const jstring &s);
	int_t getInteger(const jstring &s);
	long_t getLong(const jstring &s);
	float getFloat(const jstring &s);
	double getDouble(const jstring &s);
	jstring getString(const jstring &s);
	std::vector<byte_t> getByteArray(const jstring &s);
	std::vector<int_t> getIntArray(const jstring &s);
	std::vector<int_t> func_48182_l(const jstring &s);
	NBTBase *getTag(const jstring &s);
	NBTTagCompound *getCompoundTag(const jstring &s);
	NBTTagList *getTagList(const jstring &s);
	bool getBoolean(const jstring &s);

	jstring toString() override;

private:
	struct JavaStringHash
	{
		uint_t operator()(const jstring &value) const
		{
			return static_cast<uint_t>(String::hashCode(value));
		}
	};

	struct JavaStringEqual
	{
		bool operator()(const jstring &left, const jstring &right) const
		{
			return left == right;
		}
	};

	void clearTags();

	std::unordered_map<jstring, NBTBase *> tagMap;
	JavaHashSet<jstring, JavaStringHash, JavaStringEqual> tagOrder;
};
