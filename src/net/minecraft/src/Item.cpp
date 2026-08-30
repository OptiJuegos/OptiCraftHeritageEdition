#include "platform/Log.h"
#include "Item.h"
#include "ItemStack.h"
#include "ItemSpade.h"
#include "ItemPickaxe.h"
#include "ItemAxe.h"
#include "ItemSword.h"
#include "ItemHoe.h"
#include "ItemFood.h"
#include "Potion.h"
#include "PotionHelper.h"
#include "Enchantment.h"
#include "ItemBucketMilk.h"
#include "ItemAppleGold.h"
#include "ItemSoup.h"
#include "ItemSeeds.h"
#include "ItemArmor.h"
#include "ItemBow.h"
#include "ItemCoal.h"
#include "ItemFlintAndSteel.h"
#include "ItemPainting.h"
#include "ItemSign.h"
#include "ItemDoor.h"
#include "ItemBucket.h"
#include "ItemMinecart.h"
#include "ItemSaddle.h"
#include "ItemRedstone.h"
#include "ItemSnowball.h"
#include "ItemBoat.h"
#include "ItemReed.h"
#include "ItemEgg.h"
#include "ItemFishingRod.h"
#include "ItemDye.h"
#include "ItemBed.h"
#include "ItemCookie.h"
#include "ItemMap.h"
#include "ItemShears.h"
#include "ItemRecord.h"
#include "ItemPotion.h"
#include "ItemGlassBottle.h"
#include "ItemEnderPearl.h"
#include "ItemEnderEye.h"
#include "ItemMonsterPlacer.h"
#include "ItemExpBottle.h"
#include "ItemFireball.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "EntityLiving.h"
#include "World.h"
#include "StatCollector.h"
#include "StatList.h"
#include <iostream>
#include "Material.h"
#include "MathHelper.h"
#include "MovingObjectPosition.h"
#include "Vec3D.h"
#include <memory>

Item* Item::itemsList[Item::ITEM_LIST_SIZE] = { nullptr };
Random Item::itemRand;

