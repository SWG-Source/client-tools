// ======================================================================
//
// Light.h
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Light_H
#define INCLUDED_Light_H

// ======================================================================

#include "sharedObject/Object.h"
#include "sharedMath/VectorArgb.h"

#include <vector>

namespace DPVS
{
	class RegionOfInfluence;
};

// ======================================================================

// Light emitter for vertex-based lighting

class Light : public Object
{
public:

	enum Type
	{
		T_ambient,
		T_parallel,
		T_OBSOLETE_parallelPoint,
		T_point,
		T_point_multicell,
		T_spot
	};

public:

	Light(Type newType, const VectorArgb &newColor);
	virtual ~Light(void);

	virtual void      addToWorld();
	virtual void      removeFromWorld();
	virtual void      setRegionOfInfluenceEnabled(bool enabled) const;

	virtual float     alter( float elapsedTime );

	Type              getType(void) const;	

	bool              hasDiffuseColor() const;
	const VectorArgb &getDiffuseColor() const;
	void              setDiffuseColor(const VectorArgb &newColor);
	float             getDiffuseIntensity() const;
	void              setDiffuseColorScale(float diffuseColorScale);

	const VectorArgb &getDiffuseBackColor() const;
	void              setDiffuseBackColor(const VectorArgb &newColor);
	void              setDiffuseBackColorScale(float diffuseColorScale);
	
	const VectorArgb &getDiffuseTangentColor() const;
	void              setDiffuseTangentColor(const VectorArgb &newColor);
	void              setDiffuseTangentColorScale(float diffuseColorScale);

	const VectorArgb &getScaledDiffuseColor() const;
	const VectorArgb &getScaledDiffuseBackColor() const;
	const VectorArgb &getScaledDiffuseTangentColor() const;
	float             getScaledDiffuseIntensity() const;

	bool              hasSpecularColor() const;
	const VectorArgb &getSpecularColor() const;
	void              setSpecularColor(const VectorArgb &newSpecularColor);
	float             getSpecularIntensity() const;

	void              setSpecularColorScale(float specularColorScale);
	const VectorArgb &getScaledSpecularColor() const;
	float             getScaledSpecularIntensity() const;

	real              getRange(void) const;
	void              setRange(real newRange);

	real              getConstantAttenuation(void) const;
	real              getLinearAttenuation(void) const;
	real              getQuadraticAttenuation(void) const;
	void              setConstantAttenuation(real newConstantAttenuation);
	void              setLinearAttenuation(real newLinearAttenuation);
	void              setQuadraticAttenuation(real newQuadraticAttenuation);

	real              getSpotTheta(void) const;
	real              getSpotPhi(void) const;
	real              getSpotFallOff(void) const;
	void              setSpotTheta(real newTheta);
	void              setSpotPhi(real newPhi);
	void              setSpotFallOff(real newFallOff);

	void              setAffectsShadersWithoutPrecalculatedVertexLighting(bool affectsShadersWithoutPrecalculatedVertexLighting);
	bool              affectsShadersWithoutPrecalculatedVertexLighting() const;

	void              setAffectsShadersWithPrecalculatedVertexLighting(bool affectsShadersWithPrecalculatedVertexLighting);
	bool              affectsShadersWithPrecalculatedVertexLighting() const;

	int               getShaderPrimitiveSorterLightIndex() const;
	void              setShaderPrimitiveSorterLightIndex(int shaderPrimitiveSorterLightIndex) const;

private:

	Light(void);
	Light(const Light &);
	Light &operator =(const Light &);

	void updateGhostLights(void);

private:

	// all lights
	const Type               m_type;

	VectorArgb               m_diffuseColor;
	float                    m_diffuseIntensity;
	float                    m_diffuseColorScale;
	VectorArgb               m_diffuseColorScaled;
	float                    m_diffuseIntensityScaled;
	
	VectorArgb               m_diffuseBackColor;
	float                    m_diffuseBackColorScale;
	VectorArgb               m_diffuseBackColorScaled;

	VectorArgb               m_diffuseTangentColor;
	float                    m_diffuseTangentColorScale;
	VectorArgb               m_diffuseTangentColorScaled;

	VectorArgb               m_specularColor;
	float                    m_specularIntensity;
	float                    m_specularColorScale;
	VectorArgb               m_specularColorScaled;
	float                    m_specularIntensityScaled;

