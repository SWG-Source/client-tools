//--
//-- CuiMediator.h
//-- jwatson 2001-02-11
//-- copyright 2001 Sony Online Entertainment
//--


#ifndef CuiMediator_H
#define CuiMediator_H

/**
* CuiMediator is a pure virtual class intended to be used as a base for
* the UI event callback listeners for each UI page
*
* Subclasses should override performActivate () and performDeactivate ()
* in order to provide custom functionality, add/remove listeners, etc...
*
* @author jwatson
* @date 2001-02-11
* @see nothing
*/

class  CuiPopupHelp;
class  CuiPopupHelpData;
class  CuiWorkspace;
class  CuiWorkspaceIcon;
class  UIComposite;
class  UIBaseObject;
class  UIButton;
class  UICheckbox;
class  UIData;
class  UIPage;
class  UIWidget;
class CachedNetworkId;
class NetworkId;
struct UIFloatPoint;
struct UIPoint;
struct UIRect;

#include "sharedMath/Vector.h"

#include "UIPage.h"
#include "UITypes.h"
#include "UITypeID.h"

//-----------------------------------------------------------------

class CuiMediator
{
public:

	enum MediatorStates
	{
		MS_iconifiable         = 0x0000001,
		MS_maximizable         = 0x0000002,
		MS_closeable           = 0x0000004,
		MS_movable             = 0x0000008,
		MS_resizable           = 0x0000010,
		MS_maximized           = 0x0000020,
		MS_iconified           = 0x0000040,
		MS_enabled             = 0x0000080,
		MS_pointerActive       = 0x0000100,
		MS_keyboardActive      = 0x0000200,
		MS_toggleActive        = 0x0000400,
		MS_active              = 0x0000800,
		MS_stickyVisible       = 0x0001000,
		MS_wasVisible          = 0x0002000,
		MS_closeDeactivates    = 0x0004000,
		MS_updating            = 0x0008000,
		MS_settingsEnabled     = 0x0010000,
		MS_settingsLoaded      = 0x0020000,
		MS_settingsAutoSize    = 0x0040000,
		MS_settingsAutoLoc     = 0x0080000,
		MS_hasStartSize        = 0x0100000,
		MS_hasStartedPopupHelp = 0x0200000,
		MS_closeNextFrame      = 0x0400000,
		MS_openNextFrame       = 0x0800000,
		MS_popupHelpOk         = 0x1000000,
		MS_getsKeyboardInput   = 0x2000000,
	};

	struct ReleaserFunctor
	{
		void operator ()(CuiMediator * mediator) const
		{
			if (mediator)
				mediator->release ();
		}
	};

	struct ButtonPaths
	{
		static const char * const Close;
		static const char * const MaximizeRestore;
		static const char * const Minimize;
	};

	                        CuiMediator            (const char * const mediatorDebugName, UIPage & newPage);

	void                    activate               (const char * previousMediator = 0);
	void                    deactivate             ();
	const UIPage &          getPage                () const;
	UIPage &                getPage                ();
	const UIComposite &     getComposite           () const;
	UIComposite &           getComposite           ();

	bool                    isActive               () const;

	bool                    getInputToggleActive   () const;
	bool                    getKeyboardInputActive () const;
	bool                    getPointerInputActive  () const;

	static int              getCountInputToggleActive   ();
	static int              getCountKeyboardInputActive ();
	static int              getCountPointerInputActive  ();

	const char *            getPreviousMediator    () const;
	void                    setPreviousMediator    (const char * mediator);

	//----------------------------------------------------------------------

	uint32                  getStates              () const;
	bool                    hasState               (MediatorStates s) const;

	bool                    isIconifiable          () const;
	bool                    isMaximizable          () const;
	bool                    isCloseable            () const;
	bool                    isMovable              () const;
	bool                    isResizable            () const;
	bool                    isMaximized            () const;
	bool                    isIconified            () const;
	bool                    isEnabled              () const;
	bool                    isStickyVisible        () const;
	bool                    wasVisible             () const;
	bool                    isCloseDeactivates     () const;
	bool                    isUpdating             () const;
	bool                    getSettingsEnabled     () const;

