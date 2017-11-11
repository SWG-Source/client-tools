// ======================================================================
//
// SpaceEnvironment.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SpaceEnvironment_H
#define INCLUDED_SpaceEnvironment_H

// ======================================================================

#include "clientTerrain/ClientSpaceTerrainAppearance.h"
#include "sharedMath/PackedRgb.h"

class Light;
class Camera;

// ======================================================================

class SpaceEnvironment
{
public:

	typedef stdvector<Light *>::fwd LightVector;

public:

	explicit SpaceEnvironment(ClientSpaceTerrainAppearance const & clientSpaceTerrainAppearance);
	~SpaceEnvironment();

	void render() const;

	PackedRgb const & getClearColor() const;

	LightVector const & getLightVector() const;
	LightVector const & getParallelLights() const;

	void addEnvironmentObject(Object & obj);

	void disableEnvironmentForHyperspace();
	static void setCamera(Camera * camera);
	static void alter(float elapsedTime);

private:

	SpaceEnvironment();
	SpaceEnvironment(SpaceEnvironment const &);
	SpaceEnvironment & operator=(SpaceEnvironment const &);

	void alterInternal(float elapsedTime);

private:

	ClientSpaceTerrainAppearance const & m_clientSpaceTerrainAppearance;

	PackedRgb m_clearColor;

	LightVector * const m_lightVector;
	LightVector * m_parallelLightVector;

	typedef stdvector<Object *>::fwd ObjectVector;
	ObjectVector * const m_objectVector;
	ObjectVector * const m_objectsFollowCameraVector;
	ObjectVector * const m_objectsToDisableForHyperspace;
};

// ======================================================================

#endif
