#include "Blooming.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/Material.h"

Blooming::Blooming() :
	PostProcessingLayer::Effect(),
	_bloomSpread(0.2f),
	_bloomIntensity(2.0f)
{
	Name = "Blooming Effect";
	_format = RenderTargetType::ColorRgb8;

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/bloom.glsl" }
	});
}

Blooming::~Blooming() = default;

void Blooming::Apply(const Framebuffer::Sptr& gBuffer, const Framebuffer::Sptr& lBuffer)
{  
	_shader->Bind();  
	_shader->SetUniform("u_spread", _bloomSpread);
	_shader->SetUniform("u_intens", -1.0f * _bloomIntensity);
}

void Blooming::RenderImGui()
{
	LABEL_LEFT(ImGui::SliderFloat, "Bloom Spread", &_bloomSpread, -10.0f, 10.0f);
	LABEL_LEFT(ImGui::SliderFloat, "Bloom Intensity", &_bloomIntensity, -10.0f, 10.0f);
}

Blooming::Sptr Blooming::FromJson(const nlohmann::json& data)
{
	Blooming::Sptr result = std::make_shared<Blooming>();
	return result;
}

nlohmann::json Blooming::ToJson() const
{
	return { };
}
