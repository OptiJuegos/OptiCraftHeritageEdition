#pragma once

#include <string>
#include <vector>

#include "java/Type.h"
#include "java/Random.h"

class GameSettings;
class RenderEngine;

// net.minecraft.src.FontRenderer
class FontRenderer
{
public:
	FontRenderer(GameSettings *gamesettings, const std::string &s, RenderEngine *renderengine);
	~FontRenderer();
	void refresh(RenderEngine *renderEngine);

	int_t drawStringWithShadow(const std::string &s, int_t i, int_t j, int_t k);
	void drawString(const std::string &s, int_t i, int_t j, int_t k);
	void drawStringScaled(const std::string &s, float_t x, float_t y, float_t scale, int_t color);
	int_t renderString(const std::string &s, int_t i, int_t j, int_t k, bool flag);
	void beginTextBatch();
	void endTextBatch();
	int_t getStringWidth(const std::string &s);
	std::string trimStringToWidth(const std::string &s, int_t width, bool reverse = false);
	static std::string stripFormattingCodes(const std::string &s);
	void drawSplitStringInternal(const std::string &s, int_t i, int_t j, int_t k, int_t l);
	int_t splitStringWidthInternal(const std::string &s, int_t i);
	void setUnicodeFlag(bool unicode);
	void setBidiFlag(bool bidi);
	bool getUnicodeFlag() const { return unicodeFlag; }
	bool getBidiFlag() const { return bidiFlag; }

	void drawSplitString(const std::string &s, int_t i, int_t j, int_t k, int_t l) { drawSplitStringInternal(s, i, j, k, l); }
	int_t splitStringWidth(const std::string &s, int_t i) { return splitStringWidthInternal(s, i); }

private:
	GameSettings *gameSettings;
	std::string fontTexturePath;

	float_t renderStringScaled(const std::string &s, float_t x, float_t y, int_t color, bool shadow, float_t scale);
	std::vector<std::string> split(const std::string &s, char delimiter);
	int_t getCharIndex(char_t c);
	void readCustomCharWidths(const std::string &textureFile, RenderEngine *renderEngine);

	float_t charWidth[256];

public:
	int_t fontTextureName;
	Random fontRandom;

private:
	struct DecorationRect
	{
		float_t x0;
		float_t y0;
		float_t x1;
		float_t y1;
		float_t r;
		float_t g;
		float_t b;
		float_t a;
	};

	void flushTextDecorations();

	int_t fontDisplayLists;
	bool textBatchActive;
	bool unicodeFlag;
	bool bidiFlag;
	std::vector<int_t> buffer;
	std::vector<DecorationRect> pendingDecorations;
};