Item* Item::shovelSteel = nullptr;
Item* Item::pickaxeSteel = nullptr;
Item* Item::axeSteel = nullptr;
Item* Item::flintAndSteel = nullptr;
Item* Item::appleRed = nullptr;
Item* Item::bow = nullptr;
Item* Item::arrow = nullptr;
Item* Item::coal = nullptr;
Item* Item::diamond = nullptr;
Item* Item::ingotIron = nullptr;
Item* Item::ingotGold = nullptr;
Item* Item::swordSteel = nullptr;
Item* Item::swordWood = nullptr;
Item* Item::shovelWood = nullptr;
Item* Item::pickaxeWood = nullptr;
Item* Item::axeWood = nullptr;
Item* Item::swordStone = nullptr;
Item* Item::shovelStone = nullptr;
Item* Item::pickaxeStone = nullptr;
Item* Item::axeStone = nullptr;
Item* Item::swordDiamond = nullptr;
Item* Item::shovelDiamond = nullptr;
Item* Item::pickaxeDiamond = nullptr;
Item* Item::axeDiamond = nullptr;
Item* Item::stick = nullptr;
Item* Item::bowlEmpty = nullptr;
Item* Item::bowlSoup = nullptr;
Item* Item::swordGold = nullptr;
Item* Item::shovelGold = nullptr;
Item* Item::pickaxeGold = nullptr;
Item* Item::axeGold = nullptr;
Item* Item::silk = nullptr;
Item* Item::feather = nullptr;
Item* Item::gunpowder = nullptr;
Item* Item::hoeWood = nullptr;
Item* Item::hoeStone = nullptr;
Item* Item::hoeSteel = nullptr;
Item* Item::hoeDiamond = nullptr;
Item* Item::hoeGold = nullptr;
Item* Item::seeds = nullptr;
Item* Item::wheat = nullptr;
Item* Item::bread = nullptr;
Item* Item::helmetLeather = nullptr;
Item* Item::plateLeather = nullptr;
Item* Item::legsLeather = nullptr;
Item* Item::bootsLeather = nullptr;
Item* Item::helmetChain = nullptr;
Item* Item::plateChain = nullptr;
Item* Item::legsChain = nullptr;
Item* Item::bootsChain = nullptr;
Item* Item::helmetSteel = nullptr;
Item* Item::plateSteel = nullptr;
Item* Item::legsSteel = nullptr;
Item* Item::bootsSteel = nullptr;
Item* Item::helmetDiamond = nullptr;
Item* Item::plateDiamond = nullptr;
Item* Item::legsDiamond = nullptr;
Item* Item::bootsDiamond = nullptr;
Item* Item::helmetGold = nullptr;
Item* Item::plateGold = nullptr;
Item* Item::legsGold = nullptr;
Item* Item::bootsGold = nullptr;
Item* Item::flint = nullptr;
Item* Item::porkRaw = nullptr;
Item* Item::porkCooked = nullptr;
Item* Item::painting = nullptr;
Item* Item::appleGold = nullptr;
Item* Item::sign = nullptr;
Item* Item::doorWood = nullptr;
Item* Item::bucketEmpty = nullptr;
Item* Item::bucketWater = nullptr;
Item* Item::bucketLava = nullptr;
Item* Item::minecartEmpty = nullptr;
Item* Item::saddle = nullptr;
Item* Item::doorSteel = nullptr;
Item* Item::redstone = nullptr;
Item* Item::snowball = nullptr;
Item* Item::boat = nullptr;
Item* Item::leather = nullptr;
Item* Item::bucketMilk = nullptr;
Item* Item::brick = nullptr;
Item* Item::clay = nullptr;
Item* Item::reed = nullptr;
Item* Item::paper = nullptr;
Item* Item::book = nullptr;
Item* Item::slimeBall = nullptr;
Item* Item::minecartCrate = nullptr;
Item* Item::minecartPowered = nullptr;
Item* Item::egg = nullptr;
Item* Item::compass = nullptr;
Item* Item::fishingRod = nullptr;
Item* Item::pocketSundial = nullptr;
Item* Item::lightStoneDust = nullptr;
Item* Item::fishRaw = nullptr;
Item* Item::fishCooked = nullptr;
Item* Item::dyePowder = nullptr;
Item* Item::bone = nullptr;
Item* Item::sugar = nullptr;
Item* Item::cake = nullptr;
Item* Item::bed = nullptr;
Item* Item::redstoneRepeater = nullptr;
Item* Item::cookie = nullptr;
Item* Item::mapItem = nullptr;
Item* Item::shears = nullptr;
Item* Item::melon = nullptr;
Item* Item::pumpkinSeeds = nullptr;
Item* Item::melonSeeds = nullptr;
Item* Item::netherStalkSeeds = nullptr;
Item* Item::beefRaw = nullptr;
Item* Item::beefCooked = nullptr;
Item* Item::chickenRaw = nullptr;
Item* Item::chickenCooked = nullptr;
Item* Item::rottenFlesh = nullptr;
Item* Item::enderPearl = nullptr;
Item* Item::blazeRod = nullptr;
Item* Item::ghastTear = nullptr;
Item* Item::goldNugget = nullptr;
Item* Item::potion = nullptr;
Item* Item::glassBottle = nullptr;
Item* Item::spiderEye = nullptr;
Item* Item::fermentedSpiderEye = nullptr;
Item* Item::blazePowder = nullptr;
Item* Item::magmaCream = nullptr;
Item* Item::brewingStand = nullptr;
Item* Item::cauldron = nullptr;
Item* Item::eyeOfEnder = nullptr;
Item* Item::speckledMelon = nullptr;
Item* Item::monsterPlacer = nullptr;
Item* Item::expBottle = nullptr;
Item* Item::fireballCharge = nullptr;
Item* Item::record13 = nullptr;
Item* Item::recordCat = nullptr;
Item* Item::recordBlocks = nullptr;
Item* Item::recordChirp = nullptr;
Item* Item::recordFar = nullptr;
Item* Item::recordMall = nullptr;
Item* Item::recordMellohi = nullptr;
Item* Item::recordStal = nullptr;
Item* Item::recordStrad = nullptr;
Item* Item::recordWard = nullptr;
Item* Item::record11 = nullptr;

Item::Item(int i) {
    maxStackSize = 64;
    maxDamage = 0;
    bFull3D = false;
    hasSubtypes = false;
    containerItem = nullptr;
    shiftedIndex = 256 + i;
    if (itemsList[256 + i] != nullptr) {
        MC_LOG_WARN("item", "CONFLICT @ %d\n", i);
    }
    itemsList[256 + i] = this;
}

Item* Item::setIconIndex(int i) {
    iconIndex = i;
    return this;
}

Item* Item::setMaxStackSize(int i) {
    maxStackSize = i;
    return this;
}

Item* Item::setIconCoord(int i, int j) {
    iconIndex = i + j * 16;
    return this;
}

int Item::getIconFromDamage(int i) {
    return iconIndex;
}

int Item::getIconIndex(ItemStack* itemstack) {
    return getIconFromDamage(itemstack->getItemDamage());
}

bool Item::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    return false;
}

float Item::getStrVsBlock(ItemStack* itemstack, Block* block) {
    return 1.0f;
}

ItemStack* Item::onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
    return itemstack;
}

ItemStack* Item::onFoodEaten(ItemStack* itemstack, World*, EntityPlayer*) {
    return itemstack;
}

EnumAction Item::getItemUseAction(ItemStack*) {
    return EnumAction::none;
}

int Item::getMaxItemUseDuration(ItemStack*) {
    return 0;
}

