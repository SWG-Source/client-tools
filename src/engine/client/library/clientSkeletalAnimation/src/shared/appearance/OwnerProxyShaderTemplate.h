// ======================================================================
//
// OwnerProxyShaderTemplate.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_OwnerProxyShaderTemplate_H
#define INCLUDED_OwnerProxyShaderTemplate_H

// ======================================================================

#include "clientGraphics/ShaderTemplate.h"

class Iff;
class MemoryBlockManager;
class Shader;
class ConstCharCrcString;

// ======================================================================

class OwnerProxyShaderTemplate: public ShaderTemplate
{
public:

	static void                install();

	static void               *operator new(size_t size);
	static void                operator delete(void *data);

	static Shader             &fetchNoRenderShader();

public:

	virtual bool               isOpaqueSolid() const;
	virtual bool               isCollidable() const;
	virtual bool               castsShadows() const;

	virtual const Shader      *fetchShader() const;
	virtual Shader            *fetchModifiableShader() const;

	const std::string         &getBaseShaderTemplateName() const;

private:

	static void            remove();
	static ShaderTemplate *create(const CrcString &name, Iff &iff);

private:

	OwnerProxyShaderTemplate(const CrcString &name, Iff &iff);
	virtual ~OwnerProxyShaderTemplate();

	void load_0000(Iff &iff);

	// Disabled.
	OwnerProxyShaderTemplate(const OwnerProxyShaderTemplate&);
	OwnerProxyShaderTemplate &operator =(const OwnerProxyShaderTemplate&);

private:

	static const ConstCharCrcString          cms_noRenderShaderTemplateName;

	static bool                              ms_installed;
	static MemoryBlockManager *ms_memoryBlockManager;
	static Shader                           *ms_noRenderShader;

private:

	std::string *m_baseShaderTemplateName;

};

// ======================================================================

inline const std::string &OwnerProxyShaderTemplate::getBaseShaderTemplateName() const
{
	NOT_NULL(m_baseShaderTemplateName);
	return *m_baseShaderTemplateName;
}

// ======================================================================

#endif
