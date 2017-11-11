// ======================================================================
//
// ExporterLog.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

//precompiled header include
#include "FirstMayaExporter.h"

//module include
#include "ExporterLog.h"

//engine shared includes
#include "sharedObject/Hardpoint.h"
#include "sharedMath/Transform.h"

//local mayaExporter includes
#include "AlienbrainImporter.h"
#include "Messenger.h"

//maya sdk includes
#include "maya/MFnMesh.h"

//system / STL includes
#include <utility> //for make_pair
#include <stdio.h>
#include <string>
#include <list>
#include <map>

//-------------------------------------------------------------------

bool                                      ExporterLog::ms_installed;
std::string                               ExporterLog::ms_sourceFilename;
std::string                               ExporterLog::ms_author;
int                                       ExporterLog::ms_sourceNumberOfVertices;
int                                       ExporterLog::ms_sourceNumberOfPolygons;
int                                       ExporterLog::ms_sourceNumberOfUVSets;
std::vector<ExporterLog::BaseLogInfo *>   ExporterLog::ms_currentObjects;
ExporterLog::BaseLogInfo *                ExporterLog::ms_objects;

FILE*                                     ExporterLog::ms_outfile;

std::string                               ExporterLog::ms_logFilename;
std::string                               ExporterLog::ms_mayaCommand;
std::string                               ExporterLog::ms_exportOptions;
std::string                               ExporterLog::ms_assetGroupName;
std::string                               ExporterLog::ms_assetName;
int                                       ExporterLog::ms_assetId;

std::set<std::string>                     ExporterLog::ms_destClientFileContainer;
std::set<std::string>                     ExporterLog::ms_destSharedFileContainer;
std::set<std::string>                     ExporterLog::ms_destServerFileContainer;
std::set<std::string>                     ExporterLog::ms_sourceFileContainer;
std::vector<std::string>                  ExporterLog::ms_sourceTexturesContainer;
std::string                               ExporterLog::ms_baseDir;
std::string                               ExporterLog::ms_mayaExporterVersion;

Messenger*                                ExporterLog::messenger;

const int                                 ExporterLog::ms_versionNumber        = 5;

const std::string                         ExporterLog::ms_meshDirectory        = "appearance\\mesh";
const std::string                         ExporterLog::ms_lodDirectory         = "appearance\\lod";
const std::string                         ExporterLog::ms_componentDirectory   = "appearance\\component";

//-------------------------------------------------------------------

ExporterLog::MeshLogInfo::MeshLogInfo(const std::string &filename)
: ExporterLog::BaseLogInfo(filename,"Mesh"),
  m_numberOfVertices(0),
  m_numberOfPolygons(0),
  m_sphereCenter(),
  m_sphereRadius(0),
  m_boxMin(),
  m_boxMax(),
  m_hardpointList(),
  m_numUVSets(0)
{}

//-------------------------------------------------------------------

ExporterLog::MeshLogInfo::~MeshLogInfo()
{
	for(int numHP = 0; numHP < static_cast<int>(m_hardpointList.size()); ++numHP)
		delete m_hardpointList[static_cast<unsigned int>(numHP)];
}

//-------------------------------------------------------------------

ExporterLog::ShaderLogInfo::ShaderLogInfo(const std::string &filename, const std::string &effectName)
: ExporterLog::BaseLogInfo(filename,"Shader"),
  m_effectFilename(effectName),
  m_textureFilenameList()
{
}

//-------------------------------------------------------------------

ExporterLog::SkeletalTemplateLogInfo::SkeletalTemplateLogInfo(const std::string &filename)
: ExporterLog::BaseLogInfo(filename,"SkeletalTemplate")
{
}


//-------------------------------------------------------------------

ExporterLog::SkeletalAnimationLogInfo::SkeletalAnimationLogInfo(const std::string &filename)
: ExporterLog::BaseLogInfo(filename,"SkeletalAnimation")
{
}


//-------------------------------------------------------------------

ExporterLog::SkeletalMeshGeneratorLogInfo::SkeletalMeshGeneratorLogInfo(const std::string &filename)
: ExporterLog::MeshLogInfo(filename)
{
	m_type = "SkeletalMeshGenerator";
}

//-------------------------------------------------------------------

ExporterLog::ComponentLogInfo::ComponentLogInfo(const std::string &filename)
: ExporterLog::BaseLogInfo(filename, "Component")
{
}


//-------------------------------------------------------------------

ExporterLog::LODLogInfo::LODLogInfo(const std::string &filename)
: ExporterLog::BaseLogInfo(filename, "LOD")
{
}

//-------------------------------------------------------------------

ExporterLog::TextureLogInfo::TextureLogInfo(const std::string &filename)
: ExporterLog::BaseLogInfo(filename, "Texture")
{
}

//-------------------------------------------------------------------

ExporterLog::BaseLogInfo::BaseLogInfo(const std::string &filename, const std::string &type)
: m_filename(filename),
  m_children(),
  m_type(type)
{
}

//-------------------------------------------------------------------

ExporterLog::BaseLogInfo::~BaseLogInfo()
{
	int c=static_cast<int>(m_children.size());
	int x;
	for (x=0; x<c; ++x)
	{
		delete m_children[static_cast<unsigned int>(x)];
		m_children[static_cast<unsigned int>(x)] = 0;
	}
}

//-------------------------------------------------------------------

void ExporterLog::BaseLogInfo::add(ExporterLog::BaseLogInfo *t)
{
	for(std::vector<BaseLogInfo *>::iterator it = m_children.begin(); it != m_children.end(); ++it)
	{
		//don't add duplicates (same node type with same filename)
		if ((*it)->m_filename == t->m_filename && (*it)->m_type == t->m_type)
		{
			delete t;
			BaseLogInfo * current = (*it);
			IGNORE_RETURN(m_children.erase(it));
			m_children.push_back(current);
			return;
		}
	}
	m_children.push_back(t);
}

