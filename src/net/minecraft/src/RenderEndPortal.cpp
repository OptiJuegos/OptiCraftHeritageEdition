#include "RenderEndPortal.h"

#include "ActiveRenderInfo.h"
#include "Tessellator.h"
#include "TileEntityEndPortal.h"
#include "TileEntityRenderer.h"
#include "java/Random.h"
#include "java/System.h"
#include "platform/RenderAPI.h"

#include <cmath>

#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
#include <glad/glad.h>
#endif

void RenderEndPortal::renderEndPortal(TileEntityEndPortal *, double x, double y, double z, float partialTick)
{
    (void)partialTick;
    if (tileEntityRenderer == nullptr)
        return;

#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
    const float playerX = static_cast<float>(tileEntityRenderer->playerX);
    const float playerY = static_cast<float>(tileEntityRenderer->playerY);
    const float playerZ = static_cast<float>(tileEntityRenderer->playerZ);
    const float portalY = 12.0f / 16.0f;
    Random random(31100LL);

    glDisable(GL_LIGHTING);

    for (int layer = 0; layer < 16; ++layer)
    {
        glPushMatrix();
        float depth = static_cast<float>(16 - layer);
        float textureScale = 1.0f / 16.0f;
        float brightness = 1.0f / (depth + 1.0f);

        if (layer == 0)
        {
            bindTextureByName("/misc/tunnel.png");
            brightness = 0.1f;
            depth = 65.0f;
            textureScale = 2.0f / 16.0f;
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else if (layer == 1)
        {
            bindTextureByName("/misc/particlefield.png");
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            textureScale = 0.5f;
        }

        const float relativeY = static_cast<float>(-(y + static_cast<double>(portalY)));
        const float eyeBottom = relativeY + ActiveRenderInfo::objectY;
        const float eyeTop = relativeY + depth + ActiveRenderInfo::objectY;
        float projectedY = eyeBottom / eyeTop;
        projectedY += static_cast<float>(y + static_cast<double>(portalY));
        glTranslatef(playerX, projectedY, playerZ);

        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

        const GLfloat planeS[4] = {1.0f, 0.0f, 0.0f, 0.0f};
        const GLfloat planeT[4] = {0.0f, 0.0f, 1.0f, 0.0f};
        const GLfloat planeR[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        const GLfloat planeQ[4] = {0.0f, 1.0f, 0.0f, 0.0f};
        glTexGenfv(GL_S, GL_OBJECT_PLANE, planeS);
        glTexGenfv(GL_T, GL_OBJECT_PLANE, planeT);
        glTexGenfv(GL_R, GL_OBJECT_PLANE, planeR);
        glTexGenfv(GL_Q, GL_EYE_PLANE, planeQ);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glEnable(GL_TEXTURE_GEN_R);
        glEnable(GL_TEXTURE_GEN_Q);
        glPopMatrix();

        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.0f, static_cast<float>(System::currentTimeMillis() % 700000LL) / 700000.0f, 0.0f);
        glScalef(textureScale, textureScale, textureScale);
        glTranslatef(0.5f, 0.5f, 0.0f);
        glRotatef(static_cast<float>(layer * layer * 4321 + layer * 9) * 2.0f, 0.0f, 0.0f, 1.0f);
        glTranslatef(-0.5f, -0.5f, 0.0f);
        glTranslatef(-playerX, -playerZ, -playerY);
        glTranslatef(ActiveRenderInfo::objectX * depth / eyeBottom,
                     ActiveRenderInfo::objectZ * depth / eyeBottom,
                     -playerY);

        Tessellator &tessellator = Tessellator::instance;
        tessellator.startDrawingQuads();
        float red = random.nextFloat() * 0.5f + 0.1f;
        float green = random.nextFloat() * 0.5f + 0.4f;
        float blue = random.nextFloat() * 0.5f + 0.5f;
        if (layer == 0)
            red = green = blue = 1.0f;
        tessellator.setColorRGBA_F(red * brightness, green * brightness, blue * brightness, 1.0f);
        tessellator.addVertex(x, y + static_cast<double>(portalY), z);
        tessellator.addVertex(x, y + static_cast<double>(portalY), z + 1.0);
        tessellator.addVertex(x + 1.0, y + static_cast<double>(portalY), z + 1.0);
        tessellator.addVertex(x + 1.0, y + static_cast<double>(portalY), z);
        tessellator.draw();

        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);
    glEnable(GL_LIGHTING);
#else
    renderConsoleLayers(x, y, z, partialTick);
#endif
}

void RenderEndPortal::renderConsoleLayers(double x, double y, double z, float partialTick)
{
    (void)partialTick;
    const float portalY = 12.0f / 16.0f;
    const tess_coord_t renderX = static_cast<tess_coord_t>(x);
    const tess_coord_t renderY = static_cast<tess_coord_t>(y);
    const tess_coord_t renderZ = static_cast<tess_coord_t>(z);
    const tess_coord_t renderPortalY = renderY + static_cast<tess_coord_t>(portalY);
    const float time = static_cast<float>(System::currentTimeMillis() % 700000LL) / 700000.0f;
    Random random(31100LL);

    renderDisable(RenderCapability::Lighting);
    renderEnable(RenderCapability::Blend);

    for (int layer = 0; layer < 16; ++layer)
    {
        float depth = static_cast<float>(16 - layer);
        float textureScale = 1.0f / 16.0f;
        float brightness = 1.0f / (depth + 1.0f);

        if (layer == 0)
        {
            bindTextureByName("/misc/tunnel.png");
            brightness = 0.1f;
            textureScale = 2.0f / 16.0f;
            renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
        }
        else
        {
            if (layer == 1)
                bindTextureByName("/misc/particlefield.png");
            renderBlendFunc(RenderBlendFactor::One, RenderBlendFactor::One);
            if (layer == 1)
                textureScale = 0.5f;
        }

        const float angle = static_cast<float>(layer * layer * 4321 + layer * 9) * 2.0f * 3.14159265358979323846f / 180.0f;
        const float cosine = std::cos(angle);
        const float sine = std::sin(angle);
        auto uv = [&](float localX, float localZ, float &u, float &v)
        {
            float tx = localX - 0.5f;
            float tz = localZ - 0.5f;
            const float rx = tx * cosine - tz * sine;
            const float rz = tx * sine + tz * cosine;
            u = (rx + 0.5f) * textureScale;
            v = (rz + 0.5f) * textureScale + time;
        };

        float red = random.nextFloat() * 0.5f + 0.1f;
        float green = random.nextFloat() * 0.5f + 0.4f;
        float blue = random.nextFloat() * 0.5f + 0.5f;
        if (layer == 0)
            red = green = blue = 1.0f;

        float u0, v0, u1, v1, u2, v2, u3, v3;
        uv(0.0f, 0.0f, u0, v0);
        uv(0.0f, 1.0f, u1, v1);
        uv(1.0f, 1.0f, u2, v2);
        uv(1.0f, 0.0f, u3, v3);

        Tessellator &tessellator = Tessellator::instance;
        tessellator.startDrawingQuads();
        tessellator.setColorRGBA_F(red * brightness, green * brightness, blue * brightness, 1.0f);
        tessellator.addVertexWithUV(renderX, renderPortalY, renderZ, u0, v0);
        tessellator.addVertexWithUV(renderX, renderPortalY, renderZ + static_cast<tess_coord_t>(1.0), u1, v1);
        tessellator.addVertexWithUV(renderX + static_cast<tess_coord_t>(1.0), renderPortalY, renderZ + static_cast<tess_coord_t>(1.0), u2, v2);
        tessellator.addVertexWithUV(renderX + static_cast<tess_coord_t>(1.0), renderPortalY, renderZ, u3, v3);
        tessellator.draw();
    }

    renderDisable(RenderCapability::Blend);
    renderEnable(RenderCapability::Lighting);
}

void RenderEndPortal::renderTileEntityAt(TileEntity *tileentity, double x, double y, double z, float partialTick)
{
    renderEndPortal(static_cast<TileEntityEndPortal *>(tileentity), x, y, z, partialTick);
}
