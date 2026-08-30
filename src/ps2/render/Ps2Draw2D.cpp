#if defined(PS2_PLATFORM)

#include "ps2/render/Ps2Draw2D.h"
#include "ps2/render/Ps2ClipGuard.h"

#include <gsInline.h>
#include <gsPrimitive.h>
#include <math.h>

namespace
{
    const int PS2_DRAW2D_BATCH_TRIS = 32;
    // A text string is a few dozen glyphs, and each glyph is one sprite, so a
    // whole line of text usually leaves in a single packet.
    const int PS2_DRAW2D_BATCH_SPRITES = 64;

    struct ArrayCursor {
        const char* base;
        int stride;
    };

    static inline ArrayCursor makeCursor(const void* base, int stride, int packedStride)
    {
        ArrayCursor cursor;
        cursor.base = static_cast<const char*>(base);
        cursor.stride = stride ? stride : packedStride;
        return cursor;
    }

    static inline const float* floatAt(const ArrayCursor& cursor, int index)
    {
        return reinterpret_cast<const float*>(cursor.base + index * cursor.stride);
    }

    static inline unsigned char clampColor(float value)
    {
        if (value <= 0.0f) return 0;
        if (value >= 1.0f) return 255;
        return static_cast<unsigned char>(value * 255.0f);
    }

    static inline void transformPoint(const float* matrix,
                                      float x, float y, float z,
                                      float& outX, float& outY, float& outZ, float& outW)
    {
        outX = matrix[0] * x + matrix[4] * y + matrix[8]  * z + matrix[12];
        outY = matrix[1] * x + matrix[5] * y + matrix[9]  * z + matrix[13];
        outZ = matrix[2] * x + matrix[6] * y + matrix[10] * z + matrix[14];
        outW = matrix[3] * x + matrix[7] * y + matrix[11] * z + matrix[15];
    }

    static inline int mapDepth(float eyeZ, const Ps2Draw2DState& state)
    {
        if (!state.depthTest) {
#ifdef PS2_RENDER_STATS
            if (state.debugDepthPinned) (*state.debugDepthPinned)++;
#endif
            return state.depthMax;
        }

        const float range = state.farPlane - state.nearPlane;
        if (range == 0.0f) {
#ifdef PS2_RENDER_STATS
            if (state.debugDepthPinned) (*state.debugDepthPinned)++;
#endif
            return state.depthMax;
        }

        const float zNdc = (-2.0f * eyeZ - (state.farPlane + state.nearPlane)) / range;
        float depth = (1.0f - zNdc) * 0.5f;
        if (depth < 0.0f) depth = 0.0f;
        if (depth > 1.0f) depth = 1.0f;
        const int z = static_cast<int>(depth * static_cast<float>(state.depthMax));

#ifdef PS2_RENDER_STATS
        if (state.debugDepthTested) (*state.debugDepthTested)++;
        if (state.debugDepthLow && z < *state.debugDepthLow) *state.debugDepthLow = z;
        if (state.debugDepthHigh && z > *state.debugDepthHigh) *state.debugDepthHigh = z;
        if (state.debugEyeZLow && eyeZ < *state.debugEyeZLow) *state.debugEyeZLow = eyeZ;
        if (state.debugEyeZHigh && eyeZ > *state.debugEyeZHigh) *state.debugEyeZHigh = eyeZ;
        if (state.debugNear) *state.debugNear = state.nearPlane;
        if (state.debugFar) *state.debugFar = state.farPlane;
#endif
        return z;
    }

    static inline u64 makeColor(unsigned char r, unsigned char g,
                                unsigned char b, unsigned char a)
    {
        return GS_SETREG_RGBAQ(r, g, b, static_cast<unsigned char>(a >> 1), 0);
    }
}

