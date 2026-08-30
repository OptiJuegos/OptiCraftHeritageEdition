#pragma once

#include <string>
#include <vector>

#include "J_JsonNode.h"
#include "J_JsonNodeSelector.h"

// net.minecraft.src.J_JsonNodeSelectors
// Factory for pre-built selectors that navigate a JSON tree along a path.
class J_JsonNodeSelectors
{
public:
    // func_27349_a - selector that navigates path[] and returns the final string value
    static J_JsonNodeSelector* stringAt(const std::vector<JsonPathElement>& path);

    // func_27346_b - selector that navigates path[] and returns the final array
    static J_JsonNodeSelector* arrayAt(const std::vector<JsonPathElement>& path);

    // func_27353_c - selector that navigates path[] and returns the final object
    static J_JsonNodeSelector* objectAt(const std::vector<JsonPathElement>& path);

    // func_27348_a / func_27350_a - select a field by name from an object map
    static J_JsonNodeSelector* fieldCalled(const std::string& name);

    // func_27351_b - object selector then field selector by name
    static J_JsonNodeSelector* objectFieldCalled(const std::string& name);

    // func_27347_a - element selector at a given index from an array
    static J_JsonNodeSelector* elementAt(int index);

    // func_27354_b - array selector then element selector at index
    static J_JsonNodeSelector* arrayElementAt(int index);

private:
    J_JsonNodeSelectors() {}

    // func_27352_a - build a chained selector that traverses path[] ending with the given tail
    static J_JsonNodeSelector* buildChain(
        const std::vector<JsonPathElement>& path,
        J_JsonNodeSelector* tail);

    // func_27345_a - chain two selectors
    static J_JsonNodeSelector* chainSelectors(
        J_JsonNodeSelector* head,
        J_JsonNodeSelector* tail);
};
