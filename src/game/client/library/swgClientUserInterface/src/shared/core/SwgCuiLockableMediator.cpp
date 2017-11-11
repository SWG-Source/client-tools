//======================================================================
//
// SwgCuiLockableMediator.cpp
// copyright (c) 2008 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiLockableMediator.h"

#include "clientUserInterface/CuiMenuInfoHelper.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiSettings.h"

#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UIPopupMenustyle.h"
#include "UIUtils.h"
#include "UIWidget.h"


namespace SwgCuiLockableMediatorNameSpace
{
	namespace PopupIds
	{
		const std::string lock_window   = "window_lock";
		const std::string unlock_window = "window_unlock";
	}
	namespace Settings
	{
		const std::string popupHelpEnabled = "popupHelpEnabled";
		const std::string userMovable      = "UserMovable";
		const UILowerString unlockedStateFlags("unlockedflags");
	}

	inline void setFlag(uint32 & field, uint32 flag, bool onOff)
	{
		if(onOff)
		{
			field |= flag;
		}
		else
		{
			field &= ~flag;
		}
	}

	inline bool isFlagSet(uint32 field, uint32 flag)
	{
		return (field & flag) != 0;
	}
}

using namespace SwgCuiLockableMediatorNameSpace;


SwgCuiLockableMediator::SwgCuiLockableMediator(const char * const mediatorDebugName, UIPage & newPage):
CuiMediator(mediatorDebugName, newPage),
UIEventCallback(),
m_pageLockFlags(LTF_none),
m_pageToLock(&newPage)
{
	setupUnlockedState();
}

SwgCuiLockableMediator::~SwgCuiLockableMediator(void)
{
	m_pageToLock = NULL;
}

//----------------------------------------------------------------------

void SwgCuiLockableMediator::generateLockablePopup  (UIWidget * context, const UIMessage & msg)
{
	if (!context || !context->IsA(TUIPage) || !m_pageToLock)
		return;

	UIPopupMenu * const pop = new UIPopupMenu(m_pageToLock);

	if (!pop)
		return;


	pop->SetStyle(m_pageToLock->FindPopupStyle());

	if (!getPageIsLocked())
	{
		pop->AddItem(PopupIds::lock_window, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::WINDOW_LOCK, 0));
	}
	else
	{
		pop->AddItem(PopupIds::unlock_window, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::WINDOW_UNLOCK, 0));
	}

	pop->SetLocation(context->GetWorldLocation() + msg.MouseCoords);
	UIManager::gUIManager().PushContextWidget(*pop);
	pop->AddCallback(getCallbackObject());
}

//----------------------------------------------------------------------

void SwgCuiLockableMediator::OnPopupMenuSelection (UIWidget * context)
{		
	if (!context || !context->IsA(TUIPopupMenu) || !m_pageToLock)
		return;

	UIPopupMenu * const pop = safe_cast<UIPopupMenu *>(context);

	if (!pop)
		return;

	const std::string & selection = pop->GetSelectedName();
	const int menuSelection = atoi(selection.c_str());

	if (selection == PopupIds::lock_window || menuSelection == Cui::MenuInfoTypes::WINDOW_LOCK)
	{
		setPageLocked(true);
	}
	else if (selection == PopupIds::unlock_window || menuSelection == Cui::MenuInfoTypes::WINDOW_UNLOCK)
	{
		setPageLocked(false);
	}
}

//----------------------------------------------------------------------

bool SwgCuiLockableMediator::OnMessage(UIWidget *context, const UIMessage & msg)
{
	if(!context || !context->IsA(TUIPage) || !m_pageToLock )
		return true;

	if (msg.Type == UIMessage::RightMouseUp)
	{
		generateLockablePopup(context, msg);
		return false;
	}
	
	return true;
}

//----------------------------------------------------------------------

void SwgCuiLockableMediator::appendPopupOptions (UIPopupMenu * pop)
{
	if (!pop || !m_pageToLock)
		return;

	if (!getPageIsLocked())
	{
		pop->AddItem(PopupIds::lock_window, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::WINDOW_LOCK, 0));
	}
	else
	{
		pop->AddItem(PopupIds::unlock_window, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::WINDOW_UNLOCK, 0));
	}
}

//----------------------------------------------------------------------

void SwgCuiLockableMediator::appendHelperPopupOptions (CuiMenuInfoHelper * menuHelper)
{
	if (!menuHelper || !m_pageToLock)
		return;

	if (!getPageIsLocked())
	{
		menuHelper->addRootMenu(Cui::MenuInfoTypes::WINDOW_LOCK, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::WINDOW_LOCK, 0));
	}
	else
	{
		menuHelper->addRootMenu(Cui::MenuInfoTypes::WINDOW_UNLOCK, Cui::MenuInfoTypes::getLocalizedLabel(Cui::MenuInfoTypes::WINDOW_UNLOCK, 0));
	}
}

//----------------------------------------------------------------------

