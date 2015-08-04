// ======================================================================
//
// CuiInventoryManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiInventoryManager_H
#define INCLUDED_CuiInventoryManager_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

class Object;
class CachedNetworkId;
class ClientObject;
class CreatureObject;

namespace MessageDispatch
{
	class Emitter;
};
class TangibleObject;

//-----------------------------------------------------------------

class CuiInventoryManager
{
public:

	struct Messages
	{
		struct ItemOpenRequest
		{
			typedef std::pair<ClientObject *, std::string> Payload;
		};

		struct ItemOpenRequestNewWindow
		{
			typedef std::pair<ClientObject *, std::string> Payload;
		};

		struct ItemCloseAllInstances
		{
			typedef std::pair<ClientObject *, std::string> Payload;
		};

		struct SplitContainer
		{
			typedef std::pair<NetworkId, NetworkId> Payload;
		};
	};
	
	//-----------------------------------------------------------------

	static void           install ();
	static void           remove  ();
	static void           reset   ();
	static void           update  (float elapsedTime);

	static bool           canPotentiallyEquip (const TangibleObject & obj, const TangibleObject & target);

	static bool           unequipItem    (TangibleObject & obj, TangibleObject & target);
	static bool           equipItem      (TangibleObject & obj, TangibleObject & target, size_t arrangementId);
	/** pick the least occupied arrangement */
	static bool           equipItem      (TangibleObject & obj, TangibleObject & target);
	static bool           itemIsEquipped (const TangibleObject & obj, const TangibleObject & target);

	static bool           findLeastOccupiedArrangementForEquipping (const TangibleObject & obj, TangibleObject & target, size_t & arrangementId, int & requiredVolume);

	static void           requestItemOpen         (ClientObject & obj, const std::string & slotname, int x, int y, bool newWindow, bool ignoreResult);
	static void           handleItemOpen          (ClientObject & obj, const std::string & slotname, int x, int y, bool newWindow, int sequence);
	static void           notifyItemClosed        (ClientObject & obj, const std::string & slotname);
	static void           closeAllInstancesOfItem (ClientObject & obj, const std::string & slotname);

	static bool           isNestedDatapad     (const ClientObject & obj, const CreatureObject * owner = 0);
	static bool           isNestedInventory   (const ClientObject & obj, const CreatureObject * owner = 0);
	static bool           isNestedEquipped    (const ClientObject & obj, const CreatureObject * owner = 0, bool specialsReturn = false);
	static bool           isNestedInContainer (const ClientObject & obj, const ClientObject & container, const Object ** immediateSubContainer = 0);

	static bool           isOnCreature        (const ClientObject & obj, const CreatureObject * owner = 0);

	static bool           isPlayerInventory   (const Object & obj);
	static ClientObject * getPlayerInventory  ();
	static ClientObject * getPlayerDatapad    ();

	static bool           isNestedAppearanceInventory(const ClientObject & obj, const CreatureObject * owner = 0);
	static bool           isPlayerAppearanceInventory(const Object & obj);
	static ClientObject * getPlayerAppearanceInventory ();
	
	static void           pickupObject        (const NetworkId & id);
	static void           equipObject         (const NetworkId & id);
	static void           unequipObject       (const NetworkId & id);
	static void           dropObject          (const NetworkId & id);
	static void           destroyObject       (const NetworkId & id, bool confirmed);

	static void           splitContainer      (const NetworkId & id);

	static void           equipAppearanceItem   (const NetworkId & id);
	static void           unequipAppearanceItem (const NetworkId & id);
		
private:
	CuiInventoryManager (const CuiInventoryManager & rhs);
	CuiInventoryManager & operator= (const CuiInventoryManager & rhs);
};

// ======================================================================

#endif
