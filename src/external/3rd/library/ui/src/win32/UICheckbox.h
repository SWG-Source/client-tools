#ifndef __UICHECKBOX_H__
#define __UICHECKBOX_H__

#include "UIWidget.h"

class UICheckboxStyle;

class UICheckbox : public UIWidget
{
public:

	class PropertyName
	{
	public:
		static const UILowerString Alignment;
		static const UILowerString Checked;
		static const UILowerString LocalText;
		static const UILowerString MaxLines;
		static const UILowerString OnSet;
		static const UILowerString OnUnset;
		static const UILowerString Radio;
		static const UILowerString SetSound;
		static const UILowerString Style;
		static const UILowerString Text;
		static const UILowerString TextColor;
		static const UILowerString UnsetSound;
	};

	enum Alignment
	{
		A_left,
		A_right
	};

	struct AlignmentNames
	{
		static const Unicode::String Left;
		static const Unicode::String Right;
	};

	static const char                  *TypeName;

	                                    UICheckbox();
	virtual                            ~UICheckbox();

	virtual bool                        IsA( const UITypeID ) const;
	virtual const char                 *GetTypeName () const;
	virtual UIBaseObject               *Clone () const;

	virtual void                        GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                        GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void                        GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool                        SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool                        GetProperty( const UILowerString & Name, UIString &Value ) const;

	virtual bool                        ProcessMessage( const UIMessage & );
	virtual void                        Render( UICanvas & ) const;

	virtual bool                        CanSelect () const;

	        void                        SetText( const UIString & );
	        void                        GetText( UIString & ) const;
	const UIString &                    GetText () const;

	        void                        SetLocalText( const UIString & );
	        void                        GetLocalText( UIString & ) const;
	const UIString &                    GetLocalText () const;

	        void                        SetStyle( UICheckboxStyle * );
	        UICheckboxStyle            *GetCheckboxStyle () const { return mStyle; };
	virtual UIStyle                    *GetStyle () const;

	        void                        SetChecked( const bool checked, const bool sendCallback = true );
	        bool                        IsChecked () const { return mChecked; };

	virtual void                        ResetLocalizedStrings ();

	const UIColor &                  GetTextColor () const;
	void                             SetTextColor (const UIColor & color);

	bool                                GetRadio () const;
	void                                SetRadio (bool b);

private:

	                                   UICheckbox( UICheckbox & );
	        UICheckbox                &operator = ( UICheckbox & );

	        void                       ToggleChecked( bool playSound, const bool sendCallback = true );

	UICheckboxStyle                   *mStyle;
	bool                               mChecked;
	UIString                           mText;
	UIString                           mLocalText;
	UIColor                            mTextColor;
	bool                               mRadio;
	int                                mMaxLines;
	int                                mAlignment;
};

//-----------------------------------------------------------------

inline void UICheckbox::ResetLocalizedStrings ()
{
	SetText (mText);
}

//----------------------------------------------------------------------

inline const UIColor & UICheckbox::GetTextColor () const
{
	return mTextColor;
}

//----------------------------------------------------------------------

inline bool UICheckbox::GetRadio () const
{
	return mRadio;
}

//----------------------------------------------------------------------

inline const UIString & UICheckbox::GetText () const
{
	return mText;
}

//----------------------------------------------------------------------

inline const UIString & UICheckbox::GetLocalText () const
{
	return mLocalText;
}

//----------------------------------------------------------------------

#endif // __UICHECKBOX_H__
