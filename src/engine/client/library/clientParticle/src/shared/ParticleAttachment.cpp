// ======================================================================
//
// ParticleAttachment.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleAttachment.h"

#include "clientGraphics/RenderWorld.h"
#include "clientParticle/ConfigClientParticle.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/ParticleManager.h"
#include "clientParticle/SwooshAppearance.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/Vector.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================
//
// ParticleAttachmentNamespace
//
// ======================================================================

namespace ParticleAttachmentNamespace
{
	bool s_installed = false;
	int s_elementsPerBlock = 512;
}

using namespace ParticleAttachmentNamespace;

// ======================================================================
//
// ParticleAttachment
//
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ParticleAttachment, false, s_elementsPerBlock, 1, ConfigClientParticle::getMaxParticleAttachments() / s_elementsPerBlock);

//-----------------------------------------------------------------------------
void ParticleAttachment::install()
{
	DEBUG_FATAL(s_installed, ("Already installed"));

	installMemoryBlockManager();

	ExitChain::add(&remove, "ParticleAttachmentNamespace::remove()");

	s_installed = true;
}

//-----------------------------------------------------------------------------
void ParticleAttachment::remove()
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	removeMemoryBlockManager();

	s_installed = false;
}

//-----------------------------------------------------------------------------
bool ParticleAttachment::isPoolFull()
{
	return ms_memoryBlockManager->isFull();
}

// ----------------------------------------------------------------------
int ParticleAttachment::getGlobalCount()
{
	return ms_memoryBlockManager->getElementCount();
}

// ----------------------------------------------------------------------
ParticleAttachment::ParticleAttachment()
 : Object()
 , m_killWhenParticleDies(true)
{
}

// ----------------------------------------------------------------------
void ParticleAttachment::setPosition(Vector const & translation)
{
	if (isChildObject())
	{
		setPosition_p(translation);
	}
	else
	{
		setPosition_w(translation);
	}
}

// ----------------------------------------------------------------------
void ParticleAttachment::kill()
{
	ParticleEffectAppearance * const particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(getAppearance());

	if (particleEffectAppearance != NULL)
	{
		particleEffectAppearance->setEnabled(false);
	}
	else
	{
		SwooshAppearance * const swooshAppearance = dynamic_cast<SwooshAppearance *>(getAppearance());
		if(swooshAppearance != NULL)
		{
			swooshAppearance->kill();
		}
	}
}

// ----------------------------------------------------------------------
void ParticleAttachment::setKillWhenParticleDies(bool const killWhenParticleDies)
{
	m_killWhenParticleDies = killWhenParticleDies;
}

// ----------------------------------------------------------------------
bool ParticleAttachment::getKillWhenParticleDies() const
{
	return m_killWhenParticleDies;
}

// ======================================================================
