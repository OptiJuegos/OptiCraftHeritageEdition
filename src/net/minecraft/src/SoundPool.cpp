#include "SoundPool.h"

#include <algorithm>
#include <stdexcept>

#include "java/String.h"
#include "java/Unicode.h"

SoundPool::SoundPool() :
	rand(),
	nameToSoundPoolEntriesMapping(),
	allSoundPoolEntries(),
	numberOfSoundPoolEntries(0),
	motionX(true)
{
}

SoundPool::~SoundPool()
{
	for (SoundPoolEntry *entry : allSoundPoolEntries)
		delete entry;

	allSoundPoolEntries.clear();
	nameToSoundPoolEntriesMapping.clear();
	numberOfSoundPoolEntries = 0;
}

SoundPoolEntry *SoundPool::addSound(const jstring &s, const std::string &file)
{
    const jstring s1 = s;
    std::vector<char_t> name = String::toUtf16(s);

    // Java uses substring(0, s.indexOf(".")); a missing dot is an error, not a
    // request to keep the whole filename.
    auto dot = std::find(name.begin(), name.end(), static_cast<char_t>('.'));
    if (dot == name.end())
        throw std::out_of_range("SoundPool name has no extension separator");
    name.erase(dot, name.end());

    if (motionX)
    {
        // Character.isDigit(char) tests the Java UTF-16 unit, not an UTF-8 byte.
        while (!name.empty() && JavaUnicode::isDigit(name.back()))
            name.pop_back();
    }

    for (char_t &c : name)
        if (c == static_cast<char_t>('/')) c = static_cast<char_t>('.');
    const jstring key = String::fromUtf16(name);

    auto it = nameToSoundPoolEntriesMapping.find(key);
    if (it == nameToSoundPoolEntriesMapping.end())
    {
        nameToSoundPoolEntriesMapping[key] = std::vector<SoundPoolEntry*>();
        it = nameToSoundPoolEntriesMapping.find(key);
    }

    SoundPoolEntry *entry = new SoundPoolEntry(s1, file);
    it->second.push_back(entry);
    allSoundPoolEntries.push_back(entry);
    numberOfSoundPoolEntries++;
    return entry;
}

SoundPoolEntry *SoundPool::getRandomSound()
{
	if (allSoundPoolEntries.empty())
		return nullptr;

	int_t idx = rand.nextInt(allSoundPoolEntries.size());
	return allSoundPoolEntries[idx];
}

SoundPoolEntry *SoundPool::getRandomSoundFromSoundPool(const jstring &s)
{
	auto it = nameToSoundPoolEntriesMapping.find(s);
	if (it == nameToSoundPoolEntriesMapping.end())
		return nullptr;

	std::vector<SoundPoolEntry*> &entries = it->second;
	if (entries.empty())
		return nullptr;

	int_t idx = rand.nextInt(entries.size());
	return entries[idx];
}
