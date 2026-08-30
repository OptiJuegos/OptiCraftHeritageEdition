#include "ThreadDownloadImageData.h"

#include "ThreadDownloadImage.h"
#include "java/BufferedImage.h"

ThreadDownloadImageData::ThreadDownloadImageData(const std::string &s, ImageBuffer *imagebuffer)
	: image(nullptr)
	, referenceCount(1)
	, textureName(-1)
	, textureSetupComplete(false)
	, downloadThread(new ThreadDownloadImage(this, s, imagebuffer))
{
	downloadThread->start();
}

ThreadDownloadImageData::~ThreadDownloadImageData()
{
	// The worker owns only a non-owning pointer back to this object. Join it
	// before freeing either the image or this object so releaseImageData() can
	// never race a detached skin download into a use-after-free.
	delete downloadThread;
	downloadThread = nullptr;
	delete image.exchange(nullptr);
}
