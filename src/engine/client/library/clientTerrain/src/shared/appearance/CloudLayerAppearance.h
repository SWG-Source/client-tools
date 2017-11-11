//===================================================================
//
// CloudLayerAppearance
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
//===================================================================

#ifndef INCLUDED_CloudLayerAppearance_H
#define INCLUDED_CloudLayerAppearance_H

//===================================================================

#include "sharedObject/Appearance.h"

//===================================================================

class CloudLayerAppearance : public Appearance
{
public:

	static void install ();

	static void setDay (bool day);

public:

	CloudLayerAppearance (const char* shaderTemplateName, float height, float velocity);
	virtual ~CloudLayerAppearance ();

	const char* getName () const;
	
	virtual void          render () const;
	virtual float         alter (float time);
	virtual void          setAlpha (bool opaqueEnabled, float opaqueAlpha, bool alphaEnabled, float alphaAlpha);

protected:

	DPVS::Object*         getDpvsObject() const;

private:

	static void remove ();

private:

	class LocalShaderPrimitive;
	LocalShaderPrimitive* m_localShaderPrimitive;
	DPVS::Object*         m_dpvsObject;

private:

	// disabled
	CloudLayerAppearance ();
	CloudLayerAppearance (const CloudLayerAppearance&);
	CloudLayerAppearance& operator= (const CloudLayerAppearance&);
};

//===================================================================

#endif
