// ============================================================================
//
// ParticleEmitterDescription.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleEmitterDescription.h"

#include "clientAudio/SoundTemplateList.h"
#include "clientParticle/ConfigClientParticle.h"
#include "clientParticle/ParticleEmitterShape.h"
#include "clientParticle/ParticleDescriptionMesh.h"
#include "clientParticle/ParticleDescriptionQuad.h"
#include "sharedFile/Iff.h"

// ============================================================================
//
// ParticleEmitterDescription
//
// ============================================================================

Tag const TAG_EMTR = TAG(E,M,T,R);

std::string const                                     ParticleEmitterDescription::m_defaultEmitterName("Default");
ParticleEmitterDescription::EmitDirection const       ParticleEmitterDescription::m_defaultEmitterEmitDirection(ParticleEmitterDescription::ED_omni);
bool const                                            ParticleEmitterDescription::m_defaultEmitterLoopImmediately(false);
float const                                           ParticleEmitterDescription::m_defaultEmitterLifeTime(4.0f);
float const                                           ParticleEmitterDescription::m_defaultEmitterMaxParticles(256.0f);
bool const                                            ParticleEmitterDescription::m_defaultEmitterOneShot(false);
int const                                             ParticleEmitterDescription::m_defaultEmitterOneShotMinMax(8);
ParticleEmitterDescription::ParticleOrientation const ParticleEmitterDescription::m_defaultParticleOrientation(PO_faceCamera);
bool const                                            ParticleEmitterDescription::m_defaultParticleRandomInitialRotation(false);
bool const                                            ParticleEmitterDescription::m_defaultParticleVisible(true);
float const                                           ParticleEmitterDescription::m_defaultParticleVelocityMaintained(1.0f);

//-----------------------------------------------------------------------------
ParticleEmitterDescription::ParticleEmitterDescription()
 : m_particleDescription(new ParticleDescriptionQuad())
#ifdef _DEBUG
 , m_emitterName(m_defaultEmitterName)
#endif // _DEBUG
 , m_timing()
 , m_emitterEmitDirection(m_defaultEmitterEmitDirection)
 , m_generationType(G_rate)
 , m_emitterShape(ParticleEmitterShape::createEmitterShape(ParticleEmitterShape::Shape_circle))
 , m_emitterLoopImmediately(m_defaultEmitterLoopImmediately)
 , m_emitterLifeTimeMin(m_defaultEmitterLifeTime)
 , m_emitterLifeTimeMax(m_defaultEmitterLifeTime)
 , m_emitterMaxParticles(m_defaultEmitterMaxParticles)
 , m_emitterOneShot(m_defaultEmitterOneShot)
 , m_emitterOneShotMin(m_defaultEmitterOneShotMinMax)
 , m_emitterOneShotMax(m_defaultEmitterOneShotMinMax)
 , m_emitterTranslationX()
 , m_emitterTranslationY()
 , m_emitterTranslationZ()
 , m_emitterRotationX()
 , m_emitterRotationY()
 , m_emitterRotationZ()
 , m_emitterDistance()
 , m_emitterShapeSize()
 , m_emitterSpread()
 , m_particleRandomInitialRotation(m_defaultParticleRandomInitialRotation)
 , m_particleOrientation(m_defaultParticleOrientation)
 , m_particleVisible(m_defaultParticleVisible)
 , m_particleGenerationRate()
 , m_particleEmitSpeed()
 , m_particleInheritVelocityPercent()
 , m_particleClusterCount()
 , m_particleClusterRadius()
 , m_particleLifeTime()
 , m_particleWeight()
 , m_particleGroundCollision(false)
 , m_particleKillParticlesOnCollision(false)
 , m_particleCollisionHeightAboveGround(0.0f)
 , m_particleForwardVelocityMaintainedMin(m_defaultParticleVelocityMaintained)
 , m_particleForwardVelocityMaintainedMax(m_defaultParticleVelocityMaintained)
 , m_particleUpVelocityMaintainedMin(m_defaultParticleVelocityMaintained)
 , m_particleUpVelocityMaintainedMax(m_defaultParticleVelocityMaintained)
 , m_windResistenceGlobalPercent(1.0f)
 , m_soundPath()
 , m_localSpaceParticles(false)
 , m_cachedSoundTemplate(NULL)
 , m_emitterLodDistanceMin(-1.0f)
 , m_emitterLodDistanceMax(-1.0f)
 , m_flockingType(FT_none)
 , m_flockingSeperationDistance(5.0f)
 , m_flockingAlignmentGain(0.3f)
 , m_flockingCohesionGain(0.5f)
 , m_flockingSeperationGain(0.4f)
 , m_flockingCageWidth(128.0f)
 , m_flockingCageHeight(10.0f)
 , m_flockingCageShape(FCS_rectangle)
 , m_flockingCageEdgeType(FCET_soft)
 , m_flockingHeadingChangeRadian(0.0f)
 , m_flockingHeadingChangeTime(0.0f)
 , m_particleTimeOfDayColorPercent(0.0f)
 , m_particleSnapToTerrainOnCreation(false)
 , m_particleAlignToTerrainNormalOnCreation(false)
 , m_particleSnapToTerrainOnCreationHeight(0.0f)
 , m_particleChangeDirectionRadian(0.0f)
 , m_particleChangeDirectionTime(0.0f)
 , m_firstParticleImmediately(false)
 , m_usePriorityParticles(false)
{
}

//-----------------------------------------------------------------------------
ParticleEmitterDescription::ParticleEmitterDescription(ParticleEmitterDescription const &rhs)
 : m_particleDescription(NULL)
#ifdef _DEBUG
 , m_emitterName(rhs.m_emitterName)
