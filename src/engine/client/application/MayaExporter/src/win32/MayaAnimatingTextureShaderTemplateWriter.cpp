// ======================================================================
//
// MayaAnimatingTextureShaderTemplateWriter.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "MayaAnimatingTextureShaderTemplateWriter.h"

#include "ExporterLog.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnStringData.h"
#include "maya/MObjectArray.h"
#include "maya/MPlug.h"
#include "maya/MPlugArray.h"

#include "MayaShaderTemplateBuilder.h"
#include "MayaStaticShaderTemplateWriter.h"
#include "Messenger.h"
#include "PluginMain.h"
#include "SetDirectoryCommand.h"

#include "sharedFile/Iff.h"

#include <string>
#include <algorithm>

// ======================================================================

const int      MayaAnimatingTextureShaderTemplateWriter::cms_priority = 30;
const int      MayaAnimatingTextureShaderTemplateWriter::cms_iffWriteSize = 8 * 1024;
const MString  MayaAnimatingTextureShaderTemplateWriter::cms_fpsMinAttribute("soe_animatingFpsMin");
const MString  MayaAnimatingTextureShaderTemplateWriter::cms_fpsMaxAttribute("soe_animatingFpsMax");
const MString  MayaAnimatingTextureShaderTemplateWriter::cms_orderAttribute("soe_animatingOrder");
const MString  MayaAnimatingTextureShaderTemplateWriter::cms_frameExtensionAttribute("useFrameExtension");

bool       MayaAnimatingTextureShaderTemplateWriter::ms_installed;
Messenger *MayaAnimatingTextureShaderTemplateWriter::messenger;

// ======================================================================

const Tag TAG_SWTS = TAG(S,W,T,S);
const Tag TAG_MAIN = TAG(M,A,I,N);
const Tag TAG_TEXT = TAG(T,E,X,T);

// supported ordering (selector) tags
const Tag TAG_DRTS = TAG(D,R,T,S);  // DeltaRandomTimeSwitcherTemplate
const Tag TAG_DRFT = TAG(D,R,F,T);  // DeltaRandomFrameTimeSwitcherTemplate
const Tag TAG_DPPT = TAG(D,P,P,T);  // DeltaPingPongSwitcherTemplate

// enumerated values for selection order. These values must match the attribute
// definition on the file object in Maya. As in the MEL command:
//    addAttr -ln soe_animatingOrder -at "enum" -en "Looping:PingPong:Random:";
const int ORDER_LOOPING = 0;
const int ORDER_PINGPONG = 1;
const int ORDER_RANDOM = 2;


// ======================================================================

void MayaAnimatingTextureShaderTemplateWriter::install(Messenger *newMessenger)
{
	DEBUG_FATAL(ms_installed, ("MayaAnimatingTextureShaderTemplateWriter already installed"));

	messenger = newMessenger;

	//-- register this MayaShaderTemplateWriter with the builder class
	MayaShaderTemplateBuilder::registerShaderTemplateWriter(new MayaAnimatingTextureShaderTemplateWriter(), cms_priority);

	ms_installed = true;
}

// ----------------------------------------------------------------------

void MayaAnimatingTextureShaderTemplateWriter::remove()
{
	ms_installed = false;
}

// ======================================================================

