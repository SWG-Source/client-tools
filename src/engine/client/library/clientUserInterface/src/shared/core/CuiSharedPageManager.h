//======================================================================
//
// CuiSharedPageManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiSharedPageManager_H
#define INCLUDED_CuiSharedPageManager_H

class UIPage;

//======================================================================

class CuiSharedPageManager
{
public:

	static bool hasActiveMediators    (const std::string & groupName, const std::string & ignoreMediator);
	static void checkPageVisibility   (const std::string & groupName, UIPage & page, UIPage * ignorePage, const std::string & ignoreMediator, bool wantsVisibility);
	static void registerMediatorType  (const std::string & groupName, const std::string & type);
};

//======================================================================

#endif
