// ======================================================================
//
// MayaOwnerProxyShaderTemplateWriter.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "MayaOwnerProxyShaderTemplateWriter.h"

#include "ExporterLog.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MObjectArray.h"
#include "MayaShaderTemplateBuilder.h"
#include "Messenger.h"
#include "sharedFile/Iff.h"

#include <string>

// ======================================================================

const int      MayaOwnerProxyShaderTemplateWriter::cms_priority = 20;
const int      MayaOwnerProxyShaderTemplateWriter::cms_iffWriteSize = 8 * 1024;
const MString  MayaOwnerProxyShaderTemplateWriter::cms_baseShaderNameAttribute("soe_skinSwap");

bool       MayaOwnerProxyShaderTemplateWriter::ms_installed;
Messenger *MayaOwnerProxyShaderTemplateWriter::messenger;

// ======================================================================

const Tag TAG_OPST = TAG(O,P,S,T);

// ======================================================================

void MayaOwnerProxyShaderTemplateWriter::install(Messenger *newMessenger)
{
	DEBUG_FATAL(ms_installed, ("MayaOwnerProxyShaderTemplateWriter already installed"));

	messenger = newMessenger;

	//-- register this MayaShaderTemplateWriter with the builder class
	MayaShaderTemplateBuilder::registerShaderTemplateWriter(new MayaOwnerProxyShaderTemplateWriter(), cms_priority);

	ms_installed = true;
}

// ----------------------------------------------------------------------

void MayaOwnerProxyShaderTemplateWriter::remove()
{
	ms_installed = false;
}

// ======================================================================

bool MayaOwnerProxyShaderTemplateWriter::canWrite(const MObject &shaderGroupObject) const
{
	//-- Retrieve the material node.
	MFnDependencyNode  materialDependencyNode;

	const bool gotDn = getMaterialDependencyNode(shaderGroupObject, materialDependencyNode);
	MESSENGER_REJECT(!gotDn, ("failed to get material node for shader group object, malformed shader group."));

	//-- Check for the presence of the "soe_skinSwap" attribute.
	std::string baseShaderName;
	const bool  hasBaseShaderNameAttribute = MayaUtility::getNodeStringValue(materialDependencyNode, cms_baseShaderNameAttribute, baseShaderName, true);

	return hasBaseShaderNameAttribute;
}

// ----------------------------------------------------------------------

bool MayaOwnerProxyShaderTemplateWriter::write(
	const std::string       &shaderTemplateWriteName, 
	const MObject           &shaderGroupObject, 
	bool                      /* hasVertexAlpha */,
	TextureRendererVector   & /* referencedTextureRenderers */, 
	const std::string       & /* textureReferenceDirectory */,
	const std::string       & /* textureRendererReferenceDirectory */,
	const std::string       & /* effectReferenceDirectory */,
	MayaUtility::TextureSet & /* textureFilenames */,
	bool                      /* hasDot3TextureCoordinate */,
	int                       /* dot3TextureCoordinateIndex */)
{
	//-- Retrieve the material node.
	MFnDependencyNode  materialDependencyNode;

	const bool gotDn = getMaterialDependencyNode(shaderGroupObject, materialDependencyNode);
	MESSENGER_REJECT(!gotDn, ("failed to get material node for shader group object, malformed shader group."));

	//-- Check for the presence of the "soe_skinSwap" attribute.
	std::string baseShaderName;
	const bool  hasBaseShaderNameAttribute = MayaUtility::getNodeStringValue(materialDependencyNode, cms_baseShaderNameAttribute, baseShaderName, true);
	MESSENGER_REJECT(!hasBaseShaderNameAttribute, ("Material [%s] doesn't have the skin swap attribute but should, logic error.", materialDependencyNode.name().asChar()));

	//-- Write the data.
	Iff iff(cms_iffWriteSize);

	iff.insertForm(TAG_OPST);
		iff.insertForm(TAG_0000);

			iff.insertChunk(TAG_INFO);
				iff.insertChunkString(baseShaderName.c_str());
			iff.exitChunk(TAG_INFO);

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_OPST);

	//-- Write iff to disk
	const bool writeResult = iff.write(shaderTemplateWriteName.c_str(), true);
	MESSENGER_REJECT(!writeResult, ("failed to write owner proxy shader template data to file [%s].\n", shaderTemplateWriteName.c_str()));

	MESSENGER_LOG(("wrote skin shader [%s] with base name [%s].\n", shaderTemplateWriteName.c_str(), baseShaderName.c_str()));

	ExporterLog::addClientDestinationFile(shaderTemplateWriteName);
	
	return true;
}

// ======================================================================

bool MayaOwnerProxyShaderTemplateWriter::getMaterialDependencyNode(const MObject &shaderGroupObject, MFnDependencyNode &materialDependencyNode)
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

// ======================================================================
