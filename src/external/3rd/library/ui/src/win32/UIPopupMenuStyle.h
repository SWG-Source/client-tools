#ifndef __UIPopupMenuStyle_H__
#define __UIPopupMenuStyle_H__

#include "UIWidgetStyle.h"

class UIImageStyle;
class UIButtonStyle;

//----------------------------------------------------------------------

class UIPopupMenuStyle : public UIWidgetStyle
{
public:

	static const char     * const TypeName;

	struct PropertyName
	{
		static const UILowerString ButtonStyle;
		static const UILowerString ButtonStyleLabel;
		static const UILowerString ItemHeight;
		static const UILowerString BackgroundOpacity;
		static const UILowerString IconSubmenu;
	};

	                        UIPopupMenuStyle     ();
	                       ~UIPopupMenuStyle     ();

	bool                    IsA                  (const UITypeID) const;
	const char *            GetTypeName          () const;
	UIBaseObject *          Clone                () const;

	virtual void            GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	void                    GetPropertyNames     (UIPropertyNameVector &, bool forCopy  ) const;
	virtual void            GetLinkPropertyNames (UIPropertyNameVector &) const;


	bool                    SetProperty          (const UILowerString & Name, const UIString &Value );
	bool                    GetProperty          (const UILowerString & Name, UIString &Value ) const;

	UIButtonStyle *         GetButtonStyle       () const;
	void                    SetButtonStyle       (UIButtonStyle *);

	UIButtonStyle *         GetButtonStyleLabel  () const;
	void                    SetButtonStyleLabel  (UIButtonStyle *);

	long                    GetItemHeight        () const;
	void                    Link                 ();
	float                   GetBackgroundOpacity () const;

	void                    SetIconSubmenu       (UIImageStyle * icon);
	UIImageStyle *          GetIconSubmenu       () const;

private:

	UIButtonStyle          *mButtonStyle;
	UIButtonStyle          *mButtonStyleLabel;

	long                    mItemHeight;

	float                   mBackgroundOpacity;
	UIImageStyle *          mIconSubmenu;
};

//----------------------------------------------------------------------

inline UIButtonStyle * UIPopupMenuStyle::GetButtonStyle () const
{
	return mButtonStyle;
}

//----------------------------------------------------------------------

inline long UIPopupMenuStyle::GetItemHeight () const
{
	return mItemHeight;
}

//----------------------------------------------------------------------

inline float UIPopupMenuStyle::GetBackgroundOpacity () const
{
	return mBackgroundOpacity;
}

//----------------------------------------------------------------------

inline UIImageStyle * UIPopupMenuStyle::GetIconSubmenu       () const
{
	return mIconSubmenu;
}

//----------------------------------------------------------------------

inline UIButtonStyle * UIPopupMenuStyle::GetButtonStyleLabel  () const
{
	return mButtonStyleLabel;
}

//-----------------------------------------------------------------

#endif // __UIPopupMenuStyle_H__