bool ps2_draw_2d_mesh(const Ps2NativeMeshView& mesh, const Ps2Draw2DState& state)
{
    if (!state.gsGlobal || !state.modelView || !mesh.vertices || mesh.count < 1)
        return false;
    if (mesh.drawMode != PS2_NATIVE_PRIM_QUADS &&
        mesh.drawMode != PS2_NATIVE_PRIM_TRIANGLES &&
        mesh.drawMode != PS2_NATIVE_PRIM_TRIANGLE_STRIP)
        return false;
    if (mesh.vertexSize < 2)
        return false;
    if (mesh.colorEnabled && !mesh.colors)
        return false;
    if (mesh.texCoordEnabled && !mesh.texCoords)
        return false;

    const ArrayCursor vertices = makeCursor(mesh.vertices, mesh.vertexStride,
                                             mesh.vertexSize * static_cast<int>(sizeof(float)));
    const ArrayCursor texCoords = makeCursor(mesh.texCoords, mesh.texCoordStride,
                                              2 * static_cast<int>(sizeof(float)));
    const ArrayCursor colors = makeCursor(mesh.colors, mesh.colorStride,
                                           mesh.colorSize * (mesh.colorFloat ? static_cast<int>(sizeof(float)) : 1));

    const float texW = state.texture ? static_cast<float>(state.texture->Width) : 1.0f;
    const float texH = state.texture ? static_cast<float>(state.texture->Height) : 1.0f;

    auto fetchColor = [&](int index,
                          unsigned char& r, unsigned char& g,
                          unsigned char& b, unsigned char& a) {
        if (!mesh.colorEnabled || !mesh.colors) {
            r = state.render.flatR;
            g = state.render.flatG;
            b = state.render.flatB;
            a = state.render.flatA;
        } else if (mesh.colorFloat) {
            const float* color = floatAt(colors, index);
            r = clampColor(color[0]);
            g = clampColor(color[1]);
            b = clampColor(color[2]);
            a = mesh.colorSize >= 4 ? clampColor(color[3]) : 255;
        } else {
            const unsigned char* color = reinterpret_cast<const unsigned char*>(colors.base + index * colors.stride);
            r = color[0];
            g = color[1];
            b = color[2];
            a = mesh.colorSize >= 4 ? color[3] : 255;
        }

        if (state.render.lightVertex)
            state.render.lightVertex(index, r, g, b);
    };

    auto project = [&](int index, float& screenX, float& screenY, int& screenZ) {
        const float* vertex = floatAt(vertices, index);
        const float z = mesh.vertexSize >= 3 ? vertex[2] : 0.0f;
        float x, y, eyeZ, w;
        transformPoint(state.modelView, vertex[0], vertex[1], z, x, y, eyeZ, w);
        if (fabsf(w) > 1e-6f) {
            x /= w;
            y /= w;
            eyeZ /= w;
        }

        screenX = (x - state.left) / (state.right - state.left) * state.viewW;
        screenY = (1.0f - (y - state.bottom) / (state.top - state.bottom)) * state.viewH;
        screenZ = mapDepth(eyeZ, state);
    };

    static GSPRIMSTQPOINT batch[PS2_DRAW2D_BATCH_TRIS * 3] __attribute__((aligned(16)));
    static GSPRIMUVPOINT spriteBatch[PS2_DRAW2D_BATCH_SPRITES * 2] __attribute__((aligned(16)));
    int batchTriangles = 0;
    int batchSprites = 0;
    bool clampValid = false;
    Ps2ClampSel lastClamp = { -1, -1, -1 };

    auto flushBatch = [&]() {
        if (batchTriangles <= 0)
            return;
        if (state.queueGuard)
            state.queueGuard(batchTriangles * 3);
        gsKit_prim_list_triangle_goraud_texture_stq_3d(
            state.gsGlobal, state.texture, batchTriangles * 3, batch);
        batchTriangles = 0;
    };

    auto flushSprites = [&]() {
        if (batchSprites <= 0)
            return;
        if (state.queueGuard)
            state.queueGuard(batchSprites * 2);
        gskit_prim_list_sprite_texture_uv_3d(
            state.gsGlobal, state.texture, batchSprites * 2, spriteBatch);
        batchSprites = 0;
    };

    // A sprite and a triangle carry different PRIM values, so they can never
    // share a packet. Each staging point flushes the other kind first, which
    // keeps at most one batch non-empty and -- more importantly -- preserves
    // submission order. The GUI draws back to front with no depth test, so
    // reordering two overlapping quads is a visible bug, not a detail.
    auto flushPending = [&]() {
        flushBatch();
        flushSprites();
    };

    // Clamp selection is shared: it decides CLAMP vs REGION_REPEAT from the UV
    // bounds of the primitive, and gsKit_set_clamp goes straight into the DMA
    // stream, so anything already staged must go out under the clamp it was
    // built with.
    auto ensureClamp = [&](float minU, float minV, float maxU, float maxV) {
        const Ps2ClampSel clamp = ps2_select_clamp(texW, texH, true, false,
                                                    minU, minV, maxU, maxV);
        if (clampValid && clamp == lastClamp)
            return;
        flushPending();
        if (state.applyClampSel)
            state.applyClampSel(clamp.mode, clamp.ufix, clamp.vfix);
        lastClamp = clamp;
        clampValid = true;
    };

    auto emitTriangle = [&](int i0, int i1, int i2) {
        float x0, y0, x1, y1, x2, y2;
        int z0, z1, z2;
        project(i0, x0, y0, z0);
        project(i1, x1, y1, z1);
        project(i2, x2, y2, z2);

        unsigned char r0, g0, b0, a0;
        unsigned char r1, g1, b1, a1;
        unsigned char r2, g2, b2, a2;
        fetchColor(i0, r0, g0, b0, a0);
        fetchColor(i1, r1, g1, b1, a1);
        fetchColor(i2, r2, g2, b2, a2);
        if (!state.render.smoothShading) {
            r0 = r1 = r2; g0 = g1 = g2; b0 = b1 = b2; a0 = a1 = a2;
        }

        if (!state.texture || !mesh.texCoordEnabled) {
            flushSprites();
            if (state.queueGuard)
                state.queueGuard(3);
            gsKit_prim_triangle_gouraud_3d(state.gsGlobal,
                x0, y0, z0, x1, y1, z1, x2, y2, z2,
                makeColor(r0, g0, b0, a0),
                makeColor(r1, g1, b1, a1),
                makeColor(r2, g2, b2, a2));
            return;
        }

        const float* t0 = floatAt(texCoords, i0);
        const float* t1 = floatAt(texCoords, i1);
        const float* t2 = floatAt(texCoords, i2);

        float minU, minV, maxU, maxV;
        ps2_uv_bounds3(t0[0] * texW, t0[1] * texH,
                       t1[0] * texW, t1[1] * texH,
                       t2[0] * texW, t2[1] * texH,
                       minU, minV, maxU, maxV);
        ensureClamp(minU, minV, maxU, maxV);

        flushSprites();
        if (batchTriangles == PS2_DRAW2D_BATCH_TRIS)
            flushBatch();

        GSPRIMSTQPOINT* point = &batch[batchTriangles * 3];
        point[0].rgbaq = color_to_RGBAQ(PS2_TEXCOL_GUI(r0), PS2_TEXCOL_GUI(g0), PS2_TEXCOL_GUI(b0), static_cast<u8>(a0 >> 1), 1.0f);
        point[0].stq = vertex_to_STQ(t0[0], t0[1]);
        point[0].xyz2 = vertex_to_XYZ2(state.gsGlobal, x0, y0, z0);
        point[1].rgbaq = color_to_RGBAQ(PS2_TEXCOL_GUI(r1), PS2_TEXCOL_GUI(g1), PS2_TEXCOL_GUI(b1), static_cast<u8>(a1 >> 1), 1.0f);
        point[1].stq = vertex_to_STQ(t1[0], t1[1]);
        point[1].xyz2 = vertex_to_XYZ2(state.gsGlobal, x1, y1, z1);
        point[2].rgbaq = color_to_RGBAQ(PS2_TEXCOL_GUI(r2), PS2_TEXCOL_GUI(g2), PS2_TEXCOL_GUI(b2), static_cast<u8>(a2 >> 1), 1.0f);
        point[2].stq = vertex_to_STQ(t2[0], t2[1]);
        point[2].xyz2 = vertex_to_XYZ2(state.gsGlobal, x2, y2, z2);
        batchTriangles++;
    };

    // One axis-aligned, flat-coloured, textured quad as a GS sprite: two
    // vertices and one primitive instead of six vertices and two triangles, and
    // the GS rasterises a sprite without any edge setup. The GUI, the HUD and
    // every glyph the font draws are exactly that shape.
    //
    // Returns false for anything a sprite cannot represent, which then takes the
    // triangle path unchanged.
    auto emitQuadSprite = [&](int i0, int i1, int i2, int i3) -> bool {
        if (!state.texture || !mesh.texCoordEnabled)
            return false;

        const int index[4] = { i0, i1, i2, i3 };
        float px[4], py[4];
        int pz[4];
        unsigned char cr[4], cg[4], cb[4], ca[4];
        const float* uv[4];
        for (int k = 0; k < 4; k++) {
            project(index[k], px[k], py[k], pz[k]);
            fetchColor(index[k], cr[k], cg[k], cb[k], ca[k]);
            uv[k] = floatAt(texCoords, index[k]);
        }
        if (!state.render.smoothShading) {
            for (int k = 0; k < 3; ++k) {
                cr[k] = cr[3]; cg[k] = cg[3]; cb[k] = cb[3]; ca[k] = ca[3];
            }
        }

        // A sprite carries one RGBAQ for the whole rectangle and one depth, so
        // gouraud-shaded quads (the gradient the GUI background uses) and
        // non-coplanar ones cannot be represented.
        for (int k = 1; k < 4; k++) {
            if (cr[k] != cr[0] || cg[k] != cg[0] || cb[k] != cb[0] || ca[k] != ca[0])
                return false;
            if (pz[k] != pz[0])
                return false;
        }

        // Axis-aligned rectangle whose u depends only on x and v only on y.
        // Both perimeter windings the game emits are accepted; corners 0 and 2
        // are the diagonal pair either way. The comparisons are exact on
        // purpose: a quad that is merely close to axis-aligned must fall back,
        // not be silently squared off.
        const bool orderA = px[0] == px[3] && px[1] == px[2] &&
                            py[0] == py[1] && py[2] == py[3] &&
                            uv[0][0] == uv[3][0] && uv[1][0] == uv[2][0] &&
                            uv[0][1] == uv[1][1] && uv[2][1] == uv[3][1];
        const bool orderB = px[0] == px[1] && px[2] == px[3] &&
                            py[0] == py[3] && py[1] == py[2] &&
                            uv[0][0] == uv[1][0] && uv[2][0] == uv[3][0] &&
                            uv[0][1] == uv[3][1] && uv[1][1] == uv[2][1];
        if (!orderA && !orderB)
            return false;

        const float u0 = uv[0][0] * texW, v0 = uv[0][1] * texH;
        const float u2 = uv[2][0] * texW, v2 = uv[2][1] * texH;

        // vertex_to_UV clamps to the texture rectangle, so a tiling draw -- the
        // menu background walks terrain.png's dirt tile well past 1.0 -- would
        // be squashed against the edge instead of repeating. Those keep the STQ
        // triangle path, where REGION_REPEAT still covers them.
        const float minU = u0 < u2 ? u0 : u2;
        const float maxU = u0 < u2 ? u2 : u0;
        const float minV = v0 < v2 ? v0 : v2;
        const float maxV = v0 < v2 ? v2 : v0;
        if (minU < 0.0f || minV < 0.0f || maxU > texW || maxV > texH)
            return false;

        ensureClamp(minU, minV, maxU, maxV);

        flushBatch();
        if (batchSprites == PS2_DRAW2D_BATCH_SPRITES)
            flushSprites();

        // The GS fills from the first vertex to the second, so they must be
        // ordered lower-left to upper-right in screen space; the quad's corner 0
        // is neither in general. Picking the extremes per axis and taking each
        // one's own u/v keeps mirrored source rectangles mirrored, because the
        // GS simply interpolates from the first u to the second.
        const bool swapX = px[0] > px[2];
        const bool swapY = py[0] > py[2];
        const float loX = swapX ? px[2] : px[0], hiX = swapX ? px[0] : px[2];
        const float loY = swapY ? py[2] : py[0], hiY = swapY ? py[0] : py[2];
        const float loU = swapX ? u2 : u0,       hiU = swapX ? u0 : u2;
        const float loV = swapY ? v2 : v0,       hiV = swapY ? v0 : v2;

        GSPRIMUVPOINT* point = &spriteBatch[batchSprites * 2];
        point[0].rgbaq = color_to_RGBAQ(PS2_TEXCOL_GUI(cr[0]), PS2_TEXCOL_GUI(cg[0]),
                                        PS2_TEXCOL_GUI(cb[0]), static_cast<u8>(ca[0] >> 1), 1.0f);
        point[0].uv = vertex_to_UV(state.texture, loU, loV);
        point[0].xyz2 = vertex_to_XYZ2(state.gsGlobal, loX, loY, pz[0]);
        point[1].rgbaq = point[0].rgbaq;
        point[1].uv = vertex_to_UV(state.texture, hiU, hiV);
        point[1].xyz2 = vertex_to_XYZ2(state.gsGlobal, hiX, hiY, pz[0]);
        batchSprites++;
        return true;
    };

    if (mesh.drawMode == PS2_NATIVE_PRIM_QUADS) {
        const int quadCount = mesh.count / 4;
        for (int q = 0; q < quadCount; q++) {
            const int base = mesh.first + q * 4;
            if (emitQuadSprite(base + 0, base + 1, base + 2, base + 3)) {
#ifdef PS2_RENDER_STATS
                if (state.debugSprites) (*state.debugSprites)++;
#endif
                continue;
            }
#ifdef PS2_RENDER_STATS
            if (state.debugSpriteFallbacks) (*state.debugSpriteFallbacks)++;
#endif
            emitTriangle(base + 0, base + 1, base + 2);
            emitTriangle(base + 0, base + 2, base + 3);
        }
    } else if (mesh.drawMode == PS2_NATIVE_PRIM_TRIANGLES) {
        const int triangleCount = mesh.count / 3;
        for (int t = 0; t < triangleCount; t++) {
            const int base = mesh.first + t * 3;
            emitTriangle(base + 0, base + 1, base + 2);
        }
    } else {
        for (int i = 0; i + 2 < mesh.count; i++) {
            const int i0 = mesh.first + ((i & 1) ? i + 1 : i);
            const int i1 = mesh.first + ((i & 1) ? i : i + 1);
            emitTriangle(i0, i1, mesh.first + i + 2);
        }
    }

    flushPending();
    return true;
}

#endif // PS2_PLATFORM