//-------------------------------------------------------------------

/** install the system
  */
void ExporterLog::install (Messenger *newMessenger)
{
	messenger = newMessenger;
	ms_installed = true;
	reset ();

// JU_TODO: alienbrain def out
#if 0
	//clean out any custom assets stored in the importer
	AlienbrainImporter::reset();
#endif
// JU_TODO: endif alienbrain def out
}

//-------------------------------------------------------------------

/** Uninstall the system
  */
void ExporterLog::remove (void)
{
	reset();
	//since we new up another ms_objects in reset(), get rid of it
	delete ms_objects;
	ms_objects = NULL;
	ms_installed = false;
}

//-------------------------------------------------------------------
/**
 *
 * @param logFilename the local name of the logfile, since it must exist in ..\log from the source file
 * @pre setSourceFilename() must have been set (we need to know the type of export wanted to determine the
 * name of the log file)
 * @see setSourceFilename()
 */

bool ExporterLog::loadLogFile(const std::string& logFilename)
{
	//the log file should be in ..\log, so find it
	std::string fullLogFilename = ms_sourceFilename;
	std::string::size_type pos = fullLogFilename.find_last_of('\\');
	if (pos == static_cast<std::string::size_type>(std::string::npos))
	{
		//forward slashes, build rest of path using them
		pos = fullLogFilename.find_last_of('/');
		if (pos == static_cast<std::string::size_type>(std::string::npos))
		{
			//bad, can't find a '/' or a '\\'
			MESSENGER_LOG_WARNING(("WARNING, bad filename in loadLogFile, can't load previous export settings\n"));
			return false;
		}
		//erase the filename
		IGNORE_RETURN(fullLogFilename.erase(pos));
		pos = fullLogFilename.find_last_of('/');
		if (pos == static_cast<std::string::size_type>(std::string::npos))
		{
			//bad, there isn't another directory level
			MESSENGER_LOG_WARNING(("WARNING, can't move to ..\\log to look for log file\n"));
			return false;
		}
		//delete the \scenes directory
		IGNORE_RETURN(fullLogFilename.erase(pos));
		//add path to log file
		fullLogFilename += "/log/";
	}
	else
	{
		//success with back slashes, so build rest of path
		//erase to end
		IGNORE_RETURN(fullLogFilename.erase(pos));
		pos = fullLogFilename.find_last_of('\\');
		if (pos == static_cast<std::string::size_type>(std::string::npos))
		{
			//bad, there isn't another directory level
			MESSENGER_LOG_WARNING(("WARNING, malformed directory structure in loadLogFile, can't load previous settings\n"));
			return false;
		}
		//delete the \scenes directory
		IGNORE_RETURN(fullLogFilename.erase(pos));
		//add path to log file
		fullLogFilename += "\\log\\";
	}

	fullLogFilename += logFilename;

	FILE* file = fopen(fullLogFilename.c_str(), "r");
	if(!file)
	{
		//no log file, can occur if this is the first time the asset has been exported
		return false;
	}
	char buffer[8092];
	memset(buffer, 0, 8092);
	IGNORE_RETURN(fread(buffer, 8092, sizeof(char), file));

	std::string bufferString = buffer;

	//find and store the asset group name
	pos = 0;
	std::string::size_type endPos = 0;
	const std::string assetGroupConst = "asset_group = ";
	pos = bufferString.find(assetGroupConst);
	if(pos != static_cast<std::string::size_type>(std::string::npos))
	{
		pos += assetGroupConst.size();
		endPos = bufferString.find('\n', pos);
		if(endPos != static_cast<std::string::size_type>(std::string::npos))
		{
			ms_assetGroupName = bufferString.substr(pos, endPos-pos);
		}
	}

	//find and store the asset name
	pos = 0;
	endPos = 0;
	const std::string assetNameConst = "asset_name = ";
	pos = bufferString.find(assetNameConst);
	if(pos != static_cast<std::string::size_type>(std::string::npos))
	{
		pos += assetNameConst.size();
		endPos = bufferString.find('\n', pos);
		if(endPos != static_cast<std::string::size_type>(std::string::npos))
		{
			ms_assetName = bufferString.substr(pos, endPos-pos);
		}
	}

	//find and store the asset id
	pos = 0;
	endPos = 0;
	const std::string assetIdConst = "asset_id = ";
	pos = bufferString.find(assetIdConst);
	if(pos != static_cast<std::string::size_type>(std::string::npos))
	{
		pos += assetIdConst.size();
		endPos = bufferString.find('\n', pos);
		if(endPos != static_cast<std::string::size_type>(std::string::npos))
		{
			std::string assetIdStr = bufferString.substr(pos, endPos-pos);
			ms_assetId = atoi(assetIdStr.c_str());
		}
	}

	fclose(file);
	return true;
}

//-------------------------------------------------------------------

void ExporterLog::pushCurrentObject(const std::string &name)
{
	DEBUG_FATAL (ms_currentObjects.empty (), ("ms_currentObjects is empty"));
	NOT_NULL (ms_currentObjects.back ());
	BaseLogInfo * child = ms_currentObjects.back()->find(name);
//	NOT_NULL (child);
	ms_currentObjects.push_back(child);
}

//-------------------------------------------------------------------

void ExporterLog::popCurrentObject()
{
	DEBUG_FATAL (ms_currentObjects.empty (), ("ms_currentObjects is empty"));
	ms_currentObjects.pop_back();
}

//-------------------------------------------------------------------

/** Add a skeletal template
  */
void ExporterLog::addSkeletalTemplate(const std::string& newSKT)
{
	IGNORE_RETURN(ms_destClientFileContainer.insert(newSKT));
	ms_objects->addOne(new SkeletalTemplateLogInfo(newSKT));
}

//-------------------------------------------------------------------

/** Add a skeletal animation
  */
