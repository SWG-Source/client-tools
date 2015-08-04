// ======================================================================
//
// SwgCuiResourceSplitter.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiResourceSplitter_H
#define INCLUDED_SwgCuiResourceSplitter_H

#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class CuiWidget3dObjectListViewer;
class UIButton;
class UIText;
class UITextbox;
class UISliderbar;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------
class SwgCuiResourceSplitter : 
public CuiMediator,
public UIEventCallback
{
public:

	SwgCuiResourceSplitter(UIPage &page);


	virtual void performActivate();
	virtual void performDeactivate();

	virtual void OnButtonPressed(UIWidget *context);
	virtual void OnSliderbarChanged(UIWidget *context);
	virtual bool OnMessage(UIWidget *context, const UIMessage &message);

	void         onSplitContainer(CuiInventoryManager::Messages::SplitContainer::Payload const &payload);

private:
	
	MessageDispatch::Callback *  m_callBack;
	UITextbox *                  m_splitAmountTextBox;
	UIText *                     m_splitMaxText;
	UISliderbar *                m_splitAmountSlider;
	UIButton *                   m_okButton;
	UIButton *                   m_cancelButton;
	NetworkId                    m_inventoryNetworkId;
	NetworkId                    m_containerNetworkId;
	CuiWidget3dObjectListViewer *m_viewer;
	bool                         m_initialize;
	UIText *                     m_textName;

	void initialize();
	void requestSplit();

	// Disabled

	~SwgCuiResourceSplitter();
	SwgCuiResourceSplitter(SwgCuiResourceSplitter const &rhs);
	SwgCuiResourceSplitter &operator =(SwgCuiResourceSplitter const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiResourceSplitter_H
