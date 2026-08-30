#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>
#include <stdexcept>
#include <utility>
#include <vector>

// Minimal value-type HashSet that preserves the bucket and iteration behavior
// of the Java HashMap implementation used by the original game.
template <typename T, typename HashCode, typename Equal>
class JavaHashSet
{
public:
    JavaHashSet() : buckets_(16, -1), threshold_(12), size_(0), freeHead_(-1)
    {
    }

    bool add(const T &value)
    {
        const std::uint32_t hash = spreadHash(static_cast<std::uint32_t>(HashCode{}(value)));
        const std::size_t bucket = hash & (buckets_.size() - 1);

        for (int index = buckets_[bucket]; index >= 0; index = entries_[static_cast<std::size_t>(index)].next)
        {
            const Entry &entry = entries_[static_cast<std::size_t>(index)];
            if (entry.hash == hash && Equal{}(value, entry.value))
                return false;
        }

        const int oldHead = buckets_[bucket];
        int newIndex;
        if (freeHead_ >= 0)
        {
            newIndex = freeHead_;
            freeHead_ = entries_[static_cast<std::size_t>(newIndex)].next;
            // T only needs Java-style value semantics: copy construction, hash
            // and equality. Some game keys intentionally contain const fields
            // (ChunkCoordIntPair), so assigning a replacement Entry is not a
            // valid generic operation. Reconstruct the removed slot in place;
            // its address/index remains stable for the bucket chains.
            Entry replacement{hash, value, oldHead};
            Entry *slot = &entries_[static_cast<std::size_t>(newIndex)];
            slot->~Entry();
            ::new (static_cast<void *>(slot)) Entry(std::move(replacement));
        }
        else
        {
            if (entries_.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
                throw std::length_error("JavaHashSet entry index overflow");
            newIndex = static_cast<int>(entries_.size());
            entries_.push_back({hash, value, oldHead});
        }
        buckets_[bucket] = newIndex;

        const std::size_t oldSize = size_++;
        if (oldSize >= threshold_)
        {
            if (buckets_.size() < MAXIMUM_CAPACITY)
            {
                const std::size_t newCapacity = std::min(buckets_.size() * 2, MAXIMUM_CAPACITY);
                resize(newCapacity);
            }
            else
            {
                threshold_ = std::numeric_limits<std::size_t>::max();
            }
        }

        return true;
    }

    bool contains(const T &value) const
    {
        const std::uint32_t hash = spreadHash(static_cast<std::uint32_t>(HashCode{}(value)));
        const std::size_t bucket = hash & (buckets_.size() - 1);
        for (int index = buckets_[bucket]; index >= 0; index = entries_[static_cast<std::size_t>(index)].next)
        {
            const Entry &entry = entries_[static_cast<std::size_t>(index)];
            if (entry.hash == hash && Equal{}(value, entry.value))
                return true;
        }
        return false;
    }

    bool remove(const T &value)
    {
        const std::uint32_t hash = spreadHash(static_cast<std::uint32_t>(HashCode{}(value)));
        const std::size_t bucket = hash & (buckets_.size() - 1);
        int previous = -1;
        int index = buckets_[bucket];
        while (index >= 0)
        {
            Entry &entry = entries_[static_cast<std::size_t>(index)];
            if (entry.hash == hash && Equal{}(value, entry.value))
            {
                if (previous < 0)
                    buckets_[bucket] = entry.next;
                else
                    entries_[static_cast<std::size_t>(previous)].next = entry.next;
                entry.next = freeHead_;
                freeHead_ = index;
                --size_;
                return true;
            }
            previous = index;
            index = entry.next;
        }
        return false;
    }

    void clear()
    {
        std::fill(buckets_.begin(), buckets_.end(), -1);
        entries_.clear();
        freeHead_ = -1;
        size_ = 0;
    }

    std::size_t size() const
    {
        return size_;
    }

    std::vector<T> valuesInIterationOrder() const
    {
        std::vector<T> values;
        values.reserve(size_);
        for (std::size_t bucket = 0; bucket < buckets_.size(); ++bucket)
        {
            for (int index = buckets_[bucket]; index >= 0; index = entries_[static_cast<std::size_t>(index)].next)
                values.push_back(entries_[static_cast<std::size_t>(index)].value);
        }
        return values;
    }

private:
    static constexpr std::size_t MAXIMUM_CAPACITY = std::size_t{1} << 30;

    struct Entry
    {
        std::uint32_t hash;
        T value;
        int next;
    };

    static std::uint32_t spreadHash(std::uint32_t hash)
    {
        hash ^= (hash >> 20) ^ (hash >> 12);
        return hash ^ (hash >> 7) ^ (hash >> 4);
    }

    void resize(std::size_t newCapacity)
    {
        std::vector<int> newBuckets(newCapacity, -1);
        for (std::size_t bucket = 0; bucket < buckets_.size(); ++bucket)
        {
            int index = buckets_[bucket];
            while (index >= 0)
            {
                Entry &entry = entries_[static_cast<std::size_t>(index)];
                const int next = entry.next;
                const std::size_t newBucket = entry.hash & (newCapacity - 1);
                entry.next = newBuckets[newBucket];
                newBuckets[newBucket] = index;
                index = next;
            }
        }
        buckets_.swap(newBuckets);
        threshold_ = newCapacity * 3 / 4;
    }

    std::vector<int> buckets_;
    std::vector<Entry> entries_;
    std::size_t threshold_;
    std::size_t size_;
    int freeHead_;
};
