#include "FontRenderer.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <cmath>

#include "ChatAllowedCharacters.h"
#include "java/String.h"
#include "java/Arithmetic.h"
#if PLATFORM_PC
#include "GLAllocation.h"
#endif
#include "GameSettings.h"
#include "Config.h"
#include "OptiFineResource.h"
#include "RenderEngine.h"
#include "Tessellator.h"
#include "platform/RenderAPI.h"
#include "platform/PlatformConfig.h"


FontRenderer::FontRenderer(GameSettings *gamesettings, const std::string &s, RenderEngine *renderengine)
	: gameSettings(gamesettings)
	, fontTexturePath(s)
	, fontTextureName(0)
	, fontDisplayLists(0)
	, textBatchActive(false)
	, unicodeFlag(false)
	, bidiFlag(false)
{
	for (int_t i = 0; i < 256; i++)
		charWidth[i] = 0;
	buffer.reserve(1024);
	refresh(renderengine);
}

void FontRenderer::refresh(RenderEngine *renderEngine)
{
	if (renderEngine == nullptr)
		return;

#if PLATFORM_PC
	if (fontDisplayLists != 0)
	{
		GLAllocation::deleteDisplayLists(fontDisplayLists);
		fontDisplayLists = 0;
	}
#endif

	fontTextureName = renderEngine->getTexture(fontTexturePath);
	std::vector<int_t> pixels = renderEngine->readTextureImageData(fontTexturePath);
	int_t imageWidth = 128;
	int_t imageHeight = 128;
	renderEngine->getTextureDimensions(fontTextureName, &imageWidth, &imageHeight);
	if (imageWidth <= 0 || imageHeight <= 0 || imageWidth % 16 != 0 || imageHeight % 16 != 0 ||
	    pixels.size() < static_cast<std::size_t>(imageWidth) * static_cast<std::size_t>(imageHeight))
	{
		for (int_t k = 0; k < 256; ++k)
			charWidth[k] = k == 32 ? 4.0f : 8.0f;
	}
	else
	{
		const int_t charW = imageWidth / 16;
		const int_t charH = imageHeight / 16;
		const float_t scaleX = static_cast<float_t>(imageWidth) / 128.0f;
		for (int_t k = 0; k < 256; ++k)
		{
			const int_t cx = k % 16;
			const int_t cy = k / 16;
			int_t px = charW - 1;
			for (; px >= 0; --px)
			{
				const int_t x = cx * charW + px;
				bool emptyColumn = true;
				for (int_t py = 0; py < charH && emptyColumn; ++py)
				{
					const int_t color = pixels[static_cast<std::size_t>(cy * charH + py) * static_cast<std::size_t>(imageWidth) + static_cast<std::size_t>(x)];
					if (((color >> 24) & 0xff) > 16)
						emptyColumn = false;
				}
				if (!emptyColumn)
					break;
			}
			if (k == 32)
				px = static_cast<int_t>(1.5f * scaleX);
			charWidth[k] = (static_cast<float_t>(px) + 1.0f) / scaleX + 1.0f;
		}
	}

	readCustomCharWidths(fontTexturePath, renderEngine);

#if PLATFORM_FONT_IMMEDIATE
	// Immediate backends build glyph quads on demand and have no display-list
	// state to rebuild. Widths and the selected texture above still refresh.
	fontDisplayLists = 0;
#else
	fontDisplayLists = GLAllocation::generateDisplayLists(288);
	Tessellator *tessellator = &Tessellator::instance;
	for (int_t i1 = 0; i1 < 256; i1++)
	{
		renderBeginDisplayList(fontDisplayLists + i1);
		tessellator->startDrawingQuads();
		int_t l1 = (i1 % 16) * 8;
		int_t k2 = (i1 / 16) * 8;
		float f = 7.99f;
		float f1 = 0.0f;
		float f2 = 0.0f;
		tessellator->addVertexWithUV(0.0, 0.0f + f, 0.0, (float)l1 / 128.0f + f1, ((float)k2 + f) / 128.0f + f2);
		tessellator->addVertexWithUV(0.0f + f, 0.0f + f, 0.0, ((float)l1 + f) / 128.0f + f1, ((float)k2 + f) / 128.0f + f2);
		tessellator->addVertexWithUV(0.0f + f, 0.0, 0.0, ((float)l1 + f) / 128.0f + f1, (float)k2 / 128.0f + f2);
		tessellator->addVertexWithUV(0.0, 0.0, 0.0, (float)l1 / 128.0f + f1, (float)k2 / 128.0f + f2);
		tessellator->draw();
		renderTranslate((float)charWidth[i1], 0.0f, 0.0f);
		renderEndDisplayList();
	}

	for (int_t j1 = 0; j1 < 32; j1++)
	{
		int_t i2 = (j1 >> 3 & 1) * 85;
		int_t l2 = (j1 >> 2 & 1) * 170 + i2;
		int_t j3 = (j1 >> 1 & 1) * 170 + i2;
		int_t k3 = (j1 >> 0 & 1) * 170 + i2;
		if (j1 == 6)
			l2 += 85;
		bool flag1 = j1 >= 16;
		if (gameSettings != nullptr && gameSettings->anaglyph)
		{
			int_t j4 = (l2 * 30 + j3 * 59 + k3 * 11) / 100;
			int_t l4 = (l2 * 30 + j3 * 70) / 100;
			int_t i5 = (l2 * 30 + k3 * 70) / 100;
			l2 = j4;
			j3 = l4;
			k3 = i5;
		}
		if (flag1)
		{
			l2 /= 4;
			j3 /= 4;
			k3 /= 4;
		}
		renderBeginDisplayList(fontDisplayLists + 256 + j1);
		renderColor3f((float)l2 / 255.0f, (float)j3 / 255.0f, (float)k3 / 255.0f);
		renderEndDisplayList();
	}
#endif
}

