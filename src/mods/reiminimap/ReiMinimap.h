#pragma once

#include "java/Type.h"
#include "Waypoint.h"
#include <vector>
#include <string>
#include <memory>

class Minecraft;
class GuiIngame;
class BufferedImage;

class ReiMinimap
{
public:
    static ReiMinimap &getInstance();

    void init(Minecraft *mc);
    void update();
    void render(GuiIngame *gui, int_t screenWidth, int_t screenHeight, float_t partialTick);

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool en) { m_enabled = en; }

    void addWaypoint(const std::string &name, int_t x, int_t y, int_t z, int_t color = 0x00FF00);
    const std::vector<Waypoint> &getWaypoints() const { return m_waypoints; }

    void saveWaypoints();
    void loadWaypoints();

private:
    ReiMinimap();
    ~ReiMinimap();
    ReiMinimap(const ReiMinimap &) = delete;
    ReiMinimap &operator=(const ReiMinimap &) = delete;

    void updateMapTexture();
    int_t getBlockColor(int_t blockId, int_t height, int_t northHeight);
    std::string getWaypointsFilePath() const;

    Minecraft *m_mc;
    bool m_enabled;
    int_t m_mapTextureId;
    std::unique_ptr<BufferedImage> m_mapImage;
    std::vector<unsigned char> m_pixelData;

    int_t m_lastPlayerX;
    int_t m_lastPlayerZ;
    int_t m_updateTicks;

    std::vector<Waypoint> m_waypoints;
    bool m_initialized;
    bool m_waypointComboWasPressed;
    std::string m_toastMessage;
    int_t m_toastTimer;
};
