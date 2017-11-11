#ifndef __UIWIDGET_H__
#define __UIWIDGET_H__

#include "UIBaseObject.h"

//======================================================================================

class UIBoundary;
class UICanvas;
class UICursor;
class UICursorSet;
class UIDeformer;
class UIEffector;
class UIEventCallback;
class UIImageStyle;
class UIPopupMenu;
class UIPopupMenuStyle;
class UIRectangleStyle;
class UIStyle;
class UITooltipStyle;
class UIWidgetBoundaries;
class UIWidgetRectangleStyles;
struct UIMessage;
class UIRenderHelper;

template<class T> class UIWatcher;

namespace UIPacking
{
	struct LocationInfo;
	struct SizeInfo;
}

//======================================================================================

class UIWidget : public UIBaseObject
{
public:

	typedef ui_stdvector<UIWidget *>::fwd                    UIWidgetVector;

	friend class UIRenderHelper;

	enum AttributeBitFlags
	{
		BF_Visible                     = 0x0000001,
		BF_ForceVisible                = 0x0000002,
		BF_Enabled                     = 0x0000004,
		BF_Selected                    = 0x0000008,
		BF_UnderMouse                  = 0x0000010,
		BF_ScrollExtentSet             = 0x0000020,
		BF_Dragable                    = 0x0000040,
		BF_Transient                   = 0x0000080,
		BF_GetsInput                   = 0x0000100,
		BF_Activated                   = 0x0000200,
		BF_DropToParent                = 0x0000400,
		BF_ContextToParent             = 0x0000800,
		BF_DropFlagOk                  = 0x0001000,
		BF_UserMovable                 = 0x0002000,
		BF_UserResizable               = 0x0004000,
		BF_UserDragScrollable          = 0x0008000,
		BF_ContextCapable              = 0x0010000,
		BF_AbsorbsInput                = 0x0020000,
		BF_UnderMouseUpdated           = 0x0040000,
		BF_Selectable                  = 0x0080000,
		BF_ContextCapableAlternate     = 0x0100000,
		BF_TextOpacityRelativeApply    = 0x0200000,
		BF_BackgroundScrolls           = 0x0400000,
		BF_TabRoot                     = 0x0800000,
		BF_AbsorbsTab                  = 0x1000000,
		BF_UserModifying			   = 0x2000000,
		BF_TooltipDelay			       = 0x4000000,
		BF_ShrinkWrap			       = 0x8000000 // Warning: Do not add more flags.
	};

	enum VisualState
	{
		Normal = 0,
		Selected,
		Disabled,
		LastState
	};