#endif // _DEBUG
 , m_timing(rhs.m_timing)
 , m_emitterEmitDirection(rhs.m_emitterEmitDirection)
 , m_generationType(rhs.m_generationType)
 , m_emitterShape(rhs.m_emitterShape->createEmitterShape())
 , m_emitterLoopImmediately(rhs.m_emitterLoopImmediately)
 , m_emitterLifeTimeMin(rhs.m_emitterLifeTimeMin)
 , m_emitterLifeTimeMax(rhs.m_emitterLifeTimeMax)
 , m_emitterMaxParticles(rhs.m_emitterMaxParticles)
 , m_emitterOneShot(rhs.m_emitterOneShot)
 , m_emitterOneShotMin(rhs.m_emitterOneShotMin)
 , m_emitterOneShotMax(rhs.m_emitterOneShotMax)
 , m_emitterTranslationX(rhs.m_emitterTranslationX)
 , m_emitterTranslationY(rhs.m_emitterTranslationY)
 , m_emitterTranslationZ(rhs.m_emitterTranslationZ)
 , m_emitterRotationX(rhs.m_emitterRotationX)
 , m_emitterRotationY(rhs.m_emitterRotationY)
 , m_emitterRotationZ(rhs.m_emitterRotationZ)
 , m_emitterDistance(rhs.m_emitterDistance)
 , m_emitterShapeSize(rhs.m_emitterShapeSize)
 , m_emitterSpread(rhs.m_emitterSpread)
 , m_particleRandomInitialRotation(rhs.m_particleRandomInitialRotation)
 , m_particleOrientation(rhs.m_particleOrientation)
 , m_particleVisible(rhs.m_particleVisible)
 , m_particleGenerationRate(rhs.m_particleGenerationRate)
 , m_particleEmitSpeed(rhs.m_particleEmitSpeed)
 , m_particleInheritVelocityPercent(rhs.m_particleInheritVelocityPercent)
 , m_particleClusterCount(rhs.m_particleClusterCount)
 , m_particleClusterRadius(rhs.m_particleClusterRadius)
 , m_particleLifeTime(rhs.m_particleLifeTime)
 , m_particleWeight(rhs.m_particleWeight)
 , m_particleGroundCollision(rhs.m_particleGroundCollision)
 , m_particleKillParticlesOnCollision(rhs.m_particleKillParticlesOnCollision)
 , m_particleCollisionHeightAboveGround(rhs.m_particleCollisionHeightAboveGround)
 , m_particleForwardVelocityMaintainedMin(rhs.m_particleForwardVelocityMaintainedMin)
 , m_particleForwardVelocityMaintainedMax(rhs.m_particleForwardVelocityMaintainedMax)
 , m_particleUpVelocityMaintainedMin(rhs.m_particleUpVelocityMaintainedMin)
 , m_particleUpVelocityMaintainedMax(rhs.m_particleUpVelocityMaintainedMax)
 , m_windResistenceGlobalPercent(rhs.m_windResistenceGlobalPercent)
 , m_soundPath()
 , m_localSpaceParticles(rhs.m_localSpaceParticles)
 , m_cachedSoundTemplate(NULL)
 , m_emitterLodDistanceMin(rhs.m_emitterLodDistanceMin)
 , m_emitterLodDistanceMax(rhs.m_emitterLodDistanceMax)
 , m_flockingType(rhs.m_flockingType)
 , m_flockingSeperationDistance(rhs.m_flockingSeperationDistance)
 , m_flockingAlignmentGain(rhs.m_flockingAlignmentGain)
 , m_flockingCohesionGain(rhs.m_flockingCohesionGain)
 , m_flockingSeperationGain(rhs.m_flockingSeperationGain)
 , m_flockingCageWidth(rhs.m_flockingCageWidth)
 , m_flockingCageHeight(rhs.m_flockingCageHeight)
 , m_flockingCageShape(rhs.m_flockingCageShape)
 , m_flockingCageEdgeType(rhs.m_flockingCageEdgeType)
 , m_flockingHeadingChangeRadian(rhs.m_flockingHeadingChangeRadian)
 , m_flockingHeadingChangeTime(rhs.m_flockingHeadingChangeTime)
 , m_particleTimeOfDayColorPercent(rhs.m_particleTimeOfDayColorPercent)
 , m_particleSnapToTerrainOnCreation(rhs.m_particleSnapToTerrainOnCreation)
 , m_particleAlignToTerrainNormalOnCreation(rhs.m_particleAlignToTerrainNormalOnCreation)
 , m_particleSnapToTerrainOnCreationHeight(rhs.m_particleSnapToTerrainOnCreationHeight)
 , m_particleChangeDirectionRadian(rhs.m_particleChangeDirectionRadian)
 , m_particleChangeDirectionTime(rhs.m_particleChangeDirectionTime)
 , m_firstParticleImmediately(rhs.m_firstParticleImmediately)
 , m_usePriorityParticles(rhs.m_usePriorityParticles)
{
	setSoundPath(rhs.m_soundPath);

	ParticleDescriptionQuad *particleDescrptionQuad = dynamic_cast<ParticleDescriptionQuad *>(rhs.m_particleDescription);

	if (particleDescrptionQuad)
	{
		m_particleDescription = new ParticleDescriptionQuad(*particleDescrptionQuad);
		NOT_NULL(m_particleDescription);
	}
	else
	{
		ParticleDescriptionMesh *particleDescriptionMesh = dynamic_cast<ParticleDescriptionMesh *>(rhs.m_particleDescription);

		if (particleDescriptionMesh)
		{
			m_particleDescription = new ParticleDescriptionMesh(*particleDescriptionMesh);
			NOT_NULL(m_particleDescription);
		}
		else
		{
			DEBUG_FATAL(true, ("ParticleEmitterDescription::ParticleEmitterDescription() - Un-supported particle type."));
		}
	}
}

//--------------------------------------------------------------------------
ParticleEmitterDescription &ParticleEmitterDescription::operator =(ParticleEmitterDescription const &rhs)
{
	if (this != &rhs)
	{
#ifdef _DEBUG
		m_emitterName = rhs.m_emitterName;
#endif // _DEBUG
		m_timing = rhs.m_timing;
		m_emitterEmitDirection = rhs.m_emitterEmitDirection;
		m_generationType = rhs.m_generationType;
		m_emitterLoopImmediately = rhs.m_emitterLoopImmediately;
		m_emitterLifeTimeMin = rhs.m_emitterLifeTimeMin;
		m_emitterLifeTimeMax = rhs.m_emitterLifeTimeMax;
		m_emitterMaxParticles = rhs.m_emitterMaxParticles;
		m_emitterOneShot = rhs.m_emitterOneShot;
		m_emitterOneShotMin = rhs.m_emitterOneShotMin;
		m_emitterOneShotMax = rhs.m_emitterOneShotMax;
		m_emitterTranslationX = rhs.m_emitterTranslationX;
		m_emitterTranslationY = rhs.m_emitterTranslationY;
		m_emitterTranslationZ = rhs.m_emitterTranslationZ;
		m_emitterRotationX = rhs.m_emitterRotationX;
		m_emitterRotationY = rhs.m_emitterRotationY;
		m_emitterRotationZ = rhs.m_emitterRotationZ;
		m_emitterDistance = rhs.m_emitterDistance;
		m_emitterShapeSize = rhs.m_emitterShapeSize;
		m_emitterSpread = rhs.m_emitterSpread;
		m_particleOrientation = rhs.m_particleOrientation;
		m_particleRandomInitialRotation = rhs.m_particleRandomInitialRotation;
		m_particleVisible = rhs.m_particleVisible;
		m_particleGenerationRate = rhs.m_particleGenerationRate;
		m_particleEmitSpeed = rhs.m_particleEmitSpeed;
		m_particleInheritVelocityPercent = rhs.m_particleInheritVelocityPercent;
		m_particleClusterCount = rhs.m_particleClusterCount;
		m_particleClusterRadius = rhs.m_particleClusterRadius;
		m_particleLifeTime = rhs.m_particleLifeTime;
		m_particleWeight = rhs.m_particleWeight;
		m_particleGroundCollision = rhs.m_particleGroundCollision;
		m_particleKillParticlesOnCollision = rhs.m_particleKillParticlesOnCollision;
		m_particleCollisionHeightAboveGround = rhs.m_particleCollisionHeightAboveGround;
		m_particleForwardVelocityMaintainedMin = rhs.m_particleForwardVelocityMaintainedMin;
		m_particleForwardVelocityMaintainedMax = rhs.m_particleForwardVelocityMaintainedMax;
		m_particleUpVelocityMaintainedMin = rhs.m_particleUpVelocityMaintainedMin;
		m_particleUpVelocityMaintainedMax = rhs.m_particleUpVelocityMaintainedMax;
		m_windResistenceGlobalPercent = rhs.m_windResistenceGlobalPercent;
		m_emitterLodDistanceMin = rhs.m_emitterLodDistanceMin;
		m_emitterLodDistanceMax = rhs.m_emitterLodDistanceMax;
		setSoundPath(rhs.m_soundPath);
		m_localSpaceParticles = rhs.m_localSpaceParticles;
		m_flockingType = rhs.m_flockingType;
		m_flockingSeperationDistance = rhs.m_flockingSeperationDistance;
 		m_flockingAlignmentGain = rhs.m_flockingAlignmentGain;
		m_flockingCohesionGain = rhs.m_flockingCohesionGain;
		m_flockingSeperationGain = rhs.m_flockingSeperationGain;
		m_flockingCageWidth = rhs.m_flockingCageWidth;
		m_flockingCageHeight = rhs.m_flockingCageHeight;
		m_flockingCageShape = rhs.m_flockingCageShape;
		m_flockingCageEdgeType = rhs.m_flockingCageEdgeType;
		m_flockingHeadingChangeRadian = rhs.m_flockingHeadingChangeRadian;
		m_flockingHeadingChangeTime = rhs.m_flockingHeadingChangeTime;
		m_particleTimeOfDayColorPercent = rhs.m_particleTimeOfDayColorPercent;
		m_particleSnapToTerrainOnCreation = rhs.m_particleSnapToTerrainOnCreation;
		m_particleAlignToTerrainNormalOnCreation = rhs.m_particleAlignToTerrainNormalOnCreation;
		m_particleSnapToTerrainOnCreationHeight = rhs.m_particleSnapToTerrainOnCreationHeight;
		m_particleChangeDirectionRadian = rhs.m_particleChangeDirectionRadian;
		m_particleChangeDirectionTime = rhs.m_particleChangeDirectionTime;
		m_firstParticleImmediately = rhs.m_firstParticleImmediately;
		m_usePriorityParticles = rhs.m_usePriorityParticles;

		delete m_emitterShape;
		m_emitterShape = ParticleEmitterShape::createEmitterShape(rhs.m_emitterShape->getEnum());

		delete m_particleDescription;
		m_particleDescription = rhs.m_particleDescription->clone();
	}

	return *this;
}

