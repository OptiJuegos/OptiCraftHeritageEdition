#pragma once

#include "J_Functor.h"
#include "J_JsonNodeSelector.h"

// net.minecraft.src.J_ChainedFunctor
// Functor that composes two selectors in sequence: first applies head, then tail.
class J_ChainedFunctor : public J_Functor
{
public:
    J_ChainedFunctor(J_JsonNodeSelector* head, J_JsonNodeSelector* tail);

    bool          matches(const SelectorValue& val)  override;  // func_27058_a
    SelectorValue getValue(const SelectorValue& val) override;  // func_27059_b
    std::string   getName()                          override;  // func_27060_a

    std::string toString() override;

private:
    J_JsonNodeSelector* head;  // field_27062_a
    J_JsonNodeSelector* tail;  // field_27061_b
};
