#pragma once

#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <typeinfo>

#include "java/Type.h"

class Block;
class NBTTagCompound;
class World;

// net.minecraft.src.TileEntity
class TileEntity
{
public:
	TileEntity();
	virtual ~TileEntity() = default;

	// Java keeps its addMapping private; we keep it public so the static
	// initializer at the bottom of TileEntity.cpp can register concrete types.
	// Templated so we can key classToNameMap by the concrete type's typeid,
	// which is what writeToNBT looks up at runtime.
	template <typename T>
	static void addMapping(const std::string &name)
	{
		if (nameToClassMap.find(name) != nameToClassMap.end())
			throw std::invalid_argument("Duplicate id: " + name);
		nameToClassMap[name] = []() -> TileEntity * { return new T(); };
		classToNameMap[typeid(T).name()] = name;
	}

	virtual void readFromNBT(NBTTagCompound *nbttagcompound);
	virtual void writeToNBT(NBTTagCompound *nbttagcompound);
	virtual void updateEntity();

	static TileEntity *createAndLoadEntity(NBTTagCompound *nbttagcompound);

	virtual int_t getBlockMetadata();
	virtual void onInventoryChanged();
	double getDistanceFrom(double d, double d1, double d2);
	Block *getBlockType();

	// Java: func_31006_g — has this tile entity been invalidated?
	bool isInvalid() { return invalid; }
	virtual void invalidate();
	virtual void validate();
	virtual void onTileEntityPowered(int_t eventId, int_t eventData);
	virtual void updateContainingBlockInfo();

	static std::map<std::string, std::function<TileEntity *()>> nameToClassMap;
	static std::map<std::string, std::string> classToNameMap;

	World *worldObj;
	int_t  xCoord;
	int_t  yCoord;
	int_t  zCoord;
	int_t blockMetadata;
	Block *blockType;

protected:
	bool invalid;
};
