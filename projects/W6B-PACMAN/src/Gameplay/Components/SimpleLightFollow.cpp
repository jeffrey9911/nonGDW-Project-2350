#include "SimpleLightFollow.h"
#include <GLFW/glfw3.h>
#define  GLM_SWIZZLE

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/Components/Light.h"
#include <GLM/gtc/random.hpp>

#include "Application/Application.h"

SimpleLightFollow::SimpleLightFollow() :
	IComponent(),
	followGObj(NULL)
{ }

SimpleLightFollow::~SimpleLightFollow() = default;

void SimpleLightFollow::setFollowObj(Gameplay::GameObject::Sptr gObj)
{
	followGObj = gObj;
}

void SimpleLightFollow::Update(float deltaTime)
{
	if (followGObj != NULL)
	{
		glm::vec3 objPos = followGObj->GetPosition();

		GetGameObject()->SetPostion(glm::vec3(objPos.x, objPos.y, objPos.z));

		glm::vec3 color = glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f));
		GetGameObject()->Get<Light>()->SetColor(color);
	}
}

void SimpleLightFollow::RenderImGui()
{
}

nlohmann::json SimpleLightFollow::ToJson() const
{
	return { };
}

SimpleLightFollow::Sptr SimpleLightFollow::FromJson(const nlohmann::json& blob)
{
	SimpleLightFollow::Sptr result = std::make_shared<SimpleLightFollow>();
	return result;
}
