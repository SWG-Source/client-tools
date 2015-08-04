//======================================================================
//
// SwgCuiLockableMediator.h
// copyright (c) 2008 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiLockableMediator_H
#define INCLUDED_SwgCuiLockableMediator_H

////======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class CuiMenuInfoHelper;
class UIPage;
class UIWidget;

namespace MessageDispatch
{
	class Callback;
};

class SwgCuiLockableMediator :
public CuiMediator,
public UIEventCallback
{
public:
	
	SwgCuiLockableMediator(const char * const mediatorDebugName, UIPage & newPage);

	virtual bool OnMessage(UIWidget *Context, const UIMessage & msg);
	virtual void OnPopupMenuSelection (UIWidget * context);
	virtual void loadSizeLocation (bool doSize, bool doLocation);
	virtual void saveSettings () const;

	void setDefaultWindowResizable(bool b);
	void setDefaultWindowUserMovable(bool b);
	void setDefaultWindowAcceptsChildMove(bool b);

	void setPageToLock(UIPage * page);
	void generateLockablePopup  (UIWidget * context, const UIMessage & msg);

protected:
	 
	void appendPopupOptions (UIPopupMenu * pop);
	void appendHelperPopupOptions (CuiMenuInfoHelper * menuHelper);
	UIPage * getPageToLock (void);
	void setupUnlockedState();

	bool getPageIsLocked() const;
	bool setPageLocked(bool locked);

	virtual ~SwgCuiLockableMediator ();
	 
private:
	
	enum LockableTypeFlags
	{
		LTF_none = 0x00,
		LTF_userMovable = 0x01,
		LTF_resizable = 0x02,
		LTF_acceptsChildMove = 0x04,
	};

	uint32 m_pageLockFlags;

	UIPage * m_pageToLock;

	SwgCuiLockableMediator ();
	SwgCuiLockableMediator (const SwgCuiLockableMediator &);
	SwgCuiLockableMediator &       operator= (const SwgCuiLockableMediator &);
};

//======================================================================

 #endif
