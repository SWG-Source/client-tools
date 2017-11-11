//===================================================================
//
// GradientSkyAppearance.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
//===================================================================

#ifndef INCLUDED_GradientSkyAppearance_H
#define INCLUDED_GradientSkyAppearance_H

//===================================================================

#include "sharedObject/Appearance.h"

//===================================================================

class GradientSkyAppearance : public Appearance
{
public:

	static void install();

public:

	GradientSkyAppearance (const char* shaderTemplateName);
	virtual ~GradientSkyAppearance (void);

	const char* getName () const;

	virtual void          render () const;
	virtual float         alter (float elapsedTime);
	virtual void          setAlpha (bool opaqueEnabled, float opaqueAlpha, bool alphaEnabled, float alphaAlpha);

	void                  setTime (float time);

protected:

	DPVS::Object*         getDpvsObject() const;

private:

	class LocalShaderPrimitive;
	LocalShaderPrimitive* m_localShaderPrimitive;
	DPVS::Object*         m_dpvsObject;

private:

	// disabled
	GradientSkyAppearance ();
	GradientSkyAppearance (const GradientSkyAppearance&);
	GradientSkyAppearance& operator= (const GradientSkyAppearance&);
};

//===================================================================

#endif
