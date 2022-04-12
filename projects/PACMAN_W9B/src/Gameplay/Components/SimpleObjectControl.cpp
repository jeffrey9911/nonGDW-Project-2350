#include "Gameplay/Components/SimpleObjectControl.h"
#include <GLFW/glfw3.h>
#define  GLM_SWIZZLE
#include <GLM/gtc/quaternion.hpp>

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/InputEngine.h"
#include "Application/Application.h"

#include "Gameplay/Physics/RigidBody.h"

#include "Gameplay/Components/DuoPlayerCamera.h"

#include <iostream>

SimpleObjectControl::SimpleObjectControl() :
	IComponent(),
	_moveSpeed(3.0f),
	playerInd(0)
{}

SimpleObjectControl::~SimpleObjectControl() = default;

void SimpleObjectControl::setCamera(Gameplay::GameObject::Sptr cmr)
{
	cameraAttached = cmr;
}

void SimpleObjectControl::Update(float deltaTime)
{
	if (Application::Get().IsFocused && playerInd == 1) {
		
		// ONLY when free camera is in use
		if (!InputEngine::IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {}
		//glm::vec3 _forwardVector = GetGameObject()->GetPosition() - cameraAttached->GetPosition();
		glm::vec3 _forwardVector = cameraAttached->Get<DuoPlayerCamera>()->dCmrLookAt - cameraAttached->GetPosition();
		_forwardVector = glm::normalize(_forwardVector);

		glm::quat rotX;
		glm::vec3 _moveVector = glm::vec3(0.0f);

		Gameplay::Physics::RigidBody::Sptr physics = GetGameObject()->Get<Gameplay::Physics::RigidBody>();

		if (InputEngine::IsKeyDown(GLFW_KEY_W)) {
			_moveVector = _forwardVector * _moveSpeed;
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_S)) {
			rotX = glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1));
			_moveVector = rotX * _forwardVector * _moveSpeed;
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_A)) {
			rotX = glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 0, 1));
			_moveVector = rotX * _forwardVector * _moveSpeed;
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_D)) {
			rotX = glm::angleAxis(glm::radians(-90.0f), glm::vec3(0, 0, 1));
			_moveVector = rotX * _forwardVector * _moveSpeed;
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
			_moveVector *= 2.0f;
		}

		
		_moveVector.z = 0.0f;
		_moveVector *= deltaTime;

		if (physics != NULL)
		{
			physics->ApplyImpulse(_moveVector);
		}
		else
		{
			GetGameObject()->SetPostion(GetGameObject()->GetPosition() + _moveVector);
		}	
	}

	if (Application::Get().IsFocused && playerInd == 2) {

		// ONLY when free camera is in use
		if (!InputEngine::IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {}

		//glm::vec3 _forwardVector = GetGameObject()->GetPosition() - cameraAttached->GetPosition();
		glm::vec3 _forwardVector = cameraAttached->Get<DuoPlayerCamera>()->dCmrLookAt - cameraAttached->GetPosition();
		_forwardVector = glm::normalize(_forwardVector);

		glm::quat rotX;
		glm::vec3 _moveVector = glm::vec3(0.0f);

		Gameplay::Physics::RigidBody::Sptr physics = GetGameObject()->Get<Gameplay::Physics::RigidBody>();

		if (InputEngine::IsKeyDown(GLFW_KEY_UP)) {
			_moveVector = _forwardVector * _moveSpeed;
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_DOWN)) {
			rotX = glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1));
			_moveVector = rotX * _forwardVector * _moveSpeed;
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_LEFT)) {
			rotX = glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 0, 1));
			_moveVector = rotX * _forwardVector * _moveSpeed;
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_RIGHT)) {
			rotX = glm::angleAxis(glm::radians(-90.0f), glm::vec3(0, 0, 1));
			_moveVector = rotX * _forwardVector * _moveSpeed;
		}

		if (InputEngine::IsKeyDown(GLFW_KEY_RIGHT_SHIFT)) {
			_moveVector *= 2.0f;
		}


		_moveVector.z = 0.0f;
		_moveVector *= deltaTime;

		if (physics != NULL)
		{
			physics->ApplyImpulse(_moveVector);
		}
		else
		{
			GetGameObject()->SetPostion(GetGameObject()->GetPosition() + _moveVector);
		}
	}
}



void SimpleObjectControl::RenderImGui()
{
}

nlohmann::json SimpleObjectControl::ToJson() const
{
	return { };
}

SimpleObjectControl::Sptr SimpleObjectControl::FromJson(const nlohmann::json& blob)
{
	SimpleObjectControl::Sptr result = std::make_shared<SimpleObjectControl>();
	return result;
}
