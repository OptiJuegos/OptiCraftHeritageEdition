#pragma once

#include <stack>

#include "J_JsonListener.h"
#include "J_JsonNodeBuilder.h"

class J_JsonRootNode;
class J_NodeContainer;

// net.minecraft.src.J_JsonListenerToJdomAdapter
// SAX-style listener that builds a JSON DOM tree as parsing events arrive.
class J_JsonListenerToJdomAdapter : public J_JsonListener
{
public:
    J_JsonListenerToJdomAdapter();

    // func_27208_a - return the fully built root node after parsing is complete
    J_JsonRootNode* getResult();

    void startDocument()                  override;  // func_27195_b
    void endDocument()                    override;  // func_27204_c
    void startArray()                     override;  // func_27200_d
    void endArray()                       override;  // func_27197_e
    void startObject()                    override;  // func_27194_f
    void endObject()                      override;  // func_27203_g
    void startField(const std::string& name) override;  // func_27205_a
    void endField()                       override;  // func_27199_h
    void stringValue(const std::string& s)  override;  // func_27198_c
    void numberValue(const std::string& s)  override;  // func_27201_b
    void trueValue()                      override;  // func_27196_i
    void falseValue()                     override;  // func_27193_j
    void nullValue()                      override;  // func_27202_k

private:
    // func_27207_a - store the builder as root if none yet, else add to current container
    void setOrAddBuilder(J_JsonNodeBuilder* builder);

    // func_27206_b - add a builder to the top container on the stack
    void addToCurrentContainer(J_JsonNodeBuilder* builder);

    std::stack<J_NodeContainer*> containerStack;  // field_27210_a
    J_JsonNodeBuilder*           rootBuilder;     // field_27209_b
};
