// ======================================================================
//
// ShaderSet.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/ShaderSet.h"

#include <algorithm>
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplate.h"
#include <utility>
#include <vector>

// ======================================================================

namespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ShaderIsFromShaderTemplate
	{
	public:

		explicit ShaderIsFromShaderTemplate(const ShaderTemplate *shaderTemplate)
			: m_shaderTemplate(shaderTemplate)
			{
			}

		bool operator ()(const ShaderSet::ShaderData &shaderData) const
			{
				return (&(shaderData.m_shader->getShaderTemplate()) == m_shaderTemplate);
			}

	private:

		const ShaderTemplate *m_shaderTemplate;

	private:
		// disabled
		ShaderIsFromShaderTemplate();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class IsShader
	{
	public:

		explicit IsShader(Shader *shader)
			: m_shader(shader)
			{
			}

		bool operator ()(const ShaderSet::ShaderData &shaderData) const
			{
				return shaderData.m_shader == m_shader;
			}

	private:

		Shader *m_shader;

	private:
		// disabled
		IsShader();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	inline void ReleaseShaderData(ShaderSet::ShaderData &shaderData)
	{
		DEBUG_WARNING(shaderData.m_referenceCount, ("ShaderSet::ShaderData deleted with [%d] references", shaderData.m_referenceCount));

		// release our reference
		shaderData.m_shader->release();
		shaderData.m_shader = 0;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

}

// ======================================================================
// struct ShaderSet::ShaderData
// ======================================================================

ShaderSet::ShaderData::ShaderData(const ShaderTemplate *shaderTemplate)
:	m_shader(shaderTemplate->fetchModifiableShader()),
	m_referenceCount(0)
{
}

// ======================================================================
// class ShaderSet
// ======================================================================

ShaderSet::ShaderSet()
:	m_referenceCount(0),
	m_shaders(new ShaderDataContainer())
{
}

// ----------------------------------------------------------------------

ShaderSet::~ShaderSet()
{
	IGNORE_RETURN( std::for_each(m_shaders->begin(), m_shaders->end(), ReleaseShaderData) );
	delete m_shaders;
}

// ----------------------------------------------------------------------

void ShaderSet::release()
{
	--m_referenceCount;
	if (m_referenceCount < 1)
	{
		//-- delete this
		DEBUG_FATAL(m_referenceCount != 0, ("expecting zero reference count on ShaderSet [%d]", m_referenceCount));
		delete this;
	}
}

// ----------------------------------------------------------------------

Shader *ShaderSet::fetchShader(const ShaderTemplate *shaderTemplate)
{
	ShaderDataContainer::iterator it = std::find_if(m_shaders->begin(), m_shaders->end(), ShaderIsFromShaderTemplate(shaderTemplate));
	if (it != m_shaders->end())
	{
		//-- we've already created the shader for this shader template, return it
		ShaderData &shaderData = *it;
		++shaderData.m_referenceCount;

		shaderData.m_shader->fetch();
		return shaderData.m_shader;
	}

	//-- create the shader data
	ShaderData newShaderData(shaderTemplate);
	++newShaderData.m_referenceCount;

	//-- keep track of it (the copy is okay here)
	m_shaders->push_back(newShaderData);

	//-- fetch and return new shader
	newShaderData.m_shader->fetch();
	return newShaderData.m_shader;
}

// ----------------------------------------------------------------------

void ShaderSet::releaseShader(Shader *shader)
{
	ShaderDataContainer::iterator it = std::find_if(m_shaders->begin(), m_shaders->end(), IsShader(shader));
	DEBUG_FATAL(it == m_shaders->end(), ("shader [0x%08x] not managed by this ShaderSet", shader));
	if (it != m_shaders->end())
	{
		NOT_NULL(shader);

		//-- release caller's reference (matches fetch in fetchShader())
		shader->release();
		shader = 0;

		ShaderData &shaderData = *it;
		--shaderData.m_referenceCount;
		if (shaderData.m_referenceCount < 1)
		{
			//-- time for this shader data to retire
			ReleaseShaderData(shaderData);
			IGNORE_RETURN(m_shaders->erase(it));
		}
	}
}

// ======================================================================
