#pragma once

#include "IMod.h"
#include "OchPackReader.h"
#include <string>

class DynamicMod : public IMod
{
public:
    DynamicMod(const OchPackInfo &info)
        : modId(info.id)
        , modName(info.name)
        , modVersion(info.version)
        , modAuthor(info.author)
        , modDescription(info.description)
        , enabled(true)
    {
        packPath = info.filePath;
    }

    std::string getId() const override { return modId; }
    std::string getName() const override { return modName; }
    std::string getVersion() const override { return modVersion; }
    std::string getDescription() const override { return modDescription; }
    std::string getAuthor() const override { return modAuthor; }

    bool isEnabled() const override { return enabled; }
    void setEnabled(bool state) override { enabled = state; }

private:
    std::string modId;
    std::string modName;
    std::string modVersion;
    std::string modAuthor;
    std::string modDescription;
    bool enabled;
};
