#include "RenderHelper.h"
#include "Vec3D.h"
#include "platform/PlatformTuning.h"
#include "platform/RenderAPI.h"
#include "platform/RenderLightingProfile.h"

#include <cmath>

std::vector<float> RenderHelper::field_1695_a(16);

RenderHelper::RenderHelper() {
}

void RenderHelper::disableStandardItemLighting() {
    renderDisable(RenderCapability::Lighting);
    renderDisable(RenderCapability::Light0);
    renderDisable(RenderCapability::Light1);
    renderDisable(RenderCapability::ColorMaterial);
}

void RenderHelper::enableStandardItemLighting() {
    renderEnable(RenderCapability::Lighting);
    renderEnable(RenderCapability::Light0);
    renderEnable(RenderCapability::Light1);
    renderEnable(RenderCapability::ColorMaterial);
    renderColorMaterial(RenderFace::FrontAndBack, RenderColorMaterialMode::AmbientAndDiffuse);
    const RenderLightingProfile lightingProfile = renderGetStandardItemLightingProfile();
    const float f = lightingProfile.ambient;
    const float f1 = lightingProfile.diffuse;
    float f2 = 0.0f;
#if PLATFORM_FLOAT_VERTEX_MATH
    const float lightX = 0.2f;
    const float lightY = 1.0f;
    const float lightZ = -0.7f;
    const float inverseLightLength = 1.0f / std::sqrt(lightX * lightX + lightY * lightY + lightZ * lightZ);
    std::vector<float> light0Pos = createFloatBuffer(lightX * inverseLightLength,
                                                     lightY * inverseLightLength,
                                                     lightZ * inverseLightLength,
                                                     0.0f);
#else
    Vec3D* vec3d = Vec3D::createVector(0.20000000298023224, 1.0, -0.69999998807907104);
    vec3d = vec3d->normalize();
    std::vector<float> light0Pos = createFloatBufferFromDoubles(vec3d->xCoord, vec3d->yCoord, vec3d->zCoord, 0.0);
#endif
    renderLightfv(0, RenderLightParameter::Position, light0Pos.data());
    std::vector<float> light0Diffuse = createFloatBuffer(f1, f1, f1, 1.0f);
    renderLightfv(0, RenderLightParameter::Diffuse, light0Diffuse.data());
    std::vector<float> light0Ambient = createFloatBuffer(0.0f, 0.0f, 0.0f, 1.0f);
    renderLightfv(0, RenderLightParameter::Ambient, light0Ambient.data());
    std::vector<float> light0Specular = createFloatBuffer(f2, f2, f2, 1.0f);
    renderLightfv(0, RenderLightParameter::Specular, light0Specular.data());

#if PLATFORM_FLOAT_VERTEX_MATH
    std::vector<float> light1Pos = createFloatBuffer(-lightX * inverseLightLength,
                                                     lightY * inverseLightLength,
                                                     -lightZ * inverseLightLength,
                                                     0.0f);
#else
    vec3d = Vec3D::createVector(-0.20000000298023224, 1.0, 0.69999998807907104);
    vec3d = vec3d->normalize();
    std::vector<float> light1Pos = createFloatBufferFromDoubles(vec3d->xCoord, vec3d->yCoord, vec3d->zCoord, 0.0);
#endif
    renderLightfv(1, RenderLightParameter::Position, light1Pos.data());
    std::vector<float> light1Diffuse = createFloatBuffer(f1, f1, f1, 1.0f);
    renderLightfv(1, RenderLightParameter::Diffuse, light1Diffuse.data());
    std::vector<float> light1Ambient = createFloatBuffer(0.0f, 0.0f, 0.0f, 1.0f);
    renderLightfv(1, RenderLightParameter::Ambient, light1Ambient.data());
    std::vector<float> light1Specular = createFloatBuffer(f2, f2, f2, 1.0f);
    renderLightfv(1, RenderLightParameter::Specular, light1Specular.data());

    renderShadeModel(RenderShadeModel::Flat);
    std::vector<float> lightModelAmbient = createFloatBuffer(f, f, f, 1.0f);
    renderLightModelAmbient(lightModelAmbient.data());
}

void RenderHelper::enableGUIStandardItemLighting() {
    renderPushMatrix();
    renderRotate(-30.0f, 0.0f, 1.0f, 0.0f);
    renderRotate(165.0f, 1.0f, 0.0f, 0.0f);
    enableStandardItemLighting();
    renderPopMatrix();
}

std::vector<float> RenderHelper::createFloatBufferFromDoubles(double d, double d1, double d2, double d3) {
    return createFloatBuffer((float)d, (float)d1, (float)d2, (float)d3);
}

std::vector<float> RenderHelper::createFloatBuffer(float f, float f1, float f2, float f3) {
    std::vector<float> buffer(4);
    buffer[0] = f;
    buffer[1] = f1;
    buffer[2] = f2;
    buffer[3] = f3;
    return buffer;
}
