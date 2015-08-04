// ======================================================================
//
// MarkerAppearance.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MarkerAppearance_H
#define INCLUDED_MarkerAppearance_H

// ======================================================================

#include "sharedObject/Appearance.h"

class MemoryBlockManager;
class MarkerAppearanceTemplate;
class Shader;
class Timer;
class VectorArgb;

// ======================================================================

class MarkerAppearance : public Appearance
{
public:

	static void  install(void);

	static void *operator new(size_t size);
	static void  operator delete(void *pointer);

public:

	MarkerAppearance(const MarkerAppearanceTemplate &spriteAppearanceTemplate);
	virtual ~MarkerAppearance(void);

	virtual const Sphere& getSphere(void) const;

	virtual void          render() const;
	virtual float         alter(float deltaTime);

	virtual void          setAlpha(bool opaqueEnabled, float opaqueAlpha, bool alphaEnabled, float alphaAlpha);

private:

	class LocalShaderPrimitive;
	friend class LocalShaderPrimitive;

private:

	static void  remove(void);

private:

	virtual DPVS::Object *getDpvsObject() const;

	// disabled
	MarkerAppearance(void);
	MarkerAppearance(const MarkerAppearance&);
	MarkerAppearance& operator=(const MarkerAppearance&);

private:

	static MemoryBlockManager *ms_memoryBlockManager;

private:

	LocalShaderPrimitive *m_localShaderPrimitive;
	DPVS::Object         *m_dpvsObject;

};

// ======================================================================

#endif
