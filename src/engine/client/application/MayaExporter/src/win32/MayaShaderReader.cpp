// ======================================================================
//
// MayaShaderReader.cpp
// Author: Ben Earhart
//
// Copyright 2005 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

//precompiled header includes
#include "FirstMayaExporter.h"

//module include
#include "MayaShaderReader.h"
#include "MayaUtility.h"

#include "Messenger.h"

//maya SDK includes
#include "maya/MAnimControl.h"
#include "maya/MDagPath.h"
#include "maya/MFn.h"
#include "maya/MFnAttribute.h"
#include "maya/MIntArray.h"
#include "maya/MItDependencyGraph.h"
#include "maya/MItMeshPolygon.h"
#include "maya/MFileIO.h"
#include "maya/MFnMesh.h"
#include "maya/MFnSet.h"
#include "maya/MFnStringData.h"
#include "maya/MGlobal.h"
#include "maya/MMatrix.h"
#include "maya/MObject.h"
#include "maya/MObjectArray.h"
#include "maya/MSelectionList.h"
#include "maya/MString.h"
#include "maya/MStringArray.h"
#include "maya/MTime.h"

// ======================================================================

extern Messenger * messenger;

// ======================================================================

class MayaShaderReader::ShaderAttribute
{
public:

	ShaderAttribute(
		const std::string &i_name, 
		const MObject &i_attributeObject, 
		const MPlug &i_plug
		) :
	 name(i_name)
	,attributeObject(i_attributeObject)
	,plug(i_plug)
	{}

	std::string name;
	MObject     attributeObject;
	MPlug       plug;
};

// ======================================================================

MayaShaderReader::MayaShaderReader(const MObject &i_shaderGroupObject)
: m_shaderGroupObject(i_shaderGroupObject)
, m_failed(false)
, m_attributeCount(0) 
{
	m_failed=_initialize();
}

MayaShaderReader::~MayaShaderReader()
{
}

bool MayaShaderReader::getStringAttribute(std::string &o_value, const char *i_attributeName)
{
	if (!i_attributeName || !*i_attributeName)
	{
		return false;
	}

	o_value.clear();
	AttributeNameLookup::iterator ai = _getAttribute(i_attributeName);
	if (ai!=m_attributeCache.end())
	{
		const ShaderAttribute *s = ai->second;

		MString value;
		MStatus status = s->plug.getValue(value);
		MESSENGER_REJECT(!status,("failed to get string value for attribute: %s (%s)\n",i_attributeName, status.errorString().asChar()));
		o_value=value.asChar();
		return true;
	}
	return false;
}

bool MayaShaderReader::getFloatAttribute(float &o_value, const char *i_attributeName)
{
	if (!i_attributeName || !*i_attributeName)
	{
		return false;
	}

	AttributeNameLookup::iterator ai = _getAttribute(i_attributeName);
	if (ai!=m_attributeCache.end())
	{
		const ShaderAttribute *s = ai->second;

		float value;
		MStatus status = s->plug.getValue(value);
		MESSENGER_REJECT(!status,("failed to get float value for attribute: %s (%s)\n",i_attributeName, status.errorString().asChar()));
		o_value=value;
		return true;
	}
	return false;
}

bool MayaShaderReader::getTextureAttributes(TextureAttributeList &o_attributes, const char *i_attributeName, const char *i_textureReferenceDirectory)
{
	if (!i_attributeName || !*i_attributeName)
	{
		return false;
	}

	if (!strstr(i_attributeName, "textureName_"))
	{
		DEBUG_WARNING(true, ("Attempt to get texture attributes from a non-texture attribute name\n"));
		return false;
	}

	AttributeNameLookup::iterator ai = _getAttribute(i_attributeName);
	if (ai!=m_attributeCache.end())
	{
		const ShaderAttribute *s = ai->second;

		const std::string prefix = m_attributePrefix + "textureName_";
		const std::string tagStr = MayaUtility::getTagFromAttributeName(i_attributeName, prefix);
		const Tag tag = MayaUtility::convertStringtoTag(tagStr);

		MStatus status;
		MString value;
		status = s->plug.getValue(value);

		// -----------------------------------------------------------------
		CompressType compressType = CT_texture;
		bool isNormalMap=false;

		//if we have a normal map (tag NRML) then do NOT compress the texture
		if (tag == TAG(N,R,M,L))
		{
			compressType = CT_none;
			isNormalMap=true;
		}
		else if (tag == TAG(C,N,R,M)) // do compress normal maps w/tag CNMR
		{
			compressType = CT_normalMap;
			isNormalMap=true;
		}
		// -----------------------------------------------------------------

		if (!status) // no plug
		{
			return _getChannelTextures(o_attributes, i_attributeName, isNormalMap, tag, compressType, i_textureReferenceDirectory);
			/*
			if (isNormalMap)
			{
				TextureSet textures;

				if (getChannelTextures(textures, shaderGroupObject, attributeName, compressType))
				{
					if (!textures.empty())
					{
						TextureSet::iterator tsi = textures.begin();
						o_name = tsi->first;

						return true;
					}
				}
			}
			*/
		}

	}
	return false;
}

