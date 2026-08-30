#pragma once

#include "Gui.h"
#include "java/String.h"

class GuiScreen;
class FontRenderer;

// net.minecraft.src.GuiTextField
class GuiTextField : public Gui
{
public:
	GuiTextField(FontRenderer *fontrenderer, int_t x, int_t y, int_t w, int_t h);
	GuiTextField(GuiScreen *parent, FontRenderer *fontrenderer,
	             int_t x, int_t y, int_t w, int_t h, const jstring &initialText);

	void updateCursorCounter();
	void setText(const jstring &s);
	const jstring &getText() const;
	jstring getSelectedText() const;
	void writeText(const jstring &s);
	void deleteWords(int_t words);
	void deleteFromCursor(int_t amount);
	int_t getNthWordFromCursor(int_t words) const;
	int_t getNthWordFromPos(int_t words, int_t position) const;
	void moveCursorBy(int_t amount);
	void setCursorPosition(int_t position);
	void setCursorPositionZero();
	void setCursorPositionEnd();
	bool textboxKeyTyped(char_t c, int_t key);
	void mouseClicked(int_t x, int_t y, int_t button);
	void drawTextBox();
	void setMaxStringLength(int_t maxLen);
	int_t getMaxStringLength() const;
	int_t getCursorPosition() const;
	bool getEnableBackgroundDrawing() const;
	void setEnableBackgroundDrawing(bool enabled);
	void setFocused(bool focused);
	bool getFocused() const;
	int_t getSelectionEnd() const;
	int_t getWidth() const;
	void setSelectionPos(int_t position);
	void setCanLoseFocus(bool canLoseFocus);
	void setEnabled(bool enabled) { isEnabled = enabled; }

	// MCP 1.2.5 compatibility names.
	jstring func_50039_c() const { return getSelectedText(); }
	void func_50031_b(const jstring &s) { writeText(s); }
	void func_50021_a(int_t words) { deleteWords(words); }
	void func_50020_b(int_t amount) { deleteFromCursor(amount); }
	int_t func_50028_c(int_t words) const { return getNthWordFromCursor(words); }
	int_t func_50024_a(int_t words, int_t position) const { return getNthWordFromPos(words, position); }
	void func_50023_d(int_t amount) { moveCursorBy(amount); }
	void func_50030_e(int_t position) { setCursorPosition(position); }
	void func_50034_d() { setCursorPositionZero(); }
	void func_50038_e() { setCursorPositionEnd(); }
	bool func_50037_a(char_t c, int_t key) { return textboxKeyTyped(c, key); }
	int_t func_50040_g() const { return getMaxStringLength(); }
	int_t func_50035_h() const { return getCursorPosition(); }
	bool func_50022_i() const { return getEnableBackgroundDrawing(); }
	void func_50027_a(bool enabled) { setEnableBackgroundDrawing(enabled); }
	void func_50033_b(bool focused) { setFocused(focused); }
	bool func_50025_j() const { return getFocused(); }
	int_t func_50036_k() const { return getSelectionEnd(); }
	int_t func_50019_l() const { return getWidth(); }
	void func_50032_g(int_t position) { setSelectionPos(position); }
	void func_50026_c(bool value) { setCanLoseFocus(value); }

public:
	bool isFocused;
	bool isEnabled;

private:
	void drawSelectionBox(int_t x1, int_t y1, int_t x2, int_t y2);
	int_t textLength() const;

	FontRenderer *fontRenderer;
	const int_t xPos;
	const int_t yPos;
	const int_t width;
	const int_t height;
	jstring text;
	int_t maxStringLength;
	int_t cursorCounter;
	bool enableBackgroundDrawing;
	bool canLoseFocus;
	int_t lineScrollOffset;
	int_t cursorPosition;
	int_t selectionEnd;
	int_t enabledColor;
	int_t disabledColor;
	GuiScreen *parentGuiScreen;
};
