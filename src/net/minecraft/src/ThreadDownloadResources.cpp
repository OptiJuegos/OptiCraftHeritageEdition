#include "ThreadDownloadResources.h"
#include "platform/Log.h"

#include "Minecraft.h"
#include "GameResources.h"
#include "platform/PlatformConfig.h"
#include "platform/Storage.h"

#include <vector>

#if !PLATFORM_LOCAL_RESOURCES_ONLY
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <sstream>

#include "java/JavaNetwork.h"
#include "java/String.h"
namespace fs = std::filesystem;
#endif

#if PLATFORM_LOCAL_RESOURCES_ONLY
// Counts every file visited by loadResource() below, regardless of whether
// installResource() recognised its top-level folder. A total of 0 after the
// scan means resourcesFolder never resolved to real files -- the first thing
// to check if audio is silent -- as distinct from files being found but not
// registering with SoundManager.
static int s_localResourceCount = 0;
#endif

ThreadDownloadResources::ThreadDownloadResources(const std::string &file, Minecraft *minecraft)
	: resourcesFolder(GameResources::getAudioResourcesDir())
	, mc(minecraft)
	, closing(false)
{
	(void)file;
#if !PLATFORM_LOCAL_RESOURCES_ONLY
	fs::create_directories(resourcesFolder);
#endif
}

ThreadDownloadResources::~ThreadDownloadResources()
{
	closing.store(true);
#if !PLATFORM_LOCAL_RESOURCES_ONLY
	if (worker.joinable() && worker.get_id() != std::this_thread::get_id())
		worker.join();
#endif
}

void ThreadDownloadResources::start()
{
#if PLATFORM_LOCAL_RESOURCES_ONLY
	// Local-resource profiles scan synchronously. This avoids a worker on console
	// ports and also gives NO_NETWORK desktop builds the same deterministic path.
	run();
	MC_LOG_DEBUG("audio", "resource scan: %d files under %s\n", s_localResourceCount, resourcesFolder.c_str());
#else
	worker = std::thread(&ThreadDownloadResources::run, this);
#endif
}

void ThreadDownloadResources::run()
{
#if PLATFORM_LOCAL_RESOURCES_ONLY
	loadResource(resourcesFolder, "");
#else
	try
	{
		std::string url = "http://s3.amazonaws.com/MinecraftResources/";
		std::vector<unsigned char> data;
		if (!httpGet(url, data))
			throw std::runtime_error("Could not load MinecraftResources listing");

		std::string xml = decodeXmlDocument(data);
		std::vector<ResourceEntry> entries = parseResourceList(xml);
		for (int_t i = 0; i < 2; i++)
		{
			for (const ResourceEntry &entry : entries)
			{
				if (entry.size <= 0LL)
					continue;
				downloadAndInstallResource(url, entry.key, entry.size, i);
				if (closing)
					return;
			}
		}
	}
	catch (std::exception &exception)
	{
		loadResource(resourcesFolder, "");
		MC_LOG_ERROR("game", "%s\n", exception.what());
	}
#endif
}

void ThreadDownloadResources::reloadResources()
{
	loadResource(resourcesFolder, "");
}

void ThreadDownloadResources::loadResource(const std::string &file, const std::string &s)
{
	std::vector<std::string> entries;
	if (!PlatformStorage::listPathEntries(file, entries))
		return;

	for (const std::string& name : entries)
	{
		if (closing)
			return;

		const std::string path = PlatformStorage::join(file, name);
		if (PlatformStorage::pathIsDirectory(path))
		{
			loadResource(path, s + name + "/");
			continue;
		}

		try
		{
			mc->installResource(s + name, path);
#if PLATFORM_LOCAL_RESOURCES_ONLY
			++s_localResourceCount;
#endif
		}
		catch (...)
		{
			MC_LOG_WARN("resources", "Failed to add %s%s\n", s.c_str(), name.c_str());
		}
	}
}

