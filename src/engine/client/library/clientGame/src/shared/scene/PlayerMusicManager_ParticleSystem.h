// ============================================================================
//
// PlayerMusicManager_ParticleSystem.h
// Copyright Sony Online Entertainment Inc.
// 
// ============================================================================

#ifndef INCLUDED_PlayerMusicManager_ParticleSystem_H
#define INCLUDED_PlayerMusicManager_ParticleSystem_H

#include "clientGame/PlayerMusicManager.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

class CreatureObject;
class Object;
class ParticleEffectAppearance;

//-------------------------------------------------------------------------
class PlayerMusicManager::ParticleSystem
{
public:
	
	ParticleSystem();
	~ParticleSystem();

	void create(CreatureObject *parentObject, std::string const &path);

	void setEnabled(bool const enabled);
	void setAllowDelete(bool const allowDelete);

	bool isEnabled() const;
	bool isDeletable() const;
	bool isAllowDelete() const;

	std::string getParticleSystemPath() const;

private:

	Watcher<CreatureObject>   m_parentObject;
	Watcher<Object>           m_particleObject;
	ParticleEffectAppearance *m_particleEffectAppearance;
	bool                      m_allowDelete;
};

// ============================================================================

#endif // INCLUDED_PlayerMusicManager_ParticleSystem_H
