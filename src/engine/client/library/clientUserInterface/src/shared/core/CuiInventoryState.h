//======================================================================
//
// CuiInventoryState.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiInventoryState_H
#define INCLUDED_CuiInventoryState_H

//======================================================================

class CachedNetworkId;
class ClientObject;
template <typename T> class Watcher;

//----------------------------------------------------------------------

class CuiInventoryState
{
public:

	typedef                       Watcher<ClientObject>                                    ObjectWatcher;
	typedef                       stdvector<ObjectWatcher>::fwd                            ObjectWatcherVector;

	typedef stdvector<CachedNetworkId>::fwd        CachedNetworkIdVector;

	static void setInventoryState  (const ObjectWatcherVector & cniv);
	static void getInventoryState  (ObjectWatcherVector & owv);
	static void saveInventoryState ();
};

//======================================================================

#endif
