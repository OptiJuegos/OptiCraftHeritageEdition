#include "ModelRenderer.h"

#if PLATFORM_PC
#include "GLAllocation.h"
#endif
#include "ModelBase.h"
#include "ModelBox.h"
#include "Tessellator.h"
#include "TexturedQuad.h"
#include "TextureOffset.h"
#include "platform/RenderAPI.h"

#include <algorithm>

#if PLATFORM_MODEL_PERSISTENT_MESH && MC_LOG_LEVEL >= 2
#define MODEL_TRANSFORM_TRACE 1
#else
#define MODEL_TRANSFORM_TRACE 0
#endif

#if MODEL_TRANSFORM_TRACE
#include <cmath>
#include "PositionTextureVertex.h"
#include "platform/Log.h"
#endif

ModelRenderer::ModelRenderer(int_t i, int_t j)
{
    init(nullptr, std::string(), i, j);
}

ModelRenderer::ModelRenderer(ModelBase* model)
{
    init(model, std::string(), 0, 0);
}

ModelRenderer::ModelRenderer(ModelBase* model, int_t i, int_t j)
{
    init(model, std::string(), i, j);
}

ModelRenderer::ModelRenderer(ModelBase* model, const std::string& name)
{
    init(model, name, 0, 0);
}

void ModelRenderer::init(ModelBase* model, const std::string& name, int_t textureX, int_t textureY)
{
    textureWidth = model != nullptr ? static_cast<float>(model->textureWidth) : 64.0f;
    textureHeight = model != nullptr ? static_cast<float>(model->textureHeight) : 32.0f;
    textureOffsetX = textureX;
    textureOffsetY = textureY;
    compiled = false;
    displayList = 0;
    baseModel = model;
    boxName = name;
    rotationPointX = 0.0f;
    rotationPointY = 0.0f;
    rotationPointZ = 0.0f;
    rotateAngleX = 0.0f;
    rotateAngleY = 0.0f;
    rotateAngleZ = 0.0f;
    mirror = false;
    showModel = true;
    field_1402_i = false;

    if (baseModel != nullptr)
        baseModel->boxList.push_back(this);
}

ModelRenderer::~ModelRenderer()
{
    if (baseModel != nullptr)
    {
        auto& boxes = baseModel->boxList;
        boxes.erase(std::remove(boxes.begin(), boxes.end(), this), boxes.end());
    }

    invalidateCompiledGeometry();
    for (ModelBox* box : cubeList)
        delete box;
    cubeList.clear();
    childModels.clear();
}

void ModelRenderer::invalidateCompiledGeometry()
{
    if (displayList != 0)
    {
#if PLATFORM_MODEL_PERSISTENT_MESH
        renderDestroyPersistentMesh(displayList);
#elif PLATFORM_PC
        GLAllocation::deleteDisplayLists(displayList);
#endif
        displayList = 0;
    }
    compiled = false;
}

ModelRenderer* ModelRenderer::addBox(float x, float y, float z, int_t sizeX, int_t sizeY, int_t sizeZ)
{
    return addBox(x, y, z, sizeX, sizeY, sizeZ, 0.0f);
}

ModelRenderer* ModelRenderer::addBox(float x, float y, float z, int_t sizeX, int_t sizeY, int_t sizeZ, float scale)
{
    invalidateCompiledGeometry();
    cubeList.push_back(new ModelBox(this, textureOffsetX, textureOffsetY,
        x, y, z, sizeX, sizeY, sizeZ, scale));
    return this;
}

ModelRenderer* ModelRenderer::addBox(const std::string& name, float x, float y, float z,
                                     int_t sizeX, int_t sizeY, int_t sizeZ)
{
    std::string fullName = boxName.empty() ? name : boxName + "." + name;
    if (baseModel != nullptr)
    {
        const TextureOffset* offset = baseModel->getTextureOffset(fullName);
        if (offset != nullptr)
            setTextureOffset(offset->field_40734_a, offset->field_40733_b);
    }
    invalidateCompiledGeometry();
    ModelBox* box = new ModelBox(this, textureOffsetX, textureOffsetY,
        x, y, z, sizeX, sizeY, sizeZ, 0.0f);
    box->setBoxName(fullName);
    cubeList.push_back(box);
    return this;
}

void ModelRenderer::addChild(ModelRenderer* child)
{
    if (child != nullptr)
        childModels.push_back(child);
}

