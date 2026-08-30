#pragma once

class BufferedImage;

// net.minecraft.src.ImageBuffer
class ImageBuffer
{
public:
	virtual ~ImageBuffer() = default;
	virtual BufferedImage *parseUserSkin(BufferedImage *bufferedimage) = 0;
};
