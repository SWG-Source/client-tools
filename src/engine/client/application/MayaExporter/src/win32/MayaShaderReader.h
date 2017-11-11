// ======================================================================
//
// MayaShaderReader.h
// Author: Ben Earhart
//
// Copyright 2005, Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef MAYA_SHADER_READER_H
#define MAYA_SHADER_READER_H

// ======================================================================

#include "MayaUtility.h"

#include "maya/MFnDependencyNode.h"
#include "maya/MFnLambertShader.h"
#include "maya/MFnPhongShader.h"

#include <string>
#include <map>
#include <list>

class MFnAttribute;

// ======================================================================

class MayaShaderReader
{
public:

	// ----------------------------------------------------------------------------------------------

	struct TextureAttribute
	{
		TextureAttribute() {}

		TextureAttribute(
			const char *fileName,
			const char *assetPath,
			bool wrapU, 
			bool wrapV,
			bool isNormalMap,
			CompressType compressType,
			Tag tag
			)
			: m_sourceFileName(fileName), 
			  m_assetPath(assetPath), 
			  m_wrapU(wrapU), 
			  m_wrapV(wrapV),
			  m_isNormalMap(isNormalMap),
			  m_compressType(compressType),
			  m_tag(tag)
		{}

		std::string   m_sourceFileName;
		std::string   m_assetPath;
		bool          m_wrapU;
		bool          m_wrapV;

		bool          m_isNormalMap;
		CompressType  m_compressType;
		Tag           m_tag;
	};
	typedef std::list<TextureAttribute> TextureAttributeList;

	// ----------------------------------------------------------------------------------------------

	MayaShaderReader(const MObject &i_shaderGroupObject);
	~MayaShaderReader();

	bool failed()                                                    { return m_failed; }

	const MObject &getShaderGroupObject()                            { return m_shaderGroupObject; }

	bool getStringAttribute(std::string &o_value, const char *i_attributeName);
	bool getFloatAttribute(float &o_value, const char *i_attributeName);

	bool getTextureAttributes(TextureAttributeList &o_attributes, const char *i_attributeName, const char *i_textureReferenceDirectory);

private:

	class ShaderAttribute;

	struct _szLT { bool operator()(const char *s1, const char *s2) const { return strcmp(s1,s2)<0; }	};
	typedef std::map<const char *, ShaderAttribute *, _szLT> AttributeNameLookup;

	// --------------------------------------------------------------

	bool _initialize();
	bool _getChannelTextures(
		TextureAttributeList  &o_attributes, 
		const char            *i_attributeName, 
		bool                  isNormalMap,
		Tag                   tag,
		const CompressType    compressType,
		const char            *i_textureReferenceDirectory
		);
	bool _getAttribute(MObject &o_attributeObject, MFnAttribute &o_attribute, int index);

	AttributeNameLookup::iterator _getAttribute(const char *i_attributeName);

	// --------------------------------------------------------------

	const MObject     &m_shaderGroupObject;

	MFnDependencyNode  m_shaderDependencyNode;
	MObject            m_shaderNode;
	MFnLambertShader   m_lambertShader;
	MFnPhongShader     m_phongShader;
	MObject            m_phongObject;
	MFnDependencyNode  m_materialDependencyNode;

	std::string         m_attributePrefix;

	bool                m_failed;
	unsigned            m_attributeCount;
	AttributeNameLookup m_attributeCache;
};


// ======================================================================

#endif

