// ======================================================================
//
// SwgClientSetup.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgClientSetup_H
#define INCLUDED_SwgClientSetup_H

// ======================================================================

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

// ======================================================================

namespace SwgClientSetupNamespace
{	
	int const cms_languageCodeEnglish = 0;
	int const cms_languageCodeJapanese = 1;
}

class SwgClientSetupApp : public CWinApp
{
public:

	static void configure ();

	static TCHAR const * getSendMinidumpsString ();
	static bool getSendMinidumps ();
	static void setSendMinidumps (bool sendMinidumps);

	static TCHAR const * getSendStationIdString ();

	static TCHAR const * getAutomaticallySendHardwareInformationString ();
	static bool getAutomaticallySendHardwareInformation ();
	static void setAutomaticallySendHardwareInformation (bool automaticallySendHardwareInformation);

	static TCHAR const * getAllowContactString ();

	static TCHAR const * getThankYouString ();

public:
	SwgClientSetupApp();

	//{{AFX_VIRTUAL(SwgClientSetupApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(SwgClientSetupApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	//void detectAndSendMinidumps ();
	//void detectAndSendHardwareInformation ();
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
