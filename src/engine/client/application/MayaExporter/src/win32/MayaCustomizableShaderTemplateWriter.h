// ======================================================================
//
// MayaCustomizableShaderTemplateWriter.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MayaCustomizableShaderTemplateWriter_H
#define INCLUDED_MayaCustomizableShaderTemplateWriter_H

// ======================================================================

#include "MayaShaderTemplateWriter.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class Messenger;
class MFnDependencyNode;
class MString;

// ======================================================================

class MayaCustomizableShaderTemplateWriter: public MayaShaderTemplateWriter
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

	class ColorChannelData;
	class SwappableTextureData;

	typedef stdvector<ColorChannelData>::fwd       ColorChannelDataVector;
	typedef stdvector<SwappableTextureData*>::fwd  SwappableTextureDataVector;
	typedef stdvector<int>::fwd                    IntVector;

private:

	static bool  getMaterialDependencyNode(const MObject &shaderGroupObject, MFnDependencyNode &materialDependencyNode);
	static bool  getTextureDependencyNode(const MFnDependencyNode &materialDependencyNode, MFnDependencyNode &textureDependencyNode, const MString &textureAttributeName);

	static bool  getPalettePathName(const MFnDependencyNode &dependencyNode, const MString &paletteAttributeName, std::string &paletteReferencePathName);
	static bool  getColorChannelData(const IntVector &colorChannelIndices, ColorChannelDataVector &colorChannelDataVector);

	static int   getTextureCount(const SwappableTextureDataVector &swappableTextureDataVector);

	static bool  collectMaterialCustomizations(const MFnDependencyNode &materialDependencyNode, ColorChannelDataVector &colorChannelDataVector);
	static bool  collectTextureCustomizations(const MFnDependencyNode &textureDependencyNode, const Tag &textureTag, SwappableTextureDataVector &swappableTextureDataVector);

	static bool  writeCustomizableShaderTemplate(const std::string                &shaderTemplateWriteName, 
	                                             const MObject                    &shaderGroupObject, 
	                                             bool                              hasVertexAlpha,
	                                             TextureRendererVector            &referencedTextureRenderers, 
	                                             const std::string                &textureReferenceDirectory,
	                                             const std::string                &textureRendererReferenceDirectory,
	                                             const std::string                &effectReferenceDirectory,
                                               MayaUtility::TextureSet          &textureFilenames,
                                               bool                              hasDot3TextureCoordinate,
                                               int                               dot3TextureCoordinateIndex,
	                                             const ColorChannelDataVector     &colorChannelDataVector,
	                                             const SwappableTextureDataVector &swappableTextureDataVector);

private:

	static const int          cms_priority;
	static const MString      cms_firstColorChannelAttributeName;
	static const MString      cms_secondColorChannelAttributeName;
	static const MString      cms_colorChannelDataNodeName;

	static const MString      cms_customizationVariableAttributeNameBase;
	static const MString      cms_customizationIsPrivateAttributeNameBase;
	static const MString      cms_paletteFileAttributeNameBase;
	static const MString      cms_defaultPaletteIndexAttributeNameBase;
	
	static const MString      cms_materialColorAttributeName;
	static const MString      cms_materialSecondHueTextureAttributeName;
	static const MString      cms_swappableTextureVariableNameAttributeName;
	static const MString      cms_swappableTexturePrivateAttributeName;
	static const MString      cms_swappableTextureDefaultIndexAttributeName;
	static const MString      cms_layeredTextureInputsAttributeName;
	static const MString      cms_texturePathNameAttributeName;

	static const char *const  cms_textureAttributeNamePrefix;

	static const std::string  cms_paletteReferenceDirectory;

	static bool               ms_installed;
	static Messenger         *messenger;

};

// ======================================================================

#endif
