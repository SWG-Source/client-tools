//======================================================================
//
// SwgCuiCraftAssemblyDialogOption.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiCraftAssemblyDialogOption_H
#define INCLUDED_SwgCuiCraftAssemblyDialogOption_H

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
//@todo: remove this #include
#include "sharedNetworkMessages/MessageQueueDraftSlots.h"

class UIButton;
class UIList;
class UIText;
class CachedNetworkId;
class ClientObject;

//======================================================================

class SwgCuiCraftAssemblyDialogOption :
public CuiMediator,
public UIEventCallback
{
public:

	explicit SwgCuiCraftAssemblyDialogOption (UIPage & page);

	virtual void            performActivate   ();
	virtual void            performDeactivate ();

	virtual void            OnButtonPressed           (UIWidget * context);
	virtual void            OnGenericSelectionChanged (UIWidget * context);

	typedef MessageQueueDraftSlots::Slot Slot;

	void                    setSlot (const ClientObject & transferringObject, int slotIndex, int optionIndex = -1);

private:
	~SwgCuiCraftAssemblyDialogOption ();
	SwgCuiCraftAssemblyDialogOption (const SwgCuiCraftAssemblyDialogOption &);
	SwgCuiCraftAssemblyDialogOption & operator= (const SwgCuiCraftAssemblyDialogOption &);

	void update ();
	void completeTransfer (int optionIndex);

	UIButton *              m_buttonOk;
	UIButton *              m_buttonCancel;
	UIList *                m_list;
	UIText *                m_text;

	CachedNetworkId *       m_object;
	int                     m_slotIndex;
};

//======================================================================

#endif
