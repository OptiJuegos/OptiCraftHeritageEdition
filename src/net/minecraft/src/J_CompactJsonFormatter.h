#pragma once

#include <string>
#include <sstream>

#include "J_JsonFormatter.h"

class J_JsonNode;

// net.minecraft.src.J_CompactJsonFormatter
// Serializes a JSON tree to a compact (no whitespace) string.
class J_CompactJsonFormatter : public J_JsonFormatter
{
public:
    J_CompactJsonFormatter() {}

    // func_27327_a - format to string
    std::string format(J_JsonRootNode* root) override;

    // func_27329_a - format to stream
    void format(J_JsonRootNode* root, std::ostream& out);

private:
    // func_27328_a - recursive formatting of any node
    void formatNode(J_JsonNode* node, std::ostream& out);
};
