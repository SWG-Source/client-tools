//======================================================================
//
// SwgCuiOptSound.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOptSound.h"

#include "UICheckbox.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIComboBox.h"
#include "UISliderbar.h"
#include "clientAudio/Audio.h"
#include "clientAudio/ConfigClientAudio.h"
#include "clientGame/GroundScene.h"
#include <algorithm>
#include <vector>

//======================================================================

namespace
{
	typedef std::vector<int> IntVector;
	IntVector    s_maxSimSounds;

	const int s_maxSimSoundsArray [] =
	{
		16,
		32,
		64
	};

	const size_t s_maxSimSoundsArraySize = sizeof (s_maxSimSoundsArray) / sizeof (s_maxSimSoundsArray [0]);

	bool s_installed = false;

	int   getDefaultIndexOne  (const SwgCuiOptBase & , const UIComboBox &) { return 1; }
}

//----------------------------------------------------------------------

SwgCuiOptSound::SwgCuiOptSound (UIPage & page) :
SwgCuiOptBase ("SwgCuiOptSound", page)
{
	if (!s_installed)
	{
		//-- maybe alphabetize, prioritize, or otherwise sort this list
		s_maxSimSounds.assign (s_maxSimSoundsArray, s_maxSimSoundsArray + s_maxSimSoundsArraySize);

		s_installed = true;
	}

	UISliderbar * slider   = 0;
	UICheckbox  * checkbox = 0;
	UIComboBox  * combo    = 0;

	getCodeDataObject (TUISliderbar, slider, "sliderVolumeEffects");
	registerSlider (*slider, Audio::setSoundEffectVolume, Audio::getSoundEffectVolume, Audio::getDefaultSoundEffectVolume, 0.f, 1.f);

	getCodeDataObject (TUISliderbar, slider, "sliderVolumeMaster");
	registerSlider (*slider, Audio::setMasterVolume, Audio::getMasterVolume, Audio::getDefaultMasterVolume, 0.f, 1.f);

	getCodeDataObject (TUISliderbar, slider, "sliderVolumeMusicBg");
	registerSlider (*slider, Audio::setBackGroundMusicVolume, Audio::getBackGroundMusicVolume, Audio::getDefaultBackGroundMusicVolume, 0.f, 1.f);

	getCodeDataObject (TUISliderbar, slider, "sliderVolumeAmbient");
	registerSlider (*slider, Audio::setAmbientEffectVolume, Audio::getAmbientEffectVolume, Audio::getDefaultAmbientEffectVolume, 0.f, 1.f);

	getCodeDataObject (TUISliderbar, slider, "sliderVolumeMusicPlayer");
	registerSlider (*slider, Audio::setPlayerMusicVolume, Audio::getPlayerMusicVolume, Audio::getDefaultPlayerMusicVolume, 0.f, 1.f);

	getCodeDataObject (TUISliderbar, slider, "sliderVolumeUi");
	registerSlider (*slider, Audio::setUserInterfaceVolume, Audio::getUserInterfaceVolume, Audio::getDefaultUserInterfaceVolume, 0.f, 1.f);

	getCodeDataObject (TUICheckbox, checkbox, "checkListenerFollowsPlayer");
	registerCheckbox (*checkbox, GroundScene::setListenerFollowsPlayer, GroundScene::getListenerFollowsPlayer, SwgCuiOptBase::getFalse);

	getCodeDataObject (TUICheckbox, checkbox, "checkEnable");

	// if Audio system was never installed, disable ability to modify audio settings 
	if (!Audio::isMilesEnabled ())
	{
		checkbox->SetChecked (false);
		checkbox->SetEnabled (false);
	}
	else
	{
		registerCheckbox (*checkbox, Audio::setEnabled, Audio::isEnabled, Audio::isEnabled);
		//-- restore checkbox & enabled states
		checkbox->SetChecked (!Audio::isEnabled ());
		checkbox->SetChecked (Audio::isEnabled ());
	}

	//----------------------------------------------------------------------

	getCodeDataObject (TUIComboBox, combo, "comboSim");
	{
		char buf [128];
		const size_t buf_size = sizeof (buf);

		combo->Clear();
		for (IntVector::const_iterator it = s_maxSimSounds.begin (); it != s_maxSimSounds.end (); ++it)		
		{
			const int size = *it;
			snprintf (buf, buf_size, "%3d", size);
			combo->AddItem(Unicode::narrowToWide (buf), buf);
		}
	}
	registerComboBox (*combo, SwgCuiOptSound::onComboSimSet, SwgCuiOptSound::onComboSimGet, getDefaultIndexOne);
}

//----------------------------------------------------------------------

int  SwgCuiOptSound::onComboSimGet      (const SwgCuiOptBase & , const UIComboBox &)
{
	const int count = Audio::getRequestedMaxNumberOfSamples();

	const IntVector::iterator it = std::find (s_maxSimSounds.begin (), s_maxSimSounds.end (), count);
	if (it != s_maxSimSounds.end ())
		return std::distance (s_maxSimSounds.begin (), it);

	return 0;
}

//----------------------------------------------------------------------

void SwgCuiOptSound::onComboSimSet      (const SwgCuiOptBase & , const UIComboBox & , int index)
{
	if (index >= 0 && index < static_cast<int>(s_maxSimSounds.size ()))
		Audio::setRequestedMaxNumberOfSamples(s_maxSimSounds [static_cast<size_t>(index)]);
}

//======================================================================