#if !PLATFORM_LOCAL_RESOURCES_ONLY
void ThreadDownloadResources::downloadAndInstallResource(const std::string &baseUrl, const std::string &s, long long l, int i)
{
	try
	{
		std::size_t j = s.find('/');
		if (j == std::string::npos)
			return;
		std::string s1 = s.substr(0, j);
		if (s1 == "sound" || s1 == "newsound")
		{
			if (i != 0)
				return;
		}
		else if (i != 1)
		{
			return;
		}

		fs::path file = fs::path(resourcesFolder) / fs::path(s);
		if (!fs::exists(file) || (long long)fs::file_size(file) != l)
		{
			fs::create_directories(file.parent_path());
			downloadResource(escapeUrlPath(resolveUrl(baseUrl, s)), file.string(), l);
			if (closing)
				return;
		}
		mc->installResource(s, file.string());
	}
	catch (std::exception &exception)
	{
		MC_LOG_ERROR("game", "%s\n", exception.what());
	}
}

void ThreadDownloadResources::downloadResource(const std::string &url, const std::string &file, long long)
{
	std::vector<unsigned char> data;
	if (!httpGet(url, data))
		throw std::runtime_error("Failed to download resource: " + url);
	std::ofstream output(file, std::ios::binary | std::ios::trunc);
	if (!output)
		throw std::runtime_error("Failed to write resource: " + file);
	const std::size_t chunk = 4096;
	for (std::size_t pos = 0; pos < data.size(); pos += chunk)
	{
		if (closing)
			return;
		std::size_t count = std::min(chunk, data.size() - pos);
		output.write(reinterpret_cast<const char *>(&data[pos]), (std::streamsize)count);
	}
}

namespace
{
static std::string xmlEntityDecode(const std::string &value)
{
    std::string out;
    out.reserve(value.size());
    for (std::size_t i = 0; i < value.size();)
    {
        if (value[i] != '&') { out += value[i++]; continue; }
        const std::size_t semi = value.find(';', i + 1);
        if (semi == std::string::npos) { out += value[i++]; continue; }
        const std::string ent = value.substr(i + 1, semi - i - 1);
        std::uint32_t cp = 0;
        bool numeric = false;
        if (ent == "amp") out += '&';
        else if (ent == "lt") out += '<';
        else if (ent == "gt") out += '>';
        else if (ent == "quot") out += '"';
        else if (ent == "apos") out += '\'';
        else if (!ent.empty() && ent[0] == '#')
        {
            numeric = true;
            int base = 10;
            std::size_t j = 1;
            if (j < ent.size() && (ent[j] == 'x' || ent[j] == 'X')) { base = 16; ++j; }
            if (j >= ent.size()) numeric = false;
            for (; numeric && j < ent.size(); ++j)
            {
                unsigned char c = static_cast<unsigned char>(ent[j]);
                int d = -1;
                if (c >= '0' && c <= '9') d = c - '0';
                else if (base == 16 && c >= 'a' && c <= 'f') d = c - 'a' + 10;
                else if (base == 16 && c >= 'A' && c <= 'F') d = c - 'A' + 10;
                if (d < 0 || d >= base || cp > (0x10ffffu - static_cast<unsigned>(d)) / static_cast<unsigned>(base)) numeric = false;
                else cp = cp * static_cast<unsigned>(base) + static_cast<unsigned>(d);
            }
            if (numeric && cp <= 0x10ffffu && !(cp >= 0xd800u && cp <= 0xdfffu))
            {
                std::vector<char_t> units;
                if (cp <= 0xffffu) units.push_back(static_cast<char_t>(cp));
                else
                {
                    cp -= 0x10000u;
                    units.push_back(static_cast<char_t>(0xd800u + (cp >> 10)));
                    units.push_back(static_cast<char_t>(0xdc00u + (cp & 0x3ffu)));
                }
                out += String::fromUtf16(units);
            }
            else numeric = false;
        }
        else
        {
            out.append(value, i, semi - i + 1);
            i = semi + 1;
            continue;
        }
        if (!numeric && !ent.empty() && ent[0] == '#')
            out.append(value, i, semi - i + 1);
        i = semi + 1;
    }
    return out;
}

static std::string localXmlName(std::string name)
{
    std::size_t colon = name.rfind(':');
    if (colon != std::string::npos) name.erase(0, colon + 1);
    return name;
}
}