void ExporterLog::addSkeletalAnimation(const std::string& newANS)
{
	IGNORE_RETURN(ms_destClientFileContainer.insert(newANS));
	ms_objects->addOne(new SkeletalAnimationLogInfo(newANS));
}
//-------------------------------------------------------------------

/** Add a reference to a source shader
  */
void ExporterLog::addSourceShader (const std::string& newShaderName)
{
	ms_objects->add(new ShaderLogInfo(newShaderName, std::string()));
} 

//-------------------------------------------------------------------

/**
  * Add a reference to a source texture
	* currently, source files are submitted to Alienbrain by the artists
  */
void ExporterLog::addSourceTexture (const std::string& newTextureName)
{
	std::vector<ShaderLogInfo *> shaderList;
	ms_objects->filter(shaderList);
	DEBUG_FATAL (shaderList.size () == 0, ("ExporterLog::addSourceTexture - no shader set"));
	ShaderLogInfo *ds = shaderList [shaderList.size () - 1];

	//don't store duplicates
	for(int numSht = 0; numSht < static_cast<int>(shaderList.size()); ++numSht)
		for(int numTex = 0; numTex < static_cast<int>(shaderList[static_cast<unsigned int>(numSht)]->m_textureFilenameList.size()); ++numTex)
			if(shaderList[static_cast<unsigned int>(numSht)]->m_textureFilenameList[static_cast<unsigned int>(numTex)] == newTextureName)
				return;

	ds->m_textureFilenameList.push_back(newTextureName);

	IGNORE_RETURN(ms_sourceFileContainer.insert(newTextureName));
	ms_sourceTexturesContainer.push_back(newTextureName);
}

//-------------------------------------------------------------------

/** Return a reference to the source textures
 */
const std::vector<std::string>& ExporterLog::getSourceTexturesFilenames()
{
	return ms_sourceTexturesContainer;
}

//-------------------------------------------------------------------

/** Add a shader to the current mesh.
  * Requires that a current mesh has been set via setms_currentMesh().
  */
void ExporterLog::addDestinationShaderToCurrentMesh (const std::string& newShaderName, const std::string& newEffectFilename)
{
	ShaderLogInfo *ds = new ShaderLogInfo(newShaderName, newEffectFilename);
	IGNORE_RETURN(ms_destClientFileContainer.insert(newShaderName));
	ms_currentObjects.back()->add(ds);
}

//-------------------------------------------------------------------

/** Add a texture to the current mesh.
  * Requires that a current mesh has been set via setms_currentMesh().
  */
void ExporterLog::addDestinationTextureToCurrentShader (const std::string& newTextureName)
{
	std::vector<ShaderLogInfo *> shaderList;
	ms_objects->filter(shaderList);

	ShaderLogInfo * currentShader = shaderList.back();
	NOT_NULL(currentShader);

	//build the complete file path and store the file to submit to perforce
	std::string filename = ms_baseDir;
	filename += newTextureName;
	IGNORE_RETURN(ms_destClientFileContainer.insert(filename.c_str()));

	//attach it to the shader, don't store duplicates
	for(int numTex = 0; numTex < static_cast<int>(currentShader->m_textureFilenameList.size()); ++numTex)
		if(currentShader->m_textureFilenameList[static_cast<unsigned int>(numTex)] == newTextureName)
			return;

	currentShader->m_textureFilenameList.push_back(newTextureName);
}

//-------------------------------------------------------------------

/** Add a hardpoint to the current mesh.
  */
void ExporterLog::addHardpointToCurrentMesh (const std::string& newName, const Transform& newTransform)
{
	MeshLogInfo * currentMesh = dynamic_cast<MeshLogInfo *>(ms_currentObjects.back());
	NOT_NULL(currentMesh);
	Hardpoint* hp = new Hardpoint (newName.c_str(), newTransform);
	currentMesh->m_hardpointList.push_back(hp);
}//lint !e429 custodial pointer not freed (okay, since we store it on in our own data struct)

//-------------------------------------------------------------------

/** Set the extent information.
  */
void ExporterLog::setExtentForCurrentMesh(const Vector& center, real radius, const Vector& boxMin, const Vector& boxMax)
{
	MeshLogInfo * currentMesh = dynamic_cast<MeshLogInfo *>(ms_currentObjects.back());
	NOT_NULL(currentMesh);
	currentMesh->m_sphereCenter = center;
	currentMesh->m_sphereRadius = radius;
	currentMesh->m_boxMin = boxMin;
	currentMesh->m_boxMax = boxMax;
}

//-------------------------------------------------------------------

/** Set the number of UVs for the mesh, if the current object is a mesh
 */
void ExporterLog::setUVSetsForCurrentMesh(int numUVSets)
{
	MeshLogInfo * currentMesh = dynamic_cast<MeshLogInfo *>(ms_currentObjects.back());
	if(currentMesh)
	{
		currentMesh->m_numUVSets = numUVSets;
	}
}

//-------------------------------------------------------------------

/** Empty out all our data structures.
  */
void ExporterLog::reset (void)
{
	if (!ms_installed)
		return;

	//clear member vars
	setAuthor ("");
	setSourceFilename ("");
	setSourceNumberOfVertices (0);
	setSourceNumberOfPolygons (0);
	setSourceNumberOfUVSets (0);
	setMayaCommand("");
	setMayaExportOptions("");
	setBaseDir("");
	ms_logFilename = "";
	ms_outfile = NULL;
	ms_assetGroupName = "";
	ms_assetName = "";

	//clear containers
	ms_destClientFileContainer.clear();
	ms_destSharedFileContainer.clear();
	ms_destServerFileContainer.clear();
	ms_sourceFileContainer.clear();
	ms_sourceTexturesContainer.clear();
	ms_currentObjects.clear();

	//recreate the root node, add to now empty container
	delete ms_objects;
	const std::string stringRoot = "Root";
	const std::string stringBase = "Base";
	ms_objects = new BaseLogInfo(stringRoot, stringBase);
	ms_currentObjects.push_back(ms_objects);
}

