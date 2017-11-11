#ifndef __UITEXTBOXSTYLE_H__
#define __UITEXTBOXSTYLE_H__

#include "UIWidget.h"
#include "UIWidgetStyle.h"

class UITextStyle;
class UIImageStyle;
class UICursor;
class UIGridStyle;

class UITextboxStyle :
public UIWidgetStyle
{
public:

   typedef UIWidget::VisualState VisualState;

	static const char * const TypeName;

	struct PropertyName
	{
		static const UILowerString CaratColor;
		static const UILowerString CaratWidth;
		static const UILowerString Cursor;
		static const UILowerString DisabledGridStyle;
		static const UILowerString DisabledTextStyle;
		static const UILowerString NormalGridStyle;
		static const UILowerString NormalTextStyle;
		static const UILowerString Padding;
		static const UILowerString SelectedGridStyle;
		static const UILowerString SelectedTextStyle;
		static const UILowerString SelectionColor;
		static const UILowerString SelectionOpacity;
		static const UILowerString TextColor;
		static const UILowerString SoundKeyclick;
	};

	UITextboxStyle( void );

	virtual               ~UITextboxStyle( void );

	virtual bool           IsA( const UITypeID ) const;
	virtual const char    *GetTypeName( void ) const;
	virtual UIBaseObject  *Clone( void ) const;

	virtual void           GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void           GetPropertyNames ( UIPropertyNameVector &, bool forCopy ) const;
	virtual void           GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool           SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool           GetProperty( const UILowerString & Name, UIString &Value ) const;

	void                   SetTextStyle( VisualState, UITextStyle * );
	UITextStyle            *GetTextStyle( VisualState ) const;

	void                   SetGridStyle( VisualState, UIGridStyle * );
	UIGridStyle            *GetGridStyle( VisualState ) const;

	void                   SetCaratColor( const UIColor & );
	UIColor                GetCaratColor( void ) const;

	void                   SetCaratWidth( const long );
	long                   GetCaratWidth( void ) const;

	void                   SetSelectionColor( const UIColor &NewSelectionColor ) { mSelectionColor = NewSelectionColor; };
	UIColor                GetSelectionColor( void ) const { return mSelectionColor; };

	void                   SetSelectionOpacity( float NewOpacity ) { mSelectionOpacity = NewOpacity; };
	float                  GetSelectionOpacity( void ) const { return mSelectionOpacity; };

	void                   SetMouseCursor( UICursor * );
	const UICursor        *GetMouseCursor( void ) const { return mCursor; };
	UICursor              *GetMouseCursor( void ) { return mCursor; };

	void                   SetTextPadding( const UIRect & );
	void                   GetTextPadding( UIRect & ) const;
	const UIRect          &GetTextPadding( void ) const { return mTextPadding; };

	const UIColor &        GetTextColor () const;

	void                   PlaySoundKeyclick () const;

private:

	VisualState            LookupGridStyleStateByName( const UILowerString & ) const;
	VisualState            LookupTextStyleStateByName( const UILowerString & ) const;

	UIColor                mCaratColor;
	long                   mCaratWidth;

	UIColor                mSelectionColor;
	float                  mSelectionOpacity;

	UIRect                 mTextPadding;
	UICursor              *mCursor;

	UIGridStyle           *mGridStyles[UIWidget::LastState];
	UITextStyle           *mTextStyles[UIWidget::LastState];

	UIColor                mTextColor;

	std::string            mSoundKeyclick;
};

//----------------------------------------------------------------------

inline const UIColor &         UITextboxStyle::GetTextColor () const
{
	return mTextColor;
}

//----------------------------------------------------------------------

#endif // __UITEXTBOXSTYLE_H__