	const UIRect &          getRestoreRect         () const;
	void                    setRestoreRect         (const UIRect & rect);

	void                    setShowFocusedGlowRect (bool on);
	bool                    getShowFocusedGlowRect () const;

	void                    setIconified           (bool b);
	void                    setEnabled             (bool b);
	void                    setStickyVisible       (bool b);
	void                    setWasVisible          (bool b);
	void                    setIsUpdating          (bool b);
	void                    setSettingsEnabled     (bool b);

	virtual bool            maximize               ();	
	virtual bool            restore                ();
	virtual bool            close                  ();
	void                    open                   ();
	void                    closeNextFrame         ();
	void                    openNextFrame          ();
	bool                    isCloseNextFrame       () const;
	bool                    isOpenNextFrame        () const;

	bool                    getLockedAspectRatio   (UIFloatPoint & ratio) const;

	bool                    getIconLocation        (UIPoint & loc) const;
	CuiWorkspaceIcon *      getIcon                () const;

	//---------------------------------------------------------------------

	CuiWorkspace *          getContainingWorkspace ();
	void                    setContainingWorkspace (CuiWorkspace * workspace);

	virtual bool            test                   (std::string & result);

	int                     getRefCount            () const;
	void                    fetch                  ();
	void                    release                ();

	static void             garbageCollect         (bool force);
	static int              getRemainingMediatorCount ();
	static void             updateAll                 (float deltaTimeSecs);
	static void             debugPrintMediators       (std::string & str);

	void                    closeThroughWorkspace     ();
	void                    iconifyThroughWorkspace   ();
	void                    maximizeThroughWorkspace  ();

	virtual void            update                    (float deltaTimeSecs);
	virtual void            saveSettings              () const;
	virtual void            loadSettings              ();

	void                    setSettingsAutoSizeLocation (bool size, bool location);

	bool                    getStartSize                (UISize & size);

	static void             incrementPointerInputActiveCount  (int num);
	static void             incrementKeyboardInputActiveCount (int num);

	void                    raisePopupHelp                    ();
	void                    onPopupHelpClose                  (const CuiPopupHelp & popupHelp, bool okNext);
	const std::string &     getMediatorDebugName              () const;

	void                    setAssociatedObjectId             (const NetworkId & id);
	void                    setAssociatedLocation             (const Vector & location);
	void                    setMaxRangeFromObject             (float range);

	void                    resetToDefaultSizeAndLocation     ();
	CuiPopupHelp *          getPopupHelpData                  () const; 

	/**
	* Users of this template method must include "UIData.h" and "UIPage.h" for it to operate
	*/
	template <typename T> T *  getCodeDataObject (const UIData * theData, UITypeID id, UISmartPointer<T> & var, const char * name, bool optional) const
	{
		return getCodeDataObject(&m_thePage, theData, id, var.pointer(), name, optional);
	}

	/**
	* Users of this template method must include "UIData.h" and "UIPage.h" for it to operate
	*/
	template <typename T> T *  getCodeDataObject (const UIData * theData, UITypeID id, T *& var, const char * name, bool optional) const
	{
		return getCodeDataObject(&m_thePage, theData, id, var, name, optional);
	}

	/**
	* Users of this template method must include "UIData.h" and "UIPage.h" for it to operate
	*/
	template <typename T> T *  getCodeDataObject (UITypeID id, T *& var, const char * name, bool optional) const
	{
		return getCodeDataObject (&m_thePage, m_codeData, id, var, name, optional);
	}

	/**
	* Users of this template method must include "UIData.h" for it to operate
	*/
	template <typename T> void getCodeDataObject (UITypeID id, T *& var, const char * name) const
	{
		IGNORE_RETURN (getCodeDataObject(&m_thePage, id, var, name, false));
	}



