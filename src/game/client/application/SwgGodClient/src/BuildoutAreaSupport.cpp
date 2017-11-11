// ======================================================================
//
// BuildoutAreaSupport.cpp
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "BuildoutAreaSupport.h"

#include "clientGame/Game.h"
#include "clientGame/WorldSnapshot.h"
#include "fileInterface/StdioFile.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcStringTable.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMath/Quaternion.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableWriter.h"
#include "sharedUtility/TemplateParameter.h"
#include "UnicodeUtils.h"

#include "AbstractFilesystemTree.h"
#include "ActionHack.h"
#include "ActionsBuildoutArea.h"
#include "FilesystemTree.h"
#include "ConfigGodClient.h"
#include "MainFrame.h"
#include "GodClientPerforce.h"
#include "ServerObjectData.h"
#include <unordered_set>
#include <string>

// ======================================================================

namespace BuildoutAreaSupportNamespace
{

	// ----------------------------------------------------------------------

	struct CachedBuildoutArea
	{
		std::vector<ServerBuildoutAreaRow> serverRows;
		std::vector<ClientBuildoutAreaRow> clientRows;
	};

	bool s_installed = false;
	bool s_unlockAll = false;
	bool s_unlockNonStructures = false;
	bool s_unlockServerOnly = false;
	std::unordered_set<int> s_unlockedIds;

	typedef std::map< NetworkId, const BuildoutArea * > AreaIdMap;
	AreaIdMap          s_areaIdMap;

	std::string s_clientSrcPath;
	std::string s_clientDataPath;
	std::string s_serverSrcPath;
	std::string s_serverDataPath;
	std::string s_serverSrcDepotPath;
	CrcStringTable s_serverTemplateCrcStringTable;
	typedef std::map<std::string, CachedBuildoutArea> BuildoutAreaCacheMap;
	BuildoutAreaCacheMap s_buildoutAreaCacheMap;
	ConstCharCrcString const s_cellSharedTemplateName("object/cell/shared_cell.iff");
	ConstCharCrcString const s_cellServerTemplateName("object/cell/cell.iff");
	char const * const s_dynamicVariableTypeNames[] =
	{
		"int",
		"int[]",
		"float",
		"float[]",
		"string",
		"string[]",
		"oid",
		"oid[]",
		"location",
		"location[]",
		"list",
		"stringid",
		"stringid[]",
		"transform",
		"transform[]",
		"vector",
		"vector[]"
	};

	// ----------------------------------------------------------------------

	void getBuildoutAreaTableNames(std::string const &areaName, std::string &serverTabFilename, std::string &serverIffFilename, std::string &clientTabFilename, std::string &clientIffFilename);
	void createDirectoriesForFiles(std::vector<std::string> const &fileList);
	void install();
	void preModifyBuildoutArea(std::string const &serverTabFilename, std::string const &serverIffFilename, std::string const &clientTabFilename, std::string const &clientIffFilename);
	void postModifyBuildoutArea(std::string const &serverTabFilename, std::string const &serverIffFilename, std::string const &clientTabFilename, std::string const &clientIffFilename);
	BuildoutArea const *getBuildoutAreaForId(NetworkId const &id);
	CachedBuildoutArea *loadBuildoutArea(BuildoutArea const &buildoutArea);
	bool isServerOnlyTemplate(uint32 serverTemplateCrc);
	float getServerTemplateUpdateRadius(CrcString const &serverTemplateName);
	uint32 getSharedTemplatePortalLayoutCrc(CrcString const &sharedTemplateName);
	ConstCharCrcString const getServerTemplateName(uint32 serverTemplateCrc);
	ConstCharCrcString const getSharedTemplateName(uint32 sharedTemplateCrc);
	ConstCharCrcString const getSharedTemplateForServerTemplate(CrcString const &serverTemplateName);
	int getSharedTemplateCellCount(CrcString const &sharedTemplateName);
	void unpackScriptList(std::string const &packedScriptList, std::vector<std::string> &unpackedScriptList);
	void unpackObjvarList(std::string const &packedObjvarList, std::vector<std::string> &unpackedObjvarList);
	std::string const packObjvarList(std::vector<std::string> const &objvarList);
	void setObjvar(Object const &obj, std::string const &objvarSpec);
	bool isLocked(BuildoutArea const &buildoutArea, int id);
	void unlock(int id);
	void writeOutServerEventArea(std::string const eventName, std::vector<std::string>  const & objects);

	// ----------------------------------------------------------------------

	class BuildoutWorldId
	{
	public:
		BuildoutWorldId( int id, int areaIndex )
		{
			NetworkId::NetworkIdType value = static_cast< NetworkId::NetworkIdType >( id );

			if ( id < 0 )
			{
				const int c_bitsToShift = 48;
				value ^= static_cast< NetworkId::NetworkIdType > ( areaIndex + 1 ) << c_bitsToShift;
			}

			m_networkId = NetworkId( value );
		}

		operator int64() const
		{
			return m_networkId.getValue();
		}

		operator const NetworkId &() const
		{
			return m_networkId;
		}
	private:
		NetworkId m_networkId;
	};
}
using namespace BuildoutAreaSupportNamespace;

#include <qmessagebox.h>

// ======================================================================

void BuildoutAreaSupport::getBuildoutAreaList(std::string const &sceneName, std::vector<std::string> &areaNames, std::vector<std::string> &areaLocations, std::vector<std::string> &areaStatuses)
{
	// load up the area datatable for this scene
	{
		SharedBuildoutAreaManager::load(sceneName.c_str());
		std::vector<BuildoutArea> const &buildoutAreas = SharedBuildoutAreaManager::getBuildoutAreasForCurrentScene();

		for (std::vector<BuildoutArea>::const_iterator i = buildoutAreas.begin(); i != buildoutAreas.end(); ++i)
		{
			areaNames.push_back((*i).areaName);
			char buf[128];
			IGNORE_RETURN(snprintf(buf, sizeof(buf)-1, "%g %g %g %g", (*i).rect.x0, (*i).rect.y0, (*i).rect.x1, (*i).rect.y1));
			buf[sizeof(buf)-1] = '\0';
			areaLocations.push_back(std::string(buf));
			areaStatuses.push_back("Missing");
		}
	}

	if (!s_installed)
		install();


	std::string areaPath = s_serverSrcDepotPath;
	areaPath += "/datatables/buildout/";
	areaPath += sceneName;

	if (ConfigGodClient::getConnectToPerforce())
	{
		// scan the datatable directory for the specified scene and update status
		std::string result;

		AbstractFilesystemTree *afst = GodClientPerforce::getInstance().getFileTree(areaPath, "tab", result, GodClientPerforce::FileState_depot);
		if (afst)
		{
			AbstractFilesystemTree::Node const * const node = afst->getRootNode();
			if (node)
			{
				for (AbstractFilesystemTree::Node::ConstIterator i = node->begin (); i != node->end (); ++i)
				{
					std::string name((*i)->name);
					if (name.size() > 4)
						name.erase(name.size()-4, 4);

					bool found = false;
					for (unsigned int areaIndex = 0; areaIndex < areaNames.size(); ++areaIndex)
					{
						if (areaNames[areaIndex] == name)
						{
							areaStatuses[areaIndex] = "Active";
							found = true;
						}
					}

					if (!found)
					{
						areaNames.push_back(name);
						areaLocations.push_back();
						areaStatuses.push_back("Inactive");
					}
				}
			}
			delete afst;
		}

		afst = GodClientPerforce::getInstance().getFileTree(areaPath, "tab", result, GodClientPerforce::FileState_add);
		if (afst)
		{
			AbstractFilesystemTree::Node const * const node = afst->getRootNode();
			if (node)
			{
				for (AbstractFilesystemTree::Node::ConstIterator i = node->begin (); i != node->end (); ++i)
				{
					std::string name((*i)->name);
					if (name.size() > 4)
						name.erase(name.size()-4, 4);

					bool found = false;
					for (unsigned int areaIndex = 0; areaIndex < areaNames.size(); ++areaIndex)
					{
						if (areaNames[areaIndex] == name)
						{
							areaStatuses[areaIndex] = "New";
							found = true;
						}
					}

					if (!found)
					{
						areaNames.push_back(name);
						areaLocations.push_back();
						areaStatuses.push_back("New+Inactive");
					}
				}
			}
			delete afst;
		}

		afst = GodClientPerforce::getInstance().getFileTree(areaPath, "tab", result, GodClientPerforce::FileState_edit);
		if (afst)
		{
			AbstractFilesystemTree::Node const * const node = afst->getRootNode();
			if (node)
			{
				for (AbstractFilesystemTree::Node::ConstIterator i = node->begin (); i != node->end (); ++i)
				{
					std::string name((*i)->name);
					if (name.size() > 4)
						name.erase(name.size()-4, 4);

					bool found = false;
					for (unsigned int areaIndex = 0; areaIndex < areaNames.size(); ++areaIndex)
					{
						if (areaNames[areaIndex] == name)
						{
							areaStatuses[areaIndex] = "Edit";
							found = true;
						}
					}

					if (!found)
					{
						areaNames.push_back(name);
						areaLocations.push_back();
						areaStatuses.push_back("Edit+Inactive");
					}
				}
			}
			delete afst;
		}
	}
	else
	{
		FilesystemTree* fst = new FilesystemTree();

		fst->setRootPath(areaPath);
		fst->setFilter("*.tab");
		fst->populateTree();

		AbstractFilesystemTree::Node const * const node = fst->getRootNode();
		if (node)
		{
			for (AbstractFilesystemTree::Node::ConstIterator i = node->begin (); i != node->end (); ++i)
			{
				std::string name((*i)->name);
				if (name.size() > 4)
					name.erase(name.size()-4, 4);

				bool found = false;
				for (unsigned int areaIndex = 0; areaIndex < areaNames.size(); ++areaIndex)
				{
					if (areaNames[areaIndex] == name)
					{
						areaStatuses[areaIndex] = "Active";
						found = true;
					}
				}

				if (!found)
				{
					areaNames.push_back(name);
					areaLocations.push_back();
					areaStatuses.push_back("Inactive");
				}
			}
		}

		delete fst;
	}
}

// ----------------------------------------------------------------------

