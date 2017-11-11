// ======================================================================
//
// MayaCustomizableShaderTemplateWriter.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "MayaCustomizableShaderTemplateWriter.h"

#include "ExporterLog.h"
#include "maya/MFnAttribute.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnStringData.h"
#include "maya/MObjectArray.h"
#include "maya/MPlug.h"
#include "maya/MString.h"
#include "MayaShaderTemplateBuilder.h"
#include "MayaStaticShaderTemplateWriter.h"
#include "MayaUtility.h"
#include "Messenger.h"
#include "PluginMain.h"
#include "SetDirectoryCommand.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/PointerDeleter.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

// ======================================================================

namespace MayaCustomizableShaderTemplateWriterNamespace
{
	typedef std::map<Tag, MFnDependencyNode>  TextureNodeMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	const char *const  cs_textureAttributeNamePrefix = "soe_textureName_";


	bool  getTextureNodes(MFnDependencyNode const &materialDependencyNode, TextureNodeMap &textureNodes);
}

using namespace MayaCustomizableShaderTemplateWriterNamespace;

// ======================================================================

const Tag TAG_CSHD = TAG(C,S,H,D);
const Tag TAG_CUST = TAG(C,U,S,T);
const Tag TAG_HUEB = TAG(H,U,E,B);
const Tag TAG_MAIN = TAG(M,A,I,N);
const Tag TAG_PAL  = TAG3(P,A,L);
const Tag TAG_TFAC = TAG(T,F,A,C);
const Tag TAG_TXTR = TAG(T,X,T,R);
const Tag TAG_TX1D = TAG(T,X,1,D);

// ======================================================================

const int          MayaCustomizableShaderTemplateWriter::cms_priority = 10;
const MString      MayaCustomizableShaderTemplateWriter::cms_firstColorChannelAttributeName("soe_colorChannelA");
const MString      MayaCustomizableShaderTemplateWriter::cms_secondColorChannelAttributeName("soe_colorChannelB");
const MString      MayaCustomizableShaderTemplateWriter::cms_colorChannelDataNodeName("colorChannelNode");

const MString      MayaCustomizableShaderTemplateWriter::cms_customizationVariableAttributeNameBase("soe_cc_name");
const MString      MayaCustomizableShaderTemplateWriter::cms_customizationIsPrivateAttributeNameBase("soe_cc_private");
const MString      MayaCustomizableShaderTemplateWriter::cms_paletteFileAttributeNameBase("soe_cc_pal");
const MString      MayaCustomizableShaderTemplateWriter::cms_defaultPaletteIndexAttributeNameBase("soe_cc_defIndex");

const MString      MayaCustomizableShaderTemplateWriter::cms_materialColorAttributeName("color");
const MString      MayaCustomizableShaderTemplateWriter::cms_materialSecondHueTextureAttributeName("soe_textureName_HUEB");
const MString      MayaCustomizableShaderTemplateWriter::cms_swappableTextureVariableNameAttributeName("soe_tc_name");
const MString      MayaCustomizableShaderTemplateWriter::cms_swappableTexturePrivateAttributeName("soe_tc_private");
const MString      MayaCustomizableShaderTemplateWriter::cms_swappableTextureDefaultIndexAttributeName("soe_tc_defIndex");
const MString      MayaCustomizableShaderTemplateWriter::cms_layeredTextureInputsAttributeName("inputs");
const MString      MayaCustomizableShaderTemplateWriter::cms_texturePathNameAttributeName("fileTextureName");

const std::string  MayaCustomizableShaderTemplateWriter::cms_paletteReferenceDirectory("palette/");

bool               MayaCustomizableShaderTemplateWriter::ms_installed;
Messenger         *MayaCustomizableShaderTemplateWriter::messenger;

// ======================================================================

class MayaCustomizableShaderTemplateWriter::ColorChannelData
{
public:

	ColorChannelData(const std::string &customizationVariableName, bool variableIsPrivate, const std::string &palettePathName, int defaultPaletteIndex);

	const std::string &getCustomizationVariableName() const;
	bool               isVariablePrivate() const;
	const std::string &getPalettePathName() const;
	int                getDefaultPaletteIndex() const;

private:

	// disabled
	ColorChannelData();

private:

	std::string  m_customizationVariableName;
	bool         m_variableIsPrivate;
	std::string  m_palettePathName;
	int          m_defaultPaletteIndex;

};

// ======================================================================

class MayaCustomizableShaderTemplateWriter::SwappableTextureData
{
public:

	SwappableTextureData(Tag textureTag, const char *customizationVariableName, bool variableIsPrivate, int defaultIndex);

	void  addTexturePathName(const std::string &texturePathName);

	Tag                   getTextureTag() const;

	const CrcLowerString &getCustomizationVariableName() const;
	bool                  isVariablePrivate() const;

	int                   getDefaultIndex() const;

	int                   getTexturePathNameCount() const;
	const std::string    &getTexturePathName(int index) const;

private:

	typedef std::vector<std::string>  StringVector;

private:

	// Disabled.
	SwappableTextureData();
	SwappableTextureData(const SwappableTextureData&); //lint -esym(754, SwappableTextureData::SwappableTextureData) // structure member not referenced // defensive hiding

private:

	CrcLowerString  m_customizationVariableName;
	bool            m_variableIsPrivate;

