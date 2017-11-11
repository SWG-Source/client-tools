// ======================================================================
//
// ClientIncubatorManager.h
// copyright (c) 2007 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientIncubatorManager_H
#define INCLUDED_ClientIncubatorManager_H

class NetworkId;
class Object;

class ClientIncubatorManager
{
public:
	static void install();

	enum Slot
	{
		slot_invalid = -1,
		slot_slot1 = 0,
		slot_slot2,
		slot_slot3,
		slot_slot4,

		slot_size
	};

	static const NetworkId& getNetworkId(ClientIncubatorManager::Slot slot);
	static void setNetworkId(ClientIncubatorManager::Slot slot, const NetworkId& networkId);
	static ClientIncubatorManager::Slot getSlotIndexForNetworkId(const NetworkId& networkId);
	static bool canEnzymeGoInSlot(const Object* obj, ClientIncubatorManager::Slot slot);
	static void resetSlots();

private:
	//disabled
	ClientIncubatorManager (ClientIncubatorManager const & rhs);
	ClientIncubatorManager & operator= (ClientIncubatorManager const & rhs);

private:
	static void remove();
};

// ======================================================================

#endif
