// ======================================================================
//
// ExportManager.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

//precompiled header includes
#include "FirstMayaExporter.h"

//module include
#include "ExportManager.h"

#include "AlienbrainImporter.h"
#include "ExportArgs.h"
#include "ExporterLog.h"
#include "Messenger.h"
#include "PerforceImporter.h"
#include "PluginMain.h"
#include "SetDirectoryCommand.h"

#include <sstream>

//////////////////////////////////////////////////////////////////////////////////

Messenger* ExportManager::messenger;
std::vector<std::string> ExportManager::cs_validBranches;

//////////////////////////////////////////////////////////////////////////////////

/**
 * Initialize the module
 */
void ExportManager::install(Messenger* newMessenger)
{
	messenger    = newMessenger;

	cs_validBranches.push_back(ExportArgs::cs_currentBranch);
	cs_validBranches.push_back(ExportArgs::cs_x1Branch);
	cs_validBranches.push_back(ExportArgs::cs_x2Branch);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Shutdown the module
 */
void ExportManager::remove()
{
	messenger = NULL;
}

//////////////////////////////////////////////////////////////////////////////////

bool ExportManager::validateTextureList(bool showGUI)
{
// JU_TODO: alienbrain def out
#if 0
	MESSENGER_LOG(("The following textures MUST exist in Alienbrain AND be synced to the newest revision, validating...\n"));
	const std::vector<std::string>& textureList = ExporterLog::getSourceTexturesFilenames();
	for(std::vector<std::string>::const_iterator textureIt = textureList.begin(); textureIt != textureList.end(); ++textureIt)
	{
		MESSENGER_LOG(("%s\n", textureIt->c_str()));
	}
	if((AlienbrainImporter::validateFiles(textureList, showGUI)))
	{
		MESSENGER_LOG(("done\n"));
	}
	else
	{
		return false;
	}
#else
	UNREF(showGUI);
#endif
// JU_TODO: end alienbrain def out

	return true;
}

//////////////////////////////////////////////////////////////////////////////////

const std::vector<std::string> & ExportManager::getValidBranches()
{
	return cs_validBranches;
}

//////////////////////////////////////////////////////////////////////////////////

void ExportManager::setValidBranches(const std::vector<std::string> & newValidBranches)
{
	cs_validBranches.clear();
	for(std::vector<std::string>::const_iterator i = newValidBranches.begin(); i != newValidBranches.end(); ++i)
		cs_validBranches.push_back(*i);
}

//////////////////////////////////////////////////////////////////////////////////

/** A valid packed branch string has all branches listed, deliniated by a comma and NO whitespace
 *  "i.e. "current,x1,zulu,test,yorkshire"
 */
void ExportManager::setValidBranchesPacked(const std::string & packedBranchNames)
{
	UNREF(packedBranchNames);
	//unpack, store branches
	cs_validBranches.clear();
	std::string::size_type pos = 0;
	while(pos != packedBranchNames.npos)
	{
		std::string::size_type endStr = packedBranchNames.find_first_of(",", pos);
		if(endStr != packedBranchNames.npos)
		{
			std::string branchName = packedBranchNames.substr(pos, endStr-pos);
			cs_validBranches.push_back(branchName);
			pos = endStr + 1;
		}
		else
		{
			std::string branchName = packedBranchNames.substr(pos);
			cs_validBranches.push_back(branchName);
			pos = endStr;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////

bool ExportManager::isValidBranch(const std::string & branch)
{
	std::vector<std::string>::iterator i = std::find(cs_validBranches.begin(), cs_validBranches.end(), branch);
	return (i != cs_validBranches.end());
}

//////////////////////////////////////////////////////////////////////////////////

// pops the viewer with the given asset
void ExportManager::LaunchViewer(const std::string& asset)
{
	std::string viewerFullPath = SetDirectoryCommand::getDirectoryString(VIEWER_LOCATION_INDEX);
	int truncIndex = viewerFullPath.find_last_of("\\");
	std::string viewerPath = viewerFullPath.substr(0,truncIndex);

	std::stringstream strCommandStream;
	strCommandStream << "\"" << viewerFullPath.c_str() << "\" " << asset.c_str();

	MESSENGER_LOG(("\n\nLaunching viewer:\n"));
	MESSENGER_LOG(("... strCommandStream.str().c_str() = [%s]\n",strCommandStream.str().c_str()));
	MESSENGER_LOG(("... viewerPath.c_str() = [%s]\n\n",viewerPath.c_str()));

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si,sizeof(si));
	si.cb = sizeof(si);
	CreateProcess(NULL,(LPTSTR)strCommandStream.str().c_str(),NULL,NULL,false,0,NULL,viewerPath.c_str(),&si,&pi);
}

//////////////////////////////////////////////////////////////////////////////////


// JU_TODO: alienbrain temp workaround

struct AssetElem
{
	char m_alienbrainDirectory[256];
	char m_perforceDirectory[256];
	char m_assetDBCategory[256];
};

// these need to be in reverse alphabetical order
static AssetElem s_assetTable[] = {
	{"art/world/kashyyk",                  "//depot/swg/current/data/sku.2/sys.client/exported/world",        "Other"},
	{"art/world/flora/kashyyyk",           "//depot/swg/current/data/sku.2/sys.client/exported/world",        "Other"},
	{"art/world/flora/mustafar",           "//depot/swg/current/data/sku.3/sys.client/exported/world",        "Other"},
	{"art/world",                          "//depot/swg/current/data/sku.0/sys.client/exported/world",        "Other"},
	{"art/vehicle",                        "//depot/swg/current/data/sku.0/sys.client/exported/vehicle",      "Vehicle"},
	{"art/space",                          "//depot/swg/current/data/sku.1/sys.client/exported/space",        "Space"},
	{"art/item/weapon",                    "//depot/swg/current/data/sku.0/sys.client/exported/item",         "Weapon"},
	{"art/item",                           "//depot/swg/current/data/sku.0/sys.client/exported/item",         "Item"},
	{"art/creature",                       "//depot/swg/current/data/sku.0/sys.client/exported/creature",     "Creature"},
	{"art/character/wearable",             "//depot/swg/current/data/sku.0/sys.client/exported/character",    "Wearable"},
	{"art/character/pc",                   "//depot/swg/current/data/sku.0/sys.client/exported/character",    "Player"},
	{"art/character/npc/droid",            "//depot/swg/current/data/sku.0/sys.client/exported/character",    "Droid"},
	{"art/character/npc",                  "//depot/swg/current/data/sku.0/sys.client/exported/character",    "NPC"},
	{"art/architecture/themepark/mustafar","//depot/swg/current/data/sku.3/sys.client/exported/architecture", "Architecture"},
	{"art/architecture/themepark/kashyyyk","//depot/swg/current/data/sku.2/sys.client/exported/architecture", "Architecture"},
	{"art/architecture",                   "//depot/swg/current/data/sku.0/sys.client/exported/architecture", "Architecture"}
};

static int getAssetTableIndex(const std::string& alienbrainDir)
{
	const int num_elems = sizeof(s_assetTable)/sizeof(AssetElem);
	for(int i = 0; i < num_elems;++i)
	{
		std::string::size_type pos = alienbrainDir.find(s_assetTable[i].m_alienbrainDirectory);
		if(pos != alienbrainDir.npos)
		{
			return i;
		}
	}
	return -1;
}

std::string ExportManager::getPerforceClientDir(const std::string& alienbrainDir)
{
	int index = getAssetTableIndex(alienbrainDir);
	if(index != -1)
	{
		return s_assetTable[index].m_perforceDirectory;
	}
	return "";
}
std::string ExportManager::getPerforceSharedDir(const std::string& alienbrainDir)
{
	int index = getAssetTableIndex(alienbrainDir);
	if(index != -1)
	{
		std::string ret =  s_assetTable[index].m_perforceDirectory;
		std::string::size_type pos = ret.find("sys.client");
		if(pos != ret.npos)
		{
			ret.replace(pos, sizeof("sys.client")-1, "sys.shared");
			return ret;
		}
	}
	return "";
}
std::string ExportManager::getPerforceServerDir(const std::string& alienbrainDir)
{
	int index = getAssetTableIndex(alienbrainDir);
	if(index != -1)
	{
		std::string ret =  s_assetTable[index].m_perforceDirectory;
		std::string::size_type pos = ret.find("sys.client");
		if(pos != ret.npos)
		{
			ret.replace(pos, sizeof("sys.client")-1, "sys.server");
			return ret;
		}
	}
	return "";
}
std::string ExportManager::getAssetDBCategory(const std::string& alienbrainDir)
{
	int index = getAssetTableIndex(alienbrainDir);
	if(index != -1)
	{
		return s_assetTable[index].m_assetDBCategory;
	}
	return "";
}
// JU_TODO: end alienbrain temp workaround