//-------------------------------------------------------------------


/**
 * Write the log information to the given filename for a static mesh.
 * This format is in an Alienbrain-friendly format.
 */
bool ExporterLog::writeStaticMesh(const std::string& filename, bool interactive)
{
	if (!ms_installed)
		return false;

	ms_logFilename = filename;

	ms_outfile = fopen (filename.c_str(), "wt");
	if (!ms_outfile)
	{
		std::string errorMsg = "Could not write log file ";
		errorMsg            += filename + "\n";
		if(interactive)
			MESSENGER_MESSAGE_BOX(NULL, errorMsg.c_str(), "Error!", MB_OK | MB_ICONERROR);
		MESSENGER_REJECT(!ms_outfile, (errorMsg.c_str()));
	}

	writeSharedData(filename);

	char startOfLogBase[256];
	strcpy(startOfLogBase, filename.c_str());
	char *endOfLogBase = strstr(startOfLogBase, "log\\");
	NOT_NULL(endOfLogBase);
	*endOfLogBase = '\0';

	std::vector<ShaderLogInfo *> shaderList;
	ms_objects->filter(shaderList);

	//log source information
	fprintf(ms_outfile, "object\n");
	fprintf(ms_outfile, "{\n");
	fprintf(ms_outfile, "	name = %s\n", ms_sourceFilename.c_str());
	fprintf(ms_outfile, "	type = source\n");
	fprintf(ms_outfile, "\n");
	fprintf(ms_outfile, "	author = %s\n", ms_author.c_str());
	fprintf(ms_outfile, "	number_of_vertices = %i\n", ms_sourceNumberOfVertices);
	fprintf(ms_outfile, "	number_of_polygons = %i\n", ms_sourceNumberOfPolygons);
	fprintf(ms_outfile, "	number_of_uvsets = %i\n", ms_sourceNumberOfUVSets);
	fprintf(ms_outfile, "	number_of_shaders = %i\n", shaderList.size());

	//log information for each shader on this source object
	for (int a = 0; a < static_cast<int>(shaderList.size()); ++a)
	{
		fprintf(ms_outfile, "	object\n");
		fprintf(ms_outfile, "	{\n");
		fprintf(ms_outfile, "		name = %s\n", shaderList[static_cast<unsigned int>(a)]->m_filename.c_str());
		fprintf(ms_outfile, "		type = shader\n");
		fprintf(ms_outfile, "		proplist = OBJ_number_of_textures\n");
		fprintf(ms_outfile, "		number_of_textures = %i\n",  shaderList[static_cast<unsigned int>(a)]->m_textureFilenameList.size());

		//log information for each texture on this shader
		for (int b = 0; b < static_cast<int>(shaderList[static_cast<unsigned int>(a)]->m_textureFilenameList.size()); ++b)
		{
			fprintf(ms_outfile, "		object\n");
			fprintf(ms_outfile, "		{\n");
			fprintf(ms_outfile, "			name = %s\n", shaderList[static_cast<unsigned int>(a)]->m_textureFilenameList[static_cast<unsigned int>(b)].c_str());
			fprintf(ms_outfile, "			type = texture\n");
			fprintf(ms_outfile, "		}\n");
		}
		fprintf(ms_outfile, "	}\n");
	}
	fprintf(ms_outfile, "}\n");
	fprintf(ms_outfile, "\n");

	std::vector<LODLogInfo  *> lodList;
	std::vector<ComponentLogInfo *> componentList;
	std::vector<MeshLogInfo *> meshList;

	ms_objects->filter(lodList);
	ms_objects->filter(componentList);
	ms_objects->filter(meshList);

	for(int lodNum = 0; lodNum < static_cast<int>(lodList.size()); ++lodNum)
		writeLOD(0, lodList[static_cast<unsigned int>(lodNum)]);
	
	for(int cmpNum = 0; cmpNum < static_cast<int>(componentList.size()); ++cmpNum)
		writeComponent(0, componentList[static_cast<unsigned int>(cmpNum)]);

	for(int meshNum = 0; meshNum < static_cast<int>(meshList.size()); ++meshNum)
		writeMesh(0, meshList[static_cast<unsigned int>(meshNum)]);

	fprintf(ms_outfile, "\nClient Files:\n{\n");
	std::set<std::string>::iterator i;
	for(i = ms_destClientFileContainer.begin(); i != ms_destClientFileContainer.end(); ++i)
	{
		std::string::size_type pos = i->find(ms_baseDir);
		std::string finalFilename = *i;
		if(pos != std::string::npos)
			finalFilename = i->substr(pos + ms_baseDir.length());
		fprintf(ms_outfile, "	%s\n", finalFilename);
	}
	fprintf(ms_outfile, "}\n");

	fprintf(ms_outfile, "\nShared Files:\n{\n");
	for(i = ms_destSharedFileContainer.begin(); i != ms_destSharedFileContainer.end(); ++i)
	{
		std::string::size_type pos = i->find(ms_baseDir);
		std::string finalFilename = *i;
		if(pos != std::string::npos)
			finalFilename = i->substr(pos + ms_baseDir.length());
		fprintf(ms_outfile, "	%s\n", finalFilename);
	}
	fprintf(ms_outfile, "}\n\n");

	fprintf(ms_outfile, "\nServer Files:\n{\n");
	for(i = ms_destServerFileContainer.begin(); i != ms_destServerFileContainer.end(); ++i)
	{
		std::string::size_type pos = i->find(ms_baseDir);
		std::string finalFilename = *i;
		if(pos != std::string::npos)
			finalFilename = i->substr(pos + ms_baseDir.length());
		fprintf(ms_outfile, "	%s\n", finalFilename);
	}
	fprintf(ms_outfile, "}\n\n");

	fclose (ms_outfile);
	MESSENGER_LOG(("logfile written successfully to %s\n", ms_logFilename.c_str()));
	return true;
}


