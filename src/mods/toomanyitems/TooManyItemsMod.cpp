#include "TooManyItemsMod.h"
#include "TooManyItems.h"

TooManyItemsMod::TooManyItemsMod()
    : m_enabled(false)
{
}

void TooManyItemsMod::onInit(Minecraft *mc)
{
    TooManyItems::init();
}

void TooManyItemsMod::onDrawContainer(GuiContainer *container, int_t mouseX, int_t mouseY)
{
    if (!m_enabled)
        return;
    TooManyItems::draw(container, mouseX, mouseY);
}

bool TooManyItemsMod::onContainerMouseClicked(GuiContainer *container, int_t x, int_t y, int_t button)
{
    if (!m_enabled)
        return false;
    return TooManyItems::mouseClicked(container, x, y, button);
}

bool TooManyItemsMod::onContainerKeyTyped(char_t c, int_t key)
{
    if (!m_enabled)
        return false;
    return TooManyItems::keyTyped(c, key);
}
