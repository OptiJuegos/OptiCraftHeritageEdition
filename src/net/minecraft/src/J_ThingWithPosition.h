#pragma once

// net.minecraft.src.J_ThingWithPosition
class J_ThingWithPosition
{
public:
    virtual ~J_ThingWithPosition() {}
    virtual int getColumn() = 0;  // func_27331_a
    virtual int getLine()   = 0;  // func_27330_b
};
