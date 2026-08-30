#include "GuiStats.h"
#include "GuiSlotStatsGeneral.h"
#include "GuiSlotStatsItem.h"
#include "GuiSlotStatsBlock.h"
#include "GuiSlot.h"
#include "GuiButton.h"
#include "StatCollector.h"
#include "StringTranslate.h"
#include "StatFileWriter.h"
#include "RenderItem.h"
#include "RenderHelper.h"
#include "RenderEngine.h"
#include "FontRenderer.h"
#include "Tessellator.h"
#include "Item.h"
#include "ItemStack.h"
#include "Minecraft.h"
#include "platform/RenderAPI.h"

RenderItem *GuiStats::field_27153_j = new RenderItem();

StatFileWriter *GuiStats::getStatFileWriter(GuiStats *g)
{
	return g->field_27156_o;
}

GuiStats::GuiStats(GuiScreen *guiscreen, StatFileWriter *statfilewriter)
	: field_27154_i("Select world")
	, field_27155_p(nullptr)
	, field_27152_a(guiscreen)
	, field_27156_o(statfilewriter)
	, field_27151_l(nullptr)
	, field_27150_m(nullptr)
	, field_27157_n(nullptr)
{
}

GuiStats::~GuiStats()
{
	delete field_27151_l;
	delete field_27150_m;
	delete field_27157_n;
}

void GuiStats::initGui()
{
	delete field_27151_l;
	delete field_27150_m;
	delete field_27157_n;
	field_27151_l = nullptr;
	field_27150_m = nullptr;
	field_27157_n = nullptr;
	field_27155_p = nullptr;
	field_27154_i = StatCollector::translateToLocal("gui.stats");
	field_27151_l = new GuiSlotStatsGeneral(this);
	field_27151_l->registerScrollButtons(controlList, 1, 1);
	field_27150_m = new GuiSlotStatsItem(this);
	field_27150_m->registerScrollButtons(controlList, 1, 1);
	field_27157_n = new GuiSlotStatsBlock(this);
	field_27157_n->registerScrollButtons(controlList, 1, 1);
	field_27155_p = field_27151_l;
	addButtons();
}

void GuiStats::addButtons()
{
	StringTranslate *stringtranslate = StringTranslate::getInstance();
	controlList.push_back(new GuiButton(0, width / 2 + 4,    height - 28, 150, 20, stringtranslate->translateKey("gui.done")));
	controlList.push_back(new GuiButton(1, width / 2 - 154,  height - 52, 100, 20, stringtranslate->translateKey("stat.generalButton")));
	GuiButton *guibutton;
	controlList.push_back(guibutton = new GuiButton(2, width / 2 - 46, height - 52, 100, 20, stringtranslate->translateKey("stat.blocksButton")));
	GuiButton *guibutton1;
	controlList.push_back(guibutton1 = new GuiButton(3, width / 2 + 62, height - 52, 100, 20, stringtranslate->translateKey("stat.itemsButton")));
	if (field_27157_n->getSize() == 0) guibutton->enabled  = false;
	if (field_27150_m->getSize() == 0) guibutton1->enabled = false;
}

void GuiStats::actionPerformed(GuiButton *guibutton)
{
	if (!guibutton->enabled) return;
	if (guibutton->id == 0)
	{
		mc->displayGuiScreen(field_27152_a);
	}
	else if (guibutton->id == 1)
	{
		field_27155_p = field_27151_l;
	}
	else if (guibutton->id == 3)
	{
		field_27155_p = field_27150_m;
	}
	else if (guibutton->id == 2)
	{
		field_27155_p = field_27157_n;
	}
	else
	{
		field_27155_p->actionPerformed(guibutton);
	}
}

void GuiStats::drawScreen(int_t i, int_t j, float_t f)
{
	field_27155_p->drawScreen(i, j, f);
	drawCenteredString(fontRenderer, field_27154_i, width / 2, 20, 0xffffff);
	GuiScreen::drawScreen(i, j, f);
}

void GuiStats::renderItemInSlot(int_t x, int_t y, int_t itemId)
{
	drawDefaultSlotIcon(x + 1, y + 1);
	if (!ItemStack::isValidItemID(itemId)) return;
	renderEnable(RenderCapability::RescaleNormal);
	renderPushMatrix();
	renderRotate(180.0f, 1.0f, 0.0f, 0.0f);
	RenderHelper::enableStandardItemLighting();
	renderPopMatrix();
	Item *item = Item::itemsList[itemId];
	field_27153_j->drawItemIntoGui(fontRenderer, mc->renderEngine, itemId, 0, item->getIconFromDamage(0), x + 2, y + 2);
	RenderHelper::disableStandardItemLighting();
	renderDisable(RenderCapability::RescaleNormal);
}

void GuiStats::drawDefaultSlotIcon(int_t x, int_t y)
{
	drawSlotIconInternal(x, y, 0, 0);
}

void GuiStats::drawSlotIconInternal(int_t x, int_t y, int_t texX, int_t texY)
{
	int_t i1 = mc->renderEngine->getTexture("/gui/slot.png");
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	mc->renderEngine->bindTexture(i1);
	Tessellator *tessellator = &Tessellator::instance;
	tessellator->startDrawingQuads();
	tessellator->addVertexWithUV(x + 0,  y + 18, zLevel, (float_t)(texX + 0)  * 0.0078125f, (float_t)(texY + 18) * 0.0078125f);
	tessellator->addVertexWithUV(x + 18, y + 18, zLevel, (float_t)(texX + 18) * 0.0078125f, (float_t)(texY + 18) * 0.0078125f);
	tessellator->addVertexWithUV(x + 18, y + 0,  zLevel, (float_t)(texX + 18) * 0.0078125f, (float_t)(texY + 0)  * 0.0078125f);
	tessellator->addVertexWithUV(x + 0,  y + 0,  zLevel, (float_t)(texX + 0)  * 0.0078125f, (float_t)(texY + 0)  * 0.0078125f);
	tessellator->draw();
}

void GuiStats::drawSlotIconAt(GuiStats *g, int_t x, int_t y, int_t texX, int_t texY)
{
	g->drawSlotIconInternal(x, y, texX, texY);
}

void GuiStats::drawItemIconAt(GuiStats *g, int_t x, int_t y, int_t itemId)
{
	g->renderItemInSlot(x, y, itemId);
}

void GuiStats::drawGradientRectStatic(GuiStats *g, int_t x1, int_t y1, int_t x2, int_t y2, int_t c1, int_t c2)
{
	g->drawGradientRect(x1, y1, x2, y2, c1, c2);
}
