#include "J_JsonObject.h"

J_JsonObject::J_JsonObject(const std::map<std::string, J_JsonNode*>& f)
    : fields(f)
{
}

EnumJsonNodeType J_JsonObject::getType()
{
    return EnumJsonNodeType::OBJECT;
}

std::string J_JsonObject::getText()
{
    throw std::logic_error("Attempt to get text on a JsonNode without text.");
}

std::map<std::string, J_JsonNode*> J_JsonObject::getFields()
{
    return fields;
}

std::vector<J_JsonNode*> J_JsonObject::getElements()
{
    throw std::logic_error("Attempt to get elements on a JsonNode without elements.");
}

bool J_JsonObject::operator==(const J_JsonObject& other) const
{
    return fields == other.fields;
}

std::string J_JsonObject::toString() const
{
    std::string result = "JsonObject fields:[";
    bool first = true;
    for (auto& kv : fields)
    {
        if (!first) result += ", ";
        first = false;
        result += kv.first;
    }
    result += "]";
    return result;
}
