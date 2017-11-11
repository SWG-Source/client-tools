// ============================================================================
//
// PEAttributeViewParticleEmitter.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef PEATTRIBUTEVIEWPARTICLEEMITTER_H
#define PEATTRIBUTEVIEWPARTICLEEMITTER_H

#include "clientParticle/ParticleEmitterDescription.h"
#include "PEAttributeView.h"

class ParticleEmitterAttribute;
class ParticleNameAttribute;
class ParticleTimingAttribute;
class WaveFormAttribute;

//-----------------------------------------------------------------------------
class PEAttributeViewParticleEmitter : public PEAttributeView
{
	Q_OBJECT

public:

	enum LevelOfDetail
	{
		LOD_global,
		LOD_none,
		LOD_specified
	};

	static ParticleBoundFloat m_boundEmitterLifeTime;
	static ParticleBoundFloat m_boundEmitterMinLod;
	static ParticleBoundFloat m_boundEmitterMaxLod;
	static ParticleBoundInt   m_boundEmitterMaxParticles;
	static ParticleBoundFloat m_boundEmitterTranslationX;
	static ParticleBoundFloat m_boundEmitterTranslationY;
	static ParticleBoundFloat m_boundEmitterTranslationZ;
	static ParticleBoundFloat m_boundEmitterRotationX;
	static ParticleBoundFloat m_boundEmitterRotationY;
	static ParticleBoundFloat m_boundEmitterRotationZ;
	static ParticleBoundFloat m_boundEmitterDistance;
	static ParticleBoundFloat m_boundEmitterShapeSize;
	static ParticleBoundFloat m_boundEmitterSpread;
	static ParticleBoundFloat m_boundParticleGenerationRate;
	static ParticleBoundFloat m_boundParticleEmitSpeed;
	static ParticleBoundFloat m_boundParticleInheritVelocityPercentage;
	static ParticleBoundFloat m_boundParticleClusterCount;
	static ParticleBoundFloat m_boundParticleClusterRadius;
	static ParticleBoundFloat m_boundParticleLifetime;
	static ParticleBoundFloat m_boundParticleWeight;
	static ParticleBoundFloat m_boundParticleHeightAboveGround;
	static ParticleBoundInt   m_boundParticleCollisionVelocityMaintained;
	static ParticleBoundFloat m_boundWindResistanceGlobal;
	static ParticleBoundFloat m_boundFlockingSeperationDistance;
	static ParticleBoundFloat m_boundFlockingSeperationGain;
	static ParticleBoundFloat m_boundFlockingAlignmentGain;
	static ParticleBoundFloat m_boundFlockingCohesionGain;
	static ParticleBoundFloat m_boundFlockingCageWidth;
	static ParticleBoundFloat m_boundFlockingCageHeight;
	static ParticleBoundFloat m_boundParticleTimeOfDayColorPercent;
	static ParticleBoundFloat m_boundParticleSnapToTerrainOnCreationHeight;
	static ParticleBoundFloat m_boundParticleChangeDirectionDegree;
	static ParticleBoundFloat m_boundParticleChangeDirectionTime;

	PEAttributeViewParticleEmitter(QWidget *parentWidget, char const *name);

	void                       setParticleEmitterDescription(ParticleEmitterDescription const &particleEmitterDescription);
	ParticleEmitterDescription getParticleEmitterDescription() const;
	virtual void               setNewCaption(char const *caption);

public slots:

	void randomInitialRotationCheckBoxClicked();
	void loopImmediatelyCheckBoxClicked();
	void oneShotCheckBoxClicked();
	void maxParticlesLineEditReturnPressed();
	void lifeTimeMinLineEditReturnPressed();
	void lifeTimeMaxLineEditReturnPressed();
	void lodMinLineEditReturnPressed();
	void lodMaxLineEditReturnPressed();
	void onAttributeNameChanged(const QString &);

private slots:

	void slotSoundLoadPushButtonClicked();
	void slotSoundRemovePushButtonClicked();
	void slotParticleGenerationRateModifiableCheckBoxClicked();
	void slotEmitterOneShotMinLineEditReturnPressed();
	void slotEmitterOneShotMaxLineEditReturnPressed();
	void slotGroundCollisionCheckBoxClicked();
	void slotKillParticlesOnCollisionCheckBoxClicked();
	void slotParticleFrictionMinLineEditReturnPressed();
	void slotParticleFrictionMaxLineEditReturnPressed();
	void slotParticleResilienceMinLineEditReturnPressed();
	void slotParticleResilienceMaxLineEditReturnPressed();
	void slotAffectedByWindCheckBoxClicked();
	void slotWindResistanceGlobalLineEditReturnPressed();
	void slotValidateAndNotifyChanged();
	void slotValidateAndNotifyChanged(int);

private:

	ParticleNameAttribute *   m_nameWidget;
	ParticleTimingAttribute * m_timingAttribute;
	ParticleEmitterAttribute *m_emitterAttribute;
	WaveFormAttribute *       m_emitterTranslationX;
	WaveFormAttribute *       m_emitterTranslationY;
	WaveFormAttribute *       m_emitterTranslationZ;
	WaveFormAttribute *       m_emitterRotationX;
	WaveFormAttribute *       m_emitterRotationY;
	WaveFormAttribute *       m_emitterRotationZ;
	WaveFormAttribute *       m_emitterDistance;
	WaveFormAttribute *       m_emitterShapeSize;
	WaveFormAttribute *       m_emitterSpread;
	WaveFormAttribute *       m_particleGenerationRate;
	WaveFormAttribute *       m_particleEmitSpeed;
	WaveFormAttribute *       m_particleInheritVelocityPercent;
	WaveFormAttribute *       m_particleClusterCount;
	WaveFormAttribute *       m_particleClusterRadius;
	WaveFormAttribute *       m_particleLifeTime;
	WaveFormAttribute *       m_particleWeight;

	void validate();
};

// ============================================================================

#endif // PEATTRIBUTEVIEWPARTICLEEMITTER_H