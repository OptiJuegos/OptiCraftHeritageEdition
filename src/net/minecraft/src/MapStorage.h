#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <typeinfo>
#include "java/Type.h"
#include "java/String.h"
#include "java/HashSet.h"

class ISaveHandler;
class MapDataBase;
class NBTTagCompound;
class NBTBase;

struct IdCountStringHash
{
	int_t operator()(const jstring &value) const { return String::hashCode(value); }
};

struct IdCountStringEqual
{
	bool operator()(const jstring &lhs, const jstring &rhs) const { return lhs == rhs; }
};

// net.minecraft.src.MapStorage
class MapStorage
{
public:
	MapStorage(ISaveHandler *isavehandler);
	~MapStorage();

	MapDataBase* loadData(const std::type_info &classType, const std::string &s);
	void setData(const std::string &s, MapDataBase *mapdatabase);
	void saveAllData();
	int_t getUniqueDataId(const std::string &s);

private:
	void saveData(MapDataBase *mapdatabase);
	void loadIdCounts();
	MapDataBase* loadDataFromFile(const std::string &file, const std::string &file1, const std::string &file2);

	ISaveHandler *saveHandler;
	std::map<std::string, MapDataBase*> loadedDataMap;
	std::vector<MapDataBase*> loadedDataList;
	std::map<std::string, short_t> idCounts;
	JavaHashSet<jstring, IdCountStringHash, IdCountStringEqual> idCountOrder;
};
