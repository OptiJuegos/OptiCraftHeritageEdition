#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "CustomAnimationFrame.h"
#include "java/Type.h"

enum class CustomAnimationCategory
{
	Generic,
	Terrain,
	Items,
	Water,
	Lava,
	Fire,
	Portal
};

// OptiFine C6 custom animation independent of the removed TextureHD classes.
// Source data is a vertical strip of RGBA frames; destination describes a
// rectangle inside any texture selected by path.
class CustomAnimation
{
public:
	CustomAnimation(const std::string &sourcePath,
	                std::vector<byte_t> rgbaData,
	                int_t width,
	                int_t height,
	                const std::map<std::string, std::string> &properties,
	                int_t defaultDuration = 1);

	bool isValid() const;
	bool nextFrame();
	const byte_t *getActiveFrameData() const;
	std::size_t getFrameByteCount() const;
	int_t getFrameCount() const;

	std::string imagePath;
	std::string destTexture;
	int_t destX;
	int_t destY;
	int_t frameWidth;
	int_t frameHeight;
	CustomAnimationCategory category;

private:
	std::vector<byte_t> imageBytes;
	std::vector<CustomAnimationFrame> frames;
	int_t activeFrame;
};
