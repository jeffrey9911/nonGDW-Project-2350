#include "NightVision.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/Material.h"

NightVision::NightVision() :
	PostProcessingLayer::Effect(),
	_nvVec(glm::vec3(0.8f, 0.3f, 0.1f))
{
	Name = "Night Vision Effect";
	_format = RenderTargetType::ColorRgb8;

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/night_vision.glsl" }
	});
}

NightVision::~NightVision() = default;

void NightVision::Apply(const Framebuffer::Sptr& gBuffer, const Framebuffer::Sptr& lBuffer)
{
	_shader->Bind();
	_shader->SetUniform("u_nvVec", _nvVec);
}

void NightVision::RenderImGui()
{
	LABEL_LEFT(ImGui::SliderFloat, "Night Vision Vector X", &_nvVec.x, -10.0f, 10.0f);
	LABEL_LEFT(ImGui::SliderFloat, "Night Vision Vector y", &_nvVec.y, -10.0f, 10.0f);
	LABEL_LEFT(ImGui::SliderFloat, "Night Vision Vector Z", &_nvVec.z, -10.0f, 10.0f);

}

NightVision::Sptr NightVision::FromJson(const nlohmann::json& data)
{
	return NightVision::Sptr();
}

nlohmann::json NightVision::ToJson() const
{
	return nlohmann::json();
}
