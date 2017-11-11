// ======================================================================
//
// MayaStaticShaderTemplateWriter.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "MayaStaticShaderTemplateWriter.h"

#include "ExporterLog.h"
#include "MayaShaderTemplateBuilder.h"
#include "MayaUtility.h"
#include "Messenger.h"

#include "clientGraphics/Material.h"
#include "maya/MColor.h"
#include "maya/MFnAttribute.h"
#include "maya/MFnBlinnShader.h"
#include "maya/MFnCompoundAttribute.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnLambertShader.h"
#include "maya/MFnPhongShader.h"
#include "maya/MFnStringData.h"
#include "maya/MObject.h"
#include "maya/MObjectArray.h"
#include "maya/MPlug.h"
#include "maya/MPlugArray.h"
#include "maya/MStatus.h"
#include "sharedFile/Iff.h"
#include "sharedMath/VectorArgb.h"

#include <map>
#include <string>

// ======================================================================

namespace MayaStaticShaderTemplateWriterNamespace
{
	const Tag TAG_ARVS = TAG(A,R,V,S); 
	const Tag TAG_CNRM = TAG(C,N,R,M);
	const Tag TAG_DOT3 = TAG(D,O,T,3);
	const Tag TAG_MAIN = TAG(M,A,I,N);
	const Tag TAG_MATS = TAG(M,A,T,S);
	const Tag TAG_NRML = TAG(N,R,M,L);
	const Tag TAG_SRVS = TAG(S,R,V,S);
	const Tag TAG_SSHT = TAG(S,S,H,T);
	const Tag TAG_TCSS = TAG(T,C,S,S);
	const Tag TAG_TFNS = TAG(T,F,N,S);
	const Tag TAG_TXMS = TAG(T,X,M,S);

	// These need to be kept up to date with the currently supported graphics layer.
	int const cs_maxTextureCoordinateSetDimensionality = 4;
	int const cs_maxTextureCoordinateSetCount          = 8;

	// ======================================================================

	struct STextureInfo
	{
		std::string  mFilename;
		bool         mWrapU;
		bool         mWrapV;
	};

	// ======================================================================

	enum TextureAddress
	{
		TA_wrap,
		TA_mirror,
		TA_clamp,
		TA_border,
		TA_mirrorOnce,

		TA_invalid
	};

	// ======================================================================

	enum TextureFilter
	{
		TF_none,
		TF_point,
		TF_linear,
		TF_anisotropic,
		TF_flatCubic,
		TF_gaussianCubic,

		TF_invalid
	};

	// ======================================================================

	typedef std::map<Tag, STextureInfo> TTextureInfoMap;

	// ======================================================================
}

using namespace MayaStaticShaderTemplateWriterNamespace;

Messenger *                         MayaStaticShaderTemplateWriter::messenger;
bool                                MayaStaticShaderTemplateWriter::ms_installed;

std::string                         MayaStaticShaderTemplateWriter::ms_effectName;
bool                                MayaStaticShaderTemplateWriter::ms_isCustomizable;
TTextureInfoMap                     MayaStaticShaderTemplateWriter::ms_textureMap;
std::map<Tag, VectorArgb>           MayaStaticShaderTemplateWriter::ms_materialAmbientMap;
std::map<Tag, VectorArgb>           MayaStaticShaderTemplateWriter::ms_materialEmissiveMap;
std::map<Tag, VectorArgb>           MayaStaticShaderTemplateWriter::ms_materialSpecularMap;
std::map<Tag, VectorArgb>           MayaStaticShaderTemplateWriter::ms_materialDiffuseMap;
std::map<Tag, float>                MayaStaticShaderTemplateWriter::ms_materialPowerMap;
std::map<Tag, std::pair<int, int> > MayaStaticShaderTemplateWriter::ms_textureCoordinateSetMap;
std::map<Tag, VectorArgb>           MayaStaticShaderTemplateWriter::ms_textureFactorMap;
std::map<Tag, int>                  MayaStaticShaderTemplateWriter::ms_alphaReferenceMap;
std::map<Tag, int>                  MayaStaticShaderTemplateWriter::ms_stencilReferenceMap;
std::set<Tag>                       MayaStaticShaderTemplateWriter::ms_materialTags;

const std::string                   MayaStaticShaderTemplateWriter::ms_defaultEffectName = "effect\\a_simple.eft";

// ======================================================================

void MayaStaticShaderTemplateWriter::install(Messenger *theMessenger)
{
	messenger    = theMessenger;
	ms_installed = true;

	MayaShaderTemplateBuilder::registerShaderTemplateWriter(new MayaStaticShaderTemplateWriter, 0);

	reset();
}

// ----------------------------------------------------------------------

void MayaStaticShaderTemplateWriter::remove()
{
	messenger    = 0;
	ms_installed = false;
}

// ----------------------------------------------------------------------

