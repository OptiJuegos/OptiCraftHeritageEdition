#include "J_JsonObjectNodeList.h"
#include "J_JsonStringNode.h"

J_JsonObjectNodeList::J_JsonObjectNodeList(J_JsonObjectNodeBuilder* builder)
{
    for (J_JsonFieldBuilder* field : J_JsonObjectNodeBuilder::getFields(builder))
    {
        (*this)[field->getKeyNode()->getText()] = field->getValueNode();
    }
}
