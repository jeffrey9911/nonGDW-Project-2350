#include "SimpleToggle.h"

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/InputEngine.h"
#include "Application/Application.h"

#include "Gameplay/Components/Light.h"

// Graphics
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"
#include "Graphics/Framebuffer.h"

#include "Utils/ResourceManager/ResourceManager.h"
#include "Gameplay/Material.h"
#include "Gameplay/Components/RenderComponent.h"

SimpleToggle::SimpleToggle() :
	IComponent(),
	toggleNoL(false),
	toggleABL(false),
	toggleSPL(false),
	toggleAnS(false),
	toggleTexture(false)
{ }

SimpleToggle::~SimpleToggle() = default;

//Gameplay::Material::Sptr ambientLightMat;

/*
void SimpleToggle::Awake()
{
	_matStore = GetGameObject()->Get<RenderComponent>()->GetMaterial();

	ShaderProgram::Sptr ambientLightShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
	});

	ambientLightMat = ResourceManager::CreateAsset<Gameplay::Material>(ambientLightShader);
	{
		ambientLightMat->Name = "Ambient Light Material";

		ambientLightMat->Set("u_Material.Shininess", 1.0f);
	}
}
*/


void SimpleToggle::Update(float deltaTime)
{
	if (Application::Get().IsFocused)
	{
		if (InputEngine::IsKeyDown(GLFW_KEY_1)) // No lighting
		{
			toggleNoL = !toggleNoL;
			toggleABL = false;
			toggleSPL = false;
			toggleAnS = false;
			toggleTexture = false;
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_2)) // Ambient Lighting Only
		{
			toggleNoL = false;
			toggleABL = !toggleABL;
			toggleSPL = false;
			toggleAnS = false;
			toggleTexture = false;
		}
		
		if (InputEngine::IsKeyDown(GLFW_KEY_3)) // Specular Lighting Only
		{
			toggleNoL = false;
			toggleABL = false;
			toggleSPL = !toggleSPL;
			toggleAnS = false;
			toggleTexture = false;
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_4)) // Ambient + Specular
		{
			toggleNoL = false;
			toggleABL = false;
			toggleSPL = false;
			toggleAnS = !toggleAnS;
			toggleTexture = false;
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_5)) // Ambient + Specular
		{
			toggleNoL = false;
			toggleABL = false;
			toggleSPL = false;
			toggleAnS = false;
			toggleTexture = !toggleTexture;
		}
	}

	if (GetGameObject()->Get<Light>() != NULL)
	{
		GetGameObject()->Get<Light>()->IsEnabled = !toggleNoL;
	}

	/*
	if (toggleABL) 
	{
		if (GetGameObject()->Get<RenderComponent>() != NULL)
		{
			_matStore = GetGameObject()->Get<RenderComponent>()->GetMaterial();
			GetGameObject()->Get<RenderComponent>()->SetMaterial(ambientLightMat);
		}
	}
	else
	{
		if (GetGameObject()->Get<RenderComponent>() != NULL)
		{
			GetGameObject()->Get<RenderComponent>()->SetMaterial(_matStore);
		}
	}*/
	
}

void SimpleToggle::RenderImGui()
{
}

nlohmann::json SimpleToggle::ToJson() const
{
	return { };
}

SimpleToggle::Sptr SimpleToggle::FromJson(const nlohmann::json& blob)
{
	SimpleToggle::Sptr result = std::make_shared<SimpleToggle>();
	return result;
}
