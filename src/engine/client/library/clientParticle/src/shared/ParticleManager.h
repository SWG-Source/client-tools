// ============================================================================
//
// ParticleManager.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleManager_H
#define INCLUDED_ParticleManager_H

class ParticleEffectAppearance;

//-----------------------------------------------------------------------------
class ParticleManager
{
public:

	static void install();

#ifdef _DEBUG
	static void debugRegister(ParticleEffectAppearance const & particleEffectAppearance);
	static void debugUnRegister(ParticleEffectAppearance const & particleEffectAppearance);
	static void debugPrint();
#endif // _DEBUG

	static void setParticlesEnabled(bool const particlesEnabled);
	static bool isParticlesEnabled();

	static void setTexturingEnabled(bool const texturingEnabled);
	static bool isTexturingEnabled();

	static void setDebugParticleOrientationEnabled(bool const debugParticleOrientationEnabled);
	static bool isDebugParticleOrientationEnabled();

	static void setDebugVelocityEnabled(bool const debugVelocityEnabled);
	static bool isDebugVelocityEnabled();

	static void setDebugExtentsEnabled(bool const debugExtentsEnabled);
	static bool isDebugExtentsEnabled();

	static void setDebugAxisEnabled(bool const debugAxisEnabled);
	static bool isDebugAxisEnabled();

	static void setDebugWorldTextEnabled(bool const debugWorldEnabled);
	static bool isDebugWorldTextEnabled();

	static void setDebugFlockingCageEnabled(bool const debugFlockingCageEnabled);
	static bool isDebugFlockingCageEnabled();

	static void setDebugShowSwooshQuadsEnabled(bool const enabled);
	static bool isDebugShowSwooshQuadsEnabled();

	static void setDebugOriginIconEnabled(bool const enabled);
	static bool isDebugOriginIconEnabled();

private:

	// Disabled

	ParticleManager();
	ParticleManager(ParticleManager const &);
	ParticleManager & operator=(ParticleManager const &);
};

//=============================================================================

#endif // INCLUDED_ParticleManager_H
