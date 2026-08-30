#include "DataWatcher.h"

#include "WatchableObject.h"
#include "ItemStack.h"
#include "ChunkCoordinates.h"
#include "Packet.h"
#include "java/IOUtil.h"
#include <stdexcept>
#include <sstream>

DataWatcher::DataWatcher() :
    objectChanged(false)
{
}

DataWatcher::~DataWatcher()
{
    for (auto &entry : watchedObjects)
        delete entry.second;
}

// Returns the type id for a given std::any value (matching Java's dataTypes map)
int_t DataWatcher::getTypeId(const std::any &obj)
{
    if (obj.type() == typeid(byte_t))   return 0;
    if (obj.type() == typeid(short_t))  return 1;
    if (obj.type() == typeid(int_t))    return 2;
    if (obj.type() == typeid(float))    return 3;
    if (obj.type() == typeid(std::string)) return 4;
    if (obj.type() == typeid(ItemStack*))  return 5;
    if (obj.type() == typeid(ChunkCoordinates*)) return 6;

    std::ostringstream oss;
    oss << "Unknown data type: " << obj.type().name();
    throw std::invalid_argument(oss.str());
}

bool DataWatcher::hasObject(int_t id) const
{
    return watchedObjects.find(id) != watchedObjects.end();
}

void DataWatcher::addObject(int_t id, std::any obj)
{
    int_t typeId = getTypeId(obj);

    if (id > 31)
    {
        std::ostringstream oss;
        oss << "Data value id is too big with " << id << "! (Max is 31)";
        throw std::invalid_argument(oss.str());
    }

    if (watchedObjects.count(id))
    {
        std::ostringstream oss;
        oss << "Duplicate id value for " << id << "!";
        throw std::invalid_argument(oss.str());
    }

    watchedObjects[id] = new WatchableObject(typeId, id, std::move(obj));
    watchedObjectOrder.add(id);
}

byte_t DataWatcher::getWatchableObjectByte(int_t id)
{
    return std::any_cast<byte_t>(watchedObjects.at(id)->getObject());
}

short_t DataWatcher::getWatchableObjectShort(int_t id)
{
    return std::any_cast<short_t>(watchedObjects.at(id)->getObject());
}

int_t DataWatcher::getWatchableObjectInt(int_t id)
{
    return std::any_cast<int_t>(watchedObjects.at(id)->getObject());
}

std::string DataWatcher::getWatchableObjectString(int_t id)
{
    return std::any_cast<std::string>(watchedObjects.at(id)->getObject());
}

static bool anyEquals(const std::any &a, const std::any &b)
{
    if (a.type() != b.type()) return false;
    if (a.type() == typeid(byte_t))        return std::any_cast<byte_t>(a)        == std::any_cast<byte_t>(b);
    if (a.type() == typeid(short_t))       return std::any_cast<short_t>(a)       == std::any_cast<short_t>(b);
    if (a.type() == typeid(int_t))         return std::any_cast<int_t>(a)         == std::any_cast<int_t>(b);
    if (a.type() == typeid(float))         return std::any_cast<float>(a)         == std::any_cast<float>(b);
    if (a.type() == typeid(std::string))   return std::any_cast<std::string>(a)   == std::any_cast<std::string>(b);
    if (a.type() == typeid(ItemStack*))
        return std::any_cast<ItemStack*>(a) == std::any_cast<ItemStack*>(b);
    if (a.type() == typeid(ChunkCoordinates*))
    {
        ChunkCoordinates *left = std::any_cast<ChunkCoordinates*>(a);
        ChunkCoordinates *right = std::any_cast<ChunkCoordinates*>(b);
        if (left == right) return true;
        if (left == nullptr || right == nullptr) return false;
        return left->x == right->x && left->y == right->y && left->z == right->z;
    }
    return false;
}

void DataWatcher::updateObject(int_t id, std::any obj)
{
    WatchableObject *wo = watchedObjects.at(id);
    if (!anyEquals(obj, wo->getObject()))
    {
        wo->setObject(std::move(obj));
        wo->setWatching(true);
        objectChanged = true;
    }
}

void DataWatcher::writeWatchableObject(std::ostream &os, WatchableObject *wo)
{
    int_t header = ((wo->getObjectType() << 5) | (wo->getDataValueId() & 0x1f)) & 0xff;
    IOUtil::writeByte(os, header);

    switch (wo->getObjectType())
    {
    case 0:
        IOUtil::writeByte(os, std::any_cast<byte_t>(wo->getObject()));
        break;
    case 1:
        IOUtil::writeShort(os, std::any_cast<short_t>(wo->getObject()));
        break;
    case 2:
        IOUtil::writeInt(os, std::any_cast<int_t>(wo->getObject()));
        break;
    case 3:
        IOUtil::writeFloat(os, std::any_cast<float>(wo->getObject()));
        break;
    case 4:
    {
        jstring s = std::any_cast<std::string>(wo->getObject());
        Packet::writeString(s, os);
        break;
    }
    case 5:
    {
        ItemStack *stack = std::any_cast<ItemStack*>(wo->getObject());
        IOUtil::writeShort(os, stack->itemID);
        IOUtil::writeByte(os, stack->stackSize);
        IOUtil::writeShort(os, stack->getItemDamage());
        break;
    }
    case 6:
    {
        ChunkCoordinates *cc = std::any_cast<ChunkCoordinates*>(wo->getObject());
        IOUtil::writeInt(os, cc->x);
        IOUtil::writeInt(os, cc->y);
        IOUtil::writeInt(os, cc->z);
        break;
    }
    }
}

