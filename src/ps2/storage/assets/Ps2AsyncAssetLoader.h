#pragma once

#ifdef PS2_PLATFORM

// Compatibility implementation for source snapshots where RenderEngine.cpp
// references Ps2AsyncAssetLoader but the loader itself was not packaged.
//
// This fallback deliberately keeps the API expected by RenderEngine. It opens
// the resource stream when request() is issued and defers PNG decode/upload to
// RenderEngine's normal per-tick async pump. It is not a worker-thread loader,
// but it preserves correct texture loading and avoids leaving requested
// textures permanently on the missing-texture placeholder.

#include "net/minecraft/src/GameResources.h"

#include <istream>
#include <memory>
#include <string>
#include <unordered_map>

namespace Ps2AsyncAssetLoader
{
    enum class State
    {
        Missing,
        Loading,
        Ready,
        Failed
    };

    struct Entry
    {
        State state = State::Missing;
        std::unique_ptr<std::istream> stream;
    };

    inline std::unordered_map<std::string, Entry>& entries()
    {
        static std::unordered_map<std::string, Entry> value;
        return value;
    }

    inline State state(const std::string& path)
    {
        const auto& table = entries();
        const auto it = table.find(path);
        return it == table.end() ? State::Missing : it->second.state;
    }

    inline bool request(const std::string& path)
    {
        auto& table = entries();
        auto it = table.find(path);
        if (it != table.end())
        {
            if (it->second.state == State::Ready ||
                it->second.state == State::Loading)
                return true;
            table.erase(it);
        }

        Entry entry;
        entry.state = State::Loading;
        entry.stream = GameResources::open(path);
        entry.state = entry.stream ? State::Ready : State::Failed;

        const bool ok = entry.state == State::Ready;
        table.emplace(path, std::move(entry));
        return ok;
    }

    inline std::unique_ptr<std::istream> takeStream(const std::string& path)
    {
        auto& table = entries();
        auto it = table.find(path);
        if (it == table.end() || it->second.state != State::Ready)
            return nullptr;

        std::unique_ptr<std::istream> result = std::move(it->second.stream);
        table.erase(it);
        return result;
    }

    inline void release(const std::string& path)
    {
        entries().erase(path);
    }

    inline void cancelAll()
    {
        entries().clear();
    }

    inline void shutdown()
    {
        entries().clear();
    }
}

#endif // PS2_PLATFORM