//-------------------------------------------------------------------

/**
 * Write the log information to the given filename for a skeletal template.
 * This format is in an Alienbrain-friendly format.
 */
bool ExporterLog::writeSkeletalTemplate(const std::string& filename, bool interactive)
{
	if (!ms_installed)
		return false;

	ms_logFilename = filename;

	ms_outfile = fopen (filename.c_str(), "wt");
	if (!ms_outfile)
	{
		std::string errorMsg = "Could not write log file ";
		errorMsg            += filename + "\n";
		if(interactive)
			MESSENGER_MESSAGE_BOX(NULL, errorMsg.c_str(), "Error!", MB_OK | MB_ICONERROR);
		MESSENGER_REJECT(!ms_outfile, (errorMsg.c_str()));
	}

	writeSharedData(filename);

	fclose (ms_outfile);
	MESSENGER_LOG(("logfile written successfully to %s\n", ms_logFilename.c_str()));
	return true;
}

//-------------------------------------------------------------------

/**
 * Write the log information to the given filename for a skeletal animation.
 * This format is in an Alienbrain-friendly format.
 */
bool ExporterLog::writeSkeletalAnimation(const std::string& filename, bool interactive)
{
	if (!ms_installed)
		return false;

	ms_logFilename = filename;

	ms_outfile = fopen (filename.c_str(), "wt");
	if (!ms_outfile)
	{
		std::string errorMsg = "Could not write log file ";
		errorMsg            += filename + "\n";
		if(interactive)
			MESSENGER_MESSAGE_BOX(NULL, errorMsg.c_str(), "Error!", MB_OK | MB_ICONERROR);
		MESSENGER_REJECT(!ms_outfile, (errorMsg.c_str()));
	}

	writeSharedData(filename);

	fclose (ms_outfile);
	MESSENGER_LOG(("logfile written successfully to %s\n", ms_logFilename.c_str()));
	return true;
}

//-------------------------------------------------------------------

/**
 * Write the log information to the given filename for a mesh generator.
 * This format is in an Alienbrain-friendly format.
 */
bool ExporterLog::writeSkeletalMeshGenerator(const std::string& filename, bool interactive)
{
	if (!ms_installed)
		return false;

	ms_logFilename = filename;

	ms_outfile = fopen (filename.c_str(), "wt");
	if (!ms_outfile)
	{
		std::string errorMsg = "Could not write log file ";
		errorMsg            += filename + "\n";
		if(interactive)
			MESSENGER_MESSAGE_BOX(NULL, errorMsg.c_str(), "Error!", MB_OK | MB_ICONERROR);
		MESSENGER_REJECT(!ms_outfile, (errorMsg.c_str()));
	}
	writeSharedData(filename);

	fclose (ms_outfile);
	MESSENGER_LOG(("logfile written successfully to %s\n", ms_logFilename.c_str()));
	return true;
}

//-------------------------------------------------------------------

/**
 * Write the log information to the given filename for a sat file.
 * This format is in an Alienbrain-friendly format.
 */
bool ExporterLog::writeSkeletalAppearanceTemplate(const std::string& filename, bool interactive)
{
	if (!ms_installed)
		return false;

	ms_logFilename = filename;

	ms_outfile = fopen (filename.c_str(), "wt");
	if (!ms_outfile)
	{
		std::string errorMsg = "Could not write log file ";
		errorMsg            += filename + "\n";
		if(interactive)
			MESSENGER_MESSAGE_BOX(NULL, errorMsg.c_str(), "Error!", MB_OK | MB_ICONERROR);
		MESSENGER_REJECT(!ms_outfile, (errorMsg.c_str()));
	}
	writeSharedData(filename);

	fclose (ms_outfile);
	MESSENGER_LOG(("logfile written successfully to %s\n", ms_logFilename.c_str()));
	return true;
}

//-------------------------------------------------------------------

/** 
 * Write all the shared data to disk
 */
void ExporterLog::writeSharedData(const std::string& filename)
{
	NOT_NULL(ms_outfile);
	//write custom data information
	fprintf(ms_outfile, "object\n");
	fprintf(ms_outfile, "{\n");
	char logFileVersionString[20];
	IGNORE_RETURN(_itoa(ms_versionNumber, logFileVersionString, 10));
	fprintf(ms_outfile, "	logfile_version = %s\n", logFileVersionString);
	fprintf(ms_outfile, "	mayaExporter_version = %s\n", ms_mayaExporterVersion.c_str());
	fprintf(ms_outfile, "	maya_filename = %s\n", ms_sourceFilename.c_str());
	fprintf(ms_outfile, "	maya_command = %s\n", ms_mayaCommand.c_str());
	fprintf(ms_outfile, "	maya_export_options = %s\n", ms_exportOptions.c_str());

	//fill in something for asset_group if we don't have anything (most likely because 
	//this file has never been exported to Alienbrain, and no asset group has ever been assigned)
	if (ms_assetGroupName == "")
		ms_assetGroupName = "<none assigned>";
	if (ms_assetName == "")
		ms_assetName = "<none assigned>";

	fprintf(ms_outfile, "	asset_group = %s\n", ms_assetGroupName.c_str());
	fprintf(ms_outfile, "	asset_name = %s\n", ms_assetName.c_str());
	fprintf(ms_outfile, "	asset_id = %d\n", ms_assetId);
	fprintf(ms_outfile, "}\n");
	fprintf(ms_outfile, "\n");

	//log base information
	fprintf(ms_outfile, "object\n");
	fprintf(ms_outfile, "{\n");
	
	const char *startOfLogFile = strstr(filename.c_str(), "log\\");

	char startOfLogBase[256];
	strcpy(startOfLogBase, filename.c_str());
	char *endOfLogBase = strstr(startOfLogBase, "log\\");
	NOT_NULL(endOfLogBase);
	*endOfLogBase = '\0';

	fprintf(ms_outfile, "	name = %s\n", startOfLogFile);
	fprintf(ms_outfile, "	type = log\n");
	fprintf(ms_outfile, "	base = %s\n", startOfLogBase);
	fprintf(ms_outfile, "}\n");
	fprintf(ms_outfile, "\n");
}

