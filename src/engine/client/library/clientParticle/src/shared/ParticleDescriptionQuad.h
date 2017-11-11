// ============================================================================
//
// ParticleDescriptionQuad.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleDescriptionQuad_H
#define INCLUDED_ParticleDescriptionQuad_H

#include "clientParticle/ParticleDescription.h"
#include "clientParticle/ParticleTexture.h"
#include "sharedMath/WaveForm.h"

//-----------------------------------------------------------------------------
class ParticleDescriptionQuad : public ParticleDescription
{
public:

	ParticleDescriptionQuad();
	ParticleDescriptionQuad(ParticleDescriptionQuad const &particleDescriptionQuad);

	static void                     setDefaultRotation(WaveForm &waveForm);
	static void                     setDefaultLength(WaveForm &waveForm);
	static void                     setDefaultWidth(WaveForm &waveForm);

	virtual ParticleDescription *clone() const;
	virtual ParticleType         getParticleType() const;
	virtual void                 initializeDefault();
	virtual bool                 load(Iff &iff);
	virtual void                 write(Iff &iff) const;

	void setRotation(WaveForm const &rotation);
	void setLength(WaveForm const &length);
	void setWidth(WaveForm const &width);
	void setLengthAndWidthLinked(bool const lengthAndWidthLinked);
	void setParticleTexture(ParticleTexture const &particleTexture);

	WaveForm const &       getRotation() const;
	WaveForm const &       getLength() const;
	WaveForm const &       getWidth() const;
	bool                   isLengthAndWidthLinked() const;
	ParticleTexture const &getParticleTexture() const;

private:

	WaveForm        m_rotation;
	WaveForm        m_length;
	WaveForm        m_width;
	bool            m_lengthAndWidthLinked;
	ParticleTexture m_particleTexture;

	void load_old_0000(Iff &iff); // These are for backwards compatibility.
	void load_old_0001(Iff &iff);

	void load_0000(Iff &iff);
	void load_0001(Iff &iff);

private:

	// Disabled

	ParticleDescriptionQuad &operator =(ParticleDescriptionQuad const &);
};

// ============================================================================

#endif // INCLUDED_ParticleDescriptionQuad_H