//======================================================================
//
// SwgCuiContainerProviderDefault.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiContainerProviderDefault_H
#define INCLUDED_SwgCuiContainerProviderDefault_H

#include "swgClientUserInterface/SwgCuiContainerProvider.h"

class ClientObject;
class CachedNetworkId;
class Container;
template <typename T> class Watcher;

namespace MessageDispatch
{
	class Callback;
}

//======================================================================

class SwgCuiContainerProviderDefault : 
public SwgCuiContainerProvider
{
public:

	SwgCuiContainerProviderDefault ();
	~SwgCuiContainerProviderDefault ();

	typedef std::pair <ClientObject *, ClientObject *> ContainerMsg;
	void                              onAddedToContainer          (const ContainerMsg & msg);
	void                              onRemovedFromContainer      (const ContainerMsg & msg);

	void                              getObjectVector             (ObjectWatcherVector & owv);

	virtual ClientObject *            getContainerObject          ();
	virtual const ClientObject *      getContainerObject          () const;
	ClientObject *                    getContainerSecondaryObject ();
	const ClientObject *              getContainerSecondaryObject () const;
	const std::string &               getSlotName                 () const;

	void                              setContainerObject          (ClientObject * containerObject, const std::string & slotName);
	virtual void                      setObjectSorting            (const IntVector & iv);
	void                              playAppropriateSound        (bool opening);
	void                              onOpeningAndClosing         (bool opening);

	virtual int                       getTotalVolume                 () const;
	virtual int                       getCurrentVolume               () const;
	virtual int                       getTotalVolumeLimitedByParents () const;

	void                              applyStates                 () const;

	void                              updateObjectVector                 ();
	virtual void                      tryToApplyOrdering      (stdvector<NetworkId>::fwd const &ordering);
	virtual void                      tryToPlaceItemBefore    (const NetworkId & object, const NetworkId & before);

	enum SoundType
	{
		ST_none,
		ST_default,
		ST_data,
		ST_hopper
	};

private:

	typedef stdset<const ClientObject *>::fwd ObjectSet;
	bool                              populateFromContainer (const ClientObject & containerObject, const ObjectSet & currentObjSet, ObjectSet & actualObjSet);

	void                              getObjectVectorFromSlot            (ObjectWatcherVector & owv);
	void                              checkReleaseContainerOpenReference ();

	CachedNetworkId *                      m_containerId;
	CachedNetworkId *                      m_containerSecondaryId;
	CachedNetworkId *                      m_containerTertiaryId;
	std::string                            m_slotName;
	ObjectWatcherVector *                  m_objects;
	MessageDispatch::Callback *            m_callback;

	SoundType                              m_soundType;

};

//----------------------------------------------------------------------

inline const std::string & SwgCuiContainerProviderDefault::getSlotName                 () const
{
	return m_slotName;
}

//======================================================================

#endif