bool BuildoutAreaSupport::openBuildoutFilesForEditing( const std::string &areaName )
{
	// determine the client and server datatable filenames
	std::string serverTabFilename, serverIffFilename, clientTabFilename, clientIffFilename;
	getBuildoutAreaTableNames(areaName, serverTabFilename, serverIffFilename, clientTabFilename, clientIffFilename);
	
	if (ConfigGodClient::getConnectToPerforce())
	{
		// open the files for edit, in case they already exist
		std::vector<std::string> perforceFiles;
		perforceFiles.push_back(serverTabFilename);
		perforceFiles.push_back(serverIffFilename);
		perforceFiles.push_back(clientTabFilename);
		perforceFiles.push_back(clientIffFilename);
		
		std::string result;
		
		//warn, and possibly abort, if anyone else has the buildout files checked out already
		std::vector<std::string> alsoOpenedBy;
		for(std::vector<std::string>::const_iterator it = perforceFiles.begin(); it != perforceFiles.end(); ++it)
		{
			bool ok = GodClientPerforce::getInstance().fileAlsoOpenedBy(*it, alsoOpenedBy, result);
			
			if (!ok)
			{
				QMessageBox::critical (0, "GodClientPerforce::getInstance().fileAlsoOpenedBy error!", result.c_str());
				return false;
			}
			
			if(!alsoOpenedBy.empty())
			{
				//build the warning message string
				std::string msg = "File:\n";
				msg += *it + " is also checked out by:\n";
				for(std::vector<std::string>::const_iterator it2 = alsoOpenedBy.begin(); it2 != alsoOpenedBy.end(); ++it2)
				{
					msg += *it2 + "\n";
				}


				if ( !Game::getSinglePlayer() )
				{
					QMessageBox::warning (0, "I can not check these files out because...", msg.c_str(), QMessageBox::Ok, 0 );
					return false;
				}

				QMessageBox::warning (0, "Uh oh!", msg.c_str(), QMessageBox::Ok, 0 );
			}
		}
		
		preModifyBuildoutArea(serverTabFilename, serverIffFilename, clientTabFilename, clientIffFilename);
		
		if ( Game::getSinglePlayer() )
		{
			if (!GodClientPerforce::getInstance().editFiles(perforceFiles, result))
			{
				const std::string msg = "Could not open file(s) for perforce edit.\n" + result;
				IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
				return false;
			}
		}
		else
		{
			if (!GodClientPerforce::getInstance().editFilesAndLock(perforceFiles, result))
			{
				const std::string msg = "Could not open file(s) for perforce edit and lock!\n" + result;
				IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
				return false;
			}
		}
	}

	return true;
}


void BuildoutAreaSupport::saveBuildoutArea(std::string const &areaName)
{
	// determine the client and server datatable filenames
	std::string serverTabFilename, serverIffFilename, clientTabFilename, clientIffFilename;
	getBuildoutAreaTableNames(areaName, serverTabFilename, serverIffFilename, clientTabFilename, clientIffFilename);
	
	// find the buildout in the cache and save it
	BuildoutAreaCacheMap::iterator i = s_buildoutAreaCacheMap.find(serverTabFilename);
	if (i == s_buildoutAreaCacheMap.end())
		return;

	std::map<std::string, std::vector<std::string>> serverEventObjectMap;
	
	CachedBuildoutArea const &cachedBuildoutArea = (*i).second;
	std::vector<ServerBuildoutAreaRow> const &serverRows = cachedBuildoutArea.serverRows;
	std::vector<ClientBuildoutAreaRow> const &clientRows = cachedBuildoutArea.clientRows;

	if (ConfigGodClient::getConnectToPerforce())
	{
		// open the files for edit, in case they already exist
		std::vector<std::string> perforceFiles;
		perforceFiles.push_back(serverTabFilename);
		perforceFiles.push_back(serverIffFilename);
		perforceFiles.push_back(clientTabFilename);
		perforceFiles.push_back(clientIffFilename);
		
		std::string result;
		
		//warn, and possibly abort, if anyone else has the buildout files checked out already
		std::vector<std::string> alsoOpenedBy;
		for(std::vector<std::string>::const_iterator it = perforceFiles.begin(); it != perforceFiles.end(); ++it)
		{
			bool ok = GodClientPerforce::getInstance().fileAlsoOpenedBy(*it, alsoOpenedBy, result);
			
			if (!ok)
			{
				QMessageBox::critical (0, "Error", result.c_str());
				return;
			}
			
			if(!alsoOpenedBy.empty())
			{
				//build the warning message string
				std::string msg = "File:\n";
				msg += *it + " is also checked out by:\n";
				for(std::vector<std::string>::const_iterator it2 = alsoOpenedBy.begin(); it2 != alsoOpenedBy.end(); ++it2)
				{
					msg += *it2 + "\n";
				}
				msg += "Check out anyway?";
				switch (QMessageBox::warning (0, "Someone else has this file checked out!", msg.c_str(), QMessageBox::Yes | QMessageBox::Default, QMessageBox::No))
				{
				case QMessageBox::No: 
					{
						QMessageBox::information (0, "Aborted", "Buildout save aborted.", QMessageBox::Ok);
						return;
					}
					break;
					
				default:
					break;
				}
			}
		}
		
		preModifyBuildoutArea(serverTabFilename, serverIffFilename, clientTabFilename, clientIffFilename);
		
		if ( Game::getSinglePlayer () )
		{
			if (!GodClientPerforce::getInstance().editFiles(perforceFiles, result))
			{
				const std::string msg = "Could not open file(s) for perforce edit.\n" + result;
				IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
			}
		}
		else
		{
			if (!GodClientPerforce::getInstance().editFilesAndLock(perforceFiles, result))
			{
				const std::string msg = "Could not open file(s) for perforce edit.\n" + result;
				IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
			}
		}
	}
	
	// write the tables
	{
		StdioFile serverOutputFile(serverTabFilename.c_str(), "w");
		StdioFile clientOutputFile(clientTabFilename.c_str(), "w");
		
		if (!serverOutputFile.isOpen())
		{
			std::string msg("File not writable: ");
			msg += serverTabFilename;
			IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		}
		else if (!clientOutputFile.isOpen())
		{
			std::string msg("File not writable: ");
			msg += clientTabFilename;
			IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		}
		else
		{
			// save the table headers
			{
				std::string const serverHeader(
					"objid\tcontainer\tserver_template_crc\tcell_index\tpx\tpy\tpz\tqw\tqx\tqy\tqz\tscripts\tobjvars\n"
					"i\ti\th\ti\tf\tf\tf\tf\tf\tf\tf\ts\tp\n");
				serverOutputFile.write(serverHeader.length(), serverHeader.c_str());
				
				std::string const clientHeader(
					"objid\tcontainer\ttype\tshared_template_crc\tcell_index\tpx\tpy\tpz\tqw\tqx\tqy\tqz\tradius\tportal_layout_crc\n"
					"i\ti\ti\th\ti\tf\tf\tf\tf\tf\tf\tf\tf\ti\n");
				clientOutputFile.write(clientHeader.length(), clientHeader.c_str());
			}
			
			// save the server rows
			{
				for (std::vector<ServerBuildoutAreaRow>::const_iterator i = serverRows.begin(); i != serverRows.end(); ++i)
				{
					ConstCharCrcString const &serverTemplateName = getServerTemplateName((*i).serverTemplateCrc);
					char buf[512];
					IGNORE_RETURN(snprintf(buf, sizeof(buf)-1, "%d\t%d\t%s\t%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t",
						(*i).id,
						(*i).container,
						serverTemplateName.getString(),
						(*i).cellIndex,
						(*i).position.x, (*i).position.y, (*i).position.z,
						(*i).orientation.w, (*i).orientation.x, (*i).orientation.y, (*i).orientation.z));
					buf[sizeof(buf)-1] = '\0';
					std::string::size_type index = (*i).objvars.find("eventRequired");

					if(index != std::string::npos )
					{
						std::string::size_type eventSubStringStart = (*i).objvars.find("|", index );
						if(eventSubStringStart == std::string::npos)
							continue; //Malformed event obj var. TODO: Warning goes here!

						eventSubStringStart = (*i).objvars.find("|", eventSubStringStart + 1);

						std::string::size_type eventSubStringEnd = (*i).objvars.find("|", eventSubStringStart + 1);

						std::string eventName = (*i).objvars.substr(eventSubStringStart + 1, (eventSubStringEnd - 1) - eventSubStringStart );

						DEBUG_WARNING(true, ("Found an Event Object! Event Name [%s]", eventName.c_str()));
			
						std::vector<std::string>* eventList = NULL;
						std::map<std::string, std::vector<std::string>>::iterator eventIter = serverEventObjectMap.find(eventName);

						if(eventIter == serverEventObjectMap.end())
						{
							std::pair<std::map<std::string, std::vector<std::string>>::iterator, bool> insertIter;
							insertIter = serverEventObjectMap.insert(std::make_pair<std::string, std::vector<std::string>>(eventName, std::vector<std::string>()));
							if(insertIter.second)
								eventList = &(*insertIter.first).second;
						}
						else
							eventList = &(*eventIter).second;

						std::string objectOutputString;
						objectOutputString.append(buf);
						objectOutputString.append((*i).scripts);
						objectOutputString.append("\t", 1);
						objectOutputString.append((*i).objvars);
						objectOutputString.append("\n", 1);
						
						if(eventList)
							eventList->push_back(objectOutputString);
						
						continue;
					}

					serverOutputFile.write(strlen(buf), buf);
					serverOutputFile.write((*i).scripts.length(), (*i).scripts.c_str());
					serverOutputFile.write(1, "\t");
					serverOutputFile.write((*i).objvars.length(), (*i).objvars.c_str());
					serverOutputFile.write(1, "\n");
				}
			}
			
			// save the client rows
			{
				for (std::vector<ClientBuildoutAreaRow>::const_iterator i = clientRows.begin(); i != clientRows.end(); ++i)
				{
					ConstCharCrcString const &sharedTemplateName = getSharedTemplateName((*i).sharedTemplateCrc);
					char buf[512];
					IGNORE_RETURN(snprintf(buf, sizeof(buf)-1, "%d\t%d\t%d\t%s\t%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%d\n",
						(*i).id,
						(*i).container,
						(*i).type,
						sharedTemplateName.getString(),
						(*i).cellIndex,
						(*i).position.x, (*i).position.y, (*i).position.z,
						(*i).orientation.w, (*i).orientation.x, (*i).orientation.y, (*i).orientation.z,
						(*i).radius,
						static_cast<int>((*i).portalLayoutCrc)));
					buf[sizeof(buf)-1] = '\0';
					clientOutputFile.write(strlen(buf), buf);
				}
			}
		}
		
		serverOutputFile.close();
		clientOutputFile.close();
		
		postModifyBuildoutArea(serverTabFilename, serverIffFilename, clientTabFilename, clientIffFilename);
		
		// Write out event specific tables.
		std::map<std::string, std::vector<std::string>>::iterator beginIter = serverEventObjectMap.begin();
		for(; beginIter != serverEventObjectMap.end(); ++beginIter)
		{
			writeOutServerEventArea((*beginIter).first, (*beginIter).second);
		}
	}
	
}

