// ============================================================================
//
// PlayerMusicManager_ParticleSystem.cpp
// Copyright Sony Online Entertainment Inc.
// 
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlayerMusicManager_ParticleSystem.h"

#include "clientGame/CreatureObject.h"
#include "clientGraphics/RenderWorld.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "sharedObject/AppearanceTemplateList.h"

// ============================================================================
//
// PlayerMusicManager::ParticleSystem
//
// ============================================================================

//-----------------------------------------------------------------------------
PlayerMusicManager::ParticleSystem::ParticleSystem()
 : m_parentObject(NULL)
 , m_particleObject(NULL)
 , m_particleEffectAppearance(NULL)
 , m_allowDelete(true)
{
	//DEBUG_REPORT_LOG(true, ("PlayerMusicManager::ParticleSystem::ParticleSystem()\n"));
}

//-----------------------------------------------------------------------------
PlayerMusicManager::ParticleSystem::~ParticleSystem()
{
	//DEBUG_REPORT_LOG(true, ("PlayerMusicManager::ParticleSystem::~ParticleSystem()\n"));

	if ((m_parentObject != NULL) &&
		(m_particleObject != NULL))
	{
		m_parentObject->removeChildObject(m_particleObject, Object::DF_parent);
	}

	if (m_particleObject != NULL)
	{
		delete m_particleObject.getPointer();
	}

	m_parentObject = NULL;
	m_particleEffectAppearance = NULL;
}

//-----------------------------------------------------------------------------
void PlayerMusicManager::ParticleSystem::create(CreatureObject *parentObject, std::string const &path)
{
	//DEBUG_REPORT_LOG(true, ("PlayerMusicManager::ParticleSystem::create()\n"));

	DEBUG_WARNING(path.empty(), ("The player music particle system path is empty."));

	m_parentObject = parentObject;

	if (m_parentObject != NULL)
	{
		setAllowDelete(false);

		if (m_particleObject == NULL)
		{
			m_particleObject = new Object();
			RenderWorld::addObjectNotifications(*m_particleObject);
			m_parentObject->addChildObject_o(m_particleObject);

			//DEBUG_REPORT_LOG(true, ("PlayerMusicManager::ParticleSystem::create() - Particle system added as child object: %s\n", m_parentObject->getObjectTemplateName()));
		}

		if ((m_particleEffectAppearance != NULL) &&
		    (m_particleEffectAppearance->getAppearanceTemplateName() == path))
		{
			// The particle system ystme is the same as the requested system so
			// just turn it back on

			m_particleEffectAppearance->setEnabled(true);
		}
		else
		{
			// Create an new particle system appearance

			m_particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(AppearanceTemplateList::createAppearance(path.c_str()));
			DEBUG_WARNING((m_particleEffectAppearance == NULL), ("Particle effect appearance is NULL: %s", path.c_str()));

			m_particleObject->setAppearance(m_particleEffectAppearance);
		}

		if (m_particleEffectAppearance != NULL)
		{
			m_particleEffectAppearance->setAutoDelete(false);
			m_particleEffectAppearance->setEnabled(true);
		}
	}
}

//-----------------------------------------------------------------------------
void PlayerMusicManager::ParticleSystem::setEnabled(bool const enabled)
{
	if ((m_particleObject != NULL) &&
	    (m_particleEffectAppearance != NULL))
	{
		m_particleEffectAppearance->setEnabled(enabled);

		//DEBUG_REPORT_LOG(true, ("PlayerMusicManager::ParticleSystem::setEnabled(%s) - %s - deletable: %s\n", m_particleEffectAppearance->getAppearanceTemplateName(), enabled ? "enabled" : "disabled", m_allowDelete ? "yes" : "no"));
	}
}

//-----------------------------------------------------------------------------
bool PlayerMusicManager::ParticleSystem::isEnabled() const
{
	bool result = false;

	if ((m_particleObject != NULL) &&
		(m_particleEffectAppearance != NULL))
	{
		result = m_particleEffectAppearance->isEnabled();
	}

	return result;
}

//-----------------------------------------------------------------------------
std::string PlayerMusicManager::ParticleSystem::getParticleSystemPath() const
{
	std::string result;

	if ((m_particleObject != NULL) &&
	    (m_particleEffectAppearance != NULL))
	{
		result = m_particleEffectAppearance->getAppearanceTemplateName();
	}

	return result;
}

//-----------------------------------------------------------------------------
bool PlayerMusicManager::ParticleSystem::isDeletable() const
{
	bool result = true;

	if ((m_particleObject != NULL) &&
	    (m_particleEffectAppearance != NULL))
	{
		if (m_allowDelete)
		{
			result = m_particleEffectAppearance->isDeletable();
		}
		else
		{
			result = false;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void PlayerMusicManager::ParticleSystem::setAllowDelete(bool const allowDelete)
{
	m_allowDelete = allowDelete;
}

//-----------------------------------------------------------------------------
bool PlayerMusicManager::ParticleSystem::isAllowDelete() const
{
	return m_allowDelete;
}

// ============================================================================
