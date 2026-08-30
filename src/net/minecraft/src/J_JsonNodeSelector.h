#pragma once

#include <string>

#include "J_Functor.h"

class J_ChainedFunctor;

// net.minecraft.src.J_JsonNodeSelector
// Wraps a J_Functor and provides the public navigation API.
class J_JsonNodeSelector
{
public:
    explicit J_JsonNodeSelector(J_Functor* functor);
    ~J_JsonNodeSelector();

    // func_27356_a - returns true if this selector matches the given value
    bool matches(const SelectorValue& val);

    // func_27357_b - extracts the value from a matched node; throws if no match
    SelectorValue getValue(const SelectorValue& val);

    // func_27355_a - creates a chained selector: apply this, then the given one
    J_JsonNodeSelector* chain(J_JsonNodeSelector* next);

    // func_27358_a - human-readable description of the selector (from the functor)
    std::string getDescription();

    std::string toString();

    J_Functor* functor;  // field_27359_a
};