void Item::onPlayerStoppedUsing(ItemStack*, World*, EntityPlayer*, int) {
}

int Item::getItemStackLimit() {
    return maxStackSize;
}

int Item::getMetadata(int) {
    return 0;
}

int Item::getPlacedBlockMetadata(int damage) {
    return getMetadata(damage);
}

bool Item::getHasSubtypes() {
    return hasSubtypes;
}

Item* Item::setHasSubtypes(bool flag) {
    hasSubtypes = flag;
    return this;
}

int Item::getMaxDamage() {
    return maxDamage;
}

Item* Item::setMaxDamage(int i) {
    maxDamage = i;
    return this;
}

bool Item::isDamageable() {
    return maxDamage > 0 && !hasSubtypes;
}

bool Item::isDamagable() {
    return isDamageable();
}

bool Item::hitEntity(ItemStack* itemstack, EntityLiving* entityliving, EntityLiving* entityliving1) {
    return false;
}

bool Item::onBlockDestroyed(ItemStack* itemstack, int i, int j, int k, int l, EntityLiving* entityliving) {
    return false;
}

int Item::getDamageVsEntity(Entity* entity) {
    return 1;
}

bool Item::canHarvestBlock(Block* block) {
    return false;
}

void Item::useItemOnEntity(ItemStack*, EntityLiving*) {
}

void Item::saddleEntity(ItemStack* itemstack, EntityLiving* entityliving) {
    useItemOnEntity(itemstack, entityliving);
}

Item* Item::setFull3D() {
    bFull3D = true;
    return this;
}

bool Item::isFull3D() {
    return bFull3D;
}

bool Item::shouldRotateAroundWhenRendering() {
    return false;
}

Item* Item::setItemName(const char* s) {
    itemName = s;
    return this;
}

std::string Item::getItemName() {
    return itemName ? std::string("item.") + itemName : "";
}

std::string Item::getItemNameIS(ItemStack*) {
    return getItemName();
}

std::string Item::getLocalItemName(ItemStack* itemstack) {
    const std::string name = getItemNameIS(itemstack);
    return name.empty() ? std::string() : StatCollector::translateToLocal(name);
}

std::string Item::getItemDisplayName(ItemStack* itemstack) {
    return StatCollector::translateToLocal(getItemNameIS(itemstack) + ".name");
}

void Item::addInformation(ItemStack*, std::vector<std::string>&) {
}

Item* Item::setContainerItem(Item* item) {
    if (maxStackSize > 1) {
        throw std::invalid_argument("Max stack size must be 1 for items with crafting results");
    }
    containerItem = item;
    return this;
}

Item* Item::getContainerItem() {
    return containerItem;
}

bool Item::doesContainerItemLeaveCraftingGrid(ItemStack*) {
    return true;
}

bool Item::hasContainerItem() {
    return containerItem != nullptr;
}

std::string Item::getStatName() {
    return StatCollector::translateToLocal(getItemName() + ".name");
}

int Item::getColorFromDamage(int i) {
    return 0xffffff;
}

int Item::getColorFromDamage(int damage, int) {
    return getColorFromDamage(damage);
}

void Item::onUpdate(ItemStack* itemstack, World* world, Entity* entity, int i, bool flag) {
}

void Item::onCreated(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
}

bool Item::hasEffect(ItemStack* itemstack) {
    return itemstack != nullptr && itemstack->isItemEnchanted();
}

const EnumRarity& Item::getRarity(ItemStack* itemstack) {
    return itemstack != nullptr && itemstack->isItemEnchanted() ? EnumRarity::rare : EnumRarity::common;
}

bool Item::isItemTool(ItemStack*) {
    return getItemStackLimit() == 1 && isDamageable();
}


bool Item::func_46056_k() {
    return false;
}

int Item::getItemEnchantability() {
    return 0;
}

Item* Item::setPotionEffect(const std::string& effect) {
    potionEffect = effect;
    return this;
}

const std::string& Item::getPotionEffect() const {
    return potionEffect;
}

bool Item::isPotionIngredient() const {
    return !potionEffect.empty();
}

bool Item::func_46058_c() {
    return false;
}

int Item::func_46057_a(int damage, int) {
    return getIconFromDamage(damage);
}

