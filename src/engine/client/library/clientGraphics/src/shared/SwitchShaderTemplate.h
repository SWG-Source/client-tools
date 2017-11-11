// ======================================================================
//
// SwitchShaderTemplate.h
// jeff grills
//
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwitchShaderTemplate_H
#define INCLUDED_SwitchShaderTemplate_H

// ======================================================================

class Iff;
class SwitcherTemplate;

#include "clientGraphics/ShaderTemplate.h"

// ======================================================================

class SwitchShaderTemplate : public ShaderTemplate
{
	friend class SwitchShader;

public:

	typedef stdvector<const ShaderTemplate *>::fwd ShaderTemplates;

public:

	static void install();

public:
	
	virtual bool          isOpaqueSolid() const;
	virtual bool          isCollidable() const;
	virtual bool          castsShadows() const;
	virtual const Shader *fetchShader() const;
	virtual Shader       *fetchModifiableShader() const;

	const ShaderTemplates *getShaderTemplates() const;

private:

	static void remove();

	static ShaderTemplate *create(const CrcString &name, Iff &iff);

private:

	SwitchShaderTemplate(const CrcString &name, Iff &iff);
	virtual ~SwitchShaderTemplate();

	void load(Iff &iff);
	void load_0000(Iff &iff);

	// disabled.
	SwitchShaderTemplate();
	SwitchShaderTemplate(const SwitchShaderTemplate &);
	SwitchShaderTemplate &operator =(const SwitchShaderTemplate &);

private:

	static bool ms_installed;

private:

	SwitcherTemplate  *m_switcherTemplate;
	ShaderTemplates   *m_shaderTemplate;
};

// ======================================================================
// @todo remove this routine.  it's only here for emergency data problem finding

inline const SwitchShaderTemplate::ShaderTemplates *SwitchShaderTemplate::getShaderTemplates() const
{
	return m_shaderTemplate;
}

// ======================================================================

#endif
