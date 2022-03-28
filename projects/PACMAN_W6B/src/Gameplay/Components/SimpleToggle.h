#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"

#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Material.h"
#include "Gameplay/Scene.h"
#include "Graphics/Textures/Texture3D.h"

struct GLFWwindow;

/// <summary>
/// 
/// </summary>
class SimpleToggle :public Gameplay::IComponent {
public:
	typedef std::shared_ptr<SimpleToggle> Sptr;

	SimpleToggle();
	virtual ~SimpleToggle();

	virtual void Awake() override;

	Gameplay::GameObject::Sptr _shaderContainer;
	int toggleMode;
	


	virtual void Update(float deltaTime) override;

public:

	virtual void RenderImGui() override;
	MAKE_TYPENAME(SimpleToggle);
	virtual nlohmann::json ToJson() const override;
	static SimpleToggle::Sptr FromJson(const nlohmann::json& blob);

protected:
	float _cdTimer;
	RenderComponent::Sptr _renderer;
	ShaderProgram::Sptr _shaderToStore;
	Texture3D::Sptr _lutToStore;
	bool isToggleOn;
};