//-------------------------------------------------------------------

/** Write the current LOD to the file
  * Assumes that the ms_outfile is already opened (via write).
  * @param indentLevel number of tabs to indent the current block (useful since we recurse)
  * @param lod the current lod to write out
  * @param isBase is this LOD the base game object?  If so, make sure to write the additional path information
  * @param base the base path information only useful if isBase is true
  */
void ExporterLog::writeLOD(int indentLevel, const LODLogInfo* lod, bool isBase, const char* base)
{
	NOT_NULL(ms_outfile);
	NOT_NULL(lod);
	int i;
	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "object\n");

	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "{\n");

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	std::string fullpath = ms_baseDir + ms_lodDirectory;
	char name [256];
	const char* ptr = strstr(fullpath.c_str(), "appearance");
	NOT_NULL(ptr);
	strcpy(name, ptr);
	strcat(name, lod->m_filename.c_str());
	strcat(name, ".lod");
	fprintf(ms_outfile, "name = %s\n", name);
	
	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	
	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "type = lod\n");
	
	//output the base information if needed
	if(isBase)
	{
		for(i = 0; i < indentLevel+1; ++i)
			fprintf(ms_outfile, "	");
		fprintf(ms_outfile, "base = %s\n", base);
	}

	std::vector<LODLogInfo  *> lodList;
	std::vector<ComponentLogInfo *> componentList;
	std::vector<MeshLogInfo *> meshList;

	lod->filter(lodList);
	lod->filter(componentList);
	lod->filter(meshList);

	for(int lodNum = 0; lodNum < static_cast<int>(lodList.size()); ++lodNum)
		writeLOD(indentLevel+1, lodList[static_cast<unsigned int>(lodNum)]);
	
	for(int cmpNum = 0; cmpNum < static_cast<int>(componentList.size()); ++cmpNum)
		writeComponent(indentLevel+1, componentList[static_cast<unsigned int>(cmpNum)]);

	for(int meshNum = 0; meshNum < static_cast<int>(meshList.size()); ++meshNum)
		writeMesh(indentLevel+1, meshList[static_cast<unsigned int>(meshNum)]);

	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "}\n");
}

//-------------------------------------------------------------------

/** Write the current component to the file
  * Assumes that the ms_outfile is already opened (via write).
  * @param indentLevel number of tabs to indent the current block (useful since we recurse)
  * @param component the current component to write out
  * @param isBase is this LOD the base game object?  If so, make sure to write the additional path information
  * @param base the base path information only useful if isBase is true
  */
void ExporterLog::writeComponent(int indentLevel, const ComponentLogInfo* component, bool isBase, const char* base)
{
	NOT_NULL(ms_outfile);
	NOT_NULL(component);
	int i;
	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "object\n");
	
	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "{\n");
	
	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	std::string fullpath = ms_baseDir + ms_componentDirectory;
	char name [256];
	const char* ptr = strstr(fullpath.c_str(), "appearance");
	NOT_NULL(ptr);
	strcpy(name, ptr);
	strcat(name, component->m_filename.c_str());
	strcat(name, ".cmp");
	fprintf(ms_outfile, "name = %s\n", name);

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "type = component\n");

	//output the base information if needed
	if(isBase)
	{
		for(i = 0; i < indentLevel+1; ++i)
			fprintf(ms_outfile, "	");
		fprintf(ms_outfile, "base = %s\n", base);
	}

	std::vector<LODLogInfo  *> lodList;
	std::vector<ComponentLogInfo *> componentList;
	std::vector<MeshLogInfo *> meshList;

	component->filter(lodList);
	component->filter(componentList);
	component->filter(meshList);

	for(int lodNum = 0; lodNum < static_cast<int>(lodList.size()); ++lodNum)
		writeLOD(indentLevel+1, lodList[static_cast<unsigned int>(lodNum)]);
	
	for(int cmpNum = 0; cmpNum < static_cast<int>(componentList.size()); ++cmpNum)
		writeComponent(indentLevel+1, componentList[static_cast<unsigned int>(cmpNum)]);

	for(int meshNum = 0; meshNum < static_cast<int>(meshList.size()); ++meshNum)
		writeMesh(indentLevel+1, meshList[static_cast<unsigned int>(meshNum)]);

	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "}\n");
}

//-------------------------------------------------------------------

/** Write the current mesh to the file
  * Assumes that the ms_outfile is already opened (via write).
  * @param indentLevel number of tabs to indent the current block (useful since we recurse)
  * @param mesh the current mesh to write out
  * @param isBase is this LOD the base game object?  If so, make sure to write the additional path information
  * @param base the base path information only useful if isBase is true
  */
