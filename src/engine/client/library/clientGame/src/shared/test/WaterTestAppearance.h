// ======================================================================
//
// WaterTestAppearance.h
// Copyright 2000, 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_WaterTestAppearance_H
#define INCLUDED_WaterTestAppearance_H

// ======================================================================

#include "sharedObject/Appearance.h"

class Camera;
class Object;
class Shader;
class StaticVertexBuffer;
class VectorArgb;

// ======================================================================

class WaterTestAppearance : public Appearance
{
public:

	WaterTestAppearance (float newRadiusX, float newRadiusY, Shader* newShader, float uvScale=2.f);
	virtual ~WaterTestAppearance ();

	virtual const Sphere&     getSphere () const;
	virtual void              render () const;
	virtual float             alter (float time);

	const Shader&             getShader () const;
	const StaticVertexBuffer& getVertexBuffer () const;
	void                      setColor (const VectorArgb& newColor);

protected:

	DPVS::Object*             getDpvsObject() const;

private:

	class LocalShaderPrimitive;

private:

	Shader*               m_shader;
	StaticVertexBuffer*   m_vertexBuffer;
	float                 m_radiusX;
	float                 m_radiusY;
	float                 m_uvScale;

	LocalShaderPrimitive* m_shaderPrimitive;
	DPVS::Object*         m_dpvsObject;

private:

	WaterTestAppearance ();
	WaterTestAppearance (const WaterTestAppearance&);
	WaterTestAppearance& operator= (const WaterTestAppearance&);
};

// ======================================================================

inline const Shader& WaterTestAppearance::getShader () const
{
	NOT_NULL(m_shader);
	return *m_shader;
}

// ----------------------------------------------------------------------

inline const StaticVertexBuffer& WaterTestAppearance::getVertexBuffer () const
{
	return *m_vertexBuffer;
}

// ======================================================================

#endif
