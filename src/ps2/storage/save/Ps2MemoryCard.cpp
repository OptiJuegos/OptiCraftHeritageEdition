#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/storage/save/Ps2MemoryCard.h"
#include "ps2/system/Ps2Iop.h"

#include <cstdio>
#include <cstring>

#include <delaythread.h>
#include <fcntl.h>
#include <libmc.h>

// mcOpen()'s mode argument is read by the IOP mc driver's own protocol, NOT by
// EE-side newlib's fcntl.h values -- and the two disagree. Newlib defines
// O_RDONLY=0, O_WRONLY=1, O_RDWR=2 (its usual convention); the IOP side
// (ps2sdk's own iop/include/sys/fcntl.h) uses O_RDONLY=0x1, O_WRONLY=0x2,
// O_RDWR=0x3 -- read and write as separate bits. O_CREAT (0x200) and O_TRUNC
// (0x400) happen to match between the two, which is why creating a file here
// has always worked, but a read-only open built from newlib's O_RDONLY passed
// mode=0 to the driver: "no access requested". mcOpen() does not reject that;
// mcRead() correctly does. That is why a world, once written, could never be
// read back. Use the IOP-side values explicitly instead of borrowing newlib's.
#define MC_MODE_RDONLY 0x0001
#define MC_MODE_WRONLY 0x0002
#define MC_MODE_RDWR   0x0003
#define MC_MODE_CREAT  0x0200
#define MC_MODE_TRUNC  0x0400

#ifndef MC_O_CREAT_FILE
#define MC_O_CREAT_FILE (MC_MODE_CREAT | MC_MODE_RDWR | MC_MODE_TRUNC)
#endif

#ifndef FIO_O_RDONLY
#define FIO_O_RDONLY MC_MODE_RDONLY
#endif

namespace
{

int s_type = 0;
int s_freeClusters = 0;
int s_format = 0;
int s_result = 0;

bool waitResult(int* outResult)
{
    int command = 0;
    int result = 0;
    mcSync(0, &command, &result);
    if (outResult)
        *outResult = result;
    return true;
}

} // namespace

namespace Ps2MemoryCard
{

bool probeWritable()
{
    const char* probeDirectory = "/__MCPEPROBE";
    const char* probeFile = "/__MCPEPROBE/p.bin";

    static unsigned char writeBuffer[512] __attribute__((aligned(64)));
    static unsigned char readBuffer[512] __attribute__((aligned(64)));
    const unsigned probeSize = sizeof(writeBuffer);

    for (unsigned i = 0; i < probeSize; ++i)
        writeBuffer[i] = static_cast<unsigned char>(i & 0xFF);

    int result = 0;
    mcMkDir(0, 0, probeDirectory);
    waitResult(&result);
    mcDelete(0, 0, probeFile);
    waitResult(&result);

    const int openRequest = mcOpen(0, 0, probeFile, MC_O_CREAT_FILE);
    const bool openOk = openRequest >= 0 && waitResult(&result) && result >= 0;
    const int fd = openOk ? result : -1;
    if (!openOk)
        return false;

    const int writeRequest = mcWrite(fd, writeBuffer, probeSize);
    const bool wrote = writeRequest >= 0 && waitResult(&result) && result == static_cast<int>(probeSize);
    mcFlush(fd);
    waitResult(&result);
    mcClose(fd);
    waitResult(&result);

    bool readOk = false;
    if (wrote)
    {
        const int readOpenRequest = mcOpen(0, 0, probeFile, FIO_O_RDONLY);
        const bool readOpenOk = readOpenRequest >= 0 && waitResult(&result) && result >= 0;
        const int readFd = readOpenOk ? result : -1;
        if (readOpenOk)
        {
            std::memset(readBuffer, 0, probeSize);
            const int readRequest = mcRead(readFd, readBuffer, probeSize);
            readOk = readRequest >= 0 && waitResult(&result)
                && result == static_cast<int>(probeSize)
                && std::memcmp(readBuffer, writeBuffer, probeSize) == 0;
            mcClose(readFd);
            waitResult(&result);
        }
    }

    mcDelete(0, 0, probeFile);
    waitResult(&result);
    mcDelete(0, 0, probeDirectory);
    waitResult(&result);

    if (readOk)
        MC_LOG_INFO("save", "[PS2] mc probe: OK\n");
    return readOk;
}

bool initialize()
{
    Ps2Iop::initFileServices();
    Ps2Iop::ensureRomModule(Ps2Iop::RomModule::MemoryCardManager);
    Ps2Iop::ensureRomModule(Ps2Iop::RomModule::MemoryCardServer);
    DelayThread(500 * 1000);

    if (mcInit(MC_TYPE_MC) >= 0)
    {
        s_type = 0;
        s_freeClusters = 0;
        s_format = 0;
        s_result = 0;
        if (mcGetInfo(0, 0, &s_type, &s_freeClusters, &s_format) >= 0 && waitResult(&s_result))
        {
            MC_LOG_INFO("save", "[PS2] mc0: type=%d free=%d format=%d\n",
                        s_type, s_freeClusters, s_format);
        }
    }

    if (s_type == MC_TYPE_NONE || s_type == 0)
    {
        MC_LOG_INFO("save", "[PS2] mc0 not detected\n");
        return false;
    }
    if (s_format != MC_FORMATTED && s_result == -2)
    {
        MC_LOG_INFO("save", "[PS2] mc0 not formatted\n");
        return false;
    }
    return available();
}

bool available()
{
    s_type = 0;
    s_format = 0;
    if (mcGetInfo(0, 0, &s_type, &s_freeClusters, &s_format) < 0)
        return false;
    waitResult(&s_result);
    return s_type != 0 && s_type != MC_TYPE_NONE && s_format == MC_FORMATTED;
}

bool isFormatted()
{
    return s_format == MC_FORMATTED;
}

} // namespace Ps2MemoryCard

#endif // PS2_PLATFORM
