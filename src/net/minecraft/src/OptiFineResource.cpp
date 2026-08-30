#include "OptiFineResource.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <memory>
#include <sstream>

#include "RenderEngine.h"

namespace OptiFineResource
{
	std::string trim(const std::string &value)
	{
		size_t begin = 0;
		while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin])))
			++begin;
		size_t end = value.size();
		while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1])))
			--end;
		return value.substr(begin, end - begin);
	}

	std::vector<std::string> split(const std::string &value, char delimiter)
	{
		std::vector<std::string> result;
		std::string item;
		std::istringstream input(value);
		while (std::getline(input, item, delimiter))
			result.push_back(trim(item));
		return result;
	}

	bool readText(RenderEngine *engine, const std::string &path, std::string &out)
	{
		out.clear();
		if (engine == nullptr)
			return false;
		std::unique_ptr<std::istream> input(engine->getResourceAsStream(path));
		if (input == nullptr || !*input)
			return false;
		std::ostringstream buffer;
		buffer << input->rdbuf();
		out = buffer.str();
		return true;
	}

	bool exists(RenderEngine *engine, const std::string &path)
	{
		return engine != nullptr && engine->hasResource(path);
	}

	std::map<std::string, std::string> readProperties(RenderEngine *engine, const std::string &path)
	{
		std::map<std::string, std::string> result;
		std::string text;
		if (!readText(engine, path, text))
			return result;

		std::istringstream input(text);
		std::string line;
		while (std::getline(input, line))
		{
			line = trim(line);
			if (line.empty() || line[0] == '#' || line[0] == '!')
				continue;
			size_t separator = line.find('=');
			if (separator == std::string::npos)
				separator = line.find(':');
			if (separator == std::string::npos)
				continue;
			const std::string key = trim(line.substr(0, separator));
			const std::string value = trim(line.substr(separator + 1));
			if (!key.empty())
				result[key] = value;
		}
		return result;
	}

	int_t parseInt(const std::string &value, int_t fallback)
	{
		char *end = nullptr;
		const long parsed = std::strtol(value.c_str(), &end, 10);
		return end != value.c_str() && *end == '\0' ? static_cast<int_t>(parsed) : fallback;
	}

	float parseFloat(const std::string &value, float fallback)
	{
		char *end = nullptr;
		const float parsed = std::strtof(value.c_str(), &end);
		return end != value.c_str() && *end == '\0' ? parsed : fallback;
	}

	int_t parseHexColor(const std::string &value, int_t fallback)
	{
		std::string clean = trim(value);
		if (!clean.empty() && clean[0] == '#')
			clean.erase(clean.begin());
		char *end = nullptr;
		const unsigned long parsed = std::strtoul(clean.c_str(), &end, 16);
		return end != clean.c_str() && *end == '\0' ? static_cast<int_t>(parsed & 0xffffffUL) : fallback;
	}

	std::vector<int_t> parseIntList(const std::string &value)
	{
		std::vector<int_t> result;
		std::string normalized = value;
		std::replace(normalized.begin(), normalized.end(), ',', ' ');
		std::istringstream input(normalized);
		std::string token;
		while (input >> token)
		{
			size_t dash = token.find('-');
			if (dash != std::string::npos)
			{
				int_t first = parseInt(token.substr(0, dash), -1);
				int_t last = parseInt(token.substr(dash + 1), -1);
				if (first >= 0 && last >= first && last - first <= 4096)
					for (int_t n = first; n <= last; ++n) result.push_back(n);
				continue;
			}
			int_t parsed = parseInt(token, -1);
			if (parsed >= 0)
				result.push_back(parsed);
		}
		return result;
	}
}
