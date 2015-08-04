//======================================================================
//
// SwgCuiContainerProviderDraft.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiContainerProviderDraft_H
#define INCLUDED_SwgCuiContainerProviderDraft_H

//======================================================================

#include "swgClientUserInterface/SwgCuiContainerProvider.h"

class ClientObject;
class CreatureObject;
template <typename T> class Watcher;

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiContainerProviderDraft : 
public SwgCuiContainerProvider
{
public:

	SwgCuiContainerProviderDraft ();
	~SwgCuiContainerProviderDraft ();

	typedef std::pair <ClientObject *, ClientObject *> ContainerMsg;
	void                              onDraftsChanged             (const CreatureObject & msg);
	void                              getObjectVector             (ObjectWatcherVector & owv);
	virtual void                      setObjectSorting            (const IntVector & iv);

private:

	typedef stdset<const ClientObject *>::fwd ObjectSet;
	bool                              populateFromDrafts (const ObjectSet & currentObjSet, ObjectSet & actualObjSet);
	void                              updateObjectVector                 ();

	ObjectWatcherVector *                  m_objects;
	MessageDispatch::Callback *            m_callback;

};

//======================================================================

#endif
