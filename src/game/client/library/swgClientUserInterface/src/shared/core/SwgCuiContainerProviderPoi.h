//======================================================================
//
// SwgCuiContainerProviderPoi.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiContainerProviderPoi_H
#define INCLUDED_SwgCuiContainerProviderPoi_H

//======================================================================

#include "swgClientUserInterface/SwgCuiContainerProvider.h"

//for inner class
#include "clientUserInterface/CuiPoiManager.h"

class ClientObject;
class CreatureObject;
template <typename T> class Watcher;

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiContainerProviderPoi : 
public SwgCuiContainerProvider
{
public:

	SwgCuiContainerProviderPoi ();
	~SwgCuiContainerProviderPoi ();

	typedef std::pair <ClientObject *, ClientObject *> ContainerMsg;
	void                              getObjectVector             (ObjectWatcherVector & owv);
	virtual void                      setObjectSorting            (const IntVector & iv);

private:

	typedef stdset<const ClientObject *>::fwd ObjectSet;
	bool                              populateFromData            ();
	void                              updateObjectVector          ();

	ObjectWatcherVector * m_objects;
	std::string           m_sceneId;
};

//======================================================================

#endif
