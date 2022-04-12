#include "ColorCorrectionEffect.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"

ColorCorrectionEffect::ColorCorrectionEffect() :
	ColorCorrectionEffect(true) { }

ColorCorrectionEffect::ColorCorrectionEffect(bool defaultLut) :
	PostProcessingLayer::Effect(),
	_shader(nullptr),
	_strength(1.0f),
	Lut(nullptr)
{
	Name = "Color Correction";
	_format = RenderTargetType::ColorRgb8;

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/color_correction.glsl" }
	});

	lut_ori = ResourceManager::CreateAsset<Texture3D>("luts/cool.cube");
	lut_coo = ResourceManager::CreateAsset<Texture3D>("luts/COOL_LUT.CUBE");
	lut_war = ResourceManager::CreateAsset<Texture3D>("luts/WARM_LUT.CUBE");
	lut_cus = ResourceManager::CreateAsset<Texture3D>("luts/CUS_LUT.CUBE");

	if (defaultLut) {
		Lut = lut_ori;
	}
}

ColorCorrectionEffect::~ColorCorrectionEffect() = default;

void ColorCorrectionEffect::Apply(const Framebuffer::Sptr& gBuffer, const Framebuffer::Sptr& lBuffer)
{
	_shader->Bind();
	Lut->Bind(1);
	_shader->SetUniform("u_Strength", _strength);
}

void ColorCorrectionEffect::RenderImGui()
{
	if (ImGui::Button("Apply Original Lut"))
	{
		Lut = lut_ori;
	}

	if (ImGui::Button("Apply Cool Lut"))
	{
		Lut = lut_coo;
	}

	if (ImGui::Button("Apply Warm Lut"))
	{
		Lut = lut_war;
	}

	if (ImGui::Button("Apply Custom Lut"))
	{
		Lut = lut_cus;
	}
	LABEL_LEFT(ImGui::LabelText, "LUT", Lut ? Lut->GetDebugName().c_str() : "none");
	LABEL_LEFT(ImGui::SliderFloat, "Strength", &_strength, 0, 1);
}

ColorCorrectionEffect::Sptr ColorCorrectionEffect::FromJson(const nlohmann::json& data)
{
	ColorCorrectionEffect::Sptr result = std::make_shared<ColorCorrectionEffect>(false);
	result->Enabled = JsonGet(data, "enabled", true);
	result->_strength = JsonGet(data, "strength", result->_strength);
	result->Lut = ResourceManager::Get<Texture3D>(Guid(data["lut"].get<std::string>()));
	return result;
}

nlohmann::json ColorCorrectionEffect::ToJson() const
{
	return {
		{ "enabled", Enabled },
		{ "lut", Lut != nullptr ? Lut->GetGUID().str() : "null" }, 
		{ "strength", _strength }
	};
}
