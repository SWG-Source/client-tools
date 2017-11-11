// ======================================================================
//
// SwgCuiDataStorage.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiDataStorage_H
#define INCLUDED_SwgCuiDataStorage_H

// ======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class CachedNetworkId;
class ClientObject;
class ClientWaypointObject;
class SwgCuiContainerProviderDefault;
class SwgCuiContainerProviderDraft;
class SwgCuiContainerProviderPoi;
class SwgCuiInventoryContainer;
class SwgCuiInventoryInfo;
class UIButton;
class UIPage;
class UITabbedPane;
class UIText;

namespace MessageDispatch
{
	class Callback;
};

//-----------------------------------------------------------------

class SwgCuiDataStorage :
public CuiMediator,
public UIEventCallback
{
public:
	struct FilterWps;
	struct FilterData;
	struct FilterPoi;
	class CallbackReceiverShowGroupWaypoints;

	explicit SwgCuiDataStorage (UIPage & page);
	~SwgCuiDataStorage ();
		
	void                       OnButtonPressed     (UIWidget *context);
	void                       OnTabbedPaneChanged (UIWidget * Context);
	void                       OnCheckboxSet       (UIWidget * context);
	void                       OnCheckboxUnset     (UIWidget * context);

	void                       setContainerObject  (ClientObject * containerObject);

	void                       queryWaypointColor  (ClientWaypointObject const & cwo);

	void                       onContentsChanged   (const std::pair<ClientObject *, ClientObject *> &);

	void                       updateGroupWaypointTab();

protected:

	void                       performActivate   ();
	void                       performDeactivate ();

	void                       saveSettings () const;
	void                       loadSettings ();

private:
	SwgCuiDataStorage (const SwgCuiDataStorage & rhs);
	SwgCuiDataStorage & operator= (const SwgCuiDataStorage & rhs);

	void populate();

	SwgCuiInventoryInfo *        m_infoMediator;
	SwgCuiInventoryContainer *   m_containerWaypoints;
	SwgCuiInventoryContainer *   m_containerData;
	SwgCuiInventoryContainer *   m_containerPoi;
	SwgCuiInventoryContainer *   m_containerDraft;

	UIPage *                     m_capacityBar;
	UIText *                     m_capacityLabel;
	UIPage *                     m_capacityBarWp;
	UIText *                     m_capacityLabelWp;
	UIButton *                   m_buttonNewWaypoint;
	UITabbedPane *               m_tabs;
	UICheckbox *                 m_waypointsCurrentPlanetOnly;
	UICheckbox *                 m_waypointsSingleWaypointMode;
	UICheckbox *                 m_waypointsOnScreen;

	MessageDispatch::Callback *  m_callback;
	bool                         m_contentsDirty;

	FilterWps *                  m_filterWps;
	FilterData *                 m_filterData;
	FilterPoi *                  m_filterPoi;
	CachedNetworkId *            m_containerObject;

	SwgCuiContainerProviderDefault * m_containerProviderWaypoints;
	SwgCuiContainerProviderDefault * m_containerProviderData;
	SwgCuiContainerProviderPoi *     m_containerProviderPoi;
	SwgCuiContainerProviderDraft *   m_containerProviderDraft;
	CallbackReceiverShowGroupWaypoints *m_callbackReceiverShowGroupWaypoints;
};

// ======================================================================

#endif
