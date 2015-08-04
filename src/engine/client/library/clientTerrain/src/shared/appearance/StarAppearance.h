// ======================================================================
//
// StarAppearance.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_StarAppearance_H
#define INCLUDED_StarAppearance_H

// ======================================================================

#include "sharedObject/Appearance.h"

class Shader;
class Sphere;
class SystemVertexBuffer;

// ======================================================================

class StarAppearance : public Appearance
{
public:

	static void install();

public:

	StarAppearance(char const * starColorRampFileName, int numberOfStars, bool allowRotate, float initialAlpha=1.f);
	virtual ~StarAppearance();

	virtual void setAlpha(bool opaqueEnabled, float opaqueAlpha, bool alphaEnabled, float alphaAlpha);
	virtual void render() const;

private:

	StarAppearance(const StarAppearance &);
	StarAppearance &operator=(const StarAppearance &);

	DPVS::Object * getDpvsObject() const;

private:

	DPVS::Object * m_dpvsObject;

	typedef std::pair<Sphere, SystemVertexBuffer *> Section;
	Section * m_sections[4][4][4];

	Shader * const m_shader;

public:
	class LocalShaderPrimitive;

private:
	friend class LocalShaderPrimitive;
	typedef stdvector<LocalShaderPrimitive *>::fwd LocalShaderPrimitiveList;
	LocalShaderPrimitiveList * const m_localShaderPrimitiveList;

	bool const m_allowRotate;
	uint32 m_lastColor;
};

// ======================================================================

#endif
