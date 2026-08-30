#pragma once

#include "J_LeafFunctor.h"

// net.minecraft.src.J_JsonStringNodeSelector
// Matches a J_JsonNode that is a STRING and extracts its text value.
class J_JsonStringNodeSelector : public J_LeafFunctor
{
public:
    J_JsonStringNodeSelector() {}

    bool        matches(const SelectorValue& val) override;  // func_27058_a / func_27072_a
    std::string getName()                         override;  // func_27060_a
    std::string toString()                        override;

protected:
    SelectorValue extractValue(const SelectorValue& val) override;  // func_27063_c / func_27073_b
};