bool MayaStaticShaderTemplateWriter::build(
	const MObject           &shaderGroupObject,
	bool                     hasVertexAlpha,
	TextureRendererVector   &referencedTextureRenderers,
	const std::string       &textureReferenceDirectory,
	const std::string       &textureRendererReferenceDirectory,
	const std::string       &effectReferenceDirectory,
	MayaUtility::TextureSet &textures,
	bool                     hasDot3TextureCoordinate,
	int                      dot3TextureCoordinateIndex
)
{
	reset();

	DEBUG_FATAL(!ms_installed,("MayaStaticShaderTemplateWriter not installed"));
	MESSENGER_REJECT(shaderGroupObject.apiType() != MFn::kShadingEngine,("getShaderGroupMaterial() expecting object of type MFn::kShaderEngine, found type %s\n", shaderGroupObject.apiTypeStr()));

	//allocate stuff to avoid repeated allocations in the main loop
	MStatus           status;
	MFnAttribute      attribute;
	MObject           attributeObject;
	MPlug             plug;
	MObjectArray      objectArray;
	MFnDependencyNode shaderDependencyNode(shaderGroupObject, &status);
	MESSENGER_REJECT(!status,("failed to set SG object for shaderDependencyNode,\"%s\"\n", status.errorString().asChar()));

	bool result = MayaUtility::findSourceObjects(shaderDependencyNode, "surfaceShader", &objectArray);
	MESSENGER_REJECT(!result,("failed to get shader connected to shader group \"%s\"\n", shaderDependencyNode.name().asChar()));
	MESSENGER_REJECT(objectArray.length() != 1,("expecting surfaceShader to have one upstream connection, found %d\n", objectArray.length()));

	//get MAIN texture, material, from other nodes
	result = buildMAINMaterial(shaderGroupObject);
	MESSENGER_REJECT(!result,("could not get MAIN material"));

	//get the MAIN texture from the color channel
	result = buildTexture(shaderGroupObject, referencedTextureRenderers, textureReferenceDirectory, textureRendererReferenceDirectory, textures, TAG_MAIN, "color", CT_texture);
	MESSENGER_REJECT(!result,("could not get MAIN texture(s)"));

	MFnLambertShader lambert(objectArray[0], &status);
	MESSENGER_REJECT(!status,("failed to set lambert shader object for shaderDependencyNode,\"%s\"\n", status.errorString().asChar()));

	const std::string &attributePrefix = MayaUtility::getAttributePrefix();
	const std::string &mayaShaderBuilderVersion = MayaUtility::getMayaShaderBuilderVersion();

	//get all the attributes, and for each one, build the appropriate part of the shader template
	const unsigned attributeCount = lambert.attributeCount(&status);
	MESSENGER_REJECT(!status,("failed to get attribute count for shader attributes\n"));
	for(unsigned i = 0; i < attributeCount; ++i)
	{
		//get the attribute
		attributeObject = lambert.attribute(i, &status);
		MESSENGER_REJECT(!status,("failed to get attribute [%u]\n", i));
		status = attribute.setObject(attributeObject);
		MESSENGER_REJECT(!status,("failed to set object into MFnAttribute\n"));
		std::string attributeName =(attribute.name(&status)).asChar();
		MESSENGER_REJECT(!status,("failed to get attribute name\n"));

		//is it one of our attributes?
		if (attributeName.find(attributePrefix) != std::string::npos)
		{
			MFnPhongShader phong(objectArray[0], &status);
			MESSENGER_REJECT(!status,("failed to set phong shader object for shaderDependencyNode,\"%s\"\n", status.errorString().asChar()));
			const MObject phongObject = phong.object();
			MFnDependencyNode materialDependencyNode(phongObject, &status);
			MESSENGER_REJECT(!status,("failed to set material object for shaderDependencyNode,\"%s\"\n", status.errorString().asChar()));
			// get the value for this attribute
			plug = materialDependencyNode.findPlug(attributeObject, &status);
			MESSENGER_REJECT_WARNING(!status,("failed to find plug for attribute [%s]\n", attributeName.c_str()));

			//ok, it is, now which one is it?
			if(attributeName.find("exportVersion") != std::string::npos)
			{
				MString value;
				status = plug.getValue(value);
				MESSENGER_REJECT(!status,("failed to get string value for attribute [%s]\n", attributeName.c_str()));
				std::string strValue = value.asChar();
				//if the wrong version of the MEL tool was used, quit
				MESSENGER_REJECT(strValue != mayaShaderBuilderVersion,("An outdated version of the MEL mayaShaderBuilder tool was used, update tool and exporter and try again"));
			}

			////////////

			else if(attributeName.find("effectName") != std::string::npos)
			{
				MString value;
				status = plug.getValue(value);
				MESSENGER_REJECT(!status,("failed to get string value for attribute [%s]\n", attributeName.c_str()));
				setEffect(value.asChar());
			}

			////////////

			else if(attributeName.find("ambientRGB") != std::string::npos)
			{
			//we iterate over the components (i.e. ...diffusreRGB_r, while we only want to process the parent one, ...diffuseRGB)
				if(attributeName.find("_r") != std::string::npos 
				|| attributeName.find("_g") != std::string::npos 
				|| attributeName.find("_b") != std::string::npos)
					continue;

				//additional material ambient component
				MESSENGER_REJECT(!attribute.isUsedAsColor(), ("the ambient color attribute isn't used as a color"));
				bool result = plug.isConnected(&status);
				MESSENGER_REJECT(result || !status, ("ambient color plug connected or unavailable"));
				const std::string prefix = attributePrefix + "ambientRGB_";
				const std::string tagStr = MayaUtility::getTagFromAttributeName(attributeName, prefix);
				Tag tag = MayaUtility::convertStringtoTag(tagStr);
				MColor color = buildColorFromAttribute(attributeObject, phong);
				addMaterialAmbient(tag, VectorArgb(1.0, color.r, color.g, color.b));
			}

			////////////

			else if(attributeName.find("specularRGB") != std::string::npos)
			{
			//we iterate over the components (i.e. ...diffusreRGB_r, while we only want to process the parent one, ...diffuseRGB)
				if(attributeName.find("_r") != std::string::npos 
				|| attributeName.find("_g") != std::string::npos 
				|| attributeName.find("_b") != std::string::npos)
					continue;

				//additional material specular component
				MESSENGER_REJECT(!attribute.isUsedAsColor(), ("the specular color attribute isn't used as a color"));
				bool result = plug.isConnected(&status);
				MESSENGER_REJECT(result || !status, ("specular color plug connected or unavailable"));
				const std::string prefix = attributePrefix + "specularRGB_";
				const std::string tagStr = MayaUtility::getTagFromAttributeName(attributeName, prefix);
				Tag tag = MayaUtility::convertStringtoTag(tagStr);
				MColor color = buildColorFromAttribute(attributeObject, phong);
				addMaterialSpecular(tag, VectorArgb(1.0, color.r, color.g, color.b));
			}

			////////////

			else if(attributeName.find("emissiveRGB") != std::string::npos)
			{
			//we iterate over the components (i.e. ...diffusreRGB_r, while we only want to process the parent one, ...diffuseRGB)
				if(attributeName.find("_r") != std::string::npos 
				|| attributeName.find("_g") != std::string::npos 
				|| attributeName.find("_b") != std::string::npos)
					continue;

				//additional material emissive component
				MESSENGER_REJECT(!attribute.isUsedAsColor(), ("the emissive color attribute isn't used as a color"));
				bool result = plug.isConnected(&status);
				MESSENGER_REJECT(result || !status, ("emissive color plug connected or unavailable"));
				const std::string prefix = attributePrefix + "emissiveRGB_";
				const std::string tagStr = MayaUtility::getTagFromAttributeName(attributeName, prefix);
				Tag tag = MayaUtility::convertStringtoTag(tagStr);
				MColor color = buildColorFromAttribute(attributeObject, phong);
				addMaterialEmissive(tag, VectorArgb(1.0, color.r, color.g, color.b));
			}

			////////////

			else if(attributeName.find("diffuseRGB") != std::string::npos)
			{
			//we iterate over the components (i.e. ...diffusreRGB_r, while we only want to process the parent one, ...diffuseRGB)
				if(attributeName.find("_r") != std::string::npos 
				|| attributeName.find("_g") != std::string::npos 
				|| attributeName.find("_b") != std::string::npos)
					continue;

				//additional material diffuse component
				MESSENGER_REJECT(!attribute.isUsedAsColor(), ("the diffuse color attribute isn't used as a color"));
				bool result = plug.isConnected(&status);
				MESSENGER_REJECT(result || !status, ("diffuse color plug connected or unavailable"));
				const std::string prefix = attributePrefix + "diffuseRGB_";
				const std::string tagStr = MayaUtility::getTagFromAttributeName(attributeName, prefix);
				Tag tag = MayaUtility::convertStringtoTag(tagStr);
				MColor color = buildColorFromAttribute(attributeObject, phong);
				addMaterialDiffuse(tag, VectorArgb(1.0, color.r, color.g, color.b));
			}

			////////////

			else if(attributeName.find("textureName") != std::string::npos)
			{
				const std::string prefix = attributePrefix + "textureName_";
				const std::string tagStr = MayaUtility::getTagFromAttributeName(attributeName, prefix);
				Tag tag = MayaUtility::convertStringtoTag(tagStr);
				MString value;
				status = plug.getValue(value);

				CompressType compressType = CT_texture;
				//if we have a normal map (tag NRML) then do NOT compress the texture
				if(tag == TAG_NRML)
				{
					compressType = CT_none;
				}
				else if(tag == TAG_CNRM) // do compress normal maps w/tag CNMR
				{
					compressType = CT_normalMap;
				}

				//see if we use a plug here or not
				if (status)
				{
					//no plug, just use the texture name
					addTexture(tag, value.asChar(), false, false);
					//it's not a texture we can actually export, so don't add to the ExporterLog
				}
				else
				{
					//we use a plug, dive into it and pull the real texture
					result = buildTexture(
						shaderGroupObject, 
						referencedTextureRenderers, 
						textureReferenceDirectory, 
						textureRendererReferenceDirectory, 
						textures, 
						tag, 
						attributeName, 
						compressType
						);

					MESSENGER_REJECT(!result,("could not get texture"));
				}
			}

			////////////

			else if(attributeName.find("texFactor") != std::string::npos)
			{
			//we iterate over the components (i.e. ...texFactor_r, while we only want to process the parent one, ...texFactor)
				if(attributeName.find("_r") != std::string::npos 
				|| attributeName.find("_g") != std::string::npos 
				|| attributeName.find("_b") != std::string::npos)
					continue;

				const std::string prefix = attributePrefix + "texFactor_";
				const std::string tagStr = MayaUtility::getTagFromAttributeName(attributeName, prefix);
				Tag tag = MayaUtility::convertStringtoTag(tagStr);
				if(!attribute.isUsedAsColor())
					continue;
				bool result = plug.isConnected(&status);
				MESSENGER_REJECT(result || !status, ("texture factor color plug connected or unavailable"));

				MColor color = buildColorFromAttribute(attributeObject, phong);
				addTextureFactor(tag, VectorArgb(1.0, color.r, color.g, color.b));
			}

			////////////

			else if(attributeName.find("texCoordSet") != std::string::npos)
			{
			//we iterate over the components (i.e. ...texCoordSet_uv, while we only want to process the parent one, ...texCoordSet)
				if(attributeName.find("_uv") != std::string::npos 
				|| attributeName.find("_dim") != std::string::npos)
					continue;

				//get the compound attribute
				MFnCompoundAttribute compoundAttribute;
				MStatus status = compoundAttribute.setObject(attributeObject);
				MESSENGER_REJECT(!status,("failed to set object into MFnCompoundAttribute\n"));
				//validate that we have 2 children (uv set to use, then the dimension)
				int numChildren = compoundAttribute.numChildren();
				MESSENGER_REJECT(numChildren != 2, ("bad number of children for texCoordSet attribute"));

				//get the uv value, from a plug
				MObject uvObject = compoundAttribute.child(0, &status);
				MESSENGER_REJECT(!status, ("could not get uv object\n"));
				MPlug uvPlug = phong.findPlug(uvObject, &status);
				MESSENGER_REJECT(!status, ("could not get uv plug\n"));
				int uvValue = 0;
				status = uvPlug.getValue(uvValue);
				MESSENGER_REJECT(!status, ("could not get value from uv plug\n"));
				MESSENGER_REJECT((uvValue < 0) || (uvValue >= cs_maxTextureCoordinateSetCount), ("texture coordinate set index [%d] out of valid range [0..%d]", uvValue, cs_maxTextureCoordinateSetCount - 1));

				//get the dimension value, from a plug
				MObject dimObject = compoundAttribute.child(1, &status);
				MESSENGER_REJECT(!status, ("could not get dim object\n"));
				MPlug dimPlug = phong.findPlug(dimObject, &status);
				MESSENGER_REJECT(!status, ("could not get dim plug\n"));
				int dimValue = 0;
				status = dimPlug.getValue(dimValue);
				MESSENGER_REJECT(!status, ("could not get value from dim plug\n"));
				MESSENGER_REJECT((dimValue < 1) || (dimValue > cs_maxTextureCoordinateSetDimensionality), ("texture coordinate set dimensionality [%d] out of valid range [1..%d]", dimValue, cs_maxTextureCoordinateSetDimensionality));

				const std::string prefix = attributePrefix + "texCoordSet_";
				const std::string tagStr = MayaUtility::getTagFromAttributeName(attributeName, prefix);
				Tag tag = MayaUtility::convertStringtoTag(tagStr);
				addTextureCoordinateSet(tag, uvValue, dimValue);

			}

			////////////

			else if(attributeName.find("alphaRef") != std::string::npos)
			{
				int value  = 0;
				status     = plug.getValue(value);
				MESSENGER_REJECT(!status,("failed to get long value for attribute [%s]\n", attributeName.c_str()));
				const std::string prefix = attributePrefix + "alphaRef_";
				const std::string tagStr = MayaUtility::getTagFromAttributeName(attributeName, prefix);
				Tag tag = MayaUtility::convertStringtoTag(tagStr);
				addAlphaReference(tag, value);
			}

			////////////

			else if(attributeName.find("stencilRef") != std::string::npos)
			{
				int value  = 0;
				status     = plug.getValue(value);
				MESSENGER_REJECT(!status,("failed to get long value for attribute [%s]\n", attributeName.c_str()));
				const std::string prefix = attributePrefix + "stencilRef_";
				const std::string tagStr = MayaUtility::getTagFromAttributeName(attributeName, prefix);
				Tag tag = MayaUtility::convertStringtoTag(tagStr);
				addStencilReference(tag, value);
			}

			////////////

			else if(attributeName.find("customizable") != std::string::npos)
			{
				//nothing needs to be done
			}

			////////////

			else if(attributeName.find("cosinepower") != std::string::npos)
			{
				//nothing needs to be done
			}

			////////////

			else if(attributeName.find("effectVersion") != std::string::npos)
			{
				//nothing needs to be done, this field is used by the artists and the MEL script
			}

			////////////

			else if(attributeName.find("colorChannelA") != std::string::npos)
			{
				//skip, used by CustomizableShaderTemplateWriter
			}

			////////////

			else if(attributeName.find("colorChannelB") != std::string::npos)
			{
				//skip, used by CustomizableShaderTemplateWriter
			}

			////////////

			else if(attributeName.find("animatingFps") != std::string::npos)
			{
				//skip, used by AnimatingTextureShaderTemplateWriter
			}

			////////////

			else if(attributeName.find("heightMapScale") != std::string::npos)
			{
				//skip, used by AnimatingTextureShaderTemplateWriter
			}

			////////////

			else
			{
				//it's one we don't currently handle
				MESSENGER_REJECT(true,("encountered unknown attribute when building StaticShaderTemplate, aborting [%s].\n", attributeName.c_str()));
			}
		}
	}


	// Handle writing dot3 bump mapping coordinate set info.
	if (hasDot3TextureCoordinate)
		addTextureCoordinateSet(TAG_DOT3, dot3TextureCoordinateIndex, 4);

	int numberOfTextures = 1;
	//build the default information if necessary (if the artist didn't assign a specialized effect)
	if(ms_effectName == "")
		buildDefaultData(shaderGroupObject, hasVertexAlpha, effectReferenceDirectory, numberOfTextures);

	return true;
}
// ======================================================================

