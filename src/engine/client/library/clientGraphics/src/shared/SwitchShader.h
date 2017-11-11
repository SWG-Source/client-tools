// ======================================================================
//
// SwitchShader.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwitchShader_H
#define INCLUDED_SwitchShader_H

// ======================================================================

class Switcher;

#include "clientGraphics/Shader.h"
#include "clientGraphics/SwitchShaderTemplate.h"

// ======================================================================

class SwitchShader :public Shader
{
	friend class SwitchShaderTemplate;

public:

	virtual Shader              *convertToModifiableShader() const;

	virtual bool                 obeysCustomizationData() const;
	virtual void                 setCustomizationData(CustomizationData *customizationData);
	virtual void                 addCustomizationVariables(CustomizationData &customizationData) const;

	virtual float                alter(float time) const;
	virtual const StaticShader  &prepareToView() const;
	virtual bool                 usesVertexShader() const;

	virtual const StaticShader  *getStaticShader() const;
	virtual StaticShader        *getStaticShader();

	const SwitchShaderTemplate  &getSwitchShaderTemplate() const;

protected:

	SwitchShader(const SwitchShaderTemplate &shaderTemplate);
	virtual ~SwitchShader();

	/// Disabled
	SwitchShader();

	/// Disabled
	SwitchShader(const SwitchShader &);

	/// Disabled
	SwitchShader &operator =(const SwitchShader &);

private:

	typedef stdvector<const Shader *>::fwd Shaders;

private:

	Switcher    *m_switcher;
	Shaders     *m_shader;
};

// ======================================================================
/**
 * Get the template for the switch shader.
 * @return This SwitchShader's SwitchShaderTemplate.
 */

inline const SwitchShaderTemplate &SwitchShader::getSwitchShaderTemplate() const
{
	return static_cast<const SwitchShaderTemplate &>(getShaderTemplate());
}

// ======================================================================

#endif
