#pragma once

#include "TexturePackBase.h"

#include <memory>
#include <string>

class BufferedImage;
class File;
class Minecraft;

// net.minecraft.src.TexturePackFolder
class TexturePackFolder : public TexturePackBase
{
public:
    explicit TexturePackFolder(File *directory);
    explicit TexturePackFolder(const std::string &directory);
    ~TexturePackFolder() override;

    void getTexturePackFolder(Minecraft *minecraft) override;
    void getResourceAsStream(Minecraft *minecraft) override;
    void bindThumbnailTexture(Minecraft *minecraft) override;
    void loadTexturePack() override;
    void closeTexturePackFile() override;
    std::istream *getResourceAsStream(const std::string &path) override;

private:
    std::string truncateDescription(const std::string &value) const;
    void loadMetadata();

    int_t thumbnailTexture;
    std::unique_ptr<BufferedImage> thumbnailImage;
    std::unique_ptr<File> packDirectory;
};
