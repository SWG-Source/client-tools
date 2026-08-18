// ======================================================================
//
// Shader.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_Shader_H
#define INCLUDED_Shader_H

// ======================================================================

class CustomizationData;
class ShaderTemplate;
class StaticShader;

// ======================================================================

class Shader
{
	friend class ShaderTemplate;

public:

	void fetch() const;
	void release() const;

	const ShaderTemplate         &getShaderTemplate() const;

	DLLEXPORT const char         *getName() const;

	virtual Shader               *convertToModifiableShader() const = 0;

	virtual bool                  obeysCustomizationData() const = 0;
	virtual void                  setCustomizationData(CustomizationData *customizationData);
	virtual void                  addCustomizationVariables(CustomizationData &customizationData) const;

	virtual float                 alter(float time) const;
	virtual const StaticShader   &prepareToView() const = 0;

	virtual bool                  usesVertexShader() const = 0;

	/**
	 * Retrieve the most appropriate StaticShader associated with this Shader.
	 *
	 * @return  the most appropriate StaticShader available for modification; NULL
	 *          if no such thing exists.
	 */
	virtual const StaticShader   *getStaticShader() const = 0;
	virtual StaticShader         *getStaticShader() = 0;

protected:

	Shader(const Shader &shader);
	explicit Shader(const ShaderTemplate &shaderTemplate);
	virtual ~Shader();

private:

	/// Disabled.
	Shader();

	/// Disabled.
	Shader &operator =(const Shader &);

private:

	// THREADING (2026-07-03): fetch/release run on BOTH the main thread (render-path
	// PrimitiveNode fetch/release, ShaderCache::alter releases) and the ClientTerrain
	// worker (fetchModifiable clones during chunk builds). A plain ++/-- can lose an
	// update and drive a live shader to `delete this`. Interlocked in Shader.cpp --
	// fetch() moved out-of-line (a header inline would need <intrin.h> in a shared
	// header the gl0X plugins compile). volatile long is layout-identical to int on
	// Win32/x64 -- no ABI change, but rebuild ALL plugin DLLs (AGENTS.md cascade trap).
	mutable volatile long  m_users;
	const ShaderTemplate &m_template;
};

// ----------------------------------------------------------------------

inline const ShaderTemplate &Shader::getShaderTemplate() const
{
	return m_template;
}

// ======================================================================

#endif

