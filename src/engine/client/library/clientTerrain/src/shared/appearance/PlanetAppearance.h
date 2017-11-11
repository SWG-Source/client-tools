// ======================================================================
//
// PlanetAppearance.h
// copyright 2000-1, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_PlanetAppearance_H
#define INCLUDED_PlanetAppearance_H

// ======================================================================

#include "sharedObject/Appearance.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Transform.h"

class PlanetAppearanceTemplate;
class Shader;
class StaticVertexBuffer;

// ======================================================================

class PlanetAppearance : public Appearance
{
public:

	explicit PlanetAppearance(PlanetAppearanceTemplate const * planetAppearanceTemplate);
	virtual ~PlanetAppearance();

	virtual Sphere const & getSphere() const;
	virtual float alter(float elapsedTime);
	virtual void render() const;

	void setHaloRoll(float haloRoll);
	void setHaloScale(float haloScale);

private:

	typedef stdvector<StaticVertexBuffer *>::fwd VertexBufferList;

	class LocalShaderPrimitive;
	typedef stdvector<LocalShaderPrimitive *>::fwd LocalShaderPrimitiveList;

private:

	PlanetAppearance();
	PlanetAppearance(PlanetAppearance const &);
	PlanetAppearance & operator=(PlanetAppearance const &);

	virtual DPVS::Object * getDpvsObject() const;

	void createVertexBuffers(float radius, float uScaleMAIN, float vScaleMAIN, float uScaleDETA, float vScaleDETA, VertexBufferList * vertexBufferList) const;

private:

	PlanetAppearanceTemplate const * const m_planetAppearanceTemplate;

	Shader const * const m_surfaceShader;
	Transform m_surfaceTransform;
	VertexBufferList * m_surfaceVertexBufferList;
	LocalShaderPrimitiveList * const m_surfaceLocalShaderPrimitiveList;

	Shader const * const m_cloudShader;
	Transform m_cloudTransform;
	VertexBufferList * m_cloudVertexBufferList;
	LocalShaderPrimitiveList * m_cloudLocalShaderPrimitiveList;

	Shader const * const m_haloShader;
	StaticVertexBuffer * m_haloVertexBuffer;
	class LocalShaderPrimitiveHalo;
	LocalShaderPrimitiveHalo * m_localShaderPrimitiveHalo;

	Sphere m_sphere;

	DPVS::Object * m_dpvsObject;
};

// ======================================================================

#endif
