#include "EnumOSMappingHelper.h"

int EnumOSMappingHelper::enumOSMappingArray[5] = {0, 0, 0, 0, 0};

void EnumOSMappingHelper::initialize()
{
    enumOSMappingArray[(int)EnumOS2::linux_]  = 1;
    enumOSMappingArray[(int)EnumOS2::solaris] = 2;
    enumOSMappingArray[(int)EnumOS2::windows] = 3;
    enumOSMappingArray[(int)EnumOS2::macos]   = 4;
}
