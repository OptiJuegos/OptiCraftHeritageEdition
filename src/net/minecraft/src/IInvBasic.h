#pragma once

class IInventory;

class IInvBasic {
public:
    virtual ~IInvBasic() = default;
    virtual void onInventoryChanged(IInventory* inventory) = 0;
};