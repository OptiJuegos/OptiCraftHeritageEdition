#pragma once

#include <string>
#include <vector>

struct OchPackInfo
{
    std::string filePath;
    std::string fileName;
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    bool valid = false;
};

namespace OchPackReader
{
    // Read file bytes with full candidate path and ISO-9660 disc fallback
    bool readFileBytes(const std::string &path, std::vector<unsigned char> &out, std::string *resolvedPath = nullptr);

    // Read and parse mod.info / manifest.txt from an .ochpack zip file
    bool readInfo(const std::string &filePath, OchPackInfo &outInfo);

    // Read and parse directly from memory buffer
    bool readInfoFromBytes(const std::vector<unsigned char> &data, const std::string &originalPath, OchPackInfo &outInfo);

    // Scan a directory path for all *.ochpack files and parse their metadata
    std::vector<OchPackInfo> scanDirectory(const std::string &dirPath, std::vector<std::string> *outDebugLogs = nullptr);
}
