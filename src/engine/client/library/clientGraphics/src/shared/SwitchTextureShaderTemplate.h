// ======================================================================
//
// SwitchTextureShaderTemplate.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwitchTextureShaderTemplate_H
#define INCLUDED_SwitchTextureShaderTemplate_H

// ======================================================================

class Iff;
class SwitcherTemplate;

#include "clientGraphics/ShaderTemplate.h"

// ======================================================================

class SwitchTextureShaderTemplate : public ShaderTemplate
{
	friend class SwitchTextureShader;

public:

	typedef stdvector<const Shader *>::fwd Shaders;

public:

	static void install();

public:
	
	virtual bool          isOpaqueSolid() const;
	virtual bool          isCollidable() const;
	virtual bool          castsShadows() const;
	virtual const Shader *fetchShader() const;
	virtual Shader       *fetchModifiableShader() const;

	const Shaders        *getShaders() const;

private:

	static void remove();

	static ShaderTemplate *create(const CrcString &name, Iff &iff);

private:

	SwitchTextureShaderTemplate(const CrcString &name, Iff &iff);
	virtual ~SwitchTextureShaderTemplate();

	void load(Iff &iff);
	void load_0000(Iff &iff);

	// disabled.
	SwitchTextureShaderTemplate();
	SwitchTextureShaderTemplate(const SwitchTextureShaderTemplate &);
	SwitchTextureShaderTemplate &operator =(const SwitchTextureShaderTemplate &);

private:

	static bool ms_installed;

private:

	SwitcherTemplate     *m_switcherTemplate;
	const ShaderTemplate *m_shaderTemplate;
	Shaders *const        m_shaders;
};

// ======================================================================
// @todo remove this routine.  it's only here for emergency data problem finding

inline const SwitchTextureShaderTemplate::Shaders *SwitchTextureShaderTemplate::getShaders() const
{
	return m_shaders;
}

// ======================================================================

#endif
