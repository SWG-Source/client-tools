#ifndef __UICHECKBOXSTYLE_H__
#define __UICHECKBOXSTYLE_H__

#include "UIWidgetStyle.h"

class UITextStyle;
class UIImageStyle;

class UICheckboxStyle : public UIWidgetStyle
{
public:

	enum UICheckboxState
	{
		Unchecked = 0,
		Checked,
		SelectedUnchecked,
		SelectedChecked,
		DisabledUnchecked,
		DisabledChecked,
		LastCheckboxState,
	};

	static const char		 *TypeName;

	class PropertyName
	{
	public:

		static const UILowerString DisabledCheckedImage;
		static const UILowerString DisabledTextStyle;
		static const UILowerString DisabledUncheckedImage;
		static const UILowerString NormalCheckedImage;
		static const UILowerString NormalTextStyle;
		static const UILowerString NormalUncheckedImage;
		static const UILowerString SelectedCheckedImage;
		static const UILowerString SelectedUncheckedImage;
		static const UILowerString SetSound;
		static const UILowerString TextOffset;
		static const UILowerString UnsetSound;
	};

												UICheckboxStyle();
	virtual							 ~UICheckboxStyle();

	virtual bool					IsA( const UITypeID ) const;
	virtual const char	 *GetTypeName( void ) const;
	virtual UIBaseObject *Clone( void ) const;

	virtual void               GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void					GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void               GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool					SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool					GetProperty( const UILowerString & Name, UIString &Value ) const;

					void					SetTextStyle( bool, UITextStyle * );
					UITextStyle	 *GetTextStyle( bool ) const;

					void					SetImageStyle( UICheckboxState, UIImageStyle * );
					UIImageStyle *GetImageStyle( UICheckboxState ) const;

					void					SetTextOffset( const UIPoint & );
					void					GetTextOffset( UIPoint & ) const;

private:

					bool					LookupPropertyImageState( const UILowerString & Name, UICheckboxState &State ) const;

	UIPoint								mTextOffset;
	UIImageStyle				 *mImages[LastCheckboxState];
	UITextStyle					 *mNormalTextStyle;
	UITextStyle					 *mDisabledTextStyle;
};

#endif // __UICHECKBOXSTYLE_H__
