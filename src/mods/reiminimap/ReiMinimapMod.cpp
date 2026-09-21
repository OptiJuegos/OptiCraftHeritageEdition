#include "ReiMinimapMod.h"
#include "ReiMinimap.h"

ReiMinimapMod::ReiMinimapMod()
    : m_enabled(false)
{
}

bool ReiMinimapMod::isEnabled() const
{
    return m_enabled;
}

void ReiMinimapMod::setEnabled(bool state)
{
    m_enabled = state;
    ReiMinimap::getInstance().setEnabled(state);
}

void ReiMinimapMod::onInit(Minecraft *mc)
{
    ReiMinimap::getInstance().init(mc);
    ReiMinimap::getInstance().setEnabled(m_enabled);
}

void ReiMinimapMod::onTick()
{
    if (!m_enabled)
        return;
    ReiMinimap::getInstance().update();
}

void ReiMinimapMod::onRenderGameOverlay(GuiIngame *gui, int_t screenWidth, int_t screenHeight, float_t partialTick)
{
    if (!m_enabled)
        return;
    ReiMinimap::getInstance().render(gui, screenWidth, screenHeight, partialTick);
}
