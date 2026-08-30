#include "EnumJsonNodeTypeMappingHelper.h"

int EnumJsonNodeTypeMappingHelper::enumJsonNodeTypeMappingArray[7] = {0, 0, 0, 0, 0, 0, 0};

void EnumJsonNodeTypeMappingHelper::initialize()
{
	enumJsonNodeTypeMappingArray[(int)EnumJsonNodeType::ARRAY]  = 1;
	enumJsonNodeTypeMappingArray[(int)EnumJsonNodeType::OBJECT] = 2;
	enumJsonNodeTypeMappingArray[(int)EnumJsonNodeType::STRING] = 3;
	enumJsonNodeTypeMappingArray[(int)EnumJsonNodeType::NUMBER] = 4;
	enumJsonNodeTypeMappingArray[(int)EnumJsonNodeType::FALSE]  = 5;
	enumJsonNodeTypeMappingArray[(int)EnumJsonNodeType::TRUE]   = 6;
	enumJsonNodeTypeMappingArray[(int)EnumJsonNodeType::NULL_]  = 7;
}
