// ======================================================================
//
// FadingTextAppearance.h
// Copyright 2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FadingTextAppearance_H
#define INCLUDED_FadingTextAppearance_H

// ======================================================================

class Object;
class Camera;
class VertexBuffer;
struct UIColor;

// ======================================================================

#include "sharedObject/TextAppearance.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMath/Sphere.h"
#include "Unicode.h"

// ======================================================================

class FadingTextAppearance : public TextAppearance
{
public:
	                          FadingTextAppearance (const Unicode::String & str, const VectorArgb & color, float startingOpacity, float fadeOutTime, float sizeModifier, int textType, bool unbounded = false);
							  FadingTextAppearance (const Unicode::String & str, const VectorArgb & color, float startingOpacity, float fadeOutTime, float sizeModifier, int textType, Vector screenSpaceLocation, Vector screenSpaceVelocity, float speedModifier = 1.0f);
	virtual                  ~FadingTextAppearance ();
		
	virtual void              render                   () const;
	virtual float             alter                    (float time);
	virtual const Sphere     &getSphere                () const;
	void                      setUnBounded             (const bool unBounded);
	virtual void              setText                  (const char* text);
	void                      setText                  (const Unicode::String & str);
	void                      setFadeEnabled           (const bool fadeEnabled);
	void                      setRenderWithChatBubbles (bool b);
	virtual void              setColor                 (const VectorArgb & color);

	static bool               isEnabled    ();
	static void               setEnabled   (bool b);

private:

	virtual DPVS::Object     *getDpvsObject() const;

	// disabled
	FadingTextAppearance (const FadingTextAppearance & rhs);
	FadingTextAppearance &operator =(const FadingTextAppearance & rhs);

private:

	Unicode::String           m_string;
	float                     m_opacityFactor;
	float                     m_falloffPerTime;
	UIColor *                 m_color;
	float                     m_sizeModifier;
	Sphere                    m_sphere;
	DPVS::Object *            m_dpvsObject;
	bool                      m_fadeEnabled;
	int                       m_textType;
	bool                      m_renderWithChatBubbles;
	Vector                    m_screenSpaceLocation;
	Vector                    m_screenSpaceVelocity;
	float                     m_speedModifier;
	bool                      m_useScreenSpace;
	static bool               ms_enabled;
};

//----------------------------------------------------------------------

inline bool FadingTextAppearance::isEnabled    ()
{
	return ms_enabled;
}

// ======================================================================

#endif
