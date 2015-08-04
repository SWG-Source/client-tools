// ============================================================================
//
// PEAttributeViewParticleMesh.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef PEATTRIBUTEVIEWPARTICLEMESH_H
#define PEATTRIBUTEVIEWPARTICLEMESH_H

#include "PEAttributeView.h"
#include "clientParticle/ParticleDescriptionMesh.h"

class ColorRampAttribute;
class ParticleAttribute;
class ParticleNameAttribute;
class PEAttributeWidgetMesh;
class TextureAttribute;
class WaveFormAttribute;

//-----------------------------------------------------------------------------
class PEAttributeViewParticleMesh : public PEAttributeView
{
	Q_OBJECT

public:

	static ParticleBoundFloat m_boundScale;
	static ParticleBoundFloat m_boundRotation;
	static ParticleBoundFloat m_boundAlpha;
	static ParticleBoundFloat m_boundSpeedScale;

	PEAttributeViewParticleMesh(QWidget *parentWidget, char const *name);

	void                    setParticleDescriptionMesh(ParticleDescriptionMesh const &particleDescriptionMesh);
	ParticleDescriptionMesh getParticleDescriptionMesh() const;
	virtual void            setNewCaption(char const *caption);

public slots:

	void onAttributeNameChanged(const QString &);

private:

	ParticleNameAttribute *m_nameWidget;
	ParticleAttribute *    m_particleBaseWidget;
	PEAttributeWidgetMesh *m_attributeWidgetMesh;
	WaveFormAttribute *    m_particleScaleWidget;
	WaveFormAttribute *    m_particleRotationXWidget;
	WaveFormAttribute *    m_particleRotationYWidget;
	WaveFormAttribute *    m_particleRotationZWidget;
	ColorRampAttribute *   m_particleColorWidget;
	WaveFormAttribute *    m_particleAlphaWidget;
	WaveFormAttribute *    m_particleSpeedScaleWidget;
};

// ============================================================================

#endif // PEATTRIBUTEVIEWPARTICLEMESH_H