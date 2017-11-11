// ======================================================================
//
// GridAppearance.h
// Portions Copyright 1999, Bootprint Entertainment.
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/GridAppearance.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Vector.h"
#include "sharedMath/VectorArgb.h"

#include "dpvsModel.hpp"
#include "dpvsObject.hpp"

// ======================================================================

class GridAppearance::LocalShaderPrimitive: public ShaderPrimitive
{
public:

	explicit LocalShaderPrimitive (GridAppearance &appearance);
	virtual ~LocalShaderPrimitive ();

	virtual const Vector        getPosition_w () const;
	virtual const StaticShader &prepareToView () const;
	virtual float               getDepthSquaredSortKey () const;
	virtual int                 getVertexBufferSortKey () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

private:

	GridAppearance &m_appearance;

private:
	// disabled
	LocalShaderPrimitive ();
	LocalShaderPrimitive& operator = (const LocalShaderPrimitive&); //lint -esym(754, LocalShaderPrimitive::operator=) // not referenced
};

// ======================================================================
// class GridAppearance::LocalShaderPrimitive
// ======================================================================

GridAppearance::LocalShaderPrimitive::LocalShaderPrimitive (GridAppearance &appearance)
:	ShaderPrimitive (),
	m_appearance (appearance)
{
}

// ----------------------------------------------------------------------

GridAppearance::LocalShaderPrimitive::~LocalShaderPrimitive ()
{
}

// ----------------------------------------------------------------------

const StaticShader &GridAppearance::LocalShaderPrimitive::prepareToView () const
{
	return ShaderTemplateList::get3dVertexColorZStaticShader ();
}

// ----------------------------------------------------------------------

const Vector GridAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return m_appearance.getTransform_w ().getPosition_p ();
}
	
// ----------------------------------------------------------------------

float GridAppearance::LocalShaderPrimitive::getDepthSquaredSortKey () const
{
	return m_appearance.getTransform_w ().getPosition_p ().magnitudeBetweenSquared (ShaderPrimitiveSorter::getCurrentCameraPosition ());
}

// ----------------------------------------------------------------------

int GridAppearance::LocalShaderPrimitive::getVertexBufferSortKey () const
{
	// not using vertex buffers yet
	return 0;
}

// ----------------------------------------------------------------------

void GridAppearance::LocalShaderPrimitive::prepareToDraw () const
{
	Graphics::setObjectToWorldTransformAndScale (m_appearance.getTransform_w (), Vector::xyz111);
}

// ----------------------------------------------------------------------

void GridAppearance::LocalShaderPrimitive::draw () const
{
	const Vector *vertexList      = m_appearance.getVertexList ();
	const int     numSubdivisions = m_appearance.getSubdivisions ();

	Vector step  = vertexList [LL] - vertexList [UL];
	step        /= static_cast<float> (numSubdivisions + 1);

	Vector v0    = vertexList [UL];
	Vector v1    = vertexList [UR];

	Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorZStaticShader ());

	const VectorArgb &color = m_appearance.getColor ();

	//-- render rows
	{
		for (int i = 0; i <= numSubdivisions+1; i++)
		{
			Graphics::drawLine (v0, v1, i == (numSubdivisions+1) / 2 ? VectorArgb::solidBlack : color);
			v0 += step;
			v1 += step;
		}
	}

	//-- render columns
	step  = vertexList [UR] - vertexList [UL];
	step /= static_cast<real> (numSubdivisions + 1);

	v0 = vertexList [UL];
	v1 = vertexList [LL];

	{
		for (int i = 0; i <= numSubdivisions+1; i++)
		{
			Graphics::drawLine (v0, v1, i == (numSubdivisions+1) / 2 ? VectorArgb::solidBlack : color);
			v0 += step;
			v1 += step;
		}
	}
}

// ======================================================================
// class GridAppearance
// ======================================================================

GridAppearance::GridAppearance (GridPlane newGridPlane, real newSize, int newNumSubdivisions, const VectorArgb& newColor)
: Appearance (0),
	m_size (newSize),
	m_vList (new Vector[4]),
	m_numSubdivisions (newNumSubdivisions),
	m_sphere (new Sphere ()),
	m_color (new VectorArgb (newColor)),
	m_shaderPrimitive (0),
	m_dpvsObject(0)
{
	resize (newGridPlane, newSize);

	m_shaderPrimitive = new LocalShaderPrimitive (*this);
}

// ----------------------------------------------------------------------

GridAppearance::~GridAppearance ()
{
	delete m_shaderPrimitive;
	delete m_color;
	delete m_sphere;
	delete [] m_vList;
	m_dpvsObject->release();
}

// ----------------------------------------------------------------------

const Sphere& GridAppearance::getSphere() const
{
	return *m_sphere;
}

// ----------------------------------------------------------------------

DPVS::Object *GridAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

// ----------------------------------------------------------------------

void GridAppearance::render () const
{
	ShaderPrimitiveSorter::add (*m_shaderPrimitive);
}

// ----------------------------------------------------------------------

void GridAppearance::resize (GridPlane newGridPlane, real newSize)
{
	switch (newGridPlane)
	{
	case GP_XZ:
		{
			m_vList [UL].set (-1.0f,  0.0f,  1.0f);
			m_vList [UR].set ( 1.0f,  0.0f,  1.0f);
			m_vList [LL].set (-1.0f,  0.0f, -1.0f);
			m_vList [LR].set ( 1.0f,  0.0f, -1.0f);
		}
		break;

	case GP_XY:
		{
			m_vList [UL].set (-1.0f,  1.0f,  0.0f);
			m_vList [UR].set ( 1.0f,  1.0f,  0.0f);
			m_vList [LL].set (-1.0f, -1.0f,  0.0f);
			m_vList [LR].set ( 1.0f, -1.0f,  0.0f);
		}
		break;

	case GP_YZ:
		{
			m_vList [UL].set ( 0.0f, -1.0f,  1.0f);
			m_vList [UR].set ( 0.0f,  1.0f,  1.0f);
			m_vList [LL].set ( 0.0f, -1.0f, -1.0f);
			m_vList [LR].set ( 0.0f,  1.0f, -1.0f);
		}
		break;
	};

	m_size = newSize;

	AxialBox box;
	for (size_t i = 0; i < 4; i++)
	{
		m_vList [i] *= m_size;
		box.add(m_vList [i]);
	}

	m_sphere->setCenter(Vector::zero);
	m_sphere->setRadius(m_vList [UL].magnitude ());

	if (m_dpvsObject)
	{
		DPVS::OBBModel *const testModel = RenderWorld::fetchBoxModel(box);
		m_dpvsObject->setTestModel(testModel);
		testModel->release();
	}
	else
		m_dpvsObject = RenderWorld::createObject(this, box);
}

// ======================================================================
