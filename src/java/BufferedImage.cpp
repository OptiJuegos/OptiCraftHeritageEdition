#include "java/BufferedImage.h"

#include <cassert>
#include <limits>
#include <stdexcept>

#include "stb_image.h"

std::size_t BufferedImage::checkedPixelCount(int_t width, int_t height)
{
	if (width <= 0 || height <= 0)
		throw std::invalid_argument("BufferedImage: non-positive dimensions");

	const std::size_t w = static_cast<std::size_t>(width);
	const std::size_t h = static_cast<std::size_t>(height);
	if (w != 0 && h > std::numeric_limits<std::size_t>::max() / w)
		throw std::overflow_error("BufferedImage: pixel count overflow");
	return w * h;
}

std::size_t BufferedImage::checkedRgbaByteCount(int_t width, int_t height)
{
	const std::size_t pixels = checkedPixelCount(width, height);
	if (pixels > std::numeric_limits<std::size_t>::max() / 4u)
		throw std::overflow_error("BufferedImage: RGBA byte count overflow");
	return pixels * 4u;
}

BufferedImage::BufferedImage(int_t width, int_t height, std::unique_ptr<unsigned char[]> raw_pixels)
{
	const std::size_t byteCount = checkedRgbaByteCount(width, height);
	if (byteCount != 0 && raw_pixels == nullptr)
		throw std::invalid_argument("BufferedImage: null pixel buffer");
	this->width = width;
	this->height = height;
	this->raw_pixels = std::move(raw_pixels);
}

BufferedImage::BufferedImage(int_t width, int_t height)
{
	this->width = width;
	this->height = height;
	this->raw_pixels = Util::make_unique<unsigned char[]>(checkedRgbaByteCount(width, height));
}

int_t BufferedImage::getWidth() const
{
	return width;
}
int_t BufferedImage::getHeight() const
{
	return height;
}
const unsigned char *BufferedImage::getRawPixels() const
{
	return raw_pixels.get();
}

void BufferedImage::validateRegion(int_t startX, int_t startY, int_t w, int_t h) const
{
	if (startX < 0 || startY < 0 || w < 0 || h < 0)
		throw std::out_of_range("BufferedImage: negative image region");

	const std::int64_t endX = static_cast<std::int64_t>(startX) + static_cast<std::int64_t>(w);
	const std::int64_t endY = static_cast<std::int64_t>(startY) + static_cast<std::int64_t>(h);
	if (endX > static_cast<std::int64_t>(width) || endY > static_cast<std::int64_t>(height))
		throw std::out_of_range("BufferedImage: image region outside bounds");
}

void BufferedImage::getRGB(int_t startX, int_t startY, int_t w, int_t h, unsigned char *rgbArray) const
{
	validateRegion(startX, startY, w, h);
	const std::size_t regionBytes = checkedRgbaByteCount(w, h);
	if (regionBytes != 0 && rgbArray == nullptr)
		throw std::invalid_argument("BufferedImage::getRGB: null output buffer");

	const std::size_t imageWidth = static_cast<std::size_t>(width);
	const std::size_t regionWidth = static_cast<std::size_t>(w);
	for (int_t y = 0; y < h; y++)
	{
		for (int_t x = 0; x < w; x++)
		{
			const std::size_t srcX = static_cast<std::size_t>(startX) + static_cast<std::size_t>(x);
			const std::size_t srcY = static_cast<std::size_t>(startY) + static_cast<std::size_t>(y);
			const std::size_t srcIndex = (srcY * imageWidth + srcX) * 4u;
			const std::size_t dstIndex = (static_cast<std::size_t>(y) * regionWidth + static_cast<std::size_t>(x)) * 4u;

			rgbArray[dstIndex + 0] = raw_pixels[srcIndex + 0];
			rgbArray[dstIndex + 1] = raw_pixels[srcIndex + 1];
			rgbArray[dstIndex + 2] = raw_pixels[srcIndex + 2];
			rgbArray[dstIndex + 3] = raw_pixels[srcIndex + 3];
		}
	}
}

