// ======================================================================
//
// MayaShaderTemplateBuilder.h
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MayaShaderTemplateBuilder_H
#define INCLUDED_MayaShaderTemplateBuilder_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

#include "MayaUtility.h"

class MayaShaderTemplateWriter;
class Messenger;
class MObject;

// ======================================================================

class MayaShaderTemplateBuilder
{
public:

	typedef stdvector<std::pair<std::string, Tag> >::fwd  TextureRendererVector;

public:

	static void install(Messenger *newMessenger);
	static void remove();

	static void registerShaderTemplateWriter(MayaShaderTemplateWriter *writer, int priority);

	static bool buildShaderTemplate(const std::string       &shaderTemplateWriteName, 
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

	class WriterInfo;

	typedef stdvector<WriterInfo*>::fwd  WriterInfoVector;

private:

	/// useful for sending messages to an output window (*must* be named "messenger" to match macros)
	static Messenger         *messenger;

	/// indicates if the module is installed
	static bool               ms_installed;

	/// registered MayaShaderTemplateWriter instances
	static WriterInfoVector   ms_writerInfoVector;
};

// ======================================================================

#endif