	// point, spot
	real                     m_range;
	real                     m_constantAttenuation;
	real                     m_linearAttenuation;
	real                     m_quadraticAttenuation;

	// spot
	real                     m_theta;
	real                     m_phi;
	real                     m_fallOff;

	DPVS::RegionOfInfluence *m_dpvsRegionOfInfluence;

	typedef std::vector<Light *> LightVector;
	LightVector              m_ghostLights;
	bool                     m_ghostLightsDirty;

	bool                     m_affectsShadersWithoutPrecalculatedVertexLighting;
	bool                     m_affectsShadersWithPrecalculatedVertexLighting;

	mutable int              m_shaderPrimitiveSorterLightIndex;
};

// ======================================================================
/**
 * Get the type of the light.
 * 
 * @return The type of the light
 */

inline Light::Type Light::getType(void) const
{
	return m_type;
}

// ----------------------------------------------------------------------
/**
 * Get the color of the light.
 * 
 * @return The color of the light
 */

inline const VectorArgb &Light::getDiffuseColor() const
{
	return m_diffuseColor; 
}

// ----------------------------------------------------------------------

inline const VectorArgb &Light::getDiffuseBackColor() const
{
	return m_diffuseBackColor; 
}

inline const VectorArgb &Light::getDiffuseTangentColor() const
{
	return m_diffuseTangentColor; 
}

// ----------------------------------------------------------------------

inline float Light::getDiffuseIntensity() const
{
	return m_diffuseIntensity; 
}

// ----------------------------------------------------------------------

inline const VectorArgb &Light::getScaledDiffuseColor() const
{
	return m_diffuseColorScaled; 
}

// ----------------------------------------------------------------------

inline const VectorArgb &Light::getScaledDiffuseBackColor() const
{
	return m_diffuseBackColorScaled; 
}

// ----------------------------------------------------------------------

inline const VectorArgb &Light::getScaledDiffuseTangentColor() const
{
	return m_diffuseTangentColorScaled; 
}

// ----------------------------------------------------------------------

inline float Light::getScaledDiffuseIntensity() const
{
	return m_diffuseIntensityScaled; 
}

// ----------------------------------------------------------------------
/**
 * Set the new color of the light.
 * 
 * Do not change the color of a light in the middle of a scene.
 * 
 * @param color  New color for the light
 */

// ----------------------------------------------------------------------
inline void Light::setDiffuseColor(const VectorArgb &diffuseColor)
{
	if (diffuseColor != m_diffuseColor)
	{
		m_diffuseColor           = diffuseColor;
		m_diffuseIntensity       = diffuseColor.rgbIntensity();
		m_diffuseColorScaled     = m_diffuseColor * m_diffuseColorScale;
		m_diffuseIntensityScaled = m_diffuseColorScaled.rgbIntensity();

		m_ghostLightsDirty = true;
	}
}

// ----------------------------------------------------------------------

inline void Light::setDiffuseColorScale(const float diffuseColorScale)
{
	if (diffuseColorScale != m_diffuseColorScale)
	{
		m_diffuseColorScale      = diffuseColorScale;
		m_diffuseColorScaled     = m_diffuseColor * m_diffuseColorScale;
		m_diffuseIntensityScaled = m_diffuseColorScaled.rgbIntensity();

		m_ghostLightsDirty = true;
	}
}

// ----------------------------------------------------------------------
inline void Light::setDiffuseBackColor(const VectorArgb &newColor)
{
	if (newColor != m_diffuseBackColor)
	{
		m_diffuseBackColor           = newColor;
		m_diffuseBackColorScaled     = m_diffuseBackColor * m_diffuseBackColorScale;

		m_ghostLightsDirty = true;
	}
}

// ----------------------------------------------------------------------

inline void Light::setDiffuseBackColorScale(const float diffuseColorScale)
{
	if (diffuseColorScale != m_diffuseColorScale)
	{
		m_diffuseBackColorScale      = diffuseColorScale;
		m_diffuseBackColorScaled     = m_diffuseBackColor * m_diffuseBackColorScale;

		m_ghostLightsDirty = true;
	}
}

// ----------------------------------------------------------------------
inline void Light::setDiffuseTangentColor(const VectorArgb &newColor)
{
	if (newColor != m_diffuseTangentColor)
	{
		m_diffuseTangentColor         = newColor;
		m_diffuseTangentColorScaled   = m_diffuseTangentColor * m_diffuseTangentColorScale;

		m_ghostLightsDirty = true;
	}
}

