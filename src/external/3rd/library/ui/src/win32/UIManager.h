#ifndef __UIMANAGER_H__
#define __UIMANAGER_H__

#include "UINotification.h"
#include "UITypeID.h"
#include "UITypes.h"
#include "UIString.h"

#if WIN32
// Work around for windows header #define of PlaySound
#undef PlaySound
#endif // WIN32

class UIActionListener;
class UICanvas;
class UICanvasFactory;
class UIClock;
class UICursor;
class UICursorInterface;
class UIEffector;
class UIIMEManager;
class UILocalizedStringFactory;
class UIMessageModifierData;
class UIPage;
class UIPalette;
class UIPopupMenu;
class UIScriptEngine;
class UISoundCanvas;
class UIWidget;
struct UIMessage;
struct UIPoint;

namespace UIPacking
{
	class Manager;
}

//----------------------------------------------------------------------

class UIManager :
public UINotification
{
public:

	                  UIManager ();
	                 ~UIManager ();

	void              SendHeartbeats ();
	void              ResetHeartbeat ();
	void              ResetTooltipCountdown ();
	void              ResetHoverPressCountdown ();

	UINotificationServer * getHeartbeatNotificationServer ();

	void              SetSize( long x, long y );
	void              SetSize( const UIPoint &pt );

	void              Render( UICanvas & ) const;	

	/** 
	* @return true if the message was eaten in order to cancel a dragging action of some sort
	*/
	bool              ProcessMessage            (const UIMessage &);

	void              DrawCursor                (bool );
	bool              GetDrawCursor             () const;

	void              SetRootPage               (UIPage * );
	UIPage           *GetRootPage               () { return mRootPage; };
	const UIPage     *GetRootPage               () const { return mRootPage; };

	UIBaseObject     *GetObjectFromPath         (const char * str ) const;
	UIBaseObject     *GetObjectFromPath         (const char * str, UITypeID type) const;
	UIWidget *        GetFocusedLeafWidget      ();

	UICanvas         *GetCanvas                 (const UIString & shaderName, const UIString & textureName);
	UICanvas         *GetCanvas                 (const UIString & textureName);

	int               GetCanvasCount            () const;
	void              GetCanvases               (ui_stdvector<UICanvas *>::fwd & canvasVector) const;
	void              AddCanvas                 (const Unicode::String & shaderName, const UIString & canvasName , UICanvas *);
	void              AddCanvasFactory          (const UICanvasFactory *);

	void              AddLocalizedStringFactory (const UILocalizedStringFactory *);
	bool              GetLocalizedString        (const UINarrowString & name, UIString &dest) const;
	
	bool              CreateLocalizedString     (const UIString & name, UIString & dest) const;

	void              RefreshGraphics           ();

	void              SetDragEcho               (UIWidget *, UIWidget *, const UIPoint & offset);
	void              ShowDragEcho              (bool);
	const UIPoint &   getDragEchoOffset         () const;

	void              AbortDragAndDrop          (UIWidget * DragTarget);

	static bool isUIReady()
	{
		return gIsUIReady;
	};

	static void createSingleton();
	
	// Access the global manager object
	static UIManager &gUIManager ()
	{
		if( !gSingleton )
		{
			createSingleton();
		}
		
		return *gSingleton;
	};

	static void              ExplicitDestroy () 
	{
		gIsUIReady = false;
		delete gSingleton; 
		gSingleton = 0;
	};

	void              AddClock( UIClock * );
	void              RemoveClock( UIClock * );

	void              SetSoundCanvas( UISoundCanvas * );
	UISoundCanvas    *GetSoundCanvas (){ return mSoundCanvas; };
	void              PlaySound                ( const char *FileName ) const;
	void              PlaySoundGenericNegative () const;
	
	void              SetScriptEngine( UIScriptEngine * );
	UIScriptEngine   *GetScriptEngine (){ return mScriptEngine; };
	void              ExecuteScript( const UIString &Script, UIBaseObject *Context );

	void              ExecuteEffector( UIEffector *Effector, UIBaseObject *Context, bool allowDuplicates = true );
	bool              HasEffector( UIEffector *Effector, UIBaseObject *Context );
	void              CancelEffector( UIEffector *Effector, UIBaseObject *Context );
	void              CancelEffectorsFor(UIBaseObject *Context, bool childObjects = false, UITypeID type = TUIEffector);

	void              Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, UINotification::Code NotificationCode );

	const             UIPacking::Manager &  GetPackingManager () const;

	enum ContextWidgetAlignment
	{
		CWA_NW,
		CWA_N,
		CWA_NE,
		CWA_E,
		CWA_SE,
		CWA_S,
		CWA_SW,
		CWA_W,
		CWA_Center
	};

	void              PushContextWidget           (UIWidget & context, ContextWidgetAlignment alignment = CWA_NW, bool fit = true);
	void              PopContextWidgets           (UIWidget * context);
	void              PopContextWidgetsNextFrame  (UIWidget * context);
	// We use Popup and Context interchangeably while the actual class is named Popup
	UIPopupMenu	*     FindAssociatedPopupMenu     (UIPage * page) const;

