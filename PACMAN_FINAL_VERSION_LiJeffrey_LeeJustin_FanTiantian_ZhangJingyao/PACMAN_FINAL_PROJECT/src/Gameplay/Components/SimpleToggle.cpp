#include "SimpleToggle.h"

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/InputEngine.h"
#include "Application/Application.h"

#include "Gameplay/Components/Light.h"
#include "Gameplay/Scene.h"

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
#include "Gameplay/Components/ShaderContainer.h"
#include "Gameplay/Components/ShadowCamera.h"

SimpleToggle::SimpleToggle() :
	IComponent(),
	isLightOn(true),
	isTextureOn(true),
	isToggleOn(false)
{ }

SimpleToggle::~SimpleToggle() = default;

void SimpleToggle::Awake()
{
	_cdTimer = 0.0f;
	_renderer = GetComponent<RenderComponent>();
	if (_renderer != nullptr)
	{
		_shaderToStore = _renderer->GetMaterial()->GetShader();
	}
}

void SimpleToggle::Update(float deltaTime)
{
	if (Application::Get().IsFocused)
	{
		if (_cdTimer > 0.0f)
			_cdTimer -= deltaTime;

		if (InputEngine::IsKeyDown(GLFW_KEY_1)) // No lighting
		{
			if (_cdTimer <= 0.0f)
			{
				//Disable Light Calculation
				if (isToggleOn == false)
				{
					if (_renderer != NULL)
					{
						_renderer->GetMaterial()->SwapShader(_shaderContainer->Get<ShaderContainer>()->getShaderByName("BASI"));
						isToggleOn = true;
					}
				}
				else
				{
					if (_renderer != NULL)
					{
						_renderer->GetMaterial()->SwapShader(_shaderToStore);
						isToggleOn = false;
					}
				}
				_cdTimer = 1.0f;
			}
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_2)) // Ambient Lighting Only
		{
			if (_cdTimer <= 0.0f)
			{
				if (isToggleOn == false)
				{
					if (_renderer != NULL)
					{
						_renderer->GetMaterial()->SwapShader(_shaderContainer->Get<ShaderContainer>()->getShaderByName("BLIN"));
						isToggleOn = true;
					}
				}
				else
				{
					if (_renderer != NULL)
					{
						_renderer->GetMaterial()->SwapShader(_shaderToStore);
						isToggleOn = false;
					}
				}
				_cdTimer = 1.0f;
			}
		}
		
		if (InputEngine::IsKeyDown(GLFW_KEY_3)) // Specular Lighting Only
		{
			if (_cdTimer <= 0.0f)
			{
				if (isToggleOn == false)
				{
					if (_renderer != NULL)
					{
						_renderer->GetMaterial()->SwapShader(_shaderContainer->Get<ShaderContainer>()->getShaderByName("SPEC"));
						isToggleOn = true;
					}
				}
				else
				{
					if (_renderer != NULL)
					{
						_renderer->GetMaterial()->SwapShader(_shaderToStore);
						isToggleOn = false;
					}
				}
				_cdTimer = 1.0f;
			}
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_4)) // Ambient + Specular
		{
			if (_cdTimer <= 0.0f)
			{
				if (isToggleOn == false)
				{
					if (_renderer != NULL)
					{
						_renderer->GetMaterial()->SwapShader(_shaderToStore);
						isToggleOn = true;
					}
				}
				else
				{
					if (_renderer != NULL)
					{
						_renderer->GetMaterial()->SwapShader(_shaderToStore);
						isToggleOn = false;
					}
				}
				_cdTimer = 1.0f;
			}
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_5)) // Ambient + Specular + Custom
		{
			
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_6)) // Diffuse warp/ramp
		{

		}

		if (InputEngine::IsKeyDown(GLFW_KEY_7)) // specular warp/ramp
		{

		}

		if (InputEngine::IsKeyDown(GLFW_KEY_8) && toggleMode == 0) // Color grading warm
		{
			if (_cdTimer <= 0.0f)
			{
				_cdTimer = 1.0f;
			}
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_9)) // on/off texture
		{
			if (_cdTimer <= 0.0f)
			{
				if (toggleMode == 1 || toggleMode == 2 || toggleMode == 3)
				{
					if(isTextureOn)
					{
						_renderer->GetMaterial()->SwapShader(_shaderContainer->Get<ShaderContainer>()->getShaderByName("BASI"));
						isTextureOn = false;
					}
					else
					{
						_renderer->GetMaterial()->SwapShader(_shaderToStore);
						isTextureOn = true;
					}
				}
				_cdTimer = 1.0f;
			}
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_0)) // on/off light
		{
			if (_cdTimer <= 0.0f)
			{
				if (isLightOn)
				{
					if (toggleMode == 0)
					{
						GetGameObject()->Get<Light>()->IsEnabled = false;
					}
					if (toggleMode == 4)
					{
						GetGameObject()->Get<ShadowCamera>()->IsEnabled = false;
					}
					if (toggleMode == 1 || toggleMode == 2 || toggleMode == 3)
					{
						_renderer->GetMaterial()->SwapShader(_shaderContainer->Get<ShaderContainer>()->getShaderByName("BLIN"));
					}
					
					isLightOn = false;
				}
				else
				{
					if (toggleMode == 0)
					{
						GetGameObject()->Get<Light>()->IsEnabled = true;
					}
					if (toggleMode == 4)
					{
						GetGameObject()->Get<ShadowCamera>()->IsEnabled = true;
					}
					if (toggleMode == 1 || toggleMode == 2 || toggleMode == 3)
					{
						_renderer->GetMaterial()->SwapShader(_shaderToStore);
					}
					isLightOn = true;
				}
				
				_cdTimer = 1.0f;
			}
		}
	}
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
