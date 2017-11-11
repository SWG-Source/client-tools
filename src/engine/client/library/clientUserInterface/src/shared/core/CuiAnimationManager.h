//======================================================================
//
// CuiAnimationManager.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiAnimationManager_H
#define INCLUDED_CuiAnimationManager_H

//======================================================================

class ClientObject;
class StateGraphAnimationController;
class StateHierarchyAnimationController;

//----------------------------------------------------------------------


class CuiAnimationManager
{
public:
	static bool attemptPlayEmote (ClientObject & source, ClientObject * target, const std::string & emoteName);
	static bool isValidEmote     (const ClientObject & source, const ClientObject * target, const std::string & emoteName);
};

//======================================================================

#endif