// ----------------------------------------------------------------------

void BuildoutAreaSupport::saveBuildoutArea(std::string const &areaName, std::vector<ServerBuildoutAreaRow> const &newServerRows, std::vector<ClientBuildoutAreaRow> const &newClientRows)
{
	// deprecated, saving of server built areas

	// determine the client and server datatable filenames
	std::string serverTabFilename, serverIffFilename, clientTabFilename, clientIffFilename;
	getBuildoutAreaTableNames(areaName, serverTabFilename, serverIffFilename, clientTabFilename, clientIffFilename);

	CachedBuildoutArea &cachedBuildoutArea = s_buildoutAreaCacheMap[serverTabFilename];

	cachedBuildoutArea.serverRows = newServerRows;
	cachedBuildoutArea.clientRows = newClientRows;

	saveBuildoutArea(areaName);

	// refresh the area listbox
	ActionsBuildoutArea::getInstance().actionRefresh->doActivate();

	// refresh the world snapshot

}

// ----------------------------------------------------------------------

Object* BuildoutAreaSupport::createNewObject(CrcString const &templateName, CellProperty const *cellProperty, Transform const &transform_p)
{

	int const cellIndex = cellProperty && !cellProperty->isWorldCell() ? cellProperty->getCellIndex() : 0;
	Object const * const container = cellIndex ? &cellProperty->getOwner() : 0;

	// only allow building in the world cell and in buildout area object cells	
	if (container && !(container->getNetworkId() < NetworkId::cms_invalid))
		return NULL;

	BuildoutArea const * const buildoutArea = container ? getBuildoutAreaForId(container->getNetworkId()) : SharedBuildoutAreaManager::findBuildoutAreaAtPosition(transform_p.getPosition_p(), false);

	if (!buildoutArea)
		return NULL;

	Quaternion const orientation(transform_p);
	Vector position(transform_p.getPosition_p());
	if (cellIndex == 0)
	{
		position.x -= buildoutArea->rect.x0;
		position.z -= buildoutArea->rect.y0;
	}

	CachedBuildoutArea * const cachedBuildoutArea = loadBuildoutArea(*buildoutArea);
	if (!cachedBuildoutArea)
		return NULL;

	ConstCharCrcString const &sharedTemplateName = getSharedTemplateForServerTemplate(templateName);
	uint32 const portalLayoutCrc = getSharedTemplatePortalLayoutCrc(sharedTemplateName);
	int const cellCount = getSharedTemplateCellCount(sharedTemplateName);
	bool const serverOnly = !portalLayoutCrc && isServerOnlyTemplate(templateName.getCrc());
	float const updateRadius = getServerTemplateUpdateRadius(templateName);

	FATAL( cellCount > 0 && cellIndex > 0, ( "It looks like you're trying to add a cell to a cell." ) );


	// determine new id to use
	int newId = Random::random() | 0x80000000; // <- the |0x80000000 thing forces it to be a negative integer

	{
		std::vector<ServerBuildoutAreaRow>::iterator rowIter = cachedBuildoutArea->serverRows.end();

		if (cellIndex != 0)
		{
			rowIter = cachedBuildoutArea->serverRows.begin();
			
			while (rowIter != cachedBuildoutArea->serverRows.end() && (*rowIter).id != static_cast< int >( container->getNetworkId().getValue()) )
				++rowIter;

			while (rowIter != cachedBuildoutArea->serverRows.end() && (*rowIter).cellIndex)
				++rowIter;
		}

		ServerBuildoutAreaRow &serverRow = *(cachedBuildoutArea->serverRows.insert(rowIter));

		serverRow.id = newId;
		serverRow.container = container ? container->getNetworkId().getValue() : 0;
		serverRow.serverTemplateCrc = templateName.getCrc();
		serverRow.cellIndex = cellIndex;
		serverRow.position = position;
		serverRow.orientation = orientation;
		if (portalLayoutCrc)
		{
			char buf[128];
			IGNORE_RETURN(snprintf(buf, sizeof(buf)-1, "portalProperty.crc|0|%d|$|", portalLayoutCrc));
			buf[sizeof(buf)-1] = '\0';
			serverRow.objvars = buf;
		}
		else
			serverRow.objvars = "$|";

		for (int cell = 0; cell < cellCount; ++cell)
		{
			ServerBuildoutAreaRow &serverRow = *( cachedBuildoutArea->serverRows.insert( cachedBuildoutArea->serverRows.end() ) );

			serverRow.id = newId+cell+1;
			serverRow.container = newId;
			serverRow.serverTemplateCrc = s_cellServerTemplateName.getCrc();
			serverRow.cellIndex = cell+1;
		}
	}

	if (!serverOnly)
	{
		std::vector<ClientBuildoutAreaRow>::iterator rowIter = cachedBuildoutArea->clientRows.end();
		if (cellIndex != 0)
		{
			rowIter = cachedBuildoutArea->clientRows.begin();
			while (rowIter != cachedBuildoutArea->clientRows.end() && (*rowIter).id != container->getNetworkId().getValue())
				++rowIter;
			while (rowIter != cachedBuildoutArea->clientRows.end() && (*rowIter).cellIndex)
				++rowIter;
		}
		ClientBuildoutAreaRow &clientRow = *(cachedBuildoutArea->clientRows.insert(rowIter));

		clientRow.id = newId;
		clientRow.container = container ? container->getNetworkId().getValue() : 0;
		clientRow.sharedTemplateCrc = sharedTemplateName.getCrc();
		clientRow.cellIndex = cellIndex;
		clientRow.position = position;
		clientRow.orientation = orientation;
		clientRow.radius = updateRadius;
		clientRow.portalLayoutCrc = portalLayoutCrc;

		for (int cell = 0; cell < cellCount; ++cell)
		{
			cachedBuildoutArea->clientRows.push_back();
			ClientBuildoutAreaRow &clientRow = cachedBuildoutArea->clientRows.back();

			clientRow.id = newId+cell+1;
			clientRow.container = newId;
			clientRow.sharedTemplateCrc = s_cellSharedTemplateName.getCrc();
			clientRow.cellIndex = cell+1;
			clientRow.radius = 0;
			clientRow.portalLayoutCrc = 0;
		}
	}

	NetworkId networkId( BuildoutWorldId( newId, buildoutArea->areaIndex )  );

	s_areaIdMap[ networkId ] = buildoutArea;

	Object* object = WorldSnapshot::addObject(
		networkId.getValue(),
		container ? container->getNetworkId().getValue() : 0,
		sharedTemplateName,
		transform_p,
		updateRadius,
		portalLayoutCrc,
		cellCount);

	BuildoutAreaSupportNamespace::unlock(newId);
	
	return object;
}

// ----------------------------------------------------------------------

void BuildoutAreaSupport::unlock(Object const &obj)
{
	BuildoutArea const * const buildoutArea = getBuildoutAreaForId(obj.getNetworkId());

	if (!buildoutArea)
		return;

	CachedBuildoutArea * const cachedBuildoutArea = loadBuildoutArea(*buildoutArea);
	if (!cachedBuildoutArea)
		return;

	BuildoutAreaSupportNamespace::unlock(obj.getNetworkId().getValue());
}

// ----------------------------------------------------------------------

void BuildoutAreaSupport::unlockAll(bool value)
{
	s_unlockAll = value;
}

// ----------------------------------------------------------------------

void BuildoutAreaSupport::unlockNonStructures(bool value)
{
	s_unlockNonStructures = value;
}

// ----------------------------------------------------------------------

void BuildoutAreaSupport::unlockServerOnly(bool value)
{
	s_unlockServerOnly = value;
}

// ----------------------------------------------------------------------

bool BuildoutAreaSupport::deleteBuildoutObject(Object &obj)
{
	BuildoutArea const * const buildoutArea = getBuildoutAreaForId(obj.getNetworkId());

	if (!buildoutArea)
		return false;

	CachedBuildoutArea * const cachedBuildoutArea = loadBuildoutArea(*buildoutArea);
	if (!cachedBuildoutArea)
		return false;

	if (isLocked(*buildoutArea, obj.getNetworkId().getValue()))
		return false;

	bool deleted = false;

	{
		for (std::vector<ServerBuildoutAreaRow>::iterator i = cachedBuildoutArea->serverRows.begin(); i != cachedBuildoutArea->serverRows.end(); ++i)
		{
			const int buildoutId = static_cast< int > ( obj.getNetworkId().getValue() );

			if ((*i).id == buildoutId )
			{
				WorldSnapshot::removeObject((*i).id);
				s_areaIdMap.erase( obj.getNetworkId() );

				// If we're deleting a building, we need to delete its contents too
				if ((*i).cellIndex == 0)
				{
					std::vector<ServerBuildoutAreaRow>::iterator j = i;
					++j;
					while (j != cachedBuildoutArea->serverRows.end() && (*j).cellIndex)
					{
						WorldSnapshot::removeObject((*j).id);
						j = cachedBuildoutArea->serverRows.erase(j);

						// remove object from area id map
						s_areaIdMap.erase( BuildoutWorldId( (*j).id, buildoutArea->areaIndex ) );
					}
				}
				// delete the object
				IGNORE_RETURN(cachedBuildoutArea->serverRows.erase(i));
				deleted = true;
				break;
			}
		}
	}

	{
		for (std::vector<ClientBuildoutAreaRow>::iterator i = cachedBuildoutArea->clientRows.begin(); i != cachedBuildoutArea->clientRows.end(); ++i)
		{
			const int buildoutId = static_cast< int > ( obj.getNetworkId().getValue() );

			if ((*i).id == buildoutId )
			{
				// If we're deleting a building, we need to delete its contents too
				if ((*i).cellIndex == 0)
				{
					std::vector<ClientBuildoutAreaRow>::iterator j = i;
					++j;
					while (j != cachedBuildoutArea->clientRows.end() && (*j).cellIndex)
					{
						j = cachedBuildoutArea->clientRows.erase(j);
					}
				}
				// delete the object
				IGNORE_RETURN(cachedBuildoutArea->clientRows.erase(i));
				break;
			}
		}
	}

	return deleted;
}

