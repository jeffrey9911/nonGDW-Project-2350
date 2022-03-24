#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"

#include "Gameplay/Material.h"

struct GLFWwindow;

class SimpleToggle :public Gameplay::IComponent {
public:
	typedef std::shared_ptr<SimpleToggle> Sptr;

	SimpleToggle();
	virtual ~SimpleToggle();

	//virtual void Awake() override;

	virtual void Update(float deltaTime) override;

public:

	virtual void RenderImGui() override;
	MAKE_TYPENAME(SimpleToggle);
	virtual nlohmann::json ToJson() const override;
	static SimpleToggle::Sptr FromJson(const nlohmann::json& blob);

protected:
	bool toggleNoL;
	bool toggleABL;
	bool toggleSPL;
	bool toggleAnS;
	bool toggleTexture;
	//Gameplay::Material::Sptr _matStore;
};