void FontRenderer::readCustomCharWidths(const std::string &textureFile, RenderEngine *renderEngine)
{
	if (!Config::isCustomFonts() || renderEngine == nullptr)
		return;
	const std::string suffix = ".png";
	if (textureFile.size() < suffix.size() || textureFile.compare(textureFile.size() - suffix.size(), suffix.size(), suffix) != 0)
		return;
	const std::string propertiesFile = textureFile.substr(0, textureFile.size() - suffix.size()) + ".properties";
	const std::map<std::string, std::string> properties = OptiFineResource::readProperties(renderEngine, propertiesFile);
	for (const auto &entry : properties)
	{
		const std::string prefix = "width.";
		if (entry.first.rfind(prefix, 0) != 0)
			continue;
		const int_t index = OptiFineResource::parseInt(entry.first.substr(prefix.size()), -1);
		const float_t width = OptiFineResource::parseFloat(entry.second, -1.0f);
		if (index >= 0 && index < 256 && width >= 0.0f)
			charWidth[index] = width;
	}
}

FontRenderer::~FontRenderer()
{
#if PLATFORM_PC
	if (fontDisplayLists != 0)
	{
		GLAllocation::deleteDisplayLists(fontDisplayLists);
		fontDisplayLists = 0;
	}
#endif
}

int_t FontRenderer::drawStringWithShadow(const std::string &s, int_t i, int_t j, int_t k)
{
	const int_t shadowEnd = renderString(s, JavaArithmetic::intAdd(i, 1), JavaArithmetic::intAdd(j, 1), k, true);
	const int_t textEnd = renderString(s, i, j, k, false);
	return std::max(shadowEnd, textEnd);
}

void FontRenderer::drawString(const std::string &s, int_t i, int_t j, int_t k)
{
	renderString(s, i, j, k, false);
}

void FontRenderer::drawStringScaled(const std::string &s, float_t x, float_t y, float_t scale, int_t color)
{
	if (scale <= 0.0f)
		return;
	renderStringScaled(s, x, y, color, false, scale);
}

void FontRenderer::beginTextBatch()
{
#if PLATFORM_FONT_IMMEDIATE
	if (textBatchActive)
		return;

	renderBindTexture(fontTextureName);
	renderEnable(RenderCapability::Texture2D);
	renderDisable(RenderCapability::Lighting);
	renderDisable(RenderCapability::Fog);
	renderEnable(RenderCapability::Blend);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
	Tessellator::instance.startDrawingQuads();
	textBatchActive = true;
#endif
}

void FontRenderer::endTextBatch()
{
#if PLATFORM_FONT_IMMEDIATE
	if (!textBatchActive)
		return;

	Tessellator::instance.draw();
	textBatchActive = false;
	flushTextDecorations();
	renderDisable(RenderCapability::Blend);
#endif
}

