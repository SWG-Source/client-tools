// ============================================================================
//
// ForceFeedbackEffectTemplate.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientDirectInput/FirstClientDirectInput.h"
#include "clientDirectInput/ForceFeedbackEffectTemplate.h"
#include "clientDirectInput/ForceFeedbackEffectTemplateList.h"
#include "sharedFoundation/Crc.h"

#include <list>
#include <string>

// ============================================================================

namespace ForceFeedbackEffectTemplateNamespace
{
	static std::list<ForceFeedbackEffectTemplate const *> ms_templatesToReleaseWhenDonePlaying;
}

using namespace ForceFeedbackEffectTemplateNamespace;

// ============================================================================

ForceFeedbackEffectTemplate::ForceFeedbackEffectTemplate(std::string const & name, bool const fireAndForget)
: m_referenceCount(0),
  m_crc(Crc::calculate(name.c_str())),
  m_effects(),
  m_fireAndForget(fireAndForget)
{
	IGNORE_RETURN(DirectInput::enumForceFeedbackEffectIntoTemplate(name, this));
}

//-----------------------------------------------------------------------------

ForceFeedbackEffectTemplate::~ForceFeedbackEffectTemplate()
{
	//if we need to stop the effect on destruct (i.e. for permanant looping effects that won't play themselves out)
	if(!m_fireAndForget && isPlaying())
		stopEffect();

	for(std::vector<DirectInput::ForceFeedbackEffect *>::iterator i = m_effects.begin(); i != m_effects.end(); ++i)
	{		
		DirectInput::destroyForceFeedbackEffect(*i);
	}

	m_referenceCount = 0;
	m_crc = 0;
	m_effects.clear();
}

//-----------------------------------------------------------------------------

int ForceFeedbackEffectTemplate::getReferenceCount() const
{
	return m_referenceCount;
}

//-----------------------------------------------------------------------------

uint32 ForceFeedbackEffectTemplate::getCrc() const
{
	return m_crc;
}

// ----------------------------------------------------------------------

void ForceFeedbackEffectTemplate::fetch() const
{
	//if it's in the deletion queue but not yet deleted, make sure to remove it
	std::list<ForceFeedbackEffectTemplate const *>::iterator i = std::find(ms_templatesToReleaseWhenDonePlaying.begin(), ms_templatesToReleaseWhenDonePlaying.end(), this);
	if(i != ms_templatesToReleaseWhenDonePlaying.end())
		ms_templatesToReleaseWhenDonePlaying.erase(i);

	++m_referenceCount;
}

// ----------------------------------------------------------------------

void ForceFeedbackEffectTemplate::release() const
{
	if (--m_referenceCount <= 0)
	{
		//add it to the deletion queue so that it is deallocated once the effect finishes playing
		if(std::find(ms_templatesToReleaseWhenDonePlaying.begin(), ms_templatesToReleaseWhenDonePlaying.end(), this) == ms_templatesToReleaseWhenDonePlaying.end())
		{
			ms_templatesToReleaseWhenDonePlaying.push_back(this);
		}
	}
}

// ----------------------------------------------------------------------

void ForceFeedbackEffectTemplate::update()
{
	deleteAllQueuedDeletes(false);
}

// ----------------------------------------------------------------------

void ForceFeedbackEffectTemplate::deleteAllQueuedDeletes(bool const deletePlayingEffects)
{
	while(!ms_templatesToReleaseWhenDonePlaying.empty())
	{
		std::list<ForceFeedbackEffectTemplate const *>::iterator i = ms_templatesToReleaseWhenDonePlaying.begin();
		ForceFeedbackEffectTemplate * const ffbet = const_cast<ForceFeedbackEffectTemplate *>(*i);
		if(ffbet)
		{
			if(deletePlayingEffects || !ffbet->isPlaying())
			{
				ForceFeedbackEffectTemplateList::removeForceFeedbackEffectTemplate(ffbet);
				delete ffbet;
				ms_templatesToReleaseWhenDonePlaying.erase(i);
			}
		}
		else
		{
			FATAL(true, ("Bad ForceFeedbackEffectTemplate in deleteAllQueuedDeletes"));
		}
	}
}

// ----------------------------------------------------------------------

void ForceFeedbackEffectTemplate::addEffect(DirectInput::ForceFeedbackEffect * const effect)
{
	DEBUG_FATAL(!effect, ("No effect"));
	m_effects.push_back(effect);
}

// ----------------------------------------------------------------------

bool ForceFeedbackEffectTemplate::isPlaying() const
{
	if(m_effects.empty()) //lint !e774 always evals to false (not in release)
		return false;

	for(std::vector<DirectInput::ForceFeedbackEffect *>::const_iterator i = m_effects.begin(); i != m_effects.end(); ++i)
	{
		bool const thisOneIsPlaying = (*i)->isPlaying();
		if(thisOneIsPlaying)
			return true;
	}
	return false;
}

// ----------------------------------------------------------------------

bool ForceFeedbackEffectTemplate::playEffect(int const iterations) const
{
	if(m_effects.empty()) //lint !e774 always evals to false (not in release)
		return false;
	
	bool result = true;
	for(std::vector<DirectInput::ForceFeedbackEffect *>::const_iterator i = m_effects.begin(); i != m_effects.end(); ++i)
	{		
		bool const success = (*i)->play(iterations);
		if(!success)
			return result = false;
	}

	return result;
}

// ----------------------------------------------------------------------

bool ForceFeedbackEffectTemplate::stopEffect() const
{
	if(m_effects.empty()) //lint !e774 always evals to false (not in release)
		return false;

	bool result = true;
	for(std::vector<DirectInput::ForceFeedbackEffect *>::const_iterator i = m_effects.begin(); i != m_effects.end(); ++i)
	{		
		bool const success = (*i)->stop();
		if(!success)
			return result = false;
	}
	return result;
}

// ======================================================================
