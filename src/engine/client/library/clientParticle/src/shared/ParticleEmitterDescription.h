// ============================================================================
//
// ParticleEmitterDescription.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleEmitterDescription_H
#define INCLUDED_ParticleEmitterDescription_H

#include "clientParticle/ParticleTiming.h"
#include "sharedMath/WaveForm.h"

class Iff;
class ParticleEmitterShape;
class ParticleDescription;
class SoundTemplate;
class Vector;

//-----------------------------------------------------------------------------
class ParticleEmitterDescription
{
public:

	enum ParticleOrientation
	{
		PO_faceCamera,
		PO_orientWithVelocity,
		PO_orientWithVelocityBankToCamera,
		PO_faceCameraMesh,
	};

	enum EmitDirection
	{
		ED_omni = 0,
		ED_directional,
		ED_invalid = -1
	};

	enum GenerationType
	{
		G_rate,                // Generates the number of particles per second
		G_distance             // Generates particles at the distance travelled
	};

	enum FlockingType
	{
		FT_none,
		FT_air,
		FT_ground,
		FT_water,
	};

	enum FlockCageShape
	{
		FCS_rectangle,
		FCS_cylinder
	};

	enum FlockCageEdgeType
	{
		FCET_soft,
		FCET_hard
	};

	// Default values

	static std::string const         m_defaultEmitterName;
	static EmitDirection const       m_defaultEmitterEmitDirection;
	static bool const                m_defaultEmitterLoopImmediately;
	static float const               m_defaultEmitterLifeTime;
	static float const               m_defaultEmitterMaxParticles;
	static bool const                m_defaultEmitterOneShot;
	static int const                 m_defaultEmitterOneShotMinMax;
	static bool const                m_defaultParticleRandomInitialRotation;
	static ParticleOrientation const m_defaultParticleOrientation;
	static bool const                m_defaultParticleVisible;
	static float const               m_defaultParticleVelocityMaintained;

public:

	//typedef stdlist<ParticleDescription *>::fwd ParticleDescriptionList;

	ParticleEmitterDescription();
	ParticleEmitterDescription(ParticleEmitterDescription const &rhs);
	~ParticleEmitterDescription();
	ParticleEmitterDescription &operator =(ParticleEmitterDescription const &rhs);

	void load(Iff &iff);
	void write(Iff &iff) const;

	void initializeDefault();
	bool isInfiniteLooping() const;

	ParticleDescription *    m_particleDescription;                   // 0
#ifdef _DEBUG
	std::string              m_emitterName;                           // 1
#endif // _DEBUG
	ParticleTiming           m_timing;                                // 2 The timing information
	EmitDirection            m_emitterEmitDirection;                  // 3
	GenerationType           m_generationType;                        // 4
	ParticleEmitterShape *   m_emitterShape;                          // 5
	bool                     m_emitterLoopImmediately;                // 6 Does the emitter wait for all its particles to die before looping (false), or just loop when its age hits the lifetime (true)
	float                    m_emitterMaxParticles;                   // 7
	bool                     m_emitterOneShot;                        // 8 Does the emitter emit all the specified particles in a single frame?
	int                      m_emitterOneShotMin;                     // 9
	int                      m_emitterOneShotMax;                     // 10
	WaveForm                 m_emitterTranslationX;                   // 11
	WaveForm                 m_emitterTranslationY;                   // 12
	WaveForm                 m_emitterTranslationZ;                   // 13
	WaveForm                 m_emitterRotationX;                      // 14
	WaveForm                 m_emitterRotationY;                      // 15
	WaveForm                 m_emitterRotationZ;                      // 16
	WaveForm                 m_emitterDistance;                       // 17
	WaveForm                 m_emitterShapeSize;                      // 18
	WaveForm                 m_emitterSpread;                         // 19
	bool                     m_particleRandomInitialRotation;         // 20
	ParticleOrientation      m_particleOrientation;                   // 21
	bool                     m_particleVisible;                       // 22 Are the particles visible, or is the movement of the particles used by an attached effect
	WaveForm                 m_particleGenerationRate;                // 23
	WaveForm                 m_particleEmitSpeed;                     // 24
	WaveForm                 m_particleInheritVelocityPercent;        // 25
	WaveForm                 m_particleClusterCount;                  // 26
	WaveForm                 m_particleClusterRadius;                 // 27
	WaveForm                 m_particleLifeTime;                      // 28
	WaveForm                 m_particleWeight;                        // 29
	bool                     m_particleGroundCollision;               // 30
	bool                     m_particleKillParticlesOnCollision;      // 31
	float                    m_particleCollisionHeightAboveGround;    // 32
	float                    m_particleForwardVelocityMaintainedMin;  // 34
	float                    m_particleForwardVelocityMaintainedMax;  // 35
	float                    m_particleUpVelocityMaintainedMin;       // 36
	float                    m_particleUpVelocityMaintainedMax;       // 37
	float                    m_windResistenceGlobalPercent;           // 38
	bool                     m_localSpaceParticles;                   // 39
	FlockingType             m_flockingType;                          // 40
	float                    m_flockingSeperationDistance;            // 41
	float                    m_flockingSeperationGain;                // 42
	float                    m_flockingAlignmentGain;                 // 43
	float                    m_flockingCohesionGain;                  // 44
	float                    m_flockingCageWidth;                     // 45
	float                    m_flockingCageHeight;                    // 46
	FlockCageShape           m_flockingCageShape;                     // 47
	FlockCageEdgeType        m_flockingCageEdgeType;                  // 48
	float                    m_flockingHeadingChangeRadian;           // 49
	float                    m_flockingHeadingChangeTime;             // 50

