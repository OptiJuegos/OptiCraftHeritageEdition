#include "platform/Log.h"

#include <cstdio>
#include <cstdarg>
#include <cstring>

#if PLATFORM_WII
extern "C" void wiiPlatformLogWrite(const char* line);
#endif

namespace
{
const char* levelName(McLog::Level level)
{
    switch (level)
    {
        case McLog::Level::Error:   return "E";
        case McLog::Level::Warning: return "W";
        case McLog::Level::Info:    return "I";
        case McLog::Level::Debug:   return "D";
        case McLog::Level::Trace:   return "T";
    }
    return "?";
}

#if MC_LOG_LEVEL > 0
FILE* g_logFile = nullptr;
alignas(64) char g_fileBuffer[8192];
alignas(64) char g_earlyLog[16384];
std::size_t g_earlyLogSize = 0;
bool g_earlyLogTruncated = false;

void appendEarly(const char* line)
{
#if defined(PS2_REMOTE_DEBUG) && PLATFORM_PS2
    (void)line;
    return;
#else
    if (!line || g_logFile)
        return;

    const std::size_t len = std::strlen(line);
    const std::size_t room = sizeof(g_earlyLog) - g_earlyLogSize;
    if (len <= room)
    {
        std::memcpy(g_earlyLog + g_earlyLogSize, line, len);
        g_earlyLogSize += len;
    }
    else
    {
        g_earlyLogTruncated = true;
    }
#endif
}

bool g_syncWrites = MC_LOG_SYNC_WRITES != 0;
char g_logPath[512] = {};
int g_sinceCommit = 0;

// Reopen the file so the filesystem records what has been written.
//
// fflush() only pushes the FILE buffer down to write(), and on the consoles that
// is where it stops being enough: usbhdfsd and libfat update the directory
// entry -- including the file LENGTH -- when the file is closed, not on every
// write. A run that hangs never closes anything, so the host sees a debug.log of
// zero bytes no matter how much was flushed into it. That is the difference
// between "the log is empty" and "the log was never written", and they look
// identical from the PC.
//
// Closing and reopening in append mode commits the directory entry. It is not
// free, so it is amortised over MC_LOG_COMMIT_EVERY lines, and forced for the
// errors and warnings that matter most.
void commitFile()
{
    if (!g_logFile || !g_logPath[0])
        return;

    std::fflush(g_logFile);
    std::fclose(g_logFile);
    g_sinceCommit = 0;

    g_logFile = std::fopen(g_logPath, "a");
    if (g_logFile)
        std::setvbuf(g_logFile, g_fileBuffer, _IOFBF, sizeof(g_fileBuffer));
}

bool isHighFrequencyTrace(McLog::Level level, const char* category)
{
#if PLATFORM_PS2
    return level == McLog::Level::Trace && category && std::strcmp(category, "frame") == 0;
#else
    (void)level;
    (void)category;
    return false;
#endif
}

bool shouldWriteConsole(McLog::Level level, const char* category)
{
#if defined(PS2_REMOTE_DEBUG) && PLATFORM_PS2
    return !isHighFrequencyTrace(level, category);
#else
    (void)level;
    (void)category;
    return true;
#endif
}

void writeFile(const char* line, McLog::Level level, const char* category)
{
    if (!g_logFile || !line)
        return;

    std::fputs(line, g_logFile);

    const bool urgent = level == McLog::Level::Error || level == McLog::Level::Warning;
    if (urgent)
    {
        commitFile();
        return;
    }

    if (!g_syncWrites || isHighFrequencyTrace(level, category))
        return;

    if (++g_sinceCommit >= MC_LOG_COMMIT_EVERY)
        commitFile();
}
#endif
}

