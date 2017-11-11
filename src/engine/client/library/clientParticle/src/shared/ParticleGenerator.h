// ============================================================================
//
// ParticleGenerator.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleGenerator_H
#define INCLUDED_ParticleGenerator_H

#include "sharedCollision/BoxExtent.h"

class Camera;
class Object;
class ParticleEffectAppearance;

//-----------------------------------------------------------------------------
class ParticleGenerator
{
public:

	ParticleGenerator(ParticleEffectAppearance const * const parentParticleEffectAppearance);
	virtual ~ParticleGenerator() {}

	virtual void             addToCameraScene(Camera const *camera, Object const *object) const = 0;
	virtual void             alter(float const deltaTime) = 0;
	virtual void             restart() = 0;
	virtual bool             isDeletable() const = 0;
	virtual void             setOwner(Object *newOwner) = 0;
	virtual void             addToWorld() = 0;
	virtual void             removeFromWorld() = 0;

	virtual int              getParticleCount() const = 0;
	virtual BoxExtent const &getExtent() const = 0;

	ParticleEffectAppearance const & getParentParticleEffectAppearance() const;

protected:

	BoxExtent mutable         m_extent_w; // World space

private:

	ParticleEffectAppearance const * const m_parentParticleEffectAppearance;

private:

	ParticleGenerator(ParticleGenerator const &);
	ParticleGenerator &operator =(ParticleGenerator const &);
};

// ============================================================================

#endif // INCLUDED_ParticleGenerator_H