	Tag             m_textureTag;
	int             m_defaultIndex;
	StringVector    m_texturePathNames;

};

// ======================================================================
// namespace MayaCustomizableShaderTemplateWriterNamespace
// ======================================================================

bool MayaCustomizableShaderTemplateWriterNamespace::getTextureNodes(MFnDependencyNode const &materialDependencyNode, TextureNodeMap &textureNodes)
{
	// Check for any other texture nodes.  These will be attributes with the format "soe_textureName_<TEXTURE_TAG_NAME>".
	MFnAttribute       attribute;
	MFnDependencyNode  textureDependencyNode;
	MObjectArray       objectArray;
	char               textureTagString[128];
	unsigned const     textureAttributeNamePrefixLength = strlen(cs_textureAttributeNamePrefix);

	unsigned const attributeCount = materialDependencyNode.attributeCount();
	for (unsigned i = 0; i < attributeCount; ++i)
	{
		//-- Get the attribute object, ensure it's a dependency node.
		MObject attributeObject = materialDependencyNode.attribute(i);
//		DEBUG_REPORT_LOG(true, ("attribute object type: [%s].\n", attributeObject.apiTypeStr()));

		if (!attribute.setObject(attributeObject))
			continue;

		//-- Check if this attribute name starts with the texture attribute prefix.
		MString const      attributeName        = attribute.name();
		char const *const  cStringAttributeName = attributeName.asChar();
		unsigned const     attributeNameLength  = strlen(cStringAttributeName);
		bool const         prefixMatched        = (attributeNameLength > textureAttributeNamePrefixLength) && (strncmp(cStringAttributeName, cs_textureAttributeNamePrefix, textureAttributeNamePrefixLength) == 0);

		if (prefixMatched)
		{
			DEBUG_REPORT_LOG(true, ("*** Attribute named [%s] matched texture attribute prefix.\n", cStringAttributeName));
			
			//-- Extract the tag name.
			for (int j = 0; j < 4; ++j)
				textureTagString[j] = ' ';
			textureTagString[4] = 0;

			int const tagLength = std::min(4, static_cast<int>(attributeNameLength - textureAttributeNamePrefixLength));

			for (int k = 0; (k < 4) && *(cStringAttributeName + textureAttributeNamePrefixLength + k); ++k)
				textureTagString[tagLength - (1 + k)] = *(cStringAttributeName + textureAttributeNamePrefixLength + k);

			Tag const textureTag = *((Tag*) textureTagString); //lint !e1924 // C-style cast // Yes, easier to follow here.

#ifdef _DEBUG
			char  debugTagName[5];

			ConvertTagToString(textureTag, debugTagName);
			DEBUG_REPORT_LOG(true, ("*** Tag constructed as [%s].\n", debugTagName));
#endif

			//-- Get the connected node.
			IGNORE_RETURN(objectArray.clear());
			IGNORE_RETURN(MayaUtility::findSourceObjects(materialDependencyNode, attributeName, &objectArray, true));
			if (objectArray.length() > 0)
			{
				if (textureDependencyNode.setObject(objectArray[0]))
				{
					//-- Add the texture/dependency node to the map.
					IGNORE_RETURN(textureNodes.insert(TextureNodeMap::value_type(textureTag, textureDependencyNode)));
				}
			}

		}
		else if (strcmp(cStringAttributeName, "color") == 0)
		{
			//-- The "color" attribute is the standard Maya color texture where we get diffuse color info from.

			IGNORE_RETURN(objectArray.clear());
			IGNORE_RETURN(MayaUtility::findSourceObjects(materialDependencyNode, attributeName, &objectArray, true));
			if (objectArray.length() > 0)
			{
				if (textureDependencyNode.setObject(objectArray[0]))
				{
					//-- Add the texture/dependency node to the map.
					IGNORE_RETURN(textureNodes.insert(TextureNodeMap::value_type(TAG_MAIN, textureDependencyNode)));
				}
			}
		}
	}

	return true;
}

// ======================================================================
// class MayaCustomizableShaderTemplateWriter::ColorChannelData
// ======================================================================

MayaCustomizableShaderTemplateWriter::ColorChannelData::ColorChannelData(const std::string &customizationVariableName, bool variableIsPrivate, const std::string &palettePathName, int defaultPaletteIndex) :
	m_customizationVariableName(customizationVariableName),
	m_variableIsPrivate(variableIsPrivate),
	m_palettePathName(palettePathName),
	m_defaultPaletteIndex(defaultPaletteIndex)
{
}

// ----------------------------------------------------------------------

inline const std::string &MayaCustomizableShaderTemplateWriter::ColorChannelData::getCustomizationVariableName() const
{
	return m_customizationVariableName;
}

// ----------------------------------------------------------------------

inline bool MayaCustomizableShaderTemplateWriter::ColorChannelData::isVariablePrivate() const
{
	return m_variableIsPrivate;
}

// ----------------------------------------------------------------------

inline const std::string &MayaCustomizableShaderTemplateWriter::ColorChannelData::getPalettePathName() const
{
	return m_palettePathName;
}

// ----------------------------------------------------------------------

inline int MayaCustomizableShaderTemplateWriter::ColorChannelData::getDefaultPaletteIndex() const
{
	return m_defaultPaletteIndex;
}

