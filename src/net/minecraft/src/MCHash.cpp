#include "MCHash.h"

#include "java/Arithmetic.h"

#include <new>
MCHash::MCHash(bool reuseEntries_)
    : slots(16, nullptr), count(0), threshold(12), growFactor(0.75f), versionStamp(0),
      reuseEntries(reuseEntries_)
{
}

MCHash::~MCHash()
{
    clearMap();
    for (MCHashEntry *entry : freeEntries)
        delete entry;
}

int_t MCHash::computeHash(int_t i)
{
    i ^= (int_t)((uint_t)i >> 20) ^ (int_t)((uint_t)i >> 12);
    return i ^ (int_t)((uint_t)i >> 7) ^ (int_t)((uint_t)i >> 4);
}

int_t MCHash::getSlotIndex(int_t i, int_t j)
{
    return i & (j - 1);
}

int_t MCHash::getHash(int_t i)
{
    return computeHash(i);
}

void *MCHash::lookup(int_t i)
{
    int_t j = computeHash(i);
    for (MCHashEntry *e = slots[getSlotIndex(j, (int_t)slots.size())]; e; e = e->nextEntry)
        if (e->hashEntry == i)
            return e->valueEntry;
    return nullptr;
}

void MCHash::addKey(int_t i, void *obj)
{
    int_t j = computeHash(i);
    int_t k = getSlotIndex(j, (int_t)slots.size());
    for (MCHashEntry *e = slots[k]; e; e = e->nextEntry)
        if (e->hashEntry == i)
        {
            e->valueEntry = obj;
            return;
        }
    versionStamp++;
    insert(j, i, obj, k);
}

void *MCHash::removeObject(int_t i)
{
    MCHashEntry *e = removeEntry(i);
    if (e == nullptr)
        return nullptr;
    void *value = e->valueEntry;
    if (reuseEntries)
        freeEntries.push_back(e);
    else
        delete e;
    return value;
}

MCHashEntry *MCHash::removeEntry(int_t i)
{
    int_t j = computeHash(i);
    int_t k = getSlotIndex(j, (int_t)slots.size());
    MCHashEntry *prev = slots[k];
    MCHashEntry *cur;
    MCHashEntry *next;
    for (cur = prev; cur; cur = next)
    {
        next = cur->nextEntry;
        if (cur->hashEntry == i)
        {
            versionStamp++;
            count--;
            if (prev == cur)
                slots[k] = next;
            else
                prev->nextEntry = next;
            return cur;
        }
        prev = cur;
    }
    return nullptr;
}

void MCHash::clearMap()
{
    versionStamp++;
    for (auto &slot : slots)
    {
        MCHashEntry *entry = slot;
        while (entry != nullptr)
        {
            MCHashEntry *next = entry->nextEntry;
            if (reuseEntries)
                freeEntries.push_back(entry);
            else
                delete entry;
            entry = next;
        }
        slot = nullptr;
    }
    count = 0;
}

void MCHash::reserveEntries(int_t expectedEntries)
{
    if (reuseEntries && expectedEntries > 0)
        freeEntries.reserve(static_cast<std::size_t>(expectedEntries));
    if (expectedEntries <= threshold)
        return;

    int_t targetSize = static_cast<int_t>(slots.size());
    while (targetSize < 0x40000000 &&
           JavaArithmetic::floatToInt(static_cast<float>(targetSize) * growFactor) < expectedEntries)
    {
        targetSize <<= 1;
    }

    if (targetSize > static_cast<int_t>(slots.size()))
        grow(targetSize);
}

void MCHash::grow(int_t i)
{
    int_t j = (int_t)slots.size();
    if (j == 0x40000000)
    {
        threshold = 0x7fffffff;
        return;
    }
    std::vector<MCHashEntry *> newSlots(i, nullptr);
    copyTo(newSlots);
    slots = std::move(newSlots);
    threshold = JavaArithmetic::floatToInt(static_cast<float>(i) * growFactor);
}

void MCHash::copyTo(std::vector<MCHashEntry *> &dest)
{
    int_t i = (int_t)dest.size();
    for (int_t j = 0; j < (int_t)slots.size(); j++)
    {
        MCHashEntry *e = slots[j];
        if (!e) continue;
        slots[j] = nullptr;
        do
        {
            MCHashEntry *next = e->nextEntry;
            int_t k = getSlotIndex(e->slotHash, i);
            e->nextEntry = dest[k];
            dest[k] = e;
            e = next;
        } while (e);
    }
}

void MCHash::insert(int_t slotHash, int_t key, void *obj, int_t k)
{
    MCHashEntry *entry = nullptr;
    if (reuseEntries && !freeEntries.empty())
    {
        entry = freeEntries.back();
        freeEntries.pop_back();
        entry->~MCHashEntry();
        new (entry) MCHashEntry(slotHash, key, obj, slots[k]);
    }
    else
    {
        entry = new MCHashEntry(slotHash, key, obj, slots[k]);
    }
    slots[k] = entry;
    if (count++ >= threshold)
        grow(JavaArithmetic::intMul(2, static_cast<int_t>(slots.size())));
}
