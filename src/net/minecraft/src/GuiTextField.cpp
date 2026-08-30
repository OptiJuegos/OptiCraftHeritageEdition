#include "GuiTextField.h"

#include <algorithm>
#include <cstdlib>

#include "ChatAllowedCharacters.h"
#include "FontRenderer.h"
#include "GuiScreen.h"
#include "java/Arithmetic.h"
#include "java/String.h"
#include "pc/lwjgl/Keyboard.h"
#include "platform/RenderAPI.h"
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
#include "VirtualKeyboard.h"
#endif

GuiTextField::GuiTextField(FontRenderer *fontrenderer, int_t x, int_t y, int_t w, int_t h)
    : GuiTextField(nullptr, fontrenderer, x, y, w, h, "")
{
}

GuiTextField::GuiTextField(GuiScreen *parent, FontRenderer *fontrenderer,
                           int_t x, int_t y, int_t w, int_t h, const jstring &initialText)
    : isFocused(false)
    , isEnabled(true)
    , fontRenderer(fontrenderer)
    , xPos(x)
    , yPos(y)
    , width(w)
    , height(h)
    , text()
    , maxStringLength(32)
    , cursorCounter(0)
    , enableBackgroundDrawing(true)
    , canLoseFocus(true)
    , lineScrollOffset(0)
    , cursorPosition(0)
    , selectionEnd(0)
    , enabledColor(14737632)
    , disabledColor(7368816)
    , parentGuiScreen(parent)
{
    setText(initialText);
}

int_t GuiTextField::textLength() const
{
    return String::utf16Length(text);
}

void GuiTextField::updateCursorCounter()
{
    ++cursorCounter;
}

void GuiTextField::setText(const jstring &s)
{
    if (String::utf16Length(s) > maxStringLength)
        text = String::substringUtf16(s, 0, maxStringLength);
    else
        text = s;
    setCursorPositionEnd();
}

const jstring &GuiTextField::getText() const
{
    return text;
}

jstring GuiTextField::getSelectedText() const
{
    int_t start = std::min(cursorPosition, selectionEnd);
    int_t end = std::max(cursorPosition, selectionEnd);
    return String::substringUtf16(text, start, end);
}

void GuiTextField::writeText(const jstring &s)
{
    jstring filtered = ChatAllowedCharacters::filterAllowedCharacters(s);
    int_t start = std::min(cursorPosition, selectionEnd);
    int_t end = std::max(cursorPosition, selectionEnd);
    int_t available = maxStringLength - textLength() - (start - end);
    if (available < 0)
        available = 0;

    int_t insertLength = std::min(available, String::utf16Length(filtered));
    jstring result;
    if (start > 0)
        result = String::substringUtf16(text, 0, start);
    if (insertLength > 0)
        result += String::substringUtf16(filtered, 0, insertLength);
    if (end < textLength())
        result += String::substringUtf16(text, end, textLength());

    text = result;
    moveCursorBy(start - selectionEnd + insertLength);
}

void GuiTextField::deleteWords(int_t words)
{
    if (textLength() == 0)
        return;
    if (selectionEnd != cursorPosition)
    {
        writeText("");
        return;
    }
    deleteFromCursor(getNthWordFromCursor(words) - cursorPosition);
}

void GuiTextField::deleteFromCursor(int_t amount)
{
    if (textLength() == 0)
        return;
    if (selectionEnd != cursorPosition)
    {
        writeText("");
        return;
    }

    const bool backwards = amount < 0;
    int_t start = backwards ? cursorPosition + amount : cursorPosition;
    int_t end = backwards ? cursorPosition : cursorPosition + amount;
    start = std::max(0, start);
    end = std::min(textLength(), end);
    if (start >= end)
        return;

    jstring result;
    if (start > 0)
        result = String::substringUtf16(text, 0, start);
    if (end < textLength())
        result += String::substringUtf16(text, end, textLength());
    text = result;
    if (backwards)
        moveCursorBy(start - cursorPosition);
}

int_t GuiTextField::getNthWordFromCursor(int_t words) const
{
    return getNthWordFromPos(words, cursorPosition);
}

int_t GuiTextField::getNthWordFromPos(int_t words, int_t position) const
{
    std::vector<char_t> units = String::toUtf16(text);
    int_t pos = std::max(0, std::min(position, (int_t)units.size()));
    const bool backwards = words < 0;
    const int_t count = JavaArithmetic::intAbs(words);

    for (int_t iteration = 0; iteration < count; ++iteration)
    {
        if (!backwards)
        {
            int_t length = (int_t)units.size();
            while (pos < length && units[(std::size_t)pos] != ' ')
                ++pos;
            while (pos < length && units[(std::size_t)pos] == ' ')
                ++pos;
        }
        else
        {
            while (pos > 0 && units[(std::size_t)pos - 1] == ' ')
                --pos;
            while (pos > 0 && units[(std::size_t)pos - 1] != ' ')
                --pos;
        }
    }
    return pos;
}