void ExporterLog::writeMesh(int indentLevel, MeshLogInfo* mesh, bool isBase, const char* base)
{
	NOT_NULL(ms_outfile);
	NOT_NULL(mesh);

	int i;
	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "object\n");

	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "{\n");

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	char name[256];
	std::string fullpath = ms_baseDir + ms_meshDirectory;
	const char* ptr = strstr(fullpath.c_str(), "appearance");
	NOT_NULL(ptr);
	strcpy(name, ptr);
	strcat(name, mesh->m_filename.c_str());
	strcat(name, ".msh");
	fprintf(ms_outfile, "name = %s\n", name);
	
	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "type = mesh\n");

	//output the base information if needed
	if(isBase)
	{
		for(i = 0; i < indentLevel+1; ++i)
			fprintf(ms_outfile, "	");
		fprintf(ms_outfile, "base = %s\n", base);
	}

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "proplist = OBJ_number_of_vertices,OBJ_number_of_polygons\n");
	
	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "number_of_vertices = %i\n", mesh->m_numberOfVertices);
	
	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "number_of_polygons = %i\n", mesh->m_numberOfPolygons);
	
	std::vector<ShaderLogInfo *> shaderList;
	mesh->filter(shaderList);

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "number_of_shaders = %i\n", shaderList.size());

	for(int numSht = 0; numSht < static_cast<int>(shaderList.size()); ++numSht)
		writeShader(indentLevel+1, shaderList[static_cast<unsigned int>(numSht)]);

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf (ms_outfile, "extent_center  = %1.3f %1.3f %1.3f\n", mesh->m_sphereCenter.x, mesh->m_sphereCenter.y, mesh->m_sphereCenter.z);
	
	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf (ms_outfile, "extent_radius  = %1.3f\n", mesh->m_sphereRadius);
	
	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf (ms_outfile, "extent_box_min = %1.3f %1.3f %1.3f\n", mesh->m_boxMin.x, mesh->m_boxMin.y, mesh->m_boxMin.z);
	
	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf (ms_outfile, "extent_box_max = %1.3f %1.3f %1.3f\n", mesh->m_boxMax.x, mesh->m_boxMax.y, mesh->m_boxMax.z);
	
	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf (ms_outfile, "number_of_hardpoints = %i\n", mesh->m_hardpointList.size());

	for(int numHP = 0; numHP < static_cast<int>(mesh->m_hardpointList.size()); ++numHP)
		writeHardpoint(indentLevel+1, mesh->m_hardpointList[static_cast<unsigned int>(numHP)]);

	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "}\n");
}

//-------------------------------------------------------------------

/** Write the current shader to the file
  * Assumes that the ms_outfile is already opened (via write).
  * @param indentLevel number of tabs to indent the current block (useful since we recurse)
  * @param shader the current shader to write out
  */
void ExporterLog::writeShader(int indentLevel, ShaderLogInfo* shader)
{
	NOT_NULL(ms_outfile);
	NOT_NULL(shader);

	int i;
	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "object\n");

	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "{\n");

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");

	char name[256];
	const char* ptr = strstr(shader->m_filename.c_str(), "shader");
	NOT_NULL(ptr);
	strcpy(name, ptr);
	fprintf(ms_outfile, "name = %s\n", name);

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "type = shader\n");

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "proplist = OBJ_number_of_textures\n");

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "effect_name = %s\n", shader->m_effectFilename.c_str());

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "number_of_textures = %i\n", shader->m_textureFilenameList.size());

	for(int numTex = 0; numTex < static_cast<int>(shader->m_textureFilenameList.size()); ++numTex)
		writeTexture(indentLevel+1, shader->m_textureFilenameList[static_cast<unsigned int>(numTex)]);

	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "}\n");
}

//-------------------------------------------------------------------

/** Write the current hardpoint to the file
  * Assumes that the ms_outfile is already opened (via write).
  * @param indentLevel number of tabs to indent the current block (useful since we recurse)
  * @param hardpoint the current hardpoint to write out
  */
void ExporterLog::writeHardpoint(int indentLevel, const Hardpoint* hardpoint)
{
	NOT_NULL(ms_outfile);
	NOT_NULL(hardpoint);

	int i;
	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "object\n");

	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "{\n");

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "name = %s\n", hardpoint->getName());

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "type = hardpoint\n");

	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "}\n");
}

//-------------------------------------------------------------------

/** Write the current texture information to the file
  * Assumes that the ms_outfile is already opened (via write).
  * @param indentLevel number of tabs to indent the current block (useful since we recurse)
  * @param textureFilename the current texture to write out
  */
void ExporterLog::writeTexture(int indentLevel, const std::string& textureFilename)
{
	NOT_NULL(ms_outfile);
	int i;
	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "object\n");

	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "{\n");

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "name = %s\n", textureFilename.c_str());

	for(i = 0; i < indentLevel+1; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "type = texture\n");

	for(i = 0; i < indentLevel; ++i)
		fprintf(ms_outfile, "	");
	fprintf(ms_outfile, "}\n");
}

//-------------------------------------------------------------------

/** Build the log information from the given node
  */
void ExporterLog::buildStaticMesh(const MayaHierarchy::Node* root)
{
	NOT_NULL(root);
	switch(root->getType())
	{
		// ----------
		// These node types have custom log info

		case MayaHierarchy::T_msh:			buildMeshLogInfo(root);						break;

		// ----------
		// These node types have generic log info

		case MayaHierarchy::T_cmp:          buildGenericLogInfo(root,"Component");      break;
		case MayaHierarchy::T_lod:          buildGenericLogInfo(root,"LOD");            break;
		case MayaHierarchy::T_pob:          buildGenericLogInfo(root,"PortalObject");   break;
		case MayaHierarchy::T_cel:          buildGenericLogInfo(root,"Cell");           break;
		case MayaHierarchy::T_collision:    buildGenericLogInfo(root,"Collision");      break;
		case MayaHierarchy::T_floor:        buildGenericLogInfo(root,"Floor");          break;
		case MayaHierarchy::T_extent:       buildGenericLogInfo(root,"Extent");         break;
		case MayaHierarchy::T_cylinder:     buildGenericLogInfo(root,"Cylinder");       break;
		case MayaHierarchy::T_sphere:       buildGenericLogInfo(root,"Sphere");         break;
		case MayaHierarchy::T_box:          buildGenericLogInfo(root,"Box");            break;
		case MayaHierarchy::T_cmesh:        buildGenericLogInfo(root,"CollisionMesh");  break;

		// ----------
		// These node types have no log info

		case MayaHierarchy::T_uninitialized:
		case MayaHierarchy::T_ext:
		case MayaHierarchy::T_prt:
		case MayaHierarchy::T_pls:
		case MayaHierarchy::T_COUNT:
		default:
			break;
	}
}

