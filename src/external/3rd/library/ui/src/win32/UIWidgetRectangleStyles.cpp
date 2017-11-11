//======================================================================
//
// UIWidgetRectangleStyles.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UIWidgetRectangleStyles.h"

#include "UIBaseObject.h"
#include "UiMemoryBlockManager.h"
#include "UIPropertyDescriptor.h"
#include "UIRectangleStyle.h"
#include "UIWidget.h"

#include <cassert>
#include <vector>

//======================================================================

UI_MEMORY_BLOCK_MANAGER_IMPLEMENTATION(UIWidgetRectangleStyles, 1);

//======================================================================

const char * const UIWidgetRectangleStyles::TypeName                            = "UIWidgetRectangleStyles";

const UILowerString UIWidgetRectangleStyles::PropertyName::Default              = UILowerString ("RStyleDefault");
const UILowerString UIWidgetRectangleStyles::PropertyName::Disabled             = UILowerString ("RStyleDisabled");
const UILowerString UIWidgetRectangleStyles::PropertyName::Selected             = UILowerString ("RStyleSelected");
const UILowerString UIWidgetRectangleStyles::PropertyName::MouseOver            = UILowerString ("RStyleMouseOver");
const UILowerString UIWidgetRectangleStyles::PropertyName::MouseOverSelected    = UILowerString ("RStyleMouseOverSelected");
const UILowerString UIWidgetRectangleStyles::PropertyName::Activated            = UILowerString ("RStyleActivated");
const UILowerString UIWidgetRectangleStyles::PropertyName::MouseOverActivated   = UILowerString ("RStyleMouseOverActivated");
const UILowerString UIWidgetRectangleStyles::PropertyName::Text                 = UILowerString ("RStyleText");

//======================================================================================
#define _TYPENAME UIWidgetRectangleStyles

namespace UIWidgetRectangleStylesNamespace
{
	const UIPoint s_pct (100,100);

	//================================================================
	// Basic category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(Default, "", T_object),
		_DESCRIPTOR(Disabled, "", T_object),
		_DESCRIPTOR(Selected, "", T_object),
		_DESCRIPTOR(MouseOver, "", T_object),
		_DESCRIPTOR(MouseOverSelected, "", T_object),
		_DESCRIPTOR(Activated, "", T_object),
		_DESCRIPTOR(MouseOverActivated, "", T_object),
		_DESCRIPTOR(Text, "", T_object),
	_GROUPEND(Appearance, 1, 0);
	//================================================================
}
using namespace UIWidgetRectangleStylesNamespace;
//======================================================================================

//----------------------------------------------------------------------

UIWidgetRectangleStyles::UIWidgetRectangleStyles ()
{
	for (size_t i = 0; i < static_cast<size_t>(RS_LastStyle); ++i)
		mRectangleStyles [i] = 0;

}

//----------------------------------------------------------------------

UIWidgetRectangleStyles::~UIWidgetRectangleStyles ()
{
	for (size_t i = 0; i < static_cast<size_t>(RS_LastStyle); ++i)
		SetRectangleStyle (static_cast<RectangleStyle>(i), 0);
}

//----------------------------------------------------------------------

bool UIWidgetRectangleStyles::GetProperty (const UIBaseObject & obj, const UILowerString & Name, UIString & Value)
{
	const RectangleStyle rs = LookupRectangleStyleIDByName (Name);

	if (rs != RS_LastStyle)
	{
		const UIRectangleStyle * const rectStyle = mRectangleStyles[static_cast<size_t>(rs)];

		if( rectStyle )
		{
			obj.GetPathTo( Value, rectStyle );
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

bool UIWidgetRectangleStyles::SetProperty (const UIBaseObject & obj, const UILowerString & Name, const UIString & Value)
{
	const RectangleStyle rs = LookupRectangleStyleIDByName (Name);

	if (rs != RS_LastStyle)
	{
		UIRectangleStyle * const rectStyle = static_cast<UIRectangleStyle * >(obj.GetObjectFromPath(Value, TUIRectangleStyle ));

		if (rectStyle || Value.empty ())
		{
			SetRectangleStyle (rs, rectStyle);
			return true;
		}
	}

	return false;
}
//----------------------------------------------------------------------
void UIWidgetRectangleStyles::GetLinkPropertyNames( UIBaseObject::UIPropertyNameVector &In ) const
{
	GetPropertyNames (In);
}

//----------------------------------------------------------------------

void UIWidgetRectangleStyles::GetStaticPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category)
{
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UIWidgetRectangleStyles::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	GetStaticPropertyGroups(o_groups, category);
}

//----------------------------------------------------------------------

void UIWidgetRectangleStyles::GetPropertyNames( UIBaseObject::UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::Default);
	In.push_back( PropertyName::Disabled);
	In.push_back( PropertyName::Selected);
	In.push_back( PropertyName::MouseOver);
	In.push_back( PropertyName::MouseOverSelected);
	In.push_back( PropertyName::Activated);
	In.push_back( PropertyName::MouseOverActivated);
	In.push_back( PropertyName::Text);
}

//----------------------------------------------------------------------

UIWidgetRectangleStyles::RectangleStyle UIWidgetRectangleStyles::LookupRectangleStyleIDByName( const UILowerString &Name )
{
	if (Name == PropertyName::Default)
		return RS_Default;
	if (Name == PropertyName::Disabled)
		return RS_Disabled;
	if (Name == PropertyName::Selected)
		return RS_Selected;
	if (Name == PropertyName::MouseOver)
		return RS_MouseOver;
	if (Name == PropertyName::MouseOverSelected)
		return RS_MouseOverSelected;
	if (Name == PropertyName::Activated)
		return RS_Activated;
	if (Name == PropertyName::MouseOverActivated)
		return RS_MouseOverActivated;
	if (Name == PropertyName::Text)
		return RS_Text;

	return RS_LastStyle;
}
//----------------------------------------------------------------------

void UIWidgetRectangleStyles::SetRectangleStyle (RectangleStyle rs, UIRectangleStyle * style)
{
	if( style )
		style->Attach(0);

	if( mRectangleStyles[rs] )
		mRectangleStyles[rs]->Detach(0);

	mRectangleStyles[rs] = style;
}

//----------------------------------------------------------------------

UIRectangleStyle * UIWidgetRectangleStyles::GetAppropriateStyle (const UIWidget & widget, const UIWidgetRectangleStyles * fallback) const
{
	UI_UNREF (fallback);

	UIRectangleStyle * style = 0;
	UIRectangleStyle * const fb_style = fallback ? fallback->GetAppropriateStyle (widget, 0) : 0;

	if (!widget.IsEnabled ())
	{
		if (widget.IsActivated ())
			style = mRectangleStyles [RS_Activated];

		if (!style)
			style    = mRectangleStyles [RS_Disabled];
	}

	if (!style && widget.IsUnderMouse ())
	{
		if (widget.IsActivated ())
		{
			//----------------------------------------------------------------------
			//-- activated state takes precedence over selected always

			if ( (style = mRectangleStyles [RS_MouseOverActivated]) == 0)
				style = mRectangleStyles [RS_Activated];
		}


		if (!style && widget.IsSelected ())
			style = mRectangleStyles [RS_MouseOverSelected];

		if (!style)
			style = mRectangleStyles [RS_MouseOver];
	}

	if (!style && widget.IsActivated ())
		style = mRectangleStyles [RS_Activated];

	if (!style && widget.IsSelected ())
		style = mRectangleStyles [RS_Selected];

	if (!style)
		style = mRectangleStyles [RS_Default];

	return style ? style : fb_style;
}

//======================================================================