	/*
	* Users of this template method must include "UIData.h" and "UIPage.h" for it to operate
	*/
	template <typename T> T *  getCodeDataObject (UITypeID id, UISmartPointer<T> & var, const char * name, bool optional) const
	{
		T * obj = NULL;
		IGNORE_RETURN(getCodeDataObject(&m_thePage, id, obj, name, optional));
		var = obj;
		return obj;
	}

	/**
	* Users of this template method must include "UIData.h" for it to operate
	*/
	template <typename T> void getCodeDataObject (UITypeID id, UISmartPointer<T> & var, const char * name) const
	{
		T * obj = NULL;
		IGNORE_RETURN(getCodeDataObject(&m_thePage, id, obj, name, false));
		var = obj;
	}
	
	
	/**
	* Users of this template method must include "UIData.h" and "UIPage.h" for it to operate
	*/
	template <typename T> T *  getCodeDataObject (UIPage *rootPage, const UIData * theData, UITypeID id, T *& var, const char * name, bool optional) const
	{
		UNREF (optional);
		NOT_NULL (name);
		
		UIBaseObject * const result = getCodeDataObject (rootPage, theData, id, name, optional);
						
		if (result && result->IsA (id))
		{
			var = static_cast<T*> (result);
			return var;
		}

		FATAL (!optional, ("Unable to find CodeData object '%s' from '%s'", name, rootPage->GetFullPath ().c_str ()));

		return 0;
	}

	/**
	* Users of this template method must include "UIData.h" and "UIPage.h" for it to operate
	*/
	template <typename T> T *  getCodeDataObject (UIPage *rootPage, UITypeID id, T *& var, const char * name, bool optional) const
	{
		return getCodeDataObject (rootPage, m_codeData, id, var, name, optional);
	}

	/**
	* Users of this template method must include "UIData.h" for it to operate
	*/
	template <typename T> void getCodeDataObject (UIPage *rootPage, UITypeID id, T *& var, const char * name) const
	{
		IGNORE_RETURN (getCodeDataObject(rootPage, id, var, name, false));
	}

	template <typename T> T *  getCodeDataObject (UIPage *rootPage, const UIData * theData, UITypeID id, T *& var, const char * name) const
	{
		return getCodeDataObject(rootPage, theData, id, var, name, false);
	}

	UIBaseObject * getCodeDataObject (const UIData * theData, UITypeID id, const char * name, bool optional) const;
	UIBaseObject * getCodeDataObject (UIPage *rootPage, const UIData * theData, UITypeID id, const char * name, bool optional) const;

	virtual void            handleMediatorPropertiesChanged ();

	// UISmartPointer interface.
	static void Lock(CuiMediator * const obj)
	{
		if (obj) 
		{
			obj->fetch();
		}
	}
	
	static void Unlock(CuiMediator * const obj)
	{
		if (obj) 
		{
			obj->release();
		}
	}

protected:

	virtual                ~CuiMediator            ();

	void                    setInputToggleActive   (bool b);
	void                    setKeyboardInputActive (bool b);
	void                    setPointerInputActive  (bool b);

	void                    setPointerToggledOn    (bool b);
	
	virtual void            performActivate        ();
	virtual void            performDeactivate      ();

	const UIData *          getCodeData            () const;
	bool                    getCodeDataString      (const char * name, Unicode::String & value) const;

	void                    setIcon                (CuiWorkspaceIcon * icon);

	bool                    setState               (MediatorStates s);
	bool                    removeState            (MediatorStates s);
	bool                    setOrRemoveState       (MediatorStates s, bool b);

	void                    setLockedAspectRatio   (const UIFloatPoint & ratio);

	UIButton *              getButtonClose ();

	void                    registerMediatorObject (UIBaseObject & obj, bool activeCallbacks);
	void                    unregisterMediatorObject (UIBaseObject & obj);
	bool					isRegisteredMediatorObject (UIBaseObject & obj);
	void                    unregisterMediatorObjects();

