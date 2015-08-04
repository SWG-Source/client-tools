// ======================================================================
//
// ExportManager.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef ExportManager_H
#define ExportManager_H

// ======================================================================

//forward declarations
class Messenger;

// ======================================================================

/** This manager class is used for non-export type specific functionality.  There is a lot
    of duplicated code in the various Export*.cpp classes and a good bit of it should probably
    move here.
 */
class ExportManager
{
public:
		static void install(Messenger* newMessenger);
		static void remove();

		static bool validateTextureList(bool showGUI);
		static const stdvector<std::string>::fwd & getValidBranches();
		static void setValidBranches(const stdvector<std::string>::fwd & newValidBranches);
		static void setValidBranchesPacked(const std::string & packedBranchNames);
		static bool isValidBranch(const std::string & branch);

		// pops the viewer with the given asset
		static void LaunchViewer(const std::string& asset);

// JU_TODO: alienbrain temp workaround
		static std::string getPerforceClientDir(const std::string& alienbrainDir);
		static std::string getPerforceSharedDir(const std::string& alienbrainDir);
		static std::string getPerforceServerDir(const std::string& alienbrainDir);
		static std::string getAssetDBCategory(const std::string& alienbrainDir);
// JU_TODO: end alienbrain temp workaround

private:
		static Messenger   *messenger;
		static stdvector<std::string>::fwd cs_validBranches;
};

// ======================================================================

#endif //ExportManager_H
