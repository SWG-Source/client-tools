// ======================================================================
//
// ForceFieldAppearance.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ForceFieldAppearance.h"

#include "clientGraphics/DynamicColorPolyPrimitive.h"
#include "clientGraphics/RenderWorld.h"
#include "sharedMath/AxialBox.h"
#include "sharedObject/AlterResult.h"

#include "dpvsObject.hpp"

// ======================================================================

ForceFieldAppearance::ForceFieldAppearance(VertexList const & verts, const VectorArgb &newColor) :
	SimpleAppearance(),
	Tweakable(),
	m_time(0.0f),
  m_fade(0.0f),
  m_block(0.0f),
  m_lerpS(0.0f),
  m_lerpT(0.0f),
	m_lerpFade(0.0f),
  m_colorA(newColor),
  m_colorB(newColor),
  m_colorC(newColor),
  m_colorD(newColor),
  m_fadeColor(newColor),
  m_blockColor(newColor),
	m_dpvsObject(0)
{
	VectorArgb clearWhite(0.08f,1.0f,1.0f,1.0f);

	VectorArgb transRed(0.85f,0.3f,0.02f,0.01f);

	VectorArgb transBlue1(0.6f,0.5f,0.5f,1.0f);
	VectorArgb transBlue2(0.6f,0.45f,0.45f,1.0f);
	VectorArgb transBlue3(0.4f,0.5f,0.5f,1.0f);
	VectorArgb transBlue4(0.4f,0.45f,0.45f,1.0f);

	// ----------

	ForceFieldAppearance::setLerpColors(transBlue1,transBlue2,transBlue3,transBlue4);
	ForceFieldAppearance::setFadeColor(clearWhite);
	ForceFieldAppearance::setBlockColor(transRed);

	// ----------

	ForceFieldAppearance::attachPrimitive( new DynamicColorPolyPrimitive(*this, verts, newColor) );

	AxialBox box;
	box.add(verts);
	m_dpvsObject = RenderWorld::createObject(this, box);
}

// ----------------------------------------------------------------------

ForceFieldAppearance::~ForceFieldAppearance()
{
	IGNORE_RETURN(m_dpvsObject->release());
}

// ----------------------------------------------------------------------

float ForceFieldAppearance::alter ( float time )
{
	IGNORE_RETURN(SimpleAppearance::alter(time));

	m_time += time;

	float rateS = 3.0f;
	float rateT = 0.2f;

	m_lerpS = (cos(m_time * PI * 2.0f * rateS) + 1.0f) / 2.0f;
	m_lerpT = (cos(m_time * PI * 2.0f * rateT) + 1.0f) / 2.0f;

	// ----------

	DynamicColorPolyPrimitive * prim = dynamic_cast<DynamicColorPolyPrimitive*>(getPrimitive());

	if(prim)
	{
		prim->setColor(getColor());
	}

	// @todo figure out what this should really return.
	return AlterResult::cms_alterNextFrame;
}

// ----------------------------------------------------------------------

DPVS::Object * ForceFieldAppearance::getDpvsObject() const
{
	return m_dpvsObject;
} //lint !e1763 // Function marked const indirectly modifies class // This is okay.

// ----------------------------------------------------------------------
// Force fields aren't drawn if they're completely faded out

void ForceFieldAppearance::render() const
{
	if (m_fade < 1.0f)
		SimpleAppearance::render();
}

// ----------------------------------------------------------------------

float ForceFieldAppearance::getTweakValue ( int whichTweak )
{
	if(whichTweak == 0)
	{
		return m_fade;
	}
	else if (whichTweak == 1)
	{
		return m_block;
	}
	else
	{
		return 0.0f;
	}
}

// ----------

void ForceFieldAppearance::setTweakValue ( int whichTweak, float value )
{
	if(whichTweak == 0)
	{
		m_fade = value;
	}
	else if(whichTweak == 1)
	{
		m_block = value;
	}
	else
	{
	}
}

// ----------------------------------------------------------------------

void ForceFieldAppearance::setLerpColors ( VectorArgb A, VectorArgb B, VectorArgb C, VectorArgb D )
{
	m_colorA = A;
	m_colorB = B;
	m_colorC = C;
	m_colorD = D;
}

// ----------

void ForceFieldAppearance::setFadeColor ( VectorArgb color )
{
	m_fadeColor = color;
}

// ----------

void ForceFieldAppearance::setBlockColor ( VectorArgb color )
{
	m_blockColor = color;
}

// ----------------------------------------------------------------------

VectorArgb ForceFieldAppearance::getColor ( void ) const
{
	VectorArgb temp1 = VectorArgb::linearInterpolate(m_colorA,m_colorB,m_lerpS);
	VectorArgb temp2 = VectorArgb::linearInterpolate(m_colorC,m_colorD,m_lerpS);

	VectorArgb color = VectorArgb::linearInterpolate(temp1,temp2,m_lerpT);

	VectorArgb faded;

	if(m_block > 0.0f)
	{
		faded = VectorArgb::linearInterpolate(color,m_blockColor,m_block);
	}
	else
	{
		faded = VectorArgb::linearInterpolate(color,m_fadeColor,m_fade);
	}

	return faded;
}

// ======================================================================
