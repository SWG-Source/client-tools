// ======================================================================
//
// SetupClientParticle.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/SetupClientParticle.h"

#include "clientParticle/ConfigClientParticle.h"
#include "clientParticle/LightningAppearanceTemplate.h"
#include "clientParticle/LightningManager.h"
#include "clientParticle/ParticleAttachment.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/ParticleEffectAppearanceTemplate.h"
#include "clientParticle/ParticleEmitter.h"
#include "clientParticle/ParticleEmitterGroup.h"
#include "clientParticle/ParticleMesh.h"
#include "clientParticle/ParticleQuad.h"
#include "clientParticle/ParticleManager.h"
#include "clientParticle/SwooshAppearanceTemplate.h"
#include "sharedDebug/InstallTimer.h"

// ======================================================================

SetupClientParticle::AddNotificationFunction SetupClientParticle::ms_addNotificationFunction;

// ======================================================================

void SetupClientParticle::install ()
{
	InstallTimer const installTimer("SetupClientParticle::install");

	ParticleEffectAppearanceTemplate::install();
	ParticleEmitterGroup::install();
	ParticleEmitter::install();
	ParticleMesh::install();
	ParticleQuad::install();
	ParticleAttachment::install();
	ConfigClientParticle::install();
	ParticleManager::install();
	ParticleEffectAppearance::install ();
	LightningAppearanceTemplate::install();
	LightningManager::install();
	SwooshAppearanceTemplate::install();
}

// ----------------------------------------------------------------------

void SetupClientParticle::setAddNotificationFunction (SetupClientParticle::AddNotificationFunction addNotificationFunction)
{
	if (addNotificationFunction != addNotifications)
		ms_addNotificationFunction = addNotificationFunction;
	else
		DEBUG_FATAL (true, ("SetupClientParticle::setAddNotificationFunction set to SetupClientParticle::addNotifications which will cause infinite recursion"));
}

// ----------------------------------------------------------------------

void SetupClientParticle::addNotifications (Object& object)
{
	if (ms_addNotificationFunction)
		ms_addNotificationFunction (object);
}

// ======================================================================
