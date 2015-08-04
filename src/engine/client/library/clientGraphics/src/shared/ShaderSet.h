// ======================================================================
//
// ShaderSet.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef	SHADER_SET_H
#define SHADER_SET_H

// ======================================================================

class Shader;
class ShaderTemplate;

// ======================================================================
/**
 * Provides the vehicle for sharing a group of Shader objects among different
 * objects that have no knowlege of one another but need to use the same
 * Shader objects.
 * 
 * This class is used by the character system's SkeletalAppearance class.
 * For each SkeletalAppearance object's ShaderTemplate, only one
 * Shader object should exist.  Across LODs, the same Shader should be
 * used.  This object is reference counted and will destroy itself when
 * the reference count reaches zero.
 */

class ShaderSet
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct ShaderData
	{
	public:

		explicit ShaderData(const ShaderTemplate *shaderTemplate);

	public:

		Shader *m_shader;
		int     m_referenceCount;

	private:
		// disabled
		ShaderData();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	ShaderSet();

	void  fetch();
	void  release();

	Shader *fetchShader(const ShaderTemplate *shaderTemplate);
	void    releaseShader(Shader *shader);

private:

	~ShaderSet();

private:

	typedef stdvector<ShaderData>::fwd ShaderDataContainer;

private:

	int                  m_referenceCount;
	ShaderDataContainer *m_shaders;

private:
	// disabled
	ShaderSet(const ShaderSet&);
	ShaderSet &operator =(const ShaderSet&);
};

// ======================================================================

inline void ShaderSet::fetch()
{
	++m_referenceCount;
}

// ======================================================================

#endif