// ----------------------------------------------------------------------

bool BuildoutAreaSupport::setObjectTransform(Object &obj, Transform const &transform_p)
{
	// Objects in cells may change position arbitrarily.
	// Objects in the world cell must remain in their current buildout area.

	BuildoutArea const * const buildoutArea = getBuildoutAreaForId(obj.getNetworkId());

	if (!buildoutArea)
		return false;

	CachedBuildoutArea * const cachedBuildoutArea = loadBuildoutArea(*buildoutArea);
	if (!cachedBuildoutArea)
		return false;

	if (isLocked(*buildoutArea, obj.getNetworkId().getValue()))
		return false;

	bool movedObject = false;

	{
		for (std::vector<ServerBuildoutAreaRow>::iterator i = cachedBuildoutArea->serverRows.begin(); i != cachedBuildoutArea->serverRows.end(); ++i)
		{
			const int buildoutId = static_cast< int > ( obj.getNetworkId().getValue() );

			if ((*i).id ==  buildoutId )
			{
				if ((*i).cellIndex)
				{
					(*i).orientation = Quaternion(transform_p);
					(*i).position.set(
						transform_p.getPosition_p().x,
						transform_p.getPosition_p().y,
						transform_p.getPosition_p().z);
					movedObject = true;
				}
				else if (SharedBuildoutAreaManager::findBuildoutAreaAtPosition(transform_p.getPosition_p(), false) == buildoutArea)
				{
					(*i).orientation = Quaternion(transform_p);
					(*i).position.set(
						transform_p.getPosition_p().x-buildoutArea->rect.x0,
						transform_p.getPosition_p().y,
						transform_p.getPosition_p().z-buildoutArea->rect.y0);
					movedObject = true;
				}
				break;
			}
		}
	}

	{
		for (std::vector<ClientBuildoutAreaRow>::iterator i = cachedBuildoutArea->clientRows.begin(); i != cachedBuildoutArea->clientRows.end(); ++i)
		{
			const int buildoutId = static_cast< int > ( obj.getNetworkId().getValue() );

			if ((*i).id == buildoutId )
			{
				if ((*i).cellIndex)
				{
					(*i).orientation = Quaternion(transform_p);
					(*i).position.set(
						transform_p.getPosition_p().x,
						transform_p.getPosition_p().y,
						transform_p.getPosition_p().z);
				}
				else if (SharedBuildoutAreaManager::findBuildoutAreaAtPosition(transform_p.getPosition_p(), false) == buildoutArea)
				{
					(*i).orientation = Quaternion(transform_p);
					(*i).position.set(
						transform_p.getPosition_p().x-buildoutArea->rect.x0,
						transform_p.getPosition_p().y,
						transform_p.getPosition_p().z-buildoutArea->rect.y0);
				}
				break;
			}
		}
	}

	if (movedObject)
	{
		WorldSnapshot::moveObject(static_cast<int>(obj.getNetworkId().getValue()), transform_p);
		BuildoutAreaSupportNamespace::unlock(obj.getNetworkId().getValue());
	}

	return movedObject;
}

// ----------------------------------------------------------------------

void BuildoutAreaSupport::attachScript(Object const &obj, std::string const &scriptName)
{
	if (scriptName.empty())
		return;

	BuildoutArea const * const buildoutArea = getBuildoutAreaForId(obj.getNetworkId());

	if (!buildoutArea)
		return;

	CachedBuildoutArea * const cachedBuildoutArea = loadBuildoutArea(*buildoutArea);
	if (!cachedBuildoutArea)
		return;

	if (isLocked(*buildoutArea, obj.getNetworkId().getValue()))
		return;

	for (std::vector<ServerBuildoutAreaRow>::iterator i = cachedBuildoutArea->serverRows.begin(); i != cachedBuildoutArea->serverRows.end(); ++i)
	{
		const int buildoutId = static_cast< int > ( obj.getNetworkId().getValue() );

		if ((*i).id == buildoutId )
		{
			std::vector<std::string> scriptList;
			unpackScriptList((*i).scripts, scriptList);
			if (std::find(scriptList.begin(), scriptList.end(), scriptName) == scriptList.end())
			{
				if (!(*i).scripts.empty())
					(*i).scripts += ':';
				(*i).scripts += scriptName;
				BuildoutAreaSupportNamespace::unlock(obj.getNetworkId().getValue());
			}
		}
	}
}

// ----------------------------------------------------------------------

void BuildoutAreaSupport::detachScript(Object const &obj, std::string const &scriptName)
{
	BuildoutArea const * const buildoutArea = getBuildoutAreaForId(obj.getNetworkId());

	if (!buildoutArea)
		return;

	CachedBuildoutArea * const cachedBuildoutArea = loadBuildoutArea(*buildoutArea);
	if (!cachedBuildoutArea)
		return;

	if (isLocked(*buildoutArea, obj.getNetworkId().getValue()))
		return;

	for (std::vector<ServerBuildoutAreaRow>::iterator i = cachedBuildoutArea->serverRows.begin(); i != cachedBuildoutArea->serverRows.end(); ++i)
	{
		const int buildoutId = static_cast< int > ( obj.getNetworkId().getValue() );

		if ((*i).id == buildoutId )
		{
			std::vector<std::string> scriptList;
			unpackScriptList((*i).scripts, scriptList);
			std::vector<std::string>::iterator f = std::find(scriptList.begin(), scriptList.end(), scriptName);
			if (f != scriptList.end())
			{
				scriptList.erase(f);
				(*i).scripts.clear();
				for (f = scriptList.begin(); f != scriptList.end(); ++f)
				{
					if (f != scriptList.begin())
						(*i).scripts += ':';
					(*i).scripts += *f;
				}
				BuildoutAreaSupportNamespace::unlock(obj.getNetworkId().getValue());
			}			
		}
	}
}

// ----------------------------------------------------------------------

void BuildoutAreaSupport::setObjvar(Object const &obj, std::string const &objvarName, int value)
{
	char buf[1024];
	IGNORE_RETURN(snprintf(buf, sizeof(buf)-1, "%s int %d", objvarName.c_str(), value));
	buf[sizeof(buf)-1] = '\0';
	BuildoutAreaSupportNamespace::setObjvar(obj, buf);
}

// ----------------------------------------------------------------------

void BuildoutAreaSupport::setObjvar(Object const &obj, std::string const &objvarName, float value)
{
	char buf[1024];
	IGNORE_RETURN(snprintf(buf, sizeof(buf)-1, "%s float %g", objvarName.c_str(), value));
	buf[sizeof(buf)-1] = '\0';
	BuildoutAreaSupportNamespace::setObjvar(obj, buf);
}

// ----------------------------------------------------------------------

void BuildoutAreaSupport::setObjvar(Object const &obj, std::string const &objvarName, std::string const &value)
{
	char buf[4096];
	IGNORE_RETURN(snprintf(buf, sizeof(buf)-1, "%s string %s", objvarName.c_str(), value.c_str()));
	buf[sizeof(buf)-1] = '\0';
	BuildoutAreaSupportNamespace::setObjvar(obj, buf);
}

// ----------------------------------------------------------------------

void BuildoutAreaSupport::removeObjvar(Object const &obj, std::string const &objvarName)
{
	BuildoutArea const * const buildoutArea = getBuildoutAreaForId(obj.getNetworkId());

	if (!buildoutArea)
		return;

	CachedBuildoutArea * const cachedBuildoutArea = loadBuildoutArea(*buildoutArea);
	if (!cachedBuildoutArea)
		return;

	if (isLocked(*buildoutArea, obj.getNetworkId().getValue()))
		return;

	for (std::vector<ServerBuildoutAreaRow>::iterator i = cachedBuildoutArea->serverRows.begin(); i != cachedBuildoutArea->serverRows.end(); ++i)
	{
		const int buildoutId = static_cast< int > ( obj.getNetworkId().getValue() );

		if ((*i).id == buildoutId )
		{
			std::vector<std::string> objvarList;
			unpackObjvarList((*i).objvars, objvarList);

			bool changed = false;

			// find matching objvar and erase it from the list
			{
				for (std::vector<std::string>::iterator f = objvarList.begin(); f != objvarList.end(); ++f)
				{
					std::string tempName;
					size_t endpos = 0;
					IGNORE_RETURN(Unicode::getFirstToken(*f, 0, endpos, tempName));
					if (tempName == objvarName)
					{
						objvarList.erase(f);
						changed = true;
						break;
					}
				}
			}

			// rebuild the packed list if changes have been made
			if (changed)
			{
				(*i).objvars = packObjvarList(objvarList);
				BuildoutAreaSupportNamespace::unlock(obj.getNetworkId().getValue());
			}
		}
	}
}

// ----------------------------------------------------------------------

void BuildoutAreaSupport::populateServerObjectData(NetworkId const &networkId)
{
	BuildoutArea const * const buildoutArea = getBuildoutAreaForId(networkId);

	if (!buildoutArea)
		return;

	CachedBuildoutArea const * const cachedBuildoutArea = loadBuildoutArea(*buildoutArea);
	if (!cachedBuildoutArea)
		return;

	for (std::vector<ServerBuildoutAreaRow>::const_iterator i = cachedBuildoutArea->serverRows.begin(); i != cachedBuildoutArea->serverRows.end(); ++i)
	{
		const int buildoutId = static_cast< int > ( networkId.getValue() );

		if ((*i).id == buildoutId )
		{
			std::vector<std::string> scriptList;
			unpackScriptList((*i).scripts, scriptList);
			std::vector<std::string> objvarList;
			unpackObjvarList((*i).objvars, objvarList);

			ServerObjectData::getInstance().setObjectInfo(
				networkId,
				getServerTemplateName((*i).serverTemplateCrc).getString(),
				scriptList,
				objvarList);
		}
	}
}

