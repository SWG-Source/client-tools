#ifndef __UIBUTTONSTYLE_H__
#define __UIBUTTONSTYLE_H__

#include "UIWidgetStyle.h"

class UITextStyle;
class UIImageStyle;

class UIButtonStyle : public UIWidgetStyle
{
public:

	enum UIButtonState
	{
		Normal = 0,
		Selected,
		Disabled,
		Pressed,
		MouseOver,
		LastButtonState,
	};

	static const char		  *TypeName;

	struct PropertyName
	{
		static const UILowerString  DisabledIconColor;
		static const UILowerString  DisabledTextColor;
		static const UILowerString  DisabledTextMargin;
		static const UILowerString  DisabledTextStyle;
		static const UILowerString  MouseOverIconColor;
		static const UILowerString  MouseOverSound;
		static const UILowerString  MouseOverTextColor;
		static const UILowerString  MouseOverTextMargin;
		static const UILowerString  MouseOverTextStyle;
		static const UILowerString  NormalIconColor;
		static const UILowerString  NormalTextColor;
		static const UILowerString  NormalTextMargin;
		static const UILowerString  NormalTextStyle;
		static const UILowerString  PressSound;
		static const UILowerString  PressedIconColor;
		static const UILowerString  PressedTextColor;
		static const UILowerString  PressedTextMargin;
		static const UILowerString  PressedTextStyle;
		static const UILowerString  ReleaseSound;
		static const UILowerString  SelectedIconColor;
		static const UILowerString  SelectedTextColor;
		static const UILowerString  SelectedTextMargin;
		static const UILowerString  SelectedTextStyle;
		static const UILowerString  TextAlignment;
		static const UILowerString  TextAlignmentVertical;

		static const UILowerString  TextCapital;
	};

	                       UIButtonStyle     ();
	virtual               ~UIButtonStyle     ();

	virtual bool           IsA               (const UITypeID) const;
	virtual const char    *GetTypeName       () const;
	virtual UIBaseObject  *Clone             () const;
	virtual void           Link              ();

	virtual void           GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const;
	virtual void           GetPropertyNames  (UIPropertyNameVector &, bool forCopy ) const;
	virtual void           GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool           SetProperty       (const UILowerString & Name, const UIString &Value );
	virtual bool           GetProperty       (const UILowerString & Name, UIString &Value ) const;

	void                   SetTextStyle      (UIButtonState, UITextStyle * );
	UITextStyle *          GetTextStyle      (UIButtonState ) const;

	const UIColor &        GetTextStyleColor (UIButtonState state) const;
	const UIColor &        GetIconColor      (UIButtonState state) const;
	const UIRect &         GetTextMargin     (UIButtonState state) const;


	int                    GetTextAlignment         () const;
	int                    GetTextAlignmentVertical () const;

	bool                   GetTextCapital           () const;
	void                   SetTextCapital           (bool b);

private:

	UIButtonStyle & operator=       (const UIButtonStyle &);
	                UIButtonStyle   (const UIButtonStyle &);


	enum StateElement
	{
		TextStyle,
		TextColor,
		TextMargin,
		IconColor
	};

	        bool            LookupPropertyStateAndElement( const char *Name, UIButtonState &State, StateElement &Element ) const;

	struct StateInfo
	{
		UITextStyle  *mTextStyle;
		UIColor       mTextColor;
		UIRect        mTextMargin;
		UIColor       mIconColor;

		StateInfo(void) : mTextStyle(0), mTextColor(0xFF,0xFF,0xFF), mTextMargin (), mIconColor (0xff,0xff,0xff) {};
	};

	StateInfo     mStateInfo[LastButtonState];

	short mTextAlignment;
	short mTextAlignmentVertical;
	bool mTextCapital;
};

//----------------------------------------------------------------------

inline int UIButtonStyle::GetTextAlignment         () const
{
	return mTextAlignment;
}

//----------------------------------------------------------------------

inline int UIButtonStyle::GetTextAlignmentVertical () const
{
	return mTextAlignmentVertical;
}

//----------------------------------------------------------------------

inline bool UIButtonStyle::GetTextCapital () const
{
	return mTextCapital;
}

//----------------------------------------------------------------------

#endif // __UIBUTTONSTYLE_H__