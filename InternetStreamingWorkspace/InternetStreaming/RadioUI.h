#pragma once

#include "imgui/imgui.h"
#include "SoundManager.h"

class RadioUI
{
public:
	RadioUI(SoundManager* soundManager);
	~RadioUI();
	void Render();

private:
	SoundManager* soundManager;
};
