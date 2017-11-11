// ======================================================================
//
// SkyBox6SidedAppearance.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SkyBox6SidedAppearance_H
#define INCLUDED_SkyBox6SidedAppearance_H

// ======================================================================

#include "sharedObject/Appearance.h"

// ======================================================================

class SkyBox6SidedAppearance : public Appearance
{
public:

	explicit SkyBox6SidedAppearance(char const * textureNameMask);
	virtual ~SkyBox6SidedAppearance();

	virtual void render() const;

private:

	DPVS::Object * getDpvsObject() const;

private:

	SkyBox6SidedAppearance();
	SkyBox6SidedAppearance(SkyBox6SidedAppearance const &);
	SkyBox6SidedAppearance & operator=(SkyBox6SidedAppearance const &);

private:

	class LocalShaderPrimitive;
	typedef stdvector<LocalShaderPrimitive *>::fwd LocalShaderPrimitiveList;
	LocalShaderPrimitiveList * const m_localShaderPrimitiveList;
	DPVS::Object * m_dpvsObject;
};

// ======================================================================

#endif