// ----------------------------------------------------------------------

inline void Light::setDiffuseTangentColorScale(const float diffuseColorScale)
{
	if (diffuseColorScale != m_diffuseColorScale)
	{
		m_diffuseTangentColorScale    = diffuseColorScale;
		m_diffuseTangentColorScaled   = m_diffuseTangentColor * m_diffuseTangentColorScale;

		m_ghostLightsDirty = true;
	}
}

// ----------------------------------------------------------------------
/**
 * Get the specular color of the light.
 * 
 * @return The specular color of the light
 */

inline const VectorArgb &Light::getSpecularColor() const
{
	return m_specularColor; 
}

// ----------------------------------------------------------------------

inline float Light::getSpecularIntensity() const
{
	return m_specularIntensity; 
}

// ----------------------------------------------------------------------

inline const VectorArgb &Light::getScaledSpecularColor() const
{
	return m_specularColorScaled; 
}

// ----------------------------------------------------------------------

inline float Light::getScaledSpecularIntensity() const
{
	return m_specularIntensityScaled; 
}

// ----------------------------------------------------------------------
/**
 * Set the new specular color of the light.
 * 
 * Do not change the specular color of a light in the middle of a scene.
 * 
 * @param newSpecularColor  New color for the light
 */

inline void Light::setSpecularColor(const VectorArgb &specularColor)
{
	if (specularColor != m_specularColor)
	{
		m_specularColor           = specularColor;
		m_specularIntensity       = m_specularColor.rgbIntensity();
		m_specularColorScaled     = m_specularColor * m_specularColorScale;
		m_specularIntensityScaled = m_specularColorScaled.rgbIntensity();

		m_ghostLightsDirty = true;
	}
}

// ----------------------------------------------------------------------

inline void Light::setSpecularColorScale(const float specularColorScale)
{
	if (m_specularColorScale != specularColorScale)
	{
		m_specularColorScale      = specularColorScale;
		m_specularColorScaled     = m_specularColor * m_specularColorScale;
		m_specularIntensityScaled = m_specularColorScaled.rgbIntensity();

		m_ghostLightsDirty = true;
	}
}

// ----------------------------------------------------------------------
/**
 * Get the range of the light.
 * 
 * This value is only valid for parallelPoint, point, and spot lights.
 * 
 * @return The range of the light
 */

inline real Light::getRange(void) const
{
	return m_range;
}

// ----------------------------------------------------------------------
/**
 * Get the constant attenuation for the light.
 * 
 * This value is only valid for parallelPoint, point, and spot lights.
 */

inline real Light::getConstantAttenuation(void) const
{
	return m_constantAttenuation;
}

// ----------------------------------------------------------------------
/**
 * Get the linear attenuation for the light.
 * 
 * This value is only valid for parallelPoint, point, and spot lights.
 */

inline real Light::getLinearAttenuation(void) const
{
	return m_linearAttenuation;
}

// ----------------------------------------------------------------------
/**
 * Get the quadratic attenuation for the light.
 * 
 * This value is only valid for parallelPoint, point, and spot lights.
 */

inline real Light::getQuadraticAttenuation(void) const
{
	return m_quadraticAttenuation;
}

// ----------------------------------------------------------------------
/**
 * Set the constant attenuation for the light.
 * 
 * Do not change this value in the middle of a scene.
 * 
 * This value is only valid for parallelPoint, point, and spot lights.
 * 
 * @param constantAttenuation  New value for the constant attenuation of the light
 */

inline void Light::setConstantAttenuation(real constantAttenuation)
{
	if (constantAttenuation != m_constantAttenuation)
	{
		m_constantAttenuation = constantAttenuation;

		m_ghostLightsDirty = true;
	}
}

// ----------------------------------------------------------------------
/**
 * Set the linear attenuation for the light.
 * 
 * Do not change this value in the middle of a scene.
 * 
 * This value is only valid for parallelPoint, point, and spot lights.
 * 
 * @param linearAttenuation  New value for the linear attenuation of the light
 */

inline void Light::setLinearAttenuation(real linearAttenuation)
{
	if (linearAttenuation != m_linearAttenuation)
	{
		m_linearAttenuation = linearAttenuation;

		m_ghostLightsDirty = true;
	}
}

// ----------------------------------------------------------------------
/**
 * Set the quadratic attenuation for the light.
 * 
 * Do not change this value in the middle of a scene.
 * 
 * This value is only valid for parallelPoint, point, and spot lights.
 */

