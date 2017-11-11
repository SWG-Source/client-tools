// ======================================================================
//
// ShaderAppearance.h
// Copyright 2000, 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_ShaderAppearance_H
#define INCLUDED_ShaderAppearance_H

// ======================================================================

#include "sharedMath/VectorArgb.h"
#include "sharedObject/Appearance.h"

class Shader;

// ======================================================================

class ShaderAppearance : public Appearance
{
public:

	ShaderAppearance (float newRadiusX, float newRadiusY, Shader* newShader, const VectorArgb& newColor, float uvScale=2.f);
	virtual ~ShaderAppearance ();

	virtual void              setAlpha (bool opaqueEnabled, float opaqueAlpha, bool alphaEnabled, float alphaAlpha);
	virtual const Sphere&     getSphere () const;
	virtual void              render () const;
	virtual float             alter (float time);

	void                      decreaseIgnoreMipmapLevelCount ();
	void                      increaseIgnoreMipmapLevelCount ();
	void                      resetIgnoreMipmapLevelCount ();
	int                       getIgnoreMipmapLevelCount () const;

	const Shader&             getShader () const;
	VectorArgb const &        getColor () const;
	void                      setColor (const VectorArgb& newColor);
	float                     getRadiusX () const;
	float                     getRadiusY () const;
	float                     getUvScale () const;

protected:

	DPVS::Object*             getDpvsObject() const;

private:

	class LocalShaderPrimitive;

private:

	int                   m_ignoreMipmapLevelCount;

	Shader * const        m_shader;
	VectorArgb            m_color;
	float                 m_radiusX;
	float                 m_radiusY;
	float                 m_uvScale;

	LocalShaderPrimitive* m_shaderPrimitive;
	DPVS::Object*         m_dpvsObject;

private:

	ShaderAppearance ();
	ShaderAppearance (const ShaderAppearance&);
	ShaderAppearance& operator= (const ShaderAppearance&);
};

// ======================================================================

inline void ShaderAppearance::decreaseIgnoreMipmapLevelCount ()
{
	m_ignoreMipmapLevelCount--;
	m_ignoreMipmapLevelCount = clamp (0, m_ignoreMipmapLevelCount, 9999);
}

// ----------------------------------------------------------------------

inline void ShaderAppearance::increaseIgnoreMipmapLevelCount ()
{
	m_ignoreMipmapLevelCount++;
	m_ignoreMipmapLevelCount = clamp (0, m_ignoreMipmapLevelCount, 9999);
}

// ----------------------------------------------------------------------

inline void ShaderAppearance::resetIgnoreMipmapLevelCount ()
{
	m_ignoreMipmapLevelCount = 0;
}

// ----------------------------------------------------------------------

inline int ShaderAppearance::getIgnoreMipmapLevelCount () const
{
	return m_ignoreMipmapLevelCount;
}

// ----------------------------------------------------------------------

inline const Shader& ShaderAppearance::getShader () const
{
	NOT_NULL(m_shader);
	return *m_shader;
}

// ----------------------------------------------------------------------

inline VectorArgb const & ShaderAppearance::getColor () const
{
	return m_color;
}

// ----------------------------------------------------------------------

inline float ShaderAppearance::getRadiusX () const
{
	return m_radiusX;
}

// ----------------------------------------------------------------------

inline float ShaderAppearance::getRadiusY () const
{
	return m_radiusY;
}

// ----------------------------------------------------------------------

inline float ShaderAppearance::getUvScale () const
{
	return m_uvScale;
}

// ======================================================================

#endif