void FontRenderer::flushTextDecorations()
{
	if (pendingDecorations.empty())
		return;

	renderDisable(RenderCapability::Texture2D);
	Tessellator &tessellator = Tessellator::instance;
	tessellator.startDrawingQuads();
	for (const DecorationRect &rect : pendingDecorations)
	{
		tessellator.setColorRGBA_F(rect.r, rect.g, rect.b, rect.a);
		tessellator.addVertex(rect.x0, rect.y1, 0.0);
		tessellator.addVertex(rect.x1, rect.y1, 0.0);
		tessellator.addVertex(rect.x1, rect.y0, 0.0);
		tessellator.addVertex(rect.x0, rect.y0, 0.0);
	}
	tessellator.draw();
	pendingDecorations.clear();
	renderEnable(RenderCapability::Texture2D);
}

int_t FontRenderer::renderString(const std::string &s, int_t i, int_t j, int_t k, bool flag)
{
	return JavaArithmetic::floatToInt(renderStringScaled(s, (float_t)i, (float_t)j, k, flag, 1.0f));
}

float_t FontRenderer::renderStringScaled(const std::string &s, float_t x, float_t y, int_t k, bool flag, float_t scale)
{
	if (s.empty())
		return x;

	if (flag)
	{
		int_t alpha = k & 0xff000000;
		k = (k & 0xfcfcfc) >> 2;
		k += alpha;
	}

	const float_t baseR = static_cast<float_t>(k >> 16 & 0xff) / 255.0f;
	const float_t baseG = static_cast<float_t>(k >> 8 & 0xff) / 255.0f;
	const float_t baseB = static_cast<float_t>(k & 0xff) / 255.0f;
	float_t baseA = static_cast<float_t>(k >> 24 & 0xff) / 255.0f;
	if (baseA == 0.0f)
		baseA = 1.0f;

	float_t currentR = baseR;
	float_t currentG = baseG;
	float_t currentB = baseB;
	float_t currentA = baseA;
	bool obfuscated = false;
	bool bold = false;
	bool strikethrough = false;
	bool underline = false;
	bool italic = false;

	auto resetStyles = [&]()
	{
		obfuscated = false;
		bold = false;
		strikethrough = false;
		underline = false;
		italic = false;
	};

	auto applyColorCode = [&](int_t colorIndex)
	{
		int_t i2 = (colorIndex >> 3 & 1) * 85;
		int_t red = (colorIndex >> 2 & 1) * 170 + i2;
		int_t green = (colorIndex >> 1 & 1) * 170 + i2;
		int_t blue = (colorIndex >> 0 & 1) * 170 + i2;
		if (colorIndex == 6)
			red += 85;
		if (flag)
		{
			red /= 4;
			green /= 4;
			blue /= 4;
		}
		currentR = static_cast<float_t>(red) / 255.0f;
		currentG = static_cast<float_t>(green) / 255.0f;
		currentB = static_cast<float_t>(blue) / 255.0f;
		currentA = baseA;
	};

	auto addDecoration = [&](float_t x0, float_t y0, float_t x1, float_t y1)
	{
		pendingDecorations.push_back({x0, y0, x1, y1,
			currentR, currentG, currentB, currentA});
	};

	const std::vector<char_t> units = String::toUtf16(jstring(s));
	const std::string formatCodes = "0123456789abcdefklmnor";

#if PLATFORM_FONT_IMMEDIATE
	const bool ownsBatch = !textBatchActive;
	if (ownsBatch)
		beginTextBatch();
	Tessellator *tessellator = &Tessellator::instance;
	tessellator->setColorRGBA_F(currentR, currentG, currentB, currentA);

	float_t cursor = 0.0f;
	for (std::size_t i = 0; i < units.size(); ++i)
	{
		if (units[i] == 0x00a7u && i + 1 < units.size())
		{
			const char_t codeUnit = units[++i];
			const char code = codeUnit <= 0x7fu
				? static_cast<char>(std::tolower(static_cast<unsigned char>(codeUnit)))
				: '\0';
			const std::size_t found = formatCodes.find(code);
			if (found < 16)
			{
				resetStyles();
				applyColorCode(static_cast<int_t>(found));
				tessellator->setColorRGBA_F(currentR, currentG, currentB, currentA);
			}
			else if (found == 16)
				obfuscated = true;
			else if (found == 17)
				bold = true;
			else if (found == 18)
				strikethrough = true;
			else if (found == 19)
				underline = true;
			else if (found == 20)
				italic = true;
			else if (found == 21)
			{
				resetStyles();
				currentR = baseR;
				currentG = baseG;
				currentB = baseB;
				currentA = baseA;
				tessellator->setColorRGBA_F(currentR, currentG, currentB, currentA);
			}
			continue;
		}

		int_t charIndex = getCharIndex(units[i]);
		if (charIndex < 0)
			continue;

		if (obfuscated && charIndex > 0)
		{
			const float_t originalWidth = charWidth[charIndex + 32];
			int_t candidate = charIndex;
			do
			{
				candidate = fontRandom.nextInt(static_cast<int_t>(ChatAllowedCharacters::allowedCharacters.length()));
			}
			while (charWidth[candidate + 32] != originalWidth);
			charIndex = candidate;
		}

		const int_t glyph = charIndex + 32;
		const float_t glyphWidth = static_cast<float_t>(charWidth[glyph]);
		const int_t col = glyph % 16;
		const int_t row = glyph / 16;
		const float_t u0 = static_cast<float_t>(col * 8) / 128.0f;
		const float_t v0 = static_cast<float_t>(row * 8) / 128.0f;
		const float_t u1 = static_cast<float_t>(col * 8 + 7.99f) / 128.0f;
		const float_t v1 = static_cast<float_t>(row * 8 + 7.99f) / 128.0f;
		const float_t italicShift = italic ? scale : 0.0f;
		const float_t x0 = x + cursor;
		const float_t y0 = y;
		const float_t x1 = x0 + 7.99f * scale;
		const float_t y1 = y0 + 7.99f * scale;

		auto emitGlyph = [&](float_t offset)
		{
			tessellator->addVertexWithUV(x0 + offset - italicShift, y1, 0.0, u0, v1);
			tessellator->addVertexWithUV(x1 + offset - italicShift, y1, 0.0, u1, v1);
			tessellator->addVertexWithUV(x1 + offset + italicShift, y0, 0.0, u1, v0);
			tessellator->addVertexWithUV(x0 + offset + italicShift, y0, 0.0, u0, v0);
		};

		emitGlyph(0.0f);
		if (bold)
			emitGlyph(scale);

		const float_t advance = (glyphWidth + (bold ? 1.0f : 0.0f)) * scale;
		if (strikethrough)
			addDecoration(x0, y + 4.0f * scale, x0 + advance, y + 5.0f * scale);
		if (underline)
			addDecoration(x0 - scale, y + 8.0f * scale, x0 + advance, y + 9.0f * scale);
		cursor += advance;
	}

	if (ownsBatch)
		endTextBatch();
	return x + cursor;
#else
	renderBindTexture(fontTextureName);
	renderColor4f(baseR, baseG, baseB, baseA);
	renderPushMatrix();
	renderTranslate(x, y, 0.0f);
	renderScale(scale, scale, scale);

	float_t cursor = 0.0f;
	for (std::size_t i = 0; i < units.size(); ++i)
	{
		if (units[i] == 0x00a7u && i + 1 < units.size())
		{
			const char_t codeUnit = units[++i];
			const char code = codeUnit <= 0x7fu
				? static_cast<char>(std::tolower(static_cast<unsigned char>(codeUnit)))
				: '\0';
			const std::size_t found = formatCodes.find(code);
			if (found < 16)
			{
				resetStyles();
				applyColorCode(static_cast<int_t>(found));
				const int_t colorList = fontDisplayLists + 256 + static_cast<int_t>(found) + (flag ? 16 : 0);
				renderCallDisplayLists(1, &colorList);
			}
			else if (found == 16)
				obfuscated = true;
			else if (found == 17)
				bold = true;
			else if (found == 18)
				strikethrough = true;
			else if (found == 19)
				underline = true;
			else if (found == 20)
				italic = true;
			else if (found == 21)
			{
				resetStyles();
				currentR = baseR;
				currentG = baseG;
				currentB = baseB;
				currentA = baseA;
				renderColor4f(baseR, baseG, baseB, baseA);
			}
			continue;
		}

		int_t charIndex = getCharIndex(units[i]);
		if (charIndex < 0)
			continue;

		if (obfuscated && charIndex > 0)
		{
			const float_t originalWidth = charWidth[charIndex + 32];
			int_t candidate = charIndex;
			do
			{
				candidate = fontRandom.nextInt(static_cast<int_t>(ChatAllowedCharacters::allowedCharacters.length()));
			}
			while (charWidth[candidate + 32] != originalWidth);
			charIndex = candidate;
		}

		const int_t glyph = charIndex + 32;
		const float_t glyphWidth = static_cast<float_t>(charWidth[glyph]);
		if (italic)
		{
			const int_t col = glyph % 16;
			const int_t row = glyph / 16;
			const float_t u0 = static_cast<float_t>(col * 8) / 128.0f;
			const float_t v0 = static_cast<float_t>(row * 8) / 128.0f;
			const float_t u1 = static_cast<float_t>(col * 8 + 7.99f) / 128.0f;
			const float_t v1 = static_cast<float_t>(row * 8 + 7.99f) / 128.0f;
			Tessellator &tessellator = Tessellator::instance;
			tessellator.startDrawingQuads();
			tessellator.addVertexWithUV(-1.0, 7.99, 0.0, u0, v1);
			tessellator.addVertexWithUV(6.99, 7.99, 0.0, u1, v1);
			tessellator.addVertexWithUV(8.99, 0.0, 0.0, u1, v0);
			tessellator.addVertexWithUV(1.0, 0.0, 0.0, u0, v0);
			if (bold)
			{
				tessellator.addVertexWithUV(0.0, 7.99, 0.0, u0, v1);
				tessellator.addVertexWithUV(7.99, 7.99, 0.0, u1, v1);
				tessellator.addVertexWithUV(9.99, 0.0, 0.0, u1, v0);
				tessellator.addVertexWithUV(2.0, 0.0, 0.0, u0, v0);
			}
			tessellator.draw();
			renderTranslate(glyphWidth + (bold ? 1.0f : 0.0f), 0.0f, 0.0f);
		}
		else
		{
			const int_t glyphList = fontDisplayLists + glyph;
			if (bold)
			{
				renderPushMatrix();
				renderTranslate(1.0f, 0.0f, 0.0f);
				renderCallDisplayLists(1, &glyphList);
				renderPopMatrix();
			}
			renderCallDisplayLists(1, &glyphList);
			if (bold)
				renderTranslate(1.0f, 0.0f, 0.0f);
		}

		const float_t advance = glyphWidth + (bold ? 1.0f : 0.0f);
		if (strikethrough)
			addDecoration(x + cursor * scale, y + 4.0f * scale,
				x + (cursor + advance) * scale, y + 5.0f * scale);
		if (underline)
			addDecoration(x + (cursor - 1.0f) * scale, y + 8.0f * scale,
				x + (cursor + advance) * scale, y + 9.0f * scale);
		cursor += advance;
	}

	renderPopMatrix();
	flushTextDecorations();
	return x + cursor * scale;
#endif
}

