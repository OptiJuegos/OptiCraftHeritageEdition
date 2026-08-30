#pragma once

#include "GuiScreen.h"
#include <string>

class StatFileWriter;
class GuiSlot;
class GuiSlotStatsGeneral;
class GuiSlotStatsItem;
class GuiSlotStatsBlock;
class RenderItem;
class Tessellator;
class Item;

// net.minecraft.src.GuiStats
class GuiStats : public GuiScreen
{
public:
	GuiStats(GuiScreen *guiscreen, StatFileWriter *statfilewriter);
	~GuiStats() override;

	void initGui() override;
	void addButtons();
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	void actionPerformed(GuiButton *button) override;

private:
	void renderItemInSlot(int_t x, int_t y, int_t itemId);
	void drawDefaultSlotIcon(int_t x, int_t y);
	void drawSlotIconInternal(int_t x, int_t y, int_t texX, int_t texY);

public:
	// Promote protected base members to public so slot classes can access via GuiStats*
	using GuiScreen::mc;
	using GuiScreen::fontRenderer;
	using Gui::drawGradientRect;
	using Gui::zLevel;

	// Static accessors used by slot classes (mirrors Java package-private pattern)
	static Minecraft    *getMc(GuiStats *g)             { return g->mc; }
	static FontRenderer *getFontRenderer(GuiStats *g)   { return g->fontRenderer; }
	static StatFileWriter *getStatFileWriter(GuiStats *g);

	static void drawSlotIconAt(GuiStats *g, int_t x, int_t y, int_t texX, int_t texY);
	static void drawItemIconAt(GuiStats *g, int_t x, int_t y, int_t itemId);
	static void drawGradientRectStatic(GuiStats *g, int_t x1, int_t y1, int_t x2, int_t y2, int_t c1, int_t c2);

protected:
	GuiScreen   *field_27152_a;
	std::string  field_27154_i;

private:
	GuiSlotStatsGeneral *field_27151_l;
	GuiSlotStatsItem    *field_27150_m;
	GuiSlotStatsBlock   *field_27157_n;
	StatFileWriter      *field_27156_o;
	GuiSlot             *field_27155_p;

	static RenderItem *field_27153_j;
};
