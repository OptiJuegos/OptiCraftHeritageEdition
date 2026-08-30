#pragma once

#include <map>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <optional>
#include "java/Type.h"

class Session;
class StatFileWriter;
class StatBase;

// net.minecraft.src.StatsSyncher
class StatsSyncher
{
public:
	StatsSyncher(Session *session, StatFileWriter *statfilewriter, const std::string &file);
	~StatsSyncher();

	void syncFromServer();
	void syncToServer(std::map<StatBase*, int_t> &map);
	void syncStatsFileWithMap(std::map<StatBase*, int_t> &map);
	bool isIdle();
	void update();
	void receiveStats();
	void sendStats(std::map<StatBase*, int_t> map);
	void setBusy(bool busy);

private:
	void renameFile(const std::string &file, const std::string &s, const std::string &file1);
	std::optional<std::map<StatBase*, int_t>> loadDataFromFile(const std::string &file, const std::string &file1, const std::string &file2);
	std::map<StatBase*, int_t> parseStatsFile(const std::string &file);
	void writeStatsFile(std::map<StatBase*, int_t> &map, const std::string &file, const std::string &file1, const std::string &file2);

	std::atomic<bool> isBusy;
	std::map<StatBase*, int_t> serverStats;
	std::mutex resultMutex;
	std::map<StatBase*, int_t> pendingStats;
	StatFileWriter *statFileWriter;

	std::string unsentDataFile;
	std::string dataFile;
	std::string unsentTempFile;
	std::string tempFile;
	std::string unsentOldFile;
	std::string oldFile;

	Session *session;
	int_t syncCounter;
	int_t packetSize;
	std::thread worker;
};
