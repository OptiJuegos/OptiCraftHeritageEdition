#include "GuiWinGame.h"

#include "EntityClientPlayerMP.h"
#include "EntityPlayerSP.h"
#include "FontRenderer.h"
#include "Minecraft.h"
#include "RenderEngine.h"
#include "Session.h"
#include "Tessellator.h"
#include "TexturePackBase.h"
#include "TexturePackList.h"
#include "World.h"
#include "java/Random.h"
#include "platform/RenderAPI.h"

#include <algorithm>
#include <memory>
#include <sstream>

GuiWinGame::GuiWinGame()
    : updateCounter(0),
      totalScrollHeight(0),
      scrollSpeed(0.5f)
{
}

void GuiWinGame::updateScreen()
{
    ++updateCounter;
    const float_t endTick = static_cast<float_t>(totalScrollHeight + height + height + 24) / scrollSpeed;
    if (static_cast<float_t>(updateCounter) > endTick)
        respawnPlayer();
}

void GuiWinGame::keyTyped(char_t, int_t key)
{
    if (key == 1)
        respawnPlayer();
}

void GuiWinGame::respawnPlayer()
{
    if (mc == nullptr || mc->thePlayer == nullptr)
        return;

    if (mc->theWorld != nullptr && mc->theWorld->multiplayerWorld)
    {
        static_cast<EntityClientPlayerMP *>(mc->thePlayer)->respawnPlayer();
    }
    else
    {
        mc->displayGuiScreen(nullptr);
        mc->respawn(false, 0, true);
    }
}

bool GuiWinGame::doesGuiPauseGame()
{
    return true;
}

std::string GuiWinGame::replaceAll(std::string value, const std::string &from, const std::string &to)
{
    if (from.empty())
        return value;

    std::size_t position = 0;
    while ((position = value.find(from, position)) != std::string::npos)
    {
        value.replace(position, from.size(), to);
        position += to.size();
    }
    return value;
}

void GuiWinGame::appendWrappedLine(const std::string &line, int_t widthPixels)
{
    if (fontRenderer == nullptr || line.empty())
    {
        lines.push_back(line);
        return;
    }

    std::string remaining = line;
    while (!remaining.empty())
    {
        if (fontRenderer->getStringWidth(remaining) <= widthPixels)
        {
            lines.push_back(remaining);
            break;
        }

        std::size_t cut = remaining.size();
        while (cut > 0 && fontRenderer->getStringWidth(remaining.substr(0, cut)) > widthPixels)
            --cut;

        if (cut == 0)
            cut = 1;

        const std::size_t space = remaining.rfind(' ', cut);
        if (space != std::string::npos && space > 0)
            cut = space;

        lines.push_back(remaining.substr(0, cut));
        remaining.erase(0, cut);
        while (!remaining.empty() && remaining.front() == ' ')
            remaining.erase(remaining.begin());
    }
}

void GuiWinGame::loadCredits()
{
    if (!lines.empty() || mc == nullptr || mc->texturePackList == nullptr)
        return;

    TexturePackBase *pack = mc->texturePackList->getSelectedTexturePack();
    if (pack == nullptr)
        return;

    const std::string username = mc->session != nullptr ? mc->session->username : "Player";
    Random random(8124371LL);
    const std::string marker = "\xC2\xA7" "f" "\xC2\xA7" "k" "\xC2\xA7" "a" "\xC2\xA7" "b";
    const int_t textWidth = 274;

    auto appendResource = [&](const std::string &path, bool obfuscate)
    {
        std::unique_ptr<std::istream> stream(pack->getResourceAsStream(path));
        if (!stream || !*stream)
            return false;

        std::string line;
        while (std::getline(*stream, line))
        {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            line = replaceAll(line, "PLAYERNAME", username);
            line = replaceAll(line, "\t", "    ");

            if (obfuscate)
            {
                std::size_t position = 0;
                while ((position = line.find(marker, position)) != std::string::npos)
                {
                    const int_t count = random.nextInt(4) + 3;
                    const std::string replacement = std::string("\xC2\xA7" "f" "\xC2\xA7" "k") + std::string(static_cast<std::size_t>(count), 'X');
                    line.replace(position, marker.size(), replacement);
                    position += replacement.size();
                }
            }

            appendWrappedLine(line, textWidth);
            lines.push_back("");
        }
        return true;
    };

    appendResource("/title/win.txt", true);
    for (int_t i = 0; i < 8; ++i)
        lines.push_back("");
    appendResource("/title/credits.txt", false);

    totalScrollHeight = static_cast<int_t>(lines.size()) * 12;
}

void GuiWinGame::initGui()
{
    loadCredits();
}

void GuiWinGame::actionPerformed(GuiButton *)
{
}

