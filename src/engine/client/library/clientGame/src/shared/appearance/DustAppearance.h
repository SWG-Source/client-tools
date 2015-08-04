// ======================================================================
//
// DustAppearance.h
// Copyright 2000-2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DustAppearance_H
#define INCLUDED_DustAppearance_H

// ======================================================================

#include "sharedObject/Appearance.h"

// ======================================================================

class DustAppearance : public Appearance
{
public:

	static void install();

public:

	DustAppearance(Object const * referenceObject, int numberOfDust, float dustRadius);
	virtual ~DustAppearance();

	virtual void render() const;

protected:

	virtual DPVS::Object * getDpvsObject() const;

private:

	DustAppearance();
	DustAppearance(const DustAppearance&);
	DustAppearance& operator=(const DustAppearance&);

private:

	class LocalShaderPrimitive;
	LocalShaderPrimitive * const m_localShaderPrimitive;
	DPVS::Object * m_dpvsObject;
};

// ======================================================================

#endif
