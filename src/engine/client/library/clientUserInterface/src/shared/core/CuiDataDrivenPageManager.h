// ======================================================================
//
// CuiDataDrivenPageManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiDataDrivenPageManager_H
#define INCLUDED_CuiDataDrivenPageManager_H

// ======================================================================

#include "sharedNetworkMessages/ServerUserInterfaceMessages.h"

class CuiDataDrivenPage;
class SuiPageData;
class SuiCreatePageMessage;
class SuiUpdatePageMessage;

//-----------------------------------------------------------------

/**
 *  This class manages server-side UI creation.  This allows scripters to specify some simple
 *  "fill-in-the-blanks" pages without much time being spent by a UI programmer.
 */
class CuiDataDrivenPageManager
{
public:

	typedef std::vector<SuiCreatePage::Command> CommandVector;

	static void install           ();
	static void remove            ();
	static void createPage        (SuiPageData const &);
	static void closePage         (int pageId);
	static void handleSceneChange ();

	static void receiveCreatePageMessage      (SuiCreatePageMessage const &);
	static void receiveForceCloseMessage      (SuiForceClosePage const &);
	static void receiveUpdatePageMessage      (SuiUpdatePageMessage const &);
	
	static void removePage        (CuiDataDrivenPage* page, bool alreadyClosing);

private:
	//disabled
	CuiDataDrivenPageManager            (const CuiDataDrivenPageManager &rhs);
	CuiDataDrivenPageManager& operator= (const CuiDataDrivenPageManager &rhs);

private:

private:
	typedef stdmap<int, CuiDataDrivenPage*>::fwd PageMap;

	static bool                                  ms_installed;
	static PageMap                               ms_pages;
};

// ======================================================================

#endif