//-------------------------------------------------------------------

void ExporterLog::buildSkeletalTemplate(const MayaHierarchy::Node* root)
{
	UNREF(root);
}

//-------------------------------------------------------------------
void ExporterLog::buildSkeletalAnimation(const MayaHierarchy::Node* root)
{
	UNREF(root);
}

//-------------------------------------------------------------------

void ExporterLog::buildSkeletalMeshGenerator(const std::string &meshName, int vertexCount, int polygonCount)
{
	MStatus status;

	SkeletalMeshGeneratorLogInfo *dmgn = new SkeletalMeshGeneratorLogInfo(meshName);

	dmgn->m_numberOfVertices = vertexCount;
	dmgn->m_numberOfPolygons = polygonCount;

	ms_currentObjects.back()->add(dmgn);
	ms_currentObjects.push_back(dmgn);
}//lint !e429 custodial pointer not freed (okay, since we store it on in our own data struct)

//-------------------------------------------------------------------

/** Build the log information from the given mesh
  * the node can have either an LOD or a component as a parent, if one is present, it is set as a child
  */
void ExporterLog::buildMeshLogInfo(const MayaHierarchy::Node* root)
{
	NOT_NULL(root);
	MStatus status;
	MFnMesh fnMesh(root->getMayaDagPath(), &status);
	if(!status)
		return;

	MeshLogInfo *dm = new MeshLogInfo(root->getName());

	dm->m_numberOfVertices = fnMesh.numVertices();
	dm->m_numberOfPolygons = fnMesh.numPolygons();

	ms_currentObjects.back()->add(dm);
	ms_currentObjects.push_back(dm);

	processChildObjects(root);

	ms_currentObjects.pop_back();
}//lint !e429 custodial pointer not freed (okay, since we store it on in our own data struct)

//-------------------------------------------------------------------

void ExporterLog::buildGenericLogInfo(const MayaHierarchy::Node* root, const char * type)
{
	NOT_NULL(root);
	const std::string stringRootName = root->getName();
	const std::string stringType = type;
	BaseLogInfo *dc = new BaseLogInfo(stringRootName, stringType);

	ms_currentObjects.back()->add(dc);
	ms_currentObjects.push_back(dc);

	processChildObjects(root);

	ms_currentObjects.pop_back();
}

//-------------------------------------------------------------------

void ExporterLog::processChildObjects(const MayaHierarchy::Node* root)
{
	const MayaHierarchy::Node* currentNode = NULL;
	for (int i = 0; i < root->getNumberOfChildren(); ++i)
	{
		currentNode = root->getChild(i);
		switch(currentNode->getType())
		{
			// ----------
			// These node types have custom log info

			case MayaHierarchy::T_msh:			buildMeshLogInfo(currentNode);						break;

			// ----------
			// These node types have generic log info

            case MayaHierarchy::T_cmp:          buildGenericLogInfo(currentNode,"Component");       break;
            case MayaHierarchy::T_cel:          buildGenericLogInfo(currentNode,"Cell");            break;
            case MayaHierarchy::T_pls:          buildGenericLogInfo(currentNode,"PortalList");      break;
            case MayaHierarchy::T_lightList:    buildGenericLogInfo(currentNode,"LightList");       break;
            case MayaHierarchy::T_light:        buildGenericLogInfo(currentNode,"Light");           break;
            case MayaHierarchy::T_lod:          buildGenericLogInfo(currentNode,"LOD");             break;
            case MayaHierarchy::T_collision:    buildGenericLogInfo(currentNode,"Collision");       break;
            case MayaHierarchy::T_floor:        buildGenericLogInfo(currentNode,"Floor");           break;
            case MayaHierarchy::T_extent:       buildGenericLogInfo(currentNode,"Extent");          break;
            case MayaHierarchy::T_cylinder:     buildGenericLogInfo(currentNode,"Cylinder");        break;
            case MayaHierarchy::T_sphere:       buildGenericLogInfo(currentNode,"Sphere");          break;
            case MayaHierarchy::T_box:          buildGenericLogInfo(currentNode,"Box");             break;
            case MayaHierarchy::T_cmesh:        buildGenericLogInfo(currentNode,"CollisionMesh");   break;

			// ----------
			// These node types have no log info

			case MayaHierarchy::T_uninitialized:
			case MayaHierarchy::T_ext:
			case MayaHierarchy::T_prt:
			case MayaHierarchy::T_pob:
			case MayaHierarchy::T_COUNT:
			default:
				break;
		}
	}
}

//-------------------------------------------------------------------
/** Get the extent sphere center for the current mesh or spirte
  * Requires that a current mesh has previously been set
  */
const Vector& ExporterLog::getCurrentSphereCenter (void)
{
	BaseLogInfo * o = ms_currentObjects.back();
	{
		MeshLogInfo * m = dynamic_cast<MeshLogInfo *>(o);
		if (m)
			return m->m_sphereCenter;
	}
	FATAL(true, ("no current value to get sphere center from"));
	//we can never get here (but all paths *must* return a value)
	return *(new Vector);   //lint !e527 unreachable
}

//-------------------------------------------------------------------

/** Get the extent sphere radius for the current mesh or spirte
  * Requires that a current mesh has previously been set
  */
real ExporterLog::getCurrentSphereRadius (void)
{
	BaseLogInfo * o = ms_currentObjects.back();
	{
		MeshLogInfo * m = dynamic_cast<MeshLogInfo *>(o);
		if (m)
			return m->m_sphereRadius;
	}
	FATAL(true, ("no current value to get sphere radius from"));
	//we can never get here
	return 0;               //lint !e527 unreachable
}

//-------------------------------------------------------------------

