// ============================================================================
//
// ConfigClientAudio.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientAudio/FirstClientAudio.h"
#include "clientAudio/ConfigClientAudio.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"

// ============================================================================

namespace ConfigClientAudioNamespace
{
	int   s_maxCached2dSampleSize = 128 * 1024;
	float s_obstruction = 0.6f;
	float s_occlusion = 0.95f;
}

using namespace ConfigClientAudioNamespace;

// ============================================================================
/**
 * Determine the Audio-specific default configuration.
 * 
 * This routine inspects the ConfigFile class to set some variables for rapid access
 * by the rest of the engine.
 */

//-----------------------------------------------------------------------------
void ConfigClientAudio::install()
{
	s_maxCached2dSampleSize = 1024 * clamp(0, ConfigFile::getKeyInt("ClientAudio", "maxCached2dSampleSize", 64), 512);
	s_obstruction = ConfigFile::getKeyFloat("ClientAudio", "obstruction", s_obstruction);
	s_occlusion = ConfigFile::getKeyFloat("ClientAudio", "occlusion", s_occlusion);

	ExitChain::add(ConfigClientAudio::remove, "ConfigClientAudio::remove", 0, false);
}

//-----------------------------------------------------------------------------
void ConfigClientAudio::remove()
{
}

//-----------------------------------------------------------------------------
int ConfigClientAudio::getMaxCached2dSampleSize()
{
	return s_maxCached2dSampleSize;
}

//-----------------------------------------------------------------------------
float ConfigClientAudio::getObstruction()
{
	return s_obstruction;
}

//-----------------------------------------------------------------------------
float ConfigClientAudio::getOcclusion()
{
	return s_occlusion;
}

// ============================================================================
