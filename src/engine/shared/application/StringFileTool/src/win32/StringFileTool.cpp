// ======================================================================
//
// StringFileTool.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstStringFileTool.h"
#include "StringFileTool.h"

#include "DialogStringFileDiff.h"
#include "DialogStringFileMerge.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedThread/SetupSharedThread.h"
#include "StringTable.h"

#include "UnicodeUtils.h"

// ======================================================================

namespace StringFileToolNamespace
{
	unsigned short space = Unicode::narrowToWide(" ").c_str()[0];
	char const * const ms_showNames[] =
	{
		"Show All",
		"Show All Differences",
		"Show Not Equal Only",
		"Show Left Differences Only",
		"Show Right Differences Only"
	};

	CString getToken(CString & line)
	{
		line.TrimLeft();
		line.TrimRight();

		if (line.IsEmpty())
			return line;

		int const index = line.Find(space);
		if (index == -1)
		{
			CString const result = line;
			line.Empty();
			return result;
		}

		CString const result = line.Left(index);
		line = line.Right(line.GetLength() - index);
		return result;
	}

	void extractFileNames(CString const & commandLine, CString & originalFileName, CString & theirsFileName, CString & yoursFileName, CString & mergedFileName)
	{
		CString line = commandLine;
		originalFileName = getToken(line);
		theirsFileName = getToken(line);
		yoursFileName = getToken(line);
		mergedFileName = getToken(line);
	}
}

using namespace StringFileToolNamespace;

// ======================================================================

StringFileTool theApp;

// ======================================================================

char const * const * StringFileTool::getShowNames()
{
	return ms_showNames;
}

// ======================================================================

void StringFileTool::convertToUnicodeString(CString string, Unicode::String &unicodeString)
{
	int stringLength = string.GetLength();
	TCHAR *stringBuffer = string.GetBufferSetLength(stringLength);

	for (int i = 0; i < stringLength; i++)
		unicodeString += stringBuffer[i];

	string.ReleaseBuffer();
}

// ======================================================================

StringFileTool::StringFileTool()
{
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(StringFileTool, CWinApp)
	//{{AFX_MSG_MAP(StringFileTool)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL StringFileTool::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	{
		//-- thread
		SetupSharedThread::install ();

		//-- debug
		SetupSharedDebug::install (4096);

		//-- install the engine
		SetupSharedFoundation::Data setupFoundationData (SetupSharedFoundation::Data::D_mfc);
		setupFoundationData.useWindowHandle  = false;
		SetupSharedFoundation::install (setupFoundationData);

		SetupSharedCompression::install ();

		//-- file
		SetupSharedFile::install (false);

		TreeFile::addSearchAbsolute (0);
	}

	CString originalFileName;
	CString theirsFileName;
	CString yoursFileName;
	CString mergedFileName;
	extractFileNames(m_lpCmdLine, originalFileName, theirsFileName, yoursFileName, mergedFileName);
	Unicode::String originalFileNameUnicode;
	Unicode::String theirsFileNameUnicode;
	Unicode::String yoursFileNameUnicode;
	Unicode::String mergedFileNameUnicode;

	convertToUnicodeString(originalFileName, originalFileNameUnicode);
	convertToUnicodeString(theirsFileName, theirsFileNameUnicode);
	convertToUnicodeString(yoursFileName, yoursFileNameUnicode);
	convertToUnicodeString(mergedFileName, mergedFileNameUnicode);

	if (theirsFileName.IsEmpty())
	{
		MessageBox(0, Unicode::narrowToWide("Invalid parameters: \"").c_str() + CString(m_lpCmdLine) + Unicode::narrowToWide("\"; original=").c_str() + originalFileName + Unicode::narrowToWide(" theirs=").c_str() + theirsFileName + Unicode::narrowToWide(" yours=").c_str() + yoursFileName, Unicode::narrowToWide("StringFileTool").c_str(), MB_ICONWARNING | MB_OK);
		return FALSE;
	}

	StringTable originalStringTable;
	if (!originalStringTable.load(Unicode::wideToNarrow(originalFileNameUnicode).c_str()))
	{
		MessageBox(0, Unicode::narrowToWide("Could not open string table ").c_str() + originalFileName, Unicode::narrowToWide("StringFileTool").c_str(), MB_ICONWARNING | MB_OK);
		return FALSE;
	}

	StringTable theirsStringTable;
	if (!theirsStringTable.load(Unicode::wideToNarrow(theirsFileNameUnicode).c_str()))
	{
		MessageBox(0, Unicode::narrowToWide("Could not open string table ").c_str() + theirsFileName, Unicode::narrowToWide("StringFileTool").c_str(), MB_ICONWARNING | MB_OK);
		return FALSE;
	}

	if (yoursFileName.IsEmpty())
	{
//		if (originalFileName.Find(Unicode::narrowToWide("\\en\\").c_str()) == -1 && theirsFileName.Find(Unicode::narrowToWide("\\en\\").c_str()) == -1 && yoursFileName.Find("\\en\\") == -1 && mergedFileName.Find("\\en\\") == -1)
//			MessageBox(0, "This tool is not unicode compliant and should ONLY be used to diff English string files.", "StringFileTool", MB_ICONWARNING | MB_OK);

		//-- We're diffing
		DialogStringFileDiff dlg(originalStringTable, theirsStringTable);

		m_pMainWnd = &dlg;
		dlg.DoModal();
	}
	else
	{
//		if (originalFileName.Find("\\en\\") == -1 && theirsFileName.Find("\\en\\") == -1 && yoursFileName.Find("\\en\\") == -1 && mergedFileName.Find("\\en\\") == -1)
//			MessageBox(0, "This tool is not unicode compliant and should ONLY be used to merge English string files.", "StringFileTool", MB_ICONWARNING | MB_OK);

		StringTable yoursStringTable;
		if (!yoursStringTable.load(Unicode::wideToNarrow(yoursFileNameUnicode).c_str()))
		{
			MessageBox(0, Unicode::narrowToWide("Could not open string table ").c_str() + yoursFileName + Unicode::narrowToWide(".  Merge cancelled.").c_str(), Unicode::narrowToWide("StringFileTool").c_str(), MB_ICONWARNING | MB_OK);
			return FALSE;
		}

		//-- We're merging
		StringTable mergedStringTable(Unicode::wideToNarrow(mergedFileNameUnicode).c_str());
		DialogStringFileMerge dlg(originalStringTable, theirsStringTable, yoursStringTable, mergedStringTable);

		int const nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
			if (!mergedStringTable.save())
			{
				MessageBox(0, Unicode::narrowToWide("Could not write string table ").c_str() + mergedFileName + Unicode::narrowToWide(".  DO NOT TRUST THIS MERGE!").c_str(), Unicode::narrowToWide("StringFileTool").c_str(), MB_ICONWARNING | MB_OK);
				mergedStringTable.save();
			}
		}
		else
		{
			MessageBox(0, Unicode::narrowToWide("Merge cancelled.").c_str(), Unicode::narrowToWide("StringFileTool").c_str(), MB_ICONWARNING | MB_OK);
			mergedStringTable.save();
		}
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

// ----------------------------------------------------------------------

int StringFileTool::ExitInstance() 
{
	SetupSharedFoundation::remove ();
	SetupSharedThread::remove ();
	
	return CWinApp::ExitInstance();
}

// ======================================================================
