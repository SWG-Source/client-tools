//
// CelestialAppearance.h
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2000-2001 Sony Online Entertainment Inc.
// All Rights Reserved.

// ----------------------------------------------------------------------

#ifndef INCLUDED_CelestialAppearance_H
#define INCLUDED_CelestialAppearance_H

// ----------------------------------------------------------------------

#include "sharedObject/Appearance.h"
#include "sharedMath/VectorArgb.h"

// ----------------------------------------------------------------------

class CelestialAppearance : public Appearance
{
public:

	CelestialAppearance(float newWidth, float newHeight, float newRoll, const char* shaderTemplateName, const VectorArgb &newColor=VectorArgb::solidWhite);
	virtual ~CelestialAppearance();

	virtual void          render() const;
	virtual float         alter(float time);
	virtual void          setAlpha(bool opaqueEnabled, float opaqueAlpha, bool alphaEnabled, float alphaAlpha);

	void                  setColor(const VectorArgb& newColor);
	void                  setWidth(float newWidth);
	void                  setLength(float newLength);

protected:
	
	DPVS::Object*         getDpvsObject() const;

private:

	class LocalShaderPrimitive;
	LocalShaderPrimitive* m_localShaderPrimitive;
	DPVS::Object*         m_dpvsObject;

private:

	CelestialAppearance();
	CelestialAppearance(const CelestialAppearance&);
	CelestialAppearance& operator=(const CelestialAppearance&);
};

//-------------------------------------------------------------------

#endif
