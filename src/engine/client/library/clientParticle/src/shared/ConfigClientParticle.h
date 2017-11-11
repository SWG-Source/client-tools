//=============================================================================
//
// ConfigClientParticle.h
// Copyright Sony Online Entertainment
//
//=============================================================================

#ifndef INCLUDED_ConfigClientParticle_H
#define INCLUDED_ConfigClientParticle_H

//-----------------------------------------------------------------------------
class ConfigClientParticle
{
public:

	static void  install();

	static void  setMinGlobalLodDistance(float const distance);
	static float getMinGlobalLodDistance();
	static float getMinGlobalLodDistanceMin();
	static float getMinGlobalLodDistanceMax();
	static float getMinGlobalLodDistanceDefault();

	static void  setMaxGlobalLodDistance(float const distance);
	static float getMaxGlobalLodDistance();
	static float getMaxGlobalLodDistanceMin();
	static float getMaxGlobalLodDistanceMax();
	static float getMaxGlobalLodDistanceDefault();

	static int getMaxQuadParticles();
	static int getMaxMeshParticles();
	static int getMaxParticleAttachments();
	static float getSwooshCullDistance();

	static int getParticleUserLimit();

#ifdef _DEBUG
	static bool isDebugEnabled();
#endif // _DEBUG

private:

	// Disabled

	ConfigClientParticle();
	ConfigClientParticle(ConfigClientParticle const &);
	ConfigClientParticle& operator=(ConfigClientParticle const &);
};

//=============================================================================

#endif // INCLUDED_ConfigClientParticle_H