bool MayaShaderReader::_getChannelTextures(
	TextureAttributeList  &o_attributes, 
	const char            *i_attributeName, 
	bool                   isNormalMap,
	Tag                    tag,
	const CompressType     compressType,
	const char            *i_textureReferenceDirectory
	)
{
	//get the shader node attached to the shader group
	bool         ret_val=false;
	bool         result;
	MStatus      status;
	MObjectArray objectArray;

	MFnDependencyNode shaderDependencyNode(m_shaderNode, &status);
	MESSENGER_REJECT (!status, ("failed to set shader object for fnDepNode,\"%s\"\n", status.errorString ().asChar()));

	result = MayaUtility::findSourceObjects(shaderDependencyNode, i_attributeName, &objectArray);
	MESSENGER_REJECT (!result, ("failed to get texture files connected to shader\"%s\"\n", shaderDependencyNode.name().asChar()));
	if (objectArray.length () < 1)
	{
		//no textures
		return false;
	}
	MESSENGER_REJECT (objectArray.length () > 1, ("expecting surfaceShader to have one upstream connection, found %d\n", objectArray.length()));

	const MObject textureNode(objectArray[0]);

	//set dependency node to the texture node
	MFnDependencyNode  textureDependencyNode(textureNode, &status);
	MESSENGER_REJECT (!status, ("failed to set file texture for fnDepNode,\"%s\"\n", status.errorString().asChar()));

	// textureDependencyNode now points to a "file" node.

	//track textures that are intended to be rendered via TextureRenderers.  these texture DG nodes end in _TR
	MString mayaNodeName = textureDependencyNode.name(&status);
	MESSENGER_REJECT(!status, ("failed to get node name for texture node\n"));
	IGNORE_RETURN(mayaNodeName.toLowerCase());
	const std::string  fullNodeName = mayaNodeName.asChar();
	const size_t       stringLength = fullNodeName.length();
	const bool isTextureRenderer    = (stringLength > 3) && (fullNodeName[stringLength-3] == '_') && (fullNodeName[stringLength-2] == 't') && (fullNodeName[stringLength-1] == 'r');

	if (textureNode.apiType() == MFn::kFileTexture)
	{
		// ----------

		bool wrapU;
		bool wrapV;
		
		result = MayaUtility::getNodeBoolValue(textureDependencyNode, "wrapU", &wrapU);
		MESSENGER_REJECT (!result, ("Failed to get texture wrap U"));
		result = MayaUtility::getNodeBoolValue(textureDependencyNode, "wrapV", &wrapV);
		MESSENGER_REJECT (!result, ("Failed to get texture wrap V"));

		// ----------

		//get texture name
		MObject      fileNameAttr = textureDependencyNode.attribute("fileTextureName");
		const MPlug  plugToFileName (textureNode, fileNameAttr); 
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
		std::string textureRelativeName =  i_textureReferenceDirectory;
			         textureRelativeName += textureBaseName;
			         textureRelativeName += ".dds";

		// only add the texture if it's not a texture renderer (a texture reference defined in maya, but the file is built later)
		if (isTextureRenderer)
		{
			MESSENGER_LOG (("%s is a texture renderer, skipping...\n", fullNodeName.c_str()));
		}
		else
		{
			ret_val=true;
			o_attributes.push_back(TextureAttribute(filename.c_str(), textureRelativeName.c_str(), wrapU, wrapV, isNormalMap, compressType, tag));
		}
	}

	return ret_val;
}

