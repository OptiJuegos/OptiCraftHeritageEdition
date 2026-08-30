#include "J_JsonListenerToJdomAdapter.h"
#include "J_JsonRootNode.h"
#include "J_JsonNodeBuilders.h"
#include "J_JsonFieldBuilder.h"
#include "J_ArrayNodeContainer.h"
#include "J_ObjectNodeContainer.h"
#include "J_FieldNodeContainer.h"
#include "J_NodeContainer.h"

J_JsonListenerToJdomAdapter::J_JsonListenerToJdomAdapter()
    : rootBuilder(nullptr)
{
}

J_JsonRootNode* J_JsonListenerToJdomAdapter::getResult()
{
    return static_cast<J_JsonRootNode*>(rootBuilder->build());
}

void J_JsonListenerToJdomAdapter::startDocument()
{
}

void J_JsonListenerToJdomAdapter::endDocument()
{
}

void J_JsonListenerToJdomAdapter::startArray()
{
    J_JsonArrayNodeBuilder* arrayBuilder = J_JsonNodeBuilders::arrayBuilder();
    setOrAddBuilder(arrayBuilder);
    containerStack.push(new J_ArrayNodeContainer(this, arrayBuilder));
}

void J_JsonListenerToJdomAdapter::endArray()
{
    delete containerStack.top();
    containerStack.pop();
}

void J_JsonListenerToJdomAdapter::startObject()
{
    J_JsonObjectNodeBuilder* objectBuilder = J_JsonNodeBuilders::objectBuilder();
    setOrAddBuilder(objectBuilder);
    containerStack.push(new J_ObjectNodeContainer(this, objectBuilder));
}

void J_JsonListenerToJdomAdapter::endObject()
{
    delete containerStack.top();
    containerStack.pop();
}

void J_JsonListenerToJdomAdapter::startField(const std::string& name)
{
    J_JsonFieldBuilder* fieldBuilder = J_JsonFieldBuilder::create()->setKey(J_JsonNodeBuilders::stringBuilder(name));
    containerStack.top()->addFieldBuilder(fieldBuilder);
    containerStack.push(new J_FieldNodeContainer(this, fieldBuilder));
}

void J_JsonListenerToJdomAdapter::endField()
{
    delete containerStack.top();
    containerStack.pop();
}

void J_JsonListenerToJdomAdapter::numberValue(const std::string& s)
{
    addToCurrentContainer(J_JsonNodeBuilders::numberBuilder(s));
}

void J_JsonListenerToJdomAdapter::trueValue()
{
    addToCurrentContainer(J_JsonNodeBuilders::trueBuilder());
}

void J_JsonListenerToJdomAdapter::stringValue(const std::string& s)
{
    addToCurrentContainer(J_JsonNodeBuilders::stringBuilder(s));
}

void J_JsonListenerToJdomAdapter::falseValue()
{
    addToCurrentContainer(J_JsonNodeBuilders::falseBuilder());
}

void J_JsonListenerToJdomAdapter::nullValue()
{
    addToCurrentContainer(J_JsonNodeBuilders::nullBuilder());
}

void J_JsonListenerToJdomAdapter::setOrAddBuilder(J_JsonNodeBuilder* builder)
{
    if (rootBuilder == nullptr)
        rootBuilder = builder;
    else
        addToCurrentContainer(builder);
}

void J_JsonListenerToJdomAdapter::addToCurrentContainer(J_JsonNodeBuilder* builder)
{
    containerStack.top()->addNodeBuilder(builder);
}
