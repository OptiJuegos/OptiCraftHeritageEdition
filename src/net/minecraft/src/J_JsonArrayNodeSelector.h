#pragma once

#include "J_LeafFunctor.h"

// net.minecraft.src.J_JsonArrayNodeSelector
// Matches a J_JsonNode that is an ARRAY and extracts its element list.
class J_JsonArrayNodeSelector : public J_LeafFunctor
{
public:
    J_JsonArrayNodeSelector() {}

    bool        matches(const SelectorValue& val) override;  // func_27058_a / func_27074_a
    std::string getName()                         override;  // func_27060_a
    std::string toString()                        override;

protected:
    SelectorValue extractValue(const SelectorValue& val) override;  // func_27063_c / func_27075_b
};