// ======================================================================
// class MayaCustomizableShaderTemplateWriter::SwappableTextureData
// ======================================================================

MayaCustomizableShaderTemplateWriter::SwappableTextureData::SwappableTextureData(Tag textureTag, const char *customizationVariableName, bool variableIsPrivate, int defaultIndex) :
	m_customizationVariableName(customizationVariableName),
	m_variableIsPrivate(variableIsPrivate),
	m_textureTag(textureTag),
	m_defaultIndex(defaultIndex),
	m_texturePathNames()
{
}

// ----------------------------------------------------------------------

void MayaCustomizableShaderTemplateWriter::SwappableTextureData::addTexturePathName(const std::string &texturePathName)
{
	m_texturePathNames.push_back(texturePathName);
}

// ----------------------------------------------------------------------

Tag MayaCustomizableShaderTemplateWriter::SwappableTextureData::getTextureTag() const
{
	return m_textureTag;
}

// ----------------------------------------------------------------------

const CrcLowerString &MayaCustomizableShaderTemplateWriter::SwappableTextureData::getCustomizationVariableName() const
{
	return m_customizationVariableName;
}

// ----------------------------------------------------------------------

bool MayaCustomizableShaderTemplateWriter::SwappableTextureData::isVariablePrivate() const
{
	return m_variableIsPrivate;
}

// ----------------------------------------------------------------------

int MayaCustomizableShaderTemplateWriter::SwappableTextureData::getDefaultIndex() const
{
	return m_defaultIndex;
}

// ----------------------------------------------------------------------

int MayaCustomizableShaderTemplateWriter::SwappableTextureData::getTexturePathNameCount() const
{
	return static_cast<int>(m_texturePathNames.size());
}

// ----------------------------------------------------------------------

const std::string &MayaCustomizableShaderTemplateWriter::SwappableTextureData::getTexturePathName(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getTexturePathNameCount());

	return m_texturePathNames[static_cast<StringVector::size_type>(index)];
}

// ======================================================================
// class MayaCustomizableShaderTemplateWriter
// ======================================================================

void MayaCustomizableShaderTemplateWriter::install(Messenger *newMessenger)
{
	DEBUG_FATAL(ms_installed, ("MayaCustomizableShaderTemplateWriter already installed"));
	NOT_NULL(newMessenger);

	//-- keep track of Messenger
	messenger = newMessenger;

	//-- register this MayaShaderTemplateWriter with the builder class
	MayaShaderTemplateBuilder::registerShaderTemplateWriter(new MayaCustomizableShaderTemplateWriter(), cms_priority);

	ms_installed = true;
}

// ----------------------------------------------------------------------

void MayaCustomizableShaderTemplateWriter::remove()
{
	DEBUG_FATAL(!ms_installed, ("MayaCustomizableShaderTemplateWriter not installed"));

	messenger    = 0;
	ms_installed = false;
}

// ======================================================================
	/**
	 * Indicates whether the specified ShaderGroupObject could be written
	 * as a ShaderTemplate type supported by this class.
	 *
	 * If this function returns true, the caller is guaranteed that 
	 * invoking write() can and should be used to reasonably export the
	 * ShaderTemplate data stored within the Maya ShaderGroup node.
	 *
	 * This particular instance looks for hue-related attributes on the Material
	 * node.  If it finds them and if soe_colorChannelA is non-zero, this function
	 * will indicate the CustomizableShaderTemplateWriter should write
	 * out the ShaderTemplate data.
	 *
	 * @return  true if this ShaderTemplate type can and should write out the
	 *          specified data; false otherwise.
	 */

bool MayaCustomizableShaderTemplateWriter::canWrite(const MObject &shaderGroupObject) const
{
	//-- Get the material node.
	MFnDependencyNode  materialDependencyNode;

	const bool getMaterialSuccess = getMaterialDependencyNode(shaderGroupObject, materialDependencyNode);
	MESSENGER_REJECT(!getMaterialSuccess, ("failed to get Material's dependency node.\n"));
	
	//-- Check for presence of hue customization.
	// Get first color channel attribute.
	int        firstColorChannelValue = 0;
	const bool attributeValueFound = MayaUtility::getNodeIntValue(materialDependencyNode, cms_firstColorChannelAttributeName, firstColorChannelValue, true);

	// If first color channel attribute exists and is non-zero, hueing is present.
	const bool hasCustomizableHue = attributeValueFound && (firstColorChannelValue != 0);

	//-- Look for swappable textures.
	TextureNodeMap  textureNodeMap;
	int             swappableTextureCount = 0;

	bool const gotTextureNodes = getTextureNodes(materialDependencyNode, textureNodeMap);
	if (gotTextureNodes)
	{
		TextureNodeMap::iterator const endIt = textureNodeMap.end();
		for (TextureNodeMap::iterator it = textureNodeMap.begin(); it != endIt; ++it)
		{
			DEBUG_REPORT_LOG(true, ("texture object api type: [%s].\n", it->second.object().apiTypeStr()));
			if (it->second.object().apiType() == MFn::kLayeredTexture)
				++swappableTextureCount;
		}
	}
#if 1
	DEBUG_REPORT_LOG(true, ("Swappable texture count: %d.\n", swappableTextureCount));
#endif

	//-- Indicate that a CustomizableShader should be written if any customizations were present.
	return hasCustomizableHue || (swappableTextureCount > 0);
}