	class PropertyName
	{ //lint !e578 // symbol hides symbol
	public:
		static const UILowerString AbsorbsInput;
		static const UILowerString AbsorbsTab;
		static const UILowerString Activated;
		static const UILowerString AcceptsMoveFromChildren;
		static const UILowerString AutoRegister;
		static const UILowerString BackgroundColor;
		static const UILowerString BackgroundColorA;
		static const UILowerString BackgroundColorB;
		static const UILowerString BackgroundColorG;
		static const UILowerString BackgroundColorR;
		static const UILowerString BackgroundOpacity;
		static const UILowerString BackgroundScrolls;
		static const UILowerString BackgroundTint;
		static const UILowerString BackgroundTintA;
		static const UILowerString BackgroundTintB;
		static const UILowerString BackgroundTintG;
		static const UILowerString BackgroundTintR;
		static const UILowerString Color;
		static const UILowerString ColorB;
		static const UILowerString ColorG;
		static const UILowerString ColorR;
		static const UILowerString ContextCapable;
		static const UILowerString ContextCapableAlternate;
		static const UILowerString ContextToParent;
		static const UILowerString Cursor;
		static const UILowerString CursorSet;
		static const UILowerString CustomDragWidget;
		static const UILowerString DragAccepts;
		static const UILowerString DragBadCursor;
		static const UILowerString DragGoodCursor;
		static const UILowerString DragType;
		static const UILowerString Dragable;
		static const UILowerString DropToParent;
		static const UILowerString Enabled;
		static const UILowerString Focus;
		static const UILowerString ForwardMoveToParent;
		static const UILowerString GetsInput;
		static const UILowerString LocalTooltip;
		static const UILowerString Location;
		static const UILowerString LocationX;
		static const UILowerString LocationY;
		static const UILowerString LockDiagonal;
		static const UILowerString MaximumSize;
		static const UILowerString MaximumSizeX;
		static const UILowerString MaximumSizeY;
		static const UILowerString MinimumScrollExtent;
		static const UILowerString MinimumSize;
		static const UILowerString MinimumSizeX;
		static const UILowerString MinimumSizeY;
		static const UILowerString OnActivate;
		static const UILowerString OnActivateEffector;
		static const UILowerString OnDeactivate;
		static const UILowerString OnDeactivateEffector;
		static const UILowerString OnDisable;
		static const UILowerString OnDisableEffector;
		static const UILowerString OnEnable;
		static const UILowerString OnEnableEffector;
		static const UILowerString OnHide;
		static const UILowerString OnHideEffector;
		static const UILowerString OnHoverIn;
		static const UILowerString OnHoverInEffector;
		static const UILowerString OnHoverOut;
		static const UILowerString OnHoverOutEffector;
		static const UILowerString OnRunScript;
		static const UILowerString OnShow;
		static const UILowerString OnShowEffector;
		static const UILowerString OnSizeChanged;
		static const UILowerString Opacity;
		static const UILowerString OpacityRelativeMin;
		static const UILowerString PackLocation;
		static const UILowerString PackSize;
		static const UILowerString PalShade;
		static const UILowerString PopupStyle;
		static const UILowerString ResizeInset;
		static const UILowerString Rotation;
		static const UILowerString ScrollExtent;
		static const UILowerString ScrollLocation;
		static const UILowerString ScrollSizeLine;
		static const UILowerString ScrollSizePage;
		static const UILowerString Selectable;
		static const UILowerString Shear;
		static const UILowerString ShrinkWrap;
		static const UILowerString Size;
		static const UILowerString SizeIncrement;
		static const UILowerString SizeX;
		static const UILowerString SizeY;
		static const UILowerString TabRoot;
		static const UILowerString TextOpacityRelativeApply;
		static const UILowerString TextOpacityRelativeMin;
		static const UILowerString Tooltip;
		static const UILowerString TooltipDelay;
		static const UILowerString TooltipStyle;
		static const UILowerString UserDragScrollable;
		static const UILowerString UserMovable;
		static const UILowerString UserResizable;
		static const UILowerString Visible;
	};

	class MethodName
	{
	public:
		static const UILowerString EffectorExecute;
		static const UILowerString EffectorCancel;
		static const UILowerString RunScript;
	};

	class CategoryName
	{
	public:
		static const UILowerString Basics;
		static const UILowerString Appearance;
		static const UILowerString AdvancedAppearance;
		static const UILowerString Behavior;
		static const UILowerString AdvancedBehavior;
	};

	static const char * const   TypeName;

	class TooltipCallback
	{
	public:
		virtual UIString const & getTooltip(UIPoint const & point) = 0;
	};

	                        UIWidget                  ();
	virtual                ~UIWidget                  ();
	virtual bool            IsA                       (const UITypeID Type ) const { return (Type == TUIWidget) || UIBaseObject::IsA( Type ); }

