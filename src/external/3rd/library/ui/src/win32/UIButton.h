#ifndef __UIBUTTON_H__
#define __UIBUTTON_H__

#include "UIWidget.h"

class UIButtonStyle;
class UIImageStyle;

//----------------------------------------------------------------------

class UIButton : public UIWidget
{
public:

	enum ButtonAttributeBitFlags
	{
//		BF_Pressed          = 0x01,
		BABF_PressedByMouse     = 0x0002,
		BABF_IsDefault          = 0x0004,
		BABF_IsCancel           = 0x0008,
		BABF_Popup              = 0x0010,
		BABF_Toggle             = 0x0020,
		BABF_Repeat             = 0x0040,
		BABF_IconShrink         = 0x0080,
		BABF_IconStretch        = 0x0100,
		BABF_PressedStateSticky = 0x0200,
		BABF_AllMouseButtons    = 0x0400,
		BABF_ToggleDown         = 0x0800,
		BABF_PopupEatPress      = 0x1000,
		BABF_TextCapital        = 0x2000,
		BABF_AutoPressByDrag    = 0x4000,
		BABF_RenderIconBehind   = 0x8000
	};

	enum IconAlignment
	{
		IA_none,
		IA_left,
		IA_center,
		IA_right,
		IA_numAlignments
	};

	class IconAlignmentNames
	{
	public:
		static const Unicode::String None;
		static const Unicode::String Left;
		static const Unicode::String Center;
		static const Unicode::String Right;
	};

	class PropertyName
	{
	public:
		static const UILowerString IsCancelButton;
		static const UILowerString IsDefaultButton;
		static const UILowerString OnPress;
		static const UILowerString Style;
		static const UILowerString Text;
		static const UILowerString TextCapital;
		static const UILowerString LocalText;
		static const UILowerString TextColor;

		static const UILowerString PressSound;
		static const UILowerString ReleaseSound;
		static const UILowerString MouseOverSound;

		static const UILowerString Icon;
		static const UILowerString IconShrink;
		static const UILowerString IconStretch;
		static const UILowerString IconColor;
		static const UILowerString IconMargin;
		static const UILowerString IconAlignment;
		static const UILowerString IconMinSize;
		static const UILowerString IconMaxSize;

		static const UILowerString Toggle;
		static const UILowerString ToggleDown;

		static const UILowerString MaxTextLines;

		// If the icon image is in front of the background, set this flag to reverse the render order.
		static const UILowerString RenderIconBehind; 
	};

	class CategoryName
	{
	public:
		static const UILowerString Text;
		static const UILowerString Icon;
		static const UILowerString Behavior;
	};

	struct MethodName
	{
		static const UILowerString Press;
	};

	static const char     * const TypeName;

	                       UIButton           ();
	virtual               ~UIButton           ();

	virtual bool           IsA                (const UITypeID ) const;
	virtual const char    *GetTypeName        () const;
	virtual UIBaseObject  *Clone              () const;

	virtual void           GetPropertyNames   (UIPropertyNameVector &, bool forCopy ) const;
	virtual void                     GetLinkPropertyNames (UIPropertyNameVector &) const;