// ----------------------------------------------------------------------

bool MayaCustomizableShaderTemplateWriter::write(
		const std::string       &shaderTemplateWriteName, 
		const MObject           &shaderGroupObject, 
		bool                     hasVertexAlpha,
		TextureRendererVector   &referencedTextureRenderers, 
		const std::string       &textureReferenceDirectory,
		const std::string       &textureRendererReferenceDirectory,
		const std::string       &effectReferenceDirectory,
		MayaUtility::TextureSet &textureFilenames,
		bool                     hasDot3TextureCoordinate,
		int                      dot3TextureCoordinateIndex)

{
	//-- Get the Maya shader material node.
	MFnDependencyNode  materialDependencyNode;

	const bool getMaterialSuccess = getMaterialDependencyNode(shaderGroupObject, materialDependencyNode);
	MESSENGER_REJECT(!getMaterialSuccess, ("failed to get Material's dependency node.\n"));

	//-- Collect shader material customizations.
	ColorChannelDataVector  colorChannelDataVector;

	const bool materialCollectionResult = collectMaterialCustomizations(materialDependencyNode, colorChannelDataVector);
	MESSENGER_REJECT(!materialCollectionResult, ("failed to collect material customizations.\n"));

	//-- Collect swappable texture customizations
	SwappableTextureDataVector  swappableTextureDataVector;
	TextureNodeMap              textureNodeMap;

	bool const gotTextureNodes = getTextureNodes(materialDependencyNode, textureNodeMap);
	if (gotTextureNodes)
	{
		TextureNodeMap::iterator const endIt = textureNodeMap.end();
		for (TextureNodeMap::iterator it = textureNodeMap.begin(); it != endIt; ++it)
		{
			DEBUG_REPORT_LOG(true, ("texture object api type: [%s].\n", it->second.object().apiTypeStr()));
			if (it->second.object().apiType() == MFn::kLayeredTexture)
			{
				const bool colorTextureCollectionResult = collectTextureCustomizations(it->second, it->first, swappableTextureDataVector);
				MESSENGER_REJECT(!colorTextureCollectionResult , ("failed to collect swappable texture customizations for node [%s].\n", it->second.name().asChar()));
			}
		}
	}

	//-- Write customizable shader template.
	const bool writeSuccess = writeCustomizableShaderTemplate(
		shaderTemplateWriteName, 
		shaderGroupObject, 
		hasVertexAlpha,
		referencedTextureRenderers, 
		textureReferenceDirectory,
		textureRendererReferenceDirectory,
		effectReferenceDirectory,
		textureFilenames,
		hasDot3TextureCoordinate,
		dot3TextureCoordinateIndex,
		colorChannelDataVector,
		swappableTextureDataVector);

	//-- Cleanup local data.
	std::for_each(swappableTextureDataVector.begin(), swappableTextureDataVector.end(), PointerDeleter());

	//-- Return results.
	return writeSuccess;
}

// ======================================================================

