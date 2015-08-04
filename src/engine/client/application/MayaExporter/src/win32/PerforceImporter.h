// ======================================================================
//
// AlienbrainImporter.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef PERFORCEIMPORTER_H
#define PERFORCEIMPORTER_H

// ======================================================================

class Messenger;
class ClientApi;

// ======================================================================


/**
 * This function is the interface for submitting files to perforce from the mayaExporter.
 * It requires the list of files that need to be exported to perforce (exported assets only,
 * source assets go to Alienbrain), so this cannot occur until after the local export process.
 * The actual list of files to export is gathered by the ExporterLog (which serves as the nexus
 * for export information), so this process must pull data from that system.
 *
 * Two types of export processes are available: one-shot exports and multi-exports.  One shot exports
 * are run from the Maya interface, when an arts says "export this to source control, now!".  A changelist
 * is built and submitted immediately.  The other option is the "multi-export", which is most useful for
 * non-interactive export processes (reexportAll) or when an artist wants to export a lot of things all
 * at once.  In this case a single changelist is created and appended to during each export process,
 * and only submitted when explicity told to (via the endMultiExport function).
 *
 */
class PerforceImporter
{
	public:
		static bool importCommon                (bool interactive, bool createNewChangelist, bool lock, bool unlock);
		static void install                     (Messenger* newMessenger);
		static void remove                      ();
		static void reset                       ();
		static void setComment                  (const std::string& comment);
		static bool setBranch                   (const std::string& branch);
		static void setRevertOnFail             (bool revertOnFail);
		static std::string getPerforcePath      (const std::string& filename);
		static bool validateCurrentChangelist   ();
		static std::string findFileOnDisk       (const std::string& perforcePath);
		static bool revertFile                  (const std::string& filename);

	private:
		static bool connectToDepot              ();
		static bool disconnectFromDepot         ();
		static std::string getClientBase        (const std::string& perforceDir);
		static bool editFile                    (const std::string& directoryInPerforceDepot, const std::string& sourceDirectory, const std::string& filename, bool addIfNecessary);
		static bool reopenFile                  (const std::string& directoryInPerforceDepot, const std::string& sourceDirectory, const std::string& filename);
		static void revertMostUnchangedFiles    ();
		static void splitDirectoryFromFilename  (const std::string& directoryAndFilename, std::string& directory, std::string& filename);
		static std::string findFinalPerforceDir (const std::string& clientBase, const std::string& sourceDir, const std::string& subDir);
		static void createChangelist            ();
		static bool doSubmission                ();
		static bool revertAll                   ();
		static bool addFilesToChangelist        (const std::string & baseDir, const stdset<std::string>::fwd & destFiles, const std::string& subDir, bool locking, bool unlocking);
		static void lockOpenFiles               ();
		static void unlockOpenFiles             ();
		static bool isChangelistEmpty           ();
		static void runP4Command                (int argc, const char* argvCommands[], const std::string& commandName, const std::string& waitingMsg);
		static void getRevertedFileList         ();
		static void addRevertedFileListToComment();


	private:
		//forward declarations
		class ClientUserWithReturnValues;
		
		///MUST be named messenger for #define's, can't be called ms_messenger (see Messenger.h)
		static Messenger* messenger;

		///P4 API class, needed to access the depot
		static ClientApi*                 ms_perforceClient;
		///derived P4 user class, needed to access the depot
		static ClientUserWithReturnValues ms_perforceUser;
		///is the system installed?
		static bool                       ms_installed;
		///are we connected to the depot currenctly?
		static bool                       ms_connected;
		///the changelist currently being dealt with
		static std::string                ms_changelistNumber;
		///
		static stdmap<std::string, std::string>::fwd ms_fileToPerforceFileMap;
		///are we running in an interactive mode? (meaning should we show GUI dialogs or not)
		static bool                       ms_interactive;
		///the submission comment
		static std::string                ms_comment;
		///should we revert all files on an export failure
		static bool                       ms_revertOnFail;
		///the list of files that we reverted
		static stdset<std::string>::fwd   ms_revertedFiles;
		///the name of the branch to export to
		static std::string                ms_branch;
};

// ======================================================================

#endif //PERFORCEIMPORTER_H
