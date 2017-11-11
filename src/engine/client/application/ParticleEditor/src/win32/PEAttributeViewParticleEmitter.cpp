// ============================================================================
//
// PEAttributeViewParticleEmitter.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "PEAttributeViewParticleEmitter.h"
#include "PEAttributeViewParticleEmitter.moc"

#include "clientAudio/Audio.h"
#include "clientParticle/ConfigClientParticle.h"
#include "clientParticle/ParticleEmitterShape.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/TreeFile.h"
#include "ParticleEmitterAttribute.h"
#include "ParticleNameAttribute.h"
#include "ParticleTimingAttribute.h"
#include "PEAttributeView.h"

///////////////////////////////////////////////////////////////////////////////
//
// PEAttributeViewParticleEmitter
//
///////////////////////////////////////////////////////////////////////////////

ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundEmitterLifeTime(4096.0f, 0.5f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundEmitterMinLod(ConfigClientParticle::getMinGlobalLodDistanceMax(), ConfigClientParticle::getMinGlobalLodDistanceMin());
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundEmitterMaxLod(ConfigClientParticle::getMaxGlobalLodDistanceMax(), ConfigClientParticle::getMaxGlobalLodDistanceMin());
ParticleBoundInt   PEAttributeViewParticleEmitter::m_boundEmitterMaxParticles(512, 1);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundEmitterTranslationX(128.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundEmitterTranslationY(128.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundEmitterTranslationZ(128.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundEmitterRotationX(4096.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundEmitterRotationY(4096.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundEmitterRotationZ(4096.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundEmitterDistance(4096.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundEmitterShapeSize(4096.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundEmitterSpread(180.0f, 0.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundParticleGenerationRate(4096.0f, 0.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundParticleEmitSpeed(4096.0f, -4096.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundParticleInheritVelocityPercentage(1.0f, 0.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundParticleClusterCount(4096.0f, 0.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundParticleClusterRadius(4096.0f, 0.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundParticleLifetime(4096.0f, 0.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundParticleWeight(4096.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundParticleHeightAboveGround(4096.0f);
ParticleBoundInt   PEAttributeViewParticleEmitter::m_boundParticleCollisionVelocityMaintained(400, 0);

ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundWindResistanceGlobal(1.0f, 0.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundFlockingSeperationDistance(64.0, 0.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundFlockingSeperationGain(32.0, 0.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundFlockingAlignmentGain(32.0f, 0.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundFlockingCohesionGain(32.0f, 0.001f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundFlockingCageWidth(256.0f, 0.01f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundFlockingCageHeight(256.0f, 0.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundParticleTimeOfDayColorPercent(1.0f, 0.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundParticleSnapToTerrainOnCreationHeight(512.0f, 0.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundParticleChangeDirectionDegree(180.0f, 0.0f);
ParticleBoundFloat PEAttributeViewParticleEmitter::m_boundParticleChangeDirectionTime(16.0f, 0.0f);

//-----------------------------------------------------------------------------
PEAttributeViewParticleEmitter::PEAttributeViewParticleEmitter(QWidget *parentWidget, char const *name)
 : PEAttributeView(parentWidget, name)
 , m_emitterAttribute(NULL)
 , m_emitterTranslationX(NULL)
 , m_emitterTranslationY(NULL)
 , m_emitterTranslationZ(NULL)
 , m_emitterRotationX(NULL)
 , m_emitterRotationY(NULL)
 , m_emitterRotationZ(NULL)
 , m_emitterDistance(NULL)
 , m_emitterShapeSize(NULL)
 , m_emitterSpread(NULL)
 , m_particleGenerationRate(NULL)
 , m_particleEmitSpeed(NULL)
 , m_particleInheritVelocityPercent(NULL)
 , m_particleClusterCount(NULL)
 , m_particleClusterRadius(NULL)
 , m_particleLifeTime(NULL)
 , m_particleWeight(NULL)
{
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);

	QIntValidator *intValidator = new QIntValidator(this, "QIntValidator");
	NOT_NULL(intValidator);

	QDoubleValidator *doubleValidator = new QDoubleValidator(this, "QDoubleValidator");
	NOT_NULL(doubleValidator);

	// Name attribute

	m_nameWidget = new ParticleNameAttribute(m_attributesVBox, "ParticleEmitterNameAttribute");
	NOT_NULL(m_nameWidget);

	connect(m_nameWidget->m_nameLineEdit, SIGNAL(textChanged(const QString &)), parentWidget, SLOT(onAttributeNameChanged(const QString &)));
	connect(m_nameWidget->m_nameLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onAttributeNameChanged(const QString &)));

	// Timing attributes

	m_timingAttribute = new ParticleTimingAttribute(m_attributesVBox, "ParticleEmitterTimingAttribute");
	NOT_NULL(m_timingAttribute);

	connect(m_timingAttribute, SIGNAL(timingChanged()), parentWidget, SLOT(onAttributeChangedForceRebuild()));

	// Emitter only attributes

	m_emitterAttribute = new ParticleEmitterAttribute(m_attributesVBox, "ParticleEmitterAttribute");
	NOT_NULL(m_emitterAttribute);

	m_emitterAttribute->m_emitterLifeTimeMinLineEdit->setValidator(doubleValidator);
	m_emitterAttribute->m_emitterLifeTimeMaxLineEdit->setValidator(doubleValidator);
	m_emitterAttribute->m_maxParticlesLineEdit->setText(QString::number(ParticleEmitterDescription::m_defaultEmitterMaxParticles));
	m_emitterAttribute->m_maxParticlesLineEdit->setValidator(intValidator);
	m_emitterAttribute->m_emitterOneShotMinLineEdit->setText(QString::number(ParticleEmitterDescription::m_defaultEmitterOneShotMinMax));
	m_emitterAttribute->m_emitterOneShotMinLineEdit->setValidator(intValidator);
	m_emitterAttribute->m_emitterOneShotMaxLineEdit->setText(QString::number(ParticleEmitterDescription::m_defaultEmitterOneShotMinMax));
	m_emitterAttribute->m_emitterOneShotMaxLineEdit->setValidator(intValidator);
	m_emitterAttribute->m_emitterMinLodDistanceLineEdit->setValidator(doubleValidator);
	m_emitterAttribute->m_emitterMaxLodDistanceLineEdit->setValidator(doubleValidator);

	connect(m_emitterAttribute->m_emitterOneShotMinLineEdit, SIGNAL(returnPressed()), this, SLOT(slotEmitterOneShotMinLineEditReturnPressed()));
	connect(m_emitterAttribute->m_emitterOneShotMaxLineEdit, SIGNAL(returnPressed()), this, SLOT(slotEmitterOneShotMaxLineEditReturnPressed()));
	connect(m_emitterAttribute->m_particleOrientationComboBox, SIGNAL(activated(int)), this, SLOT(slotValidateAndNotifyChanged(int)));
	connect(m_emitterAttribute->m_randomInitialRotationCheckBox, SIGNAL(clicked()), this, SLOT(randomInitialRotationCheckBoxClicked()));
	connect(m_emitterAttribute->m_particleReferenceFrameComboBox, SIGNAL(activated(int)), this, SLOT(slotValidateAndNotifyChanged(int)));
	connect(m_emitterAttribute->m_loopImmediatelyCheckBox, SIGNAL(clicked()), this, SLOT(loopImmediatelyCheckBoxClicked()));
	connect(m_emitterAttribute->m_oneShotCheckBox, SIGNAL(clicked()), this, SLOT(oneShotCheckBoxClicked()));
	connect(m_emitterAttribute->m_maxParticlesLineEdit, SIGNAL(returnPressed()), this, SLOT(maxParticlesLineEditReturnPressed()));
	connect(m_emitterAttribute->m_emitterLifeTimeMinLineEdit, SIGNAL(returnPressed()), this, SLOT(lifeTimeMinLineEditReturnPressed()));
	connect(m_emitterAttribute->m_emitterLifeTimeMaxLineEdit, SIGNAL(returnPressed()), this, SLOT(lifeTimeMaxLineEditReturnPressed()));
	connect(m_emitterAttribute->m_typeComboBox, SIGNAL(activated(int)), this, SLOT(slotValidateAndNotifyChanged(int)));
	connect(m_emitterAttribute->m_shapeComboBox, SIGNAL(activated(int)), this, SLOT(slotValidateAndNotifyChanged(int)));
	connect(m_emitterAttribute->m_emitterMinLodDistanceLineEdit, SIGNAL(returnPressed()), this, SLOT(lodMinLineEditReturnPressed()));
	connect(m_emitterAttribute->m_emitterMaxLodDistanceLineEdit, SIGNAL(returnPressed()), this, SLOT(lodMaxLineEditReturnPressed()));
	connect(m_emitterAttribute->m_emitterLodComboBox, SIGNAL(activated(int)), this, SLOT(slotValidateAndNotifyChanged(int)));

	// Emitter sound attributes

	m_emitterAttribute->m_soundPathLineEdit->setReadOnly(true);
	connect(m_emitterAttribute->m_soundLoadPushButton, SIGNAL(clicked()), this, SLOT(slotSoundLoadPushButtonClicked()));
	connect(m_emitterAttribute->m_soundRemovePushButton, SIGNAL(clicked()), this, SLOT(slotSoundRemovePushButtonClicked()));

	// Particle only attributes

	m_emitterAttribute->m_particleCollisionHeightAboveGroundLineEdit->setText("0.00");
	m_emitterAttribute->m_particleCollisionHeightAboveGroundLineEdit->setValidator(doubleValidator);
	m_emitterAttribute->m_particleForwardVelocityMaintainedMinLineEdit->setText(QString::number(static_cast<int>(ParticleEmitterDescription::m_defaultParticleVelocityMaintained * 100.0f)));
	m_emitterAttribute->m_particleForwardVelocityMaintainedMinLineEdit->setValidator(intValidator);
	m_emitterAttribute->m_particleForwardVelocityMaintainedMaxLineEdit->setText(QString::number(static_cast<int>(ParticleEmitterDescription::m_defaultParticleVelocityMaintained * 100.0f)));
	m_emitterAttribute->m_particleForwardVelocityMaintainedMaxLineEdit->setValidator(intValidator);
	m_emitterAttribute->m_particleUpVelocityMaintainedMinLineEdit->setText(QString::number(static_cast<int>(ParticleEmitterDescription::m_defaultParticleVelocityMaintained * 100.0f)));
	m_emitterAttribute->m_particleUpVelocityMaintainedMinLineEdit->setValidator(intValidator);
	m_emitterAttribute->m_particleUpVelocityMaintainedMaxLineEdit->setText(QString::number(static_cast<int>(ParticleEmitterDescription::m_defaultParticleVelocityMaintained * 100.0f)));
	m_emitterAttribute->m_particleUpVelocityMaintainedMaxLineEdit->setValidator(intValidator);
	m_emitterAttribute->m_windResistanceGlobalLineEdit->setText("0.00");
	m_emitterAttribute->m_windResistanceGlobalLineEdit->setValidator(doubleValidator);
	m_emitterAttribute->m_flockingSeperationDistanceLineEdit->setValidator(doubleValidator);
	m_emitterAttribute->m_flockingSeperationGainLineEdit->setValidator(doubleValidator);
	m_emitterAttribute->m_flockingAlignmentGainLineEdit->setValidator(doubleValidator);
	m_emitterAttribute->m_flockingCohesionGainLineEdit->setValidator(doubleValidator);
	m_emitterAttribute->m_flockingCageWidthLineEdit->setValidator(doubleValidator);
	m_emitterAttribute->m_flockingCageHeightLineEdit->setValidator(doubleValidator);
	m_emitterAttribute->m_particleTimeOfDayColorLineEdit->setValidator(doubleValidator);
	m_emitterAttribute->m_particleSnapToGroundOnCreationHeightLineEdit->setValidator(doubleValidator);
	m_emitterAttribute->m_particleChangeDirectionDegreeLineEdit->setValidator(doubleValidator);
	m_emitterAttribute->m_particleChangeDirectionTimeLineEdit->setValidator(doubleValidator);

	connect(m_emitterAttribute->m_groundCollisionCheckBox, SIGNAL(clicked()), this, SLOT(slotGroundCollisionCheckBoxClicked()));
	connect(m_emitterAttribute->m_killParticlesOnCollisionCheckBox, SIGNAL(clicked()), this, SLOT(slotKillParticlesOnCollisionCheckBoxClicked()));
	connect(m_emitterAttribute->m_particleCollisionHeightAboveGroundLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_particleForwardVelocityMaintainedMinLineEdit, SIGNAL(returnPressed()), this, SLOT(slotParticleFrictionMinLineEditReturnPressed()));
	connect(m_emitterAttribute->m_particleForwardVelocityMaintainedMaxLineEdit, SIGNAL(returnPressed()), this, SLOT(slotParticleFrictionMaxLineEditReturnPressed()));
	connect(m_emitterAttribute->m_particleUpVelocityMaintainedMinLineEdit, SIGNAL(returnPressed()), this, SLOT(slotParticleResilienceMinLineEditReturnPressed()));
	connect(m_emitterAttribute->m_particleUpVelocityMaintainedMaxLineEdit, SIGNAL(returnPressed()), this, SLOT(slotParticleResilienceMaxLineEditReturnPressed()));
	connect(m_emitterAttribute->m_affectedByWindCheckBox, SIGNAL(clicked()), this, SLOT(slotAffectedByWindCheckBoxClicked()));
	connect(m_emitterAttribute->m_windResistanceGlobalLineEdit, SIGNAL(returnPressed()), this, SLOT(slotWindResistanceGlobalLineEditReturnPressed()));
	connect(m_emitterAttribute->m_flockingTypeComboBox, SIGNAL(activated(int)), this, SLOT(slotValidateAndNotifyChanged(int)));
	connect(m_emitterAttribute->m_flockingSeperationDistanceLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_flockingSeperationGainLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_flockingAlignmentGainLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_flockingCohesionGainLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_flockingCageWidthLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_flockingCageHeightLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_flockingCageShapeComboBox, SIGNAL(activated(int)), this, SLOT(slotValidateAndNotifyChanged(int)));
	connect(m_emitterAttribute->m_flockingCageEdgeTypeComboBox, SIGNAL(activated(int)), this, SLOT(slotValidateAndNotifyChanged(int)));

	connect(m_emitterAttribute->m_particleTimeOfDayColorCheckBox, SIGNAL(clicked()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_particleTimeOfDayColorLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_particleSnapToGroundOnCreationCheckBox, SIGNAL(clicked()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_particleAlignToTerrainNormalOnCreationCheckBox, SIGNAL(clicked()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_particleSnapToGroundOnCreationHeightLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_particleChangeDirectionCheckBox, SIGNAL(clicked()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_particleChangeDirectionDegreeLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_particleChangeDirectionTimeLineEdit, SIGNAL(returnPressed()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_firstParticleImmediatelyCheckBox, SIGNAL(clicked()), this, SLOT(slotValidateAndNotifyChanged()));
	connect(m_emitterAttribute->m_usePriorityParticlesCheckBox, SIGNAL(clicked()), this, SLOT(slotValidateAndNotifyChanged()));

	// Waveforms

	WaveForm waveForm;

	// Translation X

	addWaveFormAttributeView(m_attributesVBox, &m_emitterTranslationX, "Translation X", m_boundEmitterTranslationX);
	ParticleEmitterDescription::setDefaultEmitterTranslationX(waveForm);
	m_emitterTranslationX->setDefaultWaveForm(waveForm);

	// Translation Y

	addWaveFormAttributeView(m_attributesVBox, &m_emitterTranslationY, "Translation Y", m_boundEmitterTranslationY);
	ParticleEmitterDescription::setDefaultEmitterTranslationY(waveForm);
	m_emitterTranslationY->setDefaultWaveForm(waveForm);
	
	// Translation Z

	addWaveFormAttributeView(m_attributesVBox, &m_emitterTranslationZ, "Translation Z", m_boundEmitterTranslationZ);
	WaveForm emitterTranslationZDefaultWaveForm;
	ParticleEmitterDescription::setDefaultEmitterTranslationZ(emitterTranslationZDefaultWaveForm);
	m_emitterTranslationZ->setDefaultWaveForm(emitterTranslationZDefaultWaveForm);

	// Rotation X

	addWaveFormAttributeView(m_attributesVBox, &m_emitterRotationX, "Rotation X", m_boundEmitterRotationX);
	WaveForm emitterRotationXDefaultWaveForm;
	ParticleEmitterDescription::setDefaultEmitterRotationX(emitterRotationXDefaultWaveForm);
	m_emitterRotationX->setDefaultWaveForm(emitterRotationXDefaultWaveForm);

	// Rotation Y

	addWaveFormAttributeView(m_attributesVBox, &m_emitterRotationY, "Rotation Y", m_boundEmitterRotationY);
	WaveForm emitterRotationYDefaultWaveForm;
	ParticleEmitterDescription::setDefaultEmitterRotationY(emitterRotationYDefaultWaveForm);
	m_emitterRotationY->setDefaultWaveForm(emitterRotationYDefaultWaveForm);

	// Rotation Z

	addWaveFormAttributeView(m_attributesVBox, &m_emitterRotationZ, "Rotation Z", m_boundEmitterRotationZ);
	WaveForm emitterRotationZDefaultWaveForm;
	ParticleEmitterDescription::setDefaultEmitterRotationZ(emitterRotationZDefaultWaveForm);
	m_emitterRotationZ->setDefaultWaveForm(emitterRotationZDefaultWaveForm);

	// Shape Size

	addWaveFormAttributeView(m_attributesVBox, &m_emitterShapeSize, "Shape Size", m_boundEmitterShapeSize);
	WaveForm emitterShapeSizeDefaultWaveForm;
	ParticleEmitterDescription::setDefaultEmitterShapeSize(emitterShapeSizeDefaultWaveForm);
	m_emitterShapeSize->setDefaultWaveForm(emitterShapeSizeDefaultWaveForm);

	// Emit Distance

	addWaveFormAttributeView(m_attributesVBox, &m_emitterDistance, "Emit Distance", m_boundEmitterDistance);
	WaveForm emitterEmitDistanceDefaultWaveForm;
	ParticleEmitterDescription::setDefaultEmitterDistance(emitterEmitDistanceDefaultWaveForm);
	m_emitterDistance->setDefaultWaveForm(emitterEmitDistanceDefaultWaveForm);

	// Emit Spread

	addWaveFormAttributeView(m_attributesVBox, &m_emitterSpread, "Emit Spread", m_boundEmitterSpread);
	WaveForm emitterSpreadDefaultWaveForm;
	ParticleEmitterDescription::setDefaultEmitterSpread(emitterSpreadDefaultWaveForm);
	m_emitterSpread->setDefaultWaveForm(emitterSpreadDefaultWaveForm);

	// Emit Speed

	addWaveFormAttributeView(m_attributesVBox, &m_particleEmitSpeed, "Emit Speed", m_boundParticleEmitSpeed);
	WaveForm particleEmitSpeedDefaultWaveForm;
	ParticleEmitterDescription::setDefaultParticleEmitSpeed(particleEmitSpeedDefaultWaveForm);
	m_particleEmitSpeed->setDefaultWaveForm(particleEmitSpeedDefaultWaveForm);

	// Generation Rate

	addWaveFormAttributeView(m_attributesVBox, &m_particleGenerationRate, "Generation Rate", m_boundParticleGenerationRate);
	WaveForm particleGenerationRateDefaultWaveForm;
	ParticleEmitterDescription::setDefaultParticleGenerationRate(particleGenerationRateDefaultWaveForm);
	m_particleGenerationRate->setDefaultWaveForm(particleGenerationRateDefaultWaveForm);
	m_particleGenerationRate->setModifiable(true, "Distance");

	connect(m_particleGenerationRate->m_modifierCheckBox, SIGNAL(clicked()), this, SLOT(slotParticleGenerationRateModifiableCheckBoxClicked()));

	// Inherit Velocity Percent

	addWaveFormAttributeView(m_attributesVBox, &m_particleInheritVelocityPercent, "Inherit Velocity %", m_boundParticleInheritVelocityPercentage);
	WaveForm particleInheritVelocityPercentDefaultWaveForm;
	ParticleEmitterDescription::setDefaultParticleInheritVelocityPercent(particleInheritVelocityPercentDefaultWaveForm);
	m_particleInheritVelocityPercent->setDefaultWaveForm(particleInheritVelocityPercentDefaultWaveForm);

	// Cluster Count

	addWaveFormAttributeView(m_attributesVBox, &m_particleClusterCount, "Cluster Count", m_boundParticleClusterCount);
	WaveForm particleClusterCountDefaultWaveForm;
	ParticleEmitterDescription::setDefaultParticleClusterCount(particleClusterCountDefaultWaveForm);
	m_particleClusterCount->setDefaultWaveForm(particleClusterCountDefaultWaveForm);

	// Cluster Radius

	addWaveFormAttributeView(m_attributesVBox, &m_particleClusterRadius, "Cluster Radius", m_boundParticleClusterRadius);
	WaveForm particleClusterRadiusDefaultWaveForm;
	ParticleEmitterDescription::setDefaultParticleClusterRadius(particleClusterRadiusDefaultWaveForm);
	m_particleClusterRadius->setDefaultWaveForm(particleClusterRadiusDefaultWaveForm);

	// Particle Lifetime

	addWaveFormAttributeView(m_attributesVBox, &m_particleLifeTime, "Particle Lifetime", m_boundParticleLifetime);
	WaveForm particleLifeTimeDefaultWaveForm;
	ParticleEmitterDescription::setDefaultParticleLifeTime(particleLifeTimeDefaultWaveForm);
	m_particleLifeTime->setDefaultWaveForm(particleLifeTimeDefaultWaveForm);

	// Particle Weight

	addWaveFormAttributeView(m_attributesVBox, &m_particleWeight, "Particle Weight", m_boundParticleWeight);
	WaveForm particleWeightDefaultWaveForm;
	ParticleEmitterDescription::setDefaultParticleWeight(particleWeightDefaultWaveForm);
	m_particleWeight->setDefaultWaveForm(particleWeightDefaultWaveForm);

	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);

	m_particleInheritVelocityPercent->setEnabled(false);
	m_particleClusterCount->setEnabled(false);
	m_particleClusterRadius->setEnabled(false);
	m_particleInheritVelocityPercent->setEnabled(false);

	validate();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotParticleGenerationRateModifiableCheckBoxClicked()
{
	if (m_particleGenerationRate->m_modifierCheckBox->isChecked())
	{
		selectAttribute(m_particleGenerationRate);
	}

	emit attributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::setNewCaption(char const *caption)
{
	char text[256];
	sprintf(text, "Emitter (%s)", caption);
	(dynamic_cast<QWidget *>(parent()))->setCaption(text);
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::onAttributeNameChanged(const QString &name)
{
	setNewCaption(name);
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::setParticleEmitterDescription(ParticleEmitterDescription const &particleEmitterDescription)
{
	// These number match to the number in ParticleEmitterDescription for tracking that all the parameters are set correctly

	// 1
#ifdef _DEBUG
	m_nameWidget->m_nameLineEdit->setText(particleEmitterDescription.m_emitterName.c_str());
#endif // _DEBUG
	
	// 2
	m_timingAttribute->setTiming(particleEmitterDescription.m_timing);

	// 3
	m_emitterAttribute->m_typeComboBox->setCurrentItem(static_cast<int>(particleEmitterDescription.m_emitterEmitDirection));

	// 4
	m_particleGenerationRate->m_modifierCheckBox->setChecked(particleEmitterDescription.m_generationType == ParticleEmitterDescription::G_distance);

	// 5
	m_emitterAttribute->m_shapeComboBox->setCurrentItem(static_cast<int>(particleEmitterDescription.m_emitterShape->getEnum()));

	// 6
	m_emitterAttribute->m_loopImmediatelyCheckBox->setChecked(particleEmitterDescription.m_emitterLoopImmediately);

	// 7
	ParticleEditorUtility::setLineEditInt(m_emitterAttribute->m_maxParticlesLineEdit, particleEmitterDescription.m_emitterMaxParticles, m_boundEmitterMaxParticles);

	// 8
	m_emitterAttribute->m_oneShotCheckBox->setChecked(particleEmitterDescription.m_emitterOneShot);

	// 9
	ParticleEditorUtility::setLineEditInt(m_emitterAttribute->m_emitterOneShotMinLineEdit, particleEmitterDescription.m_emitterOneShotMin, ParticleBoundInt(ParticleEditorUtility::getInt(m_emitterAttribute->m_maxParticlesLineEdit), 1));

	// 10
	ParticleEditorUtility::setLineEditInt(m_emitterAttribute->m_emitterOneShotMaxLineEdit, particleEmitterDescription.m_emitterOneShotMax, ParticleBoundInt(ParticleEditorUtility::getInt(m_emitterAttribute->m_maxParticlesLineEdit), 1));

	// 11
	m_emitterTranslationX->setWaveForm(particleEmitterDescription.m_emitterTranslationX, false);

	// 12
	m_emitterTranslationY->setWaveForm(particleEmitterDescription.m_emitterTranslationY, false);

	// 13
	m_emitterTranslationZ->setWaveForm(particleEmitterDescription.m_emitterTranslationZ, false);

	// 14
	m_emitterRotationX->setWaveForm(particleEmitterDescription.m_emitterRotationX, false);
	
	// 15
	m_emitterRotationY->setWaveForm(particleEmitterDescription.m_emitterRotationY, false);
	
	// 16
	m_emitterRotationZ->setWaveForm(particleEmitterDescription.m_emitterRotationZ, false);

	// 17
	m_emitterDistance->setWaveForm(particleEmitterDescription.m_emitterDistance, false);
	
	// 18
	m_emitterShapeSize->setWaveForm(particleEmitterDescription.m_emitterShapeSize, false);

	// 19
	m_emitterSpread->setWaveForm(particleEmitterDescription.m_emitterSpread, false);
	
	// 20
	m_emitterAttribute->m_randomInitialRotationCheckBox->setChecked(particleEmitterDescription.m_particleRandomInitialRotation);
	
	// 21
	m_emitterAttribute->m_particleOrientationComboBox->setCurrentItem(static_cast<int>(particleEmitterDescription.m_particleOrientation));

	// 23
	m_particleGenerationRate->setWaveForm(particleEmitterDescription.m_particleGenerationRate, false);

	// 24
	m_particleEmitSpeed->setWaveForm(particleEmitterDescription.m_particleEmitSpeed, false);

	// 25
	m_particleInheritVelocityPercent->setWaveForm(particleEmitterDescription.m_particleInheritVelocityPercent, false);

	// 26
	m_particleClusterCount->setWaveForm(particleEmitterDescription.m_particleClusterCount, false);

	// 27
	m_particleClusterRadius->setWaveForm(particleEmitterDescription.m_particleClusterRadius, false);

	// 28
	m_particleLifeTime->setWaveForm(particleEmitterDescription.m_particleLifeTime, false);

	// 29
	m_particleWeight->setWaveForm(particleEmitterDescription.m_particleWeight, false);

	// 30
	m_emitterAttribute->m_groundCollisionCheckBox->setChecked(particleEmitterDescription.m_particleGroundCollision);

	// 31
	m_emitterAttribute->m_killParticlesOnCollisionCheckBox->setChecked(particleEmitterDescription.m_particleKillParticlesOnCollision);

	// 32
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_particleCollisionHeightAboveGroundLineEdit, particleEmitterDescription.m_particleCollisionHeightAboveGround, m_boundParticleHeightAboveGround, 2);

	// 34
	ParticleEditorUtility::setLineEditInt(m_emitterAttribute->m_particleForwardVelocityMaintainedMinLineEdit, particleEmitterDescription.m_particleForwardVelocityMaintainedMin * 100.0f, m_boundParticleCollisionVelocityMaintained);

	// 35
	ParticleEditorUtility::setLineEditInt(m_emitterAttribute->m_particleForwardVelocityMaintainedMaxLineEdit, particleEmitterDescription.m_particleForwardVelocityMaintainedMax * 100.0f, m_boundParticleCollisionVelocityMaintained);

	// 36
	ParticleEditorUtility::setLineEditInt(m_emitterAttribute->m_particleUpVelocityMaintainedMinLineEdit, particleEmitterDescription.m_particleUpVelocityMaintainedMin * 100.0f, m_boundParticleCollisionVelocityMaintained);

	// 37
	ParticleEditorUtility::setLineEditInt(m_emitterAttribute->m_particleUpVelocityMaintainedMaxLineEdit, particleEmitterDescription.m_particleUpVelocityMaintainedMax * 100.0f, m_boundParticleCollisionVelocityMaintained);

	// 38
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_windResistanceGlobalLineEdit, particleEmitterDescription.m_windResistenceGlobalPercent, m_boundWindResistanceGlobal, 2);
	m_emitterAttribute->m_affectedByWindCheckBox->setChecked(particleEmitterDescription.m_windResistenceGlobalPercent < 1.0f);

	// 39
	m_emitterAttribute->m_particleReferenceFrameComboBox->setCurrentItem(particleEmitterDescription.m_localSpaceParticles);

	// 40	

	m_emitterAttribute->m_flockingTypeComboBox->setCurrentItem(static_cast<int>(particleEmitterDescription.m_flockingType));

	m_emitterAttribute->m_flockingCageShapeComboBox->setCurrentItem(static_cast<int>(particleEmitterDescription.m_flockingCageShape));
	m_emitterAttribute->m_flockingCageEdgeTypeComboBox->setCurrentItem(static_cast<int>(particleEmitterDescription.m_flockingCageEdgeType));

	// 41		
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_flockingSeperationDistanceLineEdit, particleEmitterDescription.m_flockingSeperationDistance, m_boundFlockingSeperationDistance, 2);

	// 42
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_flockingSeperationGainLineEdit, particleEmitterDescription.m_flockingSeperationGain, m_boundFlockingSeperationGain, 2);

	// 43
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_flockingAlignmentGainLineEdit, particleEmitterDescription.m_flockingAlignmentGain, m_boundFlockingAlignmentGain, 2);
	
	// 44
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_flockingCohesionGainLineEdit, particleEmitterDescription.m_flockingCohesionGain, m_boundFlockingCohesionGain, 2);
	
	// 45
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_flockingCageWidthLineEdit, particleEmitterDescription.m_flockingCageWidth, m_boundFlockingCageWidth, 2);
	
	// 46
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_flockingCageHeightLineEdit, particleEmitterDescription.m_flockingCageHeight, m_boundFlockingCageHeight, 2);
	
	// 48
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_emitterLifeTimeMinLineEdit, particleEmitterDescription.getEmitterLifeTimeMin(), m_boundEmitterLifeTime, 2);
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_emitterLifeTimeMaxLineEdit, particleEmitterDescription.getEmitterLifeTimeMax(), m_boundEmitterLifeTime, 2);

	// 49
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_emitterMinLodDistanceLineEdit, particleEmitterDescription.getEmitterLodDistanceMin(), m_boundEmitterMinLod, 2);
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_emitterMaxLodDistanceLineEdit, particleEmitterDescription.getEmitterLodDistanceMax(), m_boundEmitterMaxLod, 2);

	if (particleEmitterDescription.isEmitterUsingGlobalLod())
	{
		m_emitterAttribute->m_emitterLodComboBox->setCurrentItem(static_cast<int>(LOD_global));
	}
	else if (particleEmitterDescription.isEmitterUsingNoLod())
	{
		m_emitterAttribute->m_emitterLodComboBox->setCurrentItem(static_cast<int>(LOD_none));
	}
	else
	{
		m_emitterAttribute->m_emitterLodComboBox->setCurrentItem(static_cast<int>(LOD_specified));
	}

	// 50
	m_emitterAttribute->m_soundPathLineEdit->setText(particleEmitterDescription.getSoundPath().c_str());

	// 51
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_particleTimeOfDayColorLineEdit, particleEmitterDescription.m_particleTimeOfDayColorPercent, m_boundParticleTimeOfDayColorPercent, 2);
	m_emitterAttribute->m_particleTimeOfDayColorCheckBox->setChecked(particleEmitterDescription.m_particleTimeOfDayColorPercent > 0.0f);

	// 39
	m_emitterAttribute->m_firstParticleImmediatelyCheckBox->setChecked(particleEmitterDescription.m_firstParticleImmediately);
	m_emitterAttribute->m_usePriorityParticlesCheckBox->setChecked(particleEmitterDescription.m_usePriorityParticles);
	
	// Set the default selected attribute if nothing is selected

	if (getSelectedAttribute() == NULL)
	{
		selectAttribute(m_emitterTranslationX, false);
	}
	else
	{
		selectAttribute(getSelectedAttribute(), false);
	}

	m_emitterAttribute->m_particleSnapToGroundOnCreationCheckBox->setChecked(particleEmitterDescription.m_particleSnapToTerrainOnCreation);
	m_emitterAttribute->m_particleAlignToTerrainNormalOnCreationCheckBox->setChecked(particleEmitterDescription.m_particleAlignToTerrainNormalOnCreation);
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_particleSnapToGroundOnCreationHeightLineEdit, particleEmitterDescription.m_particleSnapToTerrainOnCreationHeight, m_boundParticleSnapToTerrainOnCreationHeight, 2);
	m_emitterAttribute->m_particleChangeDirectionCheckBox->setChecked(particleEmitterDescription.m_particleChangeDirectionRadian > 0.0f);

	float radian2Degree = 180.0f / PI;
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_particleChangeDirectionDegreeLineEdit, radian2Degree * particleEmitterDescription.m_particleChangeDirectionRadian, m_boundParticleChangeDirectionDegree, 2);
	ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_particleChangeDirectionTimeLineEdit, particleEmitterDescription.m_particleChangeDirectionTime, m_boundParticleChangeDirectionTime, 2);

	validate();
}

//-----------------------------------------------------------------------------
ParticleEmitterDescription PEAttributeViewParticleEmitter::getParticleEmitterDescription() const
{
	// These number match to the number in ParticleEmitterDescription for tracking that all the parameters are set correctly

	ParticleEmitterDescription particleEmitterDescription;

	// 1
#ifdef _DEBUG
	particleEmitterDescription.m_emitterName = m_nameWidget->m_nameLineEdit->text();
#endif // _DEBUG
	
	// 2
	particleEmitterDescription.m_timing = m_timingAttribute->getTiming();
	
	// 3
	particleEmitterDescription.m_emitterEmitDirection = static_cast<ParticleEmitterDescription::EmitDirection>(m_emitterAttribute->m_typeComboBox->currentItem());

	// 4
	particleEmitterDescription.m_generationType = m_particleGenerationRate->m_modifierCheckBox->isChecked() ? ParticleEmitterDescription::G_distance : ParticleEmitterDescription::G_rate;

	// 5
	particleEmitterDescription.m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(m_emitterAttribute->m_shapeComboBox->currentItem()));

	// 6
	particleEmitterDescription.m_emitterLoopImmediately = m_emitterAttribute->m_loopImmediatelyCheckBox->isChecked();

	// 7
	particleEmitterDescription.m_emitterMaxParticles = ParticleEditorUtility::getFloat(m_emitterAttribute->m_maxParticlesLineEdit);

	// 8
	particleEmitterDescription.m_emitterOneShot = m_emitterAttribute->m_oneShotCheckBox->isChecked();
	
	// 9
	particleEmitterDescription.m_emitterOneShotMin = ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterOneShotMinLineEdit);
	
	// 10
	particleEmitterDescription.m_emitterOneShotMax = ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterOneShotMaxLineEdit);

	// 11
	particleEmitterDescription.m_emitterTranslationX = m_emitterTranslationX->getWaveForm();
	
	// 12
	particleEmitterDescription.m_emitterTranslationY = m_emitterTranslationY->getWaveForm();
	
	// 13
	particleEmitterDescription.m_emitterTranslationZ = m_emitterTranslationZ->getWaveForm();
	
	// 14
	particleEmitterDescription.m_emitterRotationX = m_emitterRotationX->getWaveForm();
	
	// 15
	particleEmitterDescription.m_emitterRotationY = m_emitterRotationY->getWaveForm();
	
	// 16
	particleEmitterDescription.m_emitterRotationZ = m_emitterRotationZ->getWaveForm();
	
	// 17
	particleEmitterDescription.m_emitterDistance = m_emitterDistance->getWaveForm();
	
	// 18
	particleEmitterDescription.m_emitterShapeSize = m_emitterShapeSize->getWaveForm();
	
	// 19
	particleEmitterDescription.m_emitterSpread = m_emitterSpread->getWaveForm();
	
	// 20
	particleEmitterDescription.m_particleRandomInitialRotation = m_emitterAttribute->m_randomInitialRotationCheckBox->isChecked();

	// 21
	particleEmitterDescription.m_particleOrientation = static_cast<ParticleEmitterDescription::ParticleOrientation>(m_emitterAttribute->m_particleOrientationComboBox->currentItem());

	// 23
	particleEmitterDescription.m_particleGenerationRate = m_particleGenerationRate->getWaveForm();

	// 24
	particleEmitterDescription.m_particleEmitSpeed = m_particleEmitSpeed->getWaveForm();

	// 25
	particleEmitterDescription.m_particleInheritVelocityPercent = m_particleInheritVelocityPercent->getWaveForm();
	
	// 26
	particleEmitterDescription.m_particleClusterCount = m_particleClusterCount->getWaveForm();
	
	// 27
	particleEmitterDescription.m_particleClusterRadius = m_particleClusterRadius->getWaveForm();
	
	// 28
	particleEmitterDescription.m_particleLifeTime = m_particleLifeTime->getWaveForm();
	
	// 29
	particleEmitterDescription.m_particleWeight = m_particleWeight->getWaveForm();
	
	// 30
	particleEmitterDescription.m_particleGroundCollision = m_emitterAttribute->m_groundCollisionCheckBox->isChecked();

	// 31
	particleEmitterDescription.m_particleKillParticlesOnCollision = m_emitterAttribute->m_killParticlesOnCollisionCheckBox->isChecked();
	
	// 32
	particleEmitterDescription.m_particleCollisionHeightAboveGround = ParticleEditorUtility::getFloat(m_emitterAttribute->m_particleCollisionHeightAboveGroundLineEdit);
	
	// 34
	particleEmitterDescription.m_particleForwardVelocityMaintainedMin = static_cast<float>(ParticleEditorUtility::getInt(m_emitterAttribute->m_particleForwardVelocityMaintainedMinLineEdit)) / 100.0f;

	// 35
	particleEmitterDescription.m_particleForwardVelocityMaintainedMax = static_cast<float>(ParticleEditorUtility::getInt(m_emitterAttribute->m_particleForwardVelocityMaintainedMaxLineEdit)) / 100.0f;

	// 36
	particleEmitterDescription.m_particleUpVelocityMaintainedMin = static_cast<float>(ParticleEditorUtility::getInt(m_emitterAttribute->m_particleUpVelocityMaintainedMinLineEdit)) / 100.0f;
	
	// 37
	particleEmitterDescription.m_particleUpVelocityMaintainedMax = static_cast<float>(ParticleEditorUtility::getInt(m_emitterAttribute->m_particleUpVelocityMaintainedMaxLineEdit)) / 100.0f;

	// 38
	if (m_emitterAttribute->m_affectedByWindCheckBox->isChecked())
	{
		particleEmitterDescription.m_windResistenceGlobalPercent = ParticleEditorUtility::getFloat(m_emitterAttribute->m_windResistanceGlobalLineEdit);
	}
	else
	{
		particleEmitterDescription.m_windResistenceGlobalPercent = 1.0f;
	}

	// 39
	switch (m_emitterAttribute->m_particleReferenceFrameComboBox->currentItem())
	{
		case 0:  { particleEmitterDescription.m_localSpaceParticles = false; } break;
		case 1:  { particleEmitterDescription.m_localSpaceParticles = true; } break;
		default: { DEBUG_FATAL(true, ("Unexpected particle reference frame.")); } break;
	}

	// 40
	particleEmitterDescription.m_flockingType = static_cast<ParticleEmitterDescription::FlockingType>(m_emitterAttribute->m_flockingTypeComboBox->currentItem());
	particleEmitterDescription.m_flockingCageShape = static_cast<ParticleEmitterDescription::FlockCageShape>(m_emitterAttribute->m_flockingCageShapeComboBox->currentItem());
	particleEmitterDescription.m_flockingCageEdgeType = static_cast<ParticleEmitterDescription::FlockCageEdgeType>(m_emitterAttribute->m_flockingCageEdgeTypeComboBox->currentItem());

	// 41
	particleEmitterDescription.m_flockingSeperationDistance = ParticleEditorUtility::getFloat(m_emitterAttribute->m_flockingSeperationDistanceLineEdit);
	
	// 42
	particleEmitterDescription.m_flockingSeperationGain = ParticleEditorUtility::getFloat(m_emitterAttribute->m_flockingSeperationGainLineEdit);
	
	// 43
	particleEmitterDescription.m_flockingAlignmentGain = ParticleEditorUtility::getFloat(m_emitterAttribute->m_flockingAlignmentGainLineEdit);
	
	// 44
	particleEmitterDescription.m_flockingCohesionGain = ParticleEditorUtility::getFloat(m_emitterAttribute->m_flockingCohesionGainLineEdit);
	
	// 45
	particleEmitterDescription.m_flockingCageWidth = ParticleEditorUtility::getFloat(m_emitterAttribute->m_flockingCageWidthLineEdit);
	
	// 46
	particleEmitterDescription.m_flockingCageHeight = ParticleEditorUtility::getFloat(m_emitterAttribute->m_flockingCageHeightLineEdit);
	
	// 48
	particleEmitterDescription.setEmitterLifeTime(ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterLifeTimeMinLineEdit), ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterLifeTimeMaxLineEdit));
	
	// 49
	if (m_emitterAttribute->m_emitterLodComboBox->currentItem() == static_cast<int>(LOD_global))
	{
		particleEmitterDescription.setEmitterLodDistanceGlobal();
	}
	else if (m_emitterAttribute->m_emitterLodComboBox->currentItem() == static_cast<int>(LOD_none))
	{
		particleEmitterDescription.setEmitterLodDistanceNone();
	}
	else
	{
		particleEmitterDescription.setEmitterLodDistance(ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterMinLodDistanceLineEdit), ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterMaxLodDistanceLineEdit));
	}
	
	// 50
	particleEmitterDescription.setSoundPath(m_emitterAttribute->m_soundPathLineEdit->text().latin1());

	// 51
	
	if (m_emitterAttribute->m_particleTimeOfDayColorCheckBox->isChecked())
	{
		particleEmitterDescription.m_particleTimeOfDayColorPercent = ParticleEditorUtility::getFloat(m_emitterAttribute->m_particleTimeOfDayColorLineEdit);
	}
	else
	{
		particleEmitterDescription.m_particleTimeOfDayColorPercent = 0.0f;
	}

	particleEmitterDescription.m_firstParticleImmediately = m_emitterAttribute->m_firstParticleImmediatelyCheckBox->isChecked();
	particleEmitterDescription.m_usePriorityParticles = m_emitterAttribute->m_usePriorityParticlesCheckBox->isChecked();

	particleEmitterDescription.m_particleSnapToTerrainOnCreation = m_emitterAttribute->m_particleSnapToGroundOnCreationCheckBox->isChecked();

	if (m_emitterAttribute->m_particleSnapToGroundOnCreationCheckBox->isChecked())
	{
		particleEmitterDescription.m_particleAlignToTerrainNormalOnCreation = m_emitterAttribute->m_particleAlignToTerrainNormalOnCreationCheckBox->isChecked();
		particleEmitterDescription.m_particleSnapToTerrainOnCreationHeight = ParticleEditorUtility::getFloat(m_emitterAttribute->m_particleSnapToGroundOnCreationHeightLineEdit);
	}
	else
	{
		particleEmitterDescription.m_particleAlignToTerrainNormalOnCreation = false;
		particleEmitterDescription.m_particleSnapToTerrainOnCreationHeight = 0.0f;
	}

	if (m_emitterAttribute->m_particleChangeDirectionCheckBox->isChecked())
	{
		float degree2Radian = PI / 180.0f;
		particleEmitterDescription.m_particleChangeDirectionRadian = degree2Radian * ParticleEditorUtility::getFloat(m_emitterAttribute->m_particleChangeDirectionDegreeLineEdit);
		particleEmitterDescription.m_particleChangeDirectionTime = ParticleEditorUtility::getFloat(m_emitterAttribute->m_particleChangeDirectionTimeLineEdit);
	}
	else
	{
		particleEmitterDescription.m_particleChangeDirectionRadian = 0.0f;
		particleEmitterDescription.m_particleChangeDirectionTime = 0.0f;
	}

	return particleEmitterDescription;
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::randomInitialRotationCheckBoxClicked()
{
	validate();
	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::loopImmediatelyCheckBoxClicked()
{
	validate();
	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::oneShotCheckBoxClicked()
{
	validate();
	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::maxParticlesLineEditReturnPressed()
{
	validate();
	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::lifeTimeMinLineEditReturnPressed()
{
	float const min = ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterLifeTimeMinLineEdit);
	float const max = ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterLifeTimeMaxLineEdit);

	if (min > max)
	{
		ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_emitterLifeTimeMaxLineEdit, min, m_boundEmitterLifeTime, 2);
	}

	validate();
	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::lifeTimeMaxLineEditReturnPressed()
{
	float const min = ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterLifeTimeMinLineEdit);
	float const max = ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterLifeTimeMaxLineEdit);

	if (max < min)
	{
		ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_emitterLifeTimeMinLineEdit, max, m_boundEmitterLifeTime, 2);
	}

	validate();
	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::lodMinLineEditReturnPressed()
{
	float const min = ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterMinLodDistanceLineEdit);
	float const max = ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterMaxLodDistanceLineEdit);

	if (min > max)
	{
		ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_emitterMaxLodDistanceLineEdit, min, m_boundEmitterMaxLod, 2);
	}

	validate();
	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::lodMaxLineEditReturnPressed()
{
	float const min = ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterMinLodDistanceLineEdit);
	float const max = ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterMaxLodDistanceLineEdit);

	if (max < min)
	{
		ParticleEditorUtility::setLineEditFloat(m_emitterAttribute->m_emitterMinLodDistanceLineEdit, max, m_boundEmitterMinLod, 2);
	}

	validate();
	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotSoundLoadPushButtonClicked()
{
	// Get the last valid path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, ParticleEditorUtility::getSearchPath());
	QString result(settings.readEntry("SoundFileName", "c://"));

	QString soundPath(QFileDialog::getOpenFileName(result, "Sound Template files (*.snd)", this, "SoundTemplateFileDialog", "Open Sound Template"));

	if (!soundPath.isNull())
	{
		// Save the path

		settings.writeEntry("SoundFileName", static_cast<char const *>(soundPath));

		// Get the tree file relative path

		std::string treeFilePath;

		if (!TreeFile::stripTreeFileSearchPathFromFile(soundPath.latin1(), treeFilePath))
		{
			treeFilePath = soundPath;
		} 

		// Set the sound path and filename display

		FileNameUtils::swapChar(treeFilePath, '/', '\\');

		m_emitterAttribute->m_soundPathLineEdit->setText(treeFilePath.c_str());

		// Signal the sound changed

		validate();
		onAttributeChangedForceRebuild();
	}
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotSoundRemovePushButtonClicked()
{
	m_emitterAttribute->m_soundPathLineEdit->setText("");

	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotEmitterOneShotMinLineEditReturnPressed()
{
	validate();

	if (ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterOneShotMinLineEdit) > ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterOneShotMaxLineEdit))
	{
		ParticleEditorUtility::setLineEditInt(m_emitterAttribute->m_emitterOneShotMaxLineEdit, ParticleEditorUtility::getInt(m_emitterAttribute->m_emitterOneShotMinLineEdit), ParticleBoundInt(ParticleEditorUtility::getInt(m_emitterAttribute->m_maxParticlesLineEdit), 1));
	}

	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotEmitterOneShotMaxLineEditReturnPressed()
{
	validate();

	if (ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterOneShotMaxLineEdit) < ParticleEditorUtility::getFloat(m_emitterAttribute->m_emitterOneShotMinLineEdit))
	{
		ParticleEditorUtility::setLineEditInt(m_emitterAttribute->m_emitterOneShotMinLineEdit, ParticleEditorUtility::getInt(m_emitterAttribute->m_emitterOneShotMaxLineEdit), ParticleBoundInt(ParticleEditorUtility::getInt(m_emitterAttribute->m_maxParticlesLineEdit), 1));
	}

	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotGroundCollisionCheckBoxClicked()
{
	validate();

	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotKillParticlesOnCollisionCheckBoxClicked()
{
	validate();

	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotParticleFrictionMinLineEditReturnPressed()
{
	validate();

	if (ParticleEditorUtility::getInt(m_emitterAttribute->m_particleForwardVelocityMaintainedMinLineEdit) > ParticleEditorUtility::getInt(m_emitterAttribute->m_particleForwardVelocityMaintainedMaxLineEdit))
	{
		ParticleEditorUtility::setLineEditInt(m_emitterAttribute->m_particleForwardVelocityMaintainedMaxLineEdit, ParticleEditorUtility::getInt(m_emitterAttribute->m_particleForwardVelocityMaintainedMinLineEdit), m_boundParticleCollisionVelocityMaintained);
	}

	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotParticleFrictionMaxLineEditReturnPressed()
{
	validate();

	if (ParticleEditorUtility::getInt(m_emitterAttribute->m_particleForwardVelocityMaintainedMaxLineEdit) < ParticleEditorUtility::getInt(m_emitterAttribute->m_particleForwardVelocityMaintainedMinLineEdit))
	{
		ParticleEditorUtility::setLineEditInt(m_emitterAttribute->m_particleForwardVelocityMaintainedMinLineEdit, ParticleEditorUtility::getInt(m_emitterAttribute->m_particleForwardVelocityMaintainedMaxLineEdit), m_boundParticleCollisionVelocityMaintained);
	}

	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotParticleResilienceMinLineEditReturnPressed()
{
	validate();

	if (ParticleEditorUtility::getInt(m_emitterAttribute->m_particleUpVelocityMaintainedMinLineEdit) > ParticleEditorUtility::getInt(m_emitterAttribute->m_particleUpVelocityMaintainedMaxLineEdit))
	{
		ParticleEditorUtility::setLineEditInt(m_emitterAttribute->m_particleUpVelocityMaintainedMaxLineEdit, ParticleEditorUtility::getInt(m_emitterAttribute->m_particleUpVelocityMaintainedMinLineEdit), m_boundParticleCollisionVelocityMaintained);
	}

	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotParticleResilienceMaxLineEditReturnPressed()
{
	validate();

	if (ParticleEditorUtility::getInt(m_emitterAttribute->m_particleUpVelocityMaintainedMaxLineEdit) < ParticleEditorUtility::getInt(m_emitterAttribute->m_particleUpVelocityMaintainedMinLineEdit))
	{
		ParticleEditorUtility::setLineEditInt(m_emitterAttribute->m_particleUpVelocityMaintainedMinLineEdit, ParticleEditorUtility::getInt(m_emitterAttribute->m_particleUpVelocityMaintainedMaxLineEdit), m_boundParticleCollisionVelocityMaintained);
	}

	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotAffectedByWindCheckBoxClicked()
{
	validate();

	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotWindResistanceGlobalLineEditReturnPressed()
{
	validate();

	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotValidateAndNotifyChanged(int)
{
	slotValidateAndNotifyChanged();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::slotValidateAndNotifyChanged()
{
	validate();

	onAttributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleEmitter::validate()
{
	bool const worldSpaceReferenceFrame = (m_emitterAttribute->m_particleReferenceFrameComboBox->currentItem() == 0);

	// LOD

	if ((m_emitterAttribute->m_emitterLodComboBox->currentItem() == static_cast<int>(LOD_global)) ||
	    (m_emitterAttribute->m_emitterLodComboBox->currentItem() == static_cast<int>(LOD_none)))
	{
		m_emitterAttribute->m_emitterMinLodDistanceLineEdit->setText("");
		m_emitterAttribute->m_emitterMinLodDistanceLineEdit->setEnabled(false);
		
		m_emitterAttribute->m_emitterMaxLodDistanceLineEdit->setText("");
		m_emitterAttribute->m_emitterMaxLodDistanceLineEdit->setEnabled(false);
	}
	else if (m_emitterAttribute->m_emitterMinLodDistanceLineEdit->text().isEmpty() ||
	         m_emitterAttribute->m_emitterMaxLodDistanceLineEdit->text().isEmpty())
	{
		m_emitterAttribute->m_emitterMinLodDistanceLineEdit->setEnabled(true);
		QString string;
		string.sprintf("%.2f", ConfigClientParticle::getMinGlobalLodDistanceDefault());
		m_emitterAttribute->m_emitterMinLodDistanceLineEdit->setText(string);
		
		m_emitterAttribute->m_emitterMaxLodDistanceLineEdit->setEnabled(true);
		string.sprintf("%.2f", ConfigClientParticle::getMaxGlobalLodDistanceDefault());
		m_emitterAttribute->m_emitterMaxLodDistanceLineEdit->setText(string);

		ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_emitterMinLodDistanceLineEdit, m_boundEmitterMinLod, 2);
		ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_emitterMaxLodDistanceLineEdit, m_boundEmitterMaxLod, 2);
	}
	else
	{
		m_emitterAttribute->m_emitterMinLodDistanceLineEdit->setEnabled(true);
		m_emitterAttribute->m_emitterMaxLodDistanceLineEdit->setEnabled(true);

		ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_emitterMinLodDistanceLineEdit, m_boundEmitterMinLod, 2);
		ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_emitterMaxLodDistanceLineEdit, m_boundEmitterMaxLod, 2);
	}

	ParticleEditorUtility::validateLineEditInt(m_emitterAttribute->m_maxParticlesLineEdit, m_boundEmitterMaxParticles);
	ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_emitterLifeTimeMinLineEdit, m_boundEmitterLifeTime, 2);
	ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_emitterLifeTimeMaxLineEdit, m_boundEmitterLifeTime, 2);
	ParticleEditorUtility::validateLineEditInt(m_emitterAttribute->m_emitterOneShotMinLineEdit, ParticleBoundInt(ParticleEditorUtility::getInt(m_emitterAttribute->m_maxParticlesLineEdit), 1));
	ParticleEditorUtility::validateLineEditInt(m_emitterAttribute->m_emitterOneShotMaxLineEdit, ParticleBoundInt(ParticleEditorUtility::getInt(m_emitterAttribute->m_maxParticlesLineEdit), 1));
	ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_particleCollisionHeightAboveGroundLineEdit, m_boundParticleHeightAboveGround, 2);
	ParticleEditorUtility::validateLineEditInt(m_emitterAttribute->m_particleForwardVelocityMaintainedMinLineEdit, m_boundParticleCollisionVelocityMaintained);
	ParticleEditorUtility::validateLineEditInt(m_emitterAttribute->m_particleForwardVelocityMaintainedMaxLineEdit, m_boundParticleCollisionVelocityMaintained);
	ParticleEditorUtility::validateLineEditInt(m_emitterAttribute->m_particleUpVelocityMaintainedMinLineEdit, m_boundParticleCollisionVelocityMaintained);
	ParticleEditorUtility::validateLineEditInt(m_emitterAttribute->m_particleUpVelocityMaintainedMaxLineEdit, m_boundParticleCollisionVelocityMaintained);
	
	m_emitterAttribute->m_affectedByWindCheckBox->setEnabled(worldSpaceReferenceFrame);
	m_emitterAttribute->m_windResistanceGlobalLineEdit->setEnabled(worldSpaceReferenceFrame && m_emitterAttribute->m_affectedByWindCheckBox->isChecked());
	ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_windResistanceGlobalLineEdit, m_boundWindResistanceGlobal, 2);

	if (m_emitterAttribute->m_flockingTypeComboBox->currentItem() == 2)
	{
		m_emitterAttribute->m_cageHeightTextLabel->setText("Height above ground");
	}
	else
	{
		m_emitterAttribute->m_cageHeightTextLabel->setText("Cage Height");
	}

	bool const flockingEnabled = (m_emitterAttribute->m_flockingTypeComboBox->currentItem() != 0);
	m_emitterAttribute->m_flockingSeperationDistanceLineEdit->setEnabled(flockingEnabled);
	ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_flockingSeperationDistanceLineEdit, m_boundFlockingSeperationDistance, 2);
	m_emitterAttribute->m_flockingSeperationGainLineEdit->setEnabled(flockingEnabled);
	ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_flockingSeperationGainLineEdit, m_boundFlockingSeperationGain, 2);
	m_emitterAttribute->m_flockingAlignmentGainLineEdit->setEnabled(flockingEnabled);
	ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_flockingAlignmentGainLineEdit, m_boundFlockingAlignmentGain, 2);
	m_emitterAttribute->m_flockingCohesionGainLineEdit->setEnabled(flockingEnabled);
	ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_flockingCohesionGainLineEdit, m_boundFlockingCohesionGain, 2);
	m_emitterAttribute->m_flockingCageWidthLineEdit->setEnabled(flockingEnabled);
	ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_flockingCageWidthLineEdit, m_boundFlockingCageWidth, 2);
	m_emitterAttribute->m_flockingCageHeightLineEdit->setEnabled(flockingEnabled);
	ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_flockingCageHeightLineEdit, m_boundFlockingCageHeight, 2);
	m_emitterAttribute->m_particleTimeOfDayColorLineEdit->setEnabled(m_emitterAttribute->m_particleTimeOfDayColorCheckBox->isChecked());
	ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_particleTimeOfDayColorLineEdit, m_boundParticleTimeOfDayColorPercent, 2);
	m_emitterAttribute->m_flockingCageShapeComboBox->setEnabled(flockingEnabled);
	m_emitterAttribute->m_flockingCageEdgeTypeComboBox->setEnabled(flockingEnabled);

	m_emitterAttribute->m_particleSnapToGroundOnCreationCheckBox->setEnabled(worldSpaceReferenceFrame);
	ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_particleSnapToGroundOnCreationHeightLineEdit, m_boundParticleSnapToTerrainOnCreationHeight, 2);
	m_emitterAttribute->m_particleAlignToTerrainNormalOnCreationCheckBox->setEnabled(worldSpaceReferenceFrame && m_emitterAttribute->m_particleSnapToGroundOnCreationCheckBox->isChecked());
	m_emitterAttribute->m_particleSnapToGroundOnCreationHeightLineEdit->setEnabled(worldSpaceReferenceFrame && m_emitterAttribute->m_particleSnapToGroundOnCreationCheckBox->isChecked());

	ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_particleChangeDirectionDegreeLineEdit, m_boundParticleChangeDirectionDegree, 2);
	ParticleEditorUtility::validateLineEditFloat(m_emitterAttribute->m_particleChangeDirectionTimeLineEdit, m_boundParticleChangeDirectionTime, 2);
	m_emitterAttribute->m_particleChangeDirectionDegreeLineEdit->setEnabled(m_emitterAttribute->m_particleChangeDirectionCheckBox->isChecked());
	m_emitterAttribute->m_particleChangeDirectionTimeLineEdit->setEnabled(m_emitterAttribute->m_particleChangeDirectionCheckBox->isChecked());

	m_emitterAttribute->m_emitterOneShotMinLineEdit->setEnabled(m_emitterAttribute->m_oneShotCheckBox->isChecked());
	m_emitterAttribute->m_emitterOneShotMaxLineEdit->setEnabled(m_emitterAttribute->m_oneShotCheckBox->isChecked());

	// If one shot is selected, then loop immediately is not available

	if (m_emitterAttribute->m_oneShotCheckBox->isChecked())
	{
		m_emitterAttribute->m_loopImmediatelyCheckBox->setChecked(false);
		m_emitterAttribute->m_loopImmediatelyCheckBox->setEnabled(false);
	}
	else
	{
		m_emitterAttribute->m_loopImmediatelyCheckBox->setEnabled(true);
	}

	switch (m_emitterAttribute->m_particleReferenceFrameComboBox->currentItem())
	{
		case 0: // World Space
			{
				m_emitterAttribute->m_groundCollisionCheckBox->setEnabled(true);
			}
			break;
		case 1: // Object Space
			{
				m_emitterAttribute->m_groundCollisionCheckBox->setChecked(false);
				m_emitterAttribute->m_groundCollisionCheckBox->setEnabled(false);
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("Unexpected particle transform space."));
			}
			break;
	}

	m_emitterAttribute->m_killParticlesOnCollisionCheckBox->setEnabled(m_emitterAttribute->m_groundCollisionCheckBox->isChecked());
	m_emitterAttribute->m_particleCollisionHeightAboveGroundLineEdit->setEnabled(m_emitterAttribute->m_groundCollisionCheckBox->isChecked());
	m_emitterAttribute->m_particleForwardVelocityMaintainedMinLineEdit->setEnabled(m_emitterAttribute->m_groundCollisionCheckBox->isChecked());
	m_emitterAttribute->m_particleForwardVelocityMaintainedMaxLineEdit->setEnabled(m_emitterAttribute->m_groundCollisionCheckBox->isChecked());
	m_emitterAttribute->m_particleUpVelocityMaintainedMinLineEdit->setEnabled(m_emitterAttribute->m_groundCollisionCheckBox->isChecked());
	m_emitterAttribute->m_particleUpVelocityMaintainedMaxLineEdit->setEnabled(m_emitterAttribute->m_groundCollisionCheckBox->isChecked());
}

// ============================================================================