void SwgCuiLockableMediator::setDefaultWindowResizable(bool b)
{
	setFlag(m_pageLockFlags, LTF_resizable, b);
}

//----------------------------------------------------------------------

void SwgCuiLockableMediator::setDefaultWindowUserMovable(bool b)
{
	setFlag(m_pageLockFlags, LTF_userMovable, b);
}

//----------------------------------------------------------------------

void SwgCuiLockableMediator::setDefaultWindowAcceptsChildMove(bool b)
{
	setFlag(m_pageLockFlags, LTF_acceptsChildMove, b);
}

//----------------------------------------------------------------------

void SwgCuiLockableMediator::setPageToLock(UIPage * page)
{
	m_pageToLock = page;
	setupUnlockedState();
}

//----------------------------------------------------------------------

UIPage * SwgCuiLockableMediator::getPageToLock (void)
{
	return m_pageToLock;
}

//----------------------------------------------------------------------

bool SwgCuiLockableMediator::getPageIsLocked() const
{
	//since things can manually change the properties that make a page
	//'locked' we will only return true if the page is completely locked
	bool const notLocked = (m_pageToLock == NULL)
		|| (isFlagSet(m_pageLockFlags, LTF_userMovable) && m_pageToLock->IsUserMovable())
		|| (isFlagSet(m_pageLockFlags, LTF_resizable) && m_pageToLock->IsUserResizable())
		|| (isFlagSet(m_pageLockFlags, LTF_acceptsChildMove) && m_pageToLock->GetAcceptsMoveFromChildren());

	return !notLocked;
}

//----------------------------------------------------------------------

bool SwgCuiLockableMediator::setPageLocked(bool lockIt)
{
	if (m_pageToLock != NULL)
	{
		if(isFlagSet(m_pageLockFlags, LTF_userMovable))
			m_pageToLock->SetUserMovable(!lockIt);

		if(isFlagSet(m_pageLockFlags, LTF_acceptsChildMove))
			m_pageToLock->SetAcceptsMoveFromChildren(!lockIt);

		if (isFlagSet(m_pageLockFlags, LTF_resizable))
			m_pageToLock->SetUserResizable(!lockIt);

		return true;
	}
	return false;
}

//----------------------------------------------------------------------

void SwgCuiLockableMediator::setupUnlockedState()
{
	m_pageLockFlags = LTF_none;
	if(m_pageToLock)
	{
		int storedFlags = LTF_none;
		UIString propertyString;
		//if this page has a cached unlocked state (meaning it may have had its properties
		//changed via a lock) use the cached version instead of the current one
		if(m_pageToLock->GetProperty(Settings::unlockedStateFlags, propertyString) && UIUtils::ParseInteger(propertyString, storedFlags))
		{
			setDefaultWindowResizable(isFlagSet(storedFlags, LTF_resizable));
			setDefaultWindowUserMovable(isFlagSet(storedFlags, LTF_userMovable));
			setDefaultWindowAcceptsChildMove(isFlagSet(storedFlags, LTF_acceptsChildMove));
		}
		else
		{
			setDefaultWindowResizable(m_pageToLock->IsUserResizable());
			setDefaultWindowUserMovable(m_pageToLock->IsUserMovable());
			setDefaultWindowAcceptsChildMove(m_pageToLock->GetAcceptsMoveFromChildren());
			m_pageToLock->SetPropertyInteger(Settings::unlockedStateFlags, m_pageLockFlags);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiLockableMediator::loadSizeLocation (bool doSize, bool doLocation)
{
	UISize size  = getPage ().GetSize ();
	UIPoint loc  = getPage ().GetLocation ();

	if (doSize)
		CuiSettings::loadSize (getMediatorDebugName(), size);

	if (doLocation)
		CuiSettings::loadLocation (getMediatorDebugName(), loc);

	if (m_pageToLock)
	{
		bool isMovable = true;
		CuiSettings::loadBoolean(getMediatorDebugName(), Settings::userMovable, isMovable);

		setPageLocked(!isMovable);
 	}

	UIRect rect (loc, size);	

	getPage ().SetRect (rect);
}

//----------------------------------------------------------------------

void SwgCuiLockableMediator::saveSettings () const
{
	if (getPopupHelpData())
		CuiSettings::saveBoolean (getMediatorDebugName(), Settings::popupHelpEnabled, hasState (MS_popupHelpOk));

	if (hasState (MS_settingsAutoSize))
	{
		const UISize & size = getPage ().GetSize ();
		CuiSettings::saveSize     (getMediatorDebugName(), size);
	}

	if (hasState (MS_settingsAutoLoc))
	{
		const UIPoint & loc = getPage ().GetLocation ();
		CuiSettings::saveLocation (getMediatorDebugName(), loc);
	}

	if (m_pageToLock)
	{
		bool const isMovable = !getPageIsLocked();
		CuiSettings::saveBoolean(getMediatorDebugName(), Settings::userMovable, isMovable);
	}
}

//======================================================================