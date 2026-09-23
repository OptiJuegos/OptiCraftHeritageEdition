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

    // PS2 only (no-op elsewhere): drop every language whose .lang file uses
    // a codepoint at or past 256 from the list getLanguageList() returns.
    // default.png only covers 0..255 (Latin-1); anything past that needs the
    // glyph_XX.png unicode-page switching FontRenderer::setUnicodeFlag()
    // enables, which is untested through the PS2-native texture pipeline.
    // Some Latin-alphabet languages (Polish, Turkish, Czech, ...) still use
    // codepoints past 255 for their diacritics and are excluded by this same
    // rule -- "stays inside Latin-1" is the actual boundary, not "uses Latin
    // letters", and this checks the real file content rather than guessing
    // per language. Lazy and memoized: called from GuiLanguage::initGui()
    // rather than the constructor, so the ~59-file scan only happens if the
    // player actually opens the language screen, not on every boot.
    void filterToLatinLanguagesOnPs2();

    // Single-file version of the same check, for validating one already-
    // chosen language code (e.g. persisted in options.txt) cheaply -- O(1)
    // file, not the ~59-file scan filterToLatinLanguagesOnPs2() does. Always
    // true off PS2. Used at boot, where scanning every language file just to
    // validate one persisted value would reintroduce the cost
    // filterToLatinLanguagesOnPs2() is deliberately lazy to avoid.
    static bool isLatin1SafeLanguageOnPs2(const std::string &language);

    void setLanguage(const std::string &language);
    const std::string &getCurrentLanguage() const;
    bool isUnicode() const;
    static bool isBidirectional(const std::string &language);

    std::string translateKey(const std::string &s);
    std::string translateUi(const std::string &english);
    std::string translateKeyFormat(const std::string &s, const std::vector<std::string> &args);
    std::string translateKeyFormat(const std::string &s, const std::string &arg);
    std::string translateKeyFormat(const std::string &s, const char *arg);
    std::string translateNamedKey(const std::string &s);

private:
    void loadLanguageList();
    bool loadLanguageFile(const std::string &path, bool ui = false);
    static std::string trim(const std::string &s);
    void updateUnicodeFlag();

    static StringTranslate *instance;
    std::map<std::string, std::string> translateTable;
    std::map<std::string, std::string> englishUiKeys;
    std::map<std::string, std::string> languageList;
    std::string currentLanguage;
    bool unicode;
    bool latinFiltered = false;
};