bool MayaAnimatingTextureShaderTemplateWriter::canWrite(const MObject &shaderGroupObject) const
{
    bool useMe = false;

    //-- Retrieve the file node
    MFnDependencyNode fileNode;

    const bool gotFileNode = getFileNode(shaderGroupObject, fileNode);

    if (gotFileNode)
    {
	    //-- Check for the presence of the cms_frameExtensionAttribute attribute.
	    bool useFrameExtension;
	    const bool foundExtension = MayaUtility::getNodeBoolValue(fileNode, cms_frameExtensionAttribute, &useFrameExtension);

        if (foundExtension && useFrameExtension)
        {
	        //-- Check for the presence of the "soe_animating*" attributes.
	        float fpsMin;

            useMe = MayaUtility::getNodeFloatValue(fileNode, cms_fpsMinAttribute, &fpsMin);
            if (useMe)
            {
                float fpsMax;

    	        useMe = MayaUtility::getNodeFloatValue(fileNode, cms_fpsMaxAttribute, &fpsMax);
                if (useMe)
                {
                    int orderEnum;

        	        useMe = MayaUtility::getNodeIntValue(fileNode, cms_orderAttribute, orderEnum);
                    if (!useMe)
                    {
                        MESSENGER_LOG_WARNING(("%s is set, but attribute %s is missing. %s will not animate.",cms_frameExtensionAttribute.asChar(),cms_orderAttribute.asChar(),fileNode.name().asChar()) );
                    }
                }
                else
                {
                    MESSENGER_LOG_WARNING(("%s is set, but attribute %s is missing. %s will not animate.",cms_frameExtensionAttribute.asChar(),cms_fpsMaxAttribute.asChar(),fileNode.name().asChar()) );
                }
            }
            else
            {
                MESSENGER_LOG_WARNING(("%s is set, but attribute %s is missing. %s will not animate.",cms_frameExtensionAttribute.asChar(),cms_fpsMinAttribute.asChar(),fileNode.name().asChar()) );
            }

        }
    }

    return useMe ;
}

// ----------------------------------------------------------------------

