#include "TexturePackFolder.h"

#include "Minecraft.h"
#include "RenderEngine.h"
#include "java/BufferedImage.h"
#include "java/File.h"
#include "java/String.h"
#include "platform/RenderAPI.h"

#include <memory>
#include <string>

TexturePackFolder::TexturePackFolder(File *directory)
    : thumbnailTexture(-1),
      packDirectory(directory)
{
    if (packDirectory != nullptr)
        texturePackFileName = packDirectory->getName();
}

TexturePackFolder::TexturePackFolder(const std::string &directory)
    : TexturePackFolder(File::open(directory))
{
}

TexturePackFolder::~TexturePackFolder() = default;

std::string TexturePackFolder::truncateDescription(const std::string &value) const
{
    const jstring javaValue(value);
    return String::utf16Length(javaValue) > 34
        ? String::substringUtf16(javaValue, 0, 34)
        : javaValue;
}

void TexturePackFolder::loadMetadata()
{
    firstDescriptionLine.clear();
    secondDescriptionLine.clear();
    thumbnailImage.reset();

    std::unique_ptr<std::istream> description(getResourceAsStream("/pack.txt"));
    if (description != nullptr && *description)
    {
        std::string line;
        if (std::getline(*description, line))
            firstDescriptionLine = truncateDescription(line);
        if (std::getline(*description, line))
            secondDescriptionLine = truncateDescription(line);
    }

    try
    {
        std::unique_ptr<std::istream> imageStream(getResourceAsStream("/pack.png"));
        if (imageStream != nullptr && *imageStream)
            thumbnailImage.reset(new BufferedImage(BufferedImage::ImageIO_read(*imageStream)));
    }
    catch (...)
    {
        thumbnailImage.reset();
    }
}

void TexturePackFolder::getTexturePackFolder(Minecraft *minecraft)
{
    (void)minecraft;
    loadMetadata();
}

void TexturePackFolder::getResourceAsStream(Minecraft *minecraft)
{
    if (minecraft != nullptr && minecraft->renderEngine != nullptr && thumbnailTexture >= 0)
    {
        minecraft->renderEngine->deleteTexture(thumbnailTexture);
        thumbnailTexture = -1;
    }
    closeTexturePackFile();
}

void TexturePackFolder::bindThumbnailTexture(Minecraft *minecraft)
{
    if (minecraft == nullptr || minecraft->renderEngine == nullptr)
        return;

    if (thumbnailImage != nullptr && thumbnailTexture < 0)
        thumbnailTexture = minecraft->renderEngine->allocateAndSetupTexture(thumbnailImage.get());

    if (thumbnailImage != nullptr && thumbnailTexture >= 0)
        minecraft->renderEngine->bindTexture(thumbnailTexture);
    else
        renderBindTexture(minecraft->renderEngine->getTexture("/gui/unknown_pack.png"));
}

void TexturePackFolder::loadTexturePack()
{
    // Folder packs do not keep an archive handle open.
}

void TexturePackFolder::closeTexturePackFile()
{
    // Folder packs do not keep an archive handle open.
}

std::istream *TexturePackFolder::getResourceAsStream(const std::string &path)
{
    if (packDirectory != nullptr)
    {
        const std::string relativePath = !path.empty() && path[0] == '/' ? path.substr(1) : path;
        std::unique_ptr<File> resource(File::open(*packDirectory, relativePath));
        if (resource != nullptr && resource->exists() && resource->isFile())
        {
            std::istream *stream = resource->toStreamIn();
            if (stream != nullptr)
                return stream;
        }
    }

    return TexturePackBase::getResourceAsStream(path);
}
