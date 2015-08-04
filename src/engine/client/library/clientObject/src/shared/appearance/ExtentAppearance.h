//======================================================================
//
// ExtentAppearance.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ExtentAppearance_H
#define INCLUDED_ExtentAppearance_H

// ======================================================================

#include "sharedObject/Appearance.h"

class Extent;
class Sphere;
class VectorArgb;

// ======================================================================

class ExtentAppearance : public Appearance
{
public:

	// extent is now owned by ExtentAppearance!!!
	ExtentAppearance(Extent * extent, VectorArgb const & color);
	virtual ~ExtentAppearance();

	virtual Sphere const & getSphere () const;
	virtual void render() const;

	Extent const * getExtentToRender() const;
	Extent * getExtentToRender();
	VectorArgb const & getColor() const;

protected:
	DPVS::Object* getDpvsObject() const;

private:
	class LocalShaderPrimitive;

private:
	LocalShaderPrimitive * m_shaderPrimitive;
	DPVS::Object * m_dpvsObject;
	Extent * m_extentToRender;
	VectorArgb * m_color;
	Sphere * m_sphere;

private:
	ExtentAppearance();
	ExtentAppearance(const ExtentAppearance&);
	ExtentAppearance & operator=(const ExtentAppearance&);
};

// ======================================================================

#endif
