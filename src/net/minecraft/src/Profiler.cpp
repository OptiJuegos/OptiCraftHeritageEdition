#include "Profiler.h"

#include <algorithm>
#include <chrono>

bool Profiler::profilingEnabled = false;
std::vector<jstring> Profiler::sectionList;
std::vector<std::int64_t> Profiler::timestampList;
jstring Profiler::profilingSection;
std::unordered_map<jstring, std::int64_t> Profiler::profilingMap;

std::int64_t Profiler::nanoTime()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

void Profiler::clearProfiling()
{
    profilingMap.clear();
}

void Profiler::startSection(const jstring &name)
{
    if (!profilingEnabled)
        return;
    if (!profilingSection.empty())
        profilingSection += ".";
    profilingSection += name;
    sectionList.push_back(profilingSection);
    timestampList.push_back(nanoTime());
}

void Profiler::endSection()
{
    if (!profilingEnabled || timestampList.empty() || sectionList.empty())
        return;
    std::int64_t now = nanoTime();
    std::int64_t start = timestampList.back();
    timestampList.pop_back();
    sectionList.pop_back();
    std::int64_t elapsed = now - start;
    profilingMap[profilingSection] += elapsed;
    profilingSection = sectionList.empty() ? jstring() : sectionList.back();
}

std::vector<ProfilerResult> Profiler::getProfilingData(const jstring &section)
{
    std::vector<ProfilerResult> result;
    if (!profilingEnabled)
        return result;

    std::int64_t rootTime = profilingMap.count("root") != 0 ? profilingMap["root"] : 0;
    std::int64_t sectionTime = profilingMap.count(section) != 0 ? profilingMap[section] : -1;
    jstring prefix = section;
    if (!prefix.empty())
        prefix += ".";

    std::int64_t childrenTime = 0;
    for (const auto &entry : profilingMap)
    {
        const jstring &name = entry.first;
        if (name.size() <= prefix.size() || name.compare(0, prefix.size(), prefix) != 0)
            continue;
        if (name.find('.', prefix.size() + 1) == jstring::npos)
            childrenTime += entry.second;
    }

    float measuredChildren = (float)childrenTime;
    if (childrenTime < sectionTime)
        childrenTime = sectionTime;
    if (rootTime < childrenTime)
        rootTime = childrenTime;
    if (childrenTime <= 0)
        childrenTime = 1;
    if (rootTime <= 0)
        rootTime = 1;

    for (const auto &entry : profilingMap)
    {
        const jstring &name = entry.first;
        if (name.size() <= prefix.size() || name.compare(0, prefix.size(), prefix) != 0)
            continue;
        if (name.find('.', prefix.size() + 1) != jstring::npos)
            continue;
        double sectionPercent = (double)entry.second * 100.0 / (double)childrenTime;
        double rootPercent = (double)entry.second * 100.0 / (double)rootTime;
        result.emplace_back(name.substr(prefix.size()), sectionPercent, rootPercent);
    }

    for (auto &entry : profilingMap)
        entry.second = entry.second * 999 / 1000;

    if ((float)childrenTime > measuredChildren)
    {
        double unspecified = (double)((float)childrenTime - measuredChildren) * 100.0 / (double)childrenTime;
        double unspecifiedRoot = (double)((float)childrenTime - measuredChildren) * 100.0 / (double)rootTime;
        result.emplace_back("unspecified", unspecified, unspecifiedRoot);
    }

    std::stable_sort(result.begin(), result.end(), [](const ProfilerResult &a, const ProfilerResult &b)
    {
        return a.compareProfilerResult(b) < 0;
    });
    result.insert(result.begin(), ProfilerResult(prefix, 100.0, (double)childrenTime * 100.0 / (double)rootTime));
    return result;
}

void Profiler::endStartSection(const jstring &name)
{
    endSection();
    startSection(name);
}
