//======================================================================
//
// SwgCuiIMEInput.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiIMEInput.h"

#include "clientGame/Game.h"
#include "clientUserInterface/CuiSharedPageManager.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "UIIMEManager.h"
#include "UIList.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UITextboxStyle.h"

//======================================================================

namespace SwgCuiIMEInputNamespace
{
	void SendKey(BYTE const nVirtKey)
	{
		keybd_event( nVirtKey, 0, 0,               0 );
		keybd_event( nVirtKey, 0, KEYEVENTF_KEYUP, 0 );
	}
}

using namespace SwgCuiIMEInputNamespace;

//======================================================================

SwgCuiIMEInput::SwgCuiIMEInput     (UIPage & page) :
CuiMediator ("SwgCuiIMEInput", page),
m_listBox(0)
{
	UIPage * const parent = safe_cast<UIPage *>(getPage ().GetParent ());
	NOT_NULL (parent);

	getCodeDataObject (TUIList, m_listBox, "listCandidate");
		
	setStickyVisible(true);

	CuiSharedPageManager::registerMediatorType (parent->GetName (), CuiMediatorTypes::IMEInput);
}

//----------------------------------------------------------------------

SwgCuiIMEInput::~SwgCuiIMEInput ()
{
}

//----------------------------------------------------------------------

void SwgCuiIMEInput::performActivate   ()
{
	// TODO: Need to revisit this and use
	// UIManager::gUIManager().getUIIMEManager()->GetSelectedCompIndex()
	// to determine the location of the first character begin adjusted by the user
	// For now it is the first character of the composition string, which is probably
	// 99% of the use in any case.
   	m_listBox->AddCallback (this);

	UIWidget * contextWidget = UIManager::gUIManager().getUIIMEManager()->GetContextWidget();
	UIPage * const page = &(getPage());

	if (page)
	{
		//set position based on the ContextWidget being a Text or TextBox, then by cursor position,
		//then make sure it goes above if it won't fit below, and to the left if it won't fit horizontally
		if (contextWidget)
		{
			UIScalar x=0, y=0, charHeight=0;
			UIPoint textLocation = contextWidget->GetWorldLocation();


 			// Multi-line
			if (contextWidget->IsA(TUIText))
			{
				UIText * uiText = (UIText*)contextWidget;
				const UIPoint caratPos = uiText->FindCaratPos(uiText->GetCursorPoint());
				UIPoint actualCaratPos = uiText->GetCaratPos();
				UIPoint scrollLoc = uiText->GetScrollLocation();

				// This can be multiple lines, so make sure the charHeight is correct so all
				// characters which may be able to be changed through the input window are visible
				charHeight = uiText->GetMaximumCharHeight();
				long diff = ((long)((actualCaratPos.y - caratPos.y) / charHeight)) + 1;
				charHeight = (charHeight) * diff;

				x = textLocation.x + caratPos.x - scrollLoc.x;
				y = textLocation.y + caratPos.y + charHeight - scrollLoc.y;
			}
			// Single line
			else if (contextWidget->IsA(TUITextbox))
			{
				UITextbox * uiTextbox = (UITextbox*)contextWidget;
				const UIPoint caratPos = uiTextbox->GetCaratPos();
				UIPoint scrollLoc = uiTextbox->GetScrollLocation();



				UIRect boxRect = uiTextbox->GetWorldRect();
				UITextboxStyle	*currentTextboxStyle = uiTextbox->GetTextboxStyle();
				UIRect textPadding;
				currentTextboxStyle->GetTextPadding( textPadding );

				charHeight = contextWidget->GetSize().y;
				x = textLocation.x + caratPos.x - scrollLoc.x + textPadding.left;
				if (x < boxRect.left)
					x = boxRect.left;
				y = textLocation.y + charHeight;
			}
			else
			{
				DEBUG_WARNING(true, ("SwgCuiIMEInput.performActivate() called with invalid context widget set\n"));
			}

			UISize windowSize = UIManager::gUIManager().GetRootPage()->GetSize();
			UISize pageSize = page->GetSize();
			if (windowSize.y < y + pageSize.y)
				y = y - pageSize.y - charHeight;
			if (windowSize.x < x + pageSize.x)
				x = windowSize.x - pageSize.x;
			page->SetLocation(x, y);
		}

		UIPage * const parent = safe_cast<UIPage *>(page->GetParent ());
		if (parent)
		{
			CuiSharedPageManager::checkPageVisibility (parent->GetName (), *parent, &getPage (), CuiMediatorTypes::IMEInput, true);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiIMEInput::performDeactivate ()
{
	m_listBox->RemoveCallback(this);
	UIPage * const parent = safe_cast<UIPage *>(getPage ().GetParent ());
	if (parent)
		CuiSharedPageManager::checkPageVisibility (parent->GetName (), *parent, &getPage (), CuiMediatorTypes::IMEInput, false);	
	UIIMEManager * const imeManager = UIManager::gUIManager().getUIIMEManager();
	NOT_NULL(imeManager);
	UIWidget * const contextWidget = imeManager->GetContextWidget();
	if(contextWidget)
   	{
		contextWidget->SetFocus();
		contextWidget->SetSelected(true);
		setKeyboardInputActive(true);
	}
}

//--------------------------------------------------------------------------------------

bool SwgCuiIMEInput::OnMessage ( UIWidget *, const UIMessage & msg )
{
	if(msg.IsMouseDownCommand())
	{
		long selectInListBox;
		if(m_listBox->GetRowFromPoint(msg.MouseCoords, selectInListBox))
		{
			UIIMEManager * const imeManager = UIManager::gUIManager().getUIIMEManager();
			NOT_NULL(imeManager);

			int selectInIME = imeManager->GetSelectedCandidate();
			// For Japanese, move the selection to the target row,
			// then send Right, then send Left.

			BYTE nVirtKey;
			if( selectInListBox > selectInIME )
				nVirtKey = VK_DOWN;
			else
				nVirtKey = VK_UP;
			int const nNumToHit = abs( int( selectInListBox - selectInIME ) );
			for( int nStrike = 0; nStrike < nNumToHit; ++nStrike )
				SendKey( nVirtKey );

			SendKey(VK_RETURN);
		}
	}   
	return false;
}

//======================================================================