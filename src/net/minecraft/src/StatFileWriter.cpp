#include "platform/Log.h"
#include "StatFileWriter.h"
#include "LocalStatsFormat.h"
#include "java/String.h"
#include "platform/PlatformConfig.h"

#include <iostream>
#include <stdexcept>
#if PLATFORM_LOCAL_STATS
#include "platform/Storage.h"
#else
#include <fstream>
#include <filesystem>
#endif
#include "StatBase.h"
#include "Achievement.h"
#if !PLATFORM_LOCAL_STATS
#include "StatsSyncher.h"
#endif
#include "Session.h"
#include "StatList.h"
#if !PLATFORM_LOCAL_STATS
#include "MD5String.h"
#include "J_JdomParser.h"
#include "J_JsonRootNode.h"
#include "J_JsonNode.h"
#include "J_InvalidSyntaxException.h"
#endif

#if !PLATFORM_LOCAL_STATS
namespace fs = std::filesystem;
#endif

StatFileWriter::StatFileWriter(Session *session, const std::string &file) :
	hasUnsentStats(false),
	statsSyncher(nullptr)
{
#if PLATFORM_LOCAL_STATS
	// No online sync backend here (avoids std::filesystem/std::thread during
	// boot), but stats/achievements still need to survive a restart, so they
	// go through a small local flat file instead -- see loadLocalStats()/
	// saveLocalStats() below. Global, not per-world: one profile per console,
	// matching how the rest of this port treats save data.
	(void)session;
	localStatsPath = file + "/stats/stats.dat";
	loadLocalStats();
#else
	fs::path file1 = fs::path(file) / "stats";
	if (!fs::exists(file1))
		fs::create_directories(file1);
	statsSyncher = new StatsSyncher(session, this, file1.string());
#endif
}

StatFileWriter::~StatFileWriter()
{
#if !PLATFORM_LOCAL_STATS
	delete statsSyncher;
	statsSyncher = nullptr;
#endif
}

#if PLATFORM_LOCAL_STATS
// Flat "statId value" lines -- no checksum/session wrapper, since this never
// leaves the console; serializeStats()/parseStats() below are for Mojang's
// online stats sync and carry machinery (checksums, session ids) this
// local-only path does not need.
void StatFileWriter::loadLocalStats()
{
	std::vector<unsigned char> bytes;
	if (!PlatformStorage::readFile(localStatsPath, bytes) || bytes.empty())
		return;
	const std::string blob(reinterpret_cast<const char *>(bytes.data()), bytes.size());

	std::vector<LocalStatsFormat::Entry> entries;
	LocalStatsFormat::parse(blob, entries);
	for (const LocalStatsFormat::Entry &entry : entries)
	{
		StatBase *statbase = StatList::getStatById(entry.first);
		if (statbase != nullptr)
			writeStatToMap(writtenStats, statbase, entry.second);
	}
	// hasAchievementUnlocked() reads tempStats, not writtenStats -- match
	// readStat()'s behaviour so achievements loaded from disk are recognised
	// as already unlocked right away, not just after the next local write.
	tempStats = writtenStats;
}

void StatFileWriter::saveLocalStats()
{
	std::vector<LocalStatsFormat::Entry> entries;
	entries.reserve(writtenStats.size());
	for (auto &pair : writtenStats)
	{
		if (pair.first == nullptr) continue;
		entries.emplace_back(pair.first->statId, pair.second);
	}
	const std::string blob = LocalStatsFormat::serialize(entries);

	PlatformStorage::writeFile(localStatsPath, blob.data(), blob.size());
}
#endif // PLATFORM_LOCAL_STATS

void StatFileWriter::readStat(StatBase *statbase, int_t i)
{
	writeStatToMap(writtenStats, statbase, i);
	writeStatToMap(tempStats, statbase, i);
	hasUnsentStats = true;
}

void StatFileWriter::writeStatToMap(std::map<StatBase*, int_t> &map, StatBase *statbase, int_t i)
{
	if (statbase == nullptr) return;
	auto it = map.find(statbase);
	int_t j = (it != map.end()) ? it->second : 0;
	map[statbase] = j + i;
}

std::map<StatBase*, int_t> StatFileWriter::getTempStats()
{
	return writtenStats;
}

void StatFileWriter::setTempStats(std::map<StatBase*, int_t> &map)
{
	if (map.empty()) return;
	hasUnsentStats = true;
	for (auto &pair : map)
	{
		writeStatToMap(writtenStats, pair.first, pair.second);
		writeStatToMap(tempStats, pair.first, pair.second);
	}
}

void StatFileWriter::addPendingStats(std::map<StatBase*, int_t> &map)
{
	if (map.empty()) return;
	for (auto &pair : map)
	{
		auto it = writtenStats.find(pair.first);
		int_t i = (it != writtenStats.end()) ? it->second : 0;
		tempStats[pair.first] = pair.second + i;
	}
}

