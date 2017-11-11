// ======================================================================
//
// AlienbrainImporter.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef ALIENBRAINIMPORTER_H
#define ALIENBRAINIMPORTER_H

// JU_TODO: alienbrain def out
#if 0

#include "ExporterLog.h"  //using ExporterLog::ShaderLogInfo*

// ======================================================================

class Messenger;
class CNxNPath;
class DatabaseImporter;

// ======================================================================

/**
 * This handles the interface to Alienbrain, and performs the import of files from
 * the exporter to the Alienbrain server.  The ExporterLog is used to maintain all 
 * the logging data, which is also needed by this class to determine what to import,
 * and what properties to assign to each file.  So it is important to first use
 * the ExporterLog class to build up the log information needed before attempting 
 * an Alienbrain import.
 *
 * Only the source assets are being set to the Alienbrain server (the source textures
 * and the log file generated from the export process).  The maya binary file is NOT
 * submitted, since it causes some confusion among the artist.  All the 
 * rest of the files go to Perforce using the class PerforceImporter.
 * 
 * @see PerforceImporter
 *
 */
class AlienbrainImporter
{
	friend DatabaseImporter;

	public:
		static void install(Messenger* newMessenger);
		static void remove();
		static bool connectToServer();
		static bool disconnectFromServer();
		static void importLogFile();
		static bool preImport(const std::string& assetName, bool showGUI);
		static void reset();
		static void storeFileProperties();
		static void startMultiExport();
		static void endMultiExport();
		static bool validateFiles(const std::vector<std::string>& textureList, bool showGUI);
		static bool validateFile(const std::string& file, bool showGUI);

		static const std::string& getPerforceClientPath();
		static const std::string& getPerforceSharedPath();
		static const std::string& getPerforceServerPath();
		static const std::string& getAssetDBCategory();

	private:
		static bool validateTextureList(const std::vector<std::string>& textureList, bool showGUI);
		static std::string findWorkspaceLocation(const std::string& file);

	private:
		///stores the perforce depot location for the game assets (comes from a property on the source file)
		static std::string ms_perforceClientPath;
		static std::string ms_perforceSharedPath;
		static std::string ms_perforceServerPath;
		//stores thet category of the asset (comes from a property on the file)
		static std::string ms_assetDBCategory;

		///the current base alienbrain path to import into (before tacking on asset specific dirs such as \mesh and \texture)
		static std::string ms_currentPath;
		///are this module installed?
		static bool ms_installed;
		///are we connected to the Alienbrain server?
		static bool ms_connected;

		static bool ms_multiExporting;

		///MUST be named messenger for #define's, can't be called ms_messenger (see Messenger.h)
		static Messenger* messenger;

		//constants
		static const std::string ms_alienbrainProject;
		static const std::string ms_logImportPath;
		static const std::string ms_sourceImportPath;
		static const std::string ms_sourceTextureImportPath;
		static const std::string ms_shaderImportPath;
};

// ======================================================================

/**
 * Get the base perforce client path.  This piece of data is stored on the Alienbrain database objects in the "PerforceDir"
 * property.
 */
inline const std::string& AlienbrainImporter::getPerforceClientPath()
{
	return ms_perforceClientPath;
}

// ----------------------------------------------------------------------

/**
 * Get the base perforce shared path.  This piece of data is stored on the Alienbrain database objects in the "PerforceSharedDir"
 * property.
 */
inline const std::string& AlienbrainImporter::getPerforceSharedPath()
{
	return ms_perforceSharedPath;
}

// ----------------------------------------------------------------------

/**
 * Get the base perforce server path.  This piece of data is stored on the Alienbrain database objects in the "PerforceServerDir"
 * property.
 */
inline const std::string& AlienbrainImporter::getPerforceServerPath()
{
	return ms_perforceServerPath;
}

// ----------------------------------------------------------------------

/**
 * Get the asset database category.  This piece of data is stored on the Alienbrain database objects in the "AssetDBCategory"
 * property.
 */
inline const std::string& AlienbrainImporter::getAssetDBCategory()
{
	return ms_assetDBCategory;
}

// ======================================================================
#endif
// JU_TODO: end alienbrain def out

#endif //ALIENBRAINIMPORTER_H
