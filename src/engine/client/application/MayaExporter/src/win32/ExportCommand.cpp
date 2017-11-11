// ======================================================================
//
// ExportCommand.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

//precompiled header include
#include "FirstMayaExporter.h"

//module include
#include "ExportCommand.h"

//local mayaExporter inludes
#if !NO_DATABASE
#include "DatabaseImporter.h"
#endif
#include "ExportArgs.h"
#include "ExportManager.h"
#include "MayaMeshReader.h"
#include "Messenger.h"
#include "PerforceImporter.h"
#include "PluginMain.h"
#include "resource.h"
#include "SetDirectoryCommand.h"

//maya SDK includes
#include "maya/MArgList.h"
#include "maya/MGlobal.h"

// ======================================================================

namespace ExportCommandNamespace
{
	const std::string    s_exportStaticMeshStr    = "exportStaticMesh";
	const std::string    s_exportMeshGeneratorStr = "exportSkeletalMeshGenerator";
	const std::string    s_exportSkeletonStr      = "exportSkeleton";
	const std::string    s_exportAnimationStr     = "exportSkeletalAnimation";
	const std::string    s_exportSATStr           = "exportSatFile";
	std::string          s_branchName             = "";
}

using namespace ExportCommandNamespace;

// ======================================================================

static Messenger *messenger; //lint !e551 messenger not accessed (but it actually is)

// ======================================================================

std::string          ExportCommand::ms_command;

// ======================================================================

void ExportCommand::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void ExportCommand::remove()
{
	messenger = 0;
}

// ----------------------------------------------------------------------

void *ExportCommand::creator()
{
	return new ExportCommand();
}

// ----------------------------------------------------------------------

MStatus ExportCommand::doIt(const MArgList &argList)
{


	UNREF(argList);

	// force the active drive.
	const char * activeDrive = NULL;
	activeDrive = SetDirectoryCommand::getDirectoryString(ACTIVE_DRIVE_INDEX);
	if(activeDrive)
	{
		if(SetCurrentDirectory(activeDrive))
		{
			MESSENGER_LOG(("activeDrive set to [%s]\n",activeDrive));
		}
		else
		{
			MESSENGER_LOG_WARNING(("unable to set activeDrive, activeDrive is [%s]\n",activeDrive));
		}
	}
	else
	{
		MESSENGER_LOG_WARNING(("unable to set activeDrive, activeDrive is NULL\n"));
	}

	MayaMeshReader::clearGlobalVertexIndexer();

	ms_command = "";
	s_branchName = "";

	// see if a branch was specified
	MStatus  status;
	const unsigned argCount = argList.length(&status);

	for (unsigned argIndex = 0; argIndex < argCount; ++argIndex)
	{
		MString argName = argList.asString(argIndex, &status);
		if (!status)
		{
			MESSENGER_LOG_ERROR(("failed to get arg [%u] as string\n", argIndex));
			return status;
		}

		IGNORE_RETURN( argName.toLowerCase() );

		if (argName == ExportArgs::cs_branchArgName)
		{
			s_branchName = argList.asString(argIndex + 1, &status).asChar();
			// fixup argIndex
			++argIndex;
		}
	}

	int result = DialogBox(GetPluginInstanceHandle(), MAKEINTRESOURCE (IDD_EXPORT), NULL, exportDialogProc); //lint !e1924 C-style cast (this conveinent macro uses one "internally")

	if(result == IDOK)
	{
		//send the built command to Maya
		IGNORE_RETURN(MGlobal::executeCommand(ms_command.c_str()));
	}

	return MStatus(MStatus::kSuccess);
}

// ======================================================================

/**
 * This is the Win32 proceduce that the first GUI dialog box presented in an Alienbrain import uses.  It
 * presents the user with a list of asset types (built from a file), and the user must select one.
 */