MovingObjectPosition* Item::getMovingObjectPositionFromPlayer(World* world, EntityPlayer* player, bool includeLiquids) {
    const float partialTick = 1.0f;
    const float pitch = player->prevRotationPitch + (player->rotationPitch - player->prevRotationPitch) * partialTick;
    const float yaw = player->prevRotationYaw + (player->rotationYaw - player->prevRotationYaw) * partialTick;
    const double x = player->prevPosX + (player->posX - player->prevPosX) * static_cast<double>(partialTick);
    const double y = player->prevPosY + (player->posY - player->prevPosY) * static_cast<double>(partialTick) + 1.62 - static_cast<double>(player->yOffset);
    const double z = player->prevPosZ + (player->posZ - player->prevPosZ) * static_cast<double>(partialTick);
    Vec3D* start = Vec3D::createVector(x, y, z);
    const float cosYaw = MathHelper::cos(-yaw * 0.017453292f - 3.1415927f);
    const float sinYaw = MathHelper::sin(-yaw * 0.017453292f - 3.1415927f);
    const float cosPitch = -MathHelper::cos(-pitch * 0.017453292f);
    const float sinPitch = MathHelper::sin(-pitch * 0.017453292f);
    const float dirX = sinYaw * cosPitch;
    const float dirZ = cosYaw * cosPitch;
    const double reach = 5.0;
    Vec3D* end = start->addVector(static_cast<double>(dirX) * reach,
                                  static_cast<double>(sinPitch) * reach,
                                  static_cast<double>(dirZ) * reach);
    return world->rayTraceBlocks(start, end, includeLiquids, !includeLiquids);
}