MayaStaticShaderTemplateWriter::MayaStaticShaderTemplateWriter() :
	MayaShaderTemplateWriter()
{
}

// ----------------------------------------------------------------------

MayaStaticShaderTemplateWriter::~MayaStaticShaderTemplateWriter()
{
}

// ----------------------------------------------------------------------

bool MayaStaticShaderTemplateWriter::canWrite(const MObject & /* shaderGroupObject */) const
{
	// for now, assume we can write something reasonable for any kind of
	// shader data.  It is expected that the StaticShader handler will be
	// the lowest priority MayaShaderTemplateWriter, writing all it knows
	// about from the shader group object.
	return true;
}

// ----------------------------------------------------------------------

bool MayaStaticShaderTemplateWriter::write(
	const std::string       &shaderTemplateWriteName, 
	const MObject           &shaderGroupObject, 
	bool                     hasVertexAlpha,
	TextureRendererVector   &referencedTextureRenderers, 
	const std::string       &textureReferenceDirectory,
	const std::string       &textureRendererReferenceDirectory,
	const std::string       &effectReferenceDirectory,
	MayaUtility::TextureSet &textures,
	bool                     hasDot3TextureCoordinate,
	int                      dot3TextureCoordinateIndex
	)
{
	MESSENGER_LOG(("generating shader template file \"%s\"\n", shaderTemplateWriteName.c_str()));

	// ----------------------------------------------------------------------
	//-- fill the iff with the StaticShaderTemplate data
	bool writeToIffSuccess = build(
		shaderGroupObject,
		hasVertexAlpha,
		referencedTextureRenderers,
		textureReferenceDirectory,
		textureRendererReferenceDirectory,
		effectReferenceDirectory,
		textures,
		hasDot3TextureCoordinate,
		dot3TextureCoordinateIndex
	);
	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	if (writeToIffSuccess)
	{
		writeToIffSuccess = writeToIff(shaderTemplateWriteName.c_str());
	}
	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	//-- now that we've built all the data, archive it to the log file
	ExporterLog::addDestinationShaderToCurrentMesh(shaderTemplateWriteName, ms_effectName);

/*
	for(StringSet::iterator i = textures.begin(); i != textures.end(); ++i)
	{
		ExporterLog::addDestinationTextureToCurrentShader(i->first);
	}
*/
	// ----------------------------------------------------------------------

	//-- success
	return true;
}

