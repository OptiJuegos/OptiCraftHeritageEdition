#include "platform/RenderAPI.h"

void renderStaticMeshCreate(RenderStaticMesh& mesh)
{
    renderStaticMeshDestroy(mesh);
#if PLATFORM_PERSISTENT_RENDER_MESH
    mesh.persistentHandle = renderCreatePersistentMesh();
#endif
}

void renderStaticMeshDestroy(RenderStaticMesh& mesh)
{
#if PLATFORM_PERSISTENT_RENDER_MESH
    if (mesh.persistentHandle > 0)
        renderDestroyPersistentMesh(mesh.persistentHandle);
#endif
    mesh.persistentHandle = 0;
    mesh.persistentReady = false;
    mesh.captured.clear();
}

bool renderStaticMeshCompile(RenderStaticMesh& mesh, const RenderInterleavedMesh& source)
{
    mesh.persistentReady = false;
    mesh.captured.clear();

#if PLATFORM_PERSISTENT_RENDER_MESH
    if (mesh.persistentHandle > 0 && renderCompilePersistentMesh(mesh.persistentHandle, source))
    {
        mesh.persistentReady = true;
        return true;
    }
#endif

    return renderCaptureInterleaved(source, mesh.captured, false);
}

bool renderStaticMeshDraw(const RenderStaticMesh& mesh)
{
#if PLATFORM_PERSISTENT_RENDER_MESH
    if (mesh.persistentReady && renderDrawPersistentMesh(mesh.persistentHandle))
        return true;
#endif
    return renderDrawCaptured(mesh.captured);
}
