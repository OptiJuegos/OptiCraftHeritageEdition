#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "ProfilerResult.h"
#include "java/String.h"

// net.minecraft.src.Profiler
class Profiler
{
public:
    static void clearProfiling();
    static void startSection(const jstring &name);
    static void endSection();
    static std::vector<ProfilerResult> getProfilingData(const jstring &section);
    static void endStartSection(const jstring &name);

    static bool profilingEnabled;

private:
    static std::int64_t nanoTime();

    static std::vector<jstring> sectionList;
    static std::vector<std::int64_t> timestampList;
    static jstring profilingSection;
    static std::unordered_map<jstring, std::int64_t> profilingMap;
};
