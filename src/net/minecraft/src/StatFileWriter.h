#pragma once

#include <map>
#include <string>
#include <memory>
#include "java/Type.h"
#include "platform/PlatformConfig.h"

class Session;
class StatBase;
class Achievement;
class StatsSyncher;

// net.minecraft.src.StatFileWriter
class StatFileWriter
{
public:
	StatFileWriter(Session *session, const std::string &file);
	~StatFileWriter();

	void readStat(StatBase *statbase, int_t i);
	std::map<StatBase*, int_t> getTempStats();
	void setTempStats(std::map<StatBase*, int_t> &map);
	void addPendingStats(std::map<StatBase*, int_t> &map);
	void addTempStats(std::map<StatBase*, int_t> &map);

#if !PLATFORM_LOCAL_STATS
	static std::map<StatBase*, int_t> parseStats(const std::string &s);
	static std::string serializeStats(const std::string &s, const std::string &s1, std::map<StatBase*, int_t> &map);
#endif

	bool hasAchievementUnlocked(Achievement *achievement);
	bool canUnlockAchievement(Achievement *achievement);
	int_t writeStat(StatBase *statbase);

	void prepareStatsForSync();
	void syncStats();
	void updateStatsSync();

private:
	void writeStatToMap(std::map<StatBase*, int_t> &map, StatBase *statbase, int_t i);

	std::map<StatBase*, int_t> tempStats;
	std::map<StatBase*, int_t> writtenStats;
	bool hasUnsentStats;
	StatsSyncher *statsSyncher;

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
	// Local flat-file persistence, used instead of StatsSyncher's networked
	// JSON+checksum path (see the platform branch in StatFileWriter.cpp).
	void loadLocalStats();
	void saveLocalStats();
	std::string localStatsPath;
#endif
};
