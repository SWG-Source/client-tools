// ======================================================================
//
// ForceFieldAppearance.h
// Portions copyright 1999, bootprint entertainment
// Portions copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.

// ======================================================================

#ifndef INCLUDED_ForceFieldAppearance_H
#define INCLUDED_ForceFieldAppearance_H

#include "clientGraphics/SimpleAppearance.h"
#include "sharedObject/Tweakable.h"
#include "sharedMath/VectorArgb.h"

class Vector;
typedef stdvector<Vector>::fwd VertexList;
class HardwareVertexBuffer;

// ======================================================================
// A force field appearance is a simple appearance that adds some animated
// colors and fading to produce a nice force field effect. 

// Force fields can be faded out or faded to a "you're not allowed through"
// color by using tweak values. 

// Tweak 0 - fade value : 0.0f == no fade, 1.0f == invisible
// Tweak 1 - block value : 0.0f == normal, 1.0f == solid block color

class ForceFieldAppearance : public SimpleAppearance, public Tweakable
{
public:

	ForceFieldAppearance(VertexList const & verts, VectorArgb const & color);
	virtual ~ForceFieldAppearance();

	virtual float       alter           ( float time );
	virtual void        render          ( void ) const;

	// ----------
	// tweak 0 = fade value (0,1)
	// tweak 1 = block value (0,1)

	virtual float       getTweakValue   ( int whichTweak );
	virtual void        setTweakValue   ( int whichTweak, float value );

	// ----------
	
	// The force field will oscillate between these four colors to produce a flickering effect
	virtual void        setLerpColors   ( VectorArgb A, VectorArgb B, VectorArgb C, VectorArgb D );

	// The force field will fade to this color when its fade tweak value is non-zero
	virtual void        setFadeColor    ( VectorArgb color );

	// The force field will fade to this color when its block tweak value is non-zero.
	virtual void        setBlockColor   ( VectorArgb color );

	virtual VectorArgb  getColor        ( void ) const;

protected:

	DPVS::Object * getDpvsObject() const;

protected:

	float m_time;
	float m_fade;
	float m_block;

	float m_lerpS;
	float m_lerpT;
	float m_lerpFade;

	VectorArgb m_colorA;
	VectorArgb m_colorB;
	VectorArgb m_colorC;
	VectorArgb m_colorD;
	VectorArgb m_fadeColor;
	VectorArgb m_blockColor;

	DPVS::Object * m_dpvsObject;

private:

	ForceFieldAppearance();
	ForceFieldAppearance(const ForceFieldAppearance&);
	ForceFieldAppearance &operator=(const ForceFieldAppearance&);
};

// ======================================================================

#endif