	virtual void           GetPropertyGroups       (UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void           GetCategories           (UIPropertyCategories::CategoryMask &o_categories) const;
	virtual void           GetPropertiesInCategory (UIPropertyCategories::Category category, UIPropertyNameVector &) const;


	virtual bool           SetProperty        (const UILowerString & Name, const UIString &Value );
	virtual bool           GetProperty        (const UILowerString & Name, UIString &Value ) const;

	virtual bool           ProcessMessage     (const UIMessage & );
	virtual void           Render             (UICanvas & ) const;

			//----------------------------------------------------------------------
			//-- these methods are for use by specialized widgets which store collections of labeled buttons
			//-- e.g. the tabbed pane

	void                   RenderBackground   (UICanvas & ) const;
	void                   RenderIcon         (UICanvas & ) const;
	void                   RenderText         (UICanvas & ) const;

	virtual bool           CanSelect          () const;

	void                   SetStyle           (UIButtonStyle * );
	UIButtonStyle const *  GetButtonStyle     () const { return mStyle; };
	virtual UIStyle       *GetStyle           () const;

	void                   SetText            (const UIString & );
	void                   GetText            (UIString & ) const;
	bool                   GetTextSize        (UISize & size) const;
	bool                   GetDesiredSize     (UISize & size) const;

	void                   SetLocalText       (const UIString & );
	void                   GetLocalText       (UIString & ) const;
	const UIString &       GetLocalText       () const;

	void                   SetIsDefaultButton (const bool );
	bool                   IsDefaultButton    () const;

	void                   SetIsCancelButton  (const bool );
	bool                   IsCancelButton     () const;

			//-- popup buttons behave differently than normal buttons
			//-- for instance, button presses do not cause a state change,
			//-- and any button release causes the button to be 'clicked'

	void                   SetIsPopupButton       (const bool );
	bool                   IsPopupButton          () const;

	void                   SetIsPopupEatPress     (const bool );
	bool                   IsPopupEatPress        () const;

	void                   SetIsAllMouseButtons   (const bool );
	bool                   IsAllMouseButtons      () const;

	void                   SetIsToggleButton      (const bool );
	bool                   IsToggleButton         () const;

	void                   SetIsToggleDown        (const bool );
	bool                   IsToggleDown           () const;

	void                   Press                  ();
	bool                   IsPressed              () const;

	virtual void           ResetLocalizedStrings  ();

	void                   SetIcon                (UIImageStyle * icon);
	UIImageStyle *         GetIcon                ();

	void                   SetIconColor           (const UIColor & color);
	const UIColor &        GetIconColor           () const;

	bool                   IsPressedByMouse       () const;
	void                   SetIsPressedByMouse    (bool b);

	void                   EnterPressedState      ();
	void                   LeavePressedState      ();

	void                   SetPressedStateSticky  (bool b);

	void                   SetIconAlignment       (IconAlignment ia);
	IconAlignment          GetIconAlignment       () const;

	bool                   GetTextCapital         () const;
	void                   SetTextCapital         (bool b);

	void                   ApplyStyle             (const UIButtonStyle * NewStyle);

	void                   SetAutoPressByDrag     (bool b);
	bool                   IsAutoPressByDrag      () const;

	void                   SetRenderIconBehind    (bool b);
	bool                   IsRenderIconBehind     () const;

private:

	                       UIButton               (const UIButton &);
	UIButton &             operator =             (const UIButton &);

	void                   MouseOver              ();

	bool                   handleButtonRelease    ();
	bool                   handleButtonPress      ();

	static IconAlignment          GetIconAlignmentFromString (const Unicode::String & str);
	static void                   GetStringFromIconAlignment (IconAlignment align, Unicode::String & str);
	bool                          calculateRenderPoint (UIPoint & renderPoint) const;

	UIButtonStyle               *mStyle;
	UIString                     mText;
	UIString                     mLocalText;

	unsigned short               mButtonAttributeBits;

	UIImageStyle *               mIcon;

	mutable long                 mIconOffsetLastRender;
	UIRect                       mIconMargin;

	UIColor                      mIconColor;

	IconAlignment                mIconAlignment;

	UISize                       mIconMaxSize;
	UISize                       mIconMinSize;

	long                         mMaxTextLines;
};

//-----------------------------------------------------------------

inline void UIButton::ResetLocalizedStrings ()
{
	SetText (mText);
}

//----------------------------------------------------------------------

inline bool UIButton::IsPressedByMouse       () const
{
	return (mButtonAttributeBits & BABF_PressedByMouse) != 0;
}

//----------------------------------------------------------------------

inline bool UIButton::IsDefaultButton    () const
{
	return (mButtonAttributeBits & BABF_IsDefault) != 0;
};

//----------------------------------------------------------------------

inline bool UIButton::IsCancelButton     () const
{
	return (mButtonAttributeBits & BABF_IsCancel) != 0;
};

//----------------------------------------------------------------------

//-- popup buttons behave differently than normal buttons
//-- for instance, button presses do not cause a state change,
//-- and any button release causes the button to be 'clicked'

inline bool UIButton::IsPopupButton          () const
{
	return (mButtonAttributeBits & BABF_Popup) != 0;
}

//----------------------------------------------------------------------

inline bool UIButton::IsAllMouseButtons () const
{
	return (mButtonAttributeBits & BABF_AllMouseButtons) != 0;
};

//----------------------------------------------------------------------

inline bool UIButton::IsPressed              () const
{
	return IsActivated ();
}

//----------------------------------------------------------------------

inline const UIString & UIButton::GetLocalText       () const
{
	return mLocalText;
}

//----------------------------------------------------------------------

inline bool UIButton::IsToggleButton        () const
{
	return (mButtonAttributeBits & BABF_Toggle) != 0;
}

//----------------------------------------------------------------------

inline bool UIButton::IsToggleDown          () const
{
	return IsToggleButton () && (mButtonAttributeBits & BABF_ToggleDown) != 0;
}

//----------------------------------------------------------------------

inline bool UIButton::IsPopupEatPress        () const
{
	return (mButtonAttributeBits & BABF_PopupEatPress) != 0;
}

//----------------------------------------------------------------------

inline UIButton::IconAlignment    UIButton::GetIconAlignment       () const
{
	return mIconAlignment;
}

//----------------------------------------------------------------------

inline bool UIButton::GetTextCapital         () const
{
	return (mButtonAttributeBits & BABF_TextCapital) != 0;

}

//----------------------------------------------------------------------

inline const UIColor & UIButton::GetIconColor           () const
{
	return mIconColor;
}

//----------------------------------------------------------------------

inline UIImageStyle * UIButton::GetIcon                ()
{
	return mIcon;
}

//----------------------------------------------------------------------

inline bool UIButton::IsAutoPressByDrag () const
{
	return (mButtonAttributeBits & BABF_AutoPressByDrag) != 0;
}

//----------------------------------------------------------------------

inline void UIButton::SetRenderIconBehind(bool b)
{
	if (b)
	{
		mButtonAttributeBits |= BABF_RenderIconBehind;
	}
	else
	{
		mButtonAttributeBits &= ~BABF_RenderIconBehind;
	}
}

//----------------------------------------------------------------------

inline bool UIButton::IsRenderIconBehind() const
{
	return (mButtonAttributeBits & BABF_RenderIconBehind) != 0;
}

//======================================================================================

#endif // __UIBUTTON_H__
