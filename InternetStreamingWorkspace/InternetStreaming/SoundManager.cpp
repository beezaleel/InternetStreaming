#include "SoundManager.h"

SoundManager::SoundManager() :
	mTagIndex(0), 
	openState(FMOD_OPENSTATE_READY), 
	fmodSystem_(nullptr)
{
}

SoundManager::~SoundManager()
{
}

bool SoundManager::LoadSound(const std::string& url, int flags)
{
	assert(fmodSystem_ && "no system object");
	assert(sounds.find(url) == sounds.end() && "sound already loaded");

	FMOD::Sound* sound;
	if (!isOkay(fmodSystem_->createSound(url.c_str(), flags, nullptr, &sound)))
	{
		return false;
	}
	sounds.emplace(url, sound);

	return true;
}

bool SoundManager::UnloadSound(const std::string& url)
{
	assert(fmodSystem_ && "no system object");
	assert(sounds.find(url) != sounds.end() && "sound not found");

	const auto sound = sounds.find(url);
	if (sound == sounds.end())
	{
		return false;
	}

	sound->second->release();
	sounds.erase(sound);

	return true;
}

bool SoundManager::FindSound(const std::string& url, FMOD::Sound** sound)
{
	const auto iterator = sounds.find(url);
	if (iterator == sounds.end())
	{
		return false;
	}

	*sound = iterator->second;

	return true;
}

bool SoundManager::isOkay(const FMOD_RESULT& result, bool displayError)
{
	if (result == FMOD_OK)
	{
		return true;
	}

	if (displayError)
	{
		printf("%d - %s", result, FMOD_ErrorString(result));
	}

	return false;
}

bool SoundManager::Initialize()
{
	if (!isOkay(FMOD::System_Create(&fmodSystem_)))
	{
		return false;
	}

	if (!isOkay(fmodSystem_->init(MAXIMUM_CHANNELS, FMOD_INIT_NORMAL, nullptr)))
	{
		return false;
	}

	return true;
}

bool SoundManager::Shutdown()
{
	int result;

	for (unsigned int i = 0; i < NUMBER_OF_SOUNDS; i++)
	{
		if (channel[i])
		{
			result = channel[i]->stop();
			assert(!result);
		}
	}

	for (auto iterator = sounds.begin(); iterator != sounds.end(); ++iterator)
	{
		do
		{
			result = fmodSystem_->update();
			assert(!result);

			result = iterator->second->getOpenState(&openState, nullptr, nullptr, nullptr);
			//assert(!result);

			Sleep(45);

		} while (openState != FMOD_OPENSTATE_READY);

		if (iterator->second) {
			result = iterator->second->release();
			assert(!result);
		}
	}

	if (fmodSystem_) {
		result = fmodSystem_->close();
		assert(!result);
		result = fmodSystem_->release();
		assert(!result);
	}

	return true;
}

FMOD_OPENSTATE* SoundManager::GetOpenState()
{
	return &openState;
}

bool SoundManager::CreateChannelGroup(const std::string& url)
{
	FMOD::ChannelGroup* channelGroup;
	if (!isOkay(fmodSystem_->createChannelGroup(url.c_str(), &channelGroup))) {
		return false;
	}

	channelGroups.emplace(url, channelGroup);

	return true;
}

void SoundManager::RemoveChannelGroup(const std::string& url)
{
	const auto iterator = channelGroups.find(url);
	if (iterator == channelGroups.end()) {
		return;
	}
	iterator->second->release();
	channelGroups.erase(iterator);
}

bool SoundManager::FindChannelGroup(const std::string& name, FMOD::ChannelGroup** channelGroup)
{
	const auto iterator = channelGroups.find(name);
	if (iterator == channelGroups.end())
	{
		return false;
	}

	*channelGroup = iterator->second;

	return true;
}

bool SoundManager::CreateDsp(const std::string& name, FMOD_DSP_TYPE dspType, const float value)
{
	FMOD::DSP* dsp;

	if (!isOkay(fmodSystem_->createDSPByType(dspType, &dsp))) {
		return false;
	}

	if (!isOkay(dsp->setParameterFloat(0, value)))
	{
		return false;
	}

	dsps.try_emplace(name, dsp);
	return true;
}

bool SoundManager::GetDsp(const std::string& name, FMOD::DSP** dsp)
{
	const auto dspEffectIterator = dsps.find(name);
	if (dspEffectIterator == dsps.end())
	{
		return false;
	}

	*dsp = dspEffectIterator->second;

	return true;
}

bool SoundManager::_PlaySound(const std::string& url, FMOD::Channel** channel)
{
	assert(fmodSystem_ && "no system object");

	assert(sounds.find(url) != sounds.end() && "sound not found");
	const auto sound = sounds.find(url);
	if (sound == sounds.end())
	{
		return false;
	}

	const auto channelGroup = channelGroups.find(url);
	if (channelGroup == channelGroups.end())
	{
		return false;
	}

	fmodSystem_->playSound(sound->second, channelGroup->second, false, channel);

	return true;
}

bool SoundManager::GetChannelGroupVolume(const std::string& url, float* volume)
{
	const auto iterator = channelGroups.find(url);
	if (iterator == channelGroups.end())
	{
		return false;
	}

	return isOkay(iterator->second->getVolume(volume));
}

bool SoundManager::SetChannelGroupVolume(const std::string& url, float volume)
{
	const auto iterator = channelGroups.find(url);
	if (iterator == channelGroups.end())
	{
		return false;
	}

	return isOkay(iterator->second->setVolume(volume));
}

bool SoundManager::GetChannelGroupEnabled(const std::string& url, bool* enabled)
{
	const auto iterator = channelGroups.find(url);
	if (iterator == channelGroups.end())
	{
		return false;
	}

	if (!isOkay(iterator->second->getMute(enabled))) {
		return false;
	}
	*enabled = !(*enabled);

	return true;
}

bool SoundManager::SetChannelGroupEnabled(const std::string& url, const bool enabled)
{
	const auto iterator = channelGroups.find(url);
	if (iterator == channelGroups.end())
	{
		return false;
	}

	if (!isOkay(iterator->second->setMute(!enabled))) {
		return false;
	}

	return true;
}

bool SoundManager::AddDspEffect(const std::string& url, const std::string& effectName)
{
	const auto channelGroupIterator = channelGroups.find(url);
	const auto dspEffectIterator = dsps.find(effectName);
	if (channelGroupIterator == channelGroups.end() || dspEffectIterator == dsps.end())
	{
		return false;
	}

	int numDsp;
	if (!isOkay(channelGroupIterator->second->getNumDSPs(&numDsp)))
	{
		return false;
	}

	if (!isOkay(channelGroupIterator->second->addDSP(numDsp, dspEffectIterator->second)))
	{
		return false;
	}

	return true;
}

bool SoundManager::RemoveDspEffect(const std::string& url, const std::string& effectName)
{
	const auto channelGroupIterator = channelGroups.find(url);
	const auto dspEffectIterator = dsps.find(effectName);
	if (channelGroupIterator == channelGroups.end() || dspEffectIterator == dsps.end())
	{
		return false;
	}

	if (!isOkay(channelGroupIterator->second->removeDSP(dspEffectIterator->second)))
	{
		return false;
	}

	return true;
}
