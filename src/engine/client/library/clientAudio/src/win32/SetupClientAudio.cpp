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
	SoundTemplateList::remove();
	Audio::remove();
	SoundId::remove();
}

// ============================================================================