// ----------------------------------------------------------------------

void BuildoutAreaSupport::addServerOnlyObjectsToWorldSnapshot()
{
	// Run through all areas for this scene, and insert server only objects into the world snapshot.
	// Note: this destroys any temporary modifications to areas because it needs to reference
	// multiple instances of the same area, so it needs to only be called for a scene change.

	s_buildoutAreaCacheMap.clear();

	SharedBuildoutAreaManager::load(Game::getSceneId().c_str());

	std::vector<BuildoutArea> const &buildoutAreas = SharedBuildoutAreaManager::getBuildoutAreasForCurrentScene();
	for (std::vector<BuildoutArea>::const_iterator areaIter = buildoutAreas.begin(); areaIter != buildoutAreas.end(); ++areaIter)
	{
		CachedBuildoutArea const * const cachedBuildoutArea = loadBuildoutArea(*areaIter);
		if (cachedBuildoutArea)
		{

			for (std::vector<ServerBuildoutAreaRow>::const_iterator i = cachedBuildoutArea->serverRows.begin(); i != cachedBuildoutArea->serverRows.end(); ++i)
			{
				if ( isServerOnlyTemplate( i->serverTemplateCrc ) )
				{
					ConstCharCrcString const &serverTemplateName = getServerTemplateName((*i).serverTemplateCrc);

					NetworkId containerId;
					
					if ( (*i).container )
					{
						containerId = BuildoutWorldId( (*i).container, areaIter->areaIndex );
					}

					Transform transform_p;
					(*i).orientation.getTransform(&transform_p);
					transform_p.setPosition_p(
						(*i).position.x + (containerId.isValid() ? 0 : (*areaIter).rect.x0),
						(*i).position.y,
						(*i).position.z + (containerId.isValid() ? 0 : (*areaIter).rect.y0));
					
					const NetworkId networkId( BuildoutWorldId( (*i).id, areaIter->areaIndex ) );

					IGNORE_RETURN
					(
						WorldSnapshot::addObject(
							networkId.getValue(),
							containerId.getValue(),
							getSharedTemplateForServerTemplate(serverTemplateName),
							transform_p,
							getServerTemplateUpdateRadius(serverTemplateName),
							0,
							0 )
					);
				}

			}
		}
		s_buildoutAreaCacheMap.clear();
	}
}

// ----------------------------------------------------------------------

void BuildoutAreaSupportNamespace::install()
{
	std::string depotPath, clientPath, localPath, result;

	if (ConfigGodClient::getConnectToPerforce())
	{

		if (!GodClientPerforce::getInstance().getFileMapping(".", depotPath, clientPath, localPath, result))
		{
			WARNING(true, ("BuildoutAreaSupportNamespace::install(): %s", result.c_str()));
			IGNORE_RETURN(QMessageBox::warning(0, "Perforce Buildout Warning", result.c_str()));
			return;
		}

		std::string prefix("//depot/swg/");
		prefix += depotPath.substr(12, depotPath.find("/", 12)-12);

		GodClientPerforce::getInstance().getFileMapping(
			prefix + "/dsrc/sku.0/sys.server/compiled/game",
			s_serverSrcDepotPath, clientPath, s_serverSrcPath, result);
		GodClientPerforce::getInstance().getFileMapping(
			prefix + "/data/sku.0/sys.server/compiled/game",
			depotPath, clientPath, s_serverDataPath, result);
		GodClientPerforce::getInstance().getFileMapping(
			prefix + "/dsrc/sku.0/sys.client/compiled/game",
			depotPath, clientPath, s_clientSrcPath, result);
		GodClientPerforce::getInstance().getFileMapping(
			prefix + "/data/sku.0/sys.client/compiled/game",
			depotPath, clientPath, s_clientDataPath, result);
		GodClientPerforce::getInstance().getFileMapping(
			prefix + "/data/sku.0/sys.server/built/game/misc/object_template_crc_string_table.iff",
			depotPath, clientPath, localPath, result);
		s_serverTemplateCrcStringTable.load(localPath.c_str());
	}
	else
	{
		s_clientSrcPath = ConfigGodClient::getData().localClientSrcPath;
		s_clientDataPath = ConfigGodClient::getData().localClientDataPath;
		s_serverSrcPath = ConfigGodClient::getData().localServerSrcPath;
		s_serverDataPath = ConfigGodClient::getData().localServerDataPath;
		
		s_serverSrcDepotPath = "";
		
		s_serverTemplateCrcStringTable.load(ConfigGodClient::getData().localServerCrcStringTable);
	}


	s_installed = true;
}

// ----------------------------------------------------------------------

void BuildoutAreaSupportNamespace::getBuildoutAreaTableNames(std::string const &areaName, std::string &serverTabFilename, std::string &serverIffFilename, std::string &clientTabFilename, std::string &clientIffFilename)
{
	if (!s_installed)
		install();

	std::string buildoutPath("\\datatables\\buildout\\");
	buildoutPath += Game::getSceneId();
	buildoutPath += '\\';
	buildoutPath += areaName;

	serverTabFilename = s_serverSrcPath + buildoutPath + ".tab";
	serverIffFilename = s_serverDataPath + buildoutPath + ".iff";
	clientTabFilename = s_clientSrcPath + buildoutPath + ".tab";
	clientIffFilename = s_clientDataPath + buildoutPath + ".iff";
}

// ----------------------------------------------------------------------

