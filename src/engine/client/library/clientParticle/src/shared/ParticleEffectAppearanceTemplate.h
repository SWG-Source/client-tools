// ============================================================================
//
// ParticleEffectAppearanceTemplate.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleEffectAppearanceTemplate_H
#define INCLUDED_ParticleEffectAppearanceTemplate_H

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "sharedObject/AppearanceTemplate.h"

class Appearance;
class Iff;
class ParticleEffectDescription;

//-----------------------------------------------------------------------------
class ParticleEffectAppearanceTemplate : public AppearanceTemplate
{
public:

	explicit ParticleEffectAppearanceTemplate(const char *name, Iff *iff = NULL);
	virtual ~ParticleEffectAppearanceTemplate();

	virtual Appearance *       createAppearance() const;
	static AppearanceTemplate *create(const char *name, Iff *iff);
	static void                install();
	static void                remove();
	static Tag                 getTag();

	ParticleEffectDescription *m_particleEffectDescription;

	bool isInfiniteLooping() const;

private:

#ifdef _DEBUG
	int m_creationIndex;
#endif // _DEBUG

	// Disabled

	ParticleEffectAppearanceTemplate();
	ParticleEffectAppearanceTemplate(const ParticleEffectAppearanceTemplate &);
	ParticleEffectAppearanceTemplate &operator =(const ParticleEffectAppearanceTemplate &);
};

//=============================================================================

#endif // INCLUDED_ParticleEffectAppearanceTemplate_H
