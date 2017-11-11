//======================================================================
//
// SwgCuiExamine.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiExamine_H
#define INCLUDED_SwgCuiExamine_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class ClientObject;
class CuiWidget3dObjectViewer;
class NetworkId;
class Object;
class SwgCuiInventoryInfo;
class UIButton;

namespace MessageDispatch
{
	class Callback;
}

template <typename T> class Watcher;

//-----------------------------------------------------------------

class SwgCuiExamine :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                 SwgCuiExamine                 (UIPage & page);

	void                     setTarget                     (Object * obj);

	void                     onObjectDestroyed             (const NetworkId & id);

	void                     update                        (float deltaTimeSecs);
	
	bool                     OnMessage                     (UIWidget *context, const UIMessage & msg);

	void                     OnPopupMenuSelection          (UIWidget * context);
		
protected:
	virtual void             performActivate ();
	virtual void             performDeactivate ();
	
private:

	                       ~SwgCuiExamine ();
	SwgCuiExamine ();
	SwgCuiExamine (const SwgCuiExamine & rhs);
	SwgCuiExamine & operator= (const SwgCuiExamine & rhs);

	void                     applyRotationToServer         (void);
	bool                     canBeRotated                  (void);

	typedef Watcher<Object> ObjectWatcher;

	SwgCuiInventoryInfo *              m_info;
	MessageDispatch::Callback *        m_callback;
	ObjectWatcher *                    m_watcher;
};

//======================================================================

#endif
