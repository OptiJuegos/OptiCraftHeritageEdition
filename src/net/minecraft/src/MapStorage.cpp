#include "MapStorage.h"

#include <iostream>
#include <memory>
#include <algorithm>
#include <sstream>
#include <vector>
#include "platform/Log.h"
#include "platform/Storage.h"
#include "MapDataBase.h"
#include "ISaveHandler.h"
#include "NBTTagCompound.h"
#include "NBTBase.h"
#include "CompressedStreamTools.h"
#include "MapData.h"
#include "NBTTagShort.h"
#include "java/String.h"

namespace
{
// MapStorage used to reach the filesystem through std::fstream directly, which
// bypassed PlatformStorage and therefore the console path rules -- memory-card
// routing and the "nosave:" backend among them. Every read and write here now
// goes through the same seam SaveHandler uses.
bool readWholeFile(const std::string &path, std::string &out)
{
	std::vector<unsigned char> bytes;
	if (!PlatformStorage::readFile(path, bytes))
		return false;
	out.assign(reinterpret_cast<const char *>(bytes.data()), bytes.size());
	return true;
}

bool writeWholeFile(const std::string &path, const std::string &bytes)
{
	return PlatformStorage::writeFile(path, bytes.data(), bytes.size());
}
}

MapStorage::MapStorage(ISaveHandler *isavehandler) :
	saveHandler(isavehandler)
{
	loadIdCounts();
}

MapStorage::~MapStorage()
{
	for (MapDataBase *data : loadedDataList)
		delete data;
}

MapDataBase* MapStorage::loadData(const std::type_info &classType, const std::string &s)
{
	auto it = loadedDataMap.find(s);
	if (it != loadedDataMap.end())
		return it->second;

	MapDataBase *mapdatabase = nullptr;
	if (saveHandler != nullptr)
	{
		try
		{
			std::string file = saveHandler->getMapFile(s);
			std::string bytes;
			if (!file.empty() && classType == typeid(MapData) && readWholeFile(file, bytes) && !bytes.empty())
			{
				std::istringstream input(bytes, std::ios::binary);
				std::unique_ptr<NBTTagCompound> root(CompressedStreamTools::readGzippedCompound(input));
				if (root && root->hasKey("data"))
				{
					mapdatabase = new MapData(s);
					mapdatabase->readFromNBT(root->getCompoundTag("data"));
				}
			}
		}
		catch (...)
		{
		}
	}
	if (mapdatabase != nullptr)
	{
		loadedDataMap[s] = mapdatabase;
		loadedDataList.push_back(mapdatabase);
	}
	return mapdatabase;
}

void MapStorage::setData(const std::string &s, MapDataBase *mapdatabase)
{
	if (mapdatabase == nullptr)
		throw std::runtime_error("Can't set null data");

	auto it = loadedDataMap.find(s);
	if (it != loadedDataMap.end())
	{
		MapDataBase *oldData = it->second;
		if (oldData == mapdatabase)
			return;

		auto lit = std::find(loadedDataList.begin(), loadedDataList.end(), oldData);
		if (lit != loadedDataList.end()) loadedDataList.erase(lit);
		loadedDataMap.erase(it);
		delete oldData;
	}
	loadedDataMap[s] = mapdatabase;
	loadedDataList.push_back(mapdatabase);
}

void MapStorage::saveAllData()
{
	for (auto *mapdatabase : loadedDataList)
	{
		if (mapdatabase->isDirty())
		{
			saveData(mapdatabase);
			mapdatabase->setDirty(false);
		}
	}
}

void MapStorage::saveData(MapDataBase *mapdatabase)
{
	if (saveHandler == nullptr) return;
	try
	{
		std::string file = saveHandler->getMapFile(mapdatabase->mapName);
		if (!file.empty())
		{
			std::unique_ptr<NBTTagCompound> root(new NBTTagCompound());
			NBTTagCompound *data = new NBTTagCompound();
			mapdatabase->writeToNBT(data);
			root->setCompoundTag("data", data);
			std::ostringstream output(std::ios::binary);
			CompressedStreamTools::writeGzippedCompoundToOutputStream(root.get(), output);
			writeWholeFile(file, output.str());
		}
	}
	catch (...)
	{
	}
}

void MapStorage::loadIdCounts()
{
	try
	{
		idCounts.clear();
		idCountOrder.clear();
		if (saveHandler == nullptr) return;
		std::string file = saveHandler->getMapFile("idcounts");
		MC_LOG_DEBUG("save", "idcounts read begin\n");
		std::string bytes;
		const bool read = !file.empty() && readWholeFile(file, bytes);
		MC_LOG_DEBUG("save", "idcounts read %s, %u bytes\n",
		             read ? "OK" : "MISS", (unsigned)bytes.size());
		if (read && !bytes.empty())
		{
			std::istringstream input(bytes, std::ios::binary);
			std::unique_ptr<NBTTagCompound> counts(CompressedStreamTools::readCompound(input));
			if (counts)
			{
				for (NBTBase *tag : counts->getTags())
				{
					if (auto *shortTag = dynamic_cast<NBTTagShort *>(tag))
					{
						const jstring key = shortTag->getKey();
						idCounts[key] = shortTag->shortValue;
						idCountOrder.add(key);
					}
				}
			}
		}
	}
	catch (...)
	{
	}
}

int_t MapStorage::getUniqueDataId(const std::string &s)
{
	auto it = idCounts.find(s);
	short_t short1 = 0;
	if (it == idCounts.end())
		short1 = 0;
	else
		short1 = it->second + 1;

	idCounts[s] = short1;
	idCountOrder.add(jstring(s));

	if (saveHandler == nullptr)
		return short1;

	try
	{
		std::string file = saveHandler->getMapFile("idcounts");
		if (!file.empty())
		{
			std::unique_ptr<NBTTagCompound> counts(new NBTTagCompound());
			for (const jstring &key : idCountOrder.valuesInIterationOrder())
			{
				auto countIt = idCounts.find(key);
				if (countIt != idCounts.end())
					counts->setShort(key, countIt->second);
			}
			std::ostringstream output(std::ios::binary);
			CompressedStreamTools::writeCompound(counts.get(), output);
			writeWholeFile(file, output.str());
		}
	}
	catch (...)
	{
	}
	return short1;
}

MapDataBase* MapStorage::loadDataFromFile(const std::string &file, const std::string &file1, const std::string &file2)
{
	std::string bytes;
	if (!readWholeFile(file, bytes) || bytes.empty()) return nullptr;
	std::istringstream input(bytes, std::ios::binary);
	std::unique_ptr<NBTTagCompound> root(CompressedStreamTools::readGzippedCompound(input));
	if (!root || !root->hasKey(file2))
		return nullptr;
	MapData *data = new MapData(file1);
	data->readFromNBT(root->getCompoundTag(file2));
	return data;
}
