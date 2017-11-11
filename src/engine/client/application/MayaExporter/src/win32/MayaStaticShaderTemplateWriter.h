// ======================================================================
//
// MayaStaticShaderTemplateWriter.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_StaticMayaShaderTemplateWriter_H
#define INCLUDED_StaticMayaShaderTemplateWriter_H

// ======================================================================

#include "MayaShaderTemplateWriter.h"

#include "MayaUtility.h"

class Iff;
class MColor;
class Messenger;
class MFnDependencyNode;
class VectorArgb;

namespace MayaStaticShaderTemplateWriterNamespace
{
	struct STextureInfo;
}

// ======================================================================

class MayaStaticShaderTemplateWriter: public MayaShaderTemplateWriter
{
public:

	static void  install(Messenger *messenger);
	static void  remove();

	static bool build(
		const MObject           &shaderGroupObject,
		bool                    hasVertexAlpha,
		TextureRendererVector   &referencedTextureRenderers,
		const std::string       &textureReferenceDirectory,
		const std::string       &textureRendererReferenceDirectory,
		const std::string       &effectReferenceDirectory,
		MayaUtility::TextureSet &textureFilenames,
		bool                    hasDot3TextureCoordinate,
		int                     dot3TextureCoordinateIndex
		);

	static const std::string &getEffectName()                { return ms_effectName; }

	static bool  writeToIff(const char *shaderTemplateWriteName);
	static bool  writeToIff(Iff &iff);

public:

	MayaStaticShaderTemplateWriter();
	virtual ~MayaStaticShaderTemplateWriter();

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

	//attribute adding functions
	static void setEffect(const std::string& effectName);
	static void addMaterialAmbient(const Tag& tag, const VectorArgb& ambientColor);
	static void addMaterialEmissive(const Tag& tag, const VectorArgb& emissiveColor);
	static void addMaterialSpecular(const Tag& tag, const VectorArgb& specularColor);
	static void addMaterialDiffuse(const Tag& tag, const VectorArgb& diffuseColor);
	static void addMaterialSpecularPower(const Tag& tag, float power);
	static void addTexture(const Tag& tag, const std::string& textureName, bool bWrapU, bool bWrapV);
	static void addTextureCoordinateSet(const Tag& tag, int setIndex, int dimension);
	static void addTextureFactor(const Tag& tag, const VectorArgb& color);
	static void addAlphaReference(const Tag& tag, int reference);
	static void addStencilReference(const Tag& tag, int reference);
	static void setCustomizable(bool isCustomizable);

	//other functions
	static bool buildAndWriteData(Iff &iff);
	static bool buildMAINMaterial(const MObject &shaderGroupObject);

	static bool buildTexture(const MObject           &shaderGroupObject, 
	                         TextureRendererVector   &referencedTextureRenderers, 
	                         const std::string       &textureReferenceDirectory, 
	                         const std::string       &textureRendererDirectory, 
	                         MayaUtility::TextureSet &textureFilenames,
	                         Tag                      tag,
	                         const std::string&       channelName,
							 CompressType compressType);

	static bool buildDefaultData(const MObject &shaderGroupObject, bool hasVertexAlpha, const std::string& effectReferenceDir, int numberOfTextures);
	static void reset();
	static MColor buildColorFromAttribute(const MObject& attribute, const MFnDependencyNode& node);


	static void writeTextureForm	( Iff & iff, const Tag & texTag, const std::string & texFilename, bool bWrapU, bool bWrapV );

private:
	///useful for sending messages to an output window (*must* be named "messenger" to match macros)
	static Messenger         *messenger;
	///is the module installed
	static bool               ms_installed;
	///the name of the effect selected (pulled from the shader group, attribute soe_effectName)
	static std::string        ms_effectName;
	///whether the shader is customizable or not (pulled from the shader group, attribute soe_customizable)
	static bool               ms_isCustomizable;


	// ----------

	///container holding all the textures and their tags 
	static stdmap<Tag, MayaStaticShaderTemplateWriterNamespace::STextureInfo>::fwd        ms_textureMap;

	// ----------

	//why not use a Material instead of multiple std::maps? Because we don't track specular, only get parts at a time, and don't really need a Material

	///container holding all the material ambient colors and their tags
	static stdmap<Tag, VectorArgb>::fwd          ms_materialAmbientMap;
	///container holding all the material emissive colors and their tags
	static stdmap<Tag, VectorArgb>::fwd          ms_materialEmissiveMap;
	///container holding all the material specular colors and their tags
	static stdmap<Tag, VectorArgb>::fwd          ms_materialSpecularMap;
	///container holding all the material diffuse colors and their tags
	static stdmap<Tag, VectorArgb>::fwd          ms_materialDiffuseMap;
	///container holding all the material powers and their tags
	static stdmap<Tag, real>::fwd                ms_materialPowerMap;
	///container holding all the texture coordinate sets and their tags
	static stdmap<Tag, std::pair<int, int> >::fwd ms_textureCoordinateSetMap;
	///container holding all the texture factors and their tags
	static stdmap<Tag, VectorArgb>::fwd           ms_textureFactorMap;
	///container holding all the alpha references and their tags
	static stdmap<Tag, int>::fwd                  ms_alphaReferenceMap;
	///container holding all the stencil references and their tags
	static stdmap<Tag, int>::fwd                  ms_stencilReferenceMap;
	///container holding every material tag defined, used to figure out what needs to be defaulted
	static stdset<Tag>::fwd                       ms_materialTags;
	///the default effect to use if none is assigned
	static const std::string ms_defaultEffectName;

};

// ======================================================================

#endif
