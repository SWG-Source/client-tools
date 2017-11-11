#ifndef __UIWIDGETSTYLE_H__
#define __UIWIDGETSTYLE_H__

#include "UINotification.h"
#include "UIStyle.h"

class UIRectangleStyle;
class UIWidget;

//======================================================================================

class UIWidgetRectangleStyles;

class UIWidgetStyle :
public UIStyle,
public UINotificationServer
{
public:

	struct PropertyName
	{
		static const UILowerString OnShow;
		static const UILowerString OnShowEffector;
		static const UILowerString OnHide;
		static const UILowerString OnHideEffector;
		static const UILowerString OnEnable;
		static const UILowerString OnEnableEffector;
		static const UILowerString OnDisable;
		static const UILowerString OnDisableEffector;

		static const UILowerString MarginHotSpot;
	};

	static const char * const TypeName;

	virtual bool                     IsA                (const UITypeID Type) const;

	virtual void                     GetPropertyGroups    (UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const;
	virtual void                     GetPropertyNames     (UIPropertyNameVector &, bool) const;
	virtual void                     GetLinkPropertyNames (UIPropertyNameVector &) const;

	virtual bool                     SetProperty        (const UILowerString & Name, const UIString &Value);
	virtual bool                     GetProperty        (const UILowerString & Name, UIString &Value) const;

	                                 UIWidgetStyle      ();
	virtual                         ~UIWidgetStyle      ();
	UIWidgetRectangleStyles &        GetRectangleStyles ();
	const UIWidgetRectangleStyles &  GetRectangleStyles () const;

	const UIRect &                   GetMarginHotSpot   () const;

	void                             ApplyToWidget      (UIWidget & widget) const;

protected:

	UIWidgetStyle &                  operator=          (const UIWidgetStyle &);
	                                 UIWidgetStyle      (const UIWidgetStyle &);

	void                             FireStyleChanged ();

protected:

	UIWidgetRectangleStyles * mRectangleStyles;

	UIRect                    mMarginHotSpot;
};

//======================================================================================

inline bool UIWidgetStyle::IsA (const UITypeID Type) const
{
	return (Type == TUIWidgetStyle) || UIStyle::IsA( Type );
}

//----------------------------------------------------------------------

inline const UIRect & UIWidgetStyle::GetMarginHotSpot   () const
{
	return mMarginHotSpot;
}

//----------------------------------------------------------------------

#endif // __UIWIDGETSTYLE_H__