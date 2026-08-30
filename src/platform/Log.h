#pragma once

#include <cstdarg>
#include "platform/PlatformConfig.h"

#ifndef MC_LOG_LEVEL
#define MC_LOG_LEVEL 0
#endif

// Initial value of McLog::setSyncWrites(). Defaults on for the consoles, whose
// failure mode is a hang rather than a process exit: there, an unflushed tail
// is the difference between a log that names the last thing that ran and a log
// that stops 8KB early. Desktop keeps the buffered path.
#ifndef MC_LOG_SYNC_WRITES
#  if PLATFORM_PS2 || PLATFORM_WII
#    define MC_LOG_SYNC_WRITES 1
#  else
#    define MC_LOG_SYNC_WRITES 0
#  endif
#endif

// How many normal synced lines may accumulate before the log file is closed
// and reopened. A close commits the directory entry on the console filesystems,
// but doing it for every line is especially expensive on the PS2 USB/FAT path.
// Errors and warnings always commit immediately, regardless of this value.
#ifndef MC_LOG_COMMIT_EVERY
#  if PLATFORM_PS2
#    define MC_LOG_COMMIT_EVERY 16
#  else
#    define MC_LOG_COMMIT_EVERY 1
#  endif
#endif

// Unified diagnostic levels for every target:
//   0 = silent
//   1 = errors/warnings/info + boot milestones
//   2 = debug/frame diagnostics
//   3 = very verbose trace
namespace McLog
{
enum class Level
{
    Error,
    Warning,
    Info,
    Debug,
    Trace
};

// Opens <directory>/debug.log once for the current run. Messages emitted before
// the filesystem/install directory is known are buffered and replayed into the
// file, so early boot diagnostics are not lost. Safe to call more than once;
// the first successful open wins.
bool openSessionFile(const char* directory);

// Explicitly flushes the file sink. Error/warning messages are flushed
// immediately; normal traffic is buffered to avoid turning debug logging into
// a per-line filesystem sync on Wii/PS2.
void flush();

// Enables crash-oriented file commits. Normal lines are committed in batches
// selected by MC_LOG_COMMIT_EVERY; errors and warnings commit immediately.
// On PS2, per-frame trace breadcrumbs stay buffered so they do not turn the USB
// filesystem into part of the render loop. MC_LOG_SYNC_WRITES picks the initial
// value at build time.
void setSyncWrites(bool enabled);
bool syncWrites();

// Closes the file sink and clears the early buffer. Kept mainly for tests or a
// future soft-restart path; normal console startup does not need to call it.
void resetPlatformLog();

void write(Level level, const char* category, const char* fmt, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 3, 4)))
#endif
    ;
}

#define MC_LOG_ENABLED(level) (MC_LOG_LEVEL >= (level))
#define MC_LOG_ERROR(category, ...) do { if (MC_LOG_ENABLED(1)) McLog::write(McLog::Level::Error,   category, __VA_ARGS__); } while (0)
#define MC_LOG_WARN(category, ...)  do { if (MC_LOG_ENABLED(1)) McLog::write(McLog::Level::Warning, category, __VA_ARGS__); } while (0)
#define MC_LOG_INFO(category, ...)  do { if (MC_LOG_ENABLED(1)) McLog::write(McLog::Level::Info,    category, __VA_ARGS__); } while (0)
#define MC_LOG_DEBUG(category, ...) do { if (MC_LOG_ENABLED(2)) McLog::write(McLog::Level::Debug,   category, __VA_ARGS__); } while (0)
#define MC_LOG_TRACE(category, ...) do { if (MC_LOG_ENABLED(3)) McLog::write(McLog::Level::Trace,   category, __VA_ARGS__); } while (0)

#ifndef LOGI
#define LOGI(...) MC_LOG_INFO("game", __VA_ARGS__)
#endif
#ifndef LOGW
#define LOGW(...) MC_LOG_WARN("game", __VA_ARGS__)
#endif
#ifndef LOGE
#define LOGE(...) MC_LOG_ERROR("game", __VA_ARGS__)
#endif
