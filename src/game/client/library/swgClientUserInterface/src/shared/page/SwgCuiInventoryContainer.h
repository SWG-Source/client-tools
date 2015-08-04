// ======================================================================
//
// SwgCuiInventoryContainer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiInventoryContainer_H
#define INCLUDED_SwgCuiInventoryContainer_H

// ======================================================================

#include "UIEventCallback.h"
#include "UICursor.h"
#include "UITableModel.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiContainerSelectionChanged.h"
#include "sharedObject/CachedNetworkId.h"
#include "swgClientUserInterface/SwgCuiInventory.h"

class ClientObject;
class Container;
class CuiDragInfo;
class CuiIconManagerCallback;
class CuiMoney;
class CuiWidget3dObjectListViewer;
class Object;
class SwgCuiContainerProvider;
class SwgCuiInventoryContainerDetails;
class SwgCuiInventoryContainerIcons;
class UIButton;
class UIText;
class UITextStyle;
class UIVolumePage;
class VolumeContainer;

template <typename T> class Watcher;

namespace MessageDispatch
{
	class Emitter;
	class Receiver;
	template <typename MessageType, typename IdentifierType> class Transceiver;
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiInventoryContainerFilter
{
public:
	virtual bool overrideMessage (const UIWidget * , const UIMessage & ) const { return false; }
	CuiIconManagerCallback *     iconCallback;
	virtual ~SwgCuiInventoryContainerFilter () = 0 {};

	explicit SwgCuiInventoryContainerFilter (CuiIconManagerCallback * _callback = 0) :
	iconCallback (_callback)
	{
	}
};

//-----------------------------------------------------------------

class SwgCuiInventoryContainer:
public CuiMediator,
public UIEventCallback
{
public:

	enum Type
	{
		T_inventorySelf,
		T_datapadSelf,
		T_craftIngredients,
		T_draftSchematics,
		T_waypointsSelf,
		T_poi
	};

	struct Messages
	{
		typedef std::pair<int, CachedNetworkId> PositionPair;

		struct TargetChanged
		{
			typedef bool Payload;
		};

		struct ListReset
		{
			typedef bool Payload;
		};

		struct Insert
		{
			typedef PositionPair Payload;
		};

		struct Remove
		{
			typedef PositionPair Payload;
		};
	};

	struct Transceivers
	{
		typedef MessageDispatch::Transceiver <const Messages::TargetChanged::Payload &, Messages::TargetChanged> TargetChanged;
		typedef MessageDispatch::Transceiver <const Messages::ListReset::Payload &,     Messages::ListReset>     ListReset;
		typedef MessageDispatch::Transceiver <const Messages::Insert::Payload &,        Messages::Insert>        Insert;
		typedef MessageDispatch::Transceiver <const Messages::Remove::Payload &,        Messages::Remove>        Remove;
	};

	enum ViewType
	{
		VT_icon,
		VT_detail
	};

	typedef                       Watcher<ClientObject>                                    ObjectWatcher;
	typedef                       stdvector<ObjectWatcher>::fwd                            ObjectWatcherVector;
	typedef                       stdvector<int>::fwd                                      IntVector;
	typedef                       stdmap<CachedNetworkId, CuiWidget3dObjectListViewer *>::fwd ViewerMap;

	explicit                      SwgCuiInventoryContainer     (UIPage & page, Type type = T_inventorySelf);

	bool                          OnMessage                    (UIWidget *context, const UIMessage & msg );
	void                          OnButtonPressed              (UIWidget *context );

	void                          update                       (float deltaTimeSecs);

	void                          setContainerObject           (ClientObject * containerObject, const std::string & slotname);

	void                          updateContents               ();

	const ClientObject *          getContainerObject           () const;
	ClientObject *                getContainerObject           ();
	const std::string &           getContainerSlotName         () const;
	const Unicode::String &       getContainerLocalizedName    () const;

	void                          setFilter                    (const SwgCuiInventoryContainerFilter * filter);
	void                          setContainerProvider         (SwgCuiContainerProvider * provider);

	UIVolumePage *                getVolumePage                ();

	const ObjectWatcherVector &   getObjects                   () const;
	void                          setSelection                 (const ObjectWatcherVector & cidv);
	const ObjectWatcherVector &   getSelection                 () const;
	const ClientObject *          getLastSelection             () const;
	ClientObject *                getLastSelection             ();

	void                          setObjectSorting             (const IntVector & iv);

	const ViewerMap &             getViewers                   () const;

	Transceivers::ListReset &     getTransceiverListReset      ();

	ClientObject *                getObjectAtPosition          (int index);
	CuiWidget3dObjectListViewer * getWidgetAtPosition          (int index);

	CuiContainerSelectionChanged::TransceiverType &     getTransceiverSelection ();

	CuiWidget3dObjectListViewer *     getViewerForObject           (ClientObject & obj, bool create = true);

	bool                          handleDoubleClick            (CuiWidget3dObjectListViewer & viewer) const;
	bool                          handleRadialMenu             (CuiWidget3dObjectListViewer & viewer, const UIPoint & pt) const;
	void                          handleDrop                   (const CuiDragInfo & cdinfo, ClientObject & destination, const std::string & slotname, bool tryDragManager = true, bool askOnGiveItem = false);

	bool                          preHandleDragOver            (const CuiDragInfo & cdinfo, bool & ok);
	bool                          preHandleDrop                (const CuiDragInfo & cdinfo);
	bool                          preHandleDragStart           (const CuiDragInfo & cdinfo);

	ViewType                      getViewType                  () const;
	void                          setViewType                  (ViewType vt);

	Type                          getType                      () const;
	void                          setType                      (Type type);

	void                          setOwnedByUI                 (bool ownedByUI);

	void setCurrentSortOnColumnState(int const column, UITableModel::SortDirection const direction);
	void getCurrentSortOnColumnState(int & column, UITableModel::SortDirection & direction) const;
	int getDefaultSortColumn() const;

	void clearViewers();
	void removeViewer(CachedNetworkId const & id, CuiWidget3dObjectListViewer * viewer);

	
	void						  setInventoryType             (SwgCuiInventory::InventoryType type);

	void                          handleDropThrough            (UIWidget * const obj);

	void                       openSelectedRadial();
   
protected:
	virtual void               performActivate ();
	virtual void               performDeactivate ();

private:

	void                       updateContainerInfo ();

	                          ~SwgCuiInventoryContainer ();
	                           SwgCuiInventoryContainer ();
	                           SwgCuiInventoryContainer (const SwgCuiInventoryContainer & rhs);
	SwgCuiInventoryContainer & operator= (const SwgCuiInventoryContainer & rhs);

	void                       updateChildFromSlot ();
	ClientObject *             findDropDestination (UIWidget & context);
	void                       clearObjectsAndViewers ();

	void                       updateViewerStates ();
	void                       updateTooltipForObject             (ClientObject & obj, bool requestServerUpdate);
	ClientObject *             findParentContainer () const;

	//Loads the m_inventoryOrder member variable from the cuisettings string
	void                       loadInventoryOrder ();                       

	//Takes the current inventory order and saves it out to the cuisettings string
	void                       saveInventoryOrder ();

	UIText *                               m_label;

	UICursor *                             m_equipCursor;

	bool                                   m_contentsDirty;
	bool                                   m_forceUpdate;

	UIButton *                             m_buttonUp;
	CuiWidget3dObjectListViewer *          m_viewer;

	MessageDispatch::Callback *            m_callback;

	const SwgCuiInventoryContainerFilter * m_filter;

	SwgCuiInventoryContainerDetails *      m_details;
	SwgCuiInventoryContainerIcons   *      m_icons;

	ObjectWatcherVector *                  m_objects;
	ObjectWatcherVector *                  m_selection;
	ViewerMap *                            m_viewers;

	Transceivers::ListReset *              m_transceiverListReset;
	CuiContainerSelectionChanged::TransceiverType *     m_transceiverSelection;

	UIButton *                             m_buttonView;
	UIButton *                             m_buttonLootAll;
	UIButton *                             m_defaultButton;

	Unicode::String                        m_containerLocalizedName;

	ViewType                               m_viewType;

	SwgCuiContainerProvider *              m_containerProvider;

	std::string                            m_objectDragType;
	bool                                   m_disableRadial;
	bool                                   m_disabledRadialAllowsOpen;

	Type                                   m_type;

	CuiMoney *                             m_money;

	UIPage *                               m_capacityBar;
	UIPage *                               m_capacityContainedLimitBar;
	UIText *                               m_capacityText;

	bool                                   m_ownedByUI;
	UIPage *                               m_capacityArea;

	stdvector<NetworkId>::fwd              m_inventoryOrder;

	bool                                   m_saveInventoryOrder;
};

//----------------------------------------------------------------------

inline const SwgCuiInventoryContainer::ObjectWatcherVector & SwgCuiInventoryContainer::getSelection () const
{
	return *NON_NULL (m_selection);
}

//----------------------------------------------------------------------

inline SwgCuiInventoryContainer::Transceivers::ListReset &     SwgCuiInventoryContainer::getTransceiverListReset ()
{
	return *NON_NULL (m_transceiverListReset);
}

//----------------------------------------------------------------------

inline CuiContainerSelectionChanged::TransceiverType &     SwgCuiInventoryContainer::getTransceiverSelection ()
{
	return *NON_NULL (m_transceiverSelection);
}

//----------------------------------------------------------------------

inline const SwgCuiInventoryContainer::ViewerMap & SwgCuiInventoryContainer::getViewers   () const
{
	return *NON_NULL (m_viewers);
}

//----------------------------------------------------------------------

inline const Unicode::String & SwgCuiInventoryContainer::getContainerLocalizedName    () const
{
	return m_containerLocalizedName;
}

//----------------------------------------------------------------------

inline SwgCuiInventoryContainer::ViewType SwgCuiInventoryContainer::getViewType () const
{
	return m_viewType;
}

//----------------------------------------------------------------------

inline SwgCuiInventoryContainer::Type SwgCuiInventoryContainer::getType () const
{
	return m_type;
}

//----------------------------------------------------------------------

inline void SwgCuiInventoryContainer::setOwnedByUI(bool ownedByUI)
{
	m_ownedByUI = ownedByUI;
}

// ======================================================================

#endif
