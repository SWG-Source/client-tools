// ======================================================================
//
// DatabaseImporter.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef DATABASEIMPORTER_H
#define DATABASEIMPORTER_H

#include "sharedDatabaseInterface/DBSession.h" //in a namespace, so can't forward declare
#include "ExporterLog.h"                       //requires inner classes, can't forward declare

// ======================================================================

//forward declarations
class Messenger;

// ======================================================================

/**
 * This class represents the connection between the mayaExporter and the art asset database.  It
 * takes the data from a successful export (which is stored in the ExporterLog module) and submits
 * it to the art asset database (which in maintained on Oracle).
 */
class DatabaseImporter
{
	private:
		//inner class forward declaration (definition available in the cpp file)
		class RelationshipNode;

	public:
		static void install(Messenger* newMessenger);
		static void remove();
		static void activate(bool state, bool interactive = true);
		static bool startSession();
		static bool endSession();
		static bool importStaticMeshData(bool interactive, bool locking, bool unlocking);
		static bool importSkeletalMeshData(bool interactive, bool locking, bool unlocking);
		static bool importSkeletonData(bool interactive, bool locking, bool unlocking);
		static bool importAnimationData(bool interactive, bool locking, bool unlocking);
		static bool selectAppearance(bool interactive, const std::string& nodeName);
		static bool userChoseOther();
		static void reset();

	private:
		static bool updateCommonData(bool interactive, bool locking, bool unlocking);
		static bool updateAppearance(const std::string& author, int status);
		static int  updateStaticMesh(const ExporterLog::MeshLogInfo             *mesh,      RelationshipNode *node);
		static int  updateSkeletalMesh(const ExporterLog::MeshLogInfo           *mesh,      RelationshipNode *node);

		static int  updateLOD(const ExporterLog::LODLogInfo                     *LOD,       RelationshipNode *node);
		static int  updateComponent(const ExporterLog::ComponentLogInfo         *component, RelationshipNode *node);
		static int  updateTexture(const ExporterLog::TextureLogInfo             *texture,   RelationshipNode *node);
		static int  updateShader(const ExporterLog::ShaderLogInfo               *shader,    RelationshipNode *node);
		static int  updateSkeleton(const ExporterLog::SkeletalTemplateLogInfo   *skeleton,  RelationshipNode *node);
		static int  updateAnimation(const ExporterLog::SkeletalAnimationLogInfo *animation, RelationshipNode *node);

		static int  getStatus(const std::string& nodeName);

		static bool sendTreeRelationships(RelationshipNode *root);
		static bool clearRelationships();
		static bool showCategoriesGUI();
		static BOOL CALLBACK categoryDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
		static bool showAssetNamesGUI();
		static BOOL CALLBACK assetNamesDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

	private:
		///the possible asset categories, used in the IDD_ASSET_CATEGORY dialog box, and used to filter the assetdatabase
		static enum Categories
		{
			CAT_CHARACTER = 1,
			CAT_CREATURE,
			CAT_ARCHITECTURE,
			CAT_FLORA,
			CAT_ITEM,
			CAT_TERRAIN,
			CAT_VEHICLE,
			CAT_WEARABLE,
			CAT_DROID,
			CAT_NPC,
			CAT_PLAYERSPECIES,
			CAT_WEAPON,
			CAT_SPACE,
			CAT_OTHER,
			CAT_UNKNOWN
		} ms_categoryCheck;
		///the list of possible asset names (a section of all the possible asset names, filtered by the Categories enum and the Database)
		static stdvector<std::pair<std::string, int> >::fwd ms_assetNames;
		///the name of the selected asset (picked from ms_assetNames the first time, comes from the ExporterLog afterwards)
		static std::string ms_assetName;
		///the ID of the asset (picked from ms_assetNames the first time, comes from the ExporterLog afterwards)
		static int ms_assetID;
		///used to output messages to a window
		static Messenger   *messenger;
		///class that wraps a database server
		static DB::Server  *ms_server;
		///class that wraps a session to a database
		static DB::Session *ms_session;
		///do we have an open session to a database server?
		static bool         ms_connected;
		///base of the relationship tree
		static RelationshipNode   *ms_relationshipTree;
		///whether to actually use the system or not (initially disabled)
		static bool         ms_activatedState;
		///map from the asset names to the filenames
		static stdmap<std::string, std::string>::fwd ms_assetToFilenameMap;
		static stdmap<std::string, std::string>::fwd ms_filenameToAssetMap;
		///the maya node name used for the export
		static std::string ms_nodeName;
		///whether each asset is locked
		static stdmap<std::string, int>::fwd ms_statusAssetMap;
};

// ======================================================================

#endif //DATABASEIMPORTER_H
