// ============================================================================
//
// ParticleEffectAppearanceTemplate.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleEffectAppearanceTemplate.h"

#include "clientParticle/ConfigClientParticle.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AppearanceTemplateList.h"

// ============================================================================
//
// ParticleEffectAppearanceTemplateNamespace
//
// ============================================================================

namespace ParticleEffectAppearanceTemplateNamespace
{
	bool s_installed = false;
	Tag s_tag = TAG(P,E,F,T);

#ifdef _DEBUG
	int s_creationIndex = 0;
#endif // _DEBUG
}

using namespace ParticleEffectAppearanceTemplateNamespace;


// ============================================================================
//
// ParticleEffectAppearanceTemplate
//
// ============================================================================

//-----------------------------------------------------------------------------
ParticleEffectAppearanceTemplate::ParticleEffectAppearanceTemplate(const char *name, Iff *iff)
 : AppearanceTemplate(name)
 , m_particleEffectDescription(NULL)
#ifdef _DEBUG
 , m_creationIndex(++s_creationIndex)
#endif // _DEBUG
{
	DEBUG_REPORT_LOG(ConfigClientParticle::isDebugEnabled(), ("[%d] (0x%p) PARTICLE TEMPLATE CREATED(%s)\n", m_creationIndex, this, getCrcName().isEmpty() ? "empty name" : getName()));

	m_particleEffectDescription = new ParticleEffectDescription;
	NOT_NULL(m_particleEffectDescription);
	m_particleEffectDescription->load(*NON_NULL(iff));
}

//-----------------------------------------------------------------------------
ParticleEffectAppearanceTemplate::~ParticleEffectAppearanceTemplate()
{
	DEBUG_REPORT_LOG(ConfigClientParticle::isDebugEnabled(), ("[%d] (0x%p) PARTICLE TEMPLATE DELETED(%s)\n", m_creationIndex, this, getCrcName().isEmpty() ? "empty name" : getName()));

	delete m_particleEffectDescription;
}

//-----------------------------------------------------------------------------
AppearanceTemplate *ParticleEffectAppearanceTemplate::create(const char* name, Iff* iff)
{
	return new ParticleEffectAppearanceTemplate(name, iff);
}

//-----------------------------------------------------------------------------
void ParticleEffectAppearanceTemplate::install()
{
	DEBUG_FATAL(s_installed, ("Already installed"));

	AppearanceTemplateList::assignBinding(s_tag, create);

	ExitChain::add(&remove, "ParticleAppearanceTemplate::remove()");
	s_installed = true;
}

//-----------------------------------------------------------------------------
void ParticleEffectAppearanceTemplate::remove()
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	AppearanceTemplateList::removeBinding(s_tag);

	s_installed = false;
}

//-----------------------------------------------------------------------------
Appearance *ParticleEffectAppearanceTemplate::createAppearance() const
{
	return new ParticleEffectAppearance(this);
}

//-----------------------------------------------------------------------------
Tag ParticleEffectAppearanceTemplate::getTag()
{
	return s_tag;
}

//-----------------------------------------------------------------------------
bool ParticleEffectAppearanceTemplate::isInfiniteLooping() const
{
	bool result = false;

	if (m_particleEffectDescription != NULL)
	{
		result = m_particleEffectDescription->isInfiniteLooping();
	}

	return result;
}

// ============================================================================
