// ======================================================================
//
// SwitchTextureShader.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwitchTextureShader_H
#define INCLUDED_SwitchTextureShader_H

// ======================================================================

class Switcher;

#include "clientGraphics/Shader.h"
#include "clientGraphics/SwitchTextureShaderTemplate.h"

// ======================================================================

class SwitchTextureShader :public Shader
{
	friend class SwitchTextureShaderTemplate;

public:

	virtual Shader              *convertToModifiableShader() const;

	virtual bool                 obeysCustomizationData() const;
	virtual float                alter(float time) const;
	virtual const StaticShader  &prepareToView() const;
	virtual bool                 usesVertexShader() const;
	virtual const StaticShader  *getStaticShader() const;
	virtual StaticShader        *getStaticShader();

	const SwitchTextureShaderTemplate  &getSwitchTextureShaderTemplate() const;

protected:

	SwitchTextureShader(const SwitchTextureShaderTemplate &shaderTemplate);
	virtual ~SwitchTextureShader();

	/// Disabled
	SwitchTextureShader();

	/// Disabled
	SwitchTextureShader(const SwitchTextureShader &);

	/// Disabled
	SwitchTextureShader &operator =(const SwitchTextureShader &);

private:

	typedef stdvector<const Shader *>::fwd Shaders;

private:

	Switcher *const      m_switcher;
	const Shaders *const m_shaders;
};

// ======================================================================
/**
 * Get the template for the switch shader.
 * @return This SwitchTextureShader's SwitchTextureShaderTemplate.
 */

inline const SwitchTextureShaderTemplate &SwitchTextureShader::getSwitchTextureShaderTemplate() const
{
	return static_cast<const SwitchTextureShaderTemplate &>(getShaderTemplate());
}

// ======================================================================

#endif