std::string ThreadDownloadResources::decodeXmlDocument(const std::vector<unsigned char> &data)
{
    if (data.empty()) return std::string();
    std::vector<char_t> utf16;
    std::size_t off = 0;
    bool be = false, le = false;
    if (data.size() >= 2 && data[0] == 0xfe && data[1] == 0xff) { be = true; off = 2; }
    else if (data.size() >= 2 && data[0] == 0xff && data[1] == 0xfe) { le = true; off = 2; }
    else if (data.size() >= 4 && data[0] == 0x00 && data[1] == 0x3c && data[2] == 0x00 && data[3] == 0x3f) be = true;
    else if (data.size() >= 4 && data[0] == 0x3c && data[1] == 0x00 && data[2] == 0x3f && data[3] == 0x00) le = true;
    if (be || le)
    {
        for (std::size_t i = off; i + 1 < data.size(); i += 2)
            utf16.push_back(static_cast<char_t>(be ? (static_cast<unsigned>(data[i]) << 8) | data[i + 1]
                                                    : (static_cast<unsigned>(data[i + 1]) << 8) | data[i]));
        return String::fromUtf16(utf16);
    }

    std::size_t utf8off = data.size() >= 3 && data[0] == 0xef && data[1] == 0xbb && data[2] == 0xbf ? 3 : 0;
    std::string bytes(reinterpret_cast<const char *>(data.data() + utf8off), data.size() - utf8off);

    // XML without a BOM can declare a single-byte encoding. The historical S3
    // listing is UTF-8, but support ISO-8859-1 rather than silently treating it
    // as UTF-8 if the declaration says otherwise.
    std::size_t declEnd = bytes.find("?>");
    if (bytes.rfind("<?xml", 0) == 0 && declEnd != std::string::npos)
    {
        std::string decl = bytes.substr(0, declEnd + 2);
        std::string lower = decl;
        for (char &c : lower) if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
        if (lower.find("encoding=\"iso-8859-1\"") != std::string::npos || lower.find("encoding='iso-8859-1'") != std::string::npos)
        {
            std::vector<char_t> units;
            units.reserve(bytes.size());
            for (unsigned char c : bytes) units.push_back(static_cast<char_t>(c));
            return String::fromUtf16(units);
        }
    }
    return bytes;
}

std::vector<ThreadDownloadResources::ResourceEntry> ThreadDownloadResources::parseResourceList(const std::string &xml)
{
    std::vector<ResourceEntry> entries;
    std::vector<std::string> stack;
    ResourceEntry current{"", 0};
    bool inContents = false;
    bool haveKey = false, haveSize = false;
    std::string text;

    auto flushText = [&]()
    {
        if (!inContents || stack.empty() || text.empty()) { text.clear(); return; }
        const std::string tag = localXmlName(stack.back());
        if (tag == "Key") { current.key += xmlEntityDecode(text); haveKey = true; }
        else if (tag == "Size")
        {
            const std::string decoded = xmlEntityDecode(text);
            long_t parsed = 0;
            if (!String::tryParseLong(decoded, parsed))
                throw std::invalid_argument("invalid resource size");
            current.size = parsed; haveSize = true;
        }
        text.clear();
    };

    for (std::size_t i = 0; i < xml.size();)
    {
        if (xml[i] != '<') { text.push_back(xml[i++]); continue; }
        flushText();
        if (xml.compare(i, 4, "<!--") == 0)
        {
            std::size_t e = xml.find("-->", i + 4); if (e == std::string::npos) throw std::invalid_argument("unterminated XML comment"); i = e + 3; continue;
        }
        if (xml.compare(i, 9, "<![CDATA[") == 0)
        {
            std::size_t e = xml.find("]]>", i + 9); if (e == std::string::npos) throw std::invalid_argument("unterminated CDATA");
            text.append(xml, i + 9, e - (i + 9)); i = e + 3; continue;
        }
        if (xml.compare(i, 2, "<?") == 0)
        {
            std::size_t e = xml.find("?>", i + 2); if (e == std::string::npos) throw std::invalid_argument("unterminated XML PI"); i = e + 2; continue;
        }
        if (xml.compare(i, 2, "<!") == 0)
        {
            std::size_t e = xml.find('>', i + 2); if (e == std::string::npos) throw std::invalid_argument("unterminated XML declaration"); i = e + 1; continue;
        }

        bool closing = i + 1 < xml.size() && xml[i + 1] == '/';
        std::size_t j = i + (closing ? 2 : 1);
        while (j < xml.size() && std::isspace(static_cast<unsigned char>(xml[j]))) ++j;
        std::size_t ns = j;
        while (j < xml.size() && !std::isspace(static_cast<unsigned char>(xml[j])) && xml[j] != '>' && xml[j] != '/') ++j;
        if (j == ns) throw std::invalid_argument("invalid XML tag");
        std::string name = xml.substr(ns, j - ns);
        bool quote = false; char quoteChar = 0;
        std::size_t e = j;
        for (; e < xml.size(); ++e)
        {
            char c = xml[e];
            if (quote) { if (c == quoteChar) quote = false; }
            else if (c == '\'' || c == '"') { quote = true; quoteChar = c; }
            else if (c == '>') break;
        }
        if (e >= xml.size()) throw std::invalid_argument("unterminated XML tag");
        std::size_t k = e;
        while (k > j && std::isspace(static_cast<unsigned char>(xml[k - 1]))) --k;
        bool selfClosing = !closing && k > j && xml[k - 1] == '/';
        const std::string local = localXmlName(name);

        if (closing)
        {
            if (!stack.empty()) stack.pop_back();
            if (local == "Contents" && inContents)
            {
                if (haveKey && haveSize) entries.push_back(current);
                current = ResourceEntry{"", 0}; haveKey = haveSize = false; inContents = false;
            }
        }
        else
        {
            stack.push_back(name);
            if (local == "Contents") { inContents = true; current = ResourceEntry{"", 0}; haveKey = haveSize = false; }
            if (selfClosing)
            {
                if (local == "Contents") inContents = false;
                stack.pop_back();
            }
        }
        i = e + 1;
    }
    flushText();
    return entries;
}