ModelRenderer* ModelRenderer::setTextureOffset(int_t x, int_t y)
{
    textureOffsetX = x;
    textureOffsetY = y;
    return this;
}

ModelRenderer* ModelRenderer::setTextureSize(int_t width, int_t height)
{
    textureWidth = static_cast<float>(width);
    textureHeight = static_cast<float>(height);
    return this;
}

void ModelRenderer::setRotationPoint(float x, float y, float z)
{
    rotationPointX = x;
    rotationPointY = y;
    rotationPointZ = z;
}

void ModelRenderer::drawGeometry(float scale)
{
#if PLATFORM_MODEL_PERSISTENT_MESH
    if (displayList == 0 || !renderDrawPersistentMesh(displayList))
        renderImmediate(scale);
#elif PLATFORM_MODEL_IMMEDIATE
    renderImmediate(scale);
#else
    if (displayList != 0)
        renderCallDisplayList(displayList);
#endif
}

void ModelRenderer::renderChildren(float scale)
{
    for (ModelRenderer* child : childModels)
    {
        if (child != nullptr)
            child->render(scale);
    }
}

void ModelRenderer::render(float scale)
{
    if (field_1402_i || !showModel)
        return;
    if (!compiled)
        compileDisplayList(scale);

    if (rotateAngleX == 0.0f && rotateAngleY == 0.0f && rotateAngleZ == 0.0f)
    {
        if (rotationPointX == 0.0f && rotationPointY == 0.0f && rotationPointZ == 0.0f)
        {
            drawGeometry(scale);
            renderChildren(scale);
        }
        else
        {
            renderTranslate(rotationPointX * scale, rotationPointY * scale, rotationPointZ * scale);
            drawGeometry(scale);
            renderChildren(scale);
            renderTranslate(-rotationPointX * scale, -rotationPointY * scale, -rotationPointZ * scale);
        }
        return;
    }

    renderPushMatrix();
    renderTranslate(rotationPointX * scale, rotationPointY * scale, rotationPointZ * scale);
    if (rotateAngleZ != 0.0f)
        renderRotate(rotateAngleZ * 57.295776f, 0.0f, 0.0f, 1.0f);
    if (rotateAngleY != 0.0f)
        renderRotate(rotateAngleY * 57.295776f, 0.0f, 1.0f, 0.0f);
    if (rotateAngleX != 0.0f)
        renderRotate(rotateAngleX * 57.295776f, 1.0f, 0.0f, 0.0f);
    drawGeometry(scale);
    renderChildren(scale);
    renderPopMatrix();
}

void ModelRenderer::renderWithRotation(float scale)
{
    if (field_1402_i || !showModel)
        return;
    if (!compiled)
        compileDisplayList(scale);

    renderPushMatrix();
    renderTranslate(rotationPointX * scale, rotationPointY * scale, rotationPointZ * scale);
    if (rotateAngleY != 0.0f)
        renderRotate(rotateAngleY * 57.295776f, 0.0f, 1.0f, 0.0f);
    if (rotateAngleX != 0.0f)
        renderRotate(rotateAngleX * 57.295776f, 1.0f, 0.0f, 0.0f);
    if (rotateAngleZ != 0.0f)
        renderRotate(rotateAngleZ * 57.295776f, 0.0f, 0.0f, 1.0f);
    drawGeometry(scale);
    renderChildren(scale);
    renderPopMatrix();
}

void ModelRenderer::postRender(float scale)
{
    if (field_1402_i || !showModel)
        return;
    if (!compiled)
        compileDisplayList(scale);

    if (rotateAngleX == 0.0f && rotateAngleY == 0.0f && rotateAngleZ == 0.0f)
    {
        if (rotationPointX != 0.0f || rotationPointY != 0.0f || rotationPointZ != 0.0f)
            renderTranslate(rotationPointX * scale, rotationPointY * scale, rotationPointZ * scale);
        return;
    }

    renderTranslate(rotationPointX * scale, rotationPointY * scale, rotationPointZ * scale);
    if (rotateAngleZ != 0.0f)
        renderRotate(rotateAngleZ * 57.295776f, 0.0f, 0.0f, 1.0f);
    if (rotateAngleY != 0.0f)
        renderRotate(rotateAngleY * 57.295776f, 0.0f, 1.0f, 0.0f);
    if (rotateAngleX != 0.0f)
        renderRotate(rotateAngleX * 57.295776f, 1.0f, 0.0f, 0.0f);
}

