#include "LegacyMainMenuLayout.h"

#include <algorithm>

#include "LegacySceneLayout.h"
#include "LegacyMenuHints.h"

namespace
{
// Smallest button the 8 px font still sits comfortably inside. The column budget
// below never shrinks past it; it just starts overflowing the budget instead.
constexpr int_t LEGACY_MENU_MIN_BUTTON_HEIGHT = 16;
}

int_t legacyMainMenuButtonCount(bool hideQuitButton)
{
    return hideQuitButton ? 6 : 7;
}

LegacyMainMenuLayout legacyMainMenuLayout(int_t screenWidth, int_t screenHeight, int_t buttonCount)
{
    LegacyMainMenuLayout layout{};
    // Legacy keeps the menu column near half the screen width and the button height
    // near a twentieth of the screen. Both used to be absolute, which left a 240 px
    // console with buttons 68 % as wide as the screen running down to 90 % of it.
    layout.buttonWidth = std::min<int_t>(232,
        std::max<int_t>(120, std::min<int_t>(screenWidth - 16, screenWidth * 54 / 100)));
    layout.buttonHeight = legacyScaleToScreen(screenHeight, 22, LEGACY_MENU_MIN_BUTTON_HEIGHT, 22);
    // Legacy separates its buttons by about a quarter of their height; anything
    // wider opens the column up more than the reference does at desktop sizes.
    layout.buttonSpacing = std::max<int_t>(2, layout.buttonHeight / 4);
    layout.buttonX = (screenWidth - layout.buttonWidth) / 2;

    const int_t safeButtonCount = std::max<int_t>(1, buttonCount);

    // Legacy's column takes a bit over a third of the screen. Six buttons at the
    // desktop size do not fit that on a 240 px console, so the column is held to a
    // budget: the spacing gives way first and the height only after it, which keeps
    // low resolutions as large as they can be instead of running off the bottom.
    const int_t columnBudget = screenHeight * 46 / 100;
    if (safeButtonCount > 1)
    {
        const int_t spacingRoom = (columnBudget - safeButtonCount * layout.buttonHeight) /
            (safeButtonCount - 1);
        layout.buttonSpacing = std::max<int_t>(2, std::min(layout.buttonSpacing, spacingRoom));
    }
    const int_t heightRoom = (columnBudget - (safeButtonCount - 1) * layout.buttonSpacing) /
        safeButtonCount;
    layout.buttonHeight = std::max<int_t>(LEGACY_MENU_MIN_BUTTON_HEIGHT,
        std::min(layout.buttonHeight, heightRoom));

    const int_t menuHeight = safeButtonCount * layout.buttonHeight +
        (safeButtonCount - 1) * layout.buttonSpacing;
    const int_t bottomMargin = screenHeight >= 200 ? 8 : 4;
    const int_t maximumFirstY = std::max<int_t>(4, screenHeight - bottomMargin - menuHeight);
    const LegacySceneLayout scene = legacySceneLayout(screenWidth, screenHeight);
    // On short windows, centering against the whole screen places the column too
    // close to the logo because the bottom hint row is not part of the usable menu
    // area. Centre the buttons inside the band between the title and the footer.
    // This keeps 480p/500p layouts lower while preserving the original placement
    // on taller desktop windows.
    const int_t classicFirstY = std::max<int_t>(scene.contentTop,
        screenHeight / 2 - menuHeight / 2 + legacyScaleToScreen(screenHeight, 24, 8, 24));

    const int_t footerGap = legacyScaleToScreen(screenHeight, 18, 8, 18);
    const int_t usableBottom = std::max<int_t>(scene.contentTop + menuHeight,
        legacyHintRowY(screenHeight) - footerGap);
    const int_t lowResFirstY = scene.contentTop +
        std::max<int_t>(0, (usableBottom - scene.contentTop - menuHeight) / 2);

    int_t preferredFirstY = classicFirstY;
    if (screenHeight <= 540)
    {
        preferredFirstY = lowResFirstY;
    }
    else if (screenHeight < 640)
    {
        // Blend over 100 px so resizing the window does not make the menu jump.
        const int_t lowWeight = 640 - screenHeight;
        preferredFirstY = (classicFirstY * (100 - lowWeight) + lowResFirstY * lowWeight) / 100;
    }

    layout.firstButtonY = std::max<int_t>(4, std::min(preferredFirstY, maximumFirstY));

    layout.titleY = scene.titleY;
    layout.titleMaxWidth = scene.titleMaxWidth;
    layout.titleMaxHeight = scene.titleMaxHeight;
    return layout;
}

LegacyUiRect legacyFitTitleRect(int_t screenWidth, int_t y, int_t maxWidth, int_t maxHeight,
    int_t textureWidth, int_t textureHeight)
{
    LegacyUiRect rect{};
    rect.y = y;
    if (textureWidth <= 0 || textureHeight <= 0 || maxWidth <= 0 || maxHeight <= 0)
        return rect;

    const double widthScale = static_cast<double>(maxWidth) / static_cast<double>(textureWidth);
    const double heightScale = static_cast<double>(maxHeight) / static_cast<double>(textureHeight);
    const double scale = std::min(widthScale, heightScale);
    rect.width = std::max<int_t>(1, static_cast<int_t>(static_cast<double>(textureWidth) * scale));
    rect.height = std::max<int_t>(1, static_cast<int_t>(static_cast<double>(textureHeight) * scale));
    rect.x = (screenWidth - rect.width) / 2;
    return rect;
}