bool MayaCustomizableShaderTemplateWriter::getMaterialDependencyNode(const MObject &shaderGroupObject, MFnDependencyNode &materialDependencyNode)
{
	MStatus  status;

	//-- create dep node for shader group node
	MFnDependencyNode  shaderGroupDependencyNode(shaderGroupObject, &status);
	MESSENGER_REJECT(!status, ("failed to create DependencyNode for shaderGroup node [%s].\n", status.errorString().asChar()));

	//-- get the material object
	MObjectArray  connectedObjects;
	const bool findMaterialSuccess = MayaUtility::findSourceObjects(shaderGroupDependencyNode, "surfaceShader", &connectedObjects);
	MESSENGER_REJECT(!findMaterialSuccess, ("failed to find Material node attached to ShaderGroup node.\n"));
	MESSENGER_REJECT(connectedObjects.length() != 1, ("expecting ShaderGroup object to have one Material attached but found %d.\n", connectedObjects.length()));

	//-- assign object for material dep node
	status = materialDependencyNode.setObject(connectedObjects[0]);
	MESSENGER_REJECT(!status, ("failed to set MObject for material's DependencyNode [%s].\n", status.errorString().asChar()));

	//-- return success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the MFnDependencyNode instance for the texture attached
 * to the material's color attribute.
 *
 * This function guarantees that the texture node is either of type
 * MFn::kFileTexture or MFn::kLayeredTexture.
 *
 * @param materialDependencyNode  the MFnDependencyNode instance for the shader material.
 * @param textureDependencyNode   this instance is set with the associated Maya texture node upon successful return.
 *
 * @return  true upon success; false otherwise.
 */

bool MayaCustomizableShaderTemplateWriter::getTextureDependencyNode(const MFnDependencyNode &materialDependencyNode, MFnDependencyNode &textureDependencyNode, const MString &textureAttributeName)
{
	//-- Retrieve the Maya dependency node attached to the material color attribute.
	MObjectArray  mayaTextureObjects;

	const bool gtsSuccess = MayaUtility::findSourceObjects(materialDependencyNode, textureAttributeName, &mayaTextureObjects, true);
	if (!gtsSuccess)
	{
		MESSENGER_LOG(("getTextureDependencyNode(): nothing attached to material [%s] attribute [%s].\n", materialDependencyNode.name().asChar(), textureAttributeName.asChar()));
		return false;
	}

	//-- Ensure there is one and only one attached texture node.
	const unsigned mayaTextureCount = mayaTextureObjects.length();
	if (mayaTextureCount != 1)
	{
		MESSENGER_LOG(("getTextureDependencyNode(): material [%s] channel [%s] had [%u] connections, expecting 1.\n", materialDependencyNode.name().asChar(), textureAttributeName.asChar(), mayaTextureCount));
		return false;
	}

	//-- Ensure the texture node is compatible with the kFileTexture or kLayeredTexture api function set.
	const MObject    mayaTextureObject = mayaTextureObjects[0];
	const MFn::Type  apiType           = mayaTextureObject.apiType();
	const bool       isTextureNode     = (apiType == MFn::kFileTexture) || (apiType == MFn::kLayeredTexture);

	if (!isTextureNode)
	{
		MESSENGER_LOG(("getTextureDependencyNode(): material [%s] channel [%s] does not point to a texture node.\n", materialDependencyNode.name().asChar(), textureAttributeName.asChar()));
		return false;
	}

	//-- Get a MFnDependency instance for the first entry.
	MStatus status = textureDependencyNode.setObject(mayaTextureObject);
	STATUS_REJECT(status, "textureDependencyNode.setObject()");

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------

bool MayaCustomizableShaderTemplateWriter::getPalettePathName(const MFnDependencyNode &dependencyNode, const MString &paletteAttributeName, std::string &paletteReferencePathName)
{
	MStatus  status;

	MObjectArray  objectArray;
	const bool    findPaletteSuccess = MayaUtility::findSourceObjects(dependencyNode, paletteAttributeName, &objectArray);

	MESSENGER_REJECT(!findPaletteSuccess, ("failed to find file connection to palette attribute [%s].\n", paletteAttributeName.asChar()));
	MESSENGER_REJECT(objectArray.length() != 1, ("should only be one object connected to palette attribute [%s].\n", paletteAttributeName.asChar()));
	MESSENGER_REJECT(objectArray[0].apiType() != MFn::kFileTexture, ("palette attached to attribute [%s] must be a File node but isn't.\n", paletteAttributeName.asChar()));

	//-- get palette name
	MFnDependencyNode  fileDependencyNode(objectArray[0], &status);
	MESSENGER_REJECT(!status, ("failed to create dependency node for file object [%s].\n", status.errorString().asChar()));

	MObject      fileNameAttr = fileDependencyNode.attribute ("fileTextureName");
	const MPlug  plugToFileName(objectArray[0], fileNameAttr);
	MObject      valueObject;

	status = plugToFileName.getValue(valueObject);
	MESSENGER_REJECT(!status, ("failed to get value object for fileTextureName,[%s].\n", status.errorString().asChar()));

	MFnStringData fnStringData(valueObject, &status);
	MESSENGER_REJECT(!status, ("failed to set filename string object for fnStringData, ]%s].\n", status.errorString().asChar()));

	MString mFilename = fnStringData.string();
	IGNORE_RETURN(mFilename.toLowerCase());
	const std::string rawPalettePathName(mFilename.asChar());

	//-- chop off any directories and prepend the palette file reference directory
	const std::string::size_type lastPathPosition = rawPalettePathName.find_last_of("/\\");
	if (static_cast<int>(lastPathPosition) == static_cast<int>(std::string::npos))
	{
		// no path present
		paletteReferencePathName = cms_paletteReferenceDirectory + rawPalettePathName;
	}
	else
	{
		// lop off path
		paletteReferencePathName = cms_paletteReferenceDirectory;
		IGNORE_RETURN(paletteReferencePathName.append(rawPalettePathName.substr(lastPathPosition + 1)));
	}

	//-- return success
	return true;
}

// ----------------------------------------------------------------------

bool MayaCustomizableShaderTemplateWriter::getColorChannelData(const IntVector &colorChannelIndices, ColorChannelDataVector &colorChannelDataVector)
{
	//-- find color channel node
	MFnDependencyNode  colorChannelNode;

	const bool findNodeSuccess = MayaUtility::getDependencyNodeFromName(cms_colorChannelDataNodeName, colorChannelNode);
	MESSENGER_REJECT(!findNodeSuccess, ("failed to find color channel node [%s].\n", cms_colorChannelDataNodeName.asChar()));

	//-- retrieve color channel data for each color channel index specified
	std::string  customizationVariableName;
	std::string  paletteReferencePathName;
	bool         variableIsPrivate;

	const IntVector::const_iterator endIt = colorChannelIndices.end();
	for (IntVector::const_iterator it = colorChannelIndices.begin(); it != endIt; ++it)
	{
		const int index = *it;

		//-- convert index to string
		char numberBuffer[32];
		sprintf(numberBuffer, "%d", index);

		//-- get customization variable name
		const MString variableNameAttributeName = cms_customizationVariableAttributeNameBase + numberBuffer;
		if (!MayaUtility::getNodeStringValue(colorChannelNode, variableNameAttributeName, customizationVariableName))
		{
			MESSENGER_LOG_ERROR(("Failed to get string attribute [%s] from node [%s].\n", variableNameAttributeName.asChar(), cms_colorChannelDataNodeName.asChar()));
			return false;
		}

		//-- Get variable privacy status.
		const MString isPrivateAttributeName = cms_customizationIsPrivateAttributeNameBase + numberBuffer;
		if (!MayaUtility::getNodeBoolValue(colorChannelNode, isPrivateAttributeName, &variableIsPrivate, true))
		{
			MESSENGER_LOG(("attribute [%s] not specified, assuming variable [%s] is private.\n", isPrivateAttributeName.asChar(), customizationVariableName.c_str()));
			variableIsPrivate = true;
		}

		//-- get palette path name
		const MString paletteAttributeName = cms_paletteFileAttributeNameBase + numberBuffer;
		if (!getPalettePathName(colorChannelNode, paletteAttributeName, paletteReferencePathName))
		{
			MESSENGER_LOG_ERROR(("Failed to get palette file attribute [%s] from node [%s].\n", paletteAttributeName.asChar(), cms_colorChannelDataNodeName.asChar()));
			return false;
		}

		//-- get default palette index
		int defaultPaletteIndex = -1;
		const MString defaultPaletteIndexAttributeName = cms_defaultPaletteIndexAttributeNameBase + numberBuffer;

		if (!MayaUtility::getNodeIntValue(colorChannelNode, defaultPaletteIndexAttributeName, defaultPaletteIndex))
		{
			MESSENGER_LOG_ERROR(("Failed to get int attribute [%s] from node [%s].\n", defaultPaletteIndexAttributeName.asChar(), cms_colorChannelDataNodeName.asChar()));
			return false;
		}

		//-- create the new ColorChannelData
		colorChannelDataVector.push_back(ColorChannelData(customizationVariableName, variableIsPrivate, paletteReferencePathName, defaultPaletteIndex));
	}

	//-- return success
	return true;
}

// ----------------------------------------------------------------------

int MayaCustomizableShaderTemplateWriter::getTextureCount(const SwappableTextureDataVector &swappableTextureDataVector)
{
	int textureCount = 0;

	const SwappableTextureDataVector::const_iterator endIt = swappableTextureDataVector.end();
	for (SwappableTextureDataVector::const_iterator it = swappableTextureDataVector.begin(); it != endIt; ++it)
	{
		//-- Get the SwappableTextureData instance.
		const SwappableTextureData *const data = *it;
		NOT_NULL(data);

		//-- Count the number of textures for this customization.
		textureCount += data->getTexturePathNameCount();
	}

	//-- Return the texture count.
	return textureCount;
}

// ----------------------------------------------------------------------

bool MayaCustomizableShaderTemplateWriter::collectMaterialCustomizations(const MFnDependencyNode &materialDependencyNode, ColorChannelDataVector &colorChannelDataVector)
{
	//-- Get first color channel attribute.
	IntVector  colorChannelIndices;
	colorChannelIndices.reserve(2);

	int        firstColorChannelValue = 0;
	const bool firstAttributeValueFound = MayaUtility::getNodeIntValue(materialDependencyNode, cms_firstColorChannelAttributeName, firstColorChannelValue, true);

	if (firstAttributeValueFound && (firstColorChannelValue > 0))
		colorChannelIndices.push_back(firstColorChannelValue);

	if (!firstAttributeValueFound || (firstColorChannelValue == 0))
	{
		// No hue customization.
		return true;
	}

	//-- Get second color channel attribute.
	int        secondColorChannelValue = 0;
	const bool secondAttributeValueFound = MayaUtility::getNodeIntValue(materialDependencyNode, cms_secondColorChannelAttributeName, secondColorChannelValue, true);

	if (secondAttributeValueFound && (secondColorChannelValue > 0))
		colorChannelIndices.push_back(secondColorChannelValue);

	//-- Fetch the customization data from the colorChannelNode.
	colorChannelDataVector.reserve(colorChannelIndices.size());

	const bool getDataResult = getColorChannelData(colorChannelIndices, colorChannelDataVector);
	MESSENGER_REJECT(!getDataResult, ("failed to get color channel data.\n"));

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------

bool MayaCustomizableShaderTemplateWriter::collectTextureCustomizations(const MFnDependencyNode &textureDependencyNode, const Tag &textureTag, SwappableTextureDataVector &swappableTextureDataVector)
{
	//-- Check if Maya node is a layered texture.
	const MObject baseTextureObject = textureDependencyNode.object();
	if (baseTextureObject.apiType() != MFn::kLayeredTexture)
	{
		// Done, skip gracefully.
		return true;
	}

	//-- Get the customization variable name.
	std::string  customizationVariableName;

	const bool getVarNameSuccess = MayaUtility::getNodeStringValue(textureDependencyNode, cms_swappableTextureVariableNameAttributeName, customizationVariableName);
	if (!getVarNameSuccess)
	{
		MESSENGER_LOG(("Texture node [%s] is a layered texture but does not contain attribute name [%s].\n", textureDependencyNode.name().asChar(), cms_swappableTextureVariableNameAttributeName.asChar()));
		return false;
	}

	//-- Get the private/public status.
	bool  variableIsPrivate = false;

	const bool getPrivateSuccess = MayaUtility::getNodeBoolValue(textureDependencyNode, cms_swappableTexturePrivateAttributeName, &variableIsPrivate);
	if (!getPrivateSuccess)
	{
		MESSENGER_LOG(("Texture node [%s] is a layered texture but does not contain attribute name [%s].\n", textureDependencyNode.name().asChar(), cms_swappableTexturePrivateAttributeName.asChar()));
		return false;
	}

	//-- Get the default index value.
	int defaultTextureIndex = 0;

	const bool getDefaultIndexSuccess = MayaUtility::getNodeIntValue(textureDependencyNode, cms_swappableTextureDefaultIndexAttributeName, defaultTextureIndex);
	if (!getDefaultIndexSuccess)
	{
		MESSENGER_LOG(("Texture node [%s] is a layered texture but does not contain attribute name [%s].\n", textureDependencyNode.name().asChar(), cms_swappableTextureDefaultIndexAttributeName.asChar()));
		return false;
	}

	//-- Get the array of Maya nodes attached to the layered texture "inputs[x].color" attribute.
	MObjectArray  attachedTextureObjects;
	const bool gatoResult = MayaUtility::getAttachedTextureObjects(textureDependencyNode, attachedTextureObjects);
	MESSENGER_REJECT(!gatoResult ,("MayaUtility::getAttachedTextureObjects() failed.\n"));

	const unsigned attachedTextureCount = attachedTextureObjects.length();
	MESSENGER_REJECT(attachedTextureCount < 1, ("Texture node [%s] is a layered texture but does not have any texture layers assigned.\n", textureDependencyNode.name().asChar()));

	//-- Create the SwappableTextureData.
	swappableTextureDataVector.push_back(new SwappableTextureData(textureTag, customizationVariableName.c_str(), variableIsPrivate, defaultTextureIndex));
	SwappableTextureData *const data = swappableTextureDataVector.back();

	//-- Collect the texture choices.
	MFnDependencyNode  textureNode;
	std::string        mayaPathName;
	std::string        textureBaseName;
	std::string        relativePathName;
	MStatus            status;

	for (unsigned i = 0; i < attachedTextureCount; ++i)
	{
		//-- Get MFnDependencyNode instance for attached texture object.
		const MObject textureObject = attachedTextureObjects[i];

		status = textureNode.setObject(textureObject);
		STATUS_REJECT(status, "textureNode.setObject() for layered texture attached texture MObject");

		//-- Get texture path name attribute.
		const bool getNameSuccess = MayaUtility::getNodeStringValue(textureNode, cms_texturePathNameAttributeName, mayaPathName);
		MESSENGER_REJECT(!getNameSuccess, ("Attached texture node [%s] failed to retrieve path name attribute.\n", textureNode.name().asChar()));

		//-- Strip off path.
		std::string::size_type endOfPathPos = mayaPathName.rfind('/');
		if ((static_cast<int>(endOfPathPos) == static_cast<int>(std::string::npos)) || (endOfPathPos == (mayaPathName.length() - 1)))
			textureBaseName = mayaPathName;
		else
			IGNORE_RETURN(textureBaseName.assign(mayaPathName.begin() + (endOfPathPos + 1), mayaPathName.end()));

		//-- Truncate extension.
		std::string::size_type startOfExtensionPos = textureBaseName.rfind('.');
		if (static_cast<int>(startOfExtensionPos) != static_cast<int>(std::string::npos))
			textureBaseName.resize(startOfExtensionPos);

		//-- Construct TreeFile-relative path name.
		relativePathName = SetDirectoryCommand::getDirectoryString(TEXTURE_REFERENCE_DIR_INDEX);
		relativePathName += textureBaseName;
		relativePathName += ".dds";

		data->addTexturePathName(relativePathName);

		MESSENGER_LOG(("LT -| choice [%u]: texture [%s].\n", i, relativePathName.c_str()));
	}

	//-- Validate the default index against the available range.
	MESSENGER_REJECT((defaultTextureIndex < 0) || (defaultTextureIndex >= data->getTexturePathNameCount()), ("Texture node [%s] has default index [%d] out of valid range [0 - %d).\n", textureDependencyNode.name().asChar(), defaultTextureIndex, data->getTexturePathNameCount()));

	//-- Indicate success.
	return true;
}

// ----------------------------------------------------------------------

bool MayaCustomizableShaderTemplateWriter::writeCustomizableShaderTemplate(
		const std::string                &shaderTemplateWriteName, 
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
		const SwappableTextureDataVector &swappableTextureDataVector)
{
	//-- Sanity check, we should have color channel or swappable texture data at this point.
	const bool hasColorChannelData     = !colorChannelDataVector.empty();
	const bool hasSwappableTextureData = !swappableTextureDataVector.empty();

	MESSENGER_REJECT(!(hasColorChannelData || hasSwappableTextureData), ("no color channel or swappable texture data, how did we think this Maya shader was a CustomizableShader?\n"));

	//-- Start building the iff data.
	Iff          iff(64 * 1024);

	// @todo move this into a CustomizableShaderTemplateWriter
	iff.insertForm(TAG_CSHD);
		iff.insertForm(TAG_0001);

			//-- insert the static shader template data here
			bool writeStaticShaderSuccess = MayaStaticShaderTemplateWriter::build(
					shaderGroupObject, 
					hasVertexAlpha,
					referencedTextureRenderers, 
					textureReferenceDirectory,
					textureRendererReferenceDirectory,
					effectReferenceDirectory,
					textureFilenames,
					hasDot3TextureCoordinate,
					dot3TextureCoordinateIndex
			);

			if (writeStaticShaderSuccess)
			{
				MayaStaticShaderTemplateWriter::writeToIff(iff);
			}

			MESSENGER_REJECT(!writeStaticShaderSuccess, ("failed to write static shader data for [%s].\n", shaderTemplateWriteName.c_str()));

			if (hasSwappableTextureData)
			{
				iff.insertForm(TAG_TXTR);

					//-- Get total texture count.
					const int textureCount = getTextureCount(swappableTextureDataVector);
					MESSENGER_REJECT(textureCount < 1, ("invalid texture count [%d].\n", textureCount));

					//-- Write TreeFile-relative texture names.
					iff.insertChunk(TAG_DATA);
					{	
						iff.insertChunkData(static_cast<int16>(textureCount));

						const SwappableTextureDataVector::const_iterator endIt = swappableTextureDataVector.end();
						for (SwappableTextureDataVector::const_iterator it = swappableTextureDataVector.begin(); it != endIt; ++it)
						{
							const SwappableTextureData *const data = *it;
							NOT_NULL(data);

							const int localTextureCount = data->getTexturePathNameCount();
							for (int textureIndex = 0; textureIndex < localTextureCount; ++textureIndex)
							{
								//-- Retrieve the texture name.
								const std::string &texturePathName = data->getTexturePathName(textureIndex);

								//-- Insert this as a texture pathname available for texture customizations.
								// NOTE: a given texture path name may appear in this list twice if two texture 
								// customizations (textureIndex.e. for two different tagged textures of the shader) use the
								// same texture.  This is okay.
								iff.insertChunkString(texturePathName.c_str());
							}
						}
					}
					iff.exitChunk(TAG_DATA);

					//-- Write customizations.
					iff.insertForm(TAG_CUST);
					{
						int customizationFirstTextureIndex = 0;

						const SwappableTextureDataVector::const_iterator endIt = swappableTextureDataVector.end();
						for (SwappableTextureDataVector::const_iterator it = swappableTextureDataVector.begin(); it != endIt; ++it)
						{
							//-- Get the SwappableTextureData instance.
							const SwappableTextureData *const data = *it;
							NOT_NULL(data);

							//-- Add a SelectTexture1d customization.
							iff.insertChunk(TAG_TX1D);

								iff.insertChunkData(static_cast<uint32>(data->getTextureTag()));
								iff.insertChunkData(static_cast<int16>(customizationFirstTextureIndex));
								iff.insertChunkData(static_cast<int16>(data->getTexturePathNameCount()));

								iff.insertChunkString(data->getCustomizationVariableName().getString());
								iff.insertChunkData(static_cast<int8>(data->isVariablePrivate() ? 1 : 0));

								const int defaultIndex = data->getDefaultIndex();
								VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, defaultIndex, data->getTexturePathNameCount());

								iff.insertChunkData(static_cast<int16>(defaultIndex));

							iff.exitChunk(TAG_TX1D);

							//-- Increment subscript of first texture index for this particular customization.
							customizationFirstTextureIndex += data->getTexturePathNameCount();
						}

					}
					iff.exitForm(TAG_CUST);
					

				iff.exitForm(TAG_TXTR);
			}

			if (hasColorChannelData)
			{
				//-- Insert tfactor-related customizations.
				iff.insertForm(TAG_TFAC);
				{
					int channelIndex = 0;

					const ColorChannelDataVector::const_iterator endIt = colorChannelDataVector.end();
					for (ColorChannelDataVector::const_iterator it = colorChannelDataVector.begin(); it != endIt; ++it, ++channelIndex)
					{
						const ColorChannelData &ccd = *it;

						//-- figure out which tag to write for this channel index
						DEBUG_FATAL(channelIndex > 1, ("-TRF- we don't support more than two channel hueing at this time.\n"));
						const Tag tfactorTag = (channelIndex == 0) ? TAG_MAIN : TAG_HUEB;

						//--write the material entry
						iff.insertChunk(TAG_PAL);

							iff.insertChunkString(ccd.getCustomizationVariableName().c_str());
							iff.insertChunkData(static_cast<int8>(ccd.isVariablePrivate() ? 1 : 0));
							iff.insertChunkData(static_cast<uint32>(tfactorTag));
							iff.insertChunkString(ccd.getPalettePathName().c_str());
							iff.insertChunkData(static_cast<int32>(ccd.getDefaultPaletteIndex()));

						iff.exitChunk(TAG_PAL);
					}
				}

				iff.exitForm(TAG_TFAC);
			}

		iff.exitForm(TAG_0001);
	iff.exitForm(TAG_CSHD);

	//-- write iff to disk
	const bool writeResult = iff.write(shaderTemplateWriteName.c_str(), true);
	MESSENGER_REJECT(!writeResult, ("failed to write customizable shader template data to file [%s].\n", shaderTemplateWriteName.c_str()));

	MESSENGER_LOG(("* Exported customizable ShaderTemplate [%s] using effect [%s].\n", shaderTemplateWriteName.c_str(), MayaStaticShaderTemplateWriter::getEffectName().c_str()));

	//-- now that we've built all the data, archive it to the log file
	ExporterLog::addDestinationShaderToCurrentMesh(shaderTemplateWriteName, MayaStaticShaderTemplateWriter::getEffectName());

	//-- return success
	return true;
}

// ======================================================================
