#include "DuoplayerLayer.h"
#include "../Application.h"
#include "Utils/ImGuiHelper.h"
#include "imgui_internal.h"
#include "Gameplay/Scene.h"
#include "../Timing.h"
#include "Utils/Windows/FileDialogs.h"
#include <filesystem>
#include "RenderLayer.h"

#include "../Windows/DuoplayerWindow.h"

DuoplayerLayer::DuoplayerLayer() :
	ApplicationLayer()
{
	Name = "Player2 ViewPort";
	Overrides = AppLayerFunctions::OnAppLoad | AppLayerFunctions::OnAppUnload | AppLayerFunctions::OnPreRender | AppLayerFunctions::OnRender | AppLayerFunctions::OnPostRender;
}

DuoplayerLayer::~DuoplayerLayer() = default;

void DuoplayerLayer::OnAppLoad(const nlohmann::json& config)
{
	Application& app = Application::Get();

	RegisterWindow<DuoplayerWindow>();

}

void DuoplayerLayer::OnAppUnload()
{
}

void DuoplayerLayer::OnPreRender()
{
}

void DuoplayerLayer::OnRender(const Framebuffer::Sptr& prevLayer)
{
	using namespace Gameplay;
	Application& app = Application::Get();

	glm::vec2 size;

	

}

void DuoplayerLayer::OnPostRender()
{
}