void GuiWinGame::drawScrollingBackground(float_t partialTick)
{
    if (mc == nullptr || mc->renderEngine == nullptr)
        return;

    renderBindTexture(mc->renderEngine->getTexture("%blur%/gui/background.png"));
    Tessellator &tessellator = Tessellator::instance;
    tessellator.startDrawingQuads();

    const float_t offsetBottom = -static_cast<float_t>(updateCounter + partialTick) * 0.5f * scrollSpeed;
    const float_t offsetTop = static_cast<float_t>(height) - static_cast<float_t>(updateCounter + partialTick) * 0.5f * scrollSpeed;
    const float_t textureScale = 1.0f / 64.0f;
    float_t brightness = static_cast<float_t>(updateCounter + partialTick) * 0.02f;
    const float_t endTick = static_cast<float_t>(totalScrollHeight + height + height + 24) / scrollSpeed;
    const float_t endFade = (endTick - 20.0f - static_cast<float_t>(updateCounter + partialTick)) * 0.005f;
    brightness = std::min(brightness, endFade);
    brightness = std::max(0.0f, std::min(1.0f, brightness));
    brightness = brightness * brightness * 96.0f / 255.0f;

    tessellator.setColorOpaque_F(brightness, brightness, brightness);
    tessellator.addVertexWithUV(0.0f, static_cast<float_t>(height), zLevel, 0.0f, offsetBottom * textureScale);
    tessellator.addVertexWithUV(static_cast<float_t>(width), static_cast<float_t>(height), zLevel, static_cast<float_t>(width) * textureScale, offsetBottom * textureScale);
    tessellator.addVertexWithUV(static_cast<float_t>(width), 0.0f, zLevel, static_cast<float_t>(width) * textureScale, offsetTop * textureScale);
    tessellator.addVertexWithUV(0.0f, 0.0f, zLevel, 0.0f, offsetTop * textureScale);
    tessellator.draw();
}

void GuiWinGame::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    drawScrollingBackground(partialTick);
    loadCredits();

    const int_t textWidth = 274;
    const int_t left = width / 2 - textWidth / 2;
    const int_t logoY = height + 50;
    const float_t scrollOffset = -static_cast<float_t>(updateCounter + partialTick) * scrollSpeed;

    renderPushMatrix();
    renderTranslate(0.0f, scrollOffset, 0.0f);
    renderBindTexture(mc->renderEngine->getTexture("/title/mclogo.png"));
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    drawTexturedModalRect(left, logoY, 0, 0, 155, 44);
    drawTexturedModalRect(left + 155, logoY, 0, 45, 155, 44);

    int_t lineY = logoY + 200;
    for (std::size_t index = 0; index < lines.size(); ++index)
    {
        if (index == lines.size() - 1)
        {
            const float_t lastLinePosition = static_cast<float_t>(lineY) + scrollOffset - static_cast<float_t>(height / 2 - 6);
            if (lastLinePosition < 0.0f)
                renderTranslate(0.0f, -lastLinePosition, 0.0f);
        }

        if (static_cast<float_t>(lineY) + scrollOffset + 20.0f > 0.0f &&
            static_cast<float_t>(lineY) + scrollOffset < static_cast<float_t>(height))
        {
            const std::string &line = lines[index];
            if (line.rfind("[C]", 0) == 0)
            {
                const std::string centered = line.substr(3);
                fontRenderer->drawStringWithShadow(centered,
                    left + (textWidth - fontRenderer->getStringWidth(centered)) / 2,
                    lineY, 0xffffff);
            }
            else
            {
                const long_t seed = static_cast<long_t>(index) * 4238972211LL +
                    static_cast<long_t>(updateCounter / 4);
                fontRenderer->fontRandom.setSeed(seed);
                fontRenderer->renderString(line, left + 1, lineY + 1, 0xffffff, true);
                fontRenderer->fontRandom.setSeed(seed);
                fontRenderer->renderString(line, left, lineY, 0xffffff, false);
            }
        }

        lineY += 12;
    }
    renderPopMatrix();

    renderBindTexture(mc->renderEngine->getTexture("%blur%/misc/vignette.png"));
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::Zero, RenderBlendFactor::OneMinusSrcColor);
    Tessellator &tessellator = Tessellator::instance;
    tessellator.startDrawingQuads();
    tessellator.setColorRGBA_F(1.0f, 1.0f, 1.0f, 1.0f);
    tessellator.addVertexWithUV(0.0f, static_cast<float_t>(height), zLevel, 0.0f, 1.0f);
    tessellator.addVertexWithUV(static_cast<float_t>(width), static_cast<float_t>(height), zLevel, 1.0f, 1.0f);
    tessellator.addVertexWithUV(static_cast<float_t>(width), 0.0f, zLevel, 1.0f, 0.0f);
    tessellator.addVertexWithUV(0.0f, 0.0f, zLevel, 0.0f, 0.0f);
    tessellator.draw();
    renderDisable(RenderCapability::Blend);

    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
