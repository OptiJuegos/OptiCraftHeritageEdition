#include "Material.h"

#include "MapColor.h"
#include "MaterialTransparent.h"
#include "MaterialLiquid.h"
#include "MaterialLogic.h"
#include "MaterialPortal.h"
#include "MaterialWeb.h"

Material::Material(MapColor *mapcolor) :
	materialMapColor(mapcolor)
{
	canHarvest = true;
}

void Material::cleanup()
{
	delete air; air = nullptr;
	delete grassMaterial; grassMaterial = nullptr;
	delete ground; ground = nullptr;
	delete wood; wood = nullptr;
	delete rock; rock = nullptr;
	delete iron; iron = nullptr;
	delete water; water = nullptr;
	delete lava; lava = nullptr;
	delete leaves; leaves = nullptr;
	delete plants; plants = nullptr;
	delete vine; vine = nullptr;
	delete sponge; sponge = nullptr;
	delete cloth; cloth = nullptr;
	delete fire; fire = nullptr;
	delete sand; sand = nullptr;
	delete circuits; circuits = nullptr;
	delete glass; glass = nullptr;
	delete redstoneLight; redstoneLight = nullptr;
	delete tnt; tnt = nullptr;
	delete spawnableCreatureList; spawnableCreatureList = nullptr;
	delete ice; ice = nullptr;
	delete snow; snow = nullptr;
	delete builtSnow; builtSnow = nullptr;
	delete cactus; cactus = nullptr;
	delete clay; clay = nullptr;
	delete pumpkin; pumpkin = nullptr;
	delete dragonEgg; dragonEgg = nullptr;
	delete portal; portal = nullptr;
	delete cakeMaterial; cakeMaterial = nullptr;
	delete web; web = nullptr;
	delete piston; piston = nullptr; pistonBase = nullptr; pistonSticky = nullptr;
}

bool Material::getIsLiquid()
{
	return false;
}

bool Material::isSolid()
{
	return true;
}

bool Material::getCanBlockGrass()
{
	return true;
}

bool Material::getIsSolid()
{
	return true;
}

bool Material::blocksMovement()
{
	return true;
}

bool Material::isLiquid()
{
	return getIsLiquid();
}

bool Material::getCanBurn()
{
	return getBurning();
}

Material *Material::setGroundCover()
{
	return setIsGroundCover();
}

bool Material::isGroundCover()
{
	return getIsGroundCover();
}

bool Material::isOpaque()
{
	return getIsTranslucent();
}

bool Material::isHarvestable()
{
	return getIsHarvestable();
}

Material *Material::setTranslucent()
{
	translucent = true;
	return this;
}

Material *Material::setIsTranslucent()
{
	return setTranslucent();
}

Material *Material::setNoHarvest()
{
	canHarvest = false;
	return this;
}

Material *Material::setBurning()
{
	canBurn = true;
	return this;
}

bool Material::getBurning()
{
	return canBurn;
}

Material *Material::setIsGroundCover()
{
	groundCover = true;
	return this;
}

bool Material::getIsGroundCover()
{
	return groundCover;
}

bool Material::getIsTranslucent()
{
	return !translucent && blocksMovement();
}

bool Material::getIsHarvestable()
{
	return canHarvest;
}

int_t Material::getMaterialMobility()
{
	return mobilityFlag;
}

Material *Material::setNoPushMobility()
{
	mobilityFlag = 1;
	return this;
}

Material *Material::setImmovableMobility()
{
	mobilityFlag = 2;
	return this;
}

Material *Material::air = nullptr;
Material *Material::grassMaterial = nullptr;
Material *Material::ground = nullptr;
Material *Material::wood = nullptr;
Material *Material::rock = nullptr;
Material *Material::iron = nullptr;
Material *Material::water = nullptr;
Material *Material::lava = nullptr;
Material *Material::leaves = nullptr;
Material *Material::plants = nullptr;
Material *Material::vine = nullptr;
Material *Material::sponge = nullptr;
Material *Material::cloth = nullptr;
Material *Material::fire = nullptr;
Material *Material::sand = nullptr;
Material *Material::circuits = nullptr;
Material *Material::glass = nullptr;
Material *Material::redstoneLight = nullptr;
Material *Material::tnt = nullptr;
Material *Material::spawnableCreatureList = nullptr;
Material *Material::ice = nullptr;
Material *Material::snow = nullptr;
Material *Material::builtSnow = nullptr;
Material *Material::cactus = nullptr;
Material *Material::clay = nullptr;
Material *Material::pumpkin = nullptr;
Material *Material::dragonEgg = nullptr;
Material *Material::portal = nullptr;
Material *Material::cakeMaterial = nullptr;
Material *Material::web = nullptr;
Material *Material::piston = nullptr;
Material *Material::pistonBase = nullptr;
Material *Material::pistonSticky = nullptr;

void Material::initialize()
{
	air = new MaterialTransparent(MapColor::airColor);
	grassMaterial = new Material(MapColor::grassColor);
	ground = new Material(MapColor::dirtColor);
	wood = (new Material(MapColor::woodColor))->setBurning();
	rock = (new Material(MapColor::stoneColor))->setNoHarvest();
	iron = (new Material(MapColor::ironColor))->setNoHarvest();
	water = (new MaterialLiquid(MapColor::waterColor))->setNoPushMobility();
	lava = (new MaterialLiquid(MapColor::tntColor))->setNoPushMobility();
	leaves = (new Material(MapColor::foliageColor))->setBurning()->setTranslucent()->setNoPushMobility();
	plants = (new MaterialLogic(MapColor::foliageColor))->setNoPushMobility();
	vine = (new MaterialLogic(MapColor::foliageColor))->setBurning()->setNoPushMobility()->setIsGroundCover();
	sponge = new Material(MapColor::clothColor);
	cloth = (new Material(MapColor::clothColor))->setBurning();
	fire = (new MaterialTransparent(MapColor::airColor))->setNoPushMobility();
	sand = new Material(MapColor::sandColor);
	circuits = (new MaterialLogic(MapColor::airColor))->setNoPushMobility();
	glass = (new Material(MapColor::airColor))->setTranslucent();
	redstoneLight = new Material(MapColor::airColor);
	tnt = (new Material(MapColor::tntColor))->setBurning()->setTranslucent();
	spawnableCreatureList = (new Material(MapColor::foliageColor))->setNoPushMobility();
	ice = (new Material(MapColor::iceColor))->setTranslucent();
	snow = (new MaterialLogic(MapColor::snowColor))->setIsGroundCover()->setTranslucent()->setNoHarvest()->setNoPushMobility();
	builtSnow = (new Material(MapColor::snowColor))->setNoHarvest();
	cactus = (new Material(MapColor::foliageColor))->setTranslucent()->setNoPushMobility();
	clay = new Material(MapColor::clayColor);
	pumpkin = (new Material(MapColor::foliageColor))->setNoPushMobility();
	dragonEgg = (new Material(MapColor::foliageColor))->setNoPushMobility();
	portal = (new MaterialPortal(MapColor::airColor))->setImmovableMobility();
	cakeMaterial = (new Material(MapColor::airColor))->setNoPushMobility();
	web = (new MaterialWeb(MapColor::clothColor))->setNoHarvest()->setNoPushMobility();
	piston = (new Material(MapColor::stoneColor))->setImmovableMobility();
	pistonBase = piston;
	pistonSticky = piston;
}