//	void              SetContextWidget            (UIWidget * context, ContextWidgetAlignment alignment = CWA_NW, bool fit = true);
	const UIWidget *  GetTopContextWidget         () const;

	void              SetContextMessage           (const UIMessage & msg, long holdTime, bool primary);

	bool              IsContextMessage            (const UIMessage & msg, bool primary);
	bool              IsContextMessage            (const UIMessage & msg);

	void              SetDragButton               (long buttonNumber);

	bool              IsContextRequestPending     (bool primary) const;
	void              ForcePendingContextRequest  (bool primary);

	long              GetDragCounter              () const;

	const UIPoint &   GetLastMouseCoord           () const;

	float             GetDragThreshold            () const;

	void              RefreshMousePosition        ();

	static void       fitWidgetOnScreen           (UIWidget & context, ContextWidgetAlignment alignment, bool fit);

	void              SetActionListener           (UIActionListener * listener);
	bool              NotifyActionListener        (const UIBaseObject & obj);

	void              SetCursorInterface          (UICursorInterface * cursorInterface);
	void              SetLastWidgetUnderMouse     (UIWidget * const widget);

	void              SetTooltipDelaySecs         (float secs);

	const UIWidget *  GetMouseDownControl         () const;
	void              ClearMouseDownControl       ();

	void              SetKeystrokePopsContext     (bool b);

	enum EffectToken
	{
		EFTKN_Active,
		EFTKN_Assignment,
		EFTKN_Cancel,
		EFTKN_Effect,
		EFTKN_Page,
		EFTKN_Reset,
		EFTKN_Ui,
		EFTKN_ShowMediator,
		EFTKN_HideMediator
	};

	UIString const & getEffectToken(EffectToken token) const;

	void replaceCanvasTexturesByName(UINarrowString const & currentTextureName, UINarrowString const & newTextureName);

	void setUIIMEManager(UIIMEManager *manager);
	UIIMEManager *getUIIMEManager();

	enum Locale
	{
		L_usa,
		L_japan
	};

	void SetLocale(Locale const locale);
	Locale GetLocale() const;
	bool isLocaleJapanese() const;
	char const * GetLocaleString() const;
	void SetLocaleByString(std::string const & locale);

	bool IsScripting() const;

	//Any widgets added to this list actually end up getting rendered twice; once in the normal flow, and
	//then again at the end.  So use this sparingly, and keep that in mind.
	void AddToRenderLastList(UIWidget *widget);
	void RemoveFromRenderLastList(UIWidget *widget);

	UICanvas * CreateCanvas(UINarrowString const & textureName);
	void enableDrawWidgetBorders(bool enable);

	void garbageCollect();

