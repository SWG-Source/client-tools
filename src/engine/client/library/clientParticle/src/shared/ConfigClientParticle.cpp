// ============================================================================
//
// ConfigClientParticle.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ConfigClientParticle.h"

#include "clientParticle/ParticleAttachment.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/ParticleEmitter.h"
#include "clientParticle/ParticleEmitterGroup.h"
#include "clientParticle/ParticleManager.h"
#include "clientParticle/ParticleMesh.h"
#include "clientParticle/ParticleQuad.h"
#include "clientParticle/SwooshAppearance.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Os.h"
#include "sharedMath/WaveForm.h"

#include <algorithm>

// ============================================================================
//
// ConfigClientParticleNamespace
//
// ============================================================================

//-----------------------------------------------------------------------------
namespace ConfigClientParticleNamespace
{
	int s_maxQuadParticles = 2048;
	int s_maxMeshParticles = 512;
	int s_maxParticleAttachments = 1024;

	float s_minGlobalLodDistanceMin = 0.0f;
	float s_minGlobalLodDistanceMax = 4096.0f;
	float s_minGlobalLodDistanceDefault = 20.0f;
	float s_minGlobalLodDistance = s_minGlobalLodDistanceDefault;

	float s_maxGlobalLodDistanceMin = 10.0f;
	float s_maxGlobalLodDistanceMax = 4096.0f;
	float s_maxGlobalLodDistanceDefault = 200.0f;
	float s_maxGlobalLodDistance = s_maxGlobalLodDistanceDefault;
	float s_swooshCullDistance = 256.0f;
	int   s_particleUserLimit = s_maxQuadParticles;

#ifdef _DEBUG
	void showDebug();
	bool s_debug = false;
#endif // _DEBUG
}

using namespace ConfigClientParticleNamespace;

#ifdef _DEBUG
//-----------------------------------------------------------------------------
void ConfigClientParticleNamespace::showDebug()
{
	DEBUG_REPORT_PRINT(true, ("** Particle Debug Info **\n"));
	DEBUG_REPORT_PRINT(true, ("Particles Enabled       - %s\n", ParticleManager::isParticlesEnabled() ? "yes" : "no"));
	DEBUG_REPORT_PRINT(true, ("Total Particle Effects  - %d\n", ParticleEffectAppearance::getGlobalCount()));
	DEBUG_REPORT_PRINT(true, ("Total Emitter Groups    - %d\n", ParticleEmitterGroup::getGlobalCount()));
	DEBUG_REPORT_PRINT(true, ("Total Emitters          - %d\n", ParticleEmitter::getGlobalCount()));
	float const totalParticleQuadPercent = 100.0f * (static_cast<float>(ParticleQuad::getGlobalCount()) / static_cast<float>(ConfigClientParticle::getMaxQuadParticles()));
	DEBUG_REPORT_PRINT(true, ("Quad Particles          - %4d/%4d (%.0f%%)\n", ParticleQuad::getGlobalCount(), ConfigClientParticle::getMaxQuadParticles(), totalParticleQuadPercent));
	float const totalParticleMeshPercent = 100.0f * (static_cast<float>(ParticleMesh::getGlobalCount()) / static_cast<float>(ConfigClientParticle::getMaxMeshParticles()));
	DEBUG_REPORT_PRINT(true, ("Mesh Particles          - %4d/%4d (%.0f%%)\n", ParticleMesh::getGlobalCount(), ConfigClientParticle::getMaxMeshParticles(), totalParticleMeshPercent));
	float const totalParticleAttachmentPercent = 100.0f * (static_cast<float>(ParticleAttachment::getGlobalCount()) / static_cast<float>(ConfigClientParticle::getMaxParticleAttachments()));
	DEBUG_REPORT_PRINT(true, ("Particle Attachments    - %4d/%4d (%.0f%%)\n", ParticleAttachment::getGlobalCount(), ConfigClientParticle::getMaxParticleAttachments(), totalParticleAttachmentPercent));
	DEBUG_REPORT_PRINT(true, ("Extents Enabled         - %s\n", ParticleManager::isDebugExtentsEnabled() ? "yes" : "no"));
	DEBUG_REPORT_PRINT(true, ("Axis Enabled            - %s\n", ParticleManager::isDebugAxisEnabled() ? "yes" : "no"));
	DEBUG_REPORT_PRINT(true, ("Texturing Enabled       - %s\n", ParticleManager::isTexturingEnabled() ? "yes" : "no"));
	DEBUG_REPORT_PRINT(true, ("LOD Distance: min(%.2f) max(%.2f)\n", s_minGlobalLodDistance, s_maxGlobalLodDistance));
	float const waveFormSingleValueHitPercent = 100.0f * (static_cast<float>(WaveForm::getSingleValueHit()) / (static_cast<float>(WaveForm::getSingleValueHit() + WaveForm::getSingleValueMiss())));
	DEBUG_REPORT_PRINT(true, ("WaveForm: Single value hit   %.0f%% hit(%d) miss(%d)\n", waveFormSingleValueHitPercent, WaveForm::getSingleValueHit(), WaveForm::getSingleValueMiss()));
	float const waveFormLinearCalculationPercent = 100.0f * (static_cast<float>(WaveForm::getLinearCalculationCount()) / (static_cast<float>(WaveForm::getLinearCalculationCount() + WaveForm::getSplineCalculationCount())));
	DEBUG_REPORT_PRINT(true, ("WaveForm: Linear calculation %.0f%% linear(%d) spline(%d)\n", waveFormLinearCalculationPercent, WaveForm::getLinearCalculationCount(), WaveForm::getSplineCalculationCount()));
	DEBUG_REPORT_PRINT(true, ("TerrainHeightCallCount  - %d\n", ParticleEmitter::getTerrainHeightCallCount()));
	DEBUG_REPORT_PRINT(true, ("Swoosh count            - %d\n", SwooshAppearance::getGlobalCount()));
	DEBUG_REPORT_PRINT(true, ("Swoosh cull count       - %d\n", SwooshAppearance::getCullCountThisFrame()));
	Vector const &globalWind = ParticleEffectAppearance::getGlobalWind();
	DEBUG_REPORT_PRINT(true, ("Particle Wind           - (%.1f, %.1f, %.1f)\n", globalWind.x, globalWind.y, globalWind.z));

	ParticleManager::debugPrint();
}
#endif // _DEBUG

