#pragma once

#include <memory>

class ThreadRunIsoClient;

// net.minecraft.src.CanvasIsomPreview
class CanvasIsomPreview
{
public:
    CanvasIsomPreview();
    ~CanvasIsomPreview();

    static bool isRunning(CanvasIsomPreview *canvasisompreview)
    {
        return canvasisompreview != nullptr && canvasisompreview->running;
    }

    void start();
    void stop();
    void showNextBuffer();

    bool running;

private:
    std::unique_ptr<ThreadRunIsoClient> uiThread;
};
