#pragma once

#include <map>
#include <string>
#include <vector>

#include "java/Type.h"

class RenderEngine;

namespace OptiFineResource
{
	std::string trim(const std::string &value);
	std::vector<std::string> split(const std::string &value, char delimiter);
	bool readText(RenderEngine *engine, const std::string &path, std::string &out);
	bool exists(RenderEngine *engine, const std::string &path);
	std::map<std::string, std::string> readProperties(RenderEngine *engine, const std::string &path);
	int_t parseInt(const std::string &value, int_t fallback);
	float parseFloat(const std::string &value, float fallback);
	int_t parseHexColor(const std::string &value, int_t fallback);
	std::vector<int_t> parseIntList(const std::string &value);
}
