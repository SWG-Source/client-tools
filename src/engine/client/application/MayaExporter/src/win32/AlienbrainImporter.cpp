 // ======================================================================
//
// AlienbrainImporter.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"

// JU_TODO: alienbrain def out
#if 0

#include "AlienbrainImporter.h"

#include <map>
#include <list>
#include <string>

#include "maya/MFileIO.h"

#include "AlienbrainConnection.h"
#include "ExporterLog.h"
#include "Messenger.h"
#include "PluginMain.h"
#include "SetDirectoryCommand.h"

#pragma warning (disable: 4505)

//////////////////////////////////////////////////////////////////////////////////

Messenger*                                            AlienbrainImporter::messenger;
std::string                                           AlienbrainImporter::ms_currentPath;
std::string                                           AlienbrainImporter::ms_perforceClientPath;
std::string                                           AlienbrainImporter::ms_perforceSharedPath;
std::string                                           AlienbrainImporter::ms_perforceServerPath;
std::string                                           AlienbrainImporter::ms_assetDBCategory;
bool                                                  AlienbrainImporter::ms_installed;
bool                                                  AlienbrainImporter::ms_connected;
bool                                                  AlienbrainImporter::ms_multiExporting;

const std::string                                     AlienbrainImporter::ms_logImportPath = "log";
const std::string                                     AlienbrainImporter::ms_sourceImportPath = "scenes";
const std::string                                     AlienbrainImporter::ms_sourceTextureImportPath = "sourceimages";
const std::string                                     AlienbrainImporter::ms_shaderImportPath = "exported\\appearance\\shader";


//////////////////////////////////////////////////////////////////////////////////

/**
 * Install the module.
 */
