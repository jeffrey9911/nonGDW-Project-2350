#include "Depth.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"

Depth::Depth() :
	PostProcessingLayer::Effect(),
	_position(0.1f) 
{
	Name = "Depth of field Effect";

	_format = RenderTargetType::ColorRgb8;  

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/depth.glsl" }
	});
}

Depth::~Depth() = default;

void Depth::Apply(const Framebuffer::Sptr& gBuffer, const Framebuffer::Sptr& lBuffer)
{
	_shader->Bind();
	_shader->SetUniform("position", _position);
	gBuffer->BindAttachment(RenderTargetAttachment::Depth, 1);
}

void Depth::RenderImGui()
{
	LABEL_LEFT(ImGui::SliderFloat, "View Position", &_position, 0.0f, 1.0f);
}

Depth::Sptr Depth::FromJson(const nlohmann::json& data)
{
	Depth::Sptr result = std::make_shared<Depth>();
	return result;
}

nlohmann::json Depth::ToJson() const
{
	return {};
}
