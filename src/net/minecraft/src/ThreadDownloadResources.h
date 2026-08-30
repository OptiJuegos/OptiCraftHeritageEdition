#pragma once

#include <string>
#include <atomic>
#include "platform/PlatformConfig.h"

#if !PLATFORM_LOCAL_RESOURCES_ONLY
#include <vector>
#include <thread>
#endif

class Minecraft;

// net.minecraft.src.ThreadDownloadResources
class ThreadDownloadResources
{
public:
	ThreadDownloadResources(const std::string &file, Minecraft *minecraft);
	~ThreadDownloadResources();

	void start();
	void run();
	void reloadResources();
	void closeMinecraft();

	std::string resourcesFolder;

private:
#if !PLATFORM_LOCAL_RESOURCES_ONLY
	struct ResourceEntry
	{
		std::string key;
		long long size;
	};

	void downloadAndInstallResource(const std::string &baseUrl, const std::string &s, long long l, int i);
	void downloadResource(const std::string &url, const std::string &file, long long l);
	static std::vector<ResourceEntry> parseResourceList(const std::string &xml);
	static std::string decodeXmlDocument(const std::vector<unsigned char> &data);
	static std::string resolveUrl(const std::string &base, const std::string &relative);
	bool httpGet(const std::string &url, std::vector<unsigned char> &data);
	static std::string escapeUrlPath(const std::string &s);
	static std::string getXmlTag(const std::string &s, const std::string &tag);
#endif
	void loadResource(const std::string &file, const std::string &s);

	Minecraft *mc;
	std::atomic_bool closing;
#if !PLATFORM_LOCAL_RESOURCES_ONLY
	std::thread worker;
#endif
};
