#include "RadioUI.h"

RadioUI::RadioUI(SoundManager* soundManager): soundManager(soundManager)
{
}

RadioUI::~RadioUI()
{
}

const char* const BoolToString(bool b)
{
	return b ? "Yes" : "No";
}

void RadioUI::Render()
{
	static int selectedIndex = 0;
	static bool hasCopiedVectorToChar = false;

	static const char* urlsCopy[NUMBER_OF_SOUNDS];

	if (!hasCopiedVectorToChar) {
		for (int i = 0; i < NUMBER_OF_SOUNDS; i++) {
			urlsCopy[i] = soundManager->urls[i].c_str();
		}
		hasCopiedVectorToChar = true;
	}
	
	FMOD_RESULT result = FMOD_OK;
	ImGui::Begin("Internet Radio");
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Select Radio station from below");
	ImGui::ListBox("##listbox::Cavebot", &selectedIndex, urlsCopy, soundManager->urls.size());
	
	std::string url = soundManager->urls[selectedIndex];
	FMOD::Sound* sound;
	soundManager->FindSound(url, &sound);
	result = sound->getOpenState(
		soundManager->GetOpenState(),
		&soundManager->settings.percentage,
		&soundManager->settings.isStarving,
		nullptr);

	if (soundManager->channel[selectedIndex]) {
		while (sound->getTag(nullptr, -1, &soundManager->settings.tag) == FMOD_OK)
		{
			if (soundManager->settings.tag.datatype == FMOD_TAGDATATYPE_STRING)
			{
				sprintf(soundManager->mTagString[soundManager->mTagIndex], "%s = %s (%d bytes)",
					soundManager->
					settings.tag.name,
					static_cast<char*>(soundManager->settings.tag.data),
					soundManager->settings.tag.datalen);


				soundManager->mTagIndex = (soundManager->mTagIndex + 1) % NUMBER_OF_TAGS;
			}
			else
			{
				float frequency = *static_cast<float*>(soundManager->settings.tag.data);
				result = soundManager->channel[selectedIndex]->setFrequency(frequency);
				assert(!result);
			}
		}

		result = soundManager->channel[selectedIndex]->getPaused(&soundManager->settings.isPaused);
		assert(!result);
		result = soundManager->channel[selectedIndex]->isPlaying(&soundManager->settings.isPlaying);
		assert(!result);
		result = soundManager->channel[selectedIndex]->getPosition(&soundManager->settings.position, FMOD_TIMEUNIT_MS);
		assert(!result);

		result = soundManager->channel[selectedIndex]->setMute(soundManager->settings.isStarving);
		assert(!result);
	}
	else
	{
		soundManager->_PlaySound(url, &soundManager->channel[selectedIndex]);
	}

	if (*soundManager->GetOpenState() == FMOD_OPENSTATE_CONNECTING)
	{
		soundManager->settings.currentState = "Connecting...";
	}
	else if (*soundManager->GetOpenState() == FMOD_OPENSTATE_BUFFERING)
	{
		soundManager->settings.currentState = "Buffering...";
		std::string buffering = std::string("Buffering . . . ") + std::to_string(soundManager->settings.percentage).c_str() + std::string("%");
		ImGui::TextColored(ImVec4(1, 0, 0, 1), buffering.c_str());
	}
	else if (soundManager->settings.isPaused)
	{
		soundManager->settings.currentState = "Paused...";
	}
	else
	{
		soundManager->settings.currentState = "Playing";
	}

	std::string currentState = std::string("Curent state: ") + soundManager->settings.currentState;
	ImGui::TextColored(ImVec4(0, 1, 0, 1), currentState.c_str());	

	char position[512];
	sprintf(position, "Time: %02d:%02d:%02d", soundManager->settings.position / 1000 / 60, soundManager->settings.position / 100 % 60, soundManager->settings.position / 10 % 100);
	ImGui::TextColored(ImVec4(0, 0, 1, 1), position);
	
	sprintf(position, "%s", soundManager->mTagString[selectedIndex % NUMBER_OF_TAGS]);
	ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.0f, 1), position);

	std::string state = std::string("Starving: ") + BoolToString(soundManager->settings.isStarving) + 
		std::string("        ") + std::string("Playing: ") + BoolToString(soundManager->settings.isPlaying) + 
		std::string("        ") + std::string("Paused: ") + BoolToString(soundManager->settings.isPaused);
	ImGui::TextColored(ImVec4(0.0f, 0.75f, 0.79f, 1), state.c_str());
	

	FMOD::ChannelGroup* channelGroup;
	if (!soundManager->FindChannelGroup(url, &channelGroup))
	{
		return;
	}

	float currentVolume;
	if (!soundManager->GetChannelGroupVolume(url, &currentVolume)) {
		return;
	}
	currentVolume *= 100;
	ImGui::SliderFloat("volume", &currentVolume, 0.0f, 100.0f, "%.0f");
	currentVolume /= 100;

	if (!soundManager->SetChannelGroupVolume(url, currentVolume)) {
		return;
	}

	bool volumeEnabled;
	if (!soundManager->GetChannelGroupEnabled(url, &volumeEnabled)) {
		return;
	}

	ImGui::SameLine();
	ImGui::Checkbox("##channel1_volume", &volumeEnabled);

	if (!soundManager->SetChannelGroupEnabled(url, volumeEnabled)) {
		return;
	}

	for (int i = 0; i < soundManager->dspSettings.size(); i++) {
		if (soundManager->dspSettings[i].channel == url) {
			ImGui::SliderFloat(soundManager->dspSettings[i].instruction.c_str(), 
				&soundManager->dspSettings[i].defaultValue, 
				soundManager->dspSettings[i].min,
				soundManager->dspSettings[i].max,
				soundManager->dspSettings[i].format.c_str());
			FMOD::DSP* dsp;
			if (!soundManager->GetDsp(soundManager->dspSettings[i].name.c_str(), &dsp))
				return;
			dsp->setParameterFloat(0, soundManager->dspSettings[i].defaultValue);
		}
	}
	ImGui::End();
}