void StatFileWriter::addTempStats(std::map<StatBase*, int_t> &map)
{
	if (map.empty()) return;
	hasUnsentStats = true;
	for (auto &pair : map)
	{
		writeStatToMap(writtenStats, pair.first, pair.second);
	}
}

#if !PLATFORM_LOCAL_STATS
std::map<StatBase*, int_t> StatFileWriter::parseStats(const std::string &s)
{
	std::map<StatBase*, int_t> hashmap;
	try
	{
		std::string sessionKey = "local";
		std::string checksumInput;
		J_JsonRootNode *root = J_JdomParser().parse(s);
		std::vector<J_JsonNode *> list = root->getArrayAt({std::string("stats-change")});
		for (J_JsonNode *node : list)
		{
			std::map<std::string, J_JsonNode *> fields = node->getFields();
			if (fields.empty())
				continue;
			auto it = fields.begin();
			int_t statId = 0;
			int_t statVal = 0;
			if (!String::tryParseInt(it->first, statId) ||
			    !String::tryParseInt(it->second->getText(), statVal))
				throw std::invalid_argument("invalid stat integer");
			StatBase *statbase = StatList::getStatById(statId);
			if (statbase == nullptr)
			{
				MC_LOG_WARN("stats", "%d is not a valid stat\n", statId);
			}
			else
			{
				checksumInput += statbase->statGuid + ",";
				checksumInput += std::to_string(statVal) + ",";
				hashmap[statbase] = statVal;
			}
		}
		MD5String md5(sessionKey);
		std::string computed = md5.computeHash(checksumInput);
		std::string expected = root->getStringAt({std::string("checksum")});
		if (computed != expected)
		{
			MC_LOG_WARN("stats", "CHECKSUM MISMATCH\n");
			return {};
		}
	}
	catch (J_InvalidSyntaxException &e)
	{
		MC_LOG_ERROR("stats", "%s\n", e.what());
	}
	return hashmap;
}

std::string StatFileWriter::serializeStats(const std::string &s, const std::string &s1, std::map<StatBase*, int_t> &map)
{
	std::string result;
	std::string checksumInput;
	bool flag = true;
	result += "{\r\n";
	if (!s.empty() && !s1.empty())
	{
		result += "  \"user\":{\r\n";
		result += "    \"name\":\"" + s + "\",\r\n";
		result += "    \"sessionid\":\"" + s1 + "\"\r\n";
		result += "  },\r\n";
	}
	result += "  \"stats-change\":[";
	for (auto &pair : map)
	{
		StatBase *statbase = pair.first;
		if (statbase == nullptr) continue;
		int_t value = pair.second;
		if (!flag)
			result += "},";
		else
			flag = false;
		result += "\r\n    {\"" + std::to_string(statbase->statId) + "\":" + std::to_string(value);
		checksumInput += statbase->statGuid + ",";
		checksumInput += std::to_string(value) + ",";
	}
	if (!flag)
		result += "}";
	MD5String md5(s1);
	result += "\r\n  ],\r\n";
	result += "  \"checksum\":\"" + md5.computeHash(checksumInput) + "\"\r\n";
	result += "}";
	return result;
}
#endif // !PLATFORM_LOCAL_STATS

bool StatFileWriter::hasAchievementUnlocked(Achievement *achievement)
{
	return tempStats.find(achievement) != tempStats.end();
}

bool StatFileWriter::canUnlockAchievement(Achievement *achievement)
{
	return achievement->parentAchievement == nullptr || hasAchievementUnlocked(achievement->parentAchievement);
}

int_t StatFileWriter::writeStat(StatBase *statbase)
{
	auto it = tempStats.find(statbase);
	return (it != tempStats.end()) ? it->second : 0;
}

void StatFileWriter::prepareStatsForSync() // Java: empty
{
}

void StatFileWriter::syncStats() // Java: statsSyncher.syncStatsFileWithMap(func_27176_a()) — synchronous local save
{
#if PLATFORM_LOCAL_STATS
	if (!hasUnsentStats) return;
	saveLocalStats();
	hasUnsentStats = false;
#else
	if (statsSyncher == nullptr) return;
	std::map<StatBase*, int_t> map = getTempStats();
	statsSyncher->syncStatsFileWithMap(map);
#endif
}

void StatFileWriter::updateStatsSync()
{
#if PLATFORM_LOCAL_STATS
	// Gameplay dirties stats continuously (minutes played, movement, jumps,
	// etc.). The desktop synchronizer can absorb that cadence asynchronously,
	// but console storage cannot: Memory Card/libfat writes from this per-tick
	// hook turn ordinary gameplay into synchronous filesystem I/O. Keep the
	// dirty flag in RAM and persist through syncStats() at explicit save, world
	// exit and shutdown points instead.
	return;
#else
	if (statsSyncher == nullptr) return;
	if (hasUnsentStats && statsSyncher->isIdle())
	{
		std::map<StatBase*, int_t> map = getTempStats();
		statsSyncher->syncToServer(map);
	}
	statsSyncher->update();
#endif
}