bool MayaShaderReader::_initialize()
{
	MESSENGER_REJECT(m_shaderGroupObject.apiType() != MFn::kShadingEngine,("getShaderGroupMaterial() expecting object of type MFn::kShaderEngine, found type %s\n", m_shaderGroupObject.apiTypeStr()));

	MStatus           status;
	MFnAttribute      attribute;
	MObject           attributeObject;
	MPlug             plug;
	MObjectArray      objectArray;

	status=m_shaderDependencyNode.setObject(m_shaderGroupObject);
	MESSENGER_REJECT(!status,("failed to set SG object for m_shaderDependencyNode,\"%s\"\n", status.errorString().asChar()));

	bool result = MayaUtility::findSourceObjects(m_shaderDependencyNode, "surfaceShader", &objectArray);	MESSENGER_REJECT(!result,("failed to get shader connected to shader group \"%s\"\n", m_shaderDependencyNode.name().asChar()));
																																			MESSENGER_REJECT(objectArray.length() != 1,("expecting surfaceShader to have one upstream connection, found %d\n", objectArray.length()));
	m_shaderNode = objectArray[0];

	status=m_lambertShader.setObject(m_shaderNode); MESSENGER_REJECT(!status,("failed to set lambert shader object for m_shaderDependencyNode,\"%s\"\n", status.errorString().asChar()));
	m_attributeCount = m_lambertShader.attributeCount(&status);	MESSENGER_REJECT(!status,("failed to get attribute count for shader attributes\n"));

	status=m_phongShader.setObject(m_shaderNode); MESSENGER_REJECT(!status,("failed to set m_phongShader shader object for m_shaderDependencyNode,\"%s\"\n", status.errorString().asChar()));
	m_phongObject = m_phongShader.object();

	status=m_materialDependencyNode.setObject(m_phongObject);	MESSENGER_REJECT(!status,("failed to set material object for m_shaderDependencyNode,\"%s\"\n", status.errorString().asChar()));

	m_attributePrefix = MayaUtility::getAttributePrefix();

	//get all the attributes, and for each one, build the appropriate part of the shader template
	for(unsigned i = 0; i < m_attributeCount; ++i)
	{
		_getAttribute(attributeObject, attribute, i);

		std::string attributeName =attribute.name(&status).asChar();		MESSENGER_REJECT(!status,("failed to get attribute name\n"));

		//is it one of our attributes?
		if (attributeName.find(m_attributePrefix) != std::string::npos)
		{
			// get the value for this attribute
			plug = m_materialDependencyNode.findPlug(attributeObject, &status);	MESSENGER_REJECT_WARNING(!status,("failed to find plug for attribute [%s]\n", attributeName.c_str()));

			ShaderAttribute *shaderAttribute = new ShaderAttribute(attributeName, attributeObject, plug);
			m_attributeCache[shaderAttribute->name.c_str()]=shaderAttribute;

			/*
			if (attributeName.find("textureName") != std::string::npos)
			{
				const std::string prefix = m_attributePrefix + "textureName_";
				const std::string tagStr = MayaUtility::getTagFromAttributeName(attributeName, prefix);
				Tag tag = MayaUtility::convertStringtoTag(tagStr);
				MString value;
				status = plug.getValue(value);

				MayaUtility::CompressType compressType = MayaUtility::CT_texture;

				bool isNormalMap=false;
				//if we have a normal map (tag NRML) then do NOT compress the texture
				if (tag == TAG(N,R,M,L))
				{
					compressType = MayaUtility::CT_none;
					isNormalMap=true;
				}
				else if (tag == TAG(C,N,R,M)) // do compress normal maps w/tag CNMR
				{
					compressType = MayaUtility::CT_normalMap;
					isNormalMap=true;
				}

				if (!status) // no plug
				{
					if (isNormalMap)
					{
						MayaUtility::TextureSet textures;

						if (MayaUtility::getChannelTextures(textures, m_shaderGroupObject, attributeName, compressType))
						{
							if (!textures.empty())
							{
								MayaUtility::TextureSet::iterator tsi = textures.begin();
								//o_name = tsi->first;

								return true;
							}
						}
					}
				}
			}
			*/
		}
	}

	return false;
}

MayaShaderReader::AttributeNameLookup::iterator MayaShaderReader::_getAttribute(const char *i_attributeName)
{
	return m_attributeCache.find(i_attributeName);
}

bool MayaShaderReader::_getAttribute(MObject &o_attributeObject, MFnAttribute &o_attribute, int index)
{
	if (index<0 || unsigned(index)>=m_attributeCount)
	{
		return false;
	}

	MStatus status;
	MObject attributeObject;

	//get the attribute
	o_attributeObject = m_lambertShader.attribute(index, &status);	MESSENGER_REJECT(!status,("failed to get attribute [%u]\n", index));
	status = o_attribute.setObject(o_attributeObject);	MESSENGER_REJECT(!status,("failed to set object into MFnAttribute\n"));
	return true;
}