void BufferedImage::setRGB(int_t startX, int_t startY, int_t w, int_t h, unsigned char *rgbArray)
{
	validateRegion(startX, startY, w, h);
	const std::size_t regionBytes = checkedRgbaByteCount(w, h);
	if (regionBytes != 0 && rgbArray == nullptr)
		throw std::invalid_argument("BufferedImage::setRGB: null input buffer");

	const std::size_t imageWidth = static_cast<std::size_t>(width);
	const std::size_t regionWidth = static_cast<std::size_t>(w);
	for (int_t y = 0; y < h; y++)
	{
		for (int_t x = 0; x < w; x++)
		{
			const std::size_t srcIndex = (static_cast<std::size_t>(y) * regionWidth + static_cast<std::size_t>(x)) * 4u;
			const std::size_t dstX = static_cast<std::size_t>(startX) + static_cast<std::size_t>(x);
			const std::size_t dstY = static_cast<std::size_t>(startY) + static_cast<std::size_t>(y);
			const std::size_t dstIndex = (dstY * imageWidth + dstX) * 4u;

			raw_pixels[dstIndex + 0] = rgbArray[srcIndex + 0];
			raw_pixels[dstIndex + 1] = rgbArray[srcIndex + 1];
			raw_pixels[dstIndex + 2] = rgbArray[srcIndex + 2];
			raw_pixels[dstIndex + 3] = rgbArray[srcIndex + 3];
		}
	}
}

static int istream_read(void *user, char *data, int size)
{
	auto &in = *reinterpret_cast<std::istream *>(user);
	in.read(reinterpret_cast<char *>(data), size);
	return static_cast<int>(in.gcount());
}

static void istream_skip(void *user, int n)
{
	auto &in = *reinterpret_cast<std::istream *>(user);
	in.seekg(n, std::ios::cur);
}

static int istream_eof(void *user)
{
	auto &in = *reinterpret_cast<std::istream *>(user);
	return in.eof();
}

stbi_io_callbacks stbi_io_callbacks_istream = { istream_read, istream_skip, istream_eof };

BufferedImage BufferedImage::ImageIO_read(std::istream &in)
{
	// Decode image
	int w, h, comp;
	stbi_uc *raw_data = stbi_load_from_callbacks(&stbi_io_callbacks_istream, &in, &w, &h, &comp, 0);
	// Must throw (not assert): assert is a no-op in release, so an undecodable
	// download (e.g. an HTTP error page) would read uninitialized w/h and the
	// null raw_data -> UB. Throwing lets the caller fall back to the default skin.
	if (raw_data == nullptr)
		throw std::runtime_error(std::string("ImageIO_read: decode failed: ") +
		                         (stbi_failure_reason() ? stbi_failure_reason() : "unknown"));

	// Convert to RGBA. Keep all size arithmetic in size_t so malformed image
	// dimensions cannot wrap into an undersized allocation.
	const std::size_t pixelCount = checkedPixelCount(w, h);
	std::unique_ptr<unsigned char[]> data = Util::make_unique<unsigned char[]>(checkedRgbaByteCount(w, h));

	if (comp == 1)
	{
		for (std::size_t i = 0; i < pixelCount; i++)
		{
			data[i * 4 + 0] = raw_data[i];
			data[i * 4 + 1] = raw_data[i];
			data[i * 4 + 2] = raw_data[i];
			data[i * 4 + 3] = 255;
		}
		stbi_image_free(raw_data);
	}
	else if (comp == 2)
	{
		for (std::size_t i = 0; i < pixelCount; i++)
		{
			data[i * 4 + 0] = raw_data[i * 2 + 0];
			data[i * 4 + 1] = raw_data[i * 2 + 0];
			data[i * 4 + 2] = raw_data[i * 2 + 0];
			data[i * 4 + 3] = raw_data[i * 2 + 1];
		}
		stbi_image_free(raw_data);
	}
	else if (comp == 3)
	{
		for (std::size_t i = 0; i < pixelCount; i++)
		{
			data[i * 4 + 0] = raw_data[i * 3 + 0];
			data[i * 4 + 1] = raw_data[i * 3 + 1];
			data[i * 4 + 2] = raw_data[i * 3 + 2];
			data[i * 4 + 3] = 255;
		}
		stbi_image_free(raw_data);
	}
	else if (comp == 4)
	{
		for (std::size_t i = 0; i < pixelCount; i++)
		{
			data[i * 4 + 0] = raw_data[i * 4 + 0];
			data[i * 4 + 1] = raw_data[i * 4 + 1];
			data[i * 4 + 2] = raw_data[i * 4 + 2];
			data[i * 4 + 3] = raw_data[i * 4 + 3];
		}
		stbi_image_free(raw_data);
	}
	else
	{
		stbi_image_free(raw_data);
		throw std::runtime_error("ImageIO_read: unsupported channel count");
	}

	return BufferedImage(w, h, std::move(data));
}