private:

	UIWidget              *GetDragTarget          (const UIPoint &, bool & dragOk);
	static UIManager * volatile gSingleton;
	/* gIsUIReady is needed because:
	 *  - UIManager::gUIManager() is called in UIText::SetSelected(NULL) and UITextBox::SetSelected(NULL)
	 *  - UIText::SetSelected(NULL) and UITextBox::SetSelected(NULL) are called in the destructor for
	 *    UIManager if the currently selected UI element is a UIText or UITextBox and so needed to check
	 *    isUIReady() before gUIManager().
	 *  - isUIReady() checked gSingleton == NULL, which is not a valid check in this case and so needed to
	 *    use a boolean: gIsUIReady.
	 */
	static bool           gIsUIReady;


	UIPage                *mRootPage;
	UISoundCanvas         *mSoundCanvas;
	UIScriptEngine        *mScriptEngine;

	long                   mCountdownToTooltip;
	long                   mTicksToTooltip;
	long                   mCountdownToHoverPress;
	long                   mTicksToHoverPress;
	bool                   mHoverPressComplete;
	unsigned long          mTooltipAnimationState;

	// Drag & Drop variables
	UIPoint                mLastMouseCoord;
	UIPoint                mLastMouseDownCoord;
	UIWidget              *mMouseDownControl;
	UIWidget              *mDraggedControl;
	UIWidget              *mDraggedSource;
	UIWidget *             mLastWidgetUnderMouse;
	float                  mDragThreshold;	
	UIPoint                mDragEchoOffset;
	bool                   mShowDragEcho;
	bool                   mDragGood;
	bool                   mAttemptToDrag;

	unsigned long          mLastHeartbeatCall;
	bool                   mDrawCursor;
	bool                   mUpdateUnderMouse;

	struct CanvasShaderMap;
	CanvasShaderMap *      mCanvasShaders;

	struct CanvasFactoryVector;
	CanvasFactoryVector *  mCanvasFactories;

	struct ClockSet;
	ClockSet *             mClocks;

	struct LocalizedStringFactoryVector;
	LocalizedStringFactoryVector * mLocalizedStringFactories;

	typedef ui_stdmultimap<UIEffector *, UIBaseObject *>::fwd EffectorMap;
	EffectorMap * mEffectors;

	bool                   mInDestructor;
	bool                   mClockInitalized;

	UIPacking::Manager *   mPackingManager;

	struct StringTokenMapping;
	mutable StringTokenMapping * mStringTokenMapping;

	UINotificationServer * mHeartbeatNotificationServer;

	UIWidget *             mContextWidget;
 
	struct ContextInfo
	{
		UIMessage *            message;
		UIMessage *            messageComplete;
		long                   delay;
		long                   countdown;
		bool                   pending;
		UIPoint                requestPoint;
		short                  requestData;

		ContextInfo ();
		~ContextInfo ();

		bool                   update            (long ticks);
		void                   reset             ();
		void                   checkDragActivate (UIManager & manager, const UIPoint & pt);
		bool                   checkRequest      (bool start, const UIMessage & msg);
	};

	friend struct ContextInfo;

	ContextInfo             mContextInfo;	
	ContextInfo             mContextInfoAlternate;

	//-- internal use only
	UIMessage *             mDragStartMessage;
	UIMessage *             mDragEndMessage;
	UIMessageModifierData * mDragModifier;

	long                    mDragCounter;

	UIPage *                mContextPage;

	long                    mCurrentTick;

	UIActionListener *      mActionListener;

	UICursorInterface *     mCursorInterface;

	bool                    mCancelContextWithRelease;

	bool                    mKeystrokePopsContext;

	UIIMEManager*           mIMEManager;

	Locale mLocale;
	bool mIsScripting;

	ui_stdvector<UIWidget *>::fwd *mRenderLastList;
};

//-----------------------------------------------------------------

inline const UIPacking::Manager & UIManager::GetPackingManager () const
{
	return *mPackingManager;
}

//-----------------------------------------------------------------

inline UINotificationServer * UIManager::getHeartbeatNotificationServer ()
{
	return mHeartbeatNotificationServer;
}

//----------------------------------------------------------------------

inline bool UIManager::IsContextRequestPending (bool primary) const
{
	if (primary)
		return mContextInfo.pending;
	else
		return mContextInfoAlternate.pending;
}

//----------------------------------------------------------------------

inline void UIManager::ForcePendingContextRequest (bool primary)
{
	if (primary)
		mContextInfo.countdown = 0L;
	else
		mContextInfoAlternate.countdown = 0L;
}

//----------------------------------------------------------------------

/**
* Drag counter gets incremented each and every time a drag is started.
* This allows client code to better associate drag starts with drag ends.
*
*/

inline long UIManager::GetDragCounter () const
{
	return mDragCounter;
}


//-----------------------------------------------------------------

inline 	const UIPoint &   UIManager::GetLastMouseCoord () const
{
	return mLastMouseCoord;
}

//----------------------------------------------------------------------

inline float  UIManager::GetDragThreshold () const
{
	return mDragThreshold;
}

//----------------------------------------------------------------------

inline bool UIManager::IsContextMessage (const UIMessage & msg)
{
	return IsContextMessage (msg, true) || IsContextMessage (msg, false);
}

//----------------------------------------------------------------------

inline bool UIManager::GetDrawCursor             () const
{
	return mDrawCursor;
}

//----------------------------------------------------------------------

inline const UIWidget *  UIManager::GetMouseDownControl         () const
{
	return mMouseDownControl;
}

//----------------------------------------------------------------------

inline void UIManager::SetLocale(Locale const locale)
{
	mLocale = locale;
}

//----------------------------------------------------------------------

inline UIManager::Locale UIManager::GetLocale() const
{
	return mLocale;
}

//----------------------------------------------------------------------

inline bool UIManager::isLocaleJapanese() const
{
	return mLocale == L_japan;
}


//-----------------------------------------------------------------


#endif // __UIMANAGER_H__

