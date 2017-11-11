// ============================================================================
//
// PEAttributeViewer.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef PEATTRIBUTEVIEWER_H
#define PEATTRIBUTEVIEWER_H

class AttributeViewParticleAttachment;
class ColorRamp;
class ParticleAttachmentDescription;
class ParticleDescription;
class ParticleDescriptionMesh;
class ParticleDescriptionQuad;
class ParticleEmitterDescription;
class ParticleEmitterGroupDescription;
class ParticleEffectDescription;
class PEAttributeView;
class PEAttributeViewParticleQuad;
class PEAttributeViewParticleMesh;
class PEAttributeViewParticleEmitter;
class PEAttributeViewParticleEmitterGroup;
class PEAttributeViewParticleEffect;
class PEAttributeViewParticleEffectGroup;
class WaveForm;

//-----------------------------------------------------------------------------
class PEAttributeViewer : public QWidget
{
	Q_OBJECT

public:

	enum AttributeView
	{
		AV_particleEffect,
		AV_particleEffectGroup,
		AV_particleEmitter,
		AV_particleEmitterGroup,
		AV_particleMesh,
		AV_particleQuad,
		AV_particleAttachment,
		AV_invalid = -1
	};

	PEAttributeViewer(QWidget *parentWidgetWidget, char const *name, WFlags const flags = 0);

	PEAttributeViewParticleEmitter const &     getAttributeViewParticleEmitter() const;
	PEAttributeViewParticleEmitterGroup const &getAttributeViewParticleEmitterGroup() const;
	PEAttributeViewParticleEffect const &      getAttributeViewParticleEffect() const;
	PEAttributeViewParticleEffectGroup const & getAttributeViewParticleEffectGroup() const;
	PEAttributeViewParticleMesh const &        getAttributeViewParticleMesh() const;
	PEAttributeViewParticleQuad const &        getAttributeViewParticleQuad() const;
	AttributeViewParticleAttachment const &    getAttributeViewParticleAttachment() const;

signals:

	void colorRampAttributeSelected(const ColorRamp &, const WaveForm &);
	void waveFormAttributeSelected(const WaveForm &);
	void attributeChanged();
	void attributeNameChanged(const std::string &);
	void particleAttachmentDescriptionChanged(const ParticleAttachmentDescription &);
	void particleDescriptionChanged(const ParticleDescription &);
	void particleEmitterDescriptionChanged(const ParticleEmitterDescription &);
	void particleEmitterGroupDescriptionChanged(const ParticleEmitterGroupDescription &);
	void particleEffectDescriptionChanged(const ParticleEffectDescription &);
	void buildParticleEffect();

public slots:

	void slotSwitchAttributeView(const PEAttributeViewer::AttributeView attributeView);
	void onAttributeChanged();
	void onAttributeChangedForceRebuild();
	void onAttributeChangedForceRebuild(int);
	void onAttributeNameChanged(const QString &name);
	void onColorRampAttributeSelected(const ColorRamp &, const WaveForm &);
	void onColorRampChanged(const ColorRamp &, const WaveForm &, const bool);
	void onShowParticleEffectAttributes(const ParticleEffectDescription &particleEffectDescription);
	void onShowParticleEffectGroupAttributes();
	void onShowParticleEmitterAttributes(const ParticleEmitterDescription &particleEmitterDescription);
	void onShowParticleEmitterGroupAttributes(const ParticleEmitterGroupDescription &particleEmitterGroupDescription);
	void onShowParticleAttributes(const ParticleDescription &particleDescription);
	void onShowParticleAttachmentAttributes(const ParticleAttachmentDescription &particleAttachmentDescription);
	void onWaveFormAttributeSelected(const WaveForm &);
	void onWaveFormChanged(const WaveForm &, const bool);

private:

	void         hideAll();
	virtual void resizeEvent(QResizeEvent *);

	AttributeView                        m_attributeView;
	PEAttributeViewParticleEffect *      m_attributeViewParticleEffect;
	PEAttributeViewParticleEffectGroup * m_attributeViewParticleEffectGroup;
	PEAttributeViewParticleEmitter *     m_attributeViewParticleEmitter;
	PEAttributeViewParticleEmitterGroup *m_attributeViewParticleEmitterGroup;
	PEAttributeViewParticleMesh *        m_attributeViewParticleMesh;
	PEAttributeViewParticleQuad *        m_attributeViewParticleQuad;
	AttributeViewParticleAttachment *    m_attributeViewParticleAttachment;
	PEAttributeView *                    m_attributeViewSelected;
	QPoint                               m_lastParticlePosition;
	QPoint                               m_lastEmitterPosition;
};

// ============================================================================

#endif // PEATTRIBUTEVIEWER_H
