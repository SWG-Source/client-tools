// ======================================================================
//
// SwgCuiInventoryContainerIcons.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiInventoryContainerIcons_H
#define INCLUDED_SwgCuiInventoryContainerIcons_H

// ======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"

class CachedNetworkId;
class CallbackReceiver;
class ClientObject;
class CuiWidget3dObjectListViewer;
class Object;
class SwgCuiInventoryContainer;
class UIButton;
class UIText;
class UITextStyle;
class UIVolumePage;
class VolumeContainer;

namespace MessageDispatch
{
	class Callback;
};

namespace SwgCuiInventoryContainerIconsNamespace
{
	class MyIconCallback;
}


//-----------------------------------------------------------------

class SwgCuiInventoryContainerIcons:
public CuiMediator,
public UIEventCallback
{
public:

	explicit                   SwgCuiInventoryContainerIcons (UIPage & page);

	void                       activateInContainer       (SwgCuiInventoryContainer * containerMediator);

	bool                       OnMessage                     (UIWidget * context, const UIMessage & msg);
	void                       OnVolumePageSelectionChanged  (UIWidget * context);

	void                       onListReset (const bool & payload);
	void                       onSelection (const std::pair<int, ClientObject *> & payload);

	UIVolumePage *             getVolumePage ();

	virtual void               OnHoverIn  (UIWidget *Context);
	virtual void               OnHoverOut (UIWidget *Context);

	void                       onIconCallback ();

	void                       openSelectedRadial();

protected:
	void                       performActivate   ();
	void                       performDeactivate ();

private:
	                          ~SwgCuiInventoryContainerIcons ();
	                           SwgCuiInventoryContainerIcons ();
	                           SwgCuiInventoryContainerIcons (const SwgCuiInventoryContainerIcons & rhs);
	SwgCuiInventoryContainerIcons & operator= (const SwgCuiInventoryContainerIcons & rhs);

	void                        setContainerMediator          (SwgCuiInventoryContainer * containerMediator);

	void                        reset ();
	ClientObject *              findDropDestination (UIWidget & context, std::string & slotname);
	void                        updateIcons ();
	void                        addObjectIcon (ClientObject & obj);
	void                        updateSelection ();

	UIVolumePage *              m_volume;

	MessageDispatch::Callback * m_callback;

	SwgCuiInventoryContainer *  m_containerMediator;

	bool                        m_updatingSelection;

	long                        m_originalIconSize;

	SwgCuiInventoryContainerIconsNamespace::MyIconCallback * m_iconCallback;

};

//----------------------------------------------------------------------

inline UIVolumePage * SwgCuiInventoryContainerIcons::getVolumePage ()
{
	return m_volume;
}

// ======================================================================

#endif