// ======================================================================
// class MayaStaticShaderTemplateWriter private static member functions
// ======================================================================

void MayaStaticShaderTemplateWriter::reset()
{
	ms_effectName = "";
	ms_isCustomizable = false;
	ms_textureMap.clear();
	ms_materialAmbientMap.clear();
	ms_materialEmissiveMap.clear();
	ms_materialSpecularMap.clear();
	ms_materialDiffuseMap.clear();
	ms_materialPowerMap.clear();
	ms_textureCoordinateSetMap.clear();
	ms_textureFactorMap.clear();
	ms_alphaReferenceMap.clear();
	ms_stencilReferenceMap.clear();
	ms_materialTags.clear();
}

// ----------------------------------------------------------------------

void MayaStaticShaderTemplateWriter::setEffect(const std::string& effectName)
{
	ms_effectName = effectName;
}

// ----------------------------------------------------------------------

void MayaStaticShaderTemplateWriter::addMaterialAmbient(const Tag& tag, const VectorArgb& ambientColor)
{
	ms_materialAmbientMap[tag] = ambientColor;
	//add the new material tag to a set, so that we know to generate defaults for pieces not defined by Maya
	ms_materialTags.insert(tag);
}

// ----------------------------------------------------------------------

void MayaStaticShaderTemplateWriter::addMaterialSpecular(const Tag& tag, const VectorArgb& specularColor)
{
	ms_materialSpecularMap[tag] = specularColor;
	//add the new material tag to a set, so that we know to generate defaults for pieces not defined by Maya
	ms_materialTags.insert(tag);
}

// ----------------------------------------------------------------------

void MayaStaticShaderTemplateWriter::addMaterialSpecularPower(const Tag& tag, float power)
{
	ms_materialPowerMap[tag] = power;
	//add the new material tag to a set, so that we know to generate defaults for pieces not defined by Maya
	ms_materialTags.insert(tag);
}

// ----------------------------------------------------------------------

void MayaStaticShaderTemplateWriter::addMaterialEmissive(const Tag& tag, const VectorArgb& emissiveColor)
{
	ms_materialEmissiveMap[tag] = emissiveColor;
	//add the new material tag to a set, so that we know to generate defaults for pieces not defined by Maya
	ms_materialTags.insert(tag);
}

// ----------------------------------------------------------------------

void MayaStaticShaderTemplateWriter::addMaterialDiffuse(const Tag& tag, const VectorArgb& diffuseColor)
{
	ms_materialDiffuseMap[tag] = diffuseColor;
	//add the new material tag to a set, so that we know to generate defaults for pieces not defined by Maya
	ms_materialTags.insert(tag);
}

// ----------------------------------------------------------------------

void MayaStaticShaderTemplateWriter::addTexture(
		const Tag         &tag, 
		const std::string &textureName,
		bool               bWrapU,
		bool               bWrapV)
{
	STextureInfo S;

	S.mFilename = textureName;
	S.mWrapU = bWrapU;
	S.mWrapV = bWrapV;

	ms_textureMap[tag] = S;
}

// ----------------------------------------------------------------------

void MayaStaticShaderTemplateWriter::addTextureCoordinateSet(const Tag& tag, int setIndex, int dimension)
{
	ms_textureCoordinateSetMap[tag] = std::make_pair(setIndex, dimension);
}

// ----------------------------------------------------------------------

void MayaStaticShaderTemplateWriter::addTextureFactor(const Tag& tag, const VectorArgb& color)
{
	ms_textureFactorMap[tag] = color;
}

// ----------------------------------------------------------------------

void MayaStaticShaderTemplateWriter::addAlphaReference(const Tag& tag, int reference)
{
	ms_alphaReferenceMap[tag] = reference;
}

// ----------------------------------------------------------------------

void MayaStaticShaderTemplateWriter::addStencilReference(const Tag& tag, int reference)
{
	ms_stencilReferenceMap[tag] = reference;
}

// ----------------------------------------------------------------------

void MayaStaticShaderTemplateWriter::setCustomizable(bool isCustomizable)
{
	ms_isCustomizable = isCustomizable;
}

// ----------------------------------------------------------------------
// Texture info - version 0001

void	MayaStaticShaderTemplateWriter::writeTextureForm ( Iff & iff, 
	                          const Tag & texTag, 
	                          const std::string & texFilename, 
	                          bool bWrapU, 
	                          bool bWrapV )
{
	TextureAddress wrapU = (bWrapU ? TA_wrap : TA_clamp);
	TextureAddress wrapV = (bWrapV ? TA_wrap : TA_clamp);
	TextureAddress wrapW = TA_wrap;	// not supported in the exporter yet?

	TextureFilter filterMag = TF_linear;	// not supported
	TextureFilter filterMin = TF_linear;	// not supported
	TextureFilter filterMip = TF_linear;	// not supported

	// ----------

	const Tag TAG_TXM = TAG3(T,X,M); 
	iff.insertForm(TAG_TXM);
		iff.insertForm(TAG_0001);

			iff.insertChunk(TAG_DATA);
				iff.insertChunkData(texTag);
				iff.insertChunkData(static_cast<uint8>(0));				// placeholder flag - this is not a placeholder
				iff.insertChunkData(static_cast<uint8>(wrapU));
				iff.insertChunkData(static_cast<uint8>(wrapV));
				iff.insertChunkData(static_cast<uint8>(wrapW));
				iff.insertChunkData(static_cast<uint8>(filterMip));
				iff.insertChunkData(static_cast<uint8>(filterMin));
				iff.insertChunkData(static_cast<uint8>(filterMag));
			iff.exitChunk(TAG_DATA);

			iff.insertChunk(TAG_NAME);
				iff.insertChunkString(texFilename.c_str());
			iff.exitChunk(TAG_NAME);

		iff.exitForm(TAG_0001);
	iff.exitForm(TAG_TXM);
}

