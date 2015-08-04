// ======================================================================
//
// ShaderTemplateList.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_ShaderTemplateList_H
#define INCLUDED_ShaderTemplateList_H

// ======================================================================

class CrcString;
class Iff;
class Shader;
class ShaderTemplate;
class StaticShader;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

// ======================================================================

class ShaderTemplateList
{
	friend class ShaderTemplate;

public:

	typedef ShaderTemplate *(*CreationFunction)(const CrcString &name, Iff &iff);

public:

	static void install();
	static void assignAsynchronousLoaderFunctions();

	static void registerShaderTemplateType(Tag tag, CreationFunction creationFunction);
	static void deregisterShaderTemplateType(Tag tag);

	static const ShaderTemplate *fetch(Iff &iff);
	static const ShaderTemplate *fetch(Iff &iff, bool & error);
	static const ShaderTemplate *fetch(const char *name);
	static const ShaderTemplate *fetch(const char *name, bool & error);
	static const ShaderTemplate *fetch(const CrcString &name);
	static const ShaderTemplate *fetch(const CrcString &name, bool & error);

	static const Shader         *fetchShader(Iff &iff);
	static Shader               *fetchModifiableShader(Iff &iff);
	static const Shader         *fetchShader(const char *name);
	static const Shader         *fetchShader(const CrcString &name);
	static Shader               *fetchModifiableShader(const char *name);
	static Shader               *fetchModifiableShader(const CrcString &name);

	static void                  preloadVertexColorShaderTemplates();
	static void                  releaseVertexColorShaderTemplates();

	static const StaticShader   &get3dVertexColorStaticShader();
	static const StaticShader   &get3dVertexColorAStaticShader();
	static const StaticShader   &get3dVertexColorACStaticShader();
	static const StaticShader   &get3dVertexColorAZStaticShader();
	static const StaticShader   &get3dVertexColorZStaticShader();
	static const StaticShader   &get3dVertexColorLZStaticShader();

	static const StaticShader   &get2dVertexColorStaticShader();
	static const StaticShader   &get2dVertexColorAStaticShader();

	static const StaticShader   *getBadVertexShaderStaticShader();

private:

	static void remove();

	static const void           *asynchronousLoaderFetchNoCreate(const char *fileName);
	static void                  asynchronousLoaderRelease(const void *shaderTemplate);

	static const ShaderTemplate *fetch(const CrcString &name, bool create, bool & error);
	static const ShaderTemplate *fetch(const CrcString &name, Iff &iff, bool & error);
	static ShaderTemplate       *create(const CrcString &name, Iff &iff);
	static void                  stopTracking(const ShaderTemplate *shaderTemplate);

	static void                  enterCriticalSection();
	static void                  leaveCriticalSection();
};

// ======================================================================

#endif
