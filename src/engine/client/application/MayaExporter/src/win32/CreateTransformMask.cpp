// ======================================================================
//
// CreateTransformMask.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstMayaExporter.h"
#include "CreateTransformMask.h"

#include "ExportArgs.h"
#include "MayaCompoundString.h"
#include "MayaUtility.h"
#include "maya/MArgList.h"
#include "maya/MDagPath.h"
#include "maya/MGlobal.h"
#include "maya/MSelectionList.h"
#include "maya/MStatus.h"
#include "Messenger.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"

#include <commdlg.h>
#include <set>
#include <string>

// ======================================================================

struct CreateTransformMask::Arguments
{
	std::string  outputPathName;
};

// ======================================================================

const Tag TAG_INGR = TAG(I,N,G,R);
const Tag TAG_XFMS = TAG(X,F,M,S);

const char *const CreateTransformMask::cms_iffFilter = "IFF Files (*.iff)\0*.iff\0All Files (*.*)\0*.*\0\0";
const int         CreateTransformMask::cms_iffSize   = 64 * 1024;

// ======================================================================

Messenger *CreateTransformMask::messenger;

// ======================================================================

void CreateTransformMask::install(Messenger *newMessenger)
{
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void CreateTransformMask::remove()
{
	messenger = 0;
}

// ----------------------------------------------------------------------

void *CreateTransformMask::creator()
{
	return new CreateTransformMask();
}

// ======================================================================

MStatus CreateTransformMask::doIt(const MArgList &argList)
{
	//-- Process arguments.
	Arguments  collectedArgs;

	const bool paSuccess = processArguments(argList, collectedArgs);
	MESSENGER_REJECT_STATUS(!paSuccess, ("failed to process arguments."));

	//-- Collect selected transforms.
	StringSet  transformNames;

	const bool ctSuccess = collectTransformNames(transformNames);
	MESSENGER_REJECT_STATUS(!ctSuccess, ("failed to collect transform names.\n"));

	// Bail out if no transforms were collected.
	if (transformNames.empty())
	{
		MESSENGER_LOG(("No appropriate transforms were selected, ignoring command.\n"));
		return MStatus::kSuccess;
	}

	//-- Get TransformMask filename.
	CrcLowerString  pathName;

	if (!collectedArgs.outputPathName.empty())
	{
		// Get the pathname from the command line arguments.
		pathName.setString(collectedArgs.outputPathName.c_str());
	}
	else
	{
		// Get the pathname from a dialog box.
		const bool gopnSuccess = getOutputPathName(pathName);
		MESSENGER_REJECT_STATUS(!gopnSuccess, ("aborted interactive pathname retrieval.\n"));
	}

	//-- Write TransformMask file.
	const bool wtmSuccess = writeTransformMask(transformNames, pathName);
	MESSENGER_REJECT_STATUS(!wtmSuccess, ("failed to write transform mask.\n"));

	return MStatus::kSuccess;
}

// ======================================================================

bool CreateTransformMask::processArguments(const MArgList &argList, Arguments &collectedArgs)
{
	const unsigned argCount = argList.length();
	for (unsigned i = 0; i < argCount; ++i)
	{
		//-- Get the argument, convert to lower case.
		MString argString = argList.asString(i);
		IGNORE_RETURN(argString.toLowerCase());

		//-- Check for supported arguments.
		if (argString == ExportArgs::cs_filenameArgName)
		{
			MESSENGER_REJECT(i + 1 >= argCount, ("'-filename' specified but a filename was not specified.\n"));
			
			// Get the filename.
			MString filename = argList.asString(i + 1);
			IGNORE_RETURN(filename.toLowerCase());
			collectedArgs.outputPathName = filename.asChar();

			// Skip the filename arg.
			++i;
		}
		else
		{
			// Unknown arg.
			MESSENGER_LOG(("Unsupported argument [%s].\n", argString.asChar()));
			return false;
		}
	}

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------

bool CreateTransformMask::collectTransformNames(StringSet &transformNames)
{
	MStatus         status;
	MSelectionList  activeSelection;

	//-- Get the active selection list.
	status = MGlobal::getActiveSelectionList(activeSelection);
	STATUS_REJECT(status, "MGlobal::getActiveSelectionList()");

	//-- Examine all the nodes in the list.
	MDagPath    dagPath;

	const unsigned int length = activeSelection.length();
	for (unsigned int i = 0; i < length; ++i)
	{
		//-- Get the dependency node.
		status = activeSelection.getDagPath(i, dagPath);
		if (!status)
		{
			//-- Probably not a dag node, ignore it.
			continue;
		}

		//-- Check API type.
		if (dagPath.apiType() != MFn::kJoint)
		{
			// I only care about joints, skip this element.
			continue;
		}

		//-- Check for an ignored node.
		if (MayaUtility::ignoreNode(dagPath))
			continue;

		//-- Extract joint name.
		const std::string  partialPathName(dagPath.partialPathName().asChar());

		const std::string::size_type endOfPathPos = partialPathName.rfind('|');
		const char *completeNodeName = partialPathName.c_str();
		if (static_cast<int>(endOfPathPos) != static_cast<int>(std::string::npos))
			completeNodeName += (endOfPathPos + 1);

		const MayaCompoundString  compoundString(completeNodeName);
		const char *const         cTransformName = compoundString.getComponentString(0).asChar();
		
		//-- Add to transform name list.
		IGNORE_RETURN(transformNames.insert(CrcLowerString(cTransformName)));
	}

	//-- Success.
	return true;
}

// ----------------------------------------------------------------------

bool CreateTransformMask::getOutputPathName(CrcLowerString &outputPathName)
{
#if 0

	// @todo implement the real thing.
	outputPathName.setString("d:\\work\\test.iff");

	//-- Success.
	return true;

#else

	// Handle browse for the file.

	//-- Setup dialog box data.
	OPENFILENAME  ofn;
	char          pathName[1024];
	char          shortFileName[1024];

	memset(&ofn, 0, sizeof(ofn));
	//strcpy(pathName, ms_dialogLastSatPathSelected.c_str());
	pathName[0]      = 0;
	shortFileName[0] = 0;

	ofn.lStructSize    = sizeof(OPENFILENAME);
	ofn.hwndOwner      = 0;
	ofn.lpstrFilter    = cms_iffFilter;
	ofn.nFilterIndex   = 1;
	ofn.lpstrFile      = pathName;
	ofn.nMaxFile       = sizeof(pathName);
	ofn.lpstrFileTitle = shortFileName;
	ofn.nMaxFileTitle  = sizeof(shortFileName);
	ofn.lpstrTitle     = "Specify TransformMask IFF File";
	ofn.Flags          = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt    = ".asg";

	//-- open the dialog
	const BOOL cfdResult = GetSaveFileName(&ofn);
	if (cfdResult != 0)
	{
		// User specified a real file.

		//-- save path to sat most recently selected by artist.
		// ms_dialogLastSatPathSelected = pathName;

		//-- Get the full path.
		outputPathName.setString(pathName);
		return true;
	}
	else
	{
		return false;
	}

#endif
}

// ----------------------------------------------------------------------

bool CreateTransformMask::writeTransformMask(const StringSet &transformNames, const CrcLowerString &pathName)
{
	MESSENGER_LOG(("Writing IFF file [%s]:\n", pathName.getString()));

	//-- Construct the IFF data.
	Iff  iff(cms_iffSize);

	iff.insertForm(TAG_XFMS);
		iff.insertForm(TAG_0000);

			iff.insertChunk(TAG_INGR);

				//-- Write # transform entries.
				iff.insertChunkData(static_cast<int16>(transformNames.size()));

				const StringSet::const_iterator endIt = transformNames.end();
				for (StringSet::const_iterator it = transformNames.begin(); it != endIt; ++it)
				{
					//-- Add transform name entry.
					iff.insertChunkString(it->getString());
					MESSENGER_LOG(("-| added transform [%s].\n", it->getString()));
				}

			iff.exitChunk(TAG_INGR);
	
		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_XFMS);

	//-- Write IFF data to the specified file.
	const bool writeSuccess = iff.write(pathName.getString(), true);
	MESSENGER_REJECT(!writeSuccess, ("failed to write TransformMask file to location [%s].\n", pathName.getString()));

	//-- Success.
	MESSENGER_LOG(("Successfully wrote [%d] bytes to TransformMask file [%s].\n", iff.getRawDataSize(), pathName.getString()));
	return true;
}

// ======================================================================
