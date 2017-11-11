//======================================================================
//
// GlowAppearance.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_GlowAppearance_H
#define INCLUDED_GlowAppearance_H

//======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "sharedObject/Appearance.h"

class ShaderTemplate;
class VectorArgb;

//----------------------------------------------------------------------

class GlowAppearance : public Appearance
{
public:

	GlowAppearance(ShaderTemplate const * shaderTemplateFront, ShaderTemplate const * shaderTemplateBack, VectorArgb const & color, float scaleMin, float scaleMaxFront, float scaleMaxBack);
	virtual ~GlowAppearance();

	virtual float alter(float elapsedTimeSecs);
	virtual void render() const;
	virtual Sphere const & getSphere() const;

	void updateGlowVisibility(float elapsedTimeSecs) const;
	float getGlowBlockScale() const;

	void reset();

protected:

	DPVS::Object * getDpvsObject() const;

private:

	void updateDpvsTestModel();
	class LocalShaderPrimitive;

private:

	LocalShaderPrimitive * m_localShaderPrimitive;
	DPVS::Object * m_dpvsObject;
	mutable bool m_glowBlocked;
	mutable float m_glowBlockScale;
	mutable Timer m_glowTimer;

private:

	GlowAppearance();
	GlowAppearance(const GlowAppearance &);
	GlowAppearance & operator=(const GlowAppearance &);
};

//======================================================================

#endif
