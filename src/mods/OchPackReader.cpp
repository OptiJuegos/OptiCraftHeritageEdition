#include "OchPackReader.h"
#include "platform/Storage.h"
#include "platform/Log.h"
#include "platform/storage/PosixFileSystem.h"
#include "unzip.h"

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <sstream>

static void parseModInfoString(const std::string &content, OchPackInfo &outInfo)
{
    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line))
    {
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
            line.pop_back();
        while (!line.empty() && (line.front() == ' ' || line.front() == '\t'))
            line.erase(line.begin());

        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;

        size_t sep = line.find('=');
        if (sep == std::string::npos)
            sep = line.find(':');

        if (sep != std::string::npos)
        {
            std::string key = line.substr(0, sep);
            std::string val = line.substr(sep + 1);

            while (!key.empty() && (key.back() == ' ' || key.back() == '\t'))
                key.pop_back();
            while (!val.empty() && (val.front() == ' ' || val.front() == '\t'))
                val.erase(val.begin());

            std::transform(key.begin(), key.end(), key.begin(), ::tolower);

            if (key == "id")
                outInfo.id = val;
            else if (key == "name")
                outInfo.name = val;
            else if (key == "version")
                outInfo.version = val;
            else if (key == "author")
                outInfo.author = val;
            else if (key == "description")
                outInfo.description = val;
        }
    }
}

// In-memory stream buffer for minizip to avoid buggy IOP seek drivers
struct MemZipBuffer
{
    const unsigned char *buffer;
    uLong size;
    uLong pos;
};

static voidpf ZCALLBACK mem_open(voidpf opaque, const char *filename, int mode)
{
    return opaque;
}

static uLong ZCALLBACK mem_read(voidpf opaque, voidpf stream, void *buf, uLong size)
{
    MemZipBuffer *m = static_cast<MemZipBuffer *>(stream);
    if (!m || m->pos >= m->size)
        return 0;
    uLong avail = m->size - m->pos;
    uLong toRead = (size < avail) ? size : avail;
    std::memcpy(buf, m->buffer + m->pos, toRead);
    m->pos += toRead;
    return toRead;
}

static long ZCALLBACK mem_tell(voidpf opaque, voidpf stream)
{
    MemZipBuffer *m = static_cast<MemZipBuffer *>(stream);
    return m ? static_cast<long>(m->pos) : -1;
}

static long ZCALLBACK mem_seek(voidpf opaque, voidpf stream, uLong offset, int origin)
{
    MemZipBuffer *m = static_cast<MemZipBuffer *>(stream);
    if (!m)
        return -1;
    switch (origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR:
        m->pos += offset;
        break;
    case ZLIB_FILEFUNC_SEEK_END:
        m->pos = m->size + offset;
        break;
    case ZLIB_FILEFUNC_SEEK_SET:
        m->pos = offset;
        break;
    default:
        return -1;
    }
    return 0;
}

static int ZCALLBACK mem_close(voidpf opaque, voidpf stream)
{
    return 0;
}

static int ZCALLBACK mem_error(voidpf opaque, voidpf stream)
{
    return 0;
}

static void addUniqueCandidate(std::vector<std::string> &candidates, const std::string &cand)
{
    if (cand.empty())
        return;
    for (const auto &c : candidates)
    {
        if (c == cand)
            return;
    }
    candidates.push_back(cand);
}

static void generateCandidates(const std::string &path, std::vector<std::string> &candidates)
{
    addUniqueCandidate(candidates, path);

    size_t colon = path.find(':');
    std::string scheme = (colon != std::string::npos) ? path.substr(0, colon + 1) : "";
    std::string rest = (colon != std::string::npos) ? path.substr(colon + 1) : path;

    // Strip leading slashes from rest
    size_t start = 0;
    while (start < rest.size() && (rest[start] == '/' || rest[start] == '\\'))
        start++;
    std::string cleanRest = rest.substr(start);

    std::string restSlash = cleanRest;
    std::string restBs = cleanRest;
    for (char &c : restSlash) if (c == '\\') c = '/';
    for (char &c : restBs) if (c == '/') c = '\\';

    std::string upperRestBs = restBs;
    for (char &c : upperRestBs) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    std::string lowerScheme = scheme;
    for (char &c : lowerScheme) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    if (!scheme.empty())
    {
        // 1. scheme/rest
        addUniqueCandidate(candidates, lowerScheme + "/" + restSlash);
        // 2. scheme:rest
        addUniqueCandidate(candidates, lowerScheme + restSlash);

        // CDROM ISO9660 variants with uppercase and ;1
        if (lowerScheme.find("cdrom") != std::string::npos)
        {
            std::string upperScheme = "CDROM0:";
            addUniqueCandidate(candidates, upperScheme + "\\" + upperRestBs + ";1");
            addUniqueCandidate(candidates, upperScheme + "/" + upperRestBs + ";1");
            addUniqueCandidate(candidates, lowerScheme + "/" + restSlash + ";1");
        }
    }
    else
    {
        addUniqueCandidate(candidates, restSlash);
    }
}