void GuiTextField::moveCursorBy(int_t amount)
{
    setCursorPosition(selectionEnd + amount);
}

void GuiTextField::setCursorPosition(int_t position)
{
    cursorPosition = std::max(0, std::min(position, textLength()));
    setSelectionPos(cursorPosition);
}

void GuiTextField::setCursorPositionZero()
{
    setCursorPosition(0);
}

void GuiTextField::setCursorPositionEnd()
{
    setCursorPosition(textLength());
}

bool GuiTextField::textboxKeyTyped(char_t c, int_t key)
{
    if (!isEnabled || !isFocused)
        return false;

    switch (c)
    {
        case 1: // Ctrl+A
            setCursorPositionEnd();
            setSelectionPos(0);
            return true;
        case 3: // Ctrl+C
            GuiScreen::setClipboardString(getSelectedText());
            return true;
        case 22: // Ctrl+V
            writeText(GuiScreen::getClipboardString());
            return true;
        case 24: // Ctrl+X
            GuiScreen::setClipboardString(getSelectedText());
            writeText("");
            return true;
        default:
            break;
    }

    switch (key)
    {
        case lwjgl::Keyboard::KEY_BACK:
            if (GuiScreen::isCtrlKeyDown()) deleteWords(-1);
            else deleteFromCursor(-1);
            return true;
        case lwjgl::Keyboard::KEY_HOME:
            if (GuiScreen::isShiftKeyDown()) setSelectionPos(0);
            else setCursorPositionZero();
            return true;
        case lwjgl::Keyboard::KEY_LEFT:
            if (GuiScreen::isShiftKeyDown())
            {
                if (GuiScreen::isCtrlKeyDown()) setSelectionPos(getNthWordFromPos(-1, getSelectionEnd()));
                else setSelectionPos(getSelectionEnd() - 1);
            }
            else if (GuiScreen::isCtrlKeyDown()) setCursorPosition(getNthWordFromCursor(-1));
            else moveCursorBy(-1);
            return true;
        case lwjgl::Keyboard::KEY_RIGHT:
            if (GuiScreen::isShiftKeyDown())
            {
                if (GuiScreen::isCtrlKeyDown()) setSelectionPos(getNthWordFromPos(1, getSelectionEnd()));
                else setSelectionPos(getSelectionEnd() + 1);
            }
            else if (GuiScreen::isCtrlKeyDown()) setCursorPosition(getNthWordFromCursor(1));
            else moveCursorBy(1);
            return true;
        case lwjgl::Keyboard::KEY_END:
            if (GuiScreen::isShiftKeyDown()) setSelectionPos(textLength());
            else setCursorPositionEnd();
            return true;
        case lwjgl::Keyboard::KEY_DELETE:
            if (GuiScreen::isCtrlKeyDown()) deleteWords(1);
            else deleteFromCursor(1);
            return true;
        default:
            break;
    }

    if (c == '\t' && parentGuiScreen != nullptr)
    {
        parentGuiScreen->selectNextField();
        return true;
    }
    if (ChatAllowedCharacters::isAllowedCharacter(c))
    {
        jstring one;
        String::appendUtf16Unit(one, c);
        writeText(one);
        return true;
    }
    return false;
}

void GuiTextField::mouseClicked(int_t x, int_t y, int_t button)
{
    bool inside = x >= xPos && x < xPos + width && y >= yPos && y < yPos + height;
    if (canLoseFocus)
        setFocused(isEnabled && inside);

    if (isFocused && button == 0)
    {
        int_t localX = x - xPos;
        if (enableBackgroundDrawing)
            localX -= 4;
        jstring visible = fontRenderer->trimStringToWidth(
            String::substringUtf16(text, lineScrollOffset, textLength()), getWidth());
        jstring beforeCursor = fontRenderer->trimStringToWidth(visible, std::max(0, localX));
        setCursorPosition(lineScrollOffset + String::utf16Length(beforeCursor));
    }
}

