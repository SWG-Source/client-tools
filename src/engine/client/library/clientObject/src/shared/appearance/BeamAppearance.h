// ======================================================================
//
// BeamAppearance.h
// Portions copyright 1999, bootprint entertainment
// Portions copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.

// ======================================================================

#ifndef INCLUDED_BeamAppearance_H
#define INCLUDED_BeamAppearance_H

// ======================================================================

class MemoryBlockManager;
class Shader;

#include "sharedObject/Appearance.h"
#include "sharedMath/VectorArgb.h"

// ======================================================================

class BeamAppearance : public Appearance
{
public:

	static void install();

	static void *operator new(size_t size);
	static void  operator delete(void *pointer);

public:

	BeamAppearance(float newLength, float newWidth, Shader *newShader, const VectorArgb &newColor=VectorArgb::solidWhite);
	virtual ~BeamAppearance();

	virtual const Sphere &getSphere() const;

	virtual float         alter(float time);

	virtual void          setCustomizationData(CustomizationData *customizationData);
	virtual void          addCustomizationVariables(CustomizationData &customizationData) const;

	virtual void          render() const;

	void                  setAlpha(bool opaqueEnabled, float opaqueAlpha, bool alphaEnabled, float alphaAlpha);
	void                  setLength(float newLength);
	void                  setWidth(float newWidth);
	void				  useBaseShader();
	void				  useAlternateShader(int index);

protected:

	DPVS::Object         *getDpvsObject() const;

private:

	static void remove();

private:

	void updateDpvsTestModel();

private:

	static MemoryBlockManager *ms_memoryBlockManager;

private:

	class LocalShaderPrimitive;

private:

	LocalShaderPrimitive *m_localShaderPrimitive;
	DPVS::Object         *m_dpvsObject;

private:

	BeamAppearance();
	BeamAppearance(const BeamAppearance&);
	BeamAppearance &operator=(const BeamAppearance&);
};

// ======================================================================

#endif
