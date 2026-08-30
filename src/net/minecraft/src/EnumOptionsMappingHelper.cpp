#include "EnumOptionsMappingHelper.h"

int EnumOptionsMappingHelper::enumOptionsMappingHelperArray[13] = {
	0, 0, 1, 0, 0, 2, 3, 4, 0, 0, 0, 5, 0
};

void EnumOptionsMappingHelper::initialize()
{
    enumOptionsMappingHelperArray[EnumOptions::INVERT_MOUSE->returnEnumOrdinal()]    = 1;
    enumOptionsMappingHelperArray[EnumOptions::VIEW_BOBBING->returnEnumOrdinal()]    = 2;
    enumOptionsMappingHelperArray[EnumOptions::ANAGLYPH->returnEnumOrdinal()]        = 3;
    enumOptionsMappingHelperArray[EnumOptions::ADVANCED_OPENGL->returnEnumOrdinal()] = 4;
    enumOptionsMappingHelperArray[EnumOptions::AMBIENT_OCCLUSION->returnEnumOrdinal()] = 5;
}
