#include "EnumDoorHelper.h"

int_t EnumDoorHelper::value(EnumDoor door)
{
    switch (door)
    {
    case EnumDoor::OPENING: return 1;
    case EnumDoor::WOOD_DOOR: return 2;
    case EnumDoor::GRATES: return 3;
    case EnumDoor::IRON_DOOR: return 4;
    }
    return 0;
}
