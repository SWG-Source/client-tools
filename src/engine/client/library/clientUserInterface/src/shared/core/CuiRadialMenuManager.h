//======================================================================
//
// CuiRadialMenuManager.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiRadialMenuManager_H
#define INCLUDED_CuiRadialMenuManager_H

//======================================================================

class ClientObject;
class MessageQueueObjectMenuRequest;
struct ObjectMenuRequestData;
class UIRadialMenu;
class UIWidget;
class CuiMenuInfoHelper;
class Object;
class UIPopupMenu;
struct UIPoint;
class NetworkId;
class Vector;

//----------------------------------------------------------------------

class CuiRadialMenuManager
{
public:

	typedef stdvector<ObjectMenuRequestData>::fwd ObjectMenuRequestDataVector;
	struct Listener;

	static CuiMenuInfoHelper * createMenu           (Object & object, const UIPoint & pt, UIPopupMenu * pop);
	static bool createMenu           (Object & object, const UIPoint & pt, bool usePopup = false);
	static bool createMenu           (Object & object, const UIPoint & pt, const Vector & radialOffset_c, bool usePopup = false);
	static void performDefaultAction (Object & object, bool allowOpenOnly = false, int typeToUse = 0, bool fromButton = false);
	static int  findDefaultAction    (Object & object);

	static bool isActive             ();
	static void touchCache           (const NetworkId & id);

	static void install ();
	static void remove ();

	static void  update ();
	static void  clear ();

	static void  handleObjectMenuRequestMessage (const MessageQueueObjectMenuRequest & msg);
	static bool  updateRanges  ();
	static bool  updateRanges  (CuiMenuInfoHelper & helper, const Object & object);

	static void  setObjectMenuDirty (NetworkId const & id);	
	static bool performDefaultDoubleClickAction(Object const & object, bool const modifierPressed);

	typedef bool (*CanBeManipulatedFunction)(Object const * object);

	static void registerCanBeManipulated(CanBeManipulatedFunction function);

	static bool  isStorytellerMode ();
	static void  setStorytellerMode (bool enable);
	static void  createGroundPlacementMessage(Vector & location, NetworkId const & cellId);
	static bool  isGrenadeTargetMode();
	static void  setGrenadeTargetMode (bool enable);

	static void  OnPopupMenuSelection (UIWidget * context);

private:

	friend struct Listener;

	static void waitForServerNotifyState (uint16 sel, const NetworkId & id, uint8 sequence);
	static void onHideRadialMenu (const UIRadialMenu * menu);
	static void onHidePopupMenu  (const UIPopupMenu * menu);


	static void setRadial (UIRadialMenu * menu);
	static void setPopup  (UIPopupMenu  * menu, bool setCallback = true);

	static void performMenuAction (int sel, int index, bool serverNotify, uint8 sequence, bool radialComplete);
	static bool populateMenu (CuiMenuInfoHelper & helper, const Object & object, bool defaultOnly);

	static void mergeWithServerData (const ObjectMenuRequestDataVector & dataVector);

	static UIRadialMenu * ms_radial;
	static UIPopupMenu  * ms_popup;

	static bool           ms_radialComplete;
	static uint8          ms_sequence;
};

//----------------------------------------------------------------------

inline bool CuiRadialMenuManager::isActive ()
{
	return ms_radial != 0 || ms_popup != 0;
}

//======================================================================

#endif
