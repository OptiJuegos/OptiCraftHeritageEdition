#include "CustomAnimation.h"

#include <limits>

#include "OptiFineResource.h"

CustomAnimation::CustomAnimation(const std::string &sourcePath,
                                 std::vector<byte_t> rgbaData,
                                 int_t width,
                                 int_t height,
                                 const std::map<std::string, std::string> &properties,
                                 int_t defaultDuration)
	: imagePath(sourcePath)
	, destX(0)
	, destY(0)
	, frameWidth(width)
	, frameHeight(height)
	, category(CustomAnimationCategory::Generic)
	, imageBytes(std::move(rgbaData))
	, activeFrame(0)
{
	if (frameWidth <= 0 || frameHeight <= 0)
		return;
	const std::size_t pixelCount = static_cast<std::size_t>(frameWidth) * static_cast<std::size_t>(frameHeight);
	if (pixelCount > std::numeric_limits<std::size_t>::max() / 4u)
		return;
	const std::size_t frameBytes = pixelCount * 4u;
	if (frameBytes == 0 || imageBytes.size() < frameBytes || imageBytes.size() % frameBytes != 0)
		return;

	int_t sourceFrameCount = static_cast<int_t>(imageBytes.size() / frameBytes);
	int_t timelineCount = sourceFrameCount;
	if (properties.find("tile.0") != properties.end())
	{
		timelineCount = 0;
		while (properties.find("tile." + std::to_string(timelineCount)) != properties.end())
			++timelineCount;
	}
	const auto durationIt = properties.find("duration");
	const int_t durationDefault = durationIt == properties.end()
		? (defaultDuration > 0 ? defaultDuration : 1)
		: OptiFineResource::parseInt(durationIt->second, defaultDuration > 0 ? defaultDuration : 1);

	for (int_t i = 0; i < timelineCount; ++i)
	{
		int_t index = i;
		auto tileIt = properties.find("tile." + std::to_string(i));
		if (tileIt != properties.end())
			index = OptiFineResource::parseInt(tileIt->second, i);
		if (index < 0 || index >= sourceFrameCount)
			continue;
		int_t duration = durationDefault;
		auto durationFrameIt = properties.find("duration." + std::to_string(i));
		if (durationFrameIt != properties.end())
			duration = OptiFineResource::parseInt(durationFrameIt->second, durationDefault);
		frames.emplace_back(index, duration > 0 ? duration : 1);
	}
}

bool CustomAnimation::isValid() const
{
	return frameWidth > 0 && frameHeight > 0 && !frames.empty() && imageBytes.size() >= getFrameByteCount();
}

bool CustomAnimation::nextFrame()
{
	if (!isValid())
		return false;
	if (activeFrame < 0 || activeFrame >= static_cast<int_t>(frames.size()))
		activeFrame = 0;
	CustomAnimationFrame &frame = frames[static_cast<std::size_t>(activeFrame)];
	++frame.counter;
	if (frame.counter < frame.duration)
		return false;
	frame.counter = 0;
	activeFrame = (activeFrame + 1) % static_cast<int_t>(frames.size());
	return true;
}

const byte_t *CustomAnimation::getActiveFrameData() const
{
	if (!isValid())
		return nullptr;
	int_t frameSlot = activeFrame;
	if (frameSlot < 0 || frameSlot >= static_cast<int_t>(frames.size()))
		frameSlot = 0;
	const int_t sourceIndex = frames[static_cast<std::size_t>(frameSlot)].index;
	const std::size_t offset = static_cast<std::size_t>(sourceIndex) * getFrameByteCount();
	return offset + getFrameByteCount() <= imageBytes.size() ? imageBytes.data() + offset : nullptr;
}

std::size_t CustomAnimation::getFrameByteCount() const
{
	if (frameWidth <= 0 || frameHeight <= 0)
		return 0;
	return static_cast<std::size_t>(frameWidth) * static_cast<std::size_t>(frameHeight) * 4u;
}

int_t CustomAnimation::getFrameCount() const
{
	return static_cast<int_t>(frames.size());
}
