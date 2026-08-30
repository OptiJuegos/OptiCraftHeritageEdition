#pragma once

class ModelRenderer;

// net.minecraft.src.SignModel
class SignModel
{
public:
	SignModel();
	~SignModel();

	void renderSign();

	ModelRenderer *signBoard;
	ModelRenderer *signStick;
};
