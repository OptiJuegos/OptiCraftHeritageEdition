#pragma once

#include <string>

class ChatLine;
class FontRenderer;

// net.minecraft.src.ChatClickData
class ChatClickData
{
public:
    ChatClickData(FontRenderer *fontRenderer, ChatLine *chatLine, int mouseX, int mouseY);

    const std::string &getClickedText() const;
    std::string getUrl() const;

private:
    std::string findClickedToken() const;

    FontRenderer *fontRenderer;
    ChatLine *chatLine;
    int mouseX;
    int mouseY;
    std::string visiblePrefix;
    std::string clickedText;
};
