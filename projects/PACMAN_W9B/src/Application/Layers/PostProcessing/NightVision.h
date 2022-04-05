#pragma once
#include "Application/Layers/PostProcessingLayer.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture3D.h"

class NightVision : public PostProcessingLayer::Effect {
public:
	MAKE_PTRS(NightVision);

	NightVision();
	virtual ~NightVision();

	virtual void Apply(const Framebuffer::Sptr& gBuffer, const Framebuffer::Sptr& lBuffer) override;
	virtual void RenderImGui() override;

	NightVision::Sptr FromJson(const nlohmann::json& data);
	virtual nlohmann::json ToJson() const override;

protected:
	ShaderProgram::Sptr _shader;

	glm::vec3 _nvVec;
};