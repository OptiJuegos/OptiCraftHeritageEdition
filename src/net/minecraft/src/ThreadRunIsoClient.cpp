#include "ThreadRunIsoClient.h"

#include <chrono>

#include "CanvasIsomPreview.h"

ThreadRunIsoClient::ThreadRunIsoClient(CanvasIsomPreview *canvasisompreview)
    : isoCanvas(canvasisompreview)
{
}

ThreadRunIsoClient::~ThreadRunIsoClient()
{
    if (worker.joinable())
        worker.join();
}

void ThreadRunIsoClient::start()
{
    if (worker.joinable())
        return;
    worker = std::thread(&ThreadRunIsoClient::run, this);
}

void ThreadRunIsoClient::run()
{
    while (CanvasIsomPreview::isRunning(isoCanvas))
    {
        isoCanvas->showNextBuffer();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
