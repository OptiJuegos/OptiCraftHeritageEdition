#include "ChatClickData.h"

#include "ChatLine.h"
#include "FontRenderer.h"

ChatClickData::ChatClickData(FontRenderer *font, ChatLine *line, int x, int y)
    : fontRenderer(font), chatLine(line), mouseX(x), mouseY(y)
{
    if (fontRenderer != nullptr && chatLine != nullptr)
        visiblePrefix = fontRenderer->trimStringToWidth(chatLine->message, mouseX);
    clickedText = findClickedToken();
}

const std::string &ChatClickData::getClickedText() const
{
    return clickedText;
}

std::string ChatClickData::getUrl() const
{
    if (clickedText.empty())
        return std::string();

    std::string value = clickedText;
    bool hasScheme = false;
    if (value.compare(0, 7, "http://") == 0)
    {
        value.erase(0, 7);
        hasScheme = true;
    }
    else if (value.compare(0, 8, "https://") == 0)
    {
        value.erase(0, 8);
        hasScheme = true;
    }

    std::size_t slash = value.find('/');
    std::string host = slash == std::string::npos ? value : value.substr(0, slash);
    if (host.size() < 4 || host.find('.') == std::string::npos)
        return std::string();
    for (char c : host)
    {
        bool valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                     (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.';
        if (!valid)
            return std::string();
    }
    std::size_t dot = host.find_last_of('.');
    if (dot == std::string::npos || dot + 2 >= host.size() || host.size() - dot - 1 > 3)
        return std::string();
    for (std::size_t i = dot + 1; i < host.size(); ++i)
    {
        char c = host[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
            return std::string();
    }
    if (slash != std::string::npos)
    {
        for (std::size_t i = slash; i < value.size(); ++i)
            if (value[i] == ' ' || value[i] == '\t' || value[i] == '\r' || value[i] == '\n')
                return std::string();
    }
    return hasScheme ? clickedText : "http://" + clickedText;
}

std::string ChatClickData::findClickedToken() const
{
    if (fontRenderer == nullptr || chatLine == nullptr)
        return std::string();
    std::size_t start = visiblePrefix.find_last_of(' ');
    start = start == std::string::npos ? 0 : start + 1;
    std::size_t end = chatLine->message.find(' ', start);
    if (end == std::string::npos)
        end = chatLine->message.size();
    if (start > chatLine->message.size() || end < start)
        return std::string();
    return FontRenderer::stripFormattingCodes(chatLine->message.substr(start, end - start));
}
