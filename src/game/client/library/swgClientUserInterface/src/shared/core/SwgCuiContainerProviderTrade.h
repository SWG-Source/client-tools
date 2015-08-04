//======================================================================
//
// SwgCuiContainerProviderTrade.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiContainerProviderTrade_H
#define INCLUDED_SwgCuiContainerProviderTrade_H

//======================================================================

#include "swgClientUserInterface/SwgCuiContainerProvider.h"

#include <utility>	// for std::pair

class ClientObject;
class CreatureObject;
template <typename T> class Watcher;
class CachedNetworkId;
class NetworkId;

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiContainerProviderTrade : 
public SwgCuiContainerProvider
{
public:

	typedef stdvector<CachedNetworkId>::fwd CachedNetworkIdVector;

	SwgCuiContainerProviderTrade ();
	~SwgCuiContainerProviderTrade ();

	typedef std::pair <ClientObject *, ClientObject *> ContainerMsg;
	void                              onDraftsChanged             (const CreatureObject & msg);
	void                              getObjectVector             (ObjectWatcherVector & owv);
	virtual void                      setObjectSorting            (const IntVector & iv);

	void                              onAddItem                   (const std::pair<bool, NetworkId> & payload);
	void                              onRemoveItem                (const std::pair<bool, NetworkId> & payload);

	void                              setIsSelf                   (bool b);

	virtual bool                      handleDragOver              (const CuiDragInfo & cdinfo, bool & ok);
	virtual bool                      handleDrop                  (const CuiDragInfo & cdinfo);
	virtual bool                      handleDragStart             (const CuiDragInfo & cdinfo);

private:

	typedef stdset<const ClientObject *>::fwd ObjectSet;
	bool                              populateFromTrade           (const ObjectSet & currentObjSet, ObjectSet & actualObjSet);
	void                              updateObjectVector          ();
	bool                              isDropOk                    (const ClientObject & obj) const;

	ObjectWatcherVector *                  m_objects;
	MessageDispatch::Callback *            m_callback;

	bool                                   m_isSelf;
};

//======================================================================

#endif