	void                     setEmitterLifeTime(float const min, float const max); // 51
	float                    getEmitterLifeTimeMin() const;
	float                    getEmitterLifeTimeMax() const;

	void                     setEmitterLodDistance(float const min, float const max); // 52
	void                     setEmitterLodDistanceNone();
	void                     setEmitterLodDistanceGlobal();
	float                    getEmitterLodDistanceMin() const;
	float                    getEmitterLodDistanceMax() const;
	bool                     isEmitterUsingGlobalLod() const;
	bool                     isEmitterUsingNoLod() const;

	void                     setSoundPath(std::string const &path);                // 53
	std::string const &      getSoundPath() const;

	float                    m_particleTimeOfDayColorPercent;                      // 54
	bool                     m_particleSnapToTerrainOnCreation;
	bool                     m_particleAlignToTerrainNormalOnCreation;
	float                    m_particleSnapToTerrainOnCreationHeight;
	float                    m_particleChangeDirectionRadian;
	float                    m_particleChangeDirectionTime;

	bool                     m_firstParticleImmediately;
	bool                     m_usePriorityParticles;

	static void setDefaultEmitterTranslationX(WaveForm &waveForm);
	static void setDefaultEmitterTranslationY(WaveForm &waveForm);
	static void setDefaultEmitterTranslationZ(WaveForm &waveForm);
	static void setDefaultEmitterRotationX(WaveForm &waveForm);
	static void setDefaultEmitterRotationY(WaveForm &waveForm);
	static void setDefaultEmitterRotationZ(WaveForm &waveForm);
	static void setDefaultEmitterDistance(WaveForm &waveForm);
	static void setDefaultEmitterShapeSize(WaveForm &waveForm);
	static void setDefaultEmitterSpread(WaveForm &waveForm);
	static void setDefaultParticleGenerationRate(WaveForm &waveForm);
	static void setDefaultParticleEmitSpeed(WaveForm &waveForm);
	static void setDefaultParticleInheritVelocityPercent(WaveForm &waveForm);
	static void setDefaultParticleClusterCount(WaveForm &waveForm);
	static void setDefaultParticleClusterRadius(WaveForm &waveForm);
	static void setDefaultParticleLifeTime(WaveForm &waveForm);
	static void setDefaultParticleWeight(WaveForm &waveForm);

private:

	SoundTemplate const *m_cachedSoundTemplate;
	std::string          m_soundPath;
	float                m_emitterLifeTimeMin;
	float                m_emitterLifeTimeMax;
	float                m_emitterLodDistanceMin;
	float                m_emitterLodDistanceMax;

	void load_0000(Iff &iff);
	void load_0001(Iff &iff);
	void load_0002(Iff &iff);
	void load_0003(Iff &iff);
	void load_0004(Iff &iff);
	void load_0005(Iff &iff);
	void load_0006(Iff &iff);
	void load_0007(Iff &iff);
	void load_0008(Iff &iff);
	void load_0009(Iff &iff);
	void load_0010(Iff &iff);
	void load_0011(Iff &iff);
	void load_0012(Iff &iff);
	void load_0013(Iff &iff);
	void load_0014(Iff &iff);
};

// ============================================================================

#endif // INCLUDED_ParticleEmitterDescription_H
