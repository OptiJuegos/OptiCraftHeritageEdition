#pragma once

#include "J_LeafFunctor.h"

// net.minecraft.src.J_JsonElementNodeSelector
// Matches an array (SelectorValue::ARR) that has an element at a given index and extracts it.
class J_JsonElementNodeSelector : public J_LeafFunctor
{
public:
    explicit J_JsonElementNodeSelector(int index);

    bool        matches(const SelectorValue& val) override;  // func_27058_a / func_27067_a
    std::string getName()                         override;  // func_27060_a
    std::string toString()                        override;

protected:
    SelectorValue extractValue(const SelectorValue& val) override;  // func_27063_c / func_27068_b

private:
    int index;  // field_27069_a
};