void DataWatcher::writeObjectsInListToStream(const std::vector<WatchableObject*> &list, std::ostream &os)
{
    for (WatchableObject *wo : list)
        writeWatchableObject(os, wo);
    IOUtil::writeByte(os, 127);
}

void DataWatcher::writeWatchableObjects(std::ostream &os)
{
    const std::vector<int_t> orderedIds = watchedObjectOrder.valuesInIterationOrder();
    for (int_t id : orderedIds)
    {
        auto it = watchedObjects.find(id);
        if (it != watchedObjects.end())
            writeWatchableObject(os, it->second);
    }
    IOUtil::writeByte(os, 127);
}

std::vector<WatchableObject*> DataWatcher::readWatchableObjects(std::istream &is)
{
    std::vector<WatchableObject*> list;
    try
    {
        int_t b = IOUtil::readUnsignedByte(is);
        while (b != 127)
        {
            int_t typeId  = (b & 0xe0) >> 5;
            int_t valueId = b & 0x1f;
            WatchableObject *wo = nullptr;

            switch (typeId)
            {
            case 0:
                wo = new WatchableObject(typeId, valueId, (byte_t)IOUtil::readByte(is));
                break;
            case 1:
                wo = new WatchableObject(typeId, valueId, (short_t)IOUtil::readShort(is));
                break;
            case 2:
                wo = new WatchableObject(typeId, valueId, (int_t)IOUtil::readInt(is));
                break;
            case 3:
                wo = new WatchableObject(typeId, valueId, (float)IOUtil::readFloat(is));
                break;
            case 4:
            {
                // std::any preserves the exact dynamic C++ type. jstring derives
                // from std::string, but any_cast<std::string> cannot extract a
                // stored jstring. Normalize protocol type 4 to std::string here.
                jstring value = Packet::readString(is, 64);
                wo = new WatchableObject(typeId, valueId,
                    std::string(static_cast<const std::string &>(value)));
                break;
            }
            case 5:
            {
                int_t itemId = IOUtil::readShort(is);
                int_t count = IOUtil::readByte(is);
                int_t damage = IOUtil::readShort(is);
                wo = new WatchableObject(typeId, valueId,
                    (ItemStack*)new ItemStack(itemId, count, damage));
                break;
            }
            case 6:
            {
                int_t x = IOUtil::readInt(is);
                int_t y = IOUtil::readInt(is);
                int_t z = IOUtil::readInt(is);
                wo = new WatchableObject(typeId, valueId,
                    (ChunkCoordinates*)new ChunkCoordinates(x, y, z));
                break;
            }
            default:
                throw std::runtime_error("Invalid data watcher type: " + std::to_string(typeId));
            }

            if (list.size() >= 32)
            {
                delete wo;
                throw std::runtime_error("Too many data watcher entries");
            }
            list.push_back(wo);
            b = IOUtil::readUnsignedByte(is);
        }
    }
    catch (...)
    {
        for (WatchableObject *obj : list)
            delete obj;
        throw;
    }
    return list;
}

static std::any cloneWatchableValue(const WatchableObject *wo)
{
    const std::any &obj = wo->getObject();
    switch (wo->getObjectType())
    {
    case 0: return std::any_cast<byte_t>(obj);
    case 1: return std::any_cast<short_t>(obj);
    case 2: return std::any_cast<int_t>(obj);
    case 3: return std::any_cast<float>(obj);
    case 4: return std::any_cast<std::string>(obj);
    case 5:
    {
        ItemStack *stack = std::any_cast<ItemStack*>(obj);
        return stack != nullptr ? std::any(stack->copy()) : std::any((ItemStack*)nullptr);
    }
    case 6:
    {
        ChunkCoordinates *cc = std::any_cast<ChunkCoordinates*>(obj);
        return cc != nullptr ? std::any(new ChunkCoordinates(cc)) : std::any((ChunkCoordinates*)nullptr);
    }
    default:
        return std::any();
    }
}

void DataWatcher::updateWatchedObjectsFromList(const std::vector<WatchableObject*> &list)
{
    for (WatchableObject *incoming : list)
    {
        if (incoming == nullptr)
            continue;
        auto it = watchedObjects.find(incoming->getDataValueId());
        if (it == watchedObjects.end() ||
            it->second->getObjectType() != incoming->getObjectType())
            continue;

        std::any value = cloneWatchableValue(incoming);
        if (value.has_value())
            it->second->setObject(std::move(value));
    }
}
