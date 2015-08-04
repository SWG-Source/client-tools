//======================================================================
//
// CuiDragManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiDragManager_H
#define INCLUDED_CuiDragManager_H

//======================================================================

class CuiDragInfo;
class ClientObject;
class CreatureObject;

class CuiDragManager
{
public:
	static bool isDropOk     (const CuiDragInfo & info, ClientObject & dest, const std::string & slotname);
	static bool handleDrop   (const CuiDragInfo & info, ClientObject & dest, const std::string & slotname, bool askOnGiveItem = false);
	static bool handleDrop   (ClientObject & source, ClientObject & dest, const std::string & slotname, bool askOnGiveItem = false);
	static bool handleAction (const CuiDragInfo & info);

private:

	struct TargetInfo
	{
		CreatureObject * creature;
		ClientObject *   volumeContainerObject;
		ClientObject *   slottedContainerObject;
		std::string      slotName;
		bool             powerup;
		TargetInfo () : creature (0), volumeContainerObject (0), powerup (false) {}
	};

	static bool findTarget (ClientObject & source, ClientObject & dest, TargetInfo & tinfo, bool warning = false);
};

//======================================================================

#endif
