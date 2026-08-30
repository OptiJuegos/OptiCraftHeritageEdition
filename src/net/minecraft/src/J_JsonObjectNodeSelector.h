#pragma once

#include "J_LeafFunctor.h"

// net.minecraft.src.J_JsonObjectNodeSelector
// Matches a J_JsonNode that is an OBJECT and extracts its field map.
class J_JsonObjectNodeSelector : public J_LeafFunctor
{
public:
    J_JsonObjectNodeSelector() {}

    bool        matches(const SelectorValue& val) override;  // func_27058_a / func_27070_a
    std::string getName()                         override;  // func_27060_a
    std::string toString()                        override;

protected:
    SelectorValue extractValue(const SelectorValue& val) override;  // func_27063_c / func_27071_b
};
