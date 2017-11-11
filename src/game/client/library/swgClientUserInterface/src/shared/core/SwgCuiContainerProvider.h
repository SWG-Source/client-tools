//======================================================================
//
// SwgCuiContainerProvider.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiContainerProvider_H
#define INCLUDED_SwgCuiContainerProvider_H

//======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

class ClientObject;
class SwgCuiContainerProviderFilter;
class CuiDragInfo;

//----------------------------------------------------------------------

class SwgCuiContainerProvider
{
public:

	typedef                       SwgCuiContainerProviderFilter                            Filter;
	typedef                       Watcher<ClientObject>                                    ObjectWatcher;
	typedef                       stdvector<ObjectWatcher>::fwd                            ObjectWatcherVector;
	typedef                       stdvector<int>::fwd                                      IntVector;

	bool                          isContentDirty   () const;
	void                          setContentDirty  (bool b);
	bool                          isProviderDirty  () const;
	void                          setProviderDirty (bool b);

	virtual void                  getObjectVector        (ObjectWatcherVector & owv) = 0;

	virtual ClientObject *        getContainerObject     ();
	virtual const ClientObject *  getContainerObject     () const;

	virtual const std::string &   getSlotName            () const;
	virtual void                  setContainerObject     (ClientObject * containerObject, const std::string & slotName);
	virtual void                  setObjectSorting       (const IntVector & iv);

	virtual void                  onOpeningAndClosing    (bool opening);

	const Filter *                getFilter              () const;
	void                          setFilter              (Filter * filter);

	                              SwgCuiContainerProvider ();
	virtual                      ~SwgCuiContainerProvider () = 0;

	virtual bool                  handleDragOver          (const CuiDragInfo & cdinfo, bool & ok);
	virtual bool                  handleDrop              (const CuiDragInfo & cdinfo);
	virtual bool                  handleDragStart         (const CuiDragInfo & cdinfo);

	virtual int                   getTotalVolume                 () const;
	virtual int                   getCurrentVolume               () const;
	virtual int                   getTotalVolumeLimitedByParents () const;

	virtual void                  applyStates             ();

	virtual void                  tryToApplyOrdering      (stdvector<NetworkId>::fwd const &ordering);
	virtual void                  tryToPlaceItemBefore    (const NetworkId & object, const NetworkId & before);

private:

	bool                                   m_contentDirty;
	bool                                   m_providerDirty;
	Filter *                               m_filter;
};

//----------------------------------------------------------------------

inline bool SwgCuiContainerProvider::isContentDirty () const
{
	return m_contentDirty;
}

//----------------------------------------------------------------------

inline bool SwgCuiContainerProvider::isProviderDirty  () const
{
	return m_providerDirty;
}

//----------------------------------------------------------------------

inline const SwgCuiContainerProvider::Filter * SwgCuiContainerProvider::getFilter () const
{
	return m_filter;
}

//======================================================================

#endif