void GuiTextField::drawTextBox()
{
    if (enableBackgroundDrawing)
    {
        drawRect(xPos - 1, yPos - 1, xPos + width + 1, yPos + height + 1, 0xffa0a0a0);
        drawRect(xPos, yPos, xPos + width, yPos + height, 0xff000000);
    }

    int_t color = isEnabled ? enabledColor : disabledColor;
    int_t cursorOffset = cursorPosition - lineScrollOffset;
    int_t selectionOffset = selectionEnd - lineScrollOffset;
    jstring visible = fontRenderer->trimStringToWidth(
        String::substringUtf16(text, lineScrollOffset, textLength()), getWidth());
    int_t visibleLength = String::utf16Length(visible);
    bool cursorVisibleInText = cursorOffset >= 0 && cursorOffset <= visibleLength;
    bool blink = isFocused && (cursorCounter / 6) % 2 == 0 && cursorVisibleInText;
    int_t drawX = enableBackgroundDrawing ? xPos + 4 : xPos;
    int_t drawY = enableBackgroundDrawing ? yPos + (height - 8) / 2 : yPos;
    int_t cursorX = drawX;

    if (selectionOffset > visibleLength)
        selectionOffset = visibleLength;

    if (visibleLength > 0)
    {
        jstring before = cursorVisibleInText ? String::substringUtf16(visible, 0, cursorOffset) : visible;
        fontRenderer->drawStringWithShadow(before, drawX, drawY, color);
        cursorX = drawX + fontRenderer->getStringWidth(before);
    }

    bool cursorInsideExistingText = cursorPosition < textLength() || textLength() >= maxStringLength;
    int_t caretX = cursorX;
    if (!cursorVisibleInText)
        caretX = cursorOffset > 0 ? drawX + width : drawX;
    else if (cursorInsideExistingText)
        --caretX;

    if (visibleLength > 0 && cursorVisibleInText && cursorOffset < visibleLength)
    {
        jstring after = String::substringUtf16(visible, cursorOffset, visibleLength);
        fontRenderer->drawStringWithShadow(after, cursorX, drawY, color);
    }

    if (blink)
    {
        if (cursorInsideExistingText)
            drawRect(caretX, drawY - 1, caretX + 1, drawY + 1 + 8, 0xffd0d0d0);
        else
            fontRenderer->drawStringWithShadow("_", caretX, drawY, color);
    }

    if (selectionOffset != cursorOffset)
    {
        int_t selectionX = drawX + fontRenderer->getStringWidth(
            String::substringUtf16(visible, 0, std::max(0, selectionOffset)));
        drawSelectionBox(caretX, drawY - 1, selectionX - 1, drawY + 1 + 8);
    }
}

void GuiTextField::drawSelectionBox(int_t x1, int_t y1, int_t x2, int_t y2)
{
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);
    // Java used GL_OR_REVERSE. A translucent blue rectangle preserves selection
    // visibility on every backend without depending on an OpenGL-only logic op.
    drawRect(x1, y1, x2, y2, 0x804040ff);
}

void GuiTextField::setMaxStringLength(int_t maxLen)
{
    maxStringLength = std::max(0, maxLen);
    if (textLength() > maxStringLength)
        text = String::substringUtf16(text, 0, maxStringLength);
}

int_t GuiTextField::getMaxStringLength() const
{
    return maxStringLength;
}

int_t GuiTextField::getCursorPosition() const
{
    return cursorPosition;
}

bool GuiTextField::getEnableBackgroundDrawing() const
{
    return enableBackgroundDrawing;
}

void GuiTextField::setEnableBackgroundDrawing(bool enabled)
{
    enableBackgroundDrawing = enabled;
}

void GuiTextField::setFocused(bool focused)
{
    if (focused && !isFocused)
        cursorCounter = 0;
    isFocused = focused;
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
    VirtualKeyboard::instance().notifyFocus(this, focused);
#endif
}

bool GuiTextField::getFocused() const
{
    return isFocused;
}

int_t GuiTextField::getSelectionEnd() const
{
    return selectionEnd;
}

int_t GuiTextField::getWidth() const
{
    return enableBackgroundDrawing ? width - 8 : width;
}

void GuiTextField::setSelectionPos(int_t position)
{
    int_t length = textLength();
    selectionEnd = std::max(0, std::min(position, length));
    if (fontRenderer == nullptr)
        return;

    lineScrollOffset = std::max(0, std::min(lineScrollOffset, length));
    int_t fieldWidth = getWidth();
    jstring visible = fontRenderer->trimStringToWidth(
        String::substringUtf16(text, lineScrollOffset, length), fieldWidth);
    int_t visibleEnd = lineScrollOffset + String::utf16Length(visible);

    if (selectionEnd == lineScrollOffset)
    {
        jstring before = fontRenderer->trimStringToWidth(
            String::substringUtf16(text, 0, lineScrollOffset), fieldWidth, true);
        lineScrollOffset -= String::utf16Length(before);
    }
    if (selectionEnd > visibleEnd)
        lineScrollOffset += selectionEnd - visibleEnd;
    else if (selectionEnd <= lineScrollOffset)
        lineScrollOffset = selectionEnd;

    lineScrollOffset = std::max(0, std::min(lineScrollOffset, length));
}

void GuiTextField::setCanLoseFocus(bool value)
{
    canLoseFocus = value;
}
