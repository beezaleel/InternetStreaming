#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <Windows.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "SoundManager.h"
#include "RadioUI.h"

bool isDspEnabled = false;

GLFWwindow* window;
SoundManager soundManager;
RadioUI radioUI = nullptr;

void SetDspState(bool& isEnabled, std::string channelName, std::string name) {
	if (!isEnabled) {
		soundManager.AddDspEffect(channelName, name);
		isEnabled = true;
	}
	else {
		soundManager.RemoveDspEffect(channelName, name);
		isEnabled = false;
	}
}

bool AddDspEffect() {
	if (!soundManager.CreateDsp("echo", FMOD_DSP_TYPE_ECHO, 500.0f) ||
		!soundManager.CreateDsp("pitch", FMOD_DSP_TYPE_PITCHSHIFT, 0.7f) ||
		!soundManager.CreateDsp("fader", FMOD_DSP_TYPE_FADER, 0.0f) ||
		!soundManager.CreateDsp("tremolo", FMOD_DSP_TYPE_TREMOLO, 5.0f) ||
		!soundManager.CreateDsp("distortion", FMOD_DSP_TYPE_DISTORTION, 0.5f) ||
		!soundManager.CreateDsp("parameq", FMOD_DSP_TYPE_PARAMEQ, 10000.0f))
	{
		return false;
	}
	return true;
}

bool LoadDSPInstructions(std::string filename)
{
	std::ifstream infile(filename);
	int index = 0;
	while (infile.is_open()) {
		std::string line;

		if (!getline(infile, line))
			break;
		soundManager.dspSettings[index].instruction = line;
		index++;
	}

	return true;
}

bool LoadDSPSettings(std::string filename)
{
	std::ifstream infile(filename);
	int index = 0;
	while (infile.is_open()) {
		std::string line, name, defaultValue, min, max, format;

		if (!getline(infile, line))
			break;

		std::stringstream ss((line));
		ss >> name >> defaultValue >> min >> max >> format;

		soundManager.dspSettings[index].name = name;
		soundManager.dspSettings[index].defaultValue = std::stof(defaultValue);
		soundManager.dspSettings[index].min = std::stof(min);
		soundManager.dspSettings[index].max = std::stof(max);
		soundManager.dspSettings[index].format = format;
		
		index++;
	}

	return true;
}

bool LoadSoundsFromURL(std::string filename)
{
	std::ifstream infile(filename);
	int index = 0;
	int dspIndex = 0;
	while (infile.is_open()) {
		std::string line, url;

		if (!getline(infile, line))
			break;

		std::stringstream ss((line));
		ss >> url;

		if (!soundManager.LoadSound(url, FMOD_CREATESTREAM | FMOD_NONBLOCKING)) {
			return false;
		}
		if (!soundManager.CreateChannelGroup(url)) {
			return false;
		}
		soundManager.urls.push_back(url);
		for (int i = 0; i < 2; i++) {
			DSPSetting dspSetting;
			dspSetting.channel = url;
			soundManager.dspSettings.push_back(dspSetting);
			dspIndex++;
		}
	}

	return true;
}

void KeycallBack(GLFWwindow* window, const int key, int scancode, const int action, const int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	// W to enable/disable Echo dsp
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		SetDspState(isDspEnabled, soundManager.urls[0], "echo");

	// E to enable/disable pitch dsp
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
		SetDspState(isDspEnabled, soundManager.urls[0], "pitch");

	// R to enable/disable fader dsp
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		SetDspState(isDspEnabled, soundManager.urls[1], "fader");

	// T to enable/disable tremolo dsp
	if (key == GLFW_KEY_T && action == GLFW_PRESS)
		SetDspState(isDspEnabled, soundManager.urls[1], "tremolo");

	// Y to enable/disable distortion dsp
	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
		SetDspState(isDspEnabled, soundManager.urls[2], "distortion");

	// U to enable/disable parameq dsp
	if (key == GLFW_KEY_U && action == GLFW_PRESS)
		SetDspState(isDspEnabled, soundManager.urls[2], "parameq");
}

int main(int argc, char** argv) {
	glfwInit();
	window = glfwCreateWindow(1000, 600, "Internet Radio", nullptr, nullptr);

	if (!soundManager.Initialize()) {
		return -2;
	}

	if (!LoadSoundsFromURL("media_urls.txt")) {
		return -3;
	}

	if (!AddDspEffect()) {
		return -4;
	}

	if (!LoadDSPSettings("dsp_settings.txt")) {
		return -5;
	}

	if (!LoadDSPInstructions("dsp_instructions.txt")) {
		return -6;
	}

	radioUI = RadioUI(&soundManager);

	if (!window)
	{
		return 1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		return 2;
	}

	glfwSetKeyCallback(window, KeycallBack);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	if (!ImGui_ImplGlfw_InitForOpenGL(window, true) || !ImGui_ImplOpenGL3_Init("#version 460"))
	{
		return 3;
	}

	ImGui::StyleColorsDark();

	while (!glfwWindowShouldClose(window)) {
		//poll for user events
		glfwPollEvents();

		//clear the back buffer
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		radioUI.Render();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//present to the user
		glfwSwapBuffers(window);
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	soundManager.Shutdown();
	exit(0);
}
