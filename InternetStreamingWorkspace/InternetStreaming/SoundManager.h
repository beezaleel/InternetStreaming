#pragma once

#include <stdio.h>
#include <cassert>
#include <vector>
#include <string>
#include "windows.h" 
#include <FMOD/fmod.hpp>
#include <FMOD/fmod_errors.h>
#include <map>

#define MAXIMUM_CHANNELS 512
#define NUMBER_OF_SOUNDS 3
#define NUMBER_OF_TAGS 10

struct DSPSetting {
	std::string name = "";
	float defaultValue = 0.0f;
	float min = 0.0f;
	float max = 0.0f;
	std::string format = "";
	std::string channel = "";
	std::string instruction = "";
};

struct Settings {
	unsigned int position = 0;
	unsigned int percentage = 0;
	bool isPlaying = false;
	bool isPaused = false;
	bool isStarving = false;
	const char* currentState = "Stopped";
	FMOD_TAG tag;
};

class SoundManager
{
protected:
	FMOD_OPENSTATE openState;
	FMOD::System* fmodSystem_;
	std::map<std::string, FMOD::Sound*> sounds;
	std::map<std::string, FMOD::ChannelGroup*> channelGroups;
	std::map<std::string, FMOD::DSP*> dsps;
	static bool isOkay(const FMOD_RESULT& result, bool displayError = true);
public:
	SoundManager();
	~SoundManager();
	Settings settings;
	FMOD::Channel* channel[NUMBER_OF_SOUNDS];
	std::vector<std::string> urls;
	std::vector<DSPSetting> dspSettings;
	unsigned int mTagIndex;
	char mTagString[NUMBER_OF_TAGS][128] = { 0 };
	bool LoadSound(const std::string& url, int flags);
	bool UnloadSound(const std::string& url);
	bool FindSound(const std::string& name, FMOD::Sound** sound);
	bool Initialize();
	bool Shutdown();
	FMOD_OPENSTATE* GetOpenState();
	bool CreateChannelGroup(const std::string& url);
	void RemoveChannelGroup(const std::string& url);
	bool FindChannelGroup(const std::string& name, FMOD::ChannelGroup** channelGroup);
	bool CreateDsp(const std::string& name, FMOD_DSP_TYPE dspType, const float value);
	bool GetDsp(const std::string& name, FMOD::DSP** dsp);
	bool _PlaySound(const std::string& url, FMOD::Channel** channel);
	bool GetChannelGroupVolume(const std::string& url, float* volume);
	bool SetChannelGroupVolume(const std::string& url, float volume);
	bool GetChannelGroupEnabled(const std::string& url, bool* enabled);
	bool SetChannelGroupEnabled(const std::string& url, const bool enabled);
	bool AddDspEffect(const std::string& url, const std::string& effectName);
	bool RemoveDspEffect(const std::string& url, const std::string& effectName);
};
