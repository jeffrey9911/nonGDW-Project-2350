#pragma once
#include "Application/IEditorWindow.h"
#include "Gameplay/Material.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/Texture1D.h"
#include "Graphics/Textures/Texture3D.h"

class DuoplayerWindow final : public IEditorWindow {
public:
	MAKE_PTRS(DuoplayerWindow);

	DuoplayerWindow();
	virtual ~DuoplayerWindow();

	virtual void Render() override;
};
