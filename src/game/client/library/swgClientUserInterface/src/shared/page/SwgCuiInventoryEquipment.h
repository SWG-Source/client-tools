// ======================================================================
//
// SwgCuiInventoryEquipment.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiInventoryEquipment_H
#define INCLUDED_SwgCuiInventoryEquipment_H

// ======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientUserInterface/CuiContainerSelectionChanged.h"
#include "swgClientUserInterface/SwgCuiInventory.h"

class ClientObject;
class CuiWidget3dObjectListViewer;
class UIButton;
class UIText;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiInventoryEquipment:
public CuiMediator,
public UIEventCallback
{
public:
	explicit                SwgCuiInventoryEquipment  (UIPage & page);

	virtual void            OnButtonPressed           (UIWidget * context);
	void                    setupCharacterViewer      (ClientObject * obj);

	void                    setInventoryType          (SwgCuiInventory::InventoryType type);

protected:

	void                    performActivate ();
	void                    performDeactivate ();

private:
	                       ~SwgCuiInventoryEquipment ();
	SwgCuiInventoryEquipment ();
	SwgCuiInventoryEquipment (const SwgCuiInventoryEquipment & rhs);
	SwgCuiInventoryEquipment & operator= (const SwgCuiInventoryEquipment & rhs);

	CuiWidget3dObjectListViewer *     m_characterViewer;

	UIButton *                 m_zoomInButton;
	UIButton *                 m_zoomOutButton;
	UIButton *                 m_leftButton;
	UIButton *                 m_rightButton;
	UIButton *                 m_centerButton;

	UIText *                   m_label;

	MessageDispatch::Callback * m_callback;
};

// ======================================================================

#endif