void BuildoutAreaSupportNamespace::createDirectoriesForFiles(std::vector<std::string> const &fileList)
{
	for (std::vector<std::string>::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
		Os::createDirectories((*i).substr(0, (*i).rfind('\\')).c_str());
}

// ----------------------------------------------------------------------

void BuildoutAreaSupportNamespace::preModifyBuildoutArea(std::string const &serverTabFilename, std::string const &serverIffFilename, std::string const &clientTabFilename, std::string const &clientIffFilename)
{
	std::vector<std::string> perforceFiles;
	perforceFiles.push_back(serverTabFilename);
	perforceFiles.push_back(serverIffFilename);
	perforceFiles.push_back(clientTabFilename);
	perforceFiles.push_back(clientIffFilename);

	// create the directories to contain the files as needed
	createDirectoriesForFiles(perforceFiles);

	// open the files for edit, in case they already exist
	if (ConfigGodClient::getConnectToPerforce())
	{
		std::string result;
		IGNORE_RETURN(GodClientPerforce::getInstance().editFiles(perforceFiles, result));
	}
}

// ----------------------------------------------------------------------

void BuildoutAreaSupportNamespace::postModifyBuildoutArea(std::string const &serverTabFilename, std::string const &serverIffFilename, std::string const &clientTabFilename, std::string const &clientIffFilename)
{
	// compile the tables
	{
		DataTableWriter writer;
		writer.loadFromSpreadsheet(serverTabFilename.c_str());
		writer.save(serverIffFilename.c_str());
	}
	{
		DataTableWriter writer;
		writer.loadFromSpreadsheet(clientTabFilename.c_str());
		writer.save(clientIffFilename.c_str());
	}

	if (ConfigGodClient::getConnectToPerforce())
	{
		std::vector<std::string> perforceFiles;
		perforceFiles.push_back(serverTabFilename);
		perforceFiles.push_back(serverIffFilename);
		perforceFiles.push_back(clientTabFilename);
		perforceFiles.push_back(clientIffFilename);

		std::string result;
		// add the files, in case they didn't already exist
		if (!GodClientPerforce::getInstance().addFiles(perforceFiles, result))
		{
			const std::string msg = "Could not open file(s) for perforce add.\n" + result;
			IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		}
		
		// revert unchanged
		if (!GodClientPerforce::getInstance().revertFiles(perforceFiles, true, result))
		{
			const std::string msg = "Could not revert unchanged file(s).\n" + result;
			IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
		}
	}
	
}

// ----------------------------------------------------------------------

BuildoutArea const *BuildoutAreaSupportNamespace::getBuildoutAreaForId(NetworkId const &id)
{
	SharedBuildoutAreaManager::load(Game::getSceneId().c_str());


	// yeah find the object in the object map

	AreaIdMap::const_iterator it = s_areaIdMap.find( id );

	return it == s_areaIdMap.end() ? 0 : it->second;
}

// ----------------------------------------------------------------------

CachedBuildoutArea *BuildoutAreaSupportNamespace::loadBuildoutArea(BuildoutArea const &buildoutArea)
{
	std::string serverTabFilename, serverIffFilename, clientTabFilename, clientIffFilename;
	getBuildoutAreaTableNames(buildoutArea.areaName, serverTabFilename, serverIffFilename, clientTabFilename, clientIffFilename);

	BuildoutAreaCacheMap::iterator i = s_buildoutAreaCacheMap.find(serverTabFilename);
	if (i != s_buildoutAreaCacheMap.end())
	{
		CachedBuildoutArea &cachedBuildoutArea = (*i).second;
		return &cachedBuildoutArea;
	}

	// Not in the cache, so load it and put it in the cache
	CachedBuildoutArea &cachedBuildoutArea = s_buildoutAreaCacheMap[serverTabFilename];

	Iff iff;

	if (iff.open(serverIffFilename.c_str(), true))
	{
		DataTable areaBuildoutTable;
		areaBuildoutTable.load(iff);

		int const buildoutRowCount = areaBuildoutTable.getNumRows();
		if (buildoutRowCount > 0)
		{
			int const objIdColumn             = areaBuildoutTable.findColumnNumber( "objid" );
			int const containerColumn         = areaBuildoutTable.findColumnNumber( "container" );
			int const serverTemplateCrcColumn = areaBuildoutTable.findColumnNumber("server_template_crc");
			int const cellIndexColumn         = areaBuildoutTable.findColumnNumber("cell_index");
			int const pxColumn                = areaBuildoutTable.findColumnNumber("px");
			int const pyColumn                = areaBuildoutTable.findColumnNumber("py");
			int const pzColumn                = areaBuildoutTable.findColumnNumber("pz");
			int const qwColumn                = areaBuildoutTable.findColumnNumber("qw");
			int const qxColumn                = areaBuildoutTable.findColumnNumber("qx");
			int const qyColumn                = areaBuildoutTable.findColumnNumber("qy");
			int const qzColumn                = areaBuildoutTable.findColumnNumber("qz");
			int const scriptsColumn           = areaBuildoutTable.findColumnNumber("scripts");
			int const objvarsColumn           = areaBuildoutTable.findColumnNumber("objvars");

			FATAL(serverTemplateCrcColumn < 0, ("Unable to find serverTemplateCrcColumn in [%s]", serverIffFilename.c_str()));
			FATAL(cellIndexColumn < 0, ("Unable to find cellIndexColumn in [%s]", serverIffFilename.c_str()));
			FATAL(pxColumn < 0, ("Unable to find pxColumn in [%s]", serverIffFilename.c_str()));
			FATAL(pyColumn < 0, ("Unable to find pyColumn in [%s]", serverIffFilename.c_str()));
			FATAL(pzColumn < 0, ("Unable to find pzColumn in [%s]", serverIffFilename.c_str()));
			FATAL(qwColumn < 0, ("Unable to find qwColumn in [%s]", serverIffFilename.c_str()));
			FATAL(qxColumn < 0, ("Unable to find qxColumn in [%s]", serverIffFilename.c_str()));
			FATAL(qyColumn < 0, ("Unable to find qyColumn in [%s]", serverIffFilename.c_str()));
			FATAL(qzColumn < 0, ("Unable to find qzColumn in [%s]", serverIffFilename.c_str()));
			FATAL(scriptsColumn < 0, ("Unable to find scriptsColumn in [%s]", serverIffFilename.c_str()));
			FATAL(objvarsColumn < 0, ("Unable to find objvarsColumn in [%s]", serverIffFilename.c_str()));
            int buildingObjId = buildoutArea.getSharedBaseId();
            int objIdBase     = buildingObjId+2000;
            int serverOnlyId  = buildoutArea.getServerBaseId();
			int buildingId    = 0;
			int cellId        = 0;

			int buildOutFileVersion = 1;

			if ( objIdColumn != -1 )
			{
				buildOutFileVersion = 2;
			}

			for (int buildoutRow = 0; buildoutRow < buildoutRowCount; ++buildoutRow)
			{
				cachedBuildoutArea.serverRows.push_back();
				ServerBuildoutAreaRow &serverBuildoutAreaRow = cachedBuildoutArea.serverRows.back();


				serverBuildoutAreaRow.serverTemplateCrc = static_cast<uint32>(areaBuildoutTable.getIntValue(serverTemplateCrcColumn, buildoutRow));
				serverBuildoutAreaRow.cellIndex = areaBuildoutTable.getIntValue(cellIndexColumn, buildoutRow);
				serverBuildoutAreaRow.position.set(
					areaBuildoutTable.getFloatValue(pxColumn, buildoutRow),
					areaBuildoutTable.getFloatValue(pyColumn, buildoutRow),
					areaBuildoutTable.getFloatValue(pzColumn, buildoutRow));
				serverBuildoutAreaRow.orientation = Quaternion(
					areaBuildoutTable.getFloatValue(qwColumn, buildoutRow),
					areaBuildoutTable.getFloatValue(qxColumn, buildoutRow),
					areaBuildoutTable.getFloatValue(qyColumn, buildoutRow),
					areaBuildoutTable.getFloatValue(qzColumn, buildoutRow));
				serverBuildoutAreaRow.scripts = areaBuildoutTable.getStringValue(scriptsColumn, buildoutRow);
				serverBuildoutAreaRow.objvars = areaBuildoutTable.getStringValue(objvarsColumn, buildoutRow);
				ConstCharCrcString const &serverTemplateName = getServerTemplateName(serverBuildoutAreaRow.serverTemplateCrc);
				FATAL(!serverTemplateName.getString() || !*serverTemplateName.getString(), ("Unable to determine server template name for crc, crc=0x%08x, file=%s, row=%d", serverBuildoutAreaRow.serverTemplateCrc, serverIffFilename.c_str(), buildoutRow));
				ConstCharCrcString const &sharedTemplateName = getSharedTemplateForServerTemplate(serverTemplateName);

				int objId       = 0;
				int containerId = 0;

				// this code must match up with the obj ID allocation code in worldsnapshot.cpp
				if ( objIdColumn == -1 )
				{

					if ( getSharedTemplatePortalLayoutCrc( sharedTemplateName ) )
					{
						// is a building
						objId      = buildingObjId++;
						buildingId = objId;
					}
					else if ( serverBuildoutAreaRow.serverTemplateCrc == s_cellServerTemplateName.getCrc() )
					{
						// is a cell
						objId       = buildingObjId++;
						cellId      = objId;
						containerId = buildingId;
					}
  					else if ( serverBuildoutAreaRow.cellIndex > 0 )
   					{
  						// is contained in a cell ( but isn't a cell )
  						objId       = objIdBase++;
  						containerId = cellId;
  					}
  					else
					{
  						// world object
						if ( isServerOnlyTemplate( serverBuildoutAreaRow.serverTemplateCrc ) )
						{
							// server only object
							objId = serverOnlyId++;
						}
						else
						{
							// shared object
							objId = objIdBase++;
						}
					}

				}
				else
				{
  					objId       = areaBuildoutTable.getIntValue( objIdColumn, buildoutRow );
  					containerId = areaBuildoutTable.getIntValue( containerColumn, buildoutRow );
				}

  				s_areaIdMap[ BuildoutWorldId( objId, buildoutArea.areaIndex ) ] = &buildoutArea;

				serverBuildoutAreaRow.id        = objId;
				serverBuildoutAreaRow.container = containerId;
			}
		}
	}

	if (iff.open(clientIffFilename.c_str(), true))
	{
		DataTable areaBuildoutTable;
		areaBuildoutTable.load(iff);

		int const buildoutRowCount = areaBuildoutTable.getNumRows();
		if (buildoutRowCount > 0)
		{
			int const objIdColumn             = areaBuildoutTable.findColumnNumber( "objid" );
			int const containerColumn         = areaBuildoutTable.findColumnNumber( "container" );
			int const sharedTemplateCrcColumn = areaBuildoutTable.findColumnNumber("shared_template_crc");
			int const cellIndexColumn = areaBuildoutTable.findColumnNumber("cell_index");
			int const pxColumn = areaBuildoutTable.findColumnNumber("px");
			int const pyColumn = areaBuildoutTable.findColumnNumber("py");
			int const pzColumn = areaBuildoutTable.findColumnNumber("pz");
			int const qwColumn = areaBuildoutTable.findColumnNumber("qw");
			int const qxColumn = areaBuildoutTable.findColumnNumber("qx");
			int const qyColumn = areaBuildoutTable.findColumnNumber("qy");
			int const qzColumn = areaBuildoutTable.findColumnNumber("qz");
			int const radiusColumn = areaBuildoutTable.findColumnNumber("radius");
			int const portalLayoutCrcColumn = areaBuildoutTable.findColumnNumber("portal_layout_crc");
			int buildingObjId = buildoutArea.getSharedBaseId();
			int objIdBase     = buildingObjId+2000;
			int buildingId    = 0;
			int cellId        = 0;

			FATAL(sharedTemplateCrcColumn < 0, ("Unable to find sharedTemplateCrcColumn in [%s]", clientIffFilename.c_str()));
			FATAL(cellIndexColumn < 0, ("Unable to find cellIndexColumn in [%s]", clientIffFilename.c_str()));
			FATAL(pxColumn < 0, ("Unable to find pxColumn in [%s]", clientIffFilename.c_str()));
			FATAL(pyColumn < 0, ("Unable to find pyColumn in [%s]", clientIffFilename.c_str()));
			FATAL(pzColumn < 0, ("Unable to find pzColumn in [%s]", clientIffFilename.c_str()));
			FATAL(qwColumn < 0, ("Unable to find qwColumn in [%s]", clientIffFilename.c_str()));
			FATAL(qxColumn < 0, ("Unable to find qxColumn in [%s]", clientIffFilename.c_str()));
			FATAL(qyColumn < 0, ("Unable to find qyColumn in [%s]", clientIffFilename.c_str()));
			FATAL(qzColumn < 0, ("Unable to find qzColumn in [%s]", clientIffFilename.c_str()));
			FATAL(radiusColumn < 0, ("Unable to find radiusColumn in [%s]", clientIffFilename.c_str()));
			FATAL(portalLayoutCrcColumn < 0, ("Unable to find portalLayoutCrcColumn in [%s]", clientIffFilename.c_str()));

			

			for (int buildoutRow = 0; buildoutRow < buildoutRowCount; ++buildoutRow)
			{
				cachedBuildoutArea.clientRows.push_back();
				ClientBuildoutAreaRow &clientBuildoutAreaRow = cachedBuildoutArea.clientRows.back();

				clientBuildoutAreaRow.sharedTemplateCrc = static_cast<uint32>(areaBuildoutTable.getIntValue(sharedTemplateCrcColumn, buildoutRow));
				clientBuildoutAreaRow.cellIndex = areaBuildoutTable.getIntValue(cellIndexColumn, buildoutRow);
				clientBuildoutAreaRow.position.set(
					areaBuildoutTable.getFloatValue(pxColumn, buildoutRow),
					areaBuildoutTable.getFloatValue(pyColumn, buildoutRow),
					areaBuildoutTable.getFloatValue(pzColumn, buildoutRow));
				clientBuildoutAreaRow.orientation = Quaternion(
					areaBuildoutTable.getFloatValue(qwColumn, buildoutRow),
					areaBuildoutTable.getFloatValue(qxColumn, buildoutRow),
					areaBuildoutTable.getFloatValue(qyColumn, buildoutRow),
					areaBuildoutTable.getFloatValue(qzColumn, buildoutRow));
				clientBuildoutAreaRow.radius = areaBuildoutTable.getFloatValue(radiusColumn, buildoutRow);
				clientBuildoutAreaRow.portalLayoutCrc = areaBuildoutTable.getIntValue(portalLayoutCrcColumn, buildoutRow);

				int objId       = 0;
				int containerId = 0;

				// this code must match up with the obj ID allocation code in worldsnapshot.cpp
				if ( objIdColumn == -1 )
				{
					if ( clientBuildoutAreaRow.portalLayoutCrc )
					{
						// is a building
						objId      = buildingObjId++;
						buildingId = objId;
					}
					else if ( clientBuildoutAreaRow.sharedTemplateCrc == s_cellSharedTemplateName.getCrc() )
					{
						// is a cell
						objId       = buildingObjId++;
						cellId      = objId ;
						containerId = buildingId;
					}
					else if ( clientBuildoutAreaRow.cellIndex > 0 )
					{
						// is contained in a cell ( but isn't a cell )
						objId       = objIdBase++;
						containerId = cellId;
					}
					else
					{
						// world object
						objId = objIdBase++;
					}
				}
				else
				{
					objId       = areaBuildoutTable.getIntValue( objIdColumn, buildoutRow );
					containerId = areaBuildoutTable.getIntValue( containerColumn, buildoutRow );
				}

				clientBuildoutAreaRow.id        = objId;
				clientBuildoutAreaRow.container = containerId;

			}
		}
	}

	return &cachedBuildoutArea;
}

// ----------------------------------------------------------------------

bool BuildoutAreaSupportNamespace::isServerOnlyTemplate(uint32 serverTemplateCrc)
{
	// A template is server only if it derives from tangible and does not have the VF_player flag

	//////////////////////////////////////////////////////////////////////////
	// create a cache and use it
	typedef std::map< uint32, bool > Cache;
	static Cache cache;

	Cache::const_iterator it = cache.find( serverTemplateCrc);

	if ( it != cache.end() )
	{
		return it->second;
	}

	//////////////////////////////////////////////////////////////////////////
	// not in cache, so go parse the IFF file
	
	ConstCharCrcString const &serverTemplateName = getServerTemplateName(serverTemplateCrc);
	std::string templatePath(s_serverDataPath);
	templatePath += '\\';
	templatePath += serverTemplateName.getString();

	bool foundTangible = false;

	Iff iff;
	while (!templatePath.empty())
	{
		if (iff.open(templatePath.c_str()))
		{
			Tag t = iff.getCurrentName();
			while (iff.enterForm(true) && t != TAG(S,W,O,O))
			{
				if (t == TAG(T,A,N,O))
					foundTangible = true;

				// skip DERV
				iff.enterForm();
				iff.exitForm(true);
				// skip data
				iff.enterForm();
				iff.exitForm(true);
				t = iff.getCurrentName();
			}

			if (t == TAG(S,W,O,O))
			{
				// get DERV
				if (iff.enterForm(TAG(D,E,R,V), true))
				{
					iff.enterChunk();
						std::string const &baseTemplateName = iff.read_stdstring();
						if (baseTemplateName.empty())
							templatePath.clear();
						else
						{
							templatePath = s_serverDataPath;
							templatePath += '\\';
							templatePath += baseTemplateName;
						}
					iff.exitChunk();
					iff.exitForm();
				}
				else
					templatePath.clear();

				// get data, search for visible flags
				iff.enterForm();
					iff.enterChunk();
						int const paramCount = iff.read_int32();
					iff.exitChunk();
					for (int i = 0; i < paramCount; ++i)
					{
						if (!iff.enterChunk(TAG(X,X,X,X), true))
						{
							bool done = false;
							while (!done)
							{
								iff.enterForm();
								iff.exitForm(true);
								iff.enterChunk();
								done = iff.getChunkLengthTotal() == 0;
								iff.exitChunk(true);
							}
							iff.enterChunk(TAG(X,X,X,X));
						}

						std::string const &paramName = iff.read_stdstring();
						if (paramName == "visibleFlags")
						{
							IGNORE_RETURN(iff.read_bool8());
							bool found = false;
							// If we find a loaded value and aren't appending, we stop recursing into base templates
							int const listCount = iff.read_int32();
							for (int listIndex = 0; listIndex < listCount; ++listIndex)
							{
								IntegerParam value;
								value.loadFromIff(iff);
								// VF_player is set if there is a loaded value of 1 in the list
								if (value.isLoaded())
								{
									found = true;

									if (value.getMaxValue() == 1)
									{
										cache[ serverTemplateCrc ] = false;
										return false;
									}
								}
							}
							if (found && foundTangible)
							{
								cache[ serverTemplateCrc ] = true;
								return true;
							}
						}
						iff.exitChunk(true);
					}
				iff.exitForm();
			}
		}
	}

	cache[ serverTemplateCrc ] = foundTangible;

	return foundTangible;
}

// ----------------------------------------------------------------------

float BuildoutAreaSupportNamespace::getServerTemplateUpdateRadius(CrcString const &serverTemplateName)
{
	std::string templatePath(s_serverDataPath);
	templatePath += '\\';
	templatePath += serverTemplateName.getString();

	Iff iff;
	while (!templatePath.empty())
	{
		if (iff.open(templatePath.c_str()))
		{
			Tag t = iff.getCurrentName();
			while (iff.enterForm(true) && t != TAG(S,W,O,O))
			{
				// skip DERV
				iff.enterForm();
				iff.exitForm(true);
				// skip data
				iff.enterForm();
				iff.exitForm(true);
				t = iff.getCurrentName();
			}

			if (t == TAG(S,W,O,O))
			{
				// get DERV
				if (iff.enterForm(TAG(D,E,R,V), true))
				{
					iff.enterChunk();
						std::string const &baseTemplateName = iff.read_stdstring();
						if (baseTemplateName.empty())
							templatePath.clear();
						else
						{
							templatePath = s_serverDataPath;
							templatePath += '\\';
							templatePath += baseTemplateName;
						}
					iff.exitChunk();
					iff.exitForm();
				}
				else
					templatePath.clear();

				// get data, search for update radius
				iff.enterForm();
					iff.enterChunk();
						int const paramCount = iff.read_int32();
					iff.exitChunk();
					for (int i = 0; i < paramCount; ++i)
					{
						if (!iff.enterChunk(TAG(X,X,X,X), true))
						{
							bool done = false;
							while (!done)
							{
								iff.enterForm();
								iff.exitForm(true);
								iff.enterChunk();
								done = iff.getChunkLengthTotal() == 0;
								iff.exitChunk(true);
							}
							iff.enterChunk(TAG(X,X,X,X));
						}

						std::string const &paramName = iff.read_stdstring();
						if (paramName == "updateRanges")
						{
							int const listCount = iff.read_int32();
							for (int listIndex = 0; listIndex < listCount; ++listIndex)
							{
								FloatParam value;
								value.loadFromIff(iff);
								if (listIndex == 2 && value.isLoaded()) // UR_Far
									return value.getMaxValue();
							}
						}
						iff.exitChunk(true);
					}
				iff.exitForm();
			}
		}
	}

	return 512.f;
}

// ----------------------------------------------------------------------

uint32 BuildoutAreaSupportNamespace::getSharedTemplatePortalLayoutCrc(CrcString const &sharedTemplateName)
{
	SharedObjectTemplate const * const objectTemplate = safe_cast<SharedObjectTemplate const *>(NON_NULL(ObjectTemplateList::fetch(sharedTemplateName)));

	uint32 portalLayoutCrc = 0;
	IGNORE_RETURN(PortalPropertyTemplate::extractPortalLayoutCrc(objectTemplate->getPortalLayoutFilename().c_str(), portalLayoutCrc));

	objectTemplate->releaseReference();
	
	return portalLayoutCrc;
}

// ----------------------------------------------------------------------

ConstCharCrcString const BuildoutAreaSupportNamespace::getServerTemplateName(uint32 serverTemplateCrc)
{
	return s_serverTemplateCrcStringTable.lookUp(serverTemplateCrc);
}

// ----------------------------------------------------------------------

ConstCharCrcString const BuildoutAreaSupportNamespace::getSharedTemplateName(uint32 sharedTemplateCrc)
{
	return ObjectTemplateList::lookUp(sharedTemplateCrc);
}

// ----------------------------------------------------------------------

ConstCharCrcString const BuildoutAreaSupportNamespace::getSharedTemplateForServerTemplate(CrcString const &serverTemplateName)
{
	std::string templatePath(s_serverDataPath);
	templatePath += '\\';
	templatePath += serverTemplateName.getString();

	Iff iff;
	while (!templatePath.empty())
	{
		if (iff.open(templatePath.c_str()))
		{
			Tag t = iff.getCurrentName();
			while (iff.enterForm(true) && t != TAG(S,W,O,O))
			{
				// skip DERV
				iff.enterForm();
				iff.exitForm(true);
				// skip data
				iff.enterForm();
				iff.exitForm(true);
				t = iff.getCurrentName();
			}

			if (t == TAG(S,W,O,O))
			{
				// get DERV
				if (iff.enterForm(TAG(D,E,R,V), true))
				{
					iff.enterChunk();
						std::string const &baseTemplateName = iff.read_stdstring();
						if (baseTemplateName.empty())
							templatePath.clear();
						else
						{
							templatePath = s_serverDataPath;
							templatePath += '\\';
							templatePath += baseTemplateName;
						}
					iff.exitChunk();
					iff.exitForm();
				}
				else
					templatePath.clear();

				// get data, search for shared template
				iff.enterForm();
					iff.enterChunk();
						int const paramCount = iff.read_int32();
					iff.exitChunk();
					for (int i = 0; i < paramCount; ++i)
					{
						if (!iff.enterChunk(TAG(X,X,X,X), true))
						{
							bool done = false;
							while (!done)
							{
								iff.enterForm();
								iff.exitForm(true);
								iff.enterChunk();
								done = iff.getChunkLengthTotal() == 0;
								iff.exitChunk(true);
							}
							iff.enterChunk(TAG(X,X,X,X));
						}

						std::string const &paramName = iff.read_stdstring();
						if (paramName == "sharedTemplate")
						{
							StringParam value;
							value.loadFromIff(iff);
							if (value.isLoaded())
								return ObjectTemplateList::lookUp(value.getValue().c_str());
						}
						iff.exitChunk(true);
					}
				iff.exitForm();
			}
		}
	}
	
	FATAL(true, ("Unable to determine shared template for server template '%s'.", serverTemplateName.getString()));

	return ConstCharCrcString("");
}

// ----------------------------------------------------------------------

int BuildoutAreaSupportNamespace::getSharedTemplateCellCount(CrcString const &sharedTemplateName)
{
	int cellCount = 0;
	SharedObjectTemplate const * const objectTemplate = safe_cast<SharedObjectTemplate const *>(NON_NULL(ObjectTemplateList::fetch(sharedTemplateName)));

	Iff iff;
	if (iff.open(objectTemplate->getPortalLayoutFilename().c_str(), true))
	{
		iff.enterForm();
		iff.enterForm();
		iff.enterChunk();
		IGNORE_RETURN(iff.read_int32());
		cellCount = iff.read_int32()-1;
	}
	objectTemplate->releaseReference();
	return cellCount;
}

// ----------------------------------------------------------------------

void BuildoutAreaSupportNamespace::unpackScriptList(std::string const &packedScriptList, std::vector<std::string> &unpackedScriptList)
{
	if (!packedScriptList.empty())
	{
		unsigned int pos = 0;
		while (pos < packedScriptList.size())
		{
			unsigned int const oldPos = pos;
			while (packedScriptList[pos] && packedScriptList[pos] != ':')
				++pos;
			unpackedScriptList.push_back(packedScriptList.substr(oldPos, pos-oldPos));
			if (packedScriptList[pos] == ':')
				++pos;
		}
	}	
}

// ----------------------------------------------------------------------

void BuildoutAreaSupportNamespace::unpackObjvarList(std::string const &packedObjvarList, std::vector<std::string> &unpackedObjvarList)
{
	if (!packedObjvarList.empty())
	{
		unsigned int pos = 0;
		while (pos < packedObjvarList.size())
		{
			if (packedObjvarList[pos] == '$' && packedObjvarList[pos+1] == '|')
				break;
			// get objvar name
			unsigned int oldPos = pos;
			while (packedObjvarList[pos] && packedObjvarList[pos] != '|')
				++pos;
			std::string const &objvarName = packedObjvarList.substr(oldPos, pos-oldPos);
			if (packedObjvarList[pos] == '|')
				++pos;
			// get objvar type
			int const objvarType = atoi(packedObjvarList.c_str()+pos);
			FATAL(objvarType < 0 || objvarType >= sizeof(s_dynamicVariableTypeNames)/sizeof(s_dynamicVariableTypeNames[0]), ("Invalid objvar type %d", objvarType));
			while (packedObjvarList[pos] && packedObjvarList[pos] != '|')
				++pos;
			if (packedObjvarList[pos] == '|')
				++pos;
			// get objvar value
			oldPos = pos;
			while (packedObjvarList[pos] && packedObjvarList[pos] != '|')
				++pos;
			std::string const &objvarValue = packedObjvarList.substr(oldPos, pos-oldPos);
			if (packedObjvarList[pos] == '|')
				++pos;

			char buf[4096];
			IGNORE_RETURN(snprintf(buf, sizeof(buf)-1, "%s %s %s", objvarName.c_str(), s_dynamicVariableTypeNames[objvarType], objvarValue.c_str()));
			buf[sizeof(buf)-1] = '\0';
			unpackedObjvarList.push_back(std::string(buf));
		}
	}	
}

// ----------------------------------------------------------------------

std::string const BuildoutAreaSupportNamespace::packObjvarList(std::vector<std::string> const &objvarList)
{
	std::string result;

	for (std::vector<std::string>::const_iterator f = objvarList.begin(); f != objvarList.end(); ++f)
	{
		std::string tempName, tempType;
		size_t endpos = 0;

		IGNORE_RETURN(Unicode::getFirstToken(*f, 0, endpos, tempName));
		result += tempName;
		result += '|';

		IGNORE_RETURN(Unicode::getFirstToken(*f, endpos, endpos, tempType));
		int index = 0;
		for ( ; index < sizeof(s_dynamicVariableTypeNames)/sizeof(s_dynamicVariableTypeNames[0]); ++index)
			if (tempType == s_dynamicVariableTypeNames[index])
				break;
		FATAL(index == sizeof(s_dynamicVariableTypeNames)/sizeof(s_dynamicVariableTypeNames[0]), ("Invalid objvar type %s", tempName.c_str()));
		char buf[32];
		IGNORE_RETURN(snprintf(buf, sizeof(buf)-1, "%d|", index));
		buf[sizeof(buf)-1] = '\0';
		result += buf;

		if (endpos < (*f).size())
			++endpos;
		result += (*f).substr(endpos, static_cast<size_t>(std::string::npos));
		result += '|';
	}
	result += "$|";
	return result;
}

// ----------------------------------------------------------------------

void BuildoutAreaSupportNamespace::setObjvar(Object const &obj, std::string const &objvarSpec)
{
	BuildoutArea const * const buildoutArea = getBuildoutAreaForId(obj.getNetworkId());

	if (!buildoutArea)
		return;

	CachedBuildoutArea * const cachedBuildoutArea = loadBuildoutArea(*buildoutArea);
	if (!cachedBuildoutArea)
		return;

	if (isLocked(*buildoutArea, obj.getNetworkId().getValue()))
		return;

	size_t endpos = 0;
	std::string objvarName;
	IGNORE_RETURN(Unicode::getFirstToken(objvarSpec, 0, endpos, objvarName));

	for (std::vector<ServerBuildoutAreaRow>::iterator i = cachedBuildoutArea->serverRows.begin(); i != cachedBuildoutArea->serverRows.end(); ++i)
	{
		if ((*i).id == obj.getNetworkId().getValue())
		{
			std::vector<std::string> objvarList;
			unpackObjvarList((*i).objvars, objvarList);

			// find matching objvar and erase it from the list
			{
				for (std::vector<std::string>::iterator f = objvarList.begin(); f != objvarList.end(); ++f)
				{
					std::string tempName;
					IGNORE_RETURN(Unicode::getFirstToken(*f, 0, endpos, tempName));
					if (tempName == objvarName)
					{
						objvarList.erase(f);
						break;
					}
				}
			}

			// add the new objvar to the list
			objvarList.push_back(objvarSpec);

			// repack the modified list
			(*i).objvars = packObjvarList(objvarList);
			BuildoutAreaSupportNamespace::unlock(obj.getNetworkId().getValue());
			return;
		}
	}
}

// ----------------------------------------------------------------------

bool BuildoutAreaSupportNamespace::isLocked(BuildoutArea const &buildoutArea, int id)
{
	if (s_unlockAll)
		return false;

	if (s_unlockServerOnly && id >= buildoutArea.getServerBaseId() && id <= buildoutArea.getServerTopId())
		return false;

	if (   s_unlockNonStructures
	    && (   (id >= buildoutArea.getSharedBaseId()+5000 && id <= buildoutArea.getSharedTopId())
	        || (id >= buildoutArea.getServerBaseId() && id <= buildoutArea.getServerTopId())))
		return false;

	if (s_unlockedIds.count(id) > 0)
		return false;

	return true;
}

// ----------------------------------------------------------------------

void BuildoutAreaSupportNamespace::unlock(int id)
{
	IGNORE_RETURN(s_unlockedIds.insert(id));
}

// ----------------------------------------------------------------------

void BuildoutAreaSupportNamespace::writeOutServerEventArea(std::string const eventName, std::vector<std::string> const & objects)
{
	std::string buildoutPath("\\datatables\\buildout\\");
	buildoutPath += Game::getSceneId();
	buildoutPath += '\\';
	buildoutPath += Game::getSceneId();
	buildoutPath += '_';
	buildoutPath += eventName;

	std::string serverTabFilename = s_serverSrcPath + buildoutPath + ".tab";
	std::string serverIffFilename = s_serverDataPath + buildoutPath + ".iff";

	DEBUG_WARNING(true, ("Writing out Event Table [%s]", serverTabFilename.c_str()));

	std::vector<std::string> eventFiles;
	eventFiles.push_back(serverTabFilename);
	eventFiles.push_back(serverIffFilename);

	// create the directories to contain the files as needed
	createDirectoriesForFiles(eventFiles);

	// open the files for edit, in case they already exist
	if (ConfigGodClient::getConnectToPerforce())
	{
		std::string result;
		IGNORE_RETURN(GodClientPerforce::getInstance().editFiles(eventFiles, result));
	}

	StdioFile serverEventOutputFile(serverTabFilename.c_str(), "w");

	if (!serverEventOutputFile.isOpen())
	{
		std::string msg("File not writable: ");
		msg += serverTabFilename;
		IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
	}
	else
	{
		// save the table headers
		std::string const serverHeader(
			"objid\tcontainer\tserver_template_crc\tcell_index\tpx\tpy\tpz\tqw\tqx\tqy\tqz\tscripts\tobjvars\n"
			"i\ti\th\ti\tf\tf\tf\tf\tf\tf\tf\ts\tp\n");
		serverEventOutputFile.write(serverHeader.length(), serverHeader.c_str());

		// Write out the actually object info.
		std::vector<std::string>::size_type i = 0;
		for(; i < objects.size(); ++i)
			serverEventOutputFile.write(objects[i].length(), objects[i].c_str());

		serverEventOutputFile.close();

		// compile the tables
		{
			DataTableWriter writer;
			writer.loadFromSpreadsheet(serverTabFilename.c_str());
			writer.save(serverIffFilename.c_str());
		}


		if (ConfigGodClient::getConnectToPerforce())
		{
			std::vector<std::string> perforceFiles;
			perforceFiles.push_back(serverTabFilename);
			perforceFiles.push_back(serverIffFilename);

			std::string result;
			// add the files, in case they didn't already exist
			if (!GodClientPerforce::getInstance().addFiles(perforceFiles, result))
			{
				const std::string msg = "Could not open file(s) for perforce add.\n" + result;
				IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
			}

			// revert unchanged
			if (!GodClientPerforce::getInstance().revertFiles(perforceFiles, true, result))
			{
				const std::string msg = "Could not revert unchanged file(s).\n" + result;
				IGNORE_RETURN(QMessageBox::warning(&MainFrame::getInstance(), "Warning", msg.c_str()));
			}
		}

	}

}

// ----------------------------------------------------------------------

// ======================================================================