int_t FontRenderer::getStringWidth(const std::string &s)
{
	if (s.empty())
		return 0;
	const std::vector<char_t> units = String::toUtf16(jstring(s));
	float_t width = 0.0f;
	bool bold = false;
	for (std::size_t index = 0; index < units.size(); ++index)
	{
		const char_t unit = units[index];
		if (unit == 0x00a7u && index + 1 < units.size())
		{
			char_t code = units[++index];
			if (code >= 'A' && code <= 'Z')
				code = (char_t)(code + ('a' - 'A'));
			if (code == 'l')
				bold = true;
			else if (code == 'r' || (code >= '0' && code <= '9') || (code >= 'a' && code <= 'f'))
				bold = false;
			continue;
		}

		const int_t charIndex = getCharIndex(unit);
		if (charIndex < 0)
			continue;
		const float_t charSize = charWidth[charIndex + 32];
		width += charSize;
		if (bold && charSize > 0.0f)
			width += 1.0f;
	}
	return static_cast<int_t>(std::lround(width));
}

std::string FontRenderer::trimStringToWidth(const std::string &s, int_t width, bool reverse)
{
	const std::vector<char_t> units = String::toUtf16(jstring(s));
	std::vector<char_t> out;
	float_t usedWidth = 0.0f;
	bool formatting = false;
	bool bold = false;
	int_t index = reverse ? (int_t)units.size() - 1 : 0;
	int_t step = reverse ? -1 : 1;
	while (index >= 0 && index < (int_t)units.size() && usedWidth < width)
	{
		char_t c = units[(std::size_t)index];
		if (formatting)
		{
			formatting = false;
			char_t lower = c >= 'A' && c <= 'Z' ? (char_t)(c + ('a' - 'A')) : c;
			if (lower == 'l')
				bold = true;
			else if (lower == 'r' || (lower >= '0' && lower <= '9') || (lower >= 'a' && lower <= 'f'))
				bold = false;
		}
		else if (c == 0x00a7u)
		{
			formatting = true;
		}
		else
		{
			int_t charIndex = getCharIndex(c);
			float_t charSize = charIndex >= 0 ? charWidth[charIndex + 32] : 0.0f;
			if (bold && charSize > 0.0f)
				charSize += 1.0f;
			usedWidth += charSize;
			if (usedWidth > width)
				break;
		}
		if (reverse)
			out.insert(out.begin(), c);
		else
			out.push_back(c);
		index += step;
	}
	return String::fromUtf16(out);
}

