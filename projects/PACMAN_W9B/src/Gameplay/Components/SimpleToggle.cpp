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

SimpleToggle::SimpleToggle() :
	IComponent(),
	isToggleOn(false)
{ }

SimpleToggle::~SimpleToggle() = default;

void SimpleToggle::Awake()
{
	_cdTimer = 0.0f;
	_renderer = GetComponent<RenderComponent>();
	if (_renderer != NULL)
	{
		_shaderToStore = _renderer->GetMaterial()->GetShader();
	}
	_lutToStore = GetGameObject()->GetScene()->GetColorLUT();
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
			/*
			if (_cdTimer <= 0.0f)
			{
				if (isToggleOn == false)
				{
					GetGameObject()->GetScene()->SetColorLUT(_shaderContainer->Get<ShaderContainer>()->getD3TexByName("LUT_WARM"));
				}
				else
				{
					GetGameObject()->GetScene()->SetColorLUT(_shaderContainer->Get<ShaderContainer>()->getD3TexByName("LUT_NORM"));
				}
				_cdTimer = 1.0f;
			}*/
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_9) && toggleMode == 0) // Color graing cool
		{
			/*
			if (_cdTimer <= 0.0f)
			{
				if (isToggleOn == false)
				{
					GetGameObject()->GetScene()->SetColorLUT(_shaderContainer->Get<ShaderContainer>()->getD3TexByName("LUT_COOL"));
				}
				else
				{
					GetGameObject()->GetScene()->SetColorLUT(_shaderContainer->Get<ShaderContainer>()->getD3TexByName("LUT_NORM"));
				}
				_cdTimer = 1.0f;
			}*/
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_0) && toggleMode == 0) // Color grading Custom
		{
			/*
			if (_cdTimer <= 0.0f)
			{
				if (isToggleOn == false)
				{
					GetGameObject()->GetScene()->SetColorLUT(_shaderContainer->Get<ShaderContainer>()->getD3TexByName("LUT_CUS"));
				}
				else
				{
					GetGameObject()->GetScene()->SetColorLUT(_shaderContainer->Get<ShaderContainer>()->getD3TexByName("LUT_NORM"));
				}
				_cdTimer = 1.0f;
			}*/
		}

		
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
