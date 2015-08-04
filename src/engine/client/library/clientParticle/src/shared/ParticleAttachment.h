// ============================================================================
//
// ParticleAttachment.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleAttachment_H
#define INCLUDED_ParticleAttachment_H

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/Object.h"

class ParticleEffectAppearance;
class Vector;

//-----------------------------------------------------------------------------
class ParticleAttachment : public Object
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();
	static bool isPoolFull();
	static int getGlobalCount();

public:

	ParticleAttachment();

	void setKillWhenParticleDies(bool const killWhenParticleDies);
	bool getKillWhenParticleDies() const;

	void setPosition(Vector const & position);

	void kill();

private:

	bool m_killWhenParticleDies;

	static void remove();

	// Disabled

	ParticleAttachment(ParticleAttachment const &);
	ParticleAttachment & operator =(ParticleAttachment const &);
};

// ============================================================================

#endif // INCLUDED_ParticleAttachment_H