	virtual void            loadSizeLocation       (bool doSize, bool doLocation);

	void                    setMediatorDebugName   (const std::string & mediatorDebugName);

	void                    startPopupHelp         ();
	void                    stopPopupHelp          ();
	
	virtual UIEventCallback*getCallbackObject      ();

private:

	//-- explicitly disable copy constructor and assignment operators
	                        CuiMediator ();
	                        CuiMediator (const CuiMediator &);
	CuiMediator &           operator =  (const CuiMediator &);

	void                    updatePopupHelp    (float deltaTimeSecs);

	void autoRegisterWidgets();
	void autoUnregisterWidgets();

private:

	typedef stdvector<CuiPopupHelpData>::fwd PopupHelpDataVector;
	typedef std::pair<UIBaseObject *, bool> ObjectCallbackData;
	typedef stdvector<ObjectCallbackData>::fwd ObjectCallbackVector;

	std::string             m_mediatorDebugName;
	UIPage &                m_thePage;
	UIData *                m_codeData;

	const char *            m_previousMediator;

	uint32                  m_states;

	CuiWorkspaceIcon *      m_icon;
	UIFloatPoint *          m_lockedAspectRatio;

	UIRect *                m_restoreRect;

	CuiWorkspace *          m_containingWorkspace;

	UIButton *              m_buttonClose;
	UIButton *              m_buttonMaximizeRestore;
	UIButton *              m_buttonMinimize;

	class                   MediatorEventCallback;
	friend class            MediatorEventCallback;

	MediatorEventCallback * m_eventCallback;

	int                     m_refcount;
	UISize                  m_startSize;

	static int              ms_countPointer;
	static int              ms_countKeyboard;
	static int              ms_countToggle;

	CuiPopupHelp *          m_popupHelp;
	PopupHelpDataVector *   m_popupHelpData;
	UICheckbox *            m_checkboxPopupHelp;
	int                     m_popupHelpIndex;
	UIRect                  m_lastRect;

	ObjectCallbackVector *  m_objectCallbackVector;

	UIWidget *              m_widgetToFocus;

	CachedNetworkId *       m_associatedObjectId;
	Vector                  m_associatedLocation;
	float                   m_maxRangeFromObject;
	float                   m_lastObjectCheckTime;

	bool                    m_showFocusedGlowRect;

	UISize                  m_defaultSize;
	UIPoint                 m_defaultLocation;
};


//-----------------------------------------------------------------

inline const UIPage & CuiMediator::getPage () const          { return m_thePage; }

//-----------------------------------------------------------------

inline UIPage & CuiMediator::getPage ()                      { return m_thePage; }

//-----------------------------------------------------------------

inline bool CuiMediator::isActive () const                   { return hasState (MS_active); }

//-----------------------------------------------------------------

inline bool CuiMediator::getInputToggleActive (void) const   { return hasState (MS_toggleActive); }

//-----------------------------------------------------------------

inline bool CuiMediator::getKeyboardInputActive (void) const { return hasState (MS_keyboardActive); }

//-----------------------------------------------------------------

inline bool CuiMediator::getPointerInputActive (void) const  { return hasState (MS_pointerActive); }

//-----------------------------------------------------------------

inline const UIData * CuiMediator::getCodeData () const      { return m_codeData; }

//----------------------------------------------------------------------

inline uint32 CuiMediator::getStates () const      { return m_states; }

//----------------------------------------------------------------------

inline bool CuiMediator::hasState (MediatorStates s) const { return (m_states & static_cast<uint32>(s)) != 0 ; }

//----------------------------------------------------------------------

inline bool CuiMediator::isIconifiable () const    { return hasState (MS_iconifiable); }

//----------------------------------------------------------------------

inline bool CuiMediator::isMaximizable () const    { return hasState (MS_maximizable); }

//----------------------------------------------------------------------

inline bool CuiMediator::isCloseable   () const    { return hasState (MS_closeable); }

