#pragma once

#include "platform/RenderAPI.h"
#include <vector>
#include "java/Random.h"
#include <cstdint>
#include <chrono>

// Forward declarations - mismos nombres que en Java
class Minecraft;
class ItemRenderer;
class EntityLiving;
class MouseFilter;
class RenderGlobal;
class EffectRenderer;
class World;
class GameSettings;
class PlayerController;
class MovingObjectPosition;
class Vec3D;
class PlayerControllerTest;
class AxisAlignedBB;
class Entity;
class Material;
class EntityPlayer;
class Block;
class EntityPlayerSP;
class MouseHelper;
class ScaledResolution;
class GuiIngame;
class GuiScreen;
class GuiParticle;
class ChunkProviderLoadOrGenerate;
class ClippingHelperImpl;
class Frustrum;
class ICamera;
class RenderEngine;
class RenderHelper;
class InventoryPlayer;
class WorldChunkManager;
class BiomeGenBase;
class EntitySmokeFX;
class EntityRainFX;
class Tessellator;
class WorldProvider;
class IChunkProvider;

class EntityRenderer
{
public:
    static bool anaglyphEnabled;  // field_28135_a
    static int anaglyphField;

    EntityRenderer(Minecraft* minecraft);
    ~EntityRenderer();

    void updateRenderer();
    void getMouseOver(float partialTicks);
    void updateCameraAndRender(float partialTicks);
    // OptiFine: recalcula la tabla de brillo (lightBrightnessTable) del worldProvider
    // segun gameSettings.ofBrightness. Llamado al cambiar el slider y al cambiar de mundo.
    void updateWorldLightLevels();
    void renderWorld(float partialTicks, int64_t renderTimeLimitNano);
    void setupOverlayRendering();
    void disableLightmap(double partialTicks);
    void enableLightmap(double partialTicks);
    
    // Getters
    ItemRenderer* getItemRenderer() const { return itemRenderer; }

private:
    // Helper methods - renombradas de ofuscadas a legibles
    float getFOVModifier(float partialTicks, bool applyFovModifiers);
    void hurtCameraEffect(float partialTicks);
    void setupViewBobbing(float partialTicks);
    void orientCamera(float partialTicks);
    void setupCameraTransform(float partialTicks, int anaglyphPass);
    void renderHand(float partialTicks, int anaglyphPass);
    void addRainParticles();
    void renderRainSnow(float partialTicks);
    void updateFogColor(float partialTicks);
    void setupFog(int fogMode, float partialTicks);
    void updateTorchFlicker();
    void updateLightmap();
    
    // Utility
    void setupFogColorBuffer(float r, float g, float b, float a);
    
    // Renombrado de func_905_b -> setupOverlayRendering (establece proyeccion ortografica para GUI)
    // Ya esta publico arriba

    // GL_* constants come from glad.h (included via OpenGL.h) — no redefinition needed

    // Member variables - mismos nombres que Java donde son claros, renombrados donde son ofuscados
    Minecraft* mc;
    float farPlaneDistance;
    Entity* pointedEntity;  // Entidad apuntada por el cursor
    
    // Mouse smoothing filters.
    MouseFilter* mouseFilterXAxis;
    MouseFilter* mouseFilterYAxis;
    float smoothCamYaw;
    float smoothCamPitch;
    float smoothCamFilterX;
    float smoothCamFilterY;
    float smoothCamPartialTicks;
    
    // Camara tercera persona - renombrados de field_22228_r, etc.
    float cameraDistance;        // field_22228_r - distancia de camara en tercera persona
    float prevCameraDistance;    // field_22227_s
    double cameraYaw;            // Java 1.2.5: double
    double prevCameraYaw;        // Beta++ interpolation companion
    double cameraPitch;          // Java 1.2.5: double
    double prevCameraPitch;      // Beta++ interpolation companion
    float cameraRoll;            // field_22222_x
    float prevCameraRoll;        // field_22221_y
    float cameraRollTarget;      // field_22220_z
    float prevCameraRollTarget;  // field_22230_A
    
    bool cloudFog;
    
    // Zoom de camara
    bool zoomMode;
    double cameraZoom;
    double cameraYawOffset;
    double cameraPitchOffset;
    
    // Timing
    std::chrono::steady_clock::time_point prevFrameTime;
    int64_t field_28133_I;  // Tiempo de renderizado anterior en nanosegundos
    
    Random random;
    int rainSoundCounter;
    float rainXCoords[1024];
    float rainYCoords[1024];
    bool rainCoordsInitialized;
    
    // Buffer de color de niebla
    float fogColorBuffer[4];
    float fogColorRed;
    float fogColorGreen;
    float fogColorBlue;
    float fogColor2;  // Valor anterior de niebla
    float fogColor1;  // Valor actual de niebla
    float fovModifierHand;
    int lightmapTexture;
    std::vector<int_t> lightmapColors;
    bool lightmapUpdateNeeded;
    float torchFlickerX;
    float torchFlickerDX;
    float torchFlickerY;
    float torchFlickerDY;
    float fovModifierHandPrev;
    
    // Contador de actualizaciones del renderer
    int rendererUpdateCount;
    int debugViewDirection;
    
    // Renderer de items en mano
    ItemRenderer* itemRenderer;

    // OptiFine: ultimo worldProvider para el que se aplico el brillo; cuando cambia
    // (p.ej. cambio de dimension) se vuelve a llamar a updateWorldLightLevels().
    WorldProvider* updatedWorldProvider;
};