std::string FontRenderer::stripFormattingCodes(const std::string &s)
{
	const std::vector<char_t> units = String::toUtf16(jstring(s));
	std::vector<char_t> out;
	out.reserve(units.size());
	for (std::size_t i = 0; i < units.size(); ++i)
	{
		if (units[i] == 0x00a7u && i + 1 < units.size())
		{
			char_t c = units[i + 1];
			char_t lower = c >= 'A' && c <= 'Z' ? (char_t)(c + ('a' - 'A')) : c;
			if ((lower >= '0' && lower <= '9') || (lower >= 'a' && lower <= 'f') ||
				lower == 'k' || lower == 'l' || lower == 'm' || lower == 'n' || lower == 'o' || lower == 'r')
			{
				++i;
				continue;
			}
		}
		out.push_back(units[i]);
	}
	return String::fromUtf16(out);
}

void FontRenderer::drawSplitStringInternal(const std::string &s, int_t i, int_t j, int_t k, int_t l)
{
	std::vector<std::string> lines = split(s, '\n');
	if (lines.size() > 1)
	{
		for (std::string &line : lines)
		{
			drawSplitStringInternal(line, i, j, k, l);
			j += splitStringWidthInternal(line, k);
		}
		return;
	}

	std::vector<std::string> words = split(s, ' ');
	int_t j1 = 0;
	while (j1 < (int_t)words.size())
	{
		std::string s1 = words[j1++] + " ";
		while (j1 < (int_t)words.size() && getStringWidth(s1 + words[j1]) < k)
			s1 += words[j1++] + " ";
		while (getStringWidth(s1) > k)
		{
			int_t k1 = 0;
			while (k1 + 1 < String::utf16Length(s1) && getStringWidth(String::substringUtf16(s1, 0, k1 + 1)) <= k)
				k1++;
			const jstring linePart = String::substringUtf16(s1, 0, k1);
			if (!String::trimJava(linePart).empty())
			{
				drawString(linePart, i, j, l);
				j += 8;
			}
			s1 = String::substringUtf16(s1, k1, String::utf16Length(s1));
		}
		if (!String::trimJava(s1).empty())
		{
			drawString(s1, i, j, l);
			j += 8;
		}
	}
}

