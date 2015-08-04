// ============================================================================
//
// LightningManager.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/LightningManager.h"

#include "clientParticle/LightningBolt.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/Vector.h"

#include <vector>

// ============================================================================
//
// LightningManagerNamespace
//
// ============================================================================

//-----------------------------------------------------------------------------
namespace LightningManagerNamespace
{
	typedef std::vector<LightningBolt *> LightningBolts;

	bool            s_installed;
	LightningBolts *s_lightningBolts;
	int const       s_lightningBoltCount = 50;
};

using namespace LightningManagerNamespace;

// ============================================================================
//
// LightningManager
//
// ============================================================================

//--------------------------------------------------------------------------
void LightningManager::install()
{
	DEBUG_FATAL(s_installed, ("Already installed"));
	s_installed = true;

	// Create all the lightning bolts

	s_lightningBolts = new LightningBolts;
	s_lightningBolts->reserve(s_lightningBoltCount);

	for (int i = 0; i < s_lightningBoltCount; ++i)
	{
		s_lightningBolts->push_back(new LightningBolt);
	}

	ExitChain::add(&remove, "LightningAppearanceTemplate::remove()");
}

//--------------------------------------------------------------------------
void LightningManager::remove()
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	while (!s_lightningBolts->empty())
	{
		LightningBolt *lightningBolt = s_lightningBolts->back();

		s_lightningBolts->pop_back();

		delete lightningBolt;
		lightningBolt = NULL;
	}

	delete s_lightningBolts;
	s_lightningBolts = NULL;

	s_installed = false;
}

//--------------------------------------------------------------------------
void LightningManager::alter(float const deltaTime)
{
	LightningBolts::iterator iterLightningBolts = s_lightningBolts->begin();

	for (; iterLightningBolts != s_lightningBolts->end(); ++iterLightningBolts)
	{
		LightningBolt *lightningBolt = (*iterLightningBolts);
		lightningBolt->alter(deltaTime);
	}
}

//--------------------------------------------------------------------------
int LightningManager::getLightningBoltCount()
{
	return s_lightningBoltCount;
}

//--------------------------------------------------------------------------
LightningBolt const &LightningManager::getLightningBolt(int const index)
{
	int min = 0;
	int max = s_lightningBoltCount - 1;

	DEBUG_FATAL((index < min) || (index > max), ("Out of range: %i min: %i max: %i", index, min, max));

	LightningBolt &lightningBolt = *(*s_lightningBolts)[static_cast<unsigned int>(clamp(min, index, max))];

	return lightningBolt;
}

// ============================================================================