bool MayaAnimatingTextureShaderTemplateWriter::write(
	const std::string       &shaderTemplateWriteName,
	const MObject           &shaderGroupObject,
	bool                     hasVertexAlpha,
	TextureRendererVector   & referencedTextureRenderers,
	const std::string       & textureReferenceDirectory,
	const std::string       & textureRendererReferenceDirectory,
	const std::string       & effectReferenceDirectory,
	MayaUtility::TextureSet & textureFilenames,
	bool                      hasDot3TextureCoordinate,
	int                       dot3TextureCoordinateIndex)
{
	//-- Retrieve the file node, get the values for frame rate and ordering
	bool result;

	MFnDependencyNode  fileNode;
    result = getFileNode(shaderGroupObject, fileNode);
	MESSENGER_REJECT(!result, ("MayaAnimatingTextureShaderTemplateWriter failed to find texture file node\n"));

	float fpsMin = 0.f;
    result = MayaUtility::getNodeFloatValue(fileNode, cms_fpsMinAttribute, &fpsMin);
    MESSENGER_REJECT(!result, ("File texture [%s] doesn't have the [%s] attribute but should, logic error.", fileNode.name().asChar(), cms_fpsMinAttribute.asChar()));
    MESSENGER_REJECT(fpsMin <= 0.f, ("Invalid minimum frame rate specified [%f]. %s must be >= 0.0.", fpsMin, cms_fpsMinAttribute.asChar()));

	float fpsMax = 0.f;
    result = MayaUtility::getNodeFloatValue(fileNode, cms_fpsMaxAttribute, &fpsMax);
    MESSENGER_REJECT(!result, ("File texture [%s] doesn't have the [%s] attribute but should, logic error.", fileNode.name().asChar(), cms_fpsMaxAttribute.asChar()));

    if (fpsMax < fpsMin)
    {
		MESSENGER_LOG_WARNING(("File texture [%s] maxFps is less than minFps. Setting maxFps = minFps", fileNode.name().asChar()));
        fpsMax = fpsMin;
    }

    int orderEnum;
    result = MayaUtility::getNodeIntValue(fileNode, cms_orderAttribute, orderEnum);
    MESSENGER_REJECT(!result, ("File texture [%s] doesn't have the [%s] attribute but should, logic error.", fileNode.name().asChar(), cms_orderAttribute.asChar()));


    // timeMax is the inverse of the MIN frame rate
    // timeMin is the inverse of the MAX frame rate
    float timeMax = 1.f / fpsMin;
    float timeMin = 1.f / fpsMax;
    Tag orderTag;

    switch(orderEnum)
    {
    case ORDER_LOOPING:
        orderTag = TAG_DRTS;
        break;
    case ORDER_PINGPONG:
        orderTag = TAG_DPPT;
        break;
    case ORDER_RANDOM:
        orderTag = TAG_DRFT;
        break;
    default:
        MESSENGER_REJECT(true, ("Unknown ordering specified for animating texture."));
        break;
    }



    //-- Write the base shader that we will reference
    std::string baseTemplateName = shaderTemplateWriteName;
    std::string::size_type insPos = baseTemplateName.find_last_of('.');
    baseTemplateName.insert(insPos, "_base");
    MayaStaticShaderTemplateWriter baseWriter;
    bool wroteBase = baseWriter.write(baseTemplateName,
	                                            shaderGroupObject,
                                                hasVertexAlpha,
	                                            referencedTextureRenderers,
	                                            textureReferenceDirectory,
	                                            textureRendererReferenceDirectory,
	                                            effectReferenceDirectory,
	                                            textureFilenames,
	                                            hasDot3TextureCoordinate,
	                                            dot3TextureCoordinateIndex);

    MESSENGER_REJECT(!wroteBase, ("Animating texture shader could not write base shader."));

    //-- Get the MAIN texture
    TextureNameVector  relativeTextureNames;
	result = findTextureSet(shaderGroupObject, textureReferenceDirectory, textureFilenames, relativeTextureNames, CT_texture);
	MESSENGER_REJECT(!result,("could not get MAIN texture(s)"));
    MESSENGER_REJECT(relativeTextureNames.size() == 0, ("No texture files found for animating texture shader."));

    std::sort(relativeTextureNames.begin(), relativeTextureNames.end());

    //-- Write the data.
	Iff iff(cms_iffWriteSize);

	iff.insertForm(TAG_SWTS);
		iff.insertForm(TAG_0000);
			iff.insertChunk(TAG_NAME);
			{
		        char referenceFileName[512];
		        MayaUtility::stripBaseName(baseTemplateName.c_str(), referenceFileName, sizeof(referenceFileName), '\\');
				std::string const shaderRelativeName = std::string(SetDirectoryCommand::getDirectoryString(SHADER_TEMPLATE_REFERENCE_DIR_INDEX)) + referenceFileName + ".sht";
                iff.insertChunkString(shaderRelativeName.c_str());
			}
			iff.exitChunk(TAG_NAME);

            iff.insertForm(orderTag);
                iff.insertChunk(TAG_0000);
                    iff.insertChunkData(relativeTextureNames.size());
                    iff.insertChunkData(timeMin);
                    iff.insertChunkData(timeMax);
                iff.exitChunk(TAG_0000);
            iff.exitForm(orderTag);

            TextureNameVector::const_iterator i;
            for (i = relativeTextureNames.begin(); i != relativeTextureNames.end(); ++i)
            {
			    iff.insertChunk(TAG_TEXT);
				    iff.insertChunkData( TAG_MAIN );
                    iff.insertChunkString( i->c_str() );
                iff.exitChunk(TAG_TEXT);
            }

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_SWTS);

	//-- Write iff to disk
	const bool writeResult = iff.write(shaderTemplateWriteName.c_str(), true);
	MESSENGER_REJECT(!writeResult, ("failed to write owner proxy shader template data to file [%s].\n", shaderTemplateWriteName.c_str()));

	MESSENGER_LOG(("wrote animating texture shader [%s] with frame rate min/max [%f/%f].\n", shaderTemplateWriteName.c_str(), fpsMin, fpsMax));

	ExporterLog::addClientDestinationFile(shaderTemplateWriteName);

    return true;
}

// ======================================================================