void Item::initItems() {
    Potion::initPotions();
    Enchantment::initEnchantments();
    shovelSteel = (new ItemSpade(0, EnumToolMaterial::IRON))->setIconCoord(2, 5)->setItemName("shovelIron");
    pickaxeSteel = (new ItemPickaxe(1, EnumToolMaterial::IRON))->setIconCoord(2, 6)->setItemName("pickaxeIron");
    axeSteel = (new ItemAxe(2, EnumToolMaterial::IRON))->setIconCoord(2, 7)->setItemName("hatchetIron");
    flintAndSteel = (new ItemFlintAndSteel(3))->setIconCoord(5, 0)->setItemName("flintAndSteel");
    appleRed = (new ItemFood(4, 4, 0.3f, false))->setIconCoord(10, 0)->setItemName("apple");
    bow = (new ItemBow(5))->setIconCoord(5, 1)->setItemName("bow");
    arrow = (new Item(6))->setIconCoord(5, 2)->setItemName("arrow");
    coal = (new ItemCoal(7))->setIconCoord(7, 0)->setItemName("coal");
    diamond = (new Item(8))->setIconCoord(7, 3)->setItemName("emerald");
    ingotIron = (new Item(9))->setIconCoord(7, 1)->setItemName("ingotIron");
    ingotGold = (new Item(10))->setIconCoord(7, 2)->setItemName("ingotGold");
    swordSteel = (new ItemSword(11, EnumToolMaterial::IRON))->setIconCoord(2, 4)->setItemName("swordIron");
    swordWood = (new ItemSword(12, EnumToolMaterial::WOOD))->setIconCoord(0, 4)->setItemName("swordWood");
    shovelWood = (new ItemSpade(13, EnumToolMaterial::WOOD))->setIconCoord(0, 5)->setItemName("shovelWood");
    pickaxeWood = (new ItemPickaxe(14, EnumToolMaterial::WOOD))->setIconCoord(0, 6)->setItemName("pickaxeWood");
    axeWood = (new ItemAxe(15, EnumToolMaterial::WOOD))->setIconCoord(0, 7)->setItemName("hatchetWood");
    swordStone = (new ItemSword(16, EnumToolMaterial::STONE))->setIconCoord(1, 4)->setItemName("swordStone");
    shovelStone = (new ItemSpade(17, EnumToolMaterial::STONE))->setIconCoord(1, 5)->setItemName("shovelStone");
    pickaxeStone = (new ItemPickaxe(18, EnumToolMaterial::STONE))->setIconCoord(1, 6)->setItemName("pickaxeStone");
    axeStone = (new ItemAxe(19, EnumToolMaterial::STONE))->setIconCoord(1, 7)->setItemName("hatchetStone");
    swordDiamond = (new ItemSword(20, EnumToolMaterial::EMERALD))->setIconCoord(3, 4)->setItemName("swordDiamond");
    shovelDiamond = (new ItemSpade(21, EnumToolMaterial::EMERALD))->setIconCoord(3, 5)->setItemName("shovelDiamond");
    pickaxeDiamond = (new ItemPickaxe(22, EnumToolMaterial::EMERALD))->setIconCoord(3, 6)->setItemName("pickaxeDiamond");
    axeDiamond = (new ItemAxe(23, EnumToolMaterial::EMERALD))->setIconCoord(3, 7)->setItemName("hatchetDiamond");
    stick = (new Item(24))->setIconCoord(5, 3)->setFull3D()->setItemName("stick");
    bowlEmpty = (new Item(25))->setIconCoord(7, 4)->setItemName("bowl");
    bowlSoup = (new ItemSoup(26, 8))->setIconCoord(8, 4)->setItemName("mushroomStew");
    swordGold = (new ItemSword(27, EnumToolMaterial::GOLD))->setIconCoord(4, 4)->setItemName("swordGold");
    shovelGold = (new ItemSpade(28, EnumToolMaterial::GOLD))->setIconCoord(4, 5)->setItemName("shovelGold");
    pickaxeGold = (new ItemPickaxe(29, EnumToolMaterial::GOLD))->setIconCoord(4, 6)->setItemName("pickaxeGold");
    axeGold = (new ItemAxe(30, EnumToolMaterial::GOLD))->setIconCoord(4, 7)->setItemName("hatchetGold");
    silk = (new Item(31))->setIconCoord(8, 0)->setItemName("string");
    feather = (new Item(32))->setIconCoord(8, 1)->setItemName("feather");
    gunpowder = (new Item(33))->setIconCoord(8, 2)->setItemName("sulphur")->setPotionEffect(PotionHelper::gunpowderEffect);
    hoeWood = (new ItemHoe(34, EnumToolMaterial::WOOD))->setIconCoord(0, 8)->setItemName("hoeWood");
    hoeStone = (new ItemHoe(35, EnumToolMaterial::STONE))->setIconCoord(1, 8)->setItemName("hoeStone");
    hoeSteel = (new ItemHoe(36, EnumToolMaterial::IRON))->setIconCoord(2, 8)->setItemName("hoeIron");
    hoeDiamond = (new ItemHoe(37, EnumToolMaterial::EMERALD))->setIconCoord(3, 8)->setItemName("hoeDiamond");
    hoeGold = (new ItemHoe(38, EnumToolMaterial::GOLD))->setIconCoord(4, 8)->setItemName("hoeGold");
    seeds = (new ItemSeeds(39, Block::crops->blockID, Block::tilledField->blockID))->setIconCoord(9, 0)->setItemName("seeds");
    wheat = (new Item(40))->setIconCoord(9, 1)->setItemName("wheat");
    bread = (new ItemFood(41, 5, 0.6f, false))->setIconCoord(9, 2)->setItemName("bread");
    helmetLeather = (new ItemArmor(42, EnumArmorMaterial::CLOTH, 0, 0))->setIconCoord(0, 0)->setItemName("helmetCloth");
    plateLeather = (new ItemArmor(43, EnumArmorMaterial::CLOTH, 0, 1))->setIconCoord(0, 1)->setItemName("chestplateCloth");
    legsLeather = (new ItemArmor(44, EnumArmorMaterial::CLOTH, 0, 2))->setIconCoord(0, 2)->setItemName("leggingsCloth");
    bootsLeather = (new ItemArmor(45, EnumArmorMaterial::CLOTH, 0, 3))->setIconCoord(0, 3)->setItemName("bootsCloth");
    helmetChain = (new ItemArmor(46, EnumArmorMaterial::CHAIN, 1, 0))->setIconCoord(1, 0)->setItemName("helmetChain");
    plateChain = (new ItemArmor(47, EnumArmorMaterial::CHAIN, 1, 1))->setIconCoord(1, 1)->setItemName("chestplateChain");
    legsChain = (new ItemArmor(48, EnumArmorMaterial::CHAIN, 1, 2))->setIconCoord(1, 2)->setItemName("leggingsChain");
    bootsChain = (new ItemArmor(49, EnumArmorMaterial::CHAIN, 1, 3))->setIconCoord(1, 3)->setItemName("bootsChain");
    helmetSteel = (new ItemArmor(50, EnumArmorMaterial::IRON, 2, 0))->setIconCoord(2, 0)->setItemName("helmetIron");
    plateSteel = (new ItemArmor(51, EnumArmorMaterial::IRON, 2, 1))->setIconCoord(2, 1)->setItemName("chestplateIron");
    legsSteel = (new ItemArmor(52, EnumArmorMaterial::IRON, 2, 2))->setIconCoord(2, 2)->setItemName("leggingsIron");
    bootsSteel = (new ItemArmor(53, EnumArmorMaterial::IRON, 2, 3))->setIconCoord(2, 3)->setItemName("bootsIron");
    helmetDiamond = (new ItemArmor(54, EnumArmorMaterial::DIAMOND, 3, 0))->setIconCoord(3, 0)->setItemName("helmetDiamond");
    plateDiamond = (new ItemArmor(55, EnumArmorMaterial::DIAMOND, 3, 1))->setIconCoord(3, 1)->setItemName("chestplateDiamond");
    legsDiamond = (new ItemArmor(56, EnumArmorMaterial::DIAMOND, 3, 2))->setIconCoord(3, 2)->setItemName("leggingsDiamond");
    bootsDiamond = (new ItemArmor(57, EnumArmorMaterial::DIAMOND, 3, 3))->setIconCoord(3, 3)->setItemName("bootsDiamond");
    helmetGold = (new ItemArmor(58, EnumArmorMaterial::GOLD, 4, 0))->setIconCoord(4, 0)->setItemName("helmetGold");
    plateGold = (new ItemArmor(59, EnumArmorMaterial::GOLD, 4, 1))->setIconCoord(4, 1)->setItemName("chestplateGold");
    legsGold = (new ItemArmor(60, EnumArmorMaterial::GOLD, 4, 2))->setIconCoord(4, 2)->setItemName("leggingsGold");
    bootsGold = (new ItemArmor(61, EnumArmorMaterial::GOLD, 4, 3))->setIconCoord(4, 3)->setItemName("bootsGold");
    flint = (new Item(62))->setIconCoord(6, 0)->setItemName("flint");
    porkRaw = (new ItemFood(63, 3, 0.3f, true))->setIconCoord(7, 5)->setItemName("porkchopRaw");
    porkCooked = (new ItemFood(64, 8, 0.8f, true))->setIconCoord(8, 5)->setItemName("porkchopCooked");
    painting = (new ItemPainting(65))->setIconCoord(10, 1)->setItemName("painting");
    appleGold = (new ItemAppleGold(66, 4, 1.2f, false))->setAlwaysEdible()->setPotionEffect(Potion::regeneration->id, 5, 0, 1.0f)->setIconCoord(11, 0)->setItemName("appleGold");
    sign = (new ItemSign(67))->setIconCoord(10, 2)->setItemName("sign");
    doorWood = (new ItemDoor(68, Material::wood))->setIconCoord(11, 2)->setItemName("doorWood");
    bucketEmpty = (new ItemBucket(69, 0))->setIconCoord(10, 4)->setItemName("bucket");
    bucketWater = (new ItemBucket(70, Block::waterMoving->blockID))->setIconCoord(11, 4)->setItemName("bucketWater")->setContainerItem(bucketEmpty);
    bucketLava = (new ItemBucket(71, Block::lavaMoving->blockID))->setIconCoord(12, 4)->setItemName("bucketLava")->setContainerItem(bucketEmpty);
    minecartEmpty = (new ItemMinecart(72, 0))->setIconCoord(7, 8)->setItemName("minecart");
    saddle = (new ItemSaddle(73))->setIconCoord(8, 6)->setItemName("saddle");
    doorSteel = (new ItemDoor(74, Material::iron))->setIconCoord(12, 2)->setItemName("doorIron");
    redstone = (new ItemRedstone(75))->setIconCoord(8, 3)->setItemName("redstone")->setPotionEffect(PotionHelper::redstoneEffect);
    snowball = (new ItemSnowball(76))->setIconCoord(14, 0)->setItemName("snowball");
    boat = (new ItemBoat(77))->setIconCoord(8, 8)->setItemName("boat");
    leather = (new Item(78))->setIconCoord(7, 6)->setItemName("leather");
    bucketMilk = (new ItemBucketMilk(79))->setIconCoord(13, 4)->setItemName("milk")->setContainerItem(bucketEmpty);
    brick = (new Item(80))->setIconCoord(6, 1)->setItemName("brick");
    clay = (new Item(81))->setIconCoord(9, 3)->setItemName("clay");
    reed = (new ItemReed(82, Block::reed))->setIconCoord(11, 1)->setItemName("reeds");
    paper = (new Item(83))->setIconCoord(10, 3)->setItemName("paper");
    book = (new Item(84))->setIconCoord(11, 3)->setItemName("book");
    slimeBall = (new Item(85))->setIconCoord(14, 1)->setItemName("slimeball");
    minecartCrate = (new ItemMinecart(86, 1))->setIconCoord(7, 9)->setItemName("minecartChest");
    minecartPowered = (new ItemMinecart(87, 2))->setIconCoord(7, 10)->setItemName("minecartFurnace");
    egg = (new ItemEgg(88))->setIconCoord(12, 0)->setItemName("egg");
    compass = (new Item(89))->setIconCoord(6, 3)->setItemName("compass");
    fishingRod = (new ItemFishingRod(90))->setIconCoord(5, 4)->setItemName("fishingRod");
    pocketSundial = (new Item(91))->setIconCoord(6, 4)->setItemName("clock");
    lightStoneDust = (new Item(92))->setIconCoord(9, 4)->setItemName("yellowDust")->setPotionEffect(PotionHelper::glowstoneEffect);
    fishRaw = (new ItemFood(93, 2, 0.3f, false))->setIconCoord(9, 5)->setItemName("fishRaw");
    fishCooked = (new ItemFood(94, 5, 0.6f, false))->setIconCoord(10, 5)->setItemName("fishCooked");
    dyePowder = (new ItemDye(95))->setIconCoord(14, 4)->setItemName("dyePowder");
    bone = (new Item(96))->setIconCoord(12, 1)->setItemName("bone")->setFull3D();
    sugar = (new Item(97))->setIconCoord(13, 0)->setItemName("sugar")->setPotionEffect(PotionHelper::sugarEffect);
    cake = (new ItemReed(98, Block::cake))->setMaxStackSize(1)->setIconCoord(13, 1)->setItemName("cake");
    bed = (new ItemBed(99))->setMaxStackSize(1)->setIconCoord(13, 2)->setItemName("bed");
    redstoneRepeater = (new ItemReed(100, Block::redstoneRepeaterIdle))->setIconCoord(6, 5)->setItemName("diode");
    cookie = (new ItemFood(101, 1, 0.1f, false))->setIconCoord(12, 5)->setItemName("cookie");
    mapItem = (ItemMap*)(new ItemMap(102))->setIconCoord(12, 3)->setItemName("map");
    shears = (ItemShears*)(new ItemShears(103))->setIconCoord(13, 5)->setItemName("shears");
    melon = (new ItemFood(104, 2, 0.3f, false))->setIconCoord(13, 6)->setItemName("melon");
    pumpkinSeeds = (new ItemSeeds(105, Block::pumpkinStem->blockID, Block::tilledField->blockID))->setIconCoord(13, 3)->setItemName("seeds_pumpkin");
    melonSeeds = (new ItemSeeds(106, Block::melonStem->blockID, Block::tilledField->blockID))->setIconCoord(14, 3)->setItemName("seeds_melon");
    beefRaw = (new ItemFood(107, 3, 0.3f, true))->setIconCoord(9, 6)->setItemName("beefRaw");
    beefCooked = (new ItemFood(108, 8, 0.8f, true))->setIconCoord(10, 6)->setItemName("beefCooked");
    chickenRaw = (new ItemFood(109, 2, 0.3f, true))->setPotionEffect(Potion::hunger->id, 30, 0, 0.3f)->setIconCoord(9, 7)->setItemName("chickenRaw");
    chickenCooked = (new ItemFood(110, 6, 0.6f, true))->setIconCoord(10, 7)->setItemName("chickenCooked");
    rottenFlesh = (new ItemFood(111, 4, 0.1f, true))->setPotionEffect(Potion::hunger->id, 30, 0, 0.8f)->setIconCoord(11, 5)->setItemName("rottenFlesh");
    enderPearl = (new ItemEnderPearl(112))->setIconCoord(11, 6)->setItemName("enderPearl");
    blazeRod = (new Item(113))->setIconCoord(12, 6)->setItemName("blazeRod");
    ghastTear = (new Item(114))->setIconCoord(11, 7)->setItemName("ghastTear")->setPotionEffect(PotionHelper::ghastTearEffect);
    goldNugget = (new Item(115))->setIconCoord(12, 7)->setItemName("goldNugget");
    netherStalkSeeds = (new ItemSeeds(116, Block::netherStalk->blockID, Block::slowSand->blockID))->setIconCoord(13, 7)->setItemName("netherStalkSeeds")->setPotionEffect("+4");
    potion = (new ItemPotion(117))->setIconCoord(13, 8)->setItemName("potion");
    glassBottle = (new ItemGlassBottle(118))->setIconCoord(12, 8)->setItemName("glassBottle");
    spiderEye = (new ItemFood(119, 2, 0.8f, false))->setPotionEffect(Potion::poison->id, 5, 0, 1.0f)->setIconCoord(11, 8)->setItemName("spiderEye")->setPotionEffect(PotionHelper::spiderEyeEffect);
    fermentedSpiderEye = (new Item(120))->setIconCoord(10, 8)->setItemName("fermentedSpiderEye")->setPotionEffect(PotionHelper::fermentedSpiderEyeEffect);
    blazePowder = (new Item(121))->setIconCoord(13, 9)->setItemName("blazePowder")->setPotionEffect(PotionHelper::blazePowderEffect);
    magmaCream = (new Item(122))->setIconCoord(13, 10)->setItemName("magmaCream")->setPotionEffect(PotionHelper::magmaCreamEffect);
    brewingStand = (new ItemReed(123, Block::brewingStand))->setIconCoord(12, 10)->setItemName("brewingStand");
    cauldron = (new ItemReed(124, Block::cauldron))->setIconCoord(12, 9)->setItemName("cauldron");
    eyeOfEnder = (new ItemEnderEye(125))->setIconCoord(11, 9)->setItemName("eyeOfEnder");
    speckledMelon = (new Item(126))->setIconCoord(9, 8)->setItemName("speckledMelon")->setPotionEffect(PotionHelper::speckledMelonEffect);
    monsterPlacer = (new ItemMonsterPlacer(127))->setIconCoord(9, 9)->setItemName("monsterPlacer");
    expBottle = (new ItemExpBottle(128))->setIconCoord(11, 10)->setItemName("expBottle");
    fireballCharge = (new ItemFireball(129))->setIconCoord(14, 2)->setItemName("fireball");
    record13 = (new ItemRecord(2000, "13"))->setIconCoord(0, 15)->setItemName("record");
    recordCat = (new ItemRecord(2001, "cat"))->setIconCoord(1, 15)->setItemName("record");
    recordBlocks = (new ItemRecord(2002, "blocks"))->setIconCoord(2, 15)->setItemName("record");
    recordChirp = (new ItemRecord(2003, "chirp"))->setIconCoord(3, 15)->setItemName("record");
    recordFar = (new ItemRecord(2004, "far"))->setIconCoord(4, 15)->setItemName("record");
    recordMall = (new ItemRecord(2005, "mall"))->setIconCoord(5, 15)->setItemName("record");
    recordMellohi = (new ItemRecord(2006, "mellohi"))->setIconCoord(6, 15)->setItemName("record");
    recordStal = (new ItemRecord(2007, "stal"))->setIconCoord(7, 15)->setItemName("record");
    recordStrad = (new ItemRecord(2008, "strad"))->setIconCoord(8, 15)->setItemName("record");
    recordWard = (new ItemRecord(2009, "ward"))->setIconCoord(9, 15)->setItemName("record");
    record11 = (new ItemRecord(2010, "11"))->setIconCoord(10, 15)->setItemName("record");
    StatList::initItemStats(); // func_25151_b
}

