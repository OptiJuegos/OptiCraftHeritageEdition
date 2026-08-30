#include "ItemSaddle.h"
#include "ItemStack.h"
#include "EntityLiving.h"
#include "EntityPig.h"

ItemSaddle::ItemSaddle(int i)
    : Item(i) {
    maxStackSize = 1;
}

void ItemSaddle::useItemOnEntity(ItemStack* itemstack, EntityLiving* entityliving) {
    EntityPig* entitypig = dynamic_cast<EntityPig*>(entityliving);
    if (entitypig != nullptr) {
        if (!entitypig->getSaddled() && !entitypig->isChild()) {
            entitypig->setSaddled(true);
            itemstack->stackSize--;
        }
    }
}

bool ItemSaddle::hitEntity(ItemStack* itemstack, EntityLiving* entityliving, EntityLiving* entityliving1) {
    useItemOnEntity(itemstack, entityliving);
    return true;
}

void ItemSaddle::saddleEntity(ItemStack* itemstack, EntityLiving* entityliving) {
    useItemOnEntity(itemstack, entityliving);
}
