#include "DuoPlayerCamera.h"
#include <GLFW/glfw3.h>
#define  GLM_SWIZZLE

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"

#include "Application/Application.h"

DuoPlayerCamera::DuoPlayerCamera() :
	IComponent(),
	player1(nullptr),
	player2(nullptr),
	dCmrLookAt(glm::vec3(0.0f))
{ }

DuoPlayerCamera::~DuoPlayerCamera() = default;

void DuoPlayerCamera::Update(float deltaTime)
{
	if (player1 != nullptr && player2 != nullptr)
	{
		glm::vec3 p1Pos = player1->GetPosition();
		glm::vec3 p2Pos = player2->GetPosition();

		float maxDist = glm::distance(glm::vec3(100, 100, 0), glm::vec3(-100, -100, 0));
		float calcDist = glm::distance(p1Pos, p2Pos);

		glm::vec3 followPos = glm::vec3((p1Pos.x + p2Pos.x) / 2, (p1Pos.y + p2Pos.y) / 2, (p1Pos.z + p2Pos.z) / 2);
		dCmrLookAt = followPos;

		glm::vec3 camPos = GetGameObject()->GetPosition();

		GetGameObject()->LookAt(followPos);

		float camHoriDist = glm::distance(glm::vec3(GetGameObject()->GetPosition().x, GetGameObject()->GetPosition().y, 0.0f),
										glm::vec3(followPos.x, followPos.y, 0.0f));

		if (camHoriDist > 7) // MAX DISTANCE
		{
			camPos = GetGameObject()->GetPosition()
				+ glm::normalize(followPos - GetGameObject()->GetPosition()) * deltaTime
				* camHoriDist;
		}

		if (camHoriDist < 5) // MIN DISTANCE
		{
			camPos = GetGameObject()->GetPosition()
				+ glm::normalize(followPos - GetGameObject()->GetPosition()) * deltaTime * -1.0f
				* (10 - camHoriDist);
		}

		camPos.z = followPos.z + ((calcDist / maxDist) * (100 - followPos.z)) + 2.0f;

		std::cout << camPos.x << " " << camPos.y << " " << camPos.z << std::endl;

		float diffHeight = camPos.z - followPos.z;
		if (diffHeight < 2.0f) // MIN HETGHT
		{
			camPos.z += deltaTime * (2.0f - diffHeight);
		}

		GetGameObject()->SetPostion(camPos);
	}
}

void DuoPlayerCamera::RenderImGui()
{
}

nlohmann::json DuoPlayerCamera::ToJson() const
{
	return { };
}

DuoPlayerCamera::Sptr DuoPlayerCamera::FromJson(const nlohmann::json& blob)
{
	DuoPlayerCamera::Sptr result = std::make_shared<DuoPlayerCamera>();
	return result;
}