void AlienbrainImporter::install(Messenger* newMessenger)
{
	messenger = newMessenger;
	ms_installed = true;
	ms_connected = false;
	ms_multiExporting = false;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Remove the module, closing down any data structures.
 */
void AlienbrainImporter::remove()
{
	if(ms_connected)
		IGNORE_RETURN(disconnectFromServer());
	ms_multiExporting = false;
	ms_installed = false;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Remove the module, closing down any data structures.
 */
void AlienbrainImporter::startMultiExport()
{
	if(!ms_connected)
		IGNORE_RETURN(connectToServer());
	ms_multiExporting = true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Remove the module, closing down any data structures.
 */
void AlienbrainImporter::endMultiExport()
{
	ms_multiExporting = false;
	if(ms_connected)
		IGNORE_RETURN(disconnectFromServer());
}

/////////////////////////////////////////////////////////////////////////////////

/**
 * This function is used by the PerforceImporter to determine where in the Perforce depot
 * to submit the final game files.  This information is stored in the Alienbrain depot in the
 * "PerforceClientDir" property that is assigned at base directory levels, and trickes down to all the
 * source files.  We test the mb file, since it will always exist.
 */
void AlienbrainImporter::storeFileProperties()
{
	FATAL(!ms_installed, ("AlienbrainImporter not installed\n"));
	FATAL(!ms_connected, ("AlienbrainImporter connected to server\n"));
	std::string dir = ms_currentPath;
	dir += ms_sourceImportPath;
	std::string file = ExporterLog::getSourceFilename();
	CNxNPath path = AlienbrainConnection::makeNamespacePath(file.c_str(), dir.c_str());

	//try to get PerforceClientDir, if that doesn't exist then look for the old one (PerforceDir)
	CNxNString perforceClientPath = AlienbrainConnection::getProperty(path, "PerforceClientDir");
	if(!perforceClientPath.IsEmpty())
		ms_perforceClientPath = static_cast<LPCSTR>(perforceClientPath);
	else
	{
		perforceClientPath = AlienbrainConnection::getProperty(path, "PerforceDir");
		if(!perforceClientPath.IsEmpty())
			ms_perforceClientPath = static_cast<LPCSTR>(perforceClientPath);
	}

	// ----------

	ms_perforceSharedPath = static_cast<LPCSTR>(AlienbrainConnection::getProperty(path, "PerforceSharedDir"));

	if(ms_perforceSharedPath.empty())
		ms_perforceSharedPath = ms_perforceClientPath;

	// ----------

	ms_perforceServerPath = static_cast<LPCSTR>(AlienbrainConnection::getProperty(path, "PerforceServerDir"));
	
	if(ms_perforceServerPath.empty())
		ms_perforceServerPath = ms_perforceClientPath;

	// ----------

	ms_assetDBCategory = static_cast<LPCSTR>(AlienbrainConnection::getProperty(path, "AssetDBCategory"));
	ExporterLog::setAssetGroup(ms_assetDBCategory);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Reset any internal data (useful between multiple exports)
 *
 */
void AlienbrainImporter::reset()
{
	FATAL(!ms_installed, ("AlienbrainImporter not installed\n"));
	ms_currentPath = "";
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * The function determines the base Alienbrain path for the export
 * from the open maya file (which must be located in a \\scenes folder).  This function also does 
 * some validation, including 1:that the file exists in a valid location, meaning the Alienbrain database
 * 2: that the maya file is located in a \\scenes folder
 *
 * @pre Currently open Maya file is located in a \\scenes folder
 * @pre Currently open Maya file is located in the Alienbrain database
 */
bool AlienbrainImporter::preImport(const std::string& assetName, bool showGUI)
{
	UNREF(assetName);
	FATAL(!ms_installed, ("AlienbrainImporter not installed\n"));
	FATAL(!ms_connected, ("AlienbrainImporter connected to server\n"));
	bool returnValue = true;

	//the base path will always be where this maya file came from (must be in Alienbrain)
	//so find it's base and store it
	ms_currentPath = findWorkspaceLocation(ExporterLog::getSourceFilename());
	if (ms_currentPath == "")
	{
		if(showGUI)
			MESSENGER_MESSAGE_BOX(NULL, "File does not appear to exist in the Alienbrain project namespace, reasons could be:\n 1: The file is located on a different drive (i.e. c:)\n 2: The file is not being stored in the Alienbrain database\n", "Error", MB_OK);
		else
			MESSENGER_LOG_ERROR(("File does not appear to exist in the Alienbrain project namespace\n"));
		return false;
	}

	//pull off \scenes\<filename>.mb
	std::string::size_type pos = ms_currentPath.find("/scenes/");
	if (pos == static_cast<unsigned int>(std::string::npos))
	{
		if(showGUI)
			MESSENGER_MESSAGE_BOX(NULL, "Maya file not located in a \\scenes folder", "Error", MB_OK);
		else
			MESSENGER_LOG_ERROR(("Maya file not located in a \\scenes folder\n"));
		return false;
	}
	ms_currentPath = ms_currentPath.substr(0, pos+1); //+1 to keep the front slash

	return returnValue;
}

//////////////////////////////////////////////////////////////////////////////////

bool AlienbrainImporter::validateFiles(const std::vector<std::string>& textureList, bool showGUI)
{
	//validate the tgas
	if(!validateTextureList(textureList, showGUI))
		return false;

	//validate the mb
	const std::string sourceFilename = MFileIO::currentFile().asChar();
	return validateFile(sourceFilename, showGUI);
}

//////////////////////////////////////////////////////////////////////////////////

bool AlienbrainImporter::validateTextureList(const std::vector<std::string>& textureList, bool showGUI)
{
	FATAL(!ms_installed, ("AlienbrainImporter not installed\n"));
	FATAL(!ms_connected, ("AlienbrainImporter connected to server\n"));

	std::string workspaceName;

	for(std::vector<std::string>::const_iterator i = textureList.begin(); i != textureList.end(); ++i)
	{
		if(!validateFile(*i, showGUI))
			return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

bool AlienbrainImporter::validateFile(const std::string& file, bool showGUI)
{
	if(file.find("working") != file.npos)
	{
		std::string errorString = "File ";
		errorString += file.c_str();
		errorString += " is located in a \"working\" directory, move the file and reexport.\n";
		if(showGUI)
			MESSENGER_MESSAGE_BOX(NULL, errorString.c_str(), "Error", MB_OK);
		MESSENGER_LOG_ERROR((errorString.c_str()));
		return false;
	}

	if(!AlienbrainConnection::objectExistsInDB(file.c_str()))
	{
		std::string errorString = "File ";
		errorString += file.c_str();
		errorString += " is not located in Alienbrain, cannot submit.\n";
		if(showGUI)
			MESSENGER_MESSAGE_BOX(NULL, errorString.c_str(), "Error", MB_OK);
		MESSENGER_LOG_ERROR((errorString.c_str()));
		return false;
	}
	if(AlienbrainConnection::NewerFileOnServer(file.c_str()))
	{
		std::string errorString = "File ";
		errorString += file.c_str();
		errorString += " has a newer version in AB, sync the file then reexport.\n";
		if(showGUI)
			MESSENGER_MESSAGE_BOX(NULL, errorString.c_str(), "Error", MB_OK);
		MESSENGER_LOG_ERROR((errorString.c_str()));
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Import the log file built by the export process  The Alienbrain connection must have been 
 * initialized (via Init call).
 */
void AlienbrainImporter::importLogFile()
{
	FATAL(!ms_installed, ("AlienbrainImporter not installed\n"));
	FATAL(!ms_connected, ("AlienbrainImporter connected to server\n"));
	MESSENGER_LOG(("====== BEGINNING IMPORT PROCESS TO ALIENBRAIN ======\n"));
	//import the log file
	const std::string logFilename = ExporterLog::getLogFilename();
	if(logFilename == "")
	{
		MESSENGER_LOG_ERROR(("Empty log file name in AlienbrainImporter::importLogFile, is the correct node selected?\n"));
		return;
	}
	if(ms_currentPath == "")
	{
		MESSENGER_LOG_ERROR(("Empty current path in AlienbrainImporter::importLogFile, is the correct node selected?\n"));
		return;
	}
	std::string dir = ms_currentPath;
	if(ms_logImportPath == "")
	{
		MESSENGER_LOG_ERROR(("Empty log import path in AlienbrainImporter::importLogFile, is the correct node selected?\n"));
		return;
	}
	dir += ms_logImportPath;
	bool result = AlienbrainConnection::storeObject(logFilename.c_str(), dir.c_str(), "reexport from mayaExporter", "");

	CNxNPath path = AlienbrainConnection::makeNamespacePath(logFilename.c_str(), dir.c_str());

	result = AlienbrainConnection::setProperty(path, "OBJ_name", logFilename.c_str());
	result = AlienbrainConnection::setProperty(path, "OBJ_type", "log");

	UNREF(result);
	MESSENGER_LOG(("====== IMPORT PROCESS TO ALIENBRAIN COMPLETE ======\n"));
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Connect to the Alienbrain server
 */
bool AlienbrainImporter::connectToServer()
{
	FATAL(!ms_installed, ("AlienbrainImporter not installed\n"));

	//don't connect if in the middle of a multi-export
	if(ms_multiExporting)
		return true;

	//don't connect if already connected
	if(ms_connected)
		return true;
	ms_connected = true;

	return AlienbrainConnection::initConnection(SetDirectoryCommand::getDirectoryString(ALIENBRAIN_PROJECT_NAME_INDEX));
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Disconnect from the Alienbrain server
 */
bool AlienbrainImporter::disconnectFromServer()
{
	FATAL(!ms_installed, ("AlienbrainImporter not installed\n"));

	//don't disconnect if in the middle of a multi-export
	if(ms_multiExporting)
		return true;

	//don't disconnect if not connected
	if(!ms_connected)
		return true;
	AlienbrainConnection::shutdownConnection();
	ms_connected = false;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * This function takes a file or directory location on disk (i.e. z:\art\swg\asset.mb),
 * and returns it's location in the AB workspace (i.e. \Workspace\art\swg\asset.mb)
 *
 * @return the workspace location, or "" on failure
 */
std::string AlienbrainImporter::findWorkspaceLocation(const std::string& file)
{
	char *lowerStr = new char[file.size()+1];
	strcpy(lowerStr, file.c_str());
	std::string lowerFile = _strlwr(lowerStr);
	delete[] lowerStr;
	std::string::size_type pos = lowerFile.find(SetDirectoryCommand::getDirectoryString(ALIENBRAIN_PROJECT_NAME_INDEX));
	//if this files doesn't exist in the project, return an empty string
	if(pos == static_cast<unsigned int>(std::string::npos))
		return "";

	std::string workspaceLocation = "/Workspace/";
	workspaceLocation += lowerFile.substr(pos); //substring goes to end implicit
	return workspaceLocation;
}

//////////////////////////////////////////////////////////////////////////////////
#endif
// JU_TODO: end alienbrain def out

