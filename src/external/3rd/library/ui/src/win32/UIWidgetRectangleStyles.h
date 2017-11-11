//======================================================================
//
// UIWidgetRectangleStyles.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIWidgetRectangleStyles_H
#define INCLUDED_UIWidgetRectangleStyles_H

//======================================================================

class UIWidget;
class UIRectangleStyle;
class UiMemoryBlockManager;

#include "UIBaseObject.h"

/**
* Helper class to store multiple rectangle styles on various UIWidgets & UIWidgetStyles
*/

class UIWidgetRectangleStyles
{
	UI_MEMORY_BLOCK_MANAGER_INTERFACE;
public:

	typedef UIBaseObject::UIPropertyGroupVector UIPropertyGroupVector;

	enum RectangleStyle
	{
		RS_Default,
		RS_Disabled,
		RS_Selected,
		RS_MouseOver,
		RS_MouseOverSelected,
		RS_Activated,            // used for different things by different subclasses (e.g. pressed for buttons)
		RS_MouseOverActivated,
		RS_Text,
		RS_LastStyle
	};

	static const char * const TypeName;

	struct PropertyName
	{
		static const UILowerString Default;
		static const UILowerString Disabled;
		static const UILowerString Selected;
		static const UILowerString MouseOver;
		static const UILowerString MouseOverSelected;
		static const UILowerString Activated;
		static const UILowerString MouseOverActivated;
		static const UILowerString Text;
	};

	UIWidgetRectangleStyles ();
	~UIWidgetRectangleStyles ();

	static RectangleStyle  LookupRectangleStyleIDByName( const UILowerString & Name );
	void            SetRectangleStyle (RectangleStyle rs, UIRectangleStyle * style);

	UIRectangleStyle *        GetAppropriateStyle (const UIWidget & widget, const UIWidgetRectangleStyles * fallback) const;

	bool            GetProperty (const UIBaseObject & obj, const UILowerString & Name, Unicode::String & Value);
	bool            SetProperty (const UIBaseObject & obj, const UILowerString & Name, const Unicode::String & Value);
	void            GetPropertyNames( UIBaseObject::UIPropertyNameVector & ) const;
	virtual void    GetLinkPropertyNames (UIBaseObject::UIPropertyNameVector &) const;

	virtual void GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const;
	static void GetStaticPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES);

	UIRectangleStyle *              GetStyle (RectangleStyle rs);
	const UIRectangleStyle *        GetStyle (RectangleStyle rs) const;

private:
	UIRectangleStyle *        mRectangleStyles [RS_LastStyle];
};

//----------------------------------------------------------------------

inline UIRectangleStyle *        UIWidgetRectangleStyles::GetStyle (RectangleStyle rs)
{
	return mRectangleStyles [rs];
}

//----------------------------------------------------------------------

inline const UIRectangleStyle *        UIWidgetRectangleStyles::GetStyle (RectangleStyle rs) const
{
	return mRectangleStyles [rs];
}


//======================================================================

#endif
