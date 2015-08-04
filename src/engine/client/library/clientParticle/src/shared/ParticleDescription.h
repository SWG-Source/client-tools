// ============================================================================
//
// ParticleDescription.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleDescription_H
#define INCLUDED_ParticleDescription_H

#include "clientParticle/ColorRamp.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"
#include "sharedMath/WaveForm.h"

class ParticleAttachmentDescription;

//-----------------------------------------------------------------------------
class ParticleDescription
{
public:

	typedef stdvector<ParticleAttachmentDescription>::fwd ParticleAttachmentDescriptions;

public:

	ParticleDescription();
	ParticleDescription(ParticleDescription const &particleDescription);
	virtual ~ParticleDescription() = 0;

	enum ParticleType
	{
		PT_quad,
		PT_mesh
	};

	static ParticleDescription *createDescription(ParticleType const particleType);

	static void setDefaultColor(ColorRamp &colorRamp);
	static void setDefaultAlpha(WaveForm &waveForm);
	static void setDefaultSpeedScale(WaveForm &waveForm);
	static void setDefaultParticleRelativeRotationX(WaveForm &waveForm);
	static void setDefaultParticleRelativeRotationY(WaveForm &waveForm);
	static void setDefaultParticleRelativeRotationZ(WaveForm &waveForm);
	static Tag  getTag();

	virtual ParticleDescription *clone() const = 0;
	virtual ParticleType         getParticleType() const = 0;
	virtual void                 initializeDefault();
	virtual bool                 load(Iff &iff);
	virtual void                 write(Iff &iff) const;

	void setName(char const *name);
	void setRandomRotationDirection(bool const randomRotationDirection);
	void setColor(ColorRamp const &color);
	void setAlpha(WaveForm const &alpha);
	void setSpeedScale(WaveForm const &speedScale);
	void setParticleRelativeRotationX(WaveForm const &particleRelativeRotationX);
	void setParticleRelativeRotationY(WaveForm const &particleRelativeRotationY);
	void setParticleRelativeRotationZ(WaveForm const &particleRelativeRotationZ);
	void setParticleAttachmentDescriptions(ParticleAttachmentDescriptions &particleAttachmentDescriptions);

	std::string const &                   getName() const;
	bool                                  isRandomRotationDirection() const;
	ColorRamp const &                     getColor() const;
	WaveForm const &                      getAlpha() const;
	WaveForm const &                      getSpeedScale() const;
	WaveForm const &                      getParticleRelativeRotationX() const;
	WaveForm const &                      getParticleRelativeRotationY() const;
	WaveForm const &                      getParticleRelativeRotationZ() const;
	bool const                            getUsesParticleRelativeRotation() const;

	ParticleAttachmentDescriptions const &getParticleAttachmentDescriptions() const;
	bool                                  isInfiniteLooping() const;

	void clearParticleAttachmentDescriptions();

protected:

	ColorRamp                       m_color;
	WaveForm                        m_alpha;
	WaveForm                        m_speedScale;

	
	WaveForm                        m_particleRelativeRotationX;             
	WaveForm                        m_particleRelativeRotationY;             
	WaveForm                        m_particleRelativeRotationZ;        
	
	bool                            m_particleUsesRelativeRotation;

private:

	static std::string const        m_defaultName;
	static bool        const        m_defaultRandomRotationDirection;
	static Tag const                m_tag;
	std::string                     m_name;
	bool                            m_randomRotationDirection;
	ParticleAttachmentDescriptions *m_particleAttachmentDescriptions;

	void load_0000(Iff &iff);
	void load_0001(Iff &iff);
	void load_0002(Iff &iff);
	void load_0003(Iff &iff);

private:

	// Disabled

	ParticleDescription &operator =(ParticleDescription const &);
};

// ============================================================================

#endif // INCLUDED_ParticleDescription_H
