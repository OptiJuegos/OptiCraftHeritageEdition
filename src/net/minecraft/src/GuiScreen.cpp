#include "GuiScreen.h"
#include "SoundManager.h"
#include "GuiButton.h"
#include "GuiParticle.h"
#include "Tessellator.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "platform/RenderAPI.h"
#include "pc/lwjgl/Keyboard.h"
#include "pc/lwjgl/Mouse.h"
#include "platform/PlatformTuning.h"
#include "platform/Input.h"
#if !PLATFORM_PS2 && !PLATFORM_WII
#include "SDL_clipboard.h"
#endif
#if PLATFORM_PS2 || PLATFORM_WII
#include "VirtualKeyboard.h"
#include "ContainerSlotNavigator.h"
#endif

// Minecraft forward-included via header chain
class Minecraft;
#include "Minecraft.h"

#if PLATFORM_SOFTWARE_CURSOR
namespace {

#if PLATFORM_CURSOR_TEXTURE
// The pointer art is a 32x32 crosshair whose arms cross at the exact centre of
// the image, so the hotspot -- the pixel the GUI code treats as "the mouse" --
// is the middle, not the top-left corner the old hardcoded arrow used. Offset
// the quad by half its size to keep (mouseX, mouseY) under the crossing point.
void drawCursorTexture(Minecraft *mc, float_t zLevel, int_t mouseX, int_t mouseY)
{
	const int_t size = PLATFORM_CURSOR_SIZE;
	const int_t x = mouseX - size / 2;
	const int_t y = mouseY - size / 2;

	// Same GL footprint as Gui::drawRect (the calls this replaces): blending on
	// for the draw, off again afterwards, texturing left enabled. Depth is not
	// touched -- every caller already has the depth test disabled by the time it
	// reaches here, and re-enabling it would break the screens that do not.
	renderEnable(RenderCapability::Blend);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
	renderEnable(RenderCapability::Texture2D);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	mc->renderEngine->bindTexture(mc->renderEngine->getTexture("/cursor.png"));

	// Not Gui::drawTexturedModalRect: that one hardcodes a 1/256 atlas step, and
	// cursor.png is a standalone sheet drawn whole, so the UVs are just 0..1.
	Tessellator *tess = &Tessellator::instance;
	tess->startDrawingQuads();
	tess->addVertexWithUV(x + 0,    y + size, zLevel, 0.0, 1.0);
	tess->addVertexWithUV(x + size, y + size, zLevel, 1.0, 1.0);
	tess->addVertexWithUV(x + size, y + 0,    zLevel, 1.0, 0.0);
	tess->addVertexWithUV(x + 0,    y + 0,    zLevel, 0.0, 0.0);
	tess->draw();

	renderDisable(RenderCapability::Blend);
}
#endif // PLATFORM_CURSOR_TEXTURE

} // namespace
#endif // PLATFORM_SOFTWARE_CURSOR

GuiScreen::GuiScreen()
	: mc(nullptr)
	, width(0)
	, height(0)
	, field_948_f(false)
	, fontRenderer(nullptr)
	, guiParticles(nullptr)
	, selectedButton(nullptr)
{
}

GuiScreen::~GuiScreen()
{
	// Buttons and the particle system are owned solely by this screen.
	for (GuiButton *btn : controlList)
		delete btn;
	controlList.clear();
	delete guiParticles;
	guiParticles = nullptr;
}

void GuiScreen::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	for (int_t i = 0; i < (int_t)controlList.size(); i++)
	{
		controlList[i]->drawButton(mc, mouseX, mouseY);
	}

#if PLATFORM_SOFTWARE_CURSOR
	// No OS cursor on console. Draw a software pointer so the existing
	// mouse-hover/click GUI code can be used with the pad / IR pointer.
	(void)partialTick;
#if PLATFORM_CURSOR_TEXTURE
	drawCursorTexture(mc, zLevel, mouseX, mouseY);
#else
	// Vector fallback: an arrow whose tip is the hotspot at (mouseX, mouseY).
	drawRect(mouseX + 0, mouseY + 0, mouseX + 2,  mouseY + 14, 0xff000000);
	drawRect(mouseX + 2, mouseY + 2, mouseX + 4,  mouseY + 12, 0xff000000);
	drawRect(mouseX + 4, mouseY + 4, mouseX + 6,  mouseY + 10, 0xff000000);
	drawRect(mouseX + 1, mouseY + 1, mouseX + 2,  mouseY + 12, 0xffffffff);
	drawRect(mouseX + 2, mouseY + 3, mouseX + 3,  mouseY + 10, 0xffffffff);
	drawRect(mouseX + 3, mouseY + 5, mouseX + 4,  mouseY + 8,  0xffffffff);
#endif
#endif
}

void GuiScreen::keyTyped(char_t c, int_t key)
{
	if (key == 1)
	{
		mc->displayGuiScreen(nullptr);
		mc->setIngameFocus();
	}
}

jstring GuiScreen::getClipboardString()
{
	// SDL clipboard
#if !PLATFORM_PS2 && !PLATFORM_WII
	char *text = SDL_GetClipboardText();
	if (text)
	{
		std::string s(text);
		SDL_free(text);
		return s;
	}
#endif
	return jstring(nullptr);
}

void GuiScreen::setClipboardString(const std::string &text)
{
#if !PLATFORM_PS2 && !PLATFORM_WII
	SDL_SetClipboardText(text.c_str());
#else
	(void)text;
#endif
}

bool GuiScreen::isCtrlKeyDown()
{
	return lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LCONTROL) ||
	       lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RCONTROL);
}

bool GuiScreen::isShiftKeyDown()
{
	return lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LSHIFT) ||
	       lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RSHIFT);
}

void GuiScreen::mouseClicked(int_t x, int_t y, int_t button)
{
	if (button == 0)
	{
		for (int_t i = 0; i < (int_t)controlList.size(); i++)
		{
			GuiButton *btn = controlList[i];
			if (btn->mousePressed(mc, x, y))
			{
				selectedButton = btn;
				mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
				actionPerformed(btn);

				// An action is allowed to rebuild this very screen (video/aspect
				// options call setWorldAndResolution) or replace currentScreen.
				// Continuing the loop would then iterate a different controlList
				// during the same physical click. One mouse-down can activate at
				// most one widget, matching the Java/LWJGL behaviour we want here.
				return;
			}
		}
	}
}

void GuiScreen::mouseMovedOrUp(int_t x, int_t y, int_t button)
{
	if (selectedButton != nullptr && button == 0)
	{
		selectedButton->mouseReleased(x, y);
		selectedButton = nullptr;
	}
}

void GuiScreen::actionPerformed(GuiButton *button)
{
}

void GuiScreen::setWorldAndResolution(Minecraft *minecraft, int_t w, int_t h)
{
	// Called again on every resize; free the previous frame's buttons/particles
	// instead of leaking them (Java relied on GC).
	delete guiParticles;
	guiParticles = new GuiParticle(minecraft);
	mc = minecraft;
	fontRenderer = minecraft->fontRenderer;
	width = w;
	height = h;
	// selectedButton siempre apunta a un boton de controlList; al destruirlos quedaria
	// colgante y mouseMovedOrUp haria use-after-free (crash al soltar el raton tras
	// una accion que reconstruye la pantalla, p.ej. togglear una opcion). Anularlo.
	selectedButton = nullptr;
	for (GuiButton *btn : controlList)
		delete btn;
	controlList.clear();
	initGui();
}

void GuiScreen::initGui()
{
}

void GuiScreen::handleInput()
{
#if PLATFORM_PS2 || PLATFORM_WII
	// Console GUI helpers consume the platform snapshot here, after the native
	// backend has published this frame's controller state and before queued
	// mouse/keyboard events are dispatched to the screen. Keeping this routing
	// in shared GUI code prevents Wii/PS2 input backends from depending on
	// Minecraft screen classes.
	VirtualKeyboard::instance().tick();
	if (!platformTextInputExclusive())
		ContainerSlotNavigator::instance().tick();
#endif
	while (lwjgl::Mouse::next()) handleMouseInput();
	while (lwjgl::Keyboard::next()) handleKeyboardInput();
}

void GuiScreen::handleMouseInput()
{
	if (lwjgl::Mouse::getEventButtonState())
	{
		int_t x = (lwjgl::Mouse::getEventX() * width) / mc->displayWidth;
		int_t y = height - (lwjgl::Mouse::getEventY() * height) / mc->displayHeight - 1;
		mouseClicked(x, y, lwjgl::Mouse::getEventButton());
	}
	else
	{
		int_t x = (lwjgl::Mouse::getEventX() * width) / mc->displayWidth;
		int_t y = height - (lwjgl::Mouse::getEventY() * height) / mc->displayHeight - 1;
		mouseMovedOrUp(x, y, lwjgl::Mouse::getEventButton());
	}
}

void GuiScreen::handleKeyboardInput()
{
	if (lwjgl::Keyboard::getEventKeyState())
	{
		if (lwjgl::Keyboard::getEventKey() == 87)
		{
			mc->toggleFullscreen();
			return;
		}
		keyTyped(lwjgl::Keyboard::getEventCharacter(), lwjgl::Keyboard::getEventKey());
	}
}

void GuiScreen::updateScreen()
{
}

void GuiScreen::onGuiClosed()
{
}

void GuiScreen::drawDefaultBackground()
{
	drawWorldBackground(0);
}

void GuiScreen::drawWorldBackground(int_t ticks)
{
	if (mc->theWorld != nullptr)
	{
		drawGradientRect(0, 0, width, height, 0xc0101010, 0xd0101010);
	}
	else
	{
		drawBackground(ticks);
	}
}

void GuiScreen::drawBackground(int_t ticks)
{
	renderDisable(RenderCapability::Lighting);
	renderDisable(RenderCapability::Fog);
	Tessellator *tess = &Tessellator::instance;
	renderBindTexture(mc->renderEngine->getTexture("/gui/background.png"));
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	float_t f = 32.0f;
	tess->startDrawingQuads();
	tess->setColorOpaque_I(0x404040);
	tess->addVertexWithUV(0,     height, 0.0, 0.0,              (float_t)height / f + (float_t)ticks);
	tess->addVertexWithUV(width, height, 0.0, (float_t)width / f, (float_t)height / f + (float_t)ticks);
	tess->addVertexWithUV(width, 0,      0.0, (float_t)width / f, 0 + ticks);
	tess->addVertexWithUV(0,     0,      0.0, 0.0,              0 + ticks);
	tess->draw();
}

bool GuiScreen::doesGuiPauseGame()
{
	return true;
}

void GuiScreen::deleteWorld(bool confirmed, int_t worldNum)
{
}

void GuiScreen::confirmClicked(bool confirmed, int_t id)
{
	deleteWorld(confirmed, id);
}

void GuiScreen::selectNextField()
{
}
