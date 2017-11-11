// ============================================================================
//
// ParticleManager.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleManager.h"

#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/ParticleEmitter.h"
#include "clientParticle/ParticleEmitterGroup.h"
#include "clientParticle/ParticleMesh.h"
#include "clientParticle/ParticleQuad.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/AppearanceTemplate.h"

#include <map>
#include <set>
#include <vector>

// ============================================================================
//
// ParticleManagerNamespace
//
// ============================================================================

namespace ParticleManagerNamespace
{
	bool s_installed = false;
	bool s_particlesEnabled = true;
	bool s_texturingEnabled = true;
	bool s_debugAxisEnabled = false;
	bool s_debugExtentsEnabled = false;
	bool s_debugParticleOrientationEnabled = false;
	bool s_debugVelocityEnabled = false;
	bool s_debugWorldTextEnabled = false;
	bool s_debugFlockingCageEnabled = false;
	bool s_debugShowSwooshQuadsEnabled = false;
	bool s_debugOriginIconEnabled = false;

#ifdef _DEBUG
	typedef std::map<CrcLowerString, std::set<ParticleEffectAppearance const *> > ParticleEffectAppearanceList;

	ParticleEffectAppearanceList s_particleEffectAppearanceList;
#endif // _DEBUG

	void remove();
};

using namespace ParticleManagerNamespace;

//-----------------------------------------------------------------------------
void ParticleManagerNamespace::remove()
{
	DebugFlags::unregisterFlag(s_particlesEnabled);
	DebugFlags::unregisterFlag(s_texturingEnabled);
	DebugFlags::unregisterFlag(s_debugAxisEnabled);
	DebugFlags::unregisterFlag(s_debugExtentsEnabled);
	DebugFlags::unregisterFlag(s_debugParticleOrientationEnabled);
	DebugFlags::unregisterFlag(s_debugVelocityEnabled);
	DebugFlags::unregisterFlag(s_debugWorldTextEnabled);
	DebugFlags::unregisterFlag(s_debugOriginIconEnabled);
}

// ============================================================================
//
// ParticleManager
//
// ============================================================================

//-----------------------------------------------------------------------------
void ParticleManager::install()
{
	DEBUG_FATAL(s_installed, ("Already installed"));

	DebugFlags::registerFlag(s_particlesEnabled, "ClientParticle", "particlesEnabled");
	DebugFlags::registerFlag(s_texturingEnabled, "ClientParticle", "texturingEnabled");
	DebugFlags::registerFlag(s_debugAxisEnabled, "ClientParticle", "debugAxisEnabled");
	DebugFlags::registerFlag(s_debugExtentsEnabled, "ClientParticle", "debugExtentsEnabled");
	DebugFlags::registerFlag(s_debugParticleOrientationEnabled, "ClientParticle", "debugParticleOrientationEnabled");
	DebugFlags::registerFlag(s_debugVelocityEnabled, "ClientParticle", "debugVelocityEnabled");
	DebugFlags::registerFlag(s_debugWorldTextEnabled, "ClientParticle", "debugWorldTextEnabled");
	DebugFlags::registerFlag(s_debugOriginIconEnabled, "ClientParticle", "debugOriginIconEnabled");

	ExitChain::add(&remove, "ParticleManagerNamespace::remove");

	s_installed = true;
}

//-----------------------------------------------------------------------------
void ParticleManager::setParticlesEnabled(bool const particlesEnabled)
{
	s_particlesEnabled = particlesEnabled;
}

//-----------------------------------------------------------------------------
bool ParticleManager::isParticlesEnabled()
{
	return s_particlesEnabled;
}

//-----------------------------------------------------------------------------
void ParticleManager::setDebugAxisEnabled(bool const debugAxisEnabled)
{
	s_debugAxisEnabled = debugAxisEnabled;
}

//-----------------------------------------------------------------------------
bool ParticleManager::isDebugAxisEnabled()
{
	return s_debugAxisEnabled;
}

//-----------------------------------------------------------------------------
void ParticleManager::setDebugParticleOrientationEnabled(bool const debugParticleOrientationEnabled)
{
	s_debugParticleOrientationEnabled = debugParticleOrientationEnabled;
}

//-----------------------------------------------------------------------------
bool ParticleManager::isDebugParticleOrientationEnabled()
{
	return s_debugParticleOrientationEnabled;
}

//-----------------------------------------------------------------------------
void ParticleManager::setDebugVelocityEnabled(bool const debugVelocityEnabled)
{
	s_debugVelocityEnabled = debugVelocityEnabled;
}

//-----------------------------------------------------------------------------
bool ParticleManager::isDebugVelocityEnabled()
{
	return s_debugVelocityEnabled;
}

//-----------------------------------------------------------------------------
void ParticleManager::setDebugExtentsEnabled(bool const debugExtentsEnabled)
{
	s_debugExtentsEnabled = debugExtentsEnabled;
}

//-----------------------------------------------------------------------------
bool ParticleManager::isDebugExtentsEnabled()
{
	return s_debugExtentsEnabled;
}