int_t FontRenderer::splitStringWidthInternal(const std::string &s, int_t i)
{
	std::vector<std::string> lines = split(s, '\n');
	if (lines.size() > 1)
	{
		int_t j = 0;
		for (std::string &line : lines)
			j += splitStringWidthInternal(line, i);
		return j;
	}

	std::vector<std::string> words = split(s, ' ');
	int_t l = 0;
	int_t i1 = 0;
	while (l < (int_t)words.size())
	{
		std::string s1 = words[l++] + " ";
		while (l < (int_t)words.size() && getStringWidth(s1 + words[l]) < i)
			s1 += words[l++] + " ";
		while (getStringWidth(s1) > i)
		{
			int_t j1 = 0;
			while (j1 + 1 < String::utf16Length(s1) && getStringWidth(String::substringUtf16(s1, 0, j1 + 1)) <= i)
				j1++;
			if (!String::substringUtf16(s1, 0, j1).empty())
				i1 += 8;
			s1 = String::substringUtf16(s1, j1, String::utf16Length(s1));
		}
		if (!String::trimJava(s1).empty())
			i1 += 8;
	}
	if (i1 < 8)
		i1 += 8;
	return i1;
}

std::vector<std::string> FontRenderer::split(const std::string &s, char delimiter)
{
	const std::vector<jstring> javaParts = String::splitJava(s, delimiter);
	return std::vector<std::string>(javaParts.begin(), javaParts.end());
}

int_t FontRenderer::getCharIndex(char_t c)
{
	return String::indexOfUtf16Unit(ChatAllowedCharacters::allowedCharacters, c);
}

void FontRenderer::setUnicodeFlag(bool unicode)
{
	unicodeFlag = unicode;
}

void FontRenderer::setBidiFlag(bool bidi)
{
	bidiFlag = bidi;
}