void Item::cleanup()
{
    Enchantment::cleanup();
    for (int i = 0; i < ITEM_LIST_SIZE; i++)
    {
        if (itemsList[i] != nullptr)
        {
            delete itemsList[i];
            itemsList[i] = nullptr;
        }
    }
    shovelSteel = nullptr;
    pickaxeSteel = nullptr;
    axeSteel = nullptr;
    flintAndSteel = nullptr;
    appleRed = nullptr;
    bow = nullptr;
    arrow = nullptr;
    coal = nullptr;
    diamond = nullptr;
    ingotIron = nullptr;
    ingotGold = nullptr;
    swordSteel = nullptr;
    swordWood = nullptr;
    shovelWood = nullptr;
    pickaxeWood = nullptr;
    axeWood = nullptr;
    swordStone = nullptr;
    shovelStone = nullptr;
    pickaxeStone = nullptr;
    axeStone = nullptr;
    swordDiamond = nullptr;
    shovelDiamond = nullptr;
    pickaxeDiamond = nullptr;
    axeDiamond = nullptr;
    stick = nullptr;
    bowlEmpty = nullptr;
    bowlSoup = nullptr;
    swordGold = nullptr;
    shovelGold = nullptr;
    pickaxeGold = nullptr;
    axeGold = nullptr;
    silk = nullptr;
    feather = nullptr;
    gunpowder = nullptr;
    hoeWood = nullptr;
    hoeStone = nullptr;
    hoeSteel = nullptr;
    hoeDiamond = nullptr;
    hoeGold = nullptr;
    seeds = nullptr;
    wheat = nullptr;
    bread = nullptr;
    helmetLeather = nullptr;
    plateLeather = nullptr;
    legsLeather = nullptr;
    bootsLeather = nullptr;
    helmetChain = nullptr;
    plateChain = nullptr;
    legsChain = nullptr;
    bootsChain = nullptr;
    helmetSteel = nullptr;
    plateSteel = nullptr;
    legsSteel = nullptr;
    bootsSteel = nullptr;
    helmetDiamond = nullptr;
    plateDiamond = nullptr;
    legsDiamond = nullptr;
    bootsDiamond = nullptr;
    helmetGold = nullptr;
    plateGold = nullptr;
    legsGold = nullptr;
    bootsGold = nullptr;
    flint = nullptr;
    porkRaw = nullptr;
    porkCooked = nullptr;
    painting = nullptr;
    appleGold = nullptr;
    sign = nullptr;
    doorWood = nullptr;
    bucketEmpty = nullptr;
    bucketWater = nullptr;
    bucketLava = nullptr;
    minecartEmpty = nullptr;
    saddle = nullptr;
    doorSteel = nullptr;
    redstone = nullptr;
    snowball = nullptr;
    boat = nullptr;
    leather = nullptr;
    bucketMilk = nullptr;
    brick = nullptr;
    clay = nullptr;
    reed = nullptr;
    paper = nullptr;
    book = nullptr;
    slimeBall = nullptr;
    minecartCrate = nullptr;
    minecartPowered = nullptr;
    egg = nullptr;
    compass = nullptr;
    fishingRod = nullptr;
    pocketSundial = nullptr;
    lightStoneDust = nullptr;
    fishRaw = nullptr;
    fishCooked = nullptr;
    dyePowder = nullptr;
    bone = nullptr;
    sugar = nullptr;
    cake = nullptr;
    bed = nullptr;
    redstoneRepeater = nullptr;
    cookie = nullptr;
    mapItem = nullptr;
    shears = nullptr;
    melon = nullptr;
    pumpkinSeeds = nullptr;
    melonSeeds = nullptr;
    netherStalkSeeds = nullptr;
    record13 = nullptr;
    recordCat = nullptr;
}