//-----------------------------------------------------------------------------
void ParticleManager::setTexturingEnabled(bool const texturingEnabled)
{
	s_texturingEnabled = texturingEnabled;
}

//-----------------------------------------------------------------------------
bool ParticleManager::isTexturingEnabled()
{
	return s_texturingEnabled;
}

//-----------------------------------------------------------------------------
void ParticleManager::setDebugWorldTextEnabled(bool const debugWorldEnabled)
{
	s_debugWorldTextEnabled = debugWorldEnabled;
}

//-----------------------------------------------------------------------------
bool ParticleManager::isDebugWorldTextEnabled()
{
	return s_debugWorldTextEnabled;
}

//-----------------------------------------------------------------------------
void ParticleManager::setDebugFlockingCageEnabled(bool const debugFlockingCageEnabled)
{
	s_debugFlockingCageEnabled = debugFlockingCageEnabled;
}

//-----------------------------------------------------------------------------
bool ParticleManager::isDebugFlockingCageEnabled()
{
	return s_debugFlockingCageEnabled;
}

//-----------------------------------------------------------------------------
void ParticleManager::setDebugShowSwooshQuadsEnabled(bool const enabled)
{
	s_debugShowSwooshQuadsEnabled = enabled;
}

//-----------------------------------------------------------------------------
bool ParticleManager::isDebugShowSwooshQuadsEnabled()
{
	return s_debugShowSwooshQuadsEnabled;
}

//-----------------------------------------------------------------------------
void ParticleManager::setDebugOriginIconEnabled(bool const enabled)
{
	s_debugOriginIconEnabled = enabled;
}

//-----------------------------------------------------------------------------
bool ParticleManager::isDebugOriginIconEnabled()
{
	return s_debugOriginIconEnabled;
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
void ParticleManager::debugRegister(ParticleEffectAppearance const & particleEffectAppearance)
{
	CrcLowerString const & crcName = particleEffectAppearance.getAppearanceTemplate()->getCrcName();
	ParticleEffectAppearanceList::iterator iterParticleEffectAppearanceList = s_particleEffectAppearanceList.find(crcName);

	if (iterParticleEffectAppearanceList != s_particleEffectAppearanceList.end())
	{
		// Check to make sure this is not already in the list, if so, wtf?

		iterParticleEffectAppearanceList->second.insert(&particleEffectAppearance);
	}
	else
	{
		std::set<ParticleEffectAppearance const *> newSet;
		newSet.insert(&particleEffectAppearance);

		s_particleEffectAppearanceList.insert(std::make_pair(crcName, newSet));
	}
}

//-----------------------------------------------------------------------------
void ParticleManager::debugUnRegister(ParticleEffectAppearance const & particleEffectAppearance)
{
	CrcLowerString const & crcName = particleEffectAppearance.getAppearanceTemplate()->getCrcName();
	ParticleEffectAppearanceList::iterator iterParticleEffectAppearanceList = s_particleEffectAppearanceList.find(crcName);

	if (iterParticleEffectAppearanceList != s_particleEffectAppearanceList.end())
	{
		iterParticleEffectAppearanceList->second.erase(&particleEffectAppearance);

		if (iterParticleEffectAppearanceList->second.empty())
		{
			s_particleEffectAppearanceList.erase(iterParticleEffectAppearanceList);
		}
	}
	else
	{
		DEBUG_FATAL(true, ("This particle system should exist in this list!"));
	}
}

//-----------------------------------------------------------------------------
void ParticleManager::debugPrint()
{
	typedef std::vector<std::pair<int, ParticleEffectAppearanceList::const_iterator> > SortedList;
	static SortedList sortedList;
	sortedList.clear();

	ParticleEffectAppearanceList::const_iterator iterParticleEffectAppearanceList = s_particleEffectAppearanceList.begin();

	for (; iterParticleEffectAppearanceList != s_particleEffectAppearanceList.end(); ++iterParticleEffectAppearanceList)
	{
		std::set<ParticleEffectAppearance const *>::const_iterator iterParticles = iterParticleEffectAppearanceList->second.begin();

		int particleCount = 0;
		for (; iterParticles != iterParticleEffectAppearanceList->second.end(); ++iterParticles)
		{
			particleCount += (*iterParticles)->getParticleCount();
		}

		sortedList.push_back(std::make_pair(particleCount, iterParticleEffectAppearanceList));
	}

	SortedList::reverse_iterator iterSortedList = sortedList.rbegin();
	int reportCount = 0;

	for (; iterSortedList != sortedList.rend(); ++iterSortedList)
	{
		unsigned int const listSize = iterSortedList->second->second.size();
		ParticleEffectAppearance const & particleEffectAppearance = *(*iterSortedList->second->second.begin());
		CrcLowerString const & crcName = particleEffectAppearance.getAppearanceTemplate()->getCrcName();

		DEBUG_REPORT_PRINT(true, ("%4d [%3u] %s\n", iterSortedList->first, listSize, crcName.isEmpty() ? "no name" : crcName.getString()));

		if (++reportCount > 20)
		{
			break;
		}
	}
}
#endif // _DEBUG

//=============================================================================
