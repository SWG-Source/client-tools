// ======================================================================
//
// MayaAnimatingTextureShaderTemplateWriter.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MayaAnimatingTextureShaderTemplateWriter_H
#define INCLUDED_MayaAnimatingTextureShaderTemplateWriter_H

// ======================================================================

#include "MayaShaderTemplateWriter.h"

class Messenger;

// ======================================================================

class MayaAnimatingTextureShaderTemplateWriter: public MayaShaderTemplateWriter
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

    typedef stdvector<std::string>::fwd    TextureNameVector;

	static bool getMaterialDependencyNode( const MObject &shaderGroupObject,
                                           MFnDependencyNode &materialDependencyNode );

    static bool getFileNode( const MObject& shaderGroupObject,
                             MFnDependencyNode& fnFileNode );

    static bool findTextureSet( const MObject     &shaderGroupObject,
	                            const std::string         &textureReferenceDirectory,
	                            MayaUtility::TextureSet   &textureFilenames,
                                TextureNameVector         &relativeTextureNames,
						        CompressType compressType );

    static bool findTextureFiles( const std::string& filename,
                                  TextureNameVector& foundFiles );

private:

	static const int          cms_priority;
	static const int          cms_iffWriteSize;
	static const MString      cms_fpsMinAttribute;
	static const MString      cms_fpsMaxAttribute;
	static const MString      cms_orderAttribute;
	static const MString      cms_frameExtensionAttribute;

	static bool               ms_installed;
	static Messenger         *messenger;

};

// ======================================================================

#endif
