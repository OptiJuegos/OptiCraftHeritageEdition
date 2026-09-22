#include "platform/storage/PakFile.h"

#include <vector>

#if PLATFORM_PS2
#include <fcntl.h>
#include <unistd.h>
#endif

#if PLATFORM_PS2

PakFile::PakFile()
    : fd_(-1), file_(nullptr)
{
}

PakFile::~PakFile()
{
    close();
}

bool PakFile::open(const std::string &path)
{
    close();

    std::vector<std::string> candidates;
    candidates.push_back(path);

    // Try slash / backslash variants after device colon (e.g. host:assets.pak <-> host:/assets.pak)
    const size_t colon = path.find(':');
    if (colon != std::string::npos)
    {
        if (colon + 1 < path.size() && path[colon + 1] != '/' && path[colon + 1] != '\\')
        {
            candidates.push_back(path.substr(0, colon + 1) + "/" + path.substr(colon + 1));
            candidates.push_back(path.substr(0, colon + 1) + "\\" + path.substr(colon + 1));
        }
        else if (colon + 1 < path.size() && (path[colon + 1] == '/' || path[colon + 1] == '\\'))
        {
            candidates.push_back(path.substr(0, colon + 1) + path.substr(colon + 2));
        }
        // Bare filename without scheme (current directory fallback)
        candidates.push_back(path.substr(colon + 1));
    }

    // First try POSIX open/read
    for (const auto &cand : candidates)
    {
        fd_ = ::open(cand.c_str(), O_RDONLY);
        if (fd_ >= 0)
        {
            unsigned char testBuf[4];
            if (::lseek(fd_, 0, SEEK_SET) == 0 && ::read(fd_, testBuf, 4) == 4)
            {
                ::lseek(fd_, 0, SEEK_SET);
                return true;
            }
            ::close(fd_);
            fd_ = -1;
        }
    }

    // Fallback: standard stdio fopen/fread (essential for PCSX2 HostFS)
    for (const auto &cand : candidates)
    {
        file_ = std::fopen(cand.c_str(), "rb");
        if (file_ != nullptr)
        {
            unsigned char testBuf[4];
            if (std::fread(testBuf, 1, 4, file_) == 4 && std::fseek(file_, 0, SEEK_SET) == 0)
            {
                return true;
            }
            std::fclose(file_);
            file_ = nullptr;
        }
    }

    return false;
}

void PakFile::close()
{
    if (fd_ >= 0)
        ::close(fd_);
    fd_ = -1;
    if (file_ != nullptr)
        std::fclose(file_);
    file_ = nullptr;
}

bool PakFile::isOpen() const
{
    return fd_ >= 0 || file_ != nullptr;
}

bool PakFile::readAt(std::uint32_t offset, void *dst, std::uint32_t length)
{
    if (dst == nullptr)
        return false;

    if (fd_ >= 0)
    {
        const off_t target = static_cast<off_t>(offset);
        if (::lseek(fd_, target, SEEK_SET) == target)
        {
            unsigned char *out = static_cast<unsigned char *>(dst);
            std::uint32_t remaining = length;
            bool ok = true;
            while (remaining > 0)
            {
                const int got = static_cast<int>(::read(fd_, out, remaining));
                if (got <= 0)
                {
                    ok = false;
                    break;
                }
                out += got;
                remaining -= static_cast<std::uint32_t>(got);
            }
            if (ok)
                return true;
        }
    }

    if (file_ != nullptr)
    {
        if (std::fseek(file_, static_cast<long>(offset), SEEK_SET) == 0)
        {
            return std::fread(dst, 1, length, file_) == length;
        }
    }

    return false;
}

#else

PakFile::PakFile()
    : file_(nullptr)
{
}

PakFile::~PakFile()
{
    close();
}

bool PakFile::open(const std::string &path)
{
    close();
    file_ = std::fopen(path.c_str(), "rb");
    return file_ != nullptr;
}

void PakFile::close()
{
    if (file_ != nullptr)
        std::fclose(file_);
    file_ = nullptr;
}

bool PakFile::isOpen() const
{
    return file_ != nullptr;
}

bool PakFile::readAt(std::uint32_t offset, void *dst, std::uint32_t length)
{
    if (file_ == nullptr || dst == nullptr)
        return false;
    if (std::fseek(file_, static_cast<long>(offset), SEEK_SET) != 0)
        return false;
    return std::fread(dst, 1, length, file_) == length;
}

#endif