void ModelRenderer::compileDisplayList(float scale)
{
#if PLATFORM_MODEL_PERSISTENT_MESH
    displayList = renderCreatePersistentMesh();
    if (displayList != 0 && !cubeList.empty())
    {
        Tessellator* tessellator = &Tessellator::instance;
        tessellator->startDrawingQuads();
        for (ModelBox* box : cubeList)
        {
            if (box != nullptr)
                box->emitInto(tessellator, scale);
        }
        if (!tessellator->finishPersistentMesh(displayList))
        {
            tessellator->cancelDrawing();
            renderDestroyPersistentMesh(displayList);
            displayList = 0;
        }
    }
    else if (displayList != 0)
    {
        renderDestroyPersistentMesh(displayList);
        displayList = 0;
    }
    compiled = true;
#elif PLATFORM_MODEL_IMMEDIATE
    (void)scale;
    compiled = true;
#else
    if (!cubeList.empty())
    {
        displayList = GLAllocation::generateDisplayLists(1);
        renderBeginDisplayList(displayList);
        Tessellator* tessellator = &Tessellator::instance;
        for (ModelBox* box : cubeList)
        {
            if (box != nullptr)
                box->render(tessellator, scale);
        }
        renderEndDisplayList();
    }
    compiled = true;
#endif
}

#if MODEL_TRANSFORM_TRACE
namespace
{
    int_t milli(float value)
    {
        const float scaled = value * 1000.0f;
        if (scaled > 2147000000.0f) return 2147000000;
        if (scaled < -2147000000.0f) return -2147000000;
        if (!(scaled == scaled)) return 0;
        return static_cast<int_t>(scaled);
    }

    bool finiteValue(float value)
    {
        return value == value && value < 1e30f && value > -1e30f;
    }

    float axisLength(const float* axis)
    {
        return std::sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
    }

    float modelExtent(const std::vector<ModelBox*>& boxes, float scale)
    {
        float lo[3] = {1e30f, 1e30f, 1e30f};
        float hi[3] = {-1e30f, -1e30f, -1e30f};
        bool any = false;
        for (ModelBox* box : boxes)
        {
            if (box == nullptr)
                continue;
            for (int q = 0; q < 6; ++q)
            {
                TexturedQuad* quad = box->getQuad(q);
                if (quad == nullptr)
                    continue;
                for (int v = 0; v < quad->nVertices; ++v)
                {
                    PositionTextureVertex* p = quad->vertexPositions[v];
                    if (p == nullptr)
                        continue;
                    const float c[3] = {p->positionX * scale, p->positionY * scale, p->positionZ * scale};
                    for (int k = 0; k < 3; ++k)
                    {
                        if (!finiteValue(c[k]))
                            return 1e30f;
                        if (c[k] < lo[k]) lo[k] = c[k];
                        if (c[k] > hi[k]) hi[k] = c[k];
                    }
                    any = true;
                }
            }
        }
        if (!any)
            return 0.0f;
        float extent = 0.0f;
        for (int k = 0; k < 3; ++k)
            extent = std::max(extent, hi[k] - lo[k]);
        return extent;
    }

    void traceModelTransform(const std::vector<ModelBox*>& boxes, float scale)
    {
        static int_t budget = 8;
        if (budget <= 0)
            return;
        --budget;
        float matrix[16] = {};
        renderGetMatrix(RenderMatrixQuery::ModelView, matrix);
        const float lx = axisLength(&matrix[0]);
        const float ly = axisLength(&matrix[4]);
        const float lz = axisLength(&matrix[8]);
        LOGI("[WII][MODEL] f=%d |X|=%d |Y|=%d |Z|=%d T=(%d %d %d) ext=%d\n",
            (int)milli(scale), (int)milli(lx), (int)milli(ly), (int)milli(lz),
            (int)milli(matrix[12]), (int)milli(matrix[13]), (int)milli(matrix[14]),
            (int)milli(modelExtent(boxes, scale)));
    }
}
#endif

#if PLATFORM_MODEL_IMMEDIATE || PLATFORM_MODEL_PERSISTENT_MESH
void ModelRenderer::renderImmediate(float scale)
{
    if (cubeList.empty())
        return;
#if MODEL_TRANSFORM_TRACE
    traceModelTransform(cubeList, scale);
#endif
    Tessellator* tessellator = &Tessellator::instance;
    tessellator->startDrawingQuads();
    for (ModelBox* box : cubeList)
    {
        if (box != nullptr)
            box->emitInto(tessellator, scale);
    }
    tessellator->draw();
}
#endif
