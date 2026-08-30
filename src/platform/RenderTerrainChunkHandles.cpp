#include "platform/RenderTerrainAPI.h"

void renderTerrainChunkHandlesCreate(RenderTerrainChunkHandles& handles)
{
	for (int pass = 0; pass < 2; ++pass)
	{
		handles.live[pass] = renderTerrainCreateChunkHandle();
		handles.staging[pass] = renderTerrainCreateChunkHandle();
	}
}

void renderTerrainChunkHandlesDestroy(RenderTerrainChunkHandles& handles)
{
	for (int pass = 0; pass < 2; ++pass)
	{
		if (handles.live[pass] != 0)
			renderTerrainDestroyChunkHandle(handles.live[pass]);
		if (handles.staging[pass] != 0)
			renderTerrainDestroyChunkHandle(handles.staging[pass]);
		handles.live[pass] = 0;
		handles.staging[pass] = 0;
	}
}

void renderTerrainChunkHandlesClearLive(RenderTerrainChunkHandles& handles)
{
	for (int pass = 0; pass < 2; ++pass)
		if (handles.live[pass] != 0)
			renderTerrainClearChunkHandle(handles.live[pass]);
}

void renderTerrainChunkHandlesClearStaging(RenderTerrainChunkHandles& handles)
{
	for (int pass = 0; pass < 2; ++pass)
		if (handles.staging[pass] != 0)
			renderTerrainClearChunkHandle(handles.staging[pass]);
}

void renderTerrainChunkHandlesPublish(RenderTerrainChunkHandles& handles)
{
	for (int pass = 0; pass < 2; ++pass)
		renderTerrainSwapChunkHandles(handles.live[pass], handles.staging[pass]);
	renderTerrainChunkHandlesClearStaging(handles);
}
