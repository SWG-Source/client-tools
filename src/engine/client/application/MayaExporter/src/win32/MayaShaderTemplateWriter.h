// ======================================================================
//
// MayaShaderTemplateWriter.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MayaShaderTemplateWriter_H
#define INCLUDED_MayaShaderTemplateWriter_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

#include "MayaUtility.h"

class MObject;

// ======================================================================
/**
 * An abstract base class containing the capability to write out a
 * specific ShaderTemplate format from a Maya ShaderGroup node.
 *
 * A MayaShaderTemplateWriter can perform two primary tasks: (1) deterimine
 * if a given Maya ShaderGroup node contains data for this ShaderTemplate
 * type, and (2) write out a ShaderTemplate given the ShaderGroup node.
 */

class MayaShaderTemplateWriter
{
public:

	MayaShaderTemplateWriter() {}
	virtual ~MayaShaderTemplateWriter() {}

	/**
	 * Indicates whether the specified ShaderGroupObject could be written
	 * as a ShaderTemplate type supported by this class.
	 *
	 * If this function returns true, the caller is guaranteed that 
	 * invoking write() can do something reasonable with the Maya Shader
	 * data.
	 *
	 * @return  true if this ShaderTemplate type should write out the
	 *          specified data; false otherwise.
	 */
	virtual bool  canWrite(const MObject &shaderGroupObject) const = 0;

	/**
	 * Write out the Maya ShaderGroup data into a file in the format
	 * handled by this ShaderTemplate type.
	 *
	 * @return  true if the shader template data was written successfully;
	 *          false if an error occurred either during ShaderGroup processing
	 *          or during write IO.
	 */
	virtual bool  write(const std::string       &shaderTemplateWriteName, 
	                    const MObject           &shaderGroupObject, 
	                    bool                     hasVertexAlpha,
	                    TextureRendererVector   &referencedTextureRenderers, 
	                    const std::string       &textureReferenceDirectory,
	                    const std::string       &textureRendererReferenceDirectory,
	                    const std::string       &effectReferenceDirectory,
	                    MayaUtility::TextureSet &textureFilenames,
											bool                     hasDot3TextureCoordinate,
											int                      dot3TextureCoordinateIndex) = 0;

};

// ======================================================================

#endif