// ----------------------------------------------------------------------
/**
 * Take the data collected by this module, and write it to the given filename
 */
bool MayaStaticShaderTemplateWriter::writeToIff(const char *shaderTemplateWriteName)
{ 
	Iff iff(16 * 1024);
	bool result;

	result = buildAndWriteData(iff); 
	MESSENGER_REJECT(!result, ("failed to generate data for StaticShaderTemplate [%s].", shaderTemplateWriteName));

	//-- write the iff to disk
	if (result)
	{
		result = iff.write(shaderTemplateWriteName, true);
		MESSENGER_REJECT(!result, ("failed to write StaticShaderTemplate [%s] to disk.", shaderTemplateWriteName));
	}

	return result;
}

bool MayaStaticShaderTemplateWriter::writeToIff(Iff &iff)
{
	return buildAndWriteData(iff);
}

bool MayaStaticShaderTemplateWriter::buildAndWriteData(Iff &iff)
{

	MESSENGER_LOG(("shader template uses effect \"%s\"\n", ms_effectName.c_str()));

	iff.insertForm(TAG_SSHT);
	{
		iff.insertForm(TAG_0000);
		{
			//write out all the materials
			if(ms_materialTags.size() > 0)
			{
				iff.insertForm(TAG_MATS);
					iff.insertForm(TAG_0000);
					//for every material tag defined (i.e. at least part of it was defined by the Maya file) build the material and default what's needed
					for(std::set<Tag>::iterator it = ms_materialTags.begin(); it != ms_materialTags.end(); ++it)
					{
						Tag tag = (*it);

						VectorArgb diffuseColor;
						if(ms_materialDiffuseMap.find(tag) == ms_materialDiffuseMap.end())
						{
							//create a default while one if it doesn't exist
							diffuseColor.a = 1.0;
							diffuseColor.r = 1.0;
							diffuseColor.g = 1.0;
							diffuseColor.b = 1.0;
						}
						else
						{
							diffuseColor = ms_materialDiffuseMap[tag];
						}

						VectorArgb ambientColor;
						if(ms_materialAmbientMap.find(tag) == ms_materialAmbientMap.end())
						{
							//create a default while one if it doesn't exist
							ambientColor.a = 1.0;
							ambientColor.r = 1.0;
							ambientColor.g = 1.0;
							ambientColor.b = 1.0;
						}
						else
						{
							ambientColor = ms_materialAmbientMap[tag];
						}

						VectorArgb emissiveColor;
						if(ms_materialEmissiveMap.find(tag) == ms_materialEmissiveMap.end())
						{
							//create a default while one if it doesn't exist
							emissiveColor.a = 0.0;
							emissiveColor.r = 0.0;
							emissiveColor.g = 0.0;
							emissiveColor.b = 0.0;
						}
						else
						{
							emissiveColor = ms_materialEmissiveMap[tag];
						}

						VectorArgb specularColor;
						if(ms_materialSpecularMap.find(tag) == ms_materialSpecularMap.end())
						{
							//create a default while one if it doesn't exist
							specularColor.a = 1.0;
							specularColor.r = 1.0;
							specularColor.g = 1.0;
							specularColor.b = 1.0;
						}
						else
						{
							specularColor = ms_materialSpecularMap[tag];
						}

						real specularPower = 20.0;
						if(ms_materialPowerMap.find(tag) != ms_materialPowerMap.end())
						{
							specularPower = ms_materialPowerMap[tag];
						}

						Material material(ambientColor, diffuseColor, emissiveColor, specularColor, specularPower);
						iff.insertChunk(TAG3(T,A,G));
							iff.insertChunkData(tag);
						iff.exitChunk(TAG3(T,A,G));
						material.write(iff);
					}
					iff.exitForm(TAG_0000);
				iff.exitForm(TAG_MATS);
			}

			//write out all the textures
			if(ms_textureMap.size() > 0)
			{
				iff.insertForm(TAG_TXMS);
					for(TTextureInfoMap::iterator it = ms_textureMap.begin(); it != ms_textureMap.end(); ++it)
					{
						std::string::size_type pos = it->second.mFilename.find("*.dds");
						MESSENGER_REJECT(pos != std::string::npos, ("At least one texture attribute has not been filled in, i.e. it is *.dds"));

						writeTextureForm( iff,
										  it->first,
										  it->second.mFilename, 
										  it->second.mWrapU, 
										  it->second.mWrapV );
					}
				iff.exitForm(TAG_TXMS);
			}

			//put in a default coordinate set if there isn't a MAIN defined one
			if(ms_textureCoordinateSetMap.find(TAG_MAIN) == ms_textureCoordinateSetMap.end())
				addTextureCoordinateSet(TAG_MAIN, 0, 2);

			//write out all the texture coordinate sets
			iff.insertForm(TAG_TCSS);
				iff.insertChunk(TAG_0000);
					for(std::map<Tag, std::pair<int, int> >::iterator it = ms_textureCoordinateSetMap.begin(); it != ms_textureCoordinateSetMap.end(); ++it)
					{
						iff.insertChunkData(it->first);
						iff.insertChunkData(static_cast<uint8>(it->second.first));
					}
				iff.exitChunk(TAG_0000);
			iff.exitForm(TAG_TCSS);

			//write out all the texture factors
			if(ms_textureFactorMap.size() > 0)
			{
				iff.insertForm(TAG_TFNS);
					iff.insertChunk(TAG_0000);
						for(std::map<Tag, VectorArgb>::iterator it = ms_textureFactorMap.begin(); it != ms_textureFactorMap.end(); ++it)
						{
							iff.insertChunkData(it->first);
							iff.insertChunkData(it->second.convertToUint32());
						}
					iff.exitChunk(TAG_0000);
				iff.exitForm(TAG_TFNS);
			}

			//write all the alpha reference values
			if(ms_alphaReferenceMap.size() > 0)
			{
				iff.insertForm(TAG_ARVS);
					iff.insertChunk(TAG_0000);
						for(std::map<Tag, int>::iterator it = ms_alphaReferenceMap.begin(); it != ms_alphaReferenceMap.end(); ++it)
						{
							iff.insertChunkData(it->first);
							iff.insertChunkData(static_cast<uint8>(it->second));
						}
					iff.exitChunk(TAG_0000);
				iff.exitForm(TAG_ARVS);
			}
		
			//write all the stencil reference values
			if(ms_stencilReferenceMap.size() > 0)
			{
				iff.insertForm(TAG_SRVS);
					iff.insertChunk(TAG_0000);
						for(std::map<Tag, int>::iterator it = ms_stencilReferenceMap.begin(); it != ms_stencilReferenceMap.end(); ++it)
						{
							iff.insertChunkData(it->first);
							iff.insertChunkData(static_cast<uint8>(it->second));
						}
					iff.exitChunk(TAG_0000);
				iff.exitForm(TAG_SRVS);
			}

			//write the effect name
			iff.insertChunk(TAG_NAME);
				//use the default if none is assigned
				if(ms_effectName == "")
					iff.insertChunkString(ms_defaultEffectName.c_str());
				else

					iff.insertChunkString(ms_effectName.c_str());
			iff.exitChunk(TAG_NAME);
		}
		iff.exitForm(TAG_0000);
	}
	iff.exitForm(TAG_SSHT);

	return true;
}