inline void Light::setQuadraticAttenuation(real quadraticAttenuation)
{
	if (quadraticAttenuation != m_quadraticAttenuation)
	{
		m_quadraticAttenuation = quadraticAttenuation;

		m_ghostLightsDirty = true;
	}
}

// ----------------------------------------------------------------------
/**
 * Get the angle to the inner code.
 * 
 * Vertices inside the inner code are not affected by fall-off.
 * 
 * This value is only valid for spot lights.
 */

inline real Light::getSpotTheta(void) const
{
	return m_theta;
}

// ----------------------------------------------------------------------
/**
 * Get the angle to the outer code.
 * 
 * Vertices outside the outer code are not affected by the spot light.
 * 
 * This value is only valid for spot lights.
 */

inline real Light::getSpotPhi(void) const
{
	return m_phi;
}

// ----------------------------------------------------------------------
/**
 * Get the factor affecting light fall-off from the inner code to the outer code.
 * 
 * See the D3D docs for details on this value.
 * 
 * This value is only valid for spot lights.
 */

inline real Light::getSpotFallOff(void) const
{
	return m_fallOff;
}

// ----------------------------------------------------------------------
/**
 * Set the angle to the inner code.
 * 
 * Do not change this value in the middle of a scene.
 * 
 * Vertices inside the inner code are not affected by fall-off.
 * 
 * This value is only valid for spot lights.
 */

inline void Light::setSpotTheta(real theta)
{
	m_theta = theta;
}

// ----------------------------------------------------------------------
/**
 * Get the angle to the outer code.
 * 
 * Do not change this value in the middle of a scene.
 * 
 * Vertices outside the outer code are not affected by the spot light.
 * 
 * This value is only valid for spot lights.
 */

inline void Light::setSpotPhi(real phi)
{
	m_phi = phi;
}

// ----------------------------------------------------------------------
/**
 * Set the fall-off value.
 * 
 * Do not change this value in the middle of a scene.
 * 
 * See the D3D docs for details on this value.
 * 
 * This value is only valid for spot lights.
 */

inline void Light::setSpotFallOff(real fallOff)
{
	m_fallOff = fallOff;
}

// ----------------------------------------------------------------------

inline void Light::setAffectsShadersWithoutPrecalculatedVertexLighting(bool affectsShadersWithoutPrecalculatedVertexLighting)
{
	if (affectsShadersWithoutPrecalculatedVertexLighting != m_affectsShadersWithoutPrecalculatedVertexLighting)
	{
		m_affectsShadersWithoutPrecalculatedVertexLighting = affectsShadersWithoutPrecalculatedVertexLighting;

		m_ghostLightsDirty = true;
	}
}

// ----------------------------------------------------------------------

inline bool Light::affectsShadersWithoutPrecalculatedVertexLighting() const
{
	return m_affectsShadersWithoutPrecalculatedVertexLighting;
}

// ----------------------------------------------------------------------

inline void Light::setAffectsShadersWithPrecalculatedVertexLighting(bool affectsShadersWithPrecalculatedVertexLighting)
{
	if (affectsShadersWithPrecalculatedVertexLighting != m_affectsShadersWithPrecalculatedVertexLighting)
	{
		m_affectsShadersWithPrecalculatedVertexLighting = affectsShadersWithPrecalculatedVertexLighting;

		m_ghostLightsDirty = true;
	}
}

// ----------------------------------------------------------------------

inline bool Light::affectsShadersWithPrecalculatedVertexLighting() const
{
	return m_affectsShadersWithPrecalculatedVertexLighting;
}

// ----------------------------------------------------------------------
/**
 * @internal
 */

inline int Light::getShaderPrimitiveSorterLightIndex() const
{
	return m_shaderPrimitiveSorterLightIndex;
}

// ----------------------------------------------------------------------
/**
 * @internal
 */

inline void Light::setShaderPrimitiveSorterLightIndex(int shaderPrimitiveSorterLightIndex) const
{
	m_shaderPrimitiveSorterLightIndex = shaderPrimitiveSorterLightIndex;
}

// ----------------------------------------------------------------------

inline bool Light::hasDiffuseColor() const
{
	return m_diffuseColor != VectorArgb::solidBlack;
}

// ----------------------------------------------------------------------

inline bool Light::hasSpecularColor() const
{
	return m_specularColor != VectorArgb::solidBlack;
}

// ======================================================================

#endif
