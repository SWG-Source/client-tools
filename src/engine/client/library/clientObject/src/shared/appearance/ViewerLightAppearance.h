//===================================================================
//
// ViewerLightAppearance.h
// asommers
//
// copyright 1998 Bootprint Entertainment
// copyright 2000-01, sony online entertainment
// All Rights Reserved
//
//===================================================================

#ifndef INCLUDED_ViewerLightAppearance_H
#define INCLUDED_ViewerLightAppearance_H

//===================================================================

#include "sharedObject/Appearance.h"
#include "sharedMath/Sphere.h"

class Light;
class ShaderPrimitive;

//===================================================================

class ViewerLightAppearance : public Appearance
{
public:

	ViewerLightAppearance (const Light* newLight, real newLength);
	virtual ~ViewerLightAppearance (void);

	virtual const Sphere& getSphere (void) const;
	virtual void          render () const;

	static bool getShowLights (void);
	static void setShowLights (bool newShowLights);

protected:

	DPVS::Object * getDpvsObject() const;

private:

	ViewerLightAppearance (void);
	ViewerLightAppearance (const ViewerLightAppearance&);
	ViewerLightAppearance& operator= (const ViewerLightAppearance&);

private:

	class LocalShaderPrimitiveParallel;
	class LocalShaderPrimitivePoint;
	class LocalShaderPrimitiveSpot;

	ShaderPrimitive* m_localShaderPrimitive;
	Sphere           m_sphere;
	DPVS::Object*    m_dpvsObject;
};

//===================================================================

#endif