// ============================================================================
//
// ConfigClientParticle
//
// ============================================================================
#define KEY_INT(a,b)     (s_ ## a = ConfigFile::getKeyInt("ClientParticle", #a, b))
//-----------------------------------------------------------------------------
void ConfigClientParticle::install()
{
	// Enabled

	bool const particlesEnabled = !ConfigFile::getKeyBool("ClientParticle", "disable", false);

	ParticleManager::setParticlesEnabled(particlesEnabled);

	// Max particles

	s_maxQuadParticles = clamp(0, ConfigFile::getKeyInt("ClientParticle", "maxQuadParticles", s_maxQuadParticles), s_maxQuadParticles);
	s_maxMeshParticles = clamp(0, ConfigFile::getKeyInt("ClientParticle", "maxMeshParticles", s_maxMeshParticles), s_maxMeshParticles);
	s_maxParticleAttachments = clamp(0, ConfigFile::getKeyInt("ClientParticle", "maxParticleAttachments", s_maxParticleAttachments), s_maxParticleAttachments);

	// LOD distance

	setMinGlobalLodDistance(ConfigFile::getKeyFloat("ClientParticle", "minGlobalLodDistance", s_minGlobalLodDistanceDefault));
	setMaxGlobalLodDistance(ConfigFile::getKeyFloat("ClientParticle", "maxGlobalLodDistance", s_maxGlobalLodDistanceDefault));
	s_swooshCullDistance = ConfigFile::getKeyFloat("ClientParticle", "swooshCullDistance", s_swooshCullDistance);

	// Debug Text

#ifdef _DEBUG
	s_debug = ConfigFile::getKeyBool("ClientParticle", "debug", false);

	DebugFlags::registerFlag(s_debug, "ClientParticle", "debugInfo", showDebug);
#endif // _DEBUG

	KEY_INT(particleUserLimit, 2048);
}

//-----------------------------------------------------------------------------
int ConfigClientParticle::getMaxQuadParticles()
{
	return s_maxQuadParticles;
}

//-----------------------------------------------------------------------------
int ConfigClientParticle::getMaxMeshParticles()
{
	return s_maxMeshParticles;
}

//-----------------------------------------------------------------------------
int ConfigClientParticle::getMaxParticleAttachments()
{
	return s_maxParticleAttachments;
}

//-----------------------------------------------------------------------------
void ConfigClientParticle::setMinGlobalLodDistance(float const distance)
{
	s_minGlobalLodDistance = clamp(s_minGlobalLodDistanceMin, distance, s_minGlobalLodDistanceMax);

	if (s_minGlobalLodDistance > s_maxGlobalLodDistance)
	{
		std::swap(s_minGlobalLodDistance, s_maxGlobalLodDistance);
	}
}

//-----------------------------------------------------------------------------
float ConfigClientParticle::getMinGlobalLodDistance()
{
	return s_minGlobalLodDistance;
}

//-----------------------------------------------------------------------------
float ConfigClientParticle::getMinGlobalLodDistanceMin()
{
	return s_minGlobalLodDistanceMin;
}

//-----------------------------------------------------------------------------
float ConfigClientParticle::getMinGlobalLodDistanceMax()
{
	return s_minGlobalLodDistanceMax;
}

//-----------------------------------------------------------------------------
float ConfigClientParticle::getMinGlobalLodDistanceDefault()
{
	return s_minGlobalLodDistanceDefault;
}

//-----------------------------------------------------------------------------
void ConfigClientParticle::setMaxGlobalLodDistance(float const distance)
{
	s_maxGlobalLodDistance = clamp(s_maxGlobalLodDistanceMin, distance, s_maxGlobalLodDistanceMax);

	if (s_minGlobalLodDistance > s_maxGlobalLodDistance)
	{
		std::swap(s_minGlobalLodDistance, s_maxGlobalLodDistance);
	}
}

//-----------------------------------------------------------------------------
float ConfigClientParticle::getMaxGlobalLodDistance()
{
	return s_maxGlobalLodDistance;
}

//-----------------------------------------------------------------------------
float ConfigClientParticle::getMaxGlobalLodDistanceMin()
{
	return s_maxGlobalLodDistanceMin;
}

//-----------------------------------------------------------------------------
float ConfigClientParticle::getMaxGlobalLodDistanceMax()
{
	return s_maxGlobalLodDistanceMax;
}

//-----------------------------------------------------------------------------
float ConfigClientParticle::getMaxGlobalLodDistanceDefault()
{
	return s_maxGlobalLodDistanceDefault;
}

//-----------------------------------------------------------------------------
float ConfigClientParticle::getSwooshCullDistance()
{
	return s_swooshCullDistance;
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
bool ConfigClientParticle::isDebugEnabled()
{
	return s_debug;
}
#endif // _DEBUG

//-----------------------------------------------------------------------------

int ConfigClientParticle::getParticleUserLimit()
{
	return s_particleUserLimit;
}

// ============================================================================
