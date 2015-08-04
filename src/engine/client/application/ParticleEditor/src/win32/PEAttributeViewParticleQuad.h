// ============================================================================
//
// PEAttributeViewParticleQuad.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef PEATTRIBUTEVIEWPARTICLEQUAD_H
#define PEATTRIBUTEVIEWPARTICLEQUAD_H

#include "Attribute.h"
#include "clientParticle/ParticleDescriptionQuad.h"
#include "PEAttributeView.h"

class ParticleAttribute;
class ParticleNameAttribute;
class TextureAttribute;

//-----------------------------------------------------------------------------
class PEAttributeViewParticleQuad : public PEAttributeView
{
	Q_OBJECT

public:

	static ParticleBoundFloat m_boundLength;
	static ParticleBoundFloat m_boundWidth;
	static ParticleBoundFloat m_boundRotation;
	static ParticleBoundFloat m_boundAlpha;
	static ParticleBoundFloat m_boundSpeedScale;
	static ParticleBoundFloat m_boundParticleRelativeRotationX;
	static ParticleBoundFloat m_boundParticleRelativeRotationY;
	static ParticleBoundFloat m_boundParticleRelativeRotationZ;

	PEAttributeViewParticleQuad(QWidget *parentWidget, char const *name);

	void                    setParticleDescriptionQuad(ParticleDescriptionQuad const &particleDescriptionQuad);
	ParticleDescriptionQuad getParticleDescriptionQuad() const;
	virtual void            setNewCaption(char const *caption);

public slots:

	void onAttributeNameChanged(const QString &);

private slots:

	void slotParticleLengthWidgetModifiableCheckBoxClicked();

private:

	ParticleNameAttribute * m_nameWidget;
	ParticleAttribute *     m_particleBaseWidget;
	TextureAttribute *      m_particleTextureWidget;
	WaveFormAttribute *     m_particleLengthWidget;
	WaveFormAttribute *     m_particleWidthWidget;
	WaveFormAttribute *     m_particleRotationWidget;
	ColorRampAttribute *    m_particleColorWidget;
	WaveFormAttribute *     m_particleAlphaWidget;
	WaveFormAttribute *     m_particleSpeedScaleWidget;
	WaveFormAttribute *     m_particleRelativeRotationXWidget;
	WaveFormAttribute *     m_particleRelativeRotationYWidget;
	WaveFormAttribute *     m_particleRelativeRotationZWidget;
};

// ============================================================================

#endif // PEATTRIBUTEVIEWPARTICLEQUAD_H