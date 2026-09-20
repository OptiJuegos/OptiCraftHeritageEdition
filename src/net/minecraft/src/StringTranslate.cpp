#include "StringTranslate.h"

#include "GameResources.h"
#include "platform/Log.h"
#include <memory>

namespace
{
std::unique_ptr<std::istream> openLanguageResource(const std::string &path)
{
    return GameResources::open(path);
}

bool hasCodepointAtLeast256(const std::string &text)
{
    for (std::size_t i = 0; i < text.size();)
    {
        unsigned char c = (unsigned char)text[i];
        unsigned int codepoint = 0;
        std::size_t length = 1;
        if (c < 0x80)
        {
            codepoint = c;
        }
        else if ((c & 0xe0) == 0xc0 && i + 1 < text.size())
        {
            codepoint = ((c & 0x1f) << 6) | ((unsigned char)text[i + 1] & 0x3f);
            length = 2;
        }
        else if ((c & 0xf0) == 0xe0 && i + 2 < text.size())
        {
            codepoint = ((c & 0x0f) << 12) | (((unsigned char)text[i + 1] & 0x3f) << 6)
                      | ((unsigned char)text[i + 2] & 0x3f);
            length = 3;
        }
        else if ((c & 0xf8) == 0xf0 && i + 3 < text.size())
        {
            codepoint = ((c & 0x07) << 18) | (((unsigned char)text[i + 1] & 0x3f) << 12)
                      | (((unsigned char)text[i + 2] & 0x3f) << 6)
                      | ((unsigned char)text[i + 3] & 0x3f);
            length = 4;
        }
        else
        {
            codepoint = 0x100;
        }
        if (codepoint >= 0x100)
            return true;
        i += length;
    }
    return false;
}

#ifdef PS2_PLATFORM
// Whole-file version of hasCodepointAtLeast256: true only if every value in
// the file (key=value lines; keys are always plain-ASCII identifiers, so
// only values are worth checking) stays inside codepoints 0..255. A missing
// or unreadable file returns false -- exclude rather than guess.
bool languageFileIsLatin1Only(const std::string &path)
{
    std::unique_ptr<std::istream> owned = openLanguageResource(path);
    std::istream *input = owned.get();
    if (input == nullptr || !(*input))
        return false;

    std::string line;
    while (std::getline(*input, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty() || line[0] == '#')
            continue;
        if (hasCodepointAtLeast256(line))
            return false;
    }
    return true;
}
#endif
}

StringTranslate *StringTranslate::instance = nullptr;

StringTranslate::StringTranslate()
    : currentLanguage()
    , unicode(false)
{
    loadLanguageList();
    setLanguage("en_US");
}

StringTranslate *StringTranslate::getInstance()
{
    if (instance == nullptr)
        instance = new StringTranslate();
    return instance;
}

const std::map<std::string, std::string> &StringTranslate::getLanguageList() const
{
    return languageList;
}

void StringTranslate::filterToLatinLanguagesOnPs2()
{
#ifdef PS2_PLATFORM
    if (latinFiltered)
        return;
    latinFiltered = true;

    for (auto it = languageList.begin(); it != languageList.end(); )
    {
        // en_US is always ASCII by construction (it's the mandatory fallback
        // loaded first in setLanguage()); skip the redundant file read.
        if (it->first == "en_US" || languageFileIsLatin1Only("/lang/" + it->first + ".lang"))
            ++it;
        else
            it = languageList.erase(it);
    }

    if (languageList.find("en_US") == languageList.end())
        languageList["en_US"] = "English (US)";
#endif
}

bool StringTranslate::isLatin1SafeLanguageOnPs2(const std::string &language)
{
#ifdef PS2_PLATFORM
    if (language == "en_US")
        return true;
    return languageFileIsLatin1Only("/lang/" + language + ".lang");
#else
    (void)language;
    return true;
#endif
}

void StringTranslate::loadLanguageList()
{
    languageList.clear();
    std::unique_ptr<std::istream> owned = openLanguageResource("/lang/languages.txt");
    std::istream *input = owned.get();
    if (input != nullptr)
    {
        std::string line;
        while (std::getline(*input, line))
        {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            std::size_t equals = line.find('=');
            if (equals == std::string::npos)
                continue;
            std::string key = trim(line.substr(0, equals));
            std::string value = trim(line.substr(equals + 1));
            if (!key.empty() && !value.empty())
                languageList[key] = value;
        }
    }
    if (languageList.empty())
        languageList["en_US"] = "English (US)";
}

