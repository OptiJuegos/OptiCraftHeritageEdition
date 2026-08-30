#pragma once

class EntityLiving;

// net.minecraft.src.EntityJumpHelper
class EntityJumpHelper
{
public:
	explicit EntityJumpHelper(EntityLiving *entity);
	void setJumping();
	void doJump();

private:
	EntityLiving *entity;
	bool jumping;
};
