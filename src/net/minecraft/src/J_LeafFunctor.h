#pragma once

#include "J_Functor.h"

// net.minecraft.src.J_LeafFunctor
// Abstract base that implements getValue() via matches()+extractValue(), throwing
// J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException when there is no match.
class J_LeafFunctor : public J_Functor
{
public:
    virtual ~J_LeafFunctor() {}

    // func_27059_b - calls extractValue if matches, otherwise throws
    SelectorValue getValue(const SelectorValue& val) override final;

protected:
    // func_27063_c - called by getValue when matches() returned true
    virtual SelectorValue extractValue(const SelectorValue& val) = 0;
};