void StringTranslate::setLanguage(const std::string &language)
{
    if (language == currentLanguage && !translateTable.empty())
        return;

    translateTable.clear();
    englishUiKeys.clear();
    loadLanguageFile("/lang/en_US.lang");
    for (const auto &entry : translateTable)
    {
        const auto dot = entry.first.find('.');
        const std::string group = entry.first.substr(0, dot);
        if (group == "gui" || group == "menu" || group == "options" || group == "controls" ||
            group == "selectWorld" || group == "createWorld" || group == "gameMode" ||
            group == "multiplayer" || group == "disconnect" || group == "connect" ||
            group == "container" || group == "key" || group == "deathScreen")
            englishUiKeys.emplace(entry.second, entry.first);
    }
    const char *aliases[][2] = {
        {"Play Game", "menu.singleplayer"}, {"Help & Options", "menu.options"},
        {"Start Game", "selectWorld.title"}, {"Create New World", "selectWorld.create"},
        {"Resume Game", "menu.returnToGame"}, {"Save & Quit", "menu.returnToMenu"},
        {"Video", "options.video"}, {"Controls", "options.controls"},
        {"Language", "options.language"}, {"Fancy Graphics", "options.graphics"},
        {"Smooth Lighting", "options.ao"}, {"View Bobbing", "options.viewBobbing"},
        {"Render Clouds", "options.renderClouds"}, {"Render Distance", "options.renderDistance"},
        {"FOV", "options.fov"}, {"Sensitivity", "options.sensitivity"},
        {"Invert Mouse", "options.invertMouse"}, {"Attack", "key.attack"},
        {"Use", "key.use"}, {"Jump", "key.jump"}, {"Sneak", "key.sneak"},
        {"Drop", "key.drop"}, {"Inventory", "key.inventory"},
        {"ON", "options.on"}, {"OFF", "options.off"}
    };
    for (const auto &alias : aliases)
        if (translateTable.find(alias[1]) != translateTable.end())
            englishUiKeys[alias[0]] = alias[1];
    currentLanguage = "en_US";
    if (language != "en_US" && loadLanguageFile("/lang/" + language + ".lang"))
        currentLanguage = language;
    // Supplemental files use English labels as keys and do not alter the
    // existing resource files. Missing files/entries keep the English text.
    loadLanguageFile("/lang/ui/" + currentLanguage + ".lang", true);
    updateUnicodeFlag();
}

const std::string &StringTranslate::getCurrentLanguage() const
{
    return currentLanguage;
}

bool StringTranslate::isUnicode() const
{
    return unicode;
}

bool StringTranslate::isBidirectional(const std::string &language)
{
    return language == "ar_SA" || language == "he_IL";
}

std::string StringTranslate::translateUi(const std::string &english)
{
    auto extra = translateTable.find("ui." + english);
    if (extra != translateTable.end() && !extra->second.empty())
        return extra->second;
    auto key = englishUiKeys.find(english);
    if (key != englishUiKeys.end())
        return translateKey(key->second);
    // Source labels may have an ellipsis or a colon that the vanilla key lacks.
    const std::string suffix = english.size() >= 3 && english.compare(english.size() - 3, 3, "...") == 0
        ? "..." : (english.size() >= 2 && english.compare(english.size() - 2, 2, ": ") == 0 ? ": " : "");
    if (!suffix.empty())
        return translateUi(english.substr(0, english.size() - suffix.size())) + suffix;
    return english;
}

std::string StringTranslate::translateKey(const std::string &s)
{
    auto it = translateTable.find(s);
    return it != translateTable.end() ? it->second : s;
}

std::string StringTranslate::translateKeyFormat(const std::string &s, const std::vector<std::string> &args)
{
    std::string result = translateKey(s);
    std::size_t sequentialArg = 0;
    std::size_t searchFrom = 0;
    while (sequentialArg < args.size())
    {
        std::size_t pos = result.find("%s", searchFrom);
        if (pos == std::string::npos)
            break;
        result.replace(pos, 2, args[sequentialArg]);
        searchFrom = pos + args[sequentialArg].length();
        ++sequentialArg;
    }

    for (std::size_t i = 0; i < args.size(); ++i)
    {
        std::string token = "%" + std::to_string(i + 1) + "$s";
        std::size_t pos = 0;
        while ((pos = result.find(token, pos)) != std::string::npos)
        {
            result.replace(pos, token.length(), args[i]);
            pos += args[i].length();
        }
    }
    return result;
}

std::string StringTranslate::translateKeyFormat(const std::string &s, const std::string &arg)
{
    return translateKeyFormat(s, std::vector<std::string>{arg});
}

std::string StringTranslate::translateKeyFormat(const std::string &s, const char *arg)
{
    return translateKeyFormat(s, std::vector<std::string>{arg != nullptr ? arg : ""});
}

std::string StringTranslate::translateNamedKey(const std::string &s)
{
    auto it = translateTable.find(s + ".name");
    return it != translateTable.end() ? it->second : "";
}

bool StringTranslate::loadLanguageFile(const std::string &path, bool ui)
{
    std::unique_ptr<std::istream> owned = openLanguageResource(path);
    std::istream *input = owned.get();
    if (input == nullptr || !(*input))
    {
#ifdef PS2_PLATFORM
        MC_LOG_DEBUG("ps2", "language missing: %s\n", path.c_str());
#endif
        return false;
    }

    std::string line;
    while (std::getline(*input, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        std::size_t equals = line.find('=');
        if (equals == std::string::npos)
            continue;
        const std::string key = trim(line.substr(0, equals));
        const std::string value = trim(line.substr(equals + 1));
        // Empty overrides must not erase the English fallback.
        if (!key.empty() && !value.empty())
            translateTable[(ui ? "ui." : "") + key] = value;
    }
    return true;
}

std::string StringTranslate::trim(const std::string &s)
{
    std::size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    std::size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

void StringTranslate::updateUnicodeFlag()
{
    unicode = false;
    for (const auto &entry : translateTable)
    {
        if (hasCodepointAtLeast256(entry.second))
        {
            unicode = true;
            break;
        }
    }
}
