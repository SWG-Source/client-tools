//
// SpriteAppearance.h
//
// Portions Copyright 1998 Bootprint Entertainment
// Portions Copyright 2000-2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//

//-------------------------------------------------------------------

#ifndef INCLUDED_SpriteAppearance_H
#define INCLUDED_SpriteAppearance_H

//-------------------------------------------------------------------

#include "sharedObject/Appearance.h"

class MemoryBlockManager;
class SpriteAppearanceTemplate;
class Shader;
class Timer;
class VectorArgb;

//-------------------------------------------------------------------

class SpriteAppearance : public Appearance
{
	class LocalShaderPrimitive;
	friend class LocalShaderPrimitive;

public:

	static void  install (void);

	static void *operator new (size_t size);
	static void  operator delete (void *pointer);

public:

	SpriteAppearance (const SpriteAppearanceTemplate* spriteAppearanceTemplate, const Vector& newCenter, real newRadius, Shader* newShader, const VectorArgb& newColor);
	virtual ~SpriteAppearance (void);

	virtual const Sphere& getSphere (void) const;

	virtual void          render () const;
	virtual float         alter (float time);

	virtual void          setAlpha  (bool opaqueEnabled, float opaqueAlpha, bool alphaEnabled, float alphaAlpha);

	const Shader*         getShader (void) const;
	Shader*               getShader (void);

	void                  setPulse (bool newPulse);

	VectorArgb           &getColor(void);
	const VectorArgb     &getColor(void) const;
	void                  setColor(const VectorArgb &newColor);

	void                  setRadius (real newRadius);

	virtual AxialBox const getTangibleExtent() const;

private:

	static void remove ();

private:

	virtual DPVS::Object *getDpvsObject() const;

private:

	static MemoryBlockManager *ms_memoryBlockManager;

	DPVS::Object         *m_dpvsObject;
	LocalShaderPrimitive *m_localShaderPrimitive;

private:

	// disabled
	SpriteAppearance (void);
	SpriteAppearance (const SpriteAppearance&);
	SpriteAppearance& operator= (const SpriteAppearance&);
};

// ======================================================================

#endif
