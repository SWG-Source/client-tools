// ======================================================================
//
// DatabaseImporter.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

//precompiled header includes
#include "FirstMayaExporter.h"

#if !NO_DATABASE

//module include
#include "DatabaseImporter.h"

//engine shared includes
#include "sharedDatabaseInterface/DBServer.h"
#include "UnicodeUtils.h"

//local mayaExporter includes
#include "AlienbrainImporter.h"
#include "ExportManager.h"
#include "MayaExporterQuery.h"
#include "Messenger.h"
#include "PerforceImporter.h"
#include "PluginMain.h"
#include "Resource.h"
#include "SetDirectoryCommand.h"

//system / STL includes
#include <string>
#include <map>

//////////////////////////////////////////////////////////////////////////////////

//lint -save -e1925 // public symbols

/**
 * This class represents a parent-child relationship that we want to represent in the 
 * database.  We use an enter/exit node organization since it's more DB friendly.
 */
class DatabaseImporter::RelationshipNode
{
	public:
		explicit RelationshipNode(int value);
		~RelationshipNode();

	public:
		int m_entryNumber;
		int m_exitNumber;
		int m_nodeNumber;
		int m_parentEntryNumber;
		std::vector<RelationshipNode *> m_children;

	private:
	// disabled
	RelationshipNode();
	RelationshipNode(const RelationshipNode&);             //lint !e754 function not referenced (class declared in a cpp file, hence the issue)
	RelationshipNode &operator =(const RelationshipNode&); //lint !e754 "                                                                     "
};

//////////////////////////////////////////////////////////////////////////////////

Messenger                          *DatabaseImporter::messenger;
DB::Session                        *DatabaseImporter::ms_session;
DB::Server                         *DatabaseImporter::ms_server;
bool                                DatabaseImporter::ms_connected;
DatabaseImporter::RelationshipNode *DatabaseImporter::ms_relationshipTree;
DatabaseImporter::Categories        DatabaseImporter::ms_categoryCheck;
std::vector<std::pair<std::string, int> >  DatabaseImporter::ms_assetNames;
std::string                         DatabaseImporter::ms_assetName;
int                                 DatabaseImporter::ms_assetID;
bool                                DatabaseImporter::ms_activatedState;
std::map<std::string, std::string>  DatabaseImporter::ms_assetToFilenameMap;
std::map<std::string, std::string>  DatabaseImporter::ms_filenameToAssetMap;
std::string                         DatabaseImporter::ms_nodeName;
std::map<std::string, int>          DatabaseImporter::ms_statusAssetMap;

//these constants are used to log into the Oracle DB
namespace DatabaseImporterNamespace
{
	enum STATUS_IDS
	{
		DUMMY_ART = 1,
		REAL_ART = 2,
		LOCKED = 3
	};
}

using namespace DatabaseImporterNamespace;

//////////////////////////////////////////////////////////////////////////////////

DatabaseImporter::RelationshipNode::RelationshipNode(int value)
: m_entryNumber(value),
  m_exitNumber(value),
  m_nodeNumber(0),
  m_parentEntryNumber(0),
  m_children()
{}

//////////////////////////////////////////////////////////////////////////////////

DatabaseImporter::RelationshipNode::~RelationshipNode()
{
	//delete all our children
	for(std::vector<RelationshipNode *>::iterator it = m_children.begin(); it != m_children.end(); ++it)
		delete (*it);
	m_children.clear();
}

//////////////////////////////////////////////////////////////////////////////////
/**
 * Initialize the module
 */
