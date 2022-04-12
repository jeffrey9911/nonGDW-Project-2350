#include "Pixelation.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/Material.h"

Pixelation::Pixelation() :
	PostProcessingLayer::Effect(),
	_amount(500.0f)
{
	Name = "Pixelation Effect";
	_format = RenderTargetType::ColorRgb8;
	
	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/pixelation.glsl" }
	});
}

Pixelation::~Pixelation() = default;

void Pixelation::Apply(const Framebuffer::Sptr& gBuffer, const Framebuffer::Sptr& lBuffer)
{  
	_shader->Bind();  
	_shader->SetUniform("u_amount", _amount);
}

void Pixelation::RenderImGui()
{
	LABEL_LEFT(ImGui::SliderFloat, "Pixelation Amount", &_amount, 0.0f, 500.0f);
}

Pixelation::Sptr Pixelation::FromJson(const nlohmann::json& data)
{
	Pixelation::Sptr result = std::make_shared<Pixelation>();
	return result;
}

nlohmann::json Pixelation::ToJson() const
{
	return { };
}
