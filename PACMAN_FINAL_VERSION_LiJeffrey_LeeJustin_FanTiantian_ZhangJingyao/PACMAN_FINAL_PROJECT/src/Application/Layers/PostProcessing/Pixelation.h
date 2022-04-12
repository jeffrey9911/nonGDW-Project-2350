#pragma once
#include "Application/Layers/PostProcessingLayer.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture3D.h"

class Pixelation : public PostProcessingLayer::Effect {
public:
	MAKE_PTRS(Pixelation);

	Pixelation();
	virtual ~Pixelation();

	virtual void Apply(const Framebuffer::Sptr& gBuffer, const Framebuffer::Sptr& lBuffer) override;
	virtual void RenderImGui() override;

	Pixelation::Sptr FromJson(const nlohmann::json& data);
	virtual nlohmann::json ToJson() const override;

protected:
	ShaderProgram::Sptr _shader;
	
	float _amount;
};