void DatabaseImporter::install(Messenger* newMessenger)
{
	messenger    = newMessenger;
	ms_session   = NULL;
	ms_server    = NULL;
	ms_connected = false;
	//don't create a tree until we need it
	ms_relationshipTree = NULL;
	ms_assetID = 0;
	ms_activatedState = true;

	const std::string & dbActivated = SetDirectoryCommand::getDirectoryString(ASSET_DB_ACTIVATED_INDEX);

	if(dbActivated == "false")
		ms_activatedState = false;

	reset();
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Shutdown the module
 */
void DatabaseImporter::remove()
{
	if (ms_connected)
		IGNORE_RETURN(endSession());
	ms_connected = false;

	if(ms_server)
	{
		ms_server->releaseSession(ms_session);
		delete ms_server;
		ms_server = NULL;
		ms_session = NULL;
	}

	//reset data structures
	reset();

	messenger = NULL;
	ms_session = NULL;
}

//////////////////////////////////////////////////////////////////////////////////

/** Reset all the data structures
 */
void DatabaseImporter::reset()
{
	delete ms_relationshipTree;
	ms_relationshipTree = NULL;
	ms_categoryCheck = CAT_UNKNOWN;
	ms_assetNames.clear();
	ms_assetID = 0;
	ms_assetName.clear();
	ms_nodeName.clear();
	ms_assetToFilenameMap.clear();
	ms_filenameToAssetMap.clear();
	ms_statusAssetMap.clear();
}

//////////////////////////////////////////////////////////////////////////////////

void DatabaseImporter::activate(bool state, bool interactive)
{
	if((ms_activatedState && !state) || (!ms_activatedState && state))
	{
		ms_activatedState = state;
		if(ms_activatedState)
		{
			if(interactive)
				MESSENGER_LOG(("AssetDatabase connection activated.\n"));
			SetDirectoryCommand::setDirectoryString(ASSET_DB_ACTIVATED_INDEX, "true");

		}
		else
		{
			if(interactive)
				MESSENGER_LOG(("AssetDatabase connection deactivated.\n"));
			SetDirectoryCommand::setDirectoryString(ASSET_DB_ACTIVATED_INDEX, "false");
		}
	}
	else
		MESSENGER_LOG(("AssetDatabase connection unchanged.\n"));
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Make a connection to the asset database server, initialize it, and prepare for submissions into it
 */
bool DatabaseImporter::startSession()
{
	if(!ms_activatedState)
		return true;

	DEBUG_FATAL(ms_connected, ("DatabaseImporter already connected"));

	const std::string & DBName     = SetDirectoryCommand::getDirectoryString(ASSET_DB_NAME_DIR_INDEX);
	const std::string & DBUserName = SetDirectoryCommand::getDirectoryString(ASSET_DB_LOGINNAME_DIR_INDEX);
	const std::string & DBPassword = SetDirectoryCommand::getDirectoryString(ASSET_DB_PASSWORD_DIR_INDEX);

	//always FATAL on any db error
	DB::Server::setFatalOnError(true);

	if(!ms_server)
		ms_server = DB::Server::create(DBName.c_str(), 
		                               DBUserName.c_str(), 
		                               DBPassword.c_str(), 
		                               DB::PROTOCOL_OCI,
		                               false);

	if(!ms_session)
		ms_session = ms_server->getSession();
	ms_connected = true;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Close the active connection
 */
bool DatabaseImporter::endSession()
{
	if(!ms_activatedState)
		return true;

	DEBUG_FATAL(!ms_connected, ("DatabaseImporter not connected"));

	ms_connected = false;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Clear the tree relationships in the database
 */
bool DatabaseImporter::clearRelationships()
{
	NOT_NULL(ms_session);
	DBQuery::ClearRelationshipTreeQuery clearRelationshipTreeQuery;
	IGNORE_RETURN(clearRelationshipTreeQuery.setup(ms_session));
	clearRelationshipTreeQuery.m_appearanceNumber.setValue(ms_assetID);
	IGNORE_RETURN(clearRelationshipTreeQuery.exec());
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Submit the tree relationships to the database.  This tree data structure is built
 * during the update process, so this function must be called afterwards.
 * @pre all assets have been updated
 * @pre all previous asset relationships have been cleared with clearRelationships()
 * @post all asset relationships are in the database
 *
 */
bool DatabaseImporter::sendTreeRelationships(RelationshipNode *root)
{
	NOT_NULL(ms_session);
	if(!root)
	{
		return true;
	}
	DBQuery::UpdateRelationshipQuery updateRelationshipQuery;
	IGNORE_RETURN(updateRelationshipQuery.setup(ms_session));
	updateRelationshipQuery.m_appearanceNumber.setValue(ms_assetID);
	updateRelationshipQuery.m_nodeNumber.setValue(root->m_nodeNumber);
	updateRelationshipQuery.m_entryNumber.setValue(root->m_entryNumber);
	updateRelationshipQuery.m_exitNumber.setValue(root->m_exitNumber);
	//an item with no parent has a parent value of 0, mark it as NULL for the DB query
	if(root->m_parentEntryNumber == 0)
		updateRelationshipQuery.m_parentEntryNumber.setNull();
	else
		updateRelationshipQuery.m_parentEntryNumber.setValue(root->m_parentEntryNumber);

	IGNORE_RETURN(updateRelationshipQuery.exec());

	bool success = true;
	for(std::vector<RelationshipNode *>::iterator it = root->m_children.begin(); it != root->m_children.end(); ++it)
	{
		if(!sendTreeRelationships(*it))
			success = false;
	}
	return success;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Import the assets related to a static mesh into the asset database.  This function requires that
 * the ExporterLog has already been populated with data from the current export.
 */
bool DatabaseImporter::importStaticMeshData(bool interactive, bool locking, bool unlocking)
{
	if(!ms_activatedState)
		return true;

	//don't submit anything if the user chose the "none" category
	if (userChoseOther())
		return true;

	MESSENGER_LOG(("====== BEGINNING IMPORT TO THE ASSET DATABASE ======\n"));

	std::vector<ExporterLog::MeshLogInfo *> meshList;
	std::vector<ExporterLog::ComponentLogInfo *> componentList;
	std::vector<ExporterLog::LODLogInfo *> lodList;

	//get each type of node list
	ExporterLog::ms_objects->filter(meshList);
	ExporterLog::ms_objects->filter(componentList);
	ExporterLog::ms_objects->filter(lodList);

	bool success = true;

	for(std::vector<ExporterLog::MeshLogInfo *>::iterator meshIt = meshList.begin(); meshIt != meshList.end(); ++meshIt)
	{
		//tree should be empty at this point
		DEBUG_FATAL(ms_relationshipTree, ("building off a created tree"));
		if(!updateStaticMesh(*meshIt, ms_relationshipTree))
			success = false;
	}

	for(std::vector<ExporterLog::ComponentLogInfo *>::iterator componentIt = componentList.begin(); componentIt != componentList.end(); ++componentIt)
	{
		//tree should be empty at this point
		DEBUG_FATAL(ms_relationshipTree, ("building off a created tree"));
		if(!updateComponent(*componentIt, ms_relationshipTree))
			success = false;
	}

	for(std::vector<ExporterLog::LODLogInfo *>::iterator lodIt = lodList.begin(); lodIt != lodList.end(); ++lodIt)
	{
		//tree should be empty at this point
		DEBUG_FATAL(ms_relationshipTree, ("building off a created tree"));
		if(!updateLOD(*lodIt, ms_relationshipTree))
			success = false;
	}

	//send the relationships to the asset database
	if(!clearRelationships())
		success = false;

	if(!sendTreeRelationships(ms_relationshipTree))
		success = false;

	updateCommonData(interactive, locking, unlocking);

	delete ms_relationshipTree;
	ms_relationshipTree = NULL;

	reset();

	MESSENGER_LOG(("====== FINISHED IMPORT TO THE ASSET DATABASE ======\n"));
	return success;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Import the assets related to a skeletal mesh into the asset database.  This function requires that
 * the ExporterLog has already been populated with data from the current export.
 */
bool DatabaseImporter::importSkeletalMeshData(bool interactive, bool locking, bool unlocking)
{
	if(!ms_activatedState)
		return true;

	//don't submit anything if the user chose the "none" category
	if (userChoseOther())
		return true;

	MESSENGER_LOG(("====== BEGINNING IMPORT TO THE ASSET DATABASE ======\n"));

	std::vector<ExporterLog::SkeletalMeshGeneratorLogInfo *> meshList;
	std::vector<ExporterLog::ComponentLogInfo *> componentList;
	std::vector<ExporterLog::LODLogInfo *> lodList;

	//get each type of node list
	ExporterLog::ms_objects->filter(meshList);
	ExporterLog::ms_objects->filter(componentList);
	ExporterLog::ms_objects->filter(lodList);

	bool success = true;

	for(std::vector<ExporterLog::SkeletalMeshGeneratorLogInfo *>::iterator meshIt = meshList.begin(); meshIt != meshList.end(); ++meshIt)
	{
		//tree should be empty at this point
		DEBUG_FATAL(ms_relationshipTree, ("building off a created tree"));
		if(!updateSkeletalMesh(*meshIt, ms_relationshipTree))
			success = false;
	}

	for(std::vector<ExporterLog::ComponentLogInfo *>::iterator componentIt = componentList.begin(); componentIt != componentList.end(); ++componentIt)
	{
		//tree should be empty at this point
		DEBUG_FATAL(ms_relationshipTree, ("building off a created tree"));
		if(!updateComponent(*componentIt, ms_relationshipTree))
			success = false;
	}

	for(std::vector<ExporterLog::LODLogInfo *>::iterator lodIt = lodList.begin(); lodIt != lodList.end(); ++lodIt)
	{
		//tree should be empty at this point
		DEBUG_FATAL(ms_relationshipTree, ("building off a created tree"));
		if(!updateLOD(*lodIt, ms_relationshipTree))
			success = false;
	}

	//send the relationships to the asset database
	if(!clearRelationships())
		success = false;

	if(!sendTreeRelationships(ms_relationshipTree))
		success = false;

	updateCommonData(interactive, locking, unlocking);

	delete ms_relationshipTree;
	ms_relationshipTree = NULL;

	reset();

	MESSENGER_LOG(("====== FINISHED IMPORT TO THE ASSET DATABASE ======\n"));
	return success;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Import the assets related to a skeleton into the asset database.  This function requires that
 * the ExporterLog has already been populated with data from the current export.
 *
 * Assumes there is one skeleton to input
 */
bool DatabaseImporter::importSkeletonData(bool interactive, bool locking, bool unlocking)
{
	if(!ms_activatedState)
		return true;

	//don't submit anything if the user chose the "none" category
	if (userChoseOther())
		return true;

	MESSENGER_LOG(("====== BEGINNING IMPORT TO THE ASSET DATABASE ======\n"));

	bool success = true;

	std::vector<ExporterLog::SkeletalTemplateLogInfo *> skeletonList;
	ExporterLog::ms_objects->filter(skeletonList);
	for(std::vector<ExporterLog::SkeletalTemplateLogInfo *>::iterator skeletonIt = skeletonList.begin(); skeletonIt != skeletonList.end(); ++skeletonIt)
	{
		//tree should be empty at this point
		DEBUG_FATAL(ms_relationshipTree, ("building off a created tree"));
		if(!updateSkeleton(*skeletonIt, ms_relationshipTree))
			success = false;
	}

	//send the relationships to the asset database
	if(!clearRelationships())
		success = false;

	if(!sendTreeRelationships(ms_relationshipTree))
		success = false;

	updateCommonData(interactive, locking, unlocking);

	delete ms_relationshipTree;
	ms_relationshipTree = NULL;

	reset();

	MESSENGER_LOG(("====== FINISHED IMPORT TO THE ASSET DATABASE ======\n"));
	return success;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Import the assets related to a animation mesh into the asset database.  This function requires that
 * the ExporterLog has already been populated with data from the current export.
 */
bool DatabaseImporter::importAnimationData(bool interactive, bool locking, bool unlocking)
{
	if(!ms_activatedState)
		return true;

		//don't submit anything if the user chose the "none" category
	if (userChoseOther())
		return true;

	MESSENGER_LOG(("====== BEGINNING IMPORT TO THE ASSET DATABASE ======\n"));

	bool success = false;

	std::vector<ExporterLog::SkeletalAnimationLogInfo *> animationList;
	ExporterLog::ms_objects->filter(animationList);
	for(std::vector<ExporterLog::SkeletalAnimationLogInfo *>::iterator animationIt = animationList.begin(); animationIt != animationList.end(); ++animationIt)
	{
		//tree should be empty at this point
		DEBUG_FATAL(ms_relationshipTree, ("building off a created tree"));
		if(!updateAnimation(*animationIt, ms_relationshipTree))
			success = false;
	}

	//send the relationships to the asset database
	if(clearRelationships())
		success = false;

	if(!sendTreeRelationships(ms_relationshipTree))
		success = false;

	delete ms_relationshipTree;
	ms_relationshipTree = NULL;

	updateCommonData(interactive, locking, unlocking);

	reset();

	MESSENGER_LOG(("====== FINISHED IMPORT TO THE ASSET DATABASE ======\n"));
	return success;
}

//////////////////////////////////////////////////////////////////////////////////

bool DatabaseImporter::updateCommonData(bool interactive, bool locking, bool unlocking)
{
	FATAL(locking && unlocking, ("Can't both lock and unlock"));
	int newStatus     = REAL_ART;
	int currentStatus = getStatus(ms_assetName);

	if((locking || currentStatus == LOCKED) && !unlocking)
		newStatus = LOCKED;
	else if(unlocking)
		newStatus = REAL_ART;
	else
		newStatus = REAL_ART;

	//warn if trying to export to a locked asset w/o unlocking it
	if(currentStatus == LOCKED && !unlocking)
	{
		std::string errorMsg = "This asset is currently locked.  Please have a lead unlock it before exporting.\n";
		if(interactive)
			MESSENGER_MESSAGE_BOX(NULL, errorMsg.c_str(), "Error!", MB_OK | MB_ICONERROR);
		MESSENGER_LOG_ERROR((errorMsg.c_str()));
	}

	updateAppearance(ExporterLog::getAuthor(), newStatus);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

bool DatabaseImporter::updateAppearance(const std::string& author, int status)
{
	NOT_NULL(ms_session);

	DBQuery::UpdateAppearanceQuery updateAppearanceQuery;
	IGNORE_RETURN(updateAppearanceQuery.setup(ms_session));
	updateAppearanceQuery.m_appearanceNumber.setValue(ms_assetID);
	updateAppearanceQuery.m_author.setValue(author.c_str());
	updateAppearanceQuery.m_statusId.setValue(status);

	IGNORE_RETURN(updateAppearanceQuery.exec());

	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Build and send a query to update a shader asset
 */
int DatabaseImporter::updateShader(const ExporterLog::ShaderLogInfo *shader, RelationshipNode *node)
{
	NOT_NULL(node);
	NOT_NULL(shader);
	DEBUG_FATAL(!ms_connected, ("can't send update to DB when not connected"));
	NOT_NULL(ms_session);

	DBQuery::UpdateShaderQuery updateShaderQuery;
	IGNORE_RETURN(updateShaderQuery.setup(ms_session));

	//source shaders don't have an effect, and we don't want to track info on them
	if(shader->m_effectFilename.empty())
		return 0;

	std::string filename   = shader->m_filename;
	std::string::size_type pos = filename.find_last_of("\\");
	DEBUG_FATAL(pos == std::string::npos, ("bad filename format in DatabaseImporter::importStaticMeshData")); //lint !e650, !e737 lameness with string::find() and string::npos having different signage
	filename = filename.substr(pos+1); //to end implicit

	std::string effectName = shader->m_effectFilename;

	//fill the query with the values to submit
	updateShaderQuery.m_path.setValue("");

	updateShaderQuery.m_filename.setValue(filename.c_str());
	updateShaderQuery.m_effectName.setValue(effectName.c_str());

	IGNORE_RETURN(updateShaderQuery.exec());

	//build a new relationship node and store the entry number
	RelationshipNode *newNode = new RelationshipNode(node->m_exitNumber + 1);
	node->m_children.push_back(newNode);
	newNode->m_nodeNumber = updateShaderQuery.m_returnCode.getValue();
	newNode->m_parentEntryNumber = node->m_entryNumber;


	//import textures this shader owns
	std::vector<ExporterLog::TextureLogInfo *> textureList;
	ExporterLog::ms_objects->filter(textureList);
	for(std::vector<ExporterLog::TextureLogInfo *>::iterator textureIt = textureList.begin(); textureIt != textureList.end(); ++textureIt)
	{
		updateTexture(*textureIt, newNode);
	}
	return ++(newNode->m_exitNumber);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Build and send a query to update a texture asset
 */
int DatabaseImporter::updateTexture(const ExporterLog::TextureLogInfo *texture, RelationshipNode *node)
{
	NOT_NULL(node);
	NOT_NULL(texture);
	DEBUG_FATAL(!ms_connected, ("can't send update to DB when not connected"));
	NOT_NULL(ms_session);

	DBQuery::UpdateTextureQuery updateTextureQuery;
	IGNORE_RETURN(updateTextureQuery.setup(ms_session));

	//fill the query with the values to submit
	updateTextureQuery.m_filename.setValue(texture->m_filename.c_str());

	IGNORE_RETURN(updateTextureQuery.exec());

	//build a new relationship node and store the entry number
	RelationshipNode *newNode = new RelationshipNode(node->m_exitNumber + 1);
	node->m_children.push_back(newNode);
	newNode->m_nodeNumber = updateTextureQuery.m_returnCode.getValue();
	newNode->m_parentEntryNumber = node->m_entryNumber;
	return ++(newNode->m_exitNumber);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Build and send a query to update a static mesh asset
 */
int DatabaseImporter::updateStaticMesh(const ExporterLog::MeshLogInfo *mesh, RelationshipNode *node)
{
	NOT_NULL(mesh);
	DEBUG_FATAL(!ms_connected, ("can't send update to DB when not connected"));
	NOT_NULL(ms_session);

	DBQuery::UpdateMeshQuery updateMeshQuery;
	IGNORE_RETURN(updateMeshQuery.setup(ms_session));

	std::string filename   = mesh->m_filename;
	std::string::size_type pos = filename.find_last_of("\\");

	UNREF(pos);
	//fill the query with the values to submit
	updateMeshQuery.m_path.setValue("");
	updateMeshQuery.m_filename.setValue(filename.c_str());
	updateMeshQuery.m_type.setValue("Static");
	updateMeshQuery.m_numPolygons.setValue(mesh->m_numberOfPolygons);
	updateMeshQuery.m_numVertices.setValue(mesh->m_numberOfVertices);
	updateMeshQuery.m_numUVSets.setValue(mesh->m_numUVSets);
	updateMeshQuery.m_numHardpoints.setValue(mesh->m_hardpointList.size());
	updateMeshQuery.m_extentRadius.setValue(mesh->m_sphereRadius);
	updateMeshQuery.m_extentCenterX.setValue(mesh->m_sphereCenter.x);
	updateMeshQuery.m_extentCenterY.setValue(mesh->m_sphereCenter.y);
	updateMeshQuery.m_extentCenterZ.setValue(mesh->m_sphereCenter.y);
	updateMeshQuery.m_extentBoxMinX.setValue(mesh->m_boxMin.x);
	updateMeshQuery.m_extentBoxMinY.setValue(mesh->m_boxMin.y);
	updateMeshQuery.m_extentBoxMinZ.setValue(mesh->m_boxMin.z);
	updateMeshQuery.m_extentBoxMaxX.setValue(mesh->m_boxMax.x);
	updateMeshQuery.m_extentBoxMaxY.setValue(mesh->m_boxMax.y);
	updateMeshQuery.m_extentBoxMaxZ.setValue(mesh->m_boxMax.z);

	IGNORE_RETURN(updateMeshQuery.exec());

	//build a new relationship node and store the entry number
	//since this could be a root node (if NULL is passed in as the parent, check)
	RelationshipNode *newNode = NULL;
	if(!node)
	{
		newNode = new RelationshipNode(1);
		ms_relationshipTree = newNode;
		newNode->m_parentEntryNumber = 0;
	}
	else
	{
		newNode = new RelationshipNode(node->m_exitNumber + 1);
		node->m_children.push_back(newNode);
		newNode->m_parentEntryNumber = node->m_entryNumber;
	}
	newNode->m_nodeNumber = updateMeshQuery.m_returnCode.getValue();

	//import shaders this mesh owns
	std::vector<ExporterLog::ShaderLogInfo *> shaderList;
	mesh->filter(shaderList);
	for(std::vector<ExporterLog::ShaderLogInfo *>::iterator shaderIt = shaderList.begin(); shaderIt != shaderList.end(); ++shaderIt)
	{
		newNode->m_exitNumber = updateShader(*shaderIt, newNode);
	}
	return ++(newNode->m_exitNumber);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Build and send a query to update a skeletal mesh asset
 */
int DatabaseImporter::updateSkeletalMesh(const ExporterLog::MeshLogInfo *mesh, RelationshipNode *node)
{
	NOT_NULL(mesh);
	DEBUG_FATAL(!ms_connected, ("can't send update to DB when not connected"));
	NOT_NULL(ms_session);

	DBQuery::UpdateMeshQuery updateMeshQuery;
	IGNORE_RETURN(updateMeshQuery.setup(ms_session));


	std::string filename   = mesh->m_filename;

	//fill the query with the values to submit
	updateMeshQuery.m_path.setValue("");
	updateMeshQuery.m_filename.setValue(filename.c_str());
	updateMeshQuery.m_type.setValue("Skeletal");
	updateMeshQuery.m_numPolygons.setValue(mesh->m_numberOfPolygons);
	updateMeshQuery.m_numVertices.setValue(mesh->m_numberOfVertices);
	updateMeshQuery.m_numUVSets.setValue(mesh->m_numUVSets);
	updateMeshQuery.m_numHardpoints.setValue(mesh->m_hardpointList.size());
	updateMeshQuery.m_extentRadius.setValue(mesh->m_sphereRadius);
	updateMeshQuery.m_extentCenterX.setValue(mesh->m_sphereCenter.x);
	updateMeshQuery.m_extentCenterY.setValue(mesh->m_sphereCenter.y);
	updateMeshQuery.m_extentCenterZ.setValue(mesh->m_sphereCenter.y);
	updateMeshQuery.m_extentBoxMinX.setValue(mesh->m_boxMin.x);
	updateMeshQuery.m_extentBoxMinY.setValue(mesh->m_boxMin.y);
	updateMeshQuery.m_extentBoxMinZ.setValue(mesh->m_boxMin.z);
	updateMeshQuery.m_extentBoxMaxX.setValue(mesh->m_boxMax.x);
	updateMeshQuery.m_extentBoxMaxY.setValue(mesh->m_boxMax.y);
	updateMeshQuery.m_extentBoxMaxZ.setValue(mesh->m_boxMax.z);

	IGNORE_RETURN(updateMeshQuery.exec());

	//build a new relationship node and store the entry number
	//since this could be a root node (if NULL is passed in as the parent, check)
	RelationshipNode *newNode = NULL;
	if(!node)
	{
		newNode = new RelationshipNode(1);
		ms_relationshipTree = newNode;
		newNode->m_parentEntryNumber = 0;
	}
	else
	{
		newNode = new RelationshipNode(node->m_exitNumber + 1);
		node->m_children.push_back(newNode);
		newNode->m_parentEntryNumber = node->m_entryNumber;
	}
	newNode->m_nodeNumber = updateMeshQuery.m_returnCode.getValue();

	//import shaders this mesh owns
	std::vector<ExporterLog::ShaderLogInfo *> shaderList;
	mesh->filter(shaderList);
	for(std::vector<ExporterLog::ShaderLogInfo *>::iterator shaderIt = shaderList.begin(); shaderIt != shaderList.end(); ++shaderIt)
	{
		newNode->m_exitNumber = updateShader(*shaderIt, newNode);
	}
	return ++(newNode->m_exitNumber);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Build and send a query to update a component asset
 */
int DatabaseImporter::updateComponent(const ExporterLog::ComponentLogInfo *component, RelationshipNode *node)
{
	NOT_NULL(component);
	DEBUG_FATAL(!ms_connected, ("can't send update to DB when not connected"));
	NOT_NULL(ms_session);

	DBQuery::UpdateComponentQuery updateComponentQuery;
	IGNORE_RETURN(updateComponentQuery.setup(ms_session));


	std::string filename   = component->m_filename;
	std::string::size_type pos = filename.find_last_of("\\");

	UNREF(pos);
	//fill the query with the values to submit
	updateComponentQuery.m_path.setValue("");
	updateComponentQuery.m_filename.setValue(filename.c_str());

	IGNORE_RETURN(updateComponentQuery.exec());

	//build a new relationship node and store the entry number
	//since this could be a root node (if NULL is passed in as the parent, check)
	RelationshipNode *newNode = NULL;
	if(!node)
	{
		newNode = new RelationshipNode(1);
		ms_relationshipTree = newNode;
		newNode->m_parentEntryNumber = 0;
	}
	else
	{
		newNode = new RelationshipNode(node->m_exitNumber + 1);
		node->m_children.push_back(newNode);
		newNode->m_parentEntryNumber = node->m_entryNumber;
	}
	newNode->m_nodeNumber = updateComponentQuery.m_returnCode.getValue();

	//import all this asset's children
	std::vector<ExporterLog::MeshLogInfo *> meshList;
	std::vector<ExporterLog::ComponentLogInfo *> componentList;
	std::vector<ExporterLog::LODLogInfo *> lodList;
	component->filter(meshList);
	component->filter(componentList);
	component->filter(lodList);
	for(std::vector<ExporterLog::MeshLogInfo *>::iterator meshIt = meshList.begin(); meshIt != meshList.end(); ++meshIt)
	{
		newNode->m_exitNumber = updateStaticMesh(*meshIt, newNode);
	}
	for(std::vector<ExporterLog::ComponentLogInfo *>::iterator componentIt = componentList.begin(); componentIt != componentList.end(); ++componentIt)
	{
		newNode->m_exitNumber = updateComponent(*componentIt, newNode);
	}
	for(std::vector<ExporterLog::LODLogInfo *>::iterator lodIt = lodList.begin(); lodIt != lodList.end(); ++lodIt)
	{
		newNode->m_exitNumber = updateLOD(*lodIt, newNode);
	}
	return ++(newNode->m_exitNumber);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Build and send a query to update an LOD asset
 */
int DatabaseImporter::updateLOD(const ExporterLog::LODLogInfo *LOD, RelationshipNode *node)
{
	NOT_NULL(LOD);
	DEBUG_FATAL(!ms_connected, ("can't send update to DB when not connected"));
	NOT_NULL(ms_session);

	DBQuery::UpdateLODQuery updateLODQuery;
	IGNORE_RETURN(updateLODQuery.setup(ms_session));

	std::string filename   = LOD->m_filename;
	std::string::size_type pos = filename.find_last_of("\\");

	UNREF(pos);
	//fill the query with the values to submit
	updateLODQuery.m_path.setValue("");
	updateLODQuery.m_filename.setValue(filename.c_str());

	IGNORE_RETURN(updateLODQuery.exec());

	//build a new relationship node and store the entry number
	//since this could be a root node (if NULL is passed in as the parent, check)
	RelationshipNode *newNode = NULL;
	if(!node)
	{
		newNode = new RelationshipNode(1);
		ms_relationshipTree = newNode;
		newNode->m_parentEntryNumber = 0;
	}
	else
	{
		newNode = new RelationshipNode(node->m_exitNumber + 1);
		node->m_children.push_back(newNode);
		newNode->m_parentEntryNumber = node->m_entryNumber;
	}
	newNode->m_nodeNumber = updateLODQuery.m_returnCode.getValue();

	//import all this asset's children
	std::vector<ExporterLog::MeshLogInfo *> meshList;
	std::vector<ExporterLog::ComponentLogInfo *> componentList;
	std::vector<ExporterLog::LODLogInfo *> lodList;
	LOD->filter(meshList);
	LOD->filter(componentList);
	LOD->filter(lodList);
	for(std::vector<ExporterLog::MeshLogInfo *>::iterator meshIt = meshList.begin(); meshIt != meshList.end(); ++meshIt)
	{
		newNode->m_exitNumber = updateStaticMesh(*meshIt, newNode);
	}
	for(std::vector<ExporterLog::ComponentLogInfo *>::iterator componentIt = componentList.begin(); componentIt != componentList.end(); ++componentIt)
	{
		newNode->m_exitNumber = updateComponent(*componentIt, newNode);
	}
	for(std::vector<ExporterLog::LODLogInfo *>::iterator lodIt = lodList.begin(); lodIt != lodList.end(); ++lodIt)
	{
		newNode->m_exitNumber = updateLOD(*lodIt, newNode);
	}
	return ++(newNode->m_exitNumber);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Build and send a query to update a skeleton asset
 */
int DatabaseImporter::updateSkeleton(const ExporterLog::SkeletalTemplateLogInfo *skeleton, RelationshipNode *node)
{
	NOT_NULL(skeleton);
	DEBUG_FATAL(!ms_connected, ("can't send update to DB when not connected"));
	NOT_NULL(ms_session);

	DBQuery::UpdateSkeletonQuery updateSkeletonQuery;
	IGNORE_RETURN(updateSkeletonQuery.setup(ms_session));

	std::string filename   = skeleton->m_filename;
	std::string::size_type pos = filename.find_last_of("\\");

	UNREF(pos);
	//fill the query with the values to submit
	updateSkeletonQuery.m_path.setValue("");
	updateSkeletonQuery.m_filename.setValue(filename.c_str());
	updateSkeletonQuery.m_numBones.setValue(0);

	IGNORE_RETURN(updateSkeletonQuery.exec());

	//build a new relationship node and store the entry number
	//since this could be a root node (if NULL is passed in as the parent, check)
	RelationshipNode *newNode = NULL;
	if(!node)
	{
		newNode = new RelationshipNode(1);
		ms_relationshipTree = newNode;
		newNode->m_parentEntryNumber = 0;
	}
	else
	{
		newNode = new RelationshipNode(node->m_exitNumber + 1);
		node->m_children.push_back(newNode);
		newNode->m_parentEntryNumber = node->m_entryNumber;
	}
	newNode->m_nodeNumber = updateSkeletonQuery.m_returnCode.getValue();

	return ++(newNode->m_exitNumber);
}

//////////////////////////////////////////////////////////////////////////////////

/**
 * Build and send a query to update an animation asset
 */
int DatabaseImporter::updateAnimation(const ExporterLog::SkeletalAnimationLogInfo *animation, RelationshipNode *node)
{
	NOT_NULL(animation);
	DEBUG_FATAL(!ms_connected, ("can't send update to DB when not connected"));
	NOT_NULL(ms_session);

	DBQuery::UpdateAnimationQuery updateAnimationQuery;
	IGNORE_RETURN(updateAnimationQuery.setup(ms_session));

	std::string filename   = animation->m_filename;
	std::string::size_type pos = filename.find_last_of("\\");

	UNREF(pos);
	//fill the query with the values to submit
	updateAnimationQuery.m_path.setValue("");
	updateAnimationQuery.m_filename.setValue(filename.c_str());
	updateAnimationQuery.m_numFrames.setValue(0);
	updateAnimationQuery.m_gender.setValue(0);
	updateAnimationQuery.m_type.setValue(0);

	IGNORE_RETURN(updateAnimationQuery.exec());

	//build a new relationship node and store the entry number
	//since this could be a root node (if NULL is passed in as the parent, check)
	RelationshipNode *newNode = NULL;
	if(!node)
	{
		newNode = new RelationshipNode(1);
		ms_relationshipTree = newNode;
		newNode->m_parentEntryNumber = 0;
	}
	else
	{
		newNode = new RelationshipNode(node->m_exitNumber + 1);
		node->m_children.push_back(newNode);
		newNode->m_parentEntryNumber = node->m_entryNumber;
	}
	newNode->m_nodeNumber = updateAnimationQuery.m_returnCode.getValue();

	return ++(newNode->m_exitNumber);
}

//////////////////////////////////////////////////////////////////////////////////

/** Determine and store the asset appearance name for this export.
 *  This data can come from either the associated log file, or from the user (via a GUI).
 *  The filename for the associated appearance must match the give nodeName
 */
bool DatabaseImporter::selectAppearance(bool interactive, const std::string& nodeName)
{
	if(!ms_activatedState)
		return true;

	std::string category;

// JU_TODO: alienbrain def out
#if 0
	category = AlienbrainImporter::getAssetDBCategory();
#else
	// JU_TODO: implement
	std::string sourceFilename = ExporterLog::getSourceFilename();
	category = ExportManager::getAssetDBCategory(sourceFilename);
#endif
// JU_TODO: end alienbrain def out

	int categoryNumber = -1;
	if(_stricmp(category.c_str(), "Creature") == 0)
		categoryNumber = CAT_CREATURE;
	else if(_stricmp(category.c_str(), "Architecture") == 0)
		categoryNumber = CAT_ARCHITECTURE;
	else if(_stricmp(category.c_str(), "Item") == 0)
		categoryNumber = CAT_ITEM;
	else if(_stricmp(category.c_str(), "Vehicle") == 0)
		categoryNumber = CAT_VEHICLE;
	else if(_stricmp(category.c_str(), "Wearable") == 0)
		categoryNumber = CAT_WEARABLE;
	else if(_stricmp(category.c_str(), "Droid") == 0)
		categoryNumber = CAT_DROID;
	else if(_stricmp(category.c_str(), "NPC") == 0)
		categoryNumber = CAT_NPC;
	else if(_stricmp(category.c_str(), "Weapon") == 0)
		categoryNumber = CAT_WEAPON;
	else if(_stricmp(category.c_str(), "Player") == 0)
		categoryNumber = CAT_PLAYERSPECIES;
	else if(_stricmp(category.c_str(), "Space") == 0)
		categoryNumber = CAT_SPACE;
	else if(_stricmp(category.c_str(), "Other") == 0 || _stricmp(category.c_str(), "None") == 0)
		categoryNumber = CAT_OTHER;

	//-1 in an error case (no category matched)
	if(categoryNumber == -1)
	{
		std::string errorMsg = "Category found in Alienbrain [";
		errorMsg            += category + "] does not match any acceptable Category in the AssetDB\n";
		if(interactive)
			MESSENGER_MESSAGE_BOX(NULL, errorMsg.c_str(), "Error!", MB_OK | MB_ICONERROR);
		MESSENGER_LOG_ERROR((errorMsg.c_str()));
		return false;
	}

	ms_nodeName = nodeName;

	if (categoryNumber != CAT_OTHER)
	{
		//use the category to get the appropriate set of assets
		ms_assetNames.clear();
		DBQuery::RetrieveAppearanceQuery retrieveAppearanceQuery;
		retrieveAppearanceQuery.SetCategoryNumber(categoryNumber);
		IGNORE_RETURN(ms_session->exec(&retrieveAppearanceQuery));
		std::string assetName;
		std::string assetFilename;
		int assetID;
		int statusID;
		while(retrieveAppearanceQuery.fetch())
		{
			assetName     = Unicode::wideToNarrow(retrieveAppearanceQuery.GetAssetName());
			assetFilename = Unicode::wideToNarrow(retrieveAppearanceQuery.GetFilename());
			assetID       = retrieveAppearanceQuery.GetAssetID();
			statusID      = retrieveAppearanceQuery.GetStatusID();

			//store the asset name to filename mapping, use it to retrieve the filename later (to check against the node name)
			ms_assetToFilenameMap[assetName]     = assetFilename;
			ms_filenameToAssetMap[assetFilename] = assetName;
			//store the asset name to asset it mapping, use it in the dialog boxes
			ms_assetNames.push_back(std::make_pair(assetName, assetID));

			ms_statusAssetMap[assetName] = statusID;
		}

		if(interactive)
		{
			//get the actual asset name
			if(!showAssetNamesGUI())
				return false;
		}
		else
		{
			ms_assetName = ms_filenameToAssetMap[nodeName];
			//store the data into the exporter log
			ExporterLog::setAssetName(ms_assetName);
		}

		//assetFileName holds the name of the asset, check it
		assetFilename = ms_assetToFilenameMap[ms_assetName];
		if(assetFilename != ms_nodeName)
		{
			if(assetFilename != ms_nodeName)
			{
				std::string errorMsg = "The node name must match the chosen asset's expected filename.  Node name: \"";
				errorMsg += ms_nodeName + "\", Asset filename: \"";
				errorMsg += assetFilename + "\".\n";
				if(interactive)
					MESSENGER_MESSAGE_BOX(NULL, errorMsg.c_str(), "Error!", MB_OK | MB_ICONERROR);
				MESSENGER_LOG_ERROR((errorMsg.c_str()));
				return false;
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////

/** Return the status of an asset, or -1 on failure to find it
 */
int DatabaseImporter::getStatus(const std::string& nodeName)
{
	std::map<std::string, int>::iterator i = ms_statusAssetMap.find(nodeName);
	if(i != ms_statusAssetMap.end())
		return i->second;
	else
		return -1;
}

//////////////////////////////////////////////////////////////////////////////////

bool DatabaseImporter::userChoseOther()
{
	return ms_categoryCheck == CAT_OTHER;
}

//////////////////////////////////////////////////////////////////////////////////

/** Get the asset name and id from either the exporter log (if it's been set previously), or the user (via a GUI)
 */
bool DatabaseImporter::showAssetNamesGUI()
{
	//check the exporterlog first for the asset name and id
	std::string exporterLogAssetName = ExporterLog::getAssetName();
	if(!exporterLogAssetName.empty() && exporterLogAssetName != "<none assigned>")
	{
		ms_assetName = exporterLogAssetName;
		ms_assetID = ExporterLog::getAssetId();
		return true;
	}
	else
	{
		//run the gui and get the asset name and id from it
		int result = DialogBox(GetPluginInstanceHandle(), MAKEINTRESOURCE (IDD_ASSET_NAME), NULL, assetNamesDialogProc); //lint !e1924 C-style cast (this conveinent macro uses one "internally")
		if (result != IDOK)
			return false;
	}
	
	//store the data into the exporter log
	ExporterLog::setAssetName(ms_assetName);
	ExporterLog::setAssetId(ms_assetID);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DatabaseImporter::assetNamesDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	UNREF(lParam); 
	HWND assetNameList = GetDlgItem(hDlg, IDC_ASSET_LIST);
	std::string nodeName;

	switch(iMsg)
	{
		case WM_INITDIALOG:
		{
			//fill in the list box with the items
			for(std::vector<std::pair<std::string, int> >::iterator i = ms_assetNames.begin(); i != ms_assetNames.end(); ++i)
			{
				int index = SendMessage(assetNameList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(i->first.c_str()));
				int data = i->second;
				UNREF(data);
				SendMessage(assetNameList, LB_SETITEMDATA, index, static_cast<LPARAM>(i->second));
	
				//see if this entry matches with the node name
				if(ms_assetToFilenameMap[i->first.c_str()] == ms_nodeName)
				{
					nodeName = i->first.c_str();
				}
			}
			//set the cursor directly on the node-named item if possible, else pick the first item.
			int result = SendMessage(assetNameList, LB_FINDSTRINGEXACT, 0, reinterpret_cast<LPARAM>(nodeName.c_str()));
			if(result != LB_ERR)
				SendMessage(assetNameList, LB_SETCURSEL, result, 0);
			else
				IGNORE_RETURN(SendMessage(assetNameList, LB_SETCURSEL, 0, 0));
			return TRUE;
		}

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					IGNORE_RETURN(EndDialog(hDlg, IDOK));
					int index = SendMessage(assetNameList, LB_GETCURSEL, 0, 0);
					if(index == LB_ERR)
						return false;
					int length = SendMessage(assetNameList, LB_GETTEXTLEN, index, 0);
					char* buffer = new char[length+1];
					IGNORE_RETURN(SendMessage(assetNameList, LB_GETTEXT, index, reinterpret_cast<LPARAM>(buffer)));
					int assetID = SendMessage(assetNameList, LB_GETITEMDATA, index, 0);
					DEBUG_FATAL(assetID == LB_ERR, ("No assetID associated with the selected asset name"));
					ms_assetName = buffer;
					delete[] buffer;
					ms_assetID = assetID;
					return TRUE;
				}

				case IDCANCEL:
					IGNORE_RETURN(EndDialog(hDlg, IDCANCEL));
					return TRUE;

				default:
					return FALSE;
			}

		default:         //lint !e616 !e825 control flow falls though, otherwise we get unreachable code according to MSVC (sigh)
			return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////////////////
#endif
