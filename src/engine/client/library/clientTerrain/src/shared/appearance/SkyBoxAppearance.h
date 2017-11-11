//===================================================================
//
// SkyBoxAppearance.h
// asommers 3-31-99
//
// copyright 1999, bootprint entertainment
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SkyBoxAppearance_H
#define INCLUDED_SkyBoxAppearance_H

//===================================================================

#include "sharedObject/Appearance.h"

class Texture;

//===================================================================

class SkyBoxAppearance : public Appearance
{
public:

	static void install ();

public:

	SkyBoxAppearance (const char* shaderTemplateName, float alpha = 0.f);
	SkyBoxAppearance (const Texture* texture, float alpha = 0.f);
	virtual ~SkyBoxAppearance ();

	const char* getName () const;

	virtual void setAlpha (bool opaqueEnabled, float opaqueAlpha, bool alphaEnabled, float alphaAlpha);
	virtual void render () const;

private:

	DPVS::Object* getDpvsObject () const;

private:

	SkyBoxAppearance ();
	SkyBoxAppearance (const SkyBoxAppearance&);
	SkyBoxAppearance& operator= (const SkyBoxAppearance&);

private:

	class LocalShaderPrimitive;
	LocalShaderPrimitive* m_localShaderPrimitive;
	DPVS::Object*         m_dpvsObject;
	float                 m_alpha;
};

//===================================================================

#endif
