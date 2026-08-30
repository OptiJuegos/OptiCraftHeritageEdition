#pragma once

#include <string>

#include "J_LeafFunctor.h"

// net.minecraft.src.J_JsonFieldNodeSelector
// Matches a field-map (SelectorValue::OBJ) that contains a given key and extracts the value.
class J_JsonFieldNodeSelector : public J_LeafFunctor
{
public:
    explicit J_JsonFieldNodeSelector(const std::string& fieldName);

    bool        matches(const SelectorValue& val) override;  // func_27058_a / func_27065_a
    std::string getName()                         override;  // func_27060_a
    std::string toString()                        override;

protected:
    SelectorValue extractValue(const SelectorValue& val) override;  // func_27063_c / func_27064_b

private:
    std::string fieldName;  // field_27066_a (was J_JsonStringNode, simplified to std::string)
};