//----------------------------------------------------------------------

inline bool CuiMediator::isMovable     () const    { return hasState (MS_movable); }

//----------------------------------------------------------------------

inline bool CuiMediator::isResizable   () const    { return hasState (MS_resizable); }

//-----------------------------------------------------------------

inline bool CuiMediator::isMaximized   () const    { return hasState (MS_maximized); }

//-----------------------------------------------------------------

inline bool CuiMediator::isEnabled     () const    { return hasState (MS_enabled); }

//----------------------------------------------------------------------

inline CuiWorkspaceIcon * CuiMediator::getIcon () const    { return m_icon; }

//----------------------------------------------------------------------

inline bool CuiMediator::setState (MediatorStates s)
{ 
	if (!hasState (s))
	{
		m_states |= static_cast<uint32>(s);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

inline bool CuiMediator::removeState (MediatorStates s)
{ 
	if (hasState (s))
	{
		m_states &= ~static_cast<uint32>(s);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

inline const UIRect & CuiMediator::getRestoreRect () const    { return *NON_NULL (m_restoreRect); }

//-----------------------------------------------------------------

inline CuiWorkspace * CuiMediator::getContainingWorkspace () { return m_containingWorkspace; } 

//----------------------------------------------------------------------

inline bool CuiMediator::isIconified   () const { return hasState (MS_iconified); }

//----------------------------------------------------------------------

inline bool CuiMediator::isStickyVisible () const { return hasState (MS_stickyVisible); }

//----------------------------------------------------------------------

inline bool CuiMediator::wasVisible () const { return hasState (MS_wasVisible); }

//----------------------------------------------------------------------

inline bool CuiMediator::isCloseDeactivates    () const { return hasState (MS_closeDeactivates); }

//----------------------------------------------------------------------

inline bool CuiMediator::isUpdating             () const { return hasState (MS_updating); }

//----------------------------------------------------------------------

inline bool CuiMediator::getSettingsEnabled     () const { return hasState (MS_settingsEnabled); }

//----------------------------------------------------------------------

inline int CuiMediator::getRefCount () const
{
	return m_refcount;
}

//----------------------------------------------------------------------

inline UIButton * CuiMediator::getButtonClose ()
{
	return m_buttonClose;
}

//----------------------------------------------------------------------

inline int CuiMediator::getCountInputToggleActive ()
{
	return ms_countToggle;
}

//-----------------------------------------------------------------

inline int CuiMediator::getCountKeyboardInputActive ()
{
	return ms_countKeyboard;
}

//-----------------------------------------------------------------

inline int CuiMediator::getCountPointerInputActive ()
{
	return ms_countPointer;
}

//----------------------------------------------------------------------

inline const std::string & CuiMediator::getMediatorDebugName   () const
{
	return m_mediatorDebugName;
}

//----------------------------------------------------------------------

inline bool CuiMediator::isCloseNextFrame       () const
{
	return hasState (MS_closeNextFrame);
}

//----------------------------------------------------------------------

inline bool CuiMediator::isOpenNextFrame       () const
{
	return hasState (MS_openNextFrame);
}

//----------------------------------------------------------------------

inline void CuiMediator::setShowFocusedGlowRect (bool const on)
{
	m_showFocusedGlowRect = on;
}

//----------------------------------------------------------------------

inline bool CuiMediator::getShowFocusedGlowRect () const
{
	return m_showFocusedGlowRect;
}

//----------------------------------------------------------------------

inline CuiPopupHelp * CuiMediator::getPopupHelpData        () const
{
	return m_popupHelp;
}

//----------------------------------------------------------------------

#define GET_UI_OBJ(page, type, path)         dynamic_cast<type *> (page.GetObjectFromPath (path, T##type))
#define GET_UI_OBJ_NOTYPE(page, type, path)  dynamic_cast<type *> (page.GetObjectFromPath (path))

//-----------------------------------------------------------------

#endif

