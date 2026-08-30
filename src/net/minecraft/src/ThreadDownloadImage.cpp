#include "platform/Log.h"
#include "ThreadDownloadImage.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "ImageBuffer.h"
#include "ThreadDownloadImageData.h"
#include "java/JavaNetwork.h"
#include "java/BufferedImage.h"

ThreadDownloadImage::ThreadDownloadImage(ThreadDownloadImageData *threaddownloadimagedata, const std::string &s, ImageBuffer *imagebuffer)
	: location(s)
	, buffer(imagebuffer)
	, imageData(threaddownloadimagedata)
{
}

ThreadDownloadImage::~ThreadDownloadImage()
{
	if (worker.joinable())
		worker.join();
	delete buffer;
	buffer = nullptr;
}

void ThreadDownloadImage::start()
{
#ifdef WII_PLATFORM
	// The Wii backend currently exposes multiplayer TCP only. readUrl() returns
	// false immediately, so execute it inline instead of invoking std::thread,
	// which is an ENOSYS stub in devkitPPC.
	run();
#else
	worker = std::thread(&ThreadDownloadImage::run, this);
#endif
}

void ThreadDownloadImage::run()
{
	try
	{
		std::vector<unsigned char> data;
		if (!JavaNetwork::readUrl(location, data))
			return;
		std::string bytes(reinterpret_cast<const char *>(data.data()), data.size());
		std::istringstream input(bytes, std::ios::binary);
		BufferedImage *downloaded = new BufferedImage(BufferedImage::ImageIO_read(input));
		BufferedImage *parsed = buffer == nullptr ? downloaded : buffer->parseUserSkin(downloaded);
		if (parsed != downloaded)
			delete downloaded;

		// Publish the fully-built image atomically. If a retry ever replaces an
		// existing image, release the previous allocation instead of leaking it.
		BufferedImage *old = imageData->image.exchange(parsed);
		if (old != parsed)
			delete old;
	}
	catch (std::exception &exception)
	{
		MC_LOG_ERROR("game", "%s\n", exception.what());
	}
}
