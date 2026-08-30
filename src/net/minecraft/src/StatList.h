#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include "java/Type.h"
#include "Item.h"

class StatBase;
class StatCrafting;

// net.minecraft.src.StatList
class StatList
{
public:
	static void initStats();
	static void initBlockStats();
	static void initItemStats();
	static void initCraftingStats();
	static void cleanup();

	static StatBase* getStatById(int_t i);

	static std::unordered_map<int_t, StatBase*> statMap;
	static std::vector<StatBase*> allStats;
	static std::vector<StatBase*> generalStats;
	static std::vector<StatBase*> itemStats;
	static std::vector<StatBase*> objectCraftStats;

	static StatBase *startGameStat;
	static StatBase *createWorldStat;
	static StatBase *loadWorldStat;
	static StatBase *joinMultiplayerStat;
	static StatBase *leaveGameStat;
	static StatBase *minutesPlayedStat;
	static StatBase *distanceWalkedStat;
	static StatBase *distanceSwumStat;
	static StatBase *distanceFallenStat;
	static StatBase *distanceClimbedStat;
	static StatBase *distanceFlownStat;
	static StatBase *distanceDoveStat;
	static StatBase *distanceByMinecartStat;
	static StatBase *distanceByBoatStat;
	static StatBase *distanceByPigStat;
	static StatBase *jumpStat;
	static StatBase *dropStat;
	static StatBase *damageDealtStat;
	static StatBase *damageTakenStat;
	static StatBase *deathsStat;
	static StatBase *mobKillsStat;
	static StatBase *playerKillsStat;
	static StatBase *fishCaughtStat;

	static StatBase *mineBlockStatArray[256];
	static StatBase *craftItemStatArray[Item::ITEM_LIST_SIZE];
	static StatBase *useItemStatArray[Item::ITEM_LIST_SIZE];
	static StatBase *breakItemStatArray[Item::ITEM_LIST_SIZE];

private:
	static StatBase** initMineBlockStats(const std::string &s, int_t i);
	static StatBase** initUseItemStats(StatBase **astatbase, const std::string &s, int_t i, int_t j, int_t k);
	static StatBase** initBreakItemStats(StatBase **astatbase, const std::string &s, int_t i, int_t j, int_t k);
	static void replaceAllSimilarBlocks(StatBase **astatbase);
	static void replaceSimilarBlocks(StatBase **astatbase, int_t i, int_t j);

	static bool statsInitialized;
	static bool blockStatsInitialized;
	static bool itemStatsInitialized;
};
