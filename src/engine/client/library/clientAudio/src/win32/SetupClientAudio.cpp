// ============================================================================
//
// SetupClientAudio.h
// copyright 2000 Sony Online Interactive
//
// ============================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/SetupClientAudio.h"

#include "clientAudio/Audio.h"
#include "clientAudio/ConfigClientAudio.h"
#include "clientAudio/SoundId.h"
#include "clientAudio/Sound2.h"
#include "clientAudio/Sound2d.h"
#include "clientAudio/Sound3d.h"
#include "clientAudio/SoundTemplateList.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"

// ============================================================================
//
// SetupClientAudio
//
// ============================================================================

#if defined(CLIENT_MILES_DEV_FACADE)
namespace ClientMilesDevelopment { void removeAudioCache(); }
#endif

//-----------------------------------------------------------------------------
void SetupClientAudio::install()
{
	InstallTimer const installTimer("SetupClientAudio::install");

	ConfigClientAudio::install();
	SoundId::install();
	Sound2::install();
	Sound2d::install();
	Sound3d::install();
	SoundTemplateList::install();
	Audio::install();

	ExitChain::add(SetupClientAudio::remove, "SetupClientAudio::remove");
}

//-----------------------------------------------------------------------------
void SetupClientAudio::remove()
{
#if defined(CLIENT_MILES_DEV_FACADE)
	Audio::remove(); // paired stop; may already have run on driver-init failure
	SoundTemplateList::remove(); // registry remains installed throughout disabled gameplay
	ClientMilesDevelopment::removeAudioCache(); // templates no longer borrow cache keys
#else
	SoundTemplateList::remove();
	Audio::remove();
#endif
	SoundId::remove();
}

// ============================================================================