std::string ThreadDownloadResources::resolveUrl(const std::string &base, const std::string &relative)
{
    if (relative.find("://") != std::string::npos) return relative;
    const std::size_t scheme = base.find("://");
    if (scheme == std::string::npos) return base + relative;
    const std::size_t authorityEnd = base.find('/', scheme + 3);
    const std::string origin = authorityEnd == std::string::npos ? base : base.substr(0, authorityEnd);
    std::string path;
    if (!relative.empty() && relative[0] == '/') path = relative;
    else
    {
        std::string basePath = authorityEnd == std::string::npos ? "/" : base.substr(authorityEnd);
        std::size_t slash = basePath.rfind('/');
        path = basePath.substr(0, slash == std::string::npos ? 0 : slash + 1) + relative;
    }
    std::vector<std::string> parts;
    std::size_t pos = 0;
    while (pos <= path.size())
    {
        std::size_t slash = path.find('/', pos);
        std::string part = path.substr(pos, slash == std::string::npos ? std::string::npos : slash - pos);
        if (part == "..") { if (!parts.empty()) parts.pop_back(); }
        else if (!part.empty() && part != ".") parts.push_back(part);
        if (slash == std::string::npos) break;
        pos = slash + 1;
    }
    std::string normalized = "/";
    for (std::size_t n = 0; n < parts.size(); ++n) { if (n) normalized += '/'; normalized += parts[n]; }
    return origin + normalized;
}

bool ThreadDownloadResources::httpGet(const std::string &url, std::vector<unsigned char> &data)
{
	return JavaNetwork::readUrl(url, data);
}

std::string ThreadDownloadResources::escapeUrlPath(const std::string &s)
{
	std::string out;
	for (char c : s)
	{
		if (c == ' ')
			out += "%20";
		else
			out += c;
	}
	return out;
}

std::string ThreadDownloadResources::getXmlTag(const std::string &s, const std::string &tag)
{
    const std::string open = "<" + tag + ">";
    const std::string close = "</" + tag + ">";
    std::size_t start = s.find(open);
    if (start == std::string::npos) return "";
    start += open.size();
    std::size_t end = s.find(close, start);
    if (end == std::string::npos) return "";
    return xmlEntityDecode(s.substr(start, end - start));
}
#endif // !PLATFORM_LOCAL_RESOURCES_ONLY

void ThreadDownloadResources::closeMinecraft()
{
	closing.store(true);
}
