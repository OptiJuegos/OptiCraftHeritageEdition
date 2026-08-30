#pragma once

#include <string>

// net.minecraft.src.J_JsonListener
class J_JsonListener
{
public:
    virtual ~J_JsonListener() {}

    virtual void startDocument()            = 0;  // func_27195_b
    virtual void endDocument()              = 0;  // func_27204_c
    virtual void startArray()               = 0;  // func_27200_d
    virtual void endArray()                 = 0;  // func_27197_e
    virtual void startObject()              = 0;  // func_27194_f
    virtual void endObject()                = 0;  // func_27203_g
    virtual void startField(const std::string& name) = 0;  // func_27205_a
    virtual void endField()                 = 0;  // func_27199_h
    virtual void stringValue(const std::string& s) = 0;  // func_27198_c
    virtual void numberValue(const std::string& s) = 0;  // func_27201_b
    virtual void trueValue()                = 0;  // func_27196_i
    virtual void falseValue()               = 0;  // func_27193_j
    virtual void nullValue()                = 0;  // func_27202_k
};
