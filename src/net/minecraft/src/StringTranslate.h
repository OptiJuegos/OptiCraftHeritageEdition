#pragma once

#include <map>
#include <string>
#include <vector>

// net.minecraft.src.StringTranslate
class StringTranslate
{
private:
    StringTranslate();

public:
    static StringTranslate *getInstance();

    const std::map<std::string, std::string> &getLanguageList() const;
    void setLanguage(const std::string &language);
    const std::string &getCurrentLanguage() const;
    bool isUnicode() const;
    static bool isBidirectional(const std::string &language);

    std::string translateKey(const std::string &s);
    std::string translateKeyFormat(const std::string &s, const std::vector<std::string> &args);
    std::string translateKeyFormat(const std::string &s, const std::string &arg);
    std::string translateKeyFormat(const std::string &s, const char *arg);
    std::string translateNamedKey(const std::string &s);

private:
    void loadLanguageList();
    bool loadLanguageFile(const std::string &path);
    static std::string trim(const std::string &s);
    void updateUnicodeFlag();

    static StringTranslate *instance;
    std::map<std::string, std::string> translateTable;
    std::map<std::string, std::string> languageList;
    std::string currentLanguage;
    bool unicode;
};
