#pragma once

#include <map>
#include <string>
#include <vector>

#include "java/Type.h"

class ConnectedProperties
{
public:
	enum Method { METHOD_NONE=0, METHOD_CTM=1, METHOD_HORIZONTAL=2, METHOD_TOP=3, METHOD_RANDOM=4, METHOD_REPEAT=5, METHOD_VERTICAL=6 };
	enum Connect { CONNECT_NONE=0, CONNECT_BLOCK=1, CONNECT_TILE=2 };
	enum Face { FACE_BOTTOM=1, FACE_TOP=2, FACE_EAST=4, FACE_WEST=8, FACE_NORTH=16, FACE_SOUTH=32, FACE_SIDES=60, FACE_ALL=63 };
	enum Symmetry { SYMMETRY_NONE=1, SYMMETRY_OPPOSITE=2, SYMMETRY_ALL=6 };

	explicit ConnectedProperties(const std::map<std::string, std::string> &properties);
	bool isValid(const std::string &path);

	int_t method;
	std::string source;
	std::vector<int_t> tiles;
	int_t connect;
	int_t faces;
	std::vector<int_t> metadata;
	std::vector<int_t> weights;
	int_t symmetry;
	int_t width;
	int_t height;
	std::vector<int_t> sumWeights;
	int_t sumAllWeights;
	int_t textureId;

private:
	static int_t parseMethod(const std::string &value);
	static int_t parseConnect(const std::string &value);
	static int_t parseFaces(const std::string &value);
	static int_t parseSymmetry(const std::string &value);
	static std::string get(const std::map<std::string, std::string> &properties, const std::string &key);
};
