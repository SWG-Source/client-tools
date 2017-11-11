// ======================================================================
//
// MayaOwnerProxyShaderTemplateWriter.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MayaOwnerProxyShaderTemplateWriter_H
#define INCLUDED_MayaOwnerProxyShaderTemplateWriter_H

// ======================================================================

#include "MayaShaderTemplateWriter.h"

class Messenger;

// ======================================================================

class MayaOwnerProxyShaderTemplateWriter: public MayaShaderTemplateWriter
{
public:

	static void install(Messenger *newMessenger);
	static void remove();

public:

	virtual bool  canWrite(const MObject &shaderGroupObject) const;
	virtual bool  write(const std::string       &shaderTemplateWriteName, 
	                    const MObject           &shaderGroupObject, 
	                    bool                     hasVertexAlpha,
	                    TextureRendererVector   &referencedTextureRenderers, 
	                    const std::string       &textureReferenceDirectory,
	                    const std::string       &textureRendererReferenceDirectory,
	                    const std::string       &effectReferenceDirectory,
	                    MayaUtility::TextureSet &textureFilenames,
	                    bool                     hasDot3TextureCoordinate,
	                    int                      dot3TextureCoordinateIndex);

private:

	static bool   getMaterialDependencyNode(const MObject &shaderGroupObject, MFnDependencyNode &materialDependencyNode);

private:

	static const int          cms_priority;
	static const int          cms_iffWriteSize;
	static const MString      cms_baseShaderNameAttribute;

	static bool               ms_installed;
	static Messenger         *messenger;

};

// ======================================================================

#endif