BOOL CALLBACK exportDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	UNREF(lParam); 

	HWND exportStaticMeshRadio            = GetDlgItem(hDlg, IDC_EXPORT_STATIC_MESH);
	HWND exportSkeletalMeshGeneratorRadio = GetDlgItem(hDlg, IDC_EXPORT_SKELETAL_MESH_GENERATOR);
	HWND exportSkeletonRadio              = GetDlgItem(hDlg, IDC_EXPORT_SKELETON);
	HWND exportAnimationRadio             = GetDlgItem(hDlg, IDC_EXPORT_ANIMATION);
	HWND exportSATRadio                   = GetDlgItem(hDlg, IDC_EXPORT_SAT);

	HWND exportToSourceControlYes         = GetDlgItem(hDlg, IDC_EXPORT_TO_SOURCE_CONTROL_YES);
	HWND exportToSourceControlNo          = GetDlgItem(hDlg, IDC_EXPORT_TO_SOURCE_CONTROL_NO);

	HWND exportToNewChangeList            = GetDlgItem(hDlg, IDC_CREATE_CHANGELIST);

	HWND showViewerAfterExportYes         = GetDlgItem(hDlg, IDC_SHOW_VIEWER);

	HWND exportBranch                     = GetDlgItem(hDlg, IDC_COMBO_BRANCH);

	HWND animationExportUncompressedYes   = GetDlgItem(hDlg, IDC_EXPORT_ANIMATION_UNCOMPRESSED);

	HWND fixPobCrc                        = GetDlgItem(hDlg, IDC_EXPORT_FIX_POB_CRC);

	switch(iMsg)
	{
		case WM_INITDIALOG:
		{
			//initialize the radio buttons
			IGNORE_RETURN(SendMessage(exportStaticMeshRadio,   BM_SETCHECK,  1, 0));
			IGNORE_RETURN(SendMessage(exportToSourceControlNo, BM_SETCHECK,  1, 0));

			const std::vector<std::string> & validBranches = ExportManager::getValidBranches();
			for(std::vector<std::string>::const_iterator i = validBranches.begin(); i != validBranches.end(); ++i)
			{
				IGNORE_RETURN(SendMessage(exportBranch, CB_ADDSTRING, 0, reinterpret_cast<long>((*i).c_str())));
			}

			// if the viewer path has been set, enable the launch viewer checkbox, otherwise disable it
			std::string viewerFullPath = SetDirectoryCommand::getDirectoryString(VIEWER_LOCATION_INDEX);

			// init show viewer checkbox and make sure it's enabled
			EnableWindow(showViewerAfterExportYes,! viewerFullPath.empty());
			IGNORE_RETURN(SendMessage(showViewerAfterExportYes, BM_SETCHECK, 1, 0)); // default to no if exporting locally

			// disable the export uncompressed anim and set it to unchecked as default
			IGNORE_RETURN(SendMessage(animationExportUncompressedYes, BM_SETCHECK, 0, 0));
			IGNORE_RETURN(EnableWindow(animationExportUncompressedYes, false));

			if (s_branchName.empty())
			{
				IGNORE_RETURN(EnableWindow(exportToNewChangeList, false));
			}
			else
			{
				// set branch name to arg and disable branch selection if a branch was specified
				int index = SendMessage(exportBranch, CB_ADDSTRING, 0, reinterpret_cast<long>(s_branchName.c_str()));
				IGNORE_RETURN(SendMessage(exportBranch, CB_SETCURSEL, index, 0));
				IGNORE_RETURN(EnableWindow(exportBranch, false));

				// set export to source control flag
				IGNORE_RETURN(SendMessage(exportToSourceControlNo, BM_SETCHECK,  0, 0));
				IGNORE_RETURN(SendMessage(exportToSourceControlYes, BM_SETCHECK,  1, 0));
				IGNORE_RETURN(SendMessage(showViewerAfterExportYes, BM_SETCHECK, 0, 0)); // default to no if exporting to SC

				// enable the create changelist check box
				IGNORE_RETURN(EnableWindow(exportToNewChangeList, true));

				// set the default check state to false if there is a valid changelist open
				IGNORE_RETURN(SendMessage(exportToNewChangeList, BM_SETCHECK,  !PerforceImporter::validateCurrentChangelist(), 0));
				
				
			}

			return TRUE;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				//ok
				case IDOK:
				{
					int exportToSourceControlYesChecked = SendMessage(exportToSourceControlYes,     BM_GETCHECK, 0, 0);
					const int showViewerAfterExportChecked = SendMessage(showViewerAfterExportYes,  BM_GETCHECK, 0, 0);
					const int animationExportUncompressedChecked = SendMessage(animationExportUncompressedYes, BM_GETCHECK, 0,0);
					const int fixPobCrcChecked = SendMessage(fixPobCrc, BM_GETCHECK, 0, 0);
					
					std::string command;
					//determine the export process
					const int staticChecked           = SendMessage(exportStaticMeshRadio,            BM_GETCHECK, 0, 0);
					const int meshGeneratorChecked    = SendMessage(exportSkeletalMeshGeneratorRadio, BM_GETCHECK, 0, 0);
					const int skeletonChecked         = SendMessage(exportSkeletonRadio,              BM_GETCHECK, 0, 0);
					const int animationChecked        = SendMessage(exportAnimationRadio,             BM_GETCHECK, 0, 0);
					const int SATChecked              = SendMessage(exportSATRadio,                   BM_GETCHECK, 0, 0);
					const int createChangelistChecked = SendMessage(exportToNewChangeList,            BM_GETCHECK, 0, 0);


					std::string branch = ExportArgs::cs_currentBranch;

					// see if a branch was passed in

					if (s_branchName.empty())
					{
						const int selectedBranch       = SendMessage(exportBranch,                     CB_GETCURSEL, 0, 0);

						if(selectedBranch == -1)
						{
							if(exportToSourceControlYesChecked)
							{
								IGNORE_RETURN(MESSENGER_MESSAGE_BOX(NULL, "Please select a branch.", "No branch selected", MB_OK));
								return false;
							}
							else
								branch = ExportArgs::cs_noneBranch;
						}
						else
						{
							char branchName[256];
							SendMessage(exportBranch, CB_GETLBTEXT, selectedBranch, reinterpret_cast<LPARAM>(branchName));
							branch = branchName;
						}
					}
					else
					{
						// use the branch that was passed in
						branch = s_branchName;
					}

					if (fixPobCrcChecked && !exportToSourceControlYesChecked)
					{
						IGNORE_RETURN(MESSENGER_MESSAGE_BOX(NULL, "You must export to source control to fix pob crc.", "Not exporting to source control", MB_OK));
						return false;
					}

					if(SATChecked)
						command = s_exportSATStr           + " ";
					else if(staticChecked)
						command = s_exportStaticMeshStr    + " ";
					else if(meshGeneratorChecked)
						command = s_exportMeshGeneratorStr + " ";
					else if(skeletonChecked)
						command = s_exportSkeletonStr      + " ";
					else if(animationChecked)
						command = s_exportAnimationStr     + " ";

					//all these have to be interactive, so add that flag
					command += ExportArgs::cs_interactiveArgName.asChar();
					command += " ";

					if(animationChecked && animationExportUncompressedChecked)
					{
						command += "-nocompress ";
					}

					if(staticChecked && fixPobCrcChecked)
					{
						command += "-fixpobcrc ";
					}

					//add the commit to source control flag if they chose that option
					if(exportToSourceControlYesChecked)
					{
						command += ExportArgs::cs_branchArgName.asChar();
						command += " ";
						command += branch + " ";
						command += ExportArgs::cs_submitArgName.asChar();

						if (createChangelistChecked)
						{
							command += " ";
							command += ExportArgs::cs_createNewChangelistArgName.asChar();
						}
					}
					else
					{
						// don't show viewer if an animation or meshGenerator is being exported
						if(!animationChecked && !meshGeneratorChecked && showViewerAfterExportChecked)
						{
							command += ExportArgs::cs_showViewerAfterExport.asChar();
						}
					}
					ExportCommand::ms_command = command;

					IGNORE_RETURN(EndDialog(hDlg, IDOK));
					return TRUE;
				}
				//cancel
				case IDCANCEL:
				{
					IGNORE_RETURN(EndDialog(hDlg, IDCANCEL));
					return TRUE;
				}
				case IDC_EXPORT_SAT:
				case IDC_EXPORT_SKELETON:
				case IDC_EXPORT_STATIC_MESH:
				case IDC_EXPORT_SKELETAL_MESH_GENERATOR:
				case IDC_EXPORT_ANIMATION:
					{
						int meshGeneratorIsChecked = SendMessage(exportSkeletalMeshGeneratorRadio, BM_GETCHECK, 0, 0);
						int animationIsChecked = SendMessage(exportAnimationRadio, BM_GETCHECK, 0, 0);
						int staticMeshIsChecked = SendMessage(exportStaticMeshRadio, BM_GETCHECK, 0, 0);

						// enable launch viewer button if neither of these are checked
						IGNORE_RETURN(EnableWindow(showViewerAfterExportYes, !(meshGeneratorIsChecked || animationIsChecked)));
						
						// enable exporting uncompressed if animation is checked
						IGNORE_RETURN(EnableWindow(animationExportUncompressedYes, animationIsChecked));

						// enable fix pob crc if static mesh is checked
						IGNORE_RETURN(EnableWindow(fixPobCrc, staticMeshIsChecked));
					}
					return TRUE;

				case IDC_EXPORT_TO_SOURCE_CONTROL_YES:
				{
					// set launch viewer to no if export to SC is selected
					int exportToSourceControlYesChecked = SendMessage(exportToSourceControlYes,     BM_GETCHECK, 0, 0);
					if (exportToSourceControlYesChecked)
					{
						IGNORE_RETURN(SendMessage(showViewerAfterExportYes, BM_SETCHECK, 0, 0));
					}

					// enable the create changelist check box if source control is enabled
					IGNORE_RETURN(EnableWindow(exportToNewChangeList, exportToSourceControlYesChecked));

					// set the default check state to TRUE if there is NOT a valid changelist open
					IGNORE_RETURN(SendMessage(exportToNewChangeList, BM_SETCHECK,  exportToSourceControlYesChecked && !PerforceImporter::validateCurrentChangelist(), 0));

					return TRUE;

				}

				default:
					return FALSE;
			}
		}
		default:
		{
			return FALSE;
		}
	}
}

// ======================================================================
#if !NO_DATABASE

void UseAssetDatabaseCommand::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void UseAssetDatabaseCommand::remove()
{
	messenger = 0;
}

// ----------------------------------------------------------------------

void *UseAssetDatabaseCommand::creator()
{
	return new UseAssetDatabaseCommand();
}

// ----------------------------------------------------------------------

MStatus UseAssetDatabaseCommand::doIt(const MArgList &argList)
{
	MStatus status;

	const unsigned argCount = argList.length(&status);

	if(argCount != 1)
		DatabaseImporter::activate(true);

	MString mayaArg = argList.asString(0, &status);
	mayaArg = mayaArg.toLowerCase();

	if(mayaArg == "0" || mayaArg == "false")
		DatabaseImporter::activate(false);
	else if(mayaArg == "1" || mayaArg == "true")
		DatabaseImporter::activate(true);
	else
		MESSENGER_LOG(("Usage: useAssetDatabase <true|false>\n"));

	return MStatus(MStatus::kSuccess);
}
#endif
// ======================================================================