bool MayaAnimatingTextureShaderTemplateWriter::getMaterialDependencyNode(const MObject &shaderGroupObject, MFnDependencyNode &materialDependencyNode)
{
	MESSENGER_REJECT (shaderGroupObject.apiType () != MFn::kShadingEngine, ("getMaterialDependencyNode () expecting object of type MFn::kShaderEngine, found type %s\n", shaderGroupObject.apiTypeStr ()));

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

// ======================================================================

bool MayaAnimatingTextureShaderTemplateWriter::getFileNode( const MObject& shaderGroupObject, MFnDependencyNode& fnFileNode )
{

	//get the shader node attached to the shader group
	bool               result;
	MStatus            status;
	MObjectArray       objectArray;

	MFnDependencyNode  materialDependencyNode;
    result = getMaterialDependencyNode( shaderGroupObject, materialDependencyNode );

    MESSENGER_REJECT (!result, ("getMaterialDependencyNode failed."));

	//find texture attached to shader
	result = MayaUtility::findSourceObjects (materialDependencyNode, "color", &objectArray);

    MESSENGER_REJECT (!result, ("failed to get source objects connected to shader\"%s\"\n", materialDependencyNode.name ().asChar ()));
	MESSENGER_REJECT (objectArray.length() != 1, ("expecting surfaceShader to have one upstream connection, found %d\n", objectArray.length ()));

    if (objectArray[0].apiType () != MFn::kFileTexture)
    {
        return false;
    }

	//set dependency node to the texture node
	status = fnFileNode.setObject (objectArray [0]);
	MESSENGER_REJECT (!status, ("failed to set file texture for fnFileNode,\"%s\"\n", status.errorString ().asChar ()));

	// fnFileNode now points to a "file" node.
    return true;
}



// ----------------------------------------------------------------------
/**
 * Pull the attached textures and add them to the system
 */

bool MayaAnimatingTextureShaderTemplateWriter::findTextureSet(
	  const MObject             &shaderGroupObject,
	  const std::string         &textureReferenceDirectory,
	  MayaUtility::TextureSet   &texturesToWrite,
      TextureNameVector        &relativeTextureNames,
	  CompressType               compressType
)
{
    MFnDependencyNode fnFileNode;

    bool result = getFileNode( shaderGroupObject, fnFileNode );
	MESSENGER_REJECT(!result, ("MayaAnimatingTextureShaderTemplateWriter failed to find texture file node\n"));

	//get texture name
    std::string filename;
    result = MayaUtility::getNodeStringValue (fnFileNode, "fileTextureName", filename);
	MESSENGER_REJECT(!result, ("failed to find texture file name on file node\n"));

	//-- Remove Maya project '//' from path name.
	const std::string::size_type projectStartPos = filename.find("//");
	if (static_cast<int>(projectStartPos) != static_cast<int>(std::string::npos))
	{
		// Remove the first '/'.
		filename.erase(projectStartPos, 1);
		MESSENGER_LOG_WARNING(("texture file [%s] contained a Maya project-relative '//', please fix.\n", filename.c_str()));
	}


    TextureNameVector foundFiles;
    findTextureFiles( filename, foundFiles );

    TextureNameVector::iterator i;
    for (i = foundFiles.begin(); i != foundFiles.end(); ++i)
    {
        char textureBaseName[128];

        MayaUtility::stripBaseName(i->c_str(), textureBaseName, sizeof(textureBaseName), '/');


		//convert the texture from a fully-pathed file to texture\<filename>.dds
		std::string textureRelativeName =  textureReferenceDirectory;
			        textureRelativeName += textureBaseName;
			        textureRelativeName += ".dds";


		texturesToWrite.insert(std::make_pair(*i, compressType));
        relativeTextureNames.push_back(textureRelativeName);

		ExporterLog::addSourceTexture(*i);
		ExporterLog::addDestinationTextureToCurrentShader(textureRelativeName);
    }

	return true;
}


//-----------------------------------------------
// Search the file system for texture files in the form "texturename.??.tga"

bool MayaAnimatingTextureShaderTemplateWriter::findTextureFiles( const std::string& filename, TextureNameVector& foundFiles )
{
    WIN32_FIND_DATA findData;
    HANDLE hFind;

    std::string::size_type pathEnd = filename.find_last_of('/') + 1;
    std::string::size_type lastDot = filename.find_last_of('.');
    std::string::size_type extensionBegin =  lastDot;
    std::string::size_type nameEnd = filename.find_last_of('.', lastDot - 1);

    if (nameEnd == std::string::npos)
    {
        nameEnd = lastDot;
    }

    std::string searchMask = filename.substr(0, nameEnd) + ".??" + filename.substr(extensionBegin, filename.size());
    std::string filePath = filename.substr(0, pathEnd);


    hFind = FindFirstFile( searchMask.c_str(), &findData );

    MESSENGER_REJECT( hFind == INVALID_HANDLE_VALUE, ("Error searching for files for Animating Shader"));

    do
    {
        foundFiles.push_back(filePath + findData.cFileName);
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);

    return true;
}

