#pragma once

#include <vector>

class Random;
class StructureComponent;
class StructurePieceTreasure;

// net.minecraft.src.StructureMineshaftPieces
class StructureMineshaftPieces
{
public:
	static StructureComponent *getNextComponent(StructureComponent *parent,
	                                            std::vector<StructureComponent *> &components,
	                                            Random &random,
	                                            int x, int y, int z,
	                                            int direction, int depth);
	static const std::vector<StructurePieceTreasure> &getTreasurePieces();

private:
	static StructureComponent *getRandomComponent(std::vector<StructureComponent *> &components,
	                                              Random &random,
	                                              int x, int y, int z,
	                                              int direction, int depth);
	static StructureComponent *getNextMineShaftComponent(StructureComponent *parent,
	                                                     std::vector<StructureComponent *> &components,
	                                                     Random &random,
	                                                     int x, int y, int z,
	                                                     int direction, int depth);
};