static bool readFileBytesInternal(const std::string &path, std::vector<unsigned char> &out, std::string *resolvedPath = nullptr)
{
    std::vector<std::string> candidates;
    generateCandidates(path, candidates);

    for (const auto &candidate : candidates)
    {
        FILE *f = std::fopen(candidate.c_str(), "rb");
        if (!f)
            continue;

        long sz = -1;
        if (std::fseek(f, 0, SEEK_END) == 0)
        {
            sz = std::ftell(f);
        }

        if (sz <= 0)
        {
            const std::int64_t reported = PlatformStorage::fileSize(candidate);
            if (reported > 0 && reported <= 0x7fffffffLL)
                sz = static_cast<long>(reported);
        }

        // Always re-open fresh: on PS2 optical drives, seeking to SEEK_END can leave the driver in an invalid state
        std::fclose(f);
        f = std::fopen(candidate.c_str(), "rb");
        if (!f)
            continue;

        out.clear();
        if (sz > 0)
        {
            out.resize(static_cast<size_t>(sz));
            size_t readCount = std::fread(out.data(), 1, static_cast<size_t>(sz), f);
            std::fclose(f);
            if (readCount > 0)
            {
                if (readCount < static_cast<size_t>(sz))
                    out.resize(readCount);
                if (resolvedPath)
                    *resolvedPath = candidate;
                return true;
            }
        }
        else
        {
            // Streamed chunk read fallback for drivers that don't report size
            unsigned char buf[4096];
            size_t n = 0;
            while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0)
            {
                out.insert(out.end(), buf, buf + n);
            }
            std::fclose(f);
            if (!out.empty())
            {
                if (resolvedPath)
                    *resolvedPath = candidate;
                return true;
            }
        }
    }
    return false;
}