bool McLog::openSessionFile(const char* directory)
{
#if MC_LOG_LEVEL > 0
#if defined(PS2_REMOTE_DEBUG) && PLATFORM_PS2
    (void)directory;
    g_earlyLogSize = 0;
    g_earlyLogTruncated = false;
    g_syncWrites = false;
    std::fprintf(stdout,
                 "[MC][I][log] PS2 remote stdout-only level=%d build=%s %s\n",
                 (int)MC_LOG_LEVEL, __DATE__, __TIME__);
    std::fflush(stdout);
    return true;
#endif
    if (g_logFile)
        return true;
    if (!directory || !*directory)
        return false;

    char path[512];
    const std::size_t len = std::strlen(directory);
    const bool hasSeparator = len > 0 && (directory[len - 1] == '/' || directory[len - 1] == '\\' || directory[len - 1] == ':');
    std::snprintf(path, sizeof(path), hasSeparator ? "%sdebug.log" : "%s/debug.log", directory);

    FILE* file = std::fopen(path, "w");
    if (!file)
        return false;

    g_logFile = file;
    std::setvbuf(g_logFile, g_fileBuffer, _IOFBF, sizeof(g_fileBuffer));
    // Retained so commitFile() can reopen the same file in append mode.
    std::snprintf(g_logPath, sizeof(g_logPath), "%s", path);
    g_sinceCommit = 0;

    if (g_earlyLogSize > 0)
        std::fwrite(g_earlyLog, 1, g_earlyLogSize, g_logFile);
    if (g_earlyLogTruncated)
        std::fputs("[MC][W][log] early log buffer was truncated\n", g_logFile);

    g_earlyLogSize = 0;
    g_earlyLogTruncated = false;

    // Record the sink's own configuration first. Reading a truncated log is
    // guesswork without knowing which level was compiled in and whether the
    // tail can be trusted to be complete.
    //
    // build/commit are here to answer a question that costs a whole test cycle
    // otherwise: is the ELF on the card actually the one that was just built?
    // Nothing else in the log distinguishes them -- console boots are
    // deterministic enough (the splash line included, since Random() is seeded
    // from a clock that does not advance on this hardware) that two runs of
    // different binaries can produce byte-identical output up to the point
    // where they diverge. __DATE__/__TIME__ are this file's compile time, so
    // they move whenever Log.h or Log.cpp changes; commit prints
    // MC_LOG_COMMIT_EVERY, which is what decides how many trailing lines a hang
    // may swallow and therefore how far the last line can be trusted.
    char opened[640];
    std::snprintf(opened, sizeof(opened),
                  "[MC][I][log] file=%s level=%d sync=%d commit=%d build=%s %s\n",
                  path, (int)MC_LOG_LEVEL, g_syncWrites ? 1 : 0,
                  (int)MC_LOG_COMMIT_EVERY, __DATE__, __TIME__);
    std::fputs(opened, g_logFile);
    std::fflush(g_logFile);
    // Commit straight away: a debug.log that exists with this one line in it
    // proves the sink reached the device, which is the first thing to rule out
    // when a run produces no diagnostics at all.
    if (g_syncWrites)
        commitFile();

#if PLATFORM_WII
    wiiPlatformLogWrite(opened);
#else
    std::fputs(opened, stdout);
    std::fflush(stdout);
#endif
    return true;
#else
    (void)directory;
    return false;
#endif
}

void McLog::flush()
{
#if MC_LOG_LEVEL > 0
    if (g_logFile)
        commitFile();
#endif
}

void McLog::setSyncWrites(bool enabled)
{
#if MC_LOG_LEVEL > 0
    g_syncWrites = enabled;
    if (enabled && g_logFile)
        std::fflush(g_logFile);
#else
    (void)enabled;
#endif
}

bool McLog::syncWrites()
{
#if MC_LOG_LEVEL > 0
    return g_syncWrites;
#else
    return false;
#endif
}

void McLog::resetPlatformLog()
{
#if MC_LOG_LEVEL > 0
    if (g_logFile)
    {
        std::fflush(g_logFile);
        std::fclose(g_logFile);
        g_logFile = nullptr;
    }
    g_logPath[0] = '\0';
    g_sinceCommit = 0;
    g_earlyLogSize = 0;
    g_earlyLogTruncated = false;
#endif
}

void McLog::write(Level level, const char* category, const char* fmt, ...)
{
#if MC_LOG_LEVEL > 0
    char message[768];
    va_list ap;
    va_start(ap, fmt);
    ::vsnprintf(message, sizeof(message), fmt, ap);
    va_end(ap);

    const char* safeCategory = category ? category : "game";
    char line[896];
    ::snprintf(line, sizeof(line), "[MC][%s][%s] %s", levelName(level), safeCategory, message);

    if (g_logFile)
        writeFile(line, level, safeCategory);
    else
        appendEarly(line);

#if PLATFORM_WII
    wiiPlatformLogWrite(line);
#else
    if (shouldWriteConsole(level, safeCategory))
    {
        ::fputs(line, stdout);
        ::fflush(stdout);
    }
#endif
#else
    (void)level;
    (void)category;
    (void)fmt;
#endif
}