// ----------------------------------------------------------------------
/**
 * Pull the properties from the maya material for the MAIN tag
 */

bool MayaStaticShaderTemplateWriter::buildMAINMaterial(const MObject& shaderGroupObject)
{
	DEBUG_FATAL(!ms_installed,("MayaStaticShaderTemplateWriter not installed"));
	MESSENGER_REJECT(shaderGroupObject.apiType() != MFn::kShadingEngine,("getShaderGroupMaterial() expecting object of type MFn::kShaderEngine, found type %s\n", shaderGroupObject.apiTypeStr()));

	//get the shader node attached to the shader group
	bool               result;
	MStatus            status;
	MObjectArray       objectArray;
	MFnDependencyNode  fnDepNode(shaderGroupObject, &status);
	MESSENGER_REJECT(!status,("failed to set SG object for fnDepNode,\"%s\"\n", status.errorString().asChar()));

	Tag mainTag = TAG_MAIN;

	result = MayaUtility::findSourceObjects(fnDepNode, "surfaceShader", &objectArray);
	MESSENGER_REJECT(!result,("failed to get shader connected to shader group \"%s\"\n", fnDepNode.name().asChar()));
	MESSENGER_REJECT(objectArray.length() != 1,("expecting surfaceShader to have one upstream connection, found %d\n", objectArray.length()));

	status = fnDepNode.setObject(objectArray[0]);
	MESSENGER_REJECT(!status,("failed to set shader object for fnDepNode,\"%s\"\n", status.errorString().asChar()));
	
	MString mayaTypeName = fnDepNode.typeName(&status);
	const char* typeName = mayaTypeName.asChar();
	const MFnLambertShader lambert(objectArray[0], &status);
	MESSENGER_REJECT(!status,("failed to set lambert shader object for fnDepNode,\"%s\"\n", status.errorString().asChar()));

	//set ambient color in material
	const MColor ambient  = lambert.ambientColor(&status);
	MESSENGER_REJECT(!status,("failed to read incandescence from lambert shader object for fnDepNode,\"%s\"\n", status.errorString().asChar()));
	const float ambientA = 1.0;
	float ambientR = ambient.r;
	float ambientG = ambient.g;
	float ambientB = ambient.b;

	//if the ambient color is black (0,0,0), assume that we actually want white (since maya flips these relative to our engine)
	if(ambientR == 0.0 && ambientG == 0.0 && ambientB == 0.0)
	{
		ambientR = 1.0;
		ambientG = 1.0;
		ambientB = 1.0;
	}
	VectorArgb ambientColor (ambientA, ambientR, ambientG, ambientB);
	addMaterialAmbient(mainTag, ambientColor);

	//set emissive color in material
	const MColor emissive = lambert.incandescence(&status);
	MESSENGER_REJECT(!status,("failed to read incandescence from lambert shader object for fnDepNode,\"%s\"\n", status.errorString().asChar()));
	const VectorArgb emissiveColor(0.0f, emissive.r, emissive.g, emissive.b);
	addMaterialEmissive(mainTag, emissiveColor);

	//get shader type to get specular color
	if(_strcmpi("phong", typeName) == 0)
	{
		MFnPhongShader phong(objectArray[0], &status);
		MESSENGER_REJECT(!status,("failed to set phong shader object for fnDepNode,\"%s\"\n", status.errorString().asChar()));

		const MColor specular = phong.specularColor(&status);
		MESSENGER_REJECT(!status,("failed to read specularColor from phong shader object for fnDepNode,\"%s\"\n", status.errorString().asChar()));

		VectorArgb specularColor(1.0, specular.r, specular.g, specular.b);
		addMaterialSpecular(mainTag, specularColor);

		real cosPower = phong.cosPower(&status);
		MESSENGER_REJECT(!status,("failed to read cosPower from phong shader object for fnDepNode,\"%s\"\n", status.errorString().asChar()));
		addMaterialSpecularPower(mainTag, cosPower);
	}
	else if(strcmp("blinn", typeName) == 0)
	{
		MFnBlinnShader blinn(objectArray[0], &status);
		MESSENGER_REJECT(!status,("failed to set blinn shader object for fnDepNode,\"%s\"\n", status.errorString().asChar()));

		const MColor specular = blinn.specularColor(&status);
		MESSENGER_REJECT(!status,("failed to read specularColor from blinn shader object for fnDepNode,\"%s\"\n", status.errorString().asChar()));

		VectorArgb specularColor(specular.a, specular.r, specular.g, specular.b);
		addMaterialSpecular(mainTag, specularColor);

		real specularRollOff = blinn.specularRollOff(&status);
		MESSENGER_REJECT(!status,("failed to read specularRollOff from blinn shader object for fnDepNode,\"%s\"\n", status.errorString().asChar()));

		real eccentricity = blinn.eccentricity(&status);
		MESSENGER_REJECT(!status,("failed to read eccentricity from blinn shader object for fnDepNode,\"%s\"\n", status.errorString().asChar()));

		real cosPower =(10 * specularRollOff) +(10 *(1 - eccentricity));
		addMaterialSpecularPower(mainTag, cosPower);
	}
	else
	{
		addMaterialSpecular(mainTag, VectorArgb::solidBlack);
		addMaterialSpecularPower(mainTag, 2);
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 * Pull the attached textures and add it to the system
 */

bool MayaStaticShaderTemplateWriter::buildTexture(
	const MObject           &shaderGroupObject, 
	TextureRendererVector   &referencedTextureRenderers, 
	const std::string       &textureReferenceDirectory,
	const std::string       &textureRendererReferenceDirectory, 
	MayaUtility::TextureSet &texturesToWrite,
	Tag                      tag,
	const std::string       &channelName,
	CompressType             compressType
)
{
	MESSENGER_REJECT (shaderGroupObject.apiType () != MFn::kShadingEngine, ("buildTexture () expecting object of type MFn::kShaderEngine, found type %s\n", shaderGroupObject.apiTypeStr ()));

	int numberOfTextures = 0;

	//get the shader node attached to the shader group
	bool               result;
	MStatus            status;
	MObjectArray       objectArray;
	MFnDependencyNode  fnDepNode (shaderGroupObject, &status);
	MESSENGER_REJECT (!status, ("failed to set SG object for fnDepNode,\"%s\"\n", status.errorString ().asChar ()));

	result = MayaUtility::findSourceObjects (fnDepNode, "surfaceShader", &objectArray);
	MESSENGER_REJECT (!result, ("failed to get shader connected to shader group \"%s\"\n", fnDepNode.name ().asChar ()));
	MESSENGER_REJECT (objectArray.length () != 1, ("expecting surfaceShader to have one upstream connection, found %d\n", objectArray.length ()));

	//find texture attached to shader
	status = fnDepNode.setObject (objectArray [0]);
	MESSENGER_REJECT (!status, ("failed to set shader object for fnDepNode,\"%s\"\n", status.errorString ().asChar ()));

	result = MayaUtility::findSourceObjects (fnDepNode, channelName.c_str(), &objectArray);
	MESSENGER_REJECT (!result, ("failed to get texture files connected to shader\"%s\"\n", fnDepNode.name ().asChar ()));
	if (objectArray.length () < 1)
	{
		//no textures
		return true;
	}
	MESSENGER_REJECT (objectArray.length () > 1, ("expecting surfaceShader to have one upstream connection, found %d\n", objectArray.length ()));

	//set dependency node to the texture node
	status = fnDepNode.setObject (objectArray [0]);
	MESSENGER_REJECT (!status, ("failed to set file texture for fnDepNode,\"%s\"\n", status.errorString ().asChar ()));

	// fnDepNode now points to a "file" node.

	//track textures that are intended to be rendered via TextureRenderers.  these texture DG nodes end in _TR
	MString mayaNodeName = fnDepNode.name(&status);
	MESSENGER_REJECT(!status, ("failed to get node name for texture node\n"));
	IGNORE_RETURN(mayaNodeName.toLowerCase());
	const std::string  fullNodeName = mayaNodeName.asChar();
	const size_t       stringLength = fullNodeName.length();
	const bool isTextureRenderer    = (stringLength > 3) && (fullNodeName[stringLength-3] == '_') && (fullNodeName[stringLength-2] == 't') && (fullNodeName[stringLength-1] == 'r');

	switch (objectArray [0].apiType ())
	{
		case MFn::kFileTexture:
		{
			numberOfTextures = 1;

			// ----------

			bool wrapU;
			bool wrapV;
			
			result = MayaUtility::getNodeBoolValue(fnDepNode,"wrapU",&wrapU);
			MESSENGER_REJECT (!result, ("Failed to get texture wrap U"));
			result = MayaUtility::getNodeBoolValue(fnDepNode,"wrapV",&wrapV);
			MESSENGER_REJECT (!result, ("Failed to get texture wrap V"));

			// ----------

			//get texture name
			MObject      fileNameAttr = fnDepNode.attribute ("fileTextureName");
			const MPlug  plugToFileName (objectArray [0], fileNameAttr); 
			MObject      valueObject;

			status = plugToFileName.getValue (valueObject);
			MESSENGER_REJECT (!status, ("failed to get value object for fileTextureName,\"%s\"\n", status.errorString ().asChar ()));

			MFnStringData fnStringData (valueObject, &status);
			MESSENGER_REJECT (!status, ("failed to set filename string object for fnStringData,\"%s\"\n", status.errorString ().asChar ()));

			MString mFilename = fnStringData.string();
			IGNORE_RETURN(mFilename.toLowerCase());
			std::string filename = mFilename.asChar();

			//-- Remove Maya project '//' from path name.
			const std::string::size_type projectStartPos = filename.find("//");
			if (static_cast<int>(projectStartPos) != static_cast<int>(std::string::npos))
			{
				// Remove the first '/'.
				filename.erase(projectStartPos, 1);
				MESSENGER_LOG_WARNING(("texture file [%s] contained a Maya project-relative '//', please fix.\n", filename.c_str()));
			}

			//convert the texture from a fully-pathed file to texture\<filename>.dds
			char textureBaseName[128];
			bool result;
			result = MayaUtility::stripBaseName(filename.c_str(), textureBaseName, sizeof (textureBaseName), '/');
			MESSENGER_REJECT (!result, ("failed to strip base name from shader group texture filename \"%s\"\n", filename.c_str()));
			std::string textureRelativeName =  textureReferenceDirectory;
			            textureRelativeName += textureBaseName;
			            textureRelativeName += ".dds";

			// ----------

			addTexture(tag, textureRelativeName, wrapU, wrapV);

			//only add the texture if it's not a texture renderer (a texture reference defined in maya, but the file is built later)
			if(isTextureRenderer)
			{
				MESSENGER_LOG (("%s is a texture renderer, skipping...\n", fullNodeName.c_str()));
			}
			else
			{
				texturesToWrite.insert(std::make_pair(filename, compressType));
				ExporterLog::addSourceTexture(filename);
				ExporterLog::addDestinationTextureToCurrentShader(textureRelativeName);
			}
		}
		break;

		case MFn::kLayeredTexture:
		{
			//dependency node is now set to the layered texture, so find the inputs plug to access our data
			const MPlug inputsPlug = fnDepNode.findPlug ("inputs", &status);
			MESSENGER_REJECT (!status, ("fnDepNode.findPlug () failed,\"%s\"\n", status.errorString ().asChar ()));

			//since we're a layered texture, the number of elements represents the number of layers
			const uint numberOfElements = inputsPlug.numElements (&status);
			MESSENGER_REJECT (!status, ("inputsPlug.numElements () failed, \"%s\"\n", status.errorString ().asChar ()));

			numberOfTextures = static_cast<int>(numberOfElements);

			uint i;
			for (i = 0; i < numberOfElements; i++)
			{
				const MPlug plugElement = inputsPlug.elementByPhysicalIndex (i, &status);
				MESSENGER_REJECT (!status, ("inputsPlug.elementByLogicalIndex () failed,\"%s\"\n", status.errorString ().asChar ()));

				const MPlug plugChild = plugElement.child (0, &status);
				MESSENGER_REJECT (!status, ("plugElement.child () failed, \"%s\"\n", status.errorString ().asChar ()));

				MPlugArray plugArray;
				bool connected = plugChild.connectedTo (plugArray, true, true, &status);
				MESSENGER_REJECT (!status, ("connectedTo () failed, \"%s\"\n", status.errorString ().asChar ()));

				if (connected)
				{
					bool foundNetworked = false;

					uint plugArrayLength = plugArray.length ();
					uint k;
					for (k = 0; k < plugArrayLength; k++)
					{
						const MPlug& p = plugArray [k];
						MESSENGER_LOG (("element %u  connection %u  plug name=%s  type=%s  info=%s  elements=%u  children=%u  connectedElements=%u  connectedChildren=%u  isNetworked=%s\n", i, k, p.name (&status).asChar (), p.attribute (&status).apiTypeStr (), p.info (&status).asChar (), p.numElements (), p.numChildren (), p.numConnectedElements (), p.numConnectedChildren (), p.isNetworked () ? "true" : "false"));

						if (p.isNetworked ())
						{
							MESSENGER_REJECT (foundNetworked, ("texture %u is already networked -- expecting one network connection", i));

							foundNetworked = true;

							MObject node = plugArray [k].node ();
							status = fnDepNode.setObject (node);
							MESSENGER_REJECT (!status, ("failed to set file texture for fnDepNode,\"%s\"\n", status.errorString ().asChar ()));

							// ----------

							bool wrapU;
							bool wrapV;
							
							result = MayaUtility::getNodeBoolValue(fnDepNode,"wrapU",&wrapU);
							MESSENGER_REJECT (!result, ("Failed to get texture wrap U"));
							result = MayaUtility::getNodeBoolValue(fnDepNode,"wrapV",&wrapV);
							MESSENGER_REJECT (!result, ("Failed to get texture wrap V"));

							// ----------


							MObject      fileNameAttr = fnDepNode.attribute ("fileTextureName");
							const MPlug  plugToFileName (node, fileNameAttr); 
							MObject      valueObject;

							status = plugToFileName.getValue (valueObject);
							MESSENGER_REJECT (!status, ("failed to get value object for fileTextureName,\"%s\"\n", status.errorString ().asChar ()));

							MFnStringData fnStringData (valueObject, &status);
							MESSENGER_REJECT (!status, ("failed to set filename string object for fnStringData,\"%s\"\n", status.errorString ().asChar ()));

							MString mFilename = fnStringData.string();
							IGNORE_RETURN(mFilename.toLowerCase());
							std::string filename = mFilename.asChar();

							//-- Remove '//' for Maya project-relative pathnames.
							const std::string::size_type projectStartPos = filename.find("//");
							if (static_cast<int>(projectStartPos) != static_cast<int>(std::string::npos))
							{
								// Remove the first '/'.
								filename.erase(projectStartPos, 1);
								MESSENGER_LOG_WARNING(("texture file [%s] contained a Maya project-relative '//', please fix.\n", filename.c_str()));
							}

							// Convert the texture from a fully-pathed file to texture\<filename>.dds.
							bool result;
							char textureBaseName[128];

							std::string textureRelativeName = textureReferenceDirectory;
							result = MayaUtility::stripBaseName(filename.c_str(), textureBaseName, sizeof (textureBaseName), '/');
							MESSENGER_REJECT (!result, ("failed to strip base name from shader group texture filename \"%s\"\n", filename.c_str()));

							textureRelativeName += textureBaseName;
							textureRelativeName += ".dds";

							//-- Tell the shader writer about this texture.
							// @todo: -TRF- check what this function really does.
							addTexture(tag, textureRelativeName, wrapU, wrapV);

							//-- Tell the Texture-writing mechanism about this texture.
							texturesToWrite.insert(std::make_pair(filename, compressType));

							ExporterLog::addDestinationTextureToCurrentShader(textureRelativeName.c_str());
							MESSENGER_LOG (("texture name=%s\n", fnStringData.string ().asChar ()));
						}
					}
				}
			}
		}
		break;

		default:
		{
			MESSENGER_REJECT (objectArray [0].apiType () != MFn::kLayeredTexture, ("expecting object attached to shader be MFn::kLayeredTexture or MFn::kFileTexture, found %s\n", objectArray [0].apiTypeStr() ));
		}
	}

	if (isTextureRenderer)
	{
		//we've got a texture renderer specified and the caller is interested in hearing about it.

		//build the reference name for the TextureRendererTemplate
		std::string referenceName = textureRendererReferenceDirectory;

		//make sure we've got a trailing backslash after reference dir name
		const size_t refDirLength = referenceName.length();
		if (refDirLength && (referenceName[refDirLength - 1] != '\\') && (referenceName[refDirLength - 1] != '/'))
			referenceName.push_back('/');

		//add in base texture renderer name from node name
		IGNORE_RETURN(referenceName.append(fullNodeName, 0, stringLength-3));

		//add TextureRendererTemplate extension
		IGNORE_RETURN(referenceName.append(".trt"));

		//add the reference to the texture render, assume the given shader should have this texture renderer point to texture tag "MAIN"
		std::pair<std::string, Tag>  newReference;

		newReference.first  = referenceName;
		newReference.second = tag;

		referencedTextureRenderers.push_back(newReference);
	}
	return true;
}

// ----------------------------------------------------------------------
/**
 * This function builds up the data needed to create a shader template when the artist has not assigned a specialized effect.
 * An effect name is determined, a default coordinate set is assigned, and an alpha reference value is assigned if needed.
 */

bool MayaStaticShaderTemplateWriter::buildDefaultData(const MObject &shaderGroupObject, bool hasVertexAlpha, const std::string& effectReferenceDir, int numberOfTextures)
{
	UNREF(numberOfTextures);

	bool  effectHasAlpha;
	bool  result;

	// figure out if we need alpha
	if (hasVertexAlpha)
		effectHasAlpha = true;
	else
	{
		// check if shader group's shader has anything attached to its transparency attribute.  If so, assume we have texture alpha.
		MStatus            status;
		MObjectArray       objectArray;

		// get the shader node
		MFnDependencyNode  fnDepNode (shaderGroupObject, &status);
		MESSENGER_REJECT (!status, ("failed to assign shader group object to fnDepNode,\"%s\"\n", status.errorString ().asChar ()));

		result = MayaUtility::findSourceObjects (fnDepNode, "surfaceShader", &objectArray);
		MESSENGER_REJECT (!result, ("failed to find upstream object plugged into shader group's surfaceShader attribute.\n"));
		MESSENGER_REJECT (objectArray.length () != 1, ("expecting to find one surface attached to SG surfaceShader, found %d instead.\n", objectArray.length ()));

		// setup shader dep node
		status = fnDepNode.setObject (objectArray [0]);
		MESSENGER_REJECT (!status, ("failed to set shader fnDepNode, error = \"%s\"\n", status.errorString ().asChar ()));

		// check if it has transparency attached
		const MPlug transparencyPlug = fnDepNode.findPlug ("transparency", &status);
		MESSENGER_REJECT (!status, ("failed to create shader transparency plug,\"%s\"\n", status.errorString ().asChar ()));

		// effect has alpha (texture alpha) if transparency plug is attached (i.e. is set to some value)
		effectHasAlpha = transparencyPlug.isNetworked (&status);
		MESSENGER_REJECT (!status, ("failed to determine if transparency plug is networked,\"%s\"\n", status.errorString ().asChar ()));
	}

	// build the effect reference name
	char buffer [MAX_PATH];

	strcpy (buffer, effectReferenceDir.c_str());

	if (effectHasAlpha)
		strcat(buffer, "a_alpha.eft");
	else
		strcat(buffer, "a_simple.eft");

	ms_effectName = buffer;

	return true;
}

// ----------------------------------------------------------------------

MColor MayaStaticShaderTemplateWriter::buildColorFromAttribute(const MObject& attribute, const MFnDependencyNode& node)
{
	//get the compound attribute
	MFnCompoundAttribute compoundAttribute;
	MStatus status = compoundAttribute.setObject(attribute);
	MESSENGER_REJECT(!status,("failed to set object into MFnCompoundAttribute\n"));
	//validate that we have 3 children
	int numChildren = compoundAttribute.numChildren();
	MESSENGER_REJECT(numChildren != 3, ("bad number of children for color attribute"));

	//get the red value, from a plug
	MObject redObject = compoundAttribute.child(0, &status);
	MESSENGER_REJECT(!status, ("could not get red object\n"));
	MPlug redPlug = node.findPlug(redObject, &status);
	MESSENGER_REJECT(!status, ("could not get red plug\n"));
	float redValue = 1.0;
	status = redPlug.getValue(redValue);
	MESSENGER_REJECT(!status, ("could not get value from red plug\n"));

	//get the green value, from a plug
	MObject greenObject = compoundAttribute.child(1, &status);
	MESSENGER_REJECT(!status, ("could not get green object\n"));
	MPlug greenPlug = node.findPlug(greenObject, &status);
	MESSENGER_REJECT(!status, ("could not get green plug\n"));
	float greenValue = 1.0;
	status = greenPlug.getValue(greenValue);
	MESSENGER_REJECT(!status, ("could not get value from green plug\n"));

	//get the blue value, from a plug
	MObject blueObject = compoundAttribute.child(2, &status);
	MESSENGER_REJECT(!status, ("could not get blue object\n"));
	MPlug bluePlug = node.findPlug(blueObject, &status);
	MESSENGER_REJECT(!status, ("could not get blue plug\n"));
	float blueValue = 1.0;
	status = bluePlug.getValue(blueValue);
	MESSENGER_REJECT(!status, ("could not get value from blue plug\n"));

	//build and return the final color
	return MColor(redValue, greenValue, blueValue);
}

// ======================================================================
