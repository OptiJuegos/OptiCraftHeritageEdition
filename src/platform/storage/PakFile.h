#pragma once

#include "platform/PlatformConfig.h"

#include <cstdint>
#include <cstdio>
#include <string>

// The one read handle PakArchive keeps on assets.pak: positioned reads only.
//
// On the PS2 this is a POSIX descriptor (open/lseek/read), the path
// Ps2StreamFile already proved on the console: libcglue's _read/_lseek go to
// the IOP directly and never take the newlib stdio lock, and a seek is one
// fioLseek rather than stdio's buffer bookkeeping. Elsewhere it is a FILE.
class PakFile
{
public:
    PakFile();
    ~PakFile();
    PakFile(const PakFile &) = delete;
    PakFile &operator=(const PakFile &) = delete;

    bool open(const std::string &path);
    void close();
    bool isOpen() const;

    // Reads exactly `length` bytes starting at absolute file offset `offset`.
    bool readAt(std::uint32_t offset, void *dst, std::uint32_t length);

private:
#if PLATFORM_PS2
    int fd_;
#endif
    std::FILE *file_;
};
