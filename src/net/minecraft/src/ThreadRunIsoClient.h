#pragma once

#include <thread>

class CanvasIsomPreview;

// net.minecraft.src.ThreadRunIsoClient
class ThreadRunIsoClient
{
public:
	ThreadRunIsoClient(CanvasIsomPreview *canvasisompreview);
	~ThreadRunIsoClient();

	void start();
	void run();

private:
	CanvasIsomPreview *isoCanvas;
	std::thread worker;
};