	virtual void            GetPropertyGroups         (UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void            GetPropertyNames          (UIPropertyNameVector &, bool forCopy) const;
	virtual void            GetLinkPropertyNames      (UIPropertyNameVector &) const;

	virtual bool            SetProperty               (const UILowerString & Name, const UIString &Value );
	virtual bool            GetProperty               (const UILowerString & Name, UIString &Value ) const;

	virtual void            GetCategories             (UIPropertyCategories::CategoryMask &o_categories) const;
	virtual void            GetPropertiesInCategory   (UIPropertyCategories::Category category, UIPropertyNameVector &) const;

	        void            AddCallback               (UIEventCallback * );
	        void            RemoveCallback            (UIEventCallback * );

	        bool            HasCallback               (UIEventCallback *) const;

	virtual void            SetRect                   (const UIRect & );
	        UIRect          GetRect                   () const { return UIRect( mLocation.x, mLocation.y, mLocation.x + mSize.x, mLocation.y + mSize.y ); };
	        void            GetRect                   (UIRect &Out ) const { Out.left = mLocation.x; Out.top = mLocation.y; Out.right = mLocation.x + mSize.x; Out.bottom = mLocation.y + mSize.y; };

	        void            GetWorldRect              (UIRect & ) const;
	        UIRect          GetWorldRect              () const;
	        void            GetWorldLocation          (UIPoint & ) const;
	        UIPoint         GetWorldLocation          () const;
	        UIPoint         GetLocationRelativeTo     (const UIWidget & otherWidget) const;

	        void            SetLocation               (const UIPoint &Location, bool center = false );
	        void            SetLocation               (UIScalar x, UIScalar y, bool center = false );
	  const UIPoint &       GetLocation               () const { return mLocation; };

	  void                  SetLockDiagonal           (const bool);
	  bool                  GetLockDiagonal           () const { return mLockDiagonal; }

	  void                  SetResizeInset            (const unsigned char);
	  unsigned char         GetResizeInset            () const { return mResizeInset; }

	virtual void            SetSize                   (const UISize &);
	  const UISize &        GetSize                   () const { return mSize; };

	void                    SetMinimumSize            (const UISize &);
	  const UISize &        GetMinimumSize            () const { return mMinimumSize; }

	        void            SetMaximumSize            (const UISize &);
	  const UISize &        GetMaximumSize            () const { return mMaximumSize; }

	        void            SetMinimumScrollExtent    (const UISize &);
	  const UISize &        GetMinimumScrollExtent    () const { return mMinimumScrollExtent; }

	virtual void            SetWidth                  (const UIScalar );
	        UIScalar        GetWidth                  () const { return mSize.x; };

	virtual void            SetHeight                 (const UIScalar );
	        UIScalar        GetHeight                 () const { return mSize.y; };

	virtual void            SetScrollLocation         (const UIPoint & );
	  const UIPoint &       GetScrollLocation         () const { return mScrollLocation; };

	virtual void            SetScrollExtent           (const UISize & );
	virtual void            GetScrollExtent           (UISize & extent ) const { extent = mScrollExtent; }

	virtual void            GetScrollSizes            (UISize &PageSize, UISize &LineSize ) const;
	void                    SetScrollSizes            (const UISize & page, const UISize & line);

	const   UISize &        GetSizeIncrement          () const;
	        void            SetSizeIncrement          (const UISize &);

	        void            SetMouseCursor            (UICursor * );
	virtual const UICursor *GetMouseCursor            () const;
	virtual UICursor       *GetMouseCursor            ();

	        void            SetDragBadCursor          (UICursor * );
	        const UICursor *GetDragBadCursor          () const { return mDragBadCursor; };
	        UICursor       *GetDragBadCursor          () { return mDragBadCursor; };

	        void            SetDragGoodCursor         (UICursor * );
	        const UICursor *GetDragGoodCursor         () const { return mDragGoodCursor; };
	        UICursor       *GetDragGoodCursor         () { return mDragGoodCursor; };

	        void            SetTooltip                (const UIString &, bool = true);
	const   UIString &      GetTooltip                () const;

	        void            SetLocalTooltip           (const UIString &, bool = true);
	virtual const UIString &      GetLocalTooltip           (const UIPoint & pt) const;
			const UIString &GetLocalTooltip           () const;

	        void            SetTooltipStyle           (UITooltipStyle * );
	const   UITooltipStyle *GetTooltipStyle           () const { return mTooltipStyle; };
	        UITooltipStyle *GetTooltipStyle           () { return mTooltipStyle; };

		void            SetTooltipDelay           (bool useDelay) { SetAttribute(BF_TooltipDelay, useDelay); }
		bool            GetTooltipDelay           () const { return HasAttribute(BF_TooltipDelay); }

	        void            SetPopupStyle             (UIPopupMenuStyle * );
	        UIPopupMenuStyle *  GetPopupStyle         ();
	        UIPopupMenuStyle const *  GetPopupStyle   () const;
			UIPopupMenuStyle *  FindPopupStyle        ();

	virtual UIWidget *      GetWidgetFromPoint        (const UIPoint &, bool mustGetInput) const;

	        VisualState     GetVisualState            () const;

	        bool            HasAttribute              (AttributeBitFlags flag) const { return (mAttributeBits & static_cast<int>(flag)) != 0; }
	        bool            HasEitherAttribute        (AttributeBitFlags flag1, AttributeBitFlags flag2) const { return (mAttributeBits & (static_cast<int>(flag1) | static_cast<int>(flag2))) != 0; }

	virtual bool            CanSelect                 () const;

	virtual void            SetSelected               (const bool );
	        bool            IsSelected                () const { return HasAttribute (BF_Selected); }

	virtual void            SetSelectable             (const bool );
	        bool            IsSelectable              () const { return HasAttribute (BF_Selectable); }

	virtual void            SetTabRoot                (const bool );
	        bool            IsTabRoot                 () const { return HasAttribute (BF_TabRoot); }

	        void            SetFocus                  ();

	        void            SetEnabled                (bool );
	        bool            IsEnabled                 () const  { return HasAttribute (BF_Enabled); }

	virtual void            SetVisible                (bool );
	        bool            IsVisible                 () const  { return HasAttribute (BF_Visible); }

	        bool            IsActivated               () const { return HasAttribute (BF_Activated); }
	        void            SetActivated              (bool b);

	        void            ForceVisible              (bool value );
	        bool            WillDraw                  () const { return HasEitherAttribute (BF_Visible, BF_ForceVisible); }

	        bool            IsUnderMouse              () const { return HasAttribute (BF_UnderMouse); }
	virtual void            SetUnderMouse             (bool b);

	        bool            IsDragable                () const { return HasAttribute (BF_Dragable); }
	        void            SetDragable               (bool);

	        bool            GetsInput                 () const { return HasAttribute (BF_GetsInput); }
	        void            SetGetsInput              (bool b );

	        bool            IsTransient               () const { return HasAttribute (BF_Transient); }
	        void            SetTransient              (bool );

	        bool            IsDropToParent            () const { return HasAttribute (BF_DropToParent); }
	        void            SetDropToParent           (bool );

			bool            IsContextToParent         () const { return HasAttribute (BF_ContextToParent); }
			void            SetContextToParent        (bool );

			bool            IsDropFlagOk              () const { return HasAttribute (BF_DropFlagOk); }
			void            SetDropFlagOk             (bool );

			bool            IsUserMovable             () const { return HasAttribute (BF_UserMovable); }
			void            SetUserMovable            (bool);

			bool            IsUserDragScrollable      () const { return HasAttribute (BF_UserDragScrollable); }
			void            SetUserDragScrollable     (bool);

			bool            IsAbsorbsInput            () const { return HasAttribute (BF_AbsorbsInput); }
			void            SetAbsorbsInput           (bool);

			bool            IsUserResizable           () const { return HasAttribute (BF_UserResizable); }
			void            SetUserResizable          (bool);

			bool            IsContextCapable          (bool primary) const;
			void            SetContextCapable         (bool b, bool primary);

	virtual UIWidget *      GetFocusedLeafWidget      ();

	        void            SetOpacity                (float NewOpacity );
	        float           GetOpacity                () const;

	        void            SetOpacityRelativeMin     (float f );
	        float           GetOpacityRelativeMin     () const;

	        void            SetColor                  (const UIColor &NewColor, bool setChildren = false );
	        UIColor         GetColor                  () const;

	        void            SetRotation               (float NewRotation );
	        float           GetRotation               () const;

	        long            GetAnimationState         () const;

	        bool            HitTest                   (const UIPoint & ) const;
	virtual bool            WantsMessage              (const UIMessage & ) const;

	virtual bool            ProcessMessage            (const UIMessage & );
	        bool            ProcessUserMessage        (const UIMessage & );
	bool                    ProcessUserMouseMove      (const UIMessage & msg);

	virtual void            ProcessChildNotificationMessage( UIWidget *, const UIMessage & );

	virtual UIStyle        *GetStyle                  () const = 0;

	virtual void            Render                    (UICanvas & ) const = 0;
    virtual void			RenderText				  (UICanvas &) const;

	        bool            PackSelfSize              ();
	        bool            PackSelfLocation          ();

	        void            ResetPackSizeInfo         ();
	        void            ResetPackLocationInfo     ();

	        void            ScrollToBottom            ();
	        void            ScrollToPoint             (const UIPoint & pt);

	        void            SetBackgroundColor        (const UIColor & );
	const UIColor &         GetBackgroundColor        () const;

	        void            SetBackgroundTint         (const UIColor & );
	const UIColor &         GetBackgroundTint         () const;

	        void            SetBackgroundOpacity      (float f);
	        float           GetBackgroundOpacity      () const;

	const UIWidgetRectangleStyles * GetWidgetRectangleStyles () const;
	UIWidgetRectangleStyles *       GetWidgetRectangleStyles ();

	virtual bool            IsDropOk                  (const UIWidget & widget, const UIString & DragType, const UIPoint & point);
	virtual UIWidget *      GetCustomDragWidget       (const UIPoint & point, UIPoint & offset);

	        bool            AcceptsDragType           (const Unicode::String & dragType);

	        UIWidget       *GetParentWidget           ();
	  const UIWidget       *GetParentWidget           () const;


	UIWidget *              FindFirstDragableWidget   ();
	UIWidget *              FindFirstContextCapableWidget (bool primary);
	UIWidget *              FindTopMoveParentWidget();

	void                    Center                    (bool Horizontal = true, bool Vertical = true);
	void                    Center                    (UIPoint const & pos, bool Horizontal = true, bool Vertical = true);

	bool                    RunScript                 ();

	enum UserModificationType
	{
		UMT_N,
		UMT_NE,
		UMT_E,
		UMT_SE,
		UMT_S,
		UMT_SW,
		UMT_W,
		UMT_NW,
		UMT_MOVE,
		UMT_DRAGSCROLL,
		UMT_NONE
	};

	virtual UIBaseObject *      DuplicateObject         () const;
	virtual void                CopyPropertiesFrom      (const UIBaseObject & );

	UIWidgetBoundaries *        getBoundaries           ();
	void                        addBoundary             (UIBoundary & boundary);
	void                        removeBoundary          (UIBoundary & boundary);
	UIBoundary *                findBoundary            (const std::string & name);
	const UIBoundary  *         findBoundary            (const std::string & name) const;

    void                        CancelEffector          (UIEffector & effector);
	void                        ExecuteEffector         (UIEffector & effector);

	void                        EnsureRectVisible       (const UIRect & rect);

	void                        SetCustomDragWidget     (UIWidget * widget);

	const UIPacking::SizeInfo * getPackSize             (int index) const;

	static float                ComputeRelativeOpacity  (float opacity, float opacityRelativeMin);

	bool                        IsUserModifying         () const;

	void SetDeformer( UIDeformer *);
	UIDeformer * GetDeformer() const;
	UIDeformer * GetDeformer();

	virtual void OnSizeChanged(UISize const & newSize, UISize const & oldSize);
	virtual void OnLocationChanged(UIPoint const & newLocation, UIPoint const & oldLocation);

	void SetTooltipCallback(TooltipCallback * tooltipCallback);

	// Force the page to resize to the extents of its children.
	void SetShrinkWrap(bool shrink) { SetAttribute(BF_ShrinkWrap, shrink); }
	bool GetShrinkWrap() const { return HasAttribute(BF_ShrinkWrap); }
	bool GetChildRectForShrinkWrap(UIRect & childRect) const;
	virtual void WrapChildren();

	void SetDepth(float const depth);
	float GetDepth() const;

	void SetDepthOverride(bool depthOverride);
	bool GetDepthOverride() const;

	void SetAutoRegister(bool depthOverride);
	bool GetAutoRegister() const;

	void SetPalShade(float shade);
	float GetPalShade() const;

	void SetNotModifyingUseDefaultCursor(bool b);
	bool GetNotModifyingUseDefaultCursor() const;

	void SetForwardMoveToParent(bool b);
	bool GetForwardMoveToParent() const;

	void SetAcceptsMoveFromChildren(bool b);
	bool GetAcceptsMoveFromChildren() const;

private:

	// Copying of all widgets is forbidden, so this is private and not implemented
	                        UIWidget                    (const UIWidget & );

	// Assignment to any widget is forbidden, so this is private and not implemented
	        UIWidget &      operator =                  (const UIWidget &);

protected:
	        bool            SetAttribute                (AttributeBitFlags attribute, bool value);

	        void            RenderDefault               (UICanvas & , const UIWidgetRectangleStyles * fallback) const;

	        void            FillHorizontallyAndCap      (UICanvas &, UIScalar, UIImageStyle *, UIImageStyle *, UIImageStyle * ) const;
	        void            FillVerticallyAndCap        (UICanvas &, UIScalar, UIImageStyle *, UIImageStyle *, UIImageStyle * ) const;
	        void            FillVerticallyAndCapInverted( UICanvas &, UIScalar, UIImageStyle *, UIImageStyle *, UIImageStyle * ) const;

	        bool            SendUIMessageCallback       (const UIMessage &, UIWidget * delegateContext = 0 );

			typedef void (UIEventCallback::* CallbackFunc)( UIWidget *);

	        void            SendCallback                (CallbackFunc f, const UILowerString & ScriptPropertyName );
	        void            SendCallback                (CallbackFunc f, UIWidget *, const UILowerString & ScriptPropertyName );
	        void            SendDataChangedCallback     (const UILowerString & property, const UIString & value);

	        void            CancelEffector              (const UILowerString & ScriptPropertyName);

	UICursor *              GetUserModificationCursor   (UserModificationType type) const;
	UserModificationType    GetUserModificationType     (const UIPoint & pt) const;

	        void            SetCursorSet                (UICursorSet * cursorSet);

			void            SendTreeRowExpansionToggledCallback(int row);

private:

	float                     mOpacity;
	float                     mOpacityRelativeMin;
	float                     mTextOpacityRelativeMin;

	UIColor                   mColor;

	UIPoint                   mLocation;
	UISize                    mSize;
	UIPoint                   mScrollLocation;
	UISize                    mScrollExtent;

	float                     mRotation;

	UICursor                 *mCursor;
	UICursor                 *mDragBadCursor;
	UICursor                 *mDragGoodCursor;

	UIString * mTooltip;
	mutable UIString * mLocalTooltip;
	UITooltipStyle           *mTooltipStyle;

	typedef ui_stdvector<UIEventCallback *>::fwd EventCallbackList;
	EventCallbackList *       mCallbacks;

	unsigned int              mAttributeBits;
	UIPacking::SizeInfo *     mPackSizes [2];
	UIPacking::LocationInfo * mPackLocations [2];

	UIColor                   mBackgroundColor;
	UIColor                   mBackgroundTint;

	UIWidgetRectangleStyles * mRectangleStyles;

	UISize                    mMinimumSize;
	UISize                    mMaximumSize;
	UISize                    mMinimumScrollExtent;
	UISize                    mSizeIncrement;

	UserModificationType      mCurrentUserModificationType;
	UIPoint                   mUserModificationStartPoint;
	UIRect                    mUserModificationStartWidgetRect;

	UICursorSet *             mCursorSet;

	UIWidgetBoundaries *      mBoundaries;

	UIWidget *                mCustomDragWidget;

	UIPopupMenuStyle *        mPopupStyle;

	UISize                    mScrollSizePage;
	UISize                    mScrollSizeLine;

	UIWatcher<UIDeformer> * mDeformer;

	TooltipCallback * mTooltipCallback;

	// Depth is not a property.
	bool mDepthOverride : 1;
	float mDepth;
	bool mAutoRegister : 1;

	float mPalShade;

	bool mLockDiagonal : 1;
	unsigned char mResizeInset;  

	bool mNotModifyingUseDefaultCursor : 1;

	bool mForwardMoveToParent : 1;
	bool mAcceptsMoveFromChildren : 1;
};

//----------------------------------------------------------------------

inline const UIColor & UIWidget::GetBackgroundColor() const
{
	DEBUG_DESTROYED();

	return mBackgroundColor;
}

//----------------------------------------------------------------------

inline const UIColor & UIWidget::GetBackgroundTint() const
{
	DEBUG_DESTROYED();

	return mBackgroundTint;
}

//-----------------------------------------------------------------

inline const UIWidgetRectangleStyles * UIWidget::GetWidgetRectangleStyles () const
{
	DEBUG_DESTROYED();

	return mRectangleStyles;
}

//-----------------------------------------------------------------

inline UIWidgetRectangleStyles * UIWidget::GetWidgetRectangleStyles ()
{
	DEBUG_DESTROYED();

	return mRectangleStyles;
}

//----------------------------------------------------------------------

inline const   UISize & UIWidget::GetSizeIncrement () const
{
	DEBUG_DESTROYED();

	return mSizeIncrement;
}

//-----------------------------------------------------------------

inline float UIWidget::GetBackgroundOpacity () const
{
	DEBUG_DESTROYED();

	static const float oo_255 = 1.0f / 255.0f;
	return mBackgroundColor.a * oo_255;
}


//-----------------------------------------------------------------

inline UIWidgetBoundaries *      UIWidget::getBoundaries ()
{
	DEBUG_DESTROYED();

	return mBoundaries;
}

//----------------------------------------------------------------------

inline UIPopupMenuStyle *  UIWidget::GetPopupStyle ()
{
	DEBUG_DESTROYED();

	return mPopupStyle;
};

//----------------------------------------------------------------------

inline UIPopupMenuStyle const *  UIWidget::GetPopupStyle () const
{
	DEBUG_DESTROYED();

	return mPopupStyle;
};

//----------------------------------------------------------------------

inline bool UIWidget::IsContextCapable (bool primary) const
{
	DEBUG_DESTROYED();

	if (primary)
		return HasAttribute (BF_ContextCapable);
	else
		return HasAttribute (BF_ContextCapableAlternate);
}

//----------------------------------------------------------------------

inline void UIWidget::SetOpacity                (float NewOpacity )
{
	DEBUG_DESTROYED();

	mOpacity = NewOpacity;
}

//----------------------------------------------------------------------

inline float UIWidget::GetOpacity                () const
{
	DEBUG_DESTROYED();

	return mOpacity;
}

//----------------------------------------------------------------------

inline void UIWidget::SetOpacityRelativeMin     (float f )
{
	DEBUG_DESTROYED();

	mOpacityRelativeMin = f;
}

//----------------------------------------------------------------------

inline float UIWidget::GetOpacityRelativeMin     () const
{
	DEBUG_DESTROYED();

	return mOpacityRelativeMin;
}

//----------------------------------------------------------------------

inline UIColor UIWidget::GetColor                  () const
{
	DEBUG_DESTROYED();

	return mColor;
}

//----------------------------------------------------------------------

inline void UIWidget::SetRotation               (float NewRotation )
{
	DEBUG_DESTROYED();

	mRotation = NewRotation;
}

//----------------------------------------------------------------------

inline float UIWidget::GetRotation               () const
{
	DEBUG_DESTROYED();

	return mRotation;
}


//----------------------------------------------------------------------

inline float UIWidget::ComputeRelativeOpacity (float opacity, float opacityRelativeMin)
{
	const float opacityRelativeRange = 1.0f - opacityRelativeMin;
	const float threshold = 0.20f;

	float actualOpacity = (opacity * opacityRelativeRange) + opacityRelativeMin;

	if (threshold && opacity < opacityRelativeMin && opacity < threshold)
		actualOpacity *= opacity / threshold;

	return actualOpacity;
}

//----------------------------------------------------------------------

inline bool UIWidget::IsUserModifying() const
{
	DEBUG_DESTROYED();

	return HasAttribute(BF_UserModifying);
}

//----------------------------------------------------------------------

inline void UIWidget::SetTooltipCallback(TooltipCallback * tooltipCallback)
{
	DEBUG_DESTROYED();

	mTooltipCallback = tooltipCallback;
}

//----------------------------------------------------------------------

inline void UIWidget::SetDepth(float const depth)
{
	DEBUG_DESTROYED();

	mDepth = depth;
}

//----------------------------------------------------------------------

inline float UIWidget::GetDepth() const
{
	DEBUG_DESTROYED();

	return mDepth;
}

//----------------------------------------------------------------------

inline void UIWidget::SetDepthOverride(bool depthOverride)
{
	DEBUG_DESTROYED();

	mDepthOverride = depthOverride;
}

//----------------------------------------------------------------------

inline bool UIWidget::GetDepthOverride() const
{
	DEBUG_DESTROYED();
	
	return mDepthOverride;
}

//----------------------------------------------------------------------

inline void UIWidget::SetAutoRegister(bool const autoReg)
{
	mAutoRegister = autoReg;
}

//----------------------------------------------------------------------

inline bool UIWidget::GetAutoRegister() const
{
	return mAutoRegister;
}

//----------------------------------------------------------------------

inline void UIWidget::RenderText(UICanvas &) const
{

}
//----------------------------------------------------------------------

#endif // __UIWIDGET_H__
