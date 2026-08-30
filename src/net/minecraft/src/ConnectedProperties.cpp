#include "ConnectedProperties.h"

#include <algorithm>
#include <sstream>

#include "OptiFineResource.h"

std::string ConnectedProperties::get(const std::map<std::string, std::string> &properties, const std::string &key)
{
	auto it = properties.find(key);
	return it != properties.end() ? it->second : std::string();
}

ConnectedProperties::ConnectedProperties(const std::map<std::string, std::string> &properties)
	: method(parseMethod(get(properties, "method"))),
	  source(get(properties, "source")),
	  tiles(OptiFineResource::parseIntList(get(properties, "tiles"))),
	  connect(parseConnect(get(properties, "connect"))),
	  faces(parseFaces(get(properties, "faces"))),
	  metadata(OptiFineResource::parseIntList(get(properties, "metadata"))),
	  weights(OptiFineResource::parseIntList(get(properties, "weights"))),
	  symmetry(parseSymmetry(get(properties, "symmetry"))),
	  width(OptiFineResource::parseInt(get(properties, "width"), -1)),
	  height(OptiFineResource::parseInt(get(properties, "height"), -1)),
	  sumAllWeights(0), textureId(-1)
{
}

int_t ConnectedProperties::parseMethod(const std::string &value)
{
	if (value.empty() || value == "ctm") return METHOD_CTM;
	if (value == "horizontal") return METHOD_HORIZONTAL;
	if (value == "vertical") return METHOD_VERTICAL;
	if (value == "top") return METHOD_TOP;
	if (value == "random") return METHOD_RANDOM;
	if (value == "repeat") return METHOD_REPEAT;
	return METHOD_NONE;
}

int_t ConnectedProperties::parseConnect(const std::string &value)
{
	if (value == "block") return CONNECT_BLOCK;
	if (value == "tile") return CONNECT_TILE;
	return CONNECT_NONE;
}

int_t ConnectedProperties::parseSymmetry(const std::string &value)
{
	if (value == "opposite") return SYMMETRY_OPPOSITE;
	if (value == "all") return SYMMETRY_ALL;
	return SYMMETRY_NONE;
}

int_t ConnectedProperties::parseFaces(const std::string &value)
{
	if (value.empty()) return FACE_ALL;
	std::string normalized = value;
	std::replace(normalized.begin(), normalized.end(), ',', ' ');
	std::istringstream input(normalized);
	std::string face;
	int_t mask = 0;
	while (input >> face)
	{
		if (face == "bottom") mask |= 1;
		else if (face == "top") mask |= 2;
		else if (face == "north") mask |= 4;
		else if (face == "south") mask |= 8;
		else if (face == "west") mask |= 16;
		else if (face == "east") mask |= 32;
		else if (face == "sides") mask |= FACE_SIDES;
		else if (face == "all") mask |= FACE_ALL;
	}
	return mask;
}

bool ConnectedProperties::isValid(const std::string &path)
{
	(void)path;
	if (source.empty() || method == METHOD_NONE)
		return false;
	for (int_t tile : tiles)
		if (tile < 0 || tile > 255) return false;

	switch (method)
	{
	case METHOD_CTM:
		if (tiles.empty()) tiles = OptiFineResource::parseIntList("0-11 16-27 32-43 48-59");
		return tiles.size() == 48;
	case METHOD_HORIZONTAL:
		if (tiles.empty()) tiles = OptiFineResource::parseIntList("12-15");
		return tiles.size() == 4;
	case METHOD_VERTICAL:
		return tiles.size() == 4;
	case METHOD_TOP:
		if (tiles.empty()) tiles.push_back(66);
		return tiles.size() == 1;
	case METHOD_RANDOM:
		if (tiles.empty() || (!weights.empty() && weights.size() != tiles.size())) return false;
		if (!weights.empty())
		{
			int_t sum = 0;
			sumWeights.clear();
			for (int_t weight : weights)
			{
				if (weight < 0) return false;
				sum += weight;
				sumWeights.push_back(sum);
			}
			sumAllWeights = sum;
			if (sumAllWeights <= 0) return false;
		}
		return true;
	case METHOD_REPEAT:
		return width > 0 && width <= 16 && height > 0 && height <= 16 && tiles.size() == static_cast<size_t>(width * height);
	default:
		return false;
	}
}