//-----------------------------------------------------------------------------
ParticleEmitterDescription::~ParticleEmitterDescription()
{
	delete m_particleDescription;
	delete m_emitterShape;

	if (m_cachedSoundTemplate != NULL)
	{
		SoundTemplateList::release(m_cachedSoundTemplate);
		m_cachedSoundTemplate = NULL;
	}
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setSoundPath(std::string const &path)
{
	if (m_soundPath != path)
	{
		if (m_cachedSoundTemplate != NULL)
		{
			SoundTemplateList::release(m_cachedSoundTemplate);
			m_cachedSoundTemplate = NULL;
		}

		m_soundPath = path;

		if (!path.empty())
		{
			m_cachedSoundTemplate = SoundTemplateList::fetch(path.c_str());
		}
	}
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::initializeDefault()
{
	// Initialize values for the member variables

#ifdef _DEBUG
	m_emitterName = m_defaultEmitterName;
#endif // _DEBUG

	m_timing.reset();
	m_emitterEmitDirection = m_defaultEmitterEmitDirection;
	m_generationType = G_rate;
	m_emitterLoopImmediately = m_defaultEmitterLoopImmediately;
	m_emitterLifeTimeMin = m_defaultEmitterLifeTime;
	m_emitterLifeTimeMax = m_defaultEmitterLifeTime;
	m_emitterMaxParticles = m_defaultEmitterMaxParticles;
	m_emitterOneShot = m_defaultEmitterOneShot;
	m_emitterOneShotMin = m_defaultEmitterOneShotMinMax;
	m_emitterOneShotMax = m_defaultEmitterOneShotMinMax;
	m_soundPath = "";
	m_localSpaceParticles = false;

	setDefaultEmitterTranslationX(m_emitterTranslationX);
	setDefaultEmitterTranslationY(m_emitterTranslationY);
	setDefaultEmitterTranslationZ(m_emitterTranslationZ);
	setDefaultEmitterRotationX(m_emitterRotationX);
	setDefaultEmitterRotationY(m_emitterRotationY);
	setDefaultEmitterRotationZ(m_emitterRotationZ);
	setDefaultEmitterDistance(m_emitterDistance);
	setDefaultEmitterShapeSize(m_emitterShapeSize);
	setDefaultEmitterSpread(m_emitterSpread);

	m_particleRandomInitialRotation = m_defaultParticleRandomInitialRotation;
	m_particleOrientation = m_defaultParticleOrientation;
	m_particleVisible = m_defaultParticleVisible;

	setDefaultParticleGenerationRate(m_particleGenerationRate);
	setDefaultParticleEmitSpeed(m_particleEmitSpeed);
	setDefaultParticleInheritVelocityPercent(m_particleInheritVelocityPercent);
	setDefaultParticleClusterCount(m_particleClusterCount);
	setDefaultParticleClusterRadius(m_particleClusterRadius);
	setDefaultParticleLifeTime(m_particleLifeTime);
	setDefaultParticleWeight(m_particleWeight);

	m_particleGroundCollision = false;
	m_particleKillParticlesOnCollision = false;
	m_particleCollisionHeightAboveGround = 0.0f;
	m_particleForwardVelocityMaintainedMin = m_defaultParticleVelocityMaintained;
	m_particleForwardVelocityMaintainedMax = m_defaultParticleVelocityMaintained;
	m_particleUpVelocityMaintainedMin = m_defaultParticleVelocityMaintained;
	m_particleUpVelocityMaintainedMax = m_defaultParticleVelocityMaintained;
	m_windResistenceGlobalPercent = 1.0f;
	m_emitterLodDistanceMin = -1.0f;
	m_emitterLodDistanceMax = -1.0f;
	m_firstParticleImmediately = false;
	m_usePriorityParticles = false;

	delete m_emitterShape;
	m_emitterShape = ParticleEmitterShape::createEmitterShape(ParticleEmitterShape::Shape_circle);

	delete m_particleDescription;
	m_particleDescription = new ParticleDescriptionQuad();

	// Put a single description in the list

	//ParticleDescription *particleDescription = new ParticleDescription;
	//m_particleDescription->push_back(particleDescription);

	//m_particleDescription = new ParticleDescription;
	m_particleDescription->initializeDefault();
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultEmitterTranslationX(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Translation X");
#endif // _DEBUG
	waveForm.clear();
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultEmitterTranslationY(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Translation Y");
#endif // _DEBUG
	waveForm.clear();
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultEmitterTranslationZ(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Translation Z");
#endif // _DEBUG
	waveForm.clear();
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultEmitterRotationX(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Rotation X");
#endif // _DEBUG
	waveForm.clear();
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultEmitterRotationY(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Rotation Y");
#endif // _DEBUG
	waveForm.clear();
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultEmitterRotationZ(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Rotation Z");
#endif // _DEBUG
	waveForm.clear();
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultEmitterDistance(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Emit Distance");
#endif // _DEBUG
	waveForm.clear();
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultEmitterShapeSize(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Shape Size");
#endif // _DEBUG
	waveForm.clear();
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
	waveForm.setValueMin(0.0f);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultEmitterSpread(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Emit Spread");
#endif // _DEBUG
	waveForm.clear();
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
	waveForm.setValueMin(0.0f);
	waveForm.setValueMax(180.0f);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultParticleGenerationRate(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Particle Generation Rate");
#endif // _DEBUG
	waveForm.clear();
	waveForm.insert(WaveFormControlPoint(0.0f, 8.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 8.0f));
	waveForm.setValueMin(0.0f);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultParticleEmitSpeed(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Particle Emit Speed");
#endif // _DEBUG
	waveForm.clear();
	waveForm.insert(WaveFormControlPoint(0.0f, 1.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 1.0f));
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultParticleInheritVelocityPercent(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Particle Inherit Velocity Percent");
#endif // _DEBUG
	waveForm.clear();
	waveForm.randomize(0.0f, 0.0f);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultParticleClusterCount(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Particle Cluster Count");
#endif // _DEBUG
	waveForm.clear();
	waveForm.randomize(0.0f, 0.0f);
	waveForm.setValueMin(0.0f);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultParticleClusterRadius(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Particle Cluster Radius");
#endif // _DEBUG
	waveForm.clear();
	waveForm.randomize(0.0f, 0.0f);
	waveForm.setValueMin(0.0f);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultParticleLifeTime(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Particle Lifetime");
#endif // _DEBUG
	waveForm.clear();
	waveForm.insert(WaveFormControlPoint(0.0f, 2.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 2.0f));
	waveForm.setValueMin(0.01f);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setDefaultParticleWeight(WaveForm &waveForm)
{
#ifdef _DEBUG
	waveForm.setName("Emitter - Particle Weight");
#endif // _DEBUG
	waveForm.clear();
	waveForm.insert(WaveFormControlPoint(0.0f, 0.0f));
	waveForm.insert(WaveFormControlPoint(1.0f, 0.0f));
}

//--------------------------------------------------------------------------
std::string const &ParticleEmitterDescription::getSoundPath() const
{
	return m_soundPath;
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setEmitterLifeTime(float const min, float const max)
{
	DEBUG_FATAL((min < 0.0f) && (min != -1.0f), ("Invalid min emitter lifetime %f", min));
	DEBUG_FATAL((max < 0.0f) && (max != -1.0f), ("Invalid max emitter lifetime %f", max));

	m_emitterLifeTimeMin = min;
	m_emitterLifeTimeMax = max;

	DEBUG_FATAL((m_emitterLifeTimeMin > m_emitterLifeTimeMax), ("emitter lifetime: min(%f) > lod max(%f)", m_emitterLifeTimeMin, m_emitterLifeTimeMax));

	if (m_emitterLifeTimeMin > m_emitterLifeTimeMax)
	{
		std::swap(m_emitterLifeTimeMin, m_emitterLifeTimeMax);
	}
}

//--------------------------------------------------------------------------
float ParticleEmitterDescription::getEmitterLifeTimeMin() const
{
	return m_emitterLifeTimeMin;
}

//--------------------------------------------------------------------------
float ParticleEmitterDescription::getEmitterLifeTimeMax() const
{
	return m_emitterLifeTimeMax;
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setEmitterLodDistance(float const min, float const max)
{
	DEBUG_FATAL((min < 0.0f), ("Invalid min emitter lod distance %f", min));
	DEBUG_FATAL((max < 0.0f), ("Invalid max emitter lod distance %f", max));

	m_emitterLodDistanceMin = min;
	m_emitterLodDistanceMax = max;

	DEBUG_FATAL((m_emitterLodDistanceMin > m_emitterLodDistanceMax), ("emitter lod: min(%f) > max(%f)", m_emitterLodDistanceMin, m_emitterLodDistanceMax));

	if (m_emitterLodDistanceMin > m_emitterLodDistanceMax)
	{
		std::swap(m_emitterLodDistanceMin, m_emitterLodDistanceMax);
	}
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setEmitterLodDistanceNone()
{
	m_emitterLodDistanceMin = -2.0f;
	m_emitterLodDistanceMax = -2.0f;
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::setEmitterLodDistanceGlobal()
{
	m_emitterLodDistanceMin = -1.0f;
	m_emitterLodDistanceMax = -1.0f;
}

//--------------------------------------------------------------------------
float ParticleEmitterDescription::getEmitterLodDistanceMin() const
{
	return (m_emitterLodDistanceMin < 0.0f) ? ConfigClientParticle::getMinGlobalLodDistance() : m_emitterLodDistanceMin;
}

//--------------------------------------------------------------------------
float ParticleEmitterDescription::getEmitterLodDistanceMax() const
{
	return (m_emitterLodDistanceMax < 0.0f) ? ConfigClientParticle::getMaxGlobalLodDistance() : m_emitterLodDistanceMax;
}

//--------------------------------------------------------------------------
bool ParticleEmitterDescription::isEmitterUsingGlobalLod() const
{
	return ((m_emitterLodDistanceMin >= -1.0f) && (m_emitterLodDistanceMin < 0.0f)) ||
	        ((m_emitterLodDistanceMax >= -1.0f) && (m_emitterLodDistanceMax < 0.0f));
}

//--------------------------------------------------------------------------
bool ParticleEmitterDescription::isEmitterUsingNoLod() const
{
	bool result = true;

	if (isEmitterUsingGlobalLod())
	{
		result = false;
	}
	else
	{
		result = (m_emitterLodDistanceMin < 1.0f) || (m_emitterLodDistanceMax < 1.0f);
	}

	return result;
}

//--------------------------------------------------------------------------
bool ParticleEmitterDescription::isInfiniteLooping() const
{
	bool result = (m_timing.getLoopCountMin() == -1);

	if (!result &&
	    (m_particleDescription != NULL))
	{
		if (m_particleDescription->isInfiniteLooping())
		{
			result = true;
		}
	}

	return result;
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load(Iff &iff)
{
	iff.enterForm(TAG_EMTR);

	switch (iff.getCurrentName())
	{
		case TAG_0000: { load_0000(iff); } break;
		case TAG_0001: { load_0001(iff); } break;
		case TAG_0002: { load_0002(iff); } break;
		case TAG_0003: { load_0003(iff); } break;
		case TAG_0004: { load_0004(iff); } break;
		case TAG_0005: { load_0005(iff); } break;
		case TAG_0006: { load_0006(iff); } break;
		case TAG_0007: { load_0007(iff); } break;
		case TAG_0008: { load_0008(iff); } break;
		case TAG_0009: { load_0009(iff); } break;
		case TAG_0010: { load_0010(iff); } break;
		case TAG_0011: { load_0011(iff); } break;
		case TAG_0012: { load_0012(iff); } break;
		case TAG_0013: { load_0013(iff); } break;
		case TAG_0014: { load_0014(iff); } break;
		default:
			{
				char currentTagName[256];
				ConvertTagToString(iff.getCurrentName(), currentTagName);

				FATAL(true, ("ParticleEmitterDescription::load() - Unsupported data version: %s", currentTagName));
			}
			break;
	}

	iff.exitForm(TAG_EMTR);

	// Make sure there is at least 1 item in the description list

	NOT_NULL(m_particleDescription);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
	{
		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);
		m_particleWeight.scaleAll(0.28f);

		iff.enterChunk(TAG_0000);
		{
			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_uint32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_uint32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = m_emitterLifeTimeMin;
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = static_cast<int>(m_emitterMaxParticles - 1);
			m_emitterOneShotMax = static_cast<int>(m_emitterMaxParticles - 1);
			m_particleRandomInitialRotation = iff.read_bool8();

			bool particleOrientWithVelocity = iff.read_bool8();
			if (particleOrientWithVelocity)
			{
				m_particleOrientation = PO_orientWithVelocityBankToCamera;
			}
			else
			{
				m_particleOrientation = PO_faceCamera;
			}

			iff.read_bool8(); // old var m_particleInheritTransform
			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_uint32());
		}
		iff.exitChunk(TAG_0000);

		delete m_particleDescription;
		m_particleDescription = new ParticleDescriptionQuad();
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0000);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);
	{
		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);
		m_particleWeight.scaleAll(0.28f);

		iff.enterChunk(TAG_0000);
		{
			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_uint32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_uint32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = m_emitterLifeTimeMin;
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = static_cast<int>(m_emitterMaxParticles - 1);
			m_emitterOneShotMax = static_cast<int>(m_emitterMaxParticles - 1);
			m_particleRandomInitialRotation = iff.read_bool8();
			m_particleOrientation = static_cast<ParticleOrientation>(iff.read_uint32());


			iff.read_bool8(); // old var m_particleInheritTransform
			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_uint32());
		}
		iff.exitChunk(TAG_0000);

		delete m_particleDescription;
		m_particleDescription = new ParticleDescriptionQuad();
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0001);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);
	{
		// Get the timing information

		m_timing.load(iff);

		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);
		m_particleWeight.scaleAll(0.28f);

		iff.enterChunk(TAG_0000);
		{
			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_uint32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_uint32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = m_emitterLifeTimeMin;
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = static_cast<int>(m_emitterMaxParticles - 1);
			m_emitterOneShotMax = static_cast<int>(m_emitterMaxParticles - 1);
			m_particleRandomInitialRotation = iff.read_bool8();
			m_particleOrientation = static_cast<ParticleOrientation>(iff.read_uint32());

			if (static_cast<int>(m_particleOrientation) == 3)
			{
				m_particleOrientation = PO_orientWithVelocityBankToCamera;
			}
			iff.read_bool8(); // old var m_particleInheritTransform
			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_uint32());
		}
		iff.exitChunk(TAG_0000);

		delete m_particleDescription;
		m_particleDescription = new ParticleDescriptionQuad();
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0002);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0003(Iff &iff)
{
	iff.enterForm(TAG_0003);
	{
		// Get the timing information

		m_timing.load(iff);

		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);
		m_particleWeight.scaleAll(0.28f);

		iff.enterChunk(TAG_0000);
		{
#ifdef _DEBUG
			iff.read_string(m_emitterName);
#else
			std::string tempName;
			iff.read_string(tempName);
#endif // _DEBUG

			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_int32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_int32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = m_emitterLifeTimeMin;
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = static_cast<int>(m_emitterMaxParticles - 1);
			m_emitterOneShotMax = static_cast<int>(m_emitterMaxParticles - 1);
			m_particleRandomInitialRotation = iff.read_bool8();
			m_particleOrientation = static_cast<ParticleOrientation>(iff.read_int32());

			iff.read_bool8(); // old var m_particleInheritTransform
			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_int32());
		}
		iff.exitChunk(TAG_0000);

		delete m_particleDescription;
		m_particleDescription = ParticleDescription::createDescription(ParticleDescription::PT_quad);
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0003);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0004(Iff &iff)
{
	iff.enterForm(TAG_0004);
	{
		// Get the timing information

		m_timing.load(iff);

		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);
		m_particleWeight.scaleAll(0.28f);

		ParticleDescription::ParticleType particleType;

		iff.enterChunk(TAG_0000);
		{
#ifdef _DEBUG
			iff.read_string(m_emitterName);
#else
			std::string tempName;
			iff.read_string(tempName);
#endif // _DEBUG

			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_int32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_int32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = m_emitterLifeTimeMin;
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = static_cast<int>(m_emitterMaxParticles - 1);
			m_emitterOneShotMax = static_cast<int>(m_emitterMaxParticles - 1);
			m_particleRandomInitialRotation = iff.read_bool8();
			m_particleOrientation = static_cast<ParticleOrientation>(iff.read_int32());

			iff.read_bool8(); // old var m_particleInheritTransform
			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_int32());

			particleType = static_cast<ParticleDescription::ParticleType>(iff.read_int32());
		}
		iff.exitChunk(TAG_0000);

		delete m_particleDescription;
		m_particleDescription = ParticleDescription::createDescription(particleType);
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0004);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0005(Iff &iff)
{
	iff.enterForm(TAG_0005);
	{
		// Get the timing information

		m_timing.load(iff);

		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);
		m_particleWeight.scaleAll(0.28f);

		ParticleDescription::ParticleType particleType;

		iff.enterChunk(TAG_0000);
		{
#ifdef _DEBUG
			iff.read_string(m_emitterName);
#else
			std::string tempName;
			iff.read_string(tempName);
#endif // _DEBUG

			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_int32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_int32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = m_emitterLifeTimeMin;
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = static_cast<int>(m_emitterMaxParticles - 1);
			m_emitterOneShotMax = static_cast<int>(m_emitterMaxParticles - 1);
			m_particleRandomInitialRotation = iff.read_bool8();
			m_particleOrientation = static_cast<ParticleOrientation>(iff.read_int32());

			iff.read_bool8(); // old var m_particleInheritTransform
			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_int32());

			particleType = static_cast<ParticleDescription::ParticleType>(iff.read_int32());
			std::string soundPath;
			iff.read_string(soundPath);
			setSoundPath(soundPath);
		}
		iff.exitChunk(TAG_0000);

		delete m_particleDescription;
		m_particleDescription = ParticleDescription::createDescription(particleType);
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0005);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0006(Iff &iff)
{
	iff.enterForm(TAG_0006);
	{
		// Get the timing information

		m_timing.load(iff);

		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);
		m_particleWeight.scaleAll(0.28f);

		ParticleDescription::ParticleType particleType;

		iff.enterChunk(TAG_0000);
		{
#ifdef _DEBUG
			iff.read_string(m_emitterName);
#else
			std::string tempName;
			iff.read_string(tempName);
#endif // _DEBUG

			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_int32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_int32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = m_emitterLifeTimeMin;
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = static_cast<int>(m_emitterMaxParticles - 1);
			m_emitterOneShotMax = static_cast<int>(m_emitterMaxParticles - 1);
			m_particleRandomInitialRotation = iff.read_bool8();
			m_particleOrientation = static_cast<ParticleOrientation>(iff.read_int32());

			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_int32());

			particleType = static_cast<ParticleDescription::ParticleType>(iff.read_int32());
			std::string soundPath;
			iff.read_string(soundPath);
			setSoundPath(soundPath);

			m_localSpaceParticles = iff.read_bool8();
		}
		iff.exitChunk(TAG_0000);

		delete m_particleDescription;
		m_particleDescription = ParticleDescription::createDescription(particleType);
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0006);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0007(Iff &iff)
{
	iff.enterForm(TAG_0007);
	{
		// Get the timing information

		m_timing.load(iff);

		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);

		ParticleDescription::ParticleType particleType;

		iff.enterChunk(TAG_0000);
		{
#ifdef _DEBUG
			iff.read_string(m_emitterName);
#else
			std::string tempName;
			iff.read_string(tempName);
#endif // _DEBUG

			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_int32());
			m_generationType = static_cast<GenerationType>(iff.read_int32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_int32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = m_emitterLifeTimeMin;
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = iff.read_int32();
			m_emitterOneShotMax = iff.read_int32();
			m_particleRandomInitialRotation = iff.read_bool8();
			m_particleOrientation = static_cast<ParticleOrientation>(iff.read_int32());

			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_int32());

			particleType = static_cast<ParticleDescription::ParticleType>(iff.read_int32());
			std::string soundPath;
			iff.read_string(soundPath);
			setSoundPath(soundPath);

			m_localSpaceParticles = iff.read_bool8();

			m_particleGroundCollision = iff.read_bool8();
			iff.read_float();
			m_particleCollisionHeightAboveGround = iff.read_float();
			m_particleForwardVelocityMaintainedMin = iff.read_float();
			m_particleForwardVelocityMaintainedMax = iff.read_float();
			m_particleUpVelocityMaintainedMin = iff.read_float();
			m_particleUpVelocityMaintainedMax = iff.read_float();
			m_windResistenceGlobalPercent = iff.read_float();
		}
		iff.exitChunk(TAG_0000);

		delete m_particleDescription;
		m_particleDescription = ParticleDescription::createDescription(particleType);
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0007);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0008(Iff &iff)
{
	iff.enterForm(TAG_0008);
	{
		// Get the timing information

		m_timing.load(iff);

		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);

		ParticleDescription::ParticleType particleType;

		iff.enterChunk(TAG_0000);
		{
#ifdef _DEBUG
			iff.read_string(m_emitterName);
#else
			std::string tempName;
			iff.read_string(tempName);
#endif // _DEBUG

			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_int32());
			m_generationType = static_cast<GenerationType>(iff.read_int32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_int32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = m_emitterLifeTimeMin;
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = iff.read_int32();
			m_emitterOneShotMax = iff.read_int32();
			m_particleRandomInitialRotation = iff.read_bool8();
			m_particleOrientation = static_cast<ParticleOrientation>(iff.read_int32());

			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_int32());

			particleType = static_cast<ParticleDescription::ParticleType>(iff.read_int32());
			std::string soundPath;
			iff.read_string(soundPath);
			setSoundPath(soundPath);

			m_localSpaceParticles = iff.read_bool8();

			m_particleGroundCollision = iff.read_bool8();
			m_particleKillParticlesOnCollision = iff.read_bool8();
			iff.read_float();
			m_particleCollisionHeightAboveGround = iff.read_float();
			m_particleForwardVelocityMaintainedMin = iff.read_float();
			m_particleForwardVelocityMaintainedMax = iff.read_float();
			m_particleUpVelocityMaintainedMin = iff.read_float();
			m_particleUpVelocityMaintainedMax = iff.read_float();
			m_windResistenceGlobalPercent = iff.read_float();
		}
		iff.exitChunk(TAG_0000);

		delete m_particleDescription;
		m_particleDescription = ParticleDescription::createDescription(particleType);
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0008);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0009(Iff &iff)
{
	iff.enterForm(TAG_0009);
	{
		// Get the timing information

		m_timing.load(iff);

		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);

		ParticleDescription::ParticleType particleType;

		iff.enterChunk(TAG_0000);
		{
#ifdef _DEBUG
			iff.read_string(m_emitterName);
#else
			std::string tempName;
			iff.read_string(tempName);
#endif // _DEBUG

			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_int32());
			m_generationType = static_cast<GenerationType>(iff.read_int32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_int32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = iff.read_float();
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = iff.read_int32();
			m_emitterOneShotMax = iff.read_int32();
			m_particleRandomInitialRotation = iff.read_bool8();
			m_particleOrientation = static_cast<ParticleOrientation>(iff.read_int32());

			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_int32());

			particleType = static_cast<ParticleDescription::ParticleType>(iff.read_int32());
			std::string soundPath;
			iff.read_string(soundPath);
			setSoundPath(soundPath);

			m_localSpaceParticles = iff.read_bool8();

			m_particleGroundCollision = iff.read_bool8();
			m_particleKillParticlesOnCollision = iff.read_bool8();
			iff.read_float();
			m_particleCollisionHeightAboveGround = iff.read_float();
			m_particleForwardVelocityMaintainedMin = iff.read_float();
			m_particleForwardVelocityMaintainedMax = iff.read_float();
			m_particleUpVelocityMaintainedMin = iff.read_float();
			m_particleUpVelocityMaintainedMax = iff.read_float();
			m_windResistenceGlobalPercent = iff.read_float();
			m_emitterLodDistanceMin = iff.read_float();
			m_emitterLodDistanceMax = iff.read_float();
		}
		iff.exitChunk(TAG_0000);

		delete m_particleDescription;
		m_particleDescription = ParticleDescription::createDescription(particleType);
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0009);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0010(Iff &iff)
{
	iff.enterForm(TAG_0010);
	{
		// Get the timing information

		m_timing.load(iff);

		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);

		ParticleDescription::ParticleType particleType;

		iff.enterChunk(TAG_0000);
		{
#ifdef _DEBUG
			iff.read_string(m_emitterName);
#else
			std::string tempName;
			iff.read_string(tempName);
#endif // _DEBUG

			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_int32());
			m_generationType = static_cast<GenerationType>(iff.read_int32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_int32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = iff.read_float();
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = iff.read_int32();
			m_emitterOneShotMax = iff.read_int32();
			m_particleRandomInitialRotation = iff.read_bool8();
			m_particleOrientation = static_cast<ParticleOrientation>(iff.read_int32());

			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_int32());

			particleType = static_cast<ParticleDescription::ParticleType>(iff.read_int32());
			std::string soundPath;
			iff.read_string(soundPath);
			setSoundPath(soundPath);

			m_localSpaceParticles = iff.read_bool8();

			m_particleGroundCollision = iff.read_bool8();
			m_particleKillParticlesOnCollision = iff.read_bool8();
			iff.read_float();
			m_particleCollisionHeightAboveGround = iff.read_float();
			m_particleForwardVelocityMaintainedMin = iff.read_float();
			m_particleForwardVelocityMaintainedMax = iff.read_float();
			m_particleUpVelocityMaintainedMin = iff.read_float();
			m_particleUpVelocityMaintainedMax = iff.read_float();
			m_windResistenceGlobalPercent = iff.read_float();
			m_emitterLodDistanceMin = iff.read_float();
			m_emitterLodDistanceMax = iff.read_float();
			m_flockingType = (iff.read_bool8() == 0) ? FT_none : FT_air;
			m_flockingSeperationDistance = iff.read_float();
			m_flockingAlignmentGain = iff.read_float();
			m_flockingCohesionGain = iff.read_float();
			m_flockingSeperationGain = iff.read_float();
			m_flockingCageWidth = iff.read_float();
			float const cageMin = iff.read_float();
			float const cageMax = iff.read_float();
			m_flockingCageHeight = cageMax - cageMin;
		}
		iff.exitChunk(TAG_0000);

		delete m_particleDescription;
		m_particleDescription = ParticleDescription::createDescription(particleType);
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0010);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0011(Iff &iff)
{
	iff.enterForm(TAG_0011);
	{
		// Get the timing information

		m_timing.load(iff);

		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);

		ParticleDescription::ParticleType particleType;

		iff.enterChunk(TAG_0000);
		{
#ifdef _DEBUG
			iff.read_string(m_emitterName);
#else
			std::string tempName;
			iff.read_string(tempName);
#endif // _DEBUG

			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_int32());
			m_generationType = static_cast<GenerationType>(iff.read_int32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_int32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = iff.read_float();
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = iff.read_int32();
			m_emitterOneShotMax = iff.read_int32();
			m_particleRandomInitialRotation = iff.read_bool8();
			m_particleOrientation = static_cast<ParticleOrientation>(iff.read_int32());

			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_int32());

			particleType = static_cast<ParticleDescription::ParticleType>(iff.read_int32());
			std::string soundPath;
			iff.read_string(soundPath);
			setSoundPath(soundPath);

			m_localSpaceParticles = iff.read_bool8();

			m_particleGroundCollision = iff.read_bool8();
			m_particleKillParticlesOnCollision = iff.read_bool8();
			iff.read_float();
			m_particleCollisionHeightAboveGround = iff.read_float();
			m_particleForwardVelocityMaintainedMin = iff.read_float();
			m_particleForwardVelocityMaintainedMax = iff.read_float();
			m_particleUpVelocityMaintainedMin = iff.read_float();
			m_particleUpVelocityMaintainedMax = iff.read_float();
			m_windResistenceGlobalPercent = iff.read_float();
			m_emitterLodDistanceMin = iff.read_float();
			m_emitterLodDistanceMax = iff.read_float();
			m_flockingType = (iff.read_bool8() == 0) ? FT_none : FT_air;
			m_flockingSeperationDistance = iff.read_float();
			m_flockingAlignmentGain = iff.read_float();
			m_flockingCohesionGain = iff.read_float();
			m_flockingSeperationGain = iff.read_float();
			m_flockingCageWidth = iff.read_float();
			float const cageMin = iff.read_float();
			float const cageMax = iff.read_float();
			m_flockingCageHeight = cageMax - cageMin;
			m_particleTimeOfDayColorPercent = iff.read_float();
		}
		iff.exitChunk(TAG_0000);

		delete m_particleDescription;
		m_particleDescription = ParticleDescription::createDescription(particleType);
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0011);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0012(Iff &iff)
{
	iff.enterForm(TAG_0012);
	{
		// Get the timing information

		m_timing.load(iff);

		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);

		ParticleDescription::ParticleType particleType;

		iff.enterChunk(TAG_0000);
		{
#ifdef _DEBUG
			iff.read_string(m_emitterName);
#else
			std::string tempName;
			iff.read_string(tempName);
#endif // _DEBUG

			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_int32());
			m_generationType = static_cast<GenerationType>(iff.read_int32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_int32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = iff.read_float();
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = iff.read_int32();
			m_emitterOneShotMax = iff.read_int32();
			m_particleRandomInitialRotation = iff.read_bool8();
			m_particleOrientation = static_cast<ParticleOrientation>(iff.read_int32());

			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_int32());

			particleType = static_cast<ParticleDescription::ParticleType>(iff.read_int32());
			std::string soundPath;
			iff.read_string(soundPath);
			setSoundPath(soundPath);

			m_localSpaceParticles = iff.read_bool8();

			m_particleGroundCollision = iff.read_bool8();
			m_particleKillParticlesOnCollision = iff.read_bool8();
			m_particleCollisionHeightAboveGround = iff.read_float();
			m_particleForwardVelocityMaintainedMin = iff.read_float();
			m_particleForwardVelocityMaintainedMax = iff.read_float();
			m_particleUpVelocityMaintainedMin = iff.read_float();
			m_particleUpVelocityMaintainedMax = iff.read_float();
			m_windResistenceGlobalPercent = iff.read_float();
			m_emitterLodDistanceMin = iff.read_float();
			m_emitterLodDistanceMax = iff.read_float();
			m_flockingType = static_cast<FlockingType>(iff.read_int32());
			m_flockingSeperationDistance = iff.read_float();
			m_flockingAlignmentGain = iff.read_float();
			m_flockingCohesionGain = iff.read_float();
			m_flockingSeperationGain = iff.read_float();
			m_flockingCageWidth = iff.read_float();
			m_flockingCageHeight = iff.read_float();
			m_flockingCageShape	= static_cast<FlockCageShape>(iff.read_int32());
			m_flockingCageEdgeType = static_cast<FlockCageEdgeType>(iff.read_int32());
			m_flockingHeadingChangeRadian = iff.read_float();
			m_flockingHeadingChangeTime = iff.read_float();
			m_particleTimeOfDayColorPercent = iff.read_float();
			m_particleSnapToTerrainOnCreation = iff.read_bool8();
			m_particleAlignToTerrainNormalOnCreation = iff.read_bool8();
			m_particleSnapToTerrainOnCreationHeight = iff.read_float();
			m_particleChangeDirectionRadian = iff.read_float();
			m_particleChangeDirectionTime = iff.read_float();
		}
		iff.exitChunk(TAG_0000);

		//for (int i = 0; i < particleDescriptionCount; ++i)
		//{
		// ParticleDescription *particleDescription = new ParticleDescription;
		// particleDescription->load(iff);
		//
		// m_particleDescription->push_back(particleDescription);
		//}

		delete m_particleDescription;
		m_particleDescription = ParticleDescription::createDescription(particleType);
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0012);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0013(Iff &iff)
{
	iff.enterForm(TAG_0013);
	{
		// Get the timing information

		m_timing.load(iff);

		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);

		ParticleDescription::ParticleType particleType;

		iff.enterChunk(TAG_0000);
		{
#ifdef _DEBUG
			iff.read_string(m_emitterName);
#else
			std::string tempName;
			iff.read_string(tempName);
#endif // _DEBUG

			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_int32());
			m_generationType = static_cast<GenerationType>(iff.read_int32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_int32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = iff.read_float();
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = iff.read_int32();
			m_emitterOneShotMax = iff.read_int32();
			m_particleRandomInitialRotation = iff.read_bool8();
			m_particleOrientation = static_cast<ParticleOrientation>(iff.read_int32());

			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_int32());

			particleType = static_cast<ParticleDescription::ParticleType>(iff.read_int32());
			std::string soundPath;
			iff.read_string(soundPath);
			setSoundPath(soundPath);

			m_localSpaceParticles = iff.read_bool8();

			m_particleGroundCollision = iff.read_bool8();
			m_particleKillParticlesOnCollision = iff.read_bool8();
			m_particleCollisionHeightAboveGround = iff.read_float();
			m_particleForwardVelocityMaintainedMin = iff.read_float();
			m_particleForwardVelocityMaintainedMax = iff.read_float();
			m_particleUpVelocityMaintainedMin = iff.read_float();
			m_particleUpVelocityMaintainedMax = iff.read_float();
			m_windResistenceGlobalPercent = iff.read_float();
			m_emitterLodDistanceMin = iff.read_float();
			m_emitterLodDistanceMax = iff.read_float();
			m_flockingType = static_cast<FlockingType>(iff.read_int32());
			m_flockingSeperationDistance = iff.read_float();
			m_flockingAlignmentGain = iff.read_float();
			m_flockingCohesionGain = iff.read_float();
			m_flockingSeperationGain = iff.read_float();
			m_flockingCageWidth = iff.read_float();
			m_flockingCageHeight = iff.read_float();
			m_flockingCageShape	= static_cast<FlockCageShape>(iff.read_int32());
			m_flockingCageEdgeType = static_cast<FlockCageEdgeType>(iff.read_int32());
			m_flockingHeadingChangeRadian = iff.read_float();
			m_flockingHeadingChangeTime = iff.read_float();
			m_particleTimeOfDayColorPercent = iff.read_float();
			m_particleSnapToTerrainOnCreation = iff.read_bool8();
			m_particleAlignToTerrainNormalOnCreation = iff.read_bool8();
			m_particleSnapToTerrainOnCreationHeight = iff.read_float();
			m_particleChangeDirectionRadian = iff.read_float();
			m_particleChangeDirectionTime = iff.read_float();
			m_firstParticleImmediately = iff.read_bool8();
		}
		iff.exitChunk(TAG_0000);

		//for (int i = 0; i < particleDescriptionCount; ++i)
		//{
		// ParticleDescription *particleDescription = new ParticleDescription;
		// particleDescription->load(iff);
		//
		// m_particleDescription->push_back(particleDescription);
		//}

		delete m_particleDescription;
		m_particleDescription = ParticleDescription::createDescription(particleType);
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0013);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::load_0014(Iff &iff)
{
	iff.enterForm(TAG_0014);
	{
		// Get the timing information

		m_timing.load(iff);

		m_emitterTranslationX.load(iff);
		m_emitterTranslationY.load(iff);
		m_emitterTranslationZ.load(iff);
		m_emitterRotationX.load(iff);
		m_emitterRotationY.load(iff);
		m_emitterRotationZ.load(iff);
		m_emitterDistance.load(iff);
		m_emitterShapeSize.load(iff);
		m_emitterSpread.load(iff);
		m_particleGenerationRate.load(iff);
		m_particleEmitSpeed.load(iff);
		m_particleInheritVelocityPercent.load(iff);
		m_particleClusterCount.load(iff);
		m_particleClusterRadius.load(iff);
		m_particleLifeTime.load(iff);
		m_particleWeight.load(iff);

		ParticleDescription::ParticleType particleType;

		iff.enterChunk(TAG_0000);
		{
#ifdef _DEBUG
			iff.read_string(m_emitterName);
#else
			std::string tempName;
			iff.read_string(tempName);
#endif // _DEBUG

			m_emitterEmitDirection = static_cast<EmitDirection>(iff.read_int32());
			m_generationType = static_cast<GenerationType>(iff.read_int32());

			delete m_emitterShape;
			m_emitterShape = ParticleEmitterShape::createEmitterShape(static_cast<ParticleEmitterShape::Shape>(iff.read_int32()));
			m_emitterLoopImmediately = iff.read_bool8();
			m_emitterLifeTimeMin = iff.read_float();
			m_emitterLifeTimeMax = iff.read_float();
			m_emitterMaxParticles = iff.read_float();
			m_emitterOneShot = iff.read_bool8();
			m_emitterOneShotMin = iff.read_int32();
			m_emitterOneShotMax = iff.read_int32();
			m_particleRandomInitialRotation = iff.read_bool8();
			m_particleOrientation = static_cast<ParticleOrientation>(iff.read_int32());

			m_particleVisible = iff.read_bool8();

			// Particle description count

			IGNORE_RETURN(iff.read_int32());

			particleType = static_cast<ParticleDescription::ParticleType>(iff.read_int32());
			std::string soundPath;
			iff.read_string(soundPath);
			setSoundPath(soundPath);

			m_localSpaceParticles = iff.read_bool8();

			m_particleGroundCollision = iff.read_bool8();
			m_particleKillParticlesOnCollision = iff.read_bool8();
			m_particleCollisionHeightAboveGround = iff.read_float();
			m_particleForwardVelocityMaintainedMin = iff.read_float();
			m_particleForwardVelocityMaintainedMax = iff.read_float();
			m_particleUpVelocityMaintainedMin = iff.read_float();
			m_particleUpVelocityMaintainedMax = iff.read_float();
			m_windResistenceGlobalPercent = iff.read_float();
			m_emitterLodDistanceMin = iff.read_float();
			m_emitterLodDistanceMax = iff.read_float();
			m_flockingType = static_cast<FlockingType>(iff.read_int32());
			m_flockingSeperationDistance = iff.read_float();
			m_flockingAlignmentGain = iff.read_float();
			m_flockingCohesionGain = iff.read_float();
			m_flockingSeperationGain = iff.read_float();
			m_flockingCageWidth = iff.read_float();
			m_flockingCageHeight = iff.read_float();
			m_flockingCageShape	= static_cast<FlockCageShape>(iff.read_int32());
			m_flockingCageEdgeType = static_cast<FlockCageEdgeType>(iff.read_int32());
			m_flockingHeadingChangeRadian = iff.read_float();
			m_flockingHeadingChangeTime = iff.read_float();
			m_particleTimeOfDayColorPercent = iff.read_float();
			m_particleSnapToTerrainOnCreation = iff.read_bool8();
			m_particleAlignToTerrainNormalOnCreation = iff.read_bool8();
			m_particleSnapToTerrainOnCreationHeight = iff.read_float();
			m_particleChangeDirectionRadian = iff.read_float();
			m_particleChangeDirectionTime = iff.read_float();
			m_firstParticleImmediately = iff.read_bool8();
			m_usePriorityParticles = iff.read_bool8();			
		}
		iff.exitChunk(TAG_0000);

		//for (int i = 0; i < particleDescriptionCount; ++i)
		//{
		// ParticleDescription *particleDescription = new ParticleDescription;
		// particleDescription->load(iff);
		//
		// m_particleDescription->push_back(particleDescription);
		//}

		delete m_particleDescription;
		m_particleDescription = ParticleDescription::createDescription(particleType);
		m_particleDescription->load(iff);
	}
	iff.exitForm(TAG_0014);
}

//--------------------------------------------------------------------------
void ParticleEmitterDescription::write(Iff &iff) const
{
	// Set the current version number

	iff.insertForm(TAG_EMTR);
	{
		iff.insertForm(TAG_0014);
		{
			// Save the timing information

			m_timing.write(iff);

			// Save the waveforms

			m_emitterTranslationX.write(iff);
			m_emitterTranslationY.write(iff);
			m_emitterTranslationZ.write(iff);
			m_emitterRotationX.write(iff);
			m_emitterRotationY.write(iff);
			m_emitterRotationZ.write(iff);
			m_emitterDistance.write(iff);
			m_emitterShapeSize.write(iff);
			m_emitterSpread.write(iff);
			m_particleGenerationRate.write(iff);
			m_particleEmitSpeed.write(iff);
			m_particleInheritVelocityPercent.write(iff);
			m_particleClusterCount.write(iff);
			m_particleClusterRadius.write(iff);
			m_particleLifeTime.write(iff);
			m_particleWeight.write(iff);

			iff.insertChunk(TAG_0000);
			{
#ifdef _DEBUG
				iff.insertChunkString(m_emitterName.c_str());
#else
				iff.insertChunkString("");
#endif // _DEBUG

				iff.insertChunkData(static_cast<int32>(m_emitterEmitDirection));
				iff.insertChunkData(static_cast<int32>(m_generationType));

				int32 shapeEnum = static_cast<int32>(m_emitterShape->getEnum());
				iff.insertChunkData(shapeEnum);
				iff.insertChunkData(static_cast<uint8>(m_emitterLoopImmediately));
				iff.insertChunkData(m_emitterLifeTimeMin);
				iff.insertChunkData(m_emitterLifeTimeMax);
				iff.insertChunkData(m_emitterMaxParticles);
				iff.insertChunkData(static_cast<uint8>(m_emitterOneShot));
				iff.insertChunkData(static_cast<int32>(m_emitterOneShotMin));
				iff.insertChunkData(static_cast<int32>(m_emitterOneShotMax));
				iff.insertChunkData(static_cast<uint8>(m_particleRandomInitialRotation));
				iff.insertChunkData(static_cast<int32>(m_particleOrientation));
				iff.insertChunkData(static_cast<uint8>(m_particleVisible));

				//int const particleDescriptionCount = m_particleDescription->size();
				//
				//iff.insertChunkData(static_cast<uint32>(particleDescriptionCount));
				iff.insertChunkData(static_cast<int32>(1));

				iff.insertChunkData(static_cast<int32>(m_particleDescription->getParticleType()));
				iff.insertChunkString(m_soundPath.c_str());
				iff.insertChunkData(static_cast<uint8>(m_localSpaceParticles));
				iff.insertChunkData(static_cast<uint8>(m_particleGroundCollision));
				iff.insertChunkData(static_cast<uint8>(m_particleKillParticlesOnCollision));
				iff.insertChunkData(m_particleCollisionHeightAboveGround);
				iff.insertChunkData(m_particleForwardVelocityMaintainedMin);
				iff.insertChunkData(m_particleForwardVelocityMaintainedMax);
				iff.insertChunkData(m_particleUpVelocityMaintainedMin);
				iff.insertChunkData(m_particleUpVelocityMaintainedMax);
				iff.insertChunkData(m_windResistenceGlobalPercent);
				iff.insertChunkData(m_emitterLodDistanceMin);
				iff.insertChunkData(m_emitterLodDistanceMax);
				iff.insertChunkData(static_cast<int32>(m_flockingType));
				iff.insertChunkData(m_flockingSeperationDistance);
				iff.insertChunkData(m_flockingAlignmentGain);
				iff.insertChunkData(m_flockingCohesionGain);
				iff.insertChunkData(m_flockingSeperationGain);
				iff.insertChunkData(m_flockingCageWidth);
				iff.insertChunkData(m_flockingCageHeight);
				iff.insertChunkData(static_cast<int32>(m_flockingCageShape));
				iff.insertChunkData(static_cast<int32>(m_flockingCageEdgeType));
				iff.insertChunkData(m_flockingHeadingChangeRadian);
				iff.insertChunkData(m_flockingHeadingChangeTime);
				iff.insertChunkData(m_particleTimeOfDayColorPercent);
				iff.insertChunkData(m_particleSnapToTerrainOnCreation);
				iff.insertChunkData(m_particleAlignToTerrainNormalOnCreation);
				iff.insertChunkData(m_particleSnapToTerrainOnCreationHeight);
				iff.insertChunkData(m_particleChangeDirectionRadian);
				iff.insertChunkData(m_particleChangeDirectionTime);
				iff.insertChunkData(m_firstParticleImmediately);
				iff.insertChunkData(m_usePriorityParticles);
			}
			iff.exitChunk(TAG_0000);

			// Write all the particle descriptions

			//ParticleDescription::iterator current = m_particleDescription->begin();
			//
			//for (; current != m_particleDescription->end(); ++current)
			//{
			// ParticleDescription const *particleDescription = (*current);
			//
			// particleDescription->write(iff);
			//}

			m_particleDescription->write(iff);
		}
		iff.exitForm(TAG_0014);
	}
	iff.exitForm(TAG_EMTR);
}

// ============================================================================
