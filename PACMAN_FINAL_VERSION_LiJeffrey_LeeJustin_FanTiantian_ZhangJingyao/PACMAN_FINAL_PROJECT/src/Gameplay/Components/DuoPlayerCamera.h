#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"

struct GLFWwindow;

class DuoPlayerCamera : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<DuoPlayerCamera> Sptr;

	DuoPlayerCamera();
	virtual ~DuoPlayerCamera();

	Gameplay::GameObject::Sptr player1;
	Gameplay::GameObject::Sptr player2;

	glm::vec3 dCmrLookAt;

	virtual void Update(float deltaTime) override;

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(DuoPlayerCamera);
	virtual nlohmann::json ToJson() const override;
	static DuoPlayerCamera::Sptr FromJson(const nlohmann::json& blob);
};