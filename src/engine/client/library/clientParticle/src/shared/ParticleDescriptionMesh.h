// ============================================================================
//
// ParticleDescriptionMesh.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleDescriptionMesh_H
#define INCLUDED_ParticleDescriptionMesh_H

#include "clientParticle/ParticleDescription.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/PersistentCrcString.h"
#include "sharedMath/WaveForm.h"

class AppearanceTemplate;
class MeshAppearanceTemplate;
class MeshAppearance;

//-----------------------------------------------------------------------------
class ParticleDescriptionMesh : public ParticleDescription
{
public:

	ParticleDescriptionMesh();
	ParticleDescriptionMesh(ParticleDescriptionMesh const &particleDescription);
	~ParticleDescriptionMesh();

	static void                     setDefaultScale(WaveForm &waveForm);
	static void                     setDefaultRotationX(WaveForm &waveForm);
	static void                     setDefaultRotationY(WaveForm &waveForm);
	static void                     setDefaultRotationZ(WaveForm &waveForm);

	virtual ParticleDescription *clone() const;
	virtual ParticleType         getParticleType() const;
	virtual void                 initializeDefault();
	virtual bool                 load(Iff &iff);
	virtual void                 write(Iff &iff) const;

	void                         setMeshPath(char const *path);
	PersistentCrcString const &getMeshPath() const;

	WaveForm m_scale;
	WaveForm m_rotationX;
	WaveForm m_rotationY;
	WaveForm m_rotationZ;

private:

	void load_0000(Iff &iff);

	AppearanceTemplate const *m_cachedMeshAppearanceTemplate;
	PersistentCrcString       m_meshPath;

private:

	// Disabled

	ParticleDescriptionMesh &operator =(ParticleDescriptionMesh const &);
};

// ============================================================================

#endif // INCLUDED_ParticleDescriptionMesh_H
