#pragma once

#include "J_JsonNodeBuilder.h"
#include "J_JsonNullNodeBuilder.h"
#include "J_JsonTrueNodeBuilder.h"
#include "J_JsonFalseNodeBuilder.h"
#include "J_JsonNumberNodeBuilder.h"
#include "J_JsonStringNodeBuilder.h"
#include "J_JsonObjectNodeBuilder.h"
#include "J_JsonArrayNodeBuilder.h"

// net.minecraft.src.J_JsonNodeBuilders
// Static factory for all node builders.
class J_JsonNodeBuilders
{
public:
    // func_27248_a
    static J_JsonNodeBuilder* nullBuilder()
    {
        return new J_JsonNullNodeBuilder();
    }

    // func_27251_b
    static J_JsonNodeBuilder* trueBuilder()
    {
        return new J_JsonTrueNodeBuilder();
    }

    // func_27252_c
    static J_JsonNodeBuilder* falseBuilder()
    {
        return new J_JsonFalseNodeBuilder();
    }

    // func_27250_a
    static J_JsonNodeBuilder* numberBuilder(const std::string& s)
    {
        return new J_JsonNumberNodeBuilder(s);
    }

    // func_27254_b
    static J_JsonStringNodeBuilder* stringBuilder(const std::string& s)
    {
        return new J_JsonStringNodeBuilder(s);
    }

    // func_27253_d
    static J_JsonObjectNodeBuilder* objectBuilder()
    {
        return new J_JsonObjectNodeBuilder();
    }

    // func_27249_e
    static J_JsonArrayNodeBuilder* arrayBuilder()
    {
        return new J_JsonArrayNodeBuilder();
    }

private:
    J_JsonNodeBuilders() {}
};
