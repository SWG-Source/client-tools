// ======================================================================
//
// GridAppearance.h
// Portions Copyright 1999, Bootprint Entertainment.
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_GridAppearance_H
#define INCLUDED_GridAppearance_H

// ======================================================================

#include "sharedObject/Appearance.h"

class Shader;
class ShaderTemplate;
class Sphere;
class Vector;
class VectorArgb;

// ======================================================================

class GridAppearance : public Appearance
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	enum GridPlane
	{
		GP_XZ,
		GP_XY,
		GP_YZ
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		enum
	{
		UL,
		UR,
		LL,
		LR
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
public:

	GridAppearance (GridPlane newGridPlane, real newSize, int newNumSubdivisions, const VectorArgb& newColor);
	virtual ~GridAppearance ();

	virtual const Sphere& getSphere () const;
	virtual void          render () const;

	real  getSize () const;
	int   getSubdivisions () const;
	void  resize (GridPlane newGridPlane, real newSize);
	void  setSubdivisions (int newSubdivisions);
	void  incrementSubdivisions ();
	void  decrementSubdivisions ();

	float getStep () const;

	const VectorArgb& getColor () const;
	const Vector*     getVertexList () const;

protected:

	DPVS::Object* getDpvsObject() const;

private:

	class LocalShaderPrimitive;

private:

	real                  m_size;
	Vector               *m_vList;
	int                   m_numSubdivisions;

	Sphere               *m_sphere;

	VectorArgb           *m_color;

	LocalShaderPrimitive *m_shaderPrimitive;

	DPVS::Object         *m_dpvsObject;

private:

	GridAppearance (void);
	GridAppearance (const GridAppearance&);
	GridAppearance& operator= (const GridAppearance&);
};

// ----------------------------------------------------------------------

inline real GridAppearance::getSize (void) const
{
	return m_size;
}

// ----------------------------------------------------------------------

inline int GridAppearance::getSubdivisions (void) const
{
	return m_numSubdivisions;
}

// ----------------------------------------------------------------------

inline void GridAppearance::setSubdivisions (int newSubdivisions)
{
	m_numSubdivisions = newSubdivisions;
}

// ----------------------------------------------------------------------

inline void GridAppearance::incrementSubdivisions (void)
{
	m_numSubdivisions++;
}

// ----------------------------------------------------------------------

inline void GridAppearance::decrementSubdivisions (void)
{
	m_numSubdivisions--;
	if (m_numSubdivisions < 0)
		m_numSubdivisions = 0;
}

// ----------------------------------------------------------------------

inline float GridAppearance::getStep (void) const
{
	return 2 * m_size / (m_numSubdivisions + 1);
}

// ----------------------------------------------------------------------

inline const VectorArgb& GridAppearance::getColor () const
{
	return *m_color;
}

// ----------------------------------------------------------------------

inline const Vector* GridAppearance::getVertexList () const
{
	return m_vList;
}

// ======================================================================

#endif
