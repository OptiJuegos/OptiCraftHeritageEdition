#include "Session.h"

#include "Block.h"
#include "BlockLeaves.h"
#include "BlockFlower.h"
#include "BlockMushroom.h"

std::vector<Block *> Session::registeredBlocksList;

Session::Session(const std::string &s, const std::string &s1)
{
	username = s;
	sessionId = s1;
}

void Session::initialize()
{
	registeredBlocksList.clear();
	registeredBlocksList.push_back(Block::stone);
	registeredBlocksList.push_back(Block::cobblestone);
	registeredBlocksList.push_back(Block::brick);
	registeredBlocksList.push_back(Block::dirt);
	registeredBlocksList.push_back(Block::planks);
	registeredBlocksList.push_back(Block::wood);
	registeredBlocksList.push_back(Block::leaves);
	registeredBlocksList.push_back(Block::torchWood);
	registeredBlocksList.push_back(Block::stairSingle);
	registeredBlocksList.push_back(Block::glass);
	registeredBlocksList.push_back(Block::cobblestoneMossy);
	registeredBlocksList.push_back(Block::sapling);
	registeredBlocksList.push_back(Block::plantYellow);
	registeredBlocksList.push_back(Block::plantRed);
	registeredBlocksList.push_back(Block::mushroomBrown);
	registeredBlocksList.push_back(Block::mushroomRed);
	registeredBlocksList.push_back(Block::sand);
	registeredBlocksList.push_back(Block::gravel);
	registeredBlocksList.push_back(Block::sponge);
	registeredBlocksList.push_back(Block::cloth);
	registeredBlocksList.push_back(Block::oreCoal);
	registeredBlocksList.push_back(Block::oreIron);
	registeredBlocksList.push_back(Block::oreGold);
	registeredBlocksList.push_back(Block::blockSteel);
	registeredBlocksList.push_back(Block::blockGold);
	registeredBlocksList.push_back(Block::bookShelf);
	registeredBlocksList.push_back(Block::tnt);
	registeredBlocksList.push_back(Block::obsidian);
}
