#include "J_JdomParser.h"
#include "J_JsonListenerToJdomAdapter.h"
#include "J_SajParser.h"
#include "J_JsonRootNode.h"

J_JsonRootNode* J_JdomParser::parse(std::istream& in)
{
    J_JsonListenerToJdomAdapter adapter;
    J_SajParser sajParser;
    sajParser.parse(in, &adapter);
    return adapter.getResult();
}

J_JsonRootNode* J_JdomParser::parse(const std::string& s)
{
    std::istringstream iss(s);
    return parse(iss);
}