namespace OchPackReader
{
bool readInfoFromBytes(const std::vector<unsigned char> &data, const std::string &originalPath, OchPackInfo &outInfo)
{
    outInfo.valid = false;
    outInfo.filePath = originalPath;

    size_t lastSep = originalPath.find_last_of("/\\");
    outInfo.fileName = (lastSep == std::string::npos) ? originalPath : originalPath.substr(lastSep + 1);

    if (data.size() < 22)
        return false;

    MemZipBuffer memBuf{ data.data(), static_cast<uLong>(data.size()), 0 };
    zlib_filefunc_def filefunc;
    filefunc.zopen_file = mem_open;
    filefunc.zread_file = mem_read;
    filefunc.zwrite_file = nullptr;
    filefunc.ztell_file = mem_tell;
    filefunc.zseek_file = mem_seek;
    filefunc.zclose_file = mem_close;
    filefunc.zerror_file = mem_error;
    filefunc.opaque = &memBuf;

    unzFile uf = unzOpen2("mem", &filefunc);
    if (!uf)
    {
        std::printf("[OptiCraftMods] Failed unzOpen2 for '%s'\n", originalPath.c_str());
        return false;
    }

    int res = unzLocateFile(uf, "mod.info", 2);
    if (res != UNZ_OK)
        res = unzLocateFile(uf, "manifest.txt", 2);

    if (res != UNZ_OK)
    {
        std::printf("[OptiCraftMods] mod.info not found inside '%s'\n", originalPath.c_str());
        unzClose(uf);
        return false;
    }

    unz_file_info fileInfo;
    if (unzGetCurrentFileInfo(uf, &fileInfo, nullptr, 0, nullptr, 0, nullptr, 0) != UNZ_OK)
    {
        unzClose(uf);
        return false;
    }

    if (unzOpenCurrentFile(uf) != UNZ_OK)
    {
        unzClose(uf);
        return false;
    }

    uLong toRead = fileInfo.uncompressed_size > 4096 ? 4096 : fileInfo.uncompressed_size;
    std::vector<char> buffer(toRead + 1, 0);
    int readBytes = unzReadCurrentFile(uf, buffer.data(), static_cast<unsigned int>(toRead));
    unzCloseCurrentFile(uf);
    unzClose(uf);

    if (readBytes <= 0)
        return false;

    buffer[readBytes] = '\0';
    parseModInfoString(std::string(buffer.data()), outInfo);

    if (outInfo.id.empty())
    {
        std::string base = outInfo.fileName;
        size_t dot = base.find_last_of('.');
        if (dot != std::string::npos)
            base = base.substr(0, dot);
        outInfo.id = base;
    }
    if (outInfo.name.empty())
        outInfo.name = outInfo.id;
    if (outInfo.version.empty())
        outInfo.version = "1.0";

    outInfo.valid = true;
    std::printf("[OptiCraftMods] Loaded: id='%s' name='%s' ver='%s' (%s)\n",
                outInfo.id.c_str(), outInfo.name.c_str(), outInfo.version.c_str(), originalPath.c_str());
    return true;
}

bool readFileBytes(const std::string &path, std::vector<unsigned char> &out, std::string *resolvedPath)
{
    return readFileBytesInternal(path, out, resolvedPath);
}

bool readInfo(const std::string &filePath, OchPackInfo &outInfo)
{
    std::vector<unsigned char> data;
    std::string resolved;
    if (!readFileBytes(filePath, data, &resolved))
        return false;
    return readInfoFromBytes(data, resolved, outInfo);
}

std::vector<OchPackInfo> scanDirectory(const std::string &dirPath, std::vector<std::string> *outDebugLogs)
{
    std::vector<OchPackInfo> results;
    std::vector<std::string> candidateFiles;

    if (outDebugLogs)
        outDebugLogs->push_back("Scanning: " + dirPath);

    // 1. Try reading packlist.txt in this directory
    static const char *const LIST_NAMES[] = {
        "packlist.txt",
        "PACKLIST.TXT"
    };

    bool foundPackList = false;
    for (const char *listName : LIST_NAMES)
    {
        std::string listPath = PlatformStorage::join(dirPath, listName);
        std::vector<unsigned char> listData;
        std::string resolvedListPath;
        if (readFileBytes(listPath, listData, &resolvedListPath) && !listData.empty())
        {
            foundPackList = true;
            if (outDebugLogs)
                outDebugLogs->push_back("Found " + std::string(listName) + " at " + resolvedListPath);
            std::string listContent(listData.begin(), listData.end());
            std::istringstream stream(listContent);
            std::string line;
            while (std::getline(stream, line))
            {
                while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
                    line.pop_back();
                while (!line.empty() && (line.front() == ' ' || line.front() == '\t'))
                    line.erase(line.begin());

                if (!line.empty() && line[0] != '#' && line[0] != ';')
                {
                    bool duplicate = false;
                    for (const auto &c : candidateFiles)
                    {
                        if (c == line)
                        {
                            duplicate = true;
                            break;
                        }
                    }
                    if (!duplicate)
                        candidateFiles.push_back(line);
                }
            }
            break;
        }
    }

    // 2. Only if NO packlist was found, try directory enumeration or probe names
    if (!foundPackList)
    {
        std::vector<std::string> entries;
        if (PlatformStorage::listPathEntries(dirPath, entries) && !entries.empty())
        {
            for (const auto &entry : entries)
            {
                if (entry.size() >= 8)
                {
                    std::string ext = entry.substr(entry.size() - 8);
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    if (ext == ".ochpack")
                    {
                        candidateFiles.push_back(entry);
                    }
                }
            }
        }

        // 3. Fallback probes only if neither packlist nor directory entries found anything
        if (candidateFiles.empty())
        {
            static const char *const PROBE_NAMES[] = {
                "SampleTestMod.ochpack"
            };

            for (const char *probeName : PROBE_NAMES)
            {
                candidateFiles.push_back(probeName);
            }
        }
    }

    // 4. Test each candidate file
    for (const auto &fileName : candidateFiles)
    {
        std::string fullPath = PlatformStorage::join(dirPath, fileName);
        std::vector<unsigned char> fileData;
        std::string resolved;
        if (readFileBytes(fullPath, fileData, &resolved))
        {
            OchPackInfo info;
            if (readInfoFromBytes(fileData, resolved, info))
            {
                bool duplicate = false;
                for (const auto &existing : results)
                {
                    if (existing.id == info.id)
                    {
                        duplicate = true;
                        break;
                    }
                }
                if (!duplicate)
                {
                    results.push_back(info);
                    if (outDebugLogs)
                        outDebugLogs->push_back("Found mod: " + info.name + " (" + info.version + ")");
                }
            }
        }
    }

    return results;
}
}
