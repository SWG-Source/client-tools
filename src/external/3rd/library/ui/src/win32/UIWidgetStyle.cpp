#include "_precompile.h"

#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UIWidget.h"
#include "UIWidgetRectangleStyles.h"
#include "UIWidgetStyle.h"
#include <cassert>
#include <vector>

const char * const UIWidgetStyle::TypeName                            = "WidgetStyle";

const UILowerString UIWidgetStyle::PropertyName::OnShowEffector        = UILowerString ("OnShowEffector");
const UILowerString UIWidgetStyle::PropertyName::OnHideEffector        = UILowerString ("OnHideEffector");
const UILowerString UIWidgetStyle::PropertyName::OnEnableEffector      = UILowerString ("OnEnableEffector");
const UILowerString UIWidgetStyle::PropertyName::OnDisableEffector     = UILowerString ("OnDisableEffector");
const UILowerString UIWidgetStyle::PropertyName::MarginHotSpot         = UILowerString ("MarginHotSpot");

//----------------------------------------------------------------------
#define _TYPENAME UIWidgetStyle

namespace UIWidgetStyleNamespace
{
	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(MarginHotSpot,"",T_rect)
	_GROUPEND(Basic, 2, 0);
	//================================================================

	//================================================================
	// AdvancedBehavior category
	_GROUPBEGIN(AdvancedBehavior)
		_DESCRIPTOR(OnShowEffector,"",T_string),
		_DESCRIPTOR(OnHideEffector,"",T_string),
		_DESCRIPTOR(OnEnableEffector,"",T_string),
		_DESCRIPTOR(OnDisableEffector,"",T_string)
	_GROUPEND(AdvancedBehavior, 2, 1);
	//================================================================
}

using namespace UIWidgetStyleNamespace;

//----------------------------------------------------------------------

UIWidgetStyle::UIWidgetStyle () :
UIStyle (),
mRectangleStyles (0),
mMarginHotSpot   ()
{
	mRectangleStyles = new UIWidgetRectangleStyles;
}

//----------------------------------------------------------------------

UIWidgetStyle & UIWidgetStyle::operator= (const UIWidgetStyle & rhs)
{
	UIStyle::operator= (rhs);
	*mRectangleStyles = *rhs.mRectangleStyles;
	return *this;
}

//----------------------------------------------------------------------

UIWidgetStyle::UIWidgetStyle   (const UIWidgetStyle & rhs) :
UIStyle          (rhs),
mRectangleStyles (new UIWidgetRectangleStyles (*rhs.mRectangleStyles))
{
}

//----------------------------------------------------------------------

UIWidgetRectangleStyles & UIWidgetStyle::GetRectangleStyles ()
{
	assert (mRectangleStyles);
	return *mRectangleStyles;
}

//----------------------------------------------------------------------

const UIWidgetRectangleStyles & UIWidgetStyle::GetRectangleStyles () const
{
	assert (mRectangleStyles);
	return *mRectangleStyles;
}

//----------------------------------------------------------------------

void UIWidgetStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	if (mRectangleStyles)
		mRectangleStyles->GetPropertyNames (In);

	UIStyle::GetLinkPropertyNames (In);
}

//----------------------------------------------------------------------

void UIWidgetStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(AdvancedBehavior, category, o_groups);
	UIWidgetRectangleStyles::GetStaticPropertyGroups(o_groups, category);
}

//----------------------------------------------------------------------

void UIWidgetStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::OnShowEffector );
	In.push_back( PropertyName::OnHideEffector );
	In.push_back( PropertyName::OnEnableEffector );
	In.push_back( PropertyName::OnDisableEffector );
	In.push_back( PropertyName::MarginHotSpot );

	mRectangleStyles->GetPropertyNames (In);

	UIStyle::GetPropertyNames( In, forCopy );
}
//----------------------------------------------------------------------

bool UIWidgetStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::MarginHotSpot)
	{
		UIRect rect;
		if (UIUtils::ParseRect (Value, rect))
		{
			if (rect != mMarginHotSpot)
			{
				mMarginHotSpot = rect;
				FireStyleChanged ();
			}
			return true;
		}
		return false;
	}

	if (mRectangleStyles->SetProperty (*this, Name, Value))
		return true;

	return UIStyle::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool UIWidgetStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::MarginHotSpot)
	{
		return UIUtils::FormatRect (Value, mMarginHotSpot);
	}

	if (mRectangleStyles->GetProperty (*this, Name, Value))
		return true;

	return UIStyle::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

UIWidgetStyle::~UIWidgetStyle ()
{
	delete mRectangleStyles;
	mRectangleStyles = 0;
}

//----------------------------------------------------------------------

void UIWidgetStyle::FireStyleChanged ()
{
	SendNotification (UINotification::ObjectChanged, this);
}

//----------------------------------------------------------------------

void UIWidgetStyle::ApplyToWidget  (UIWidget & widget) const
{
	UIPropertyNameVector pnv;
	pnv.reserve (30);

	pnv.push_back( PropertyName::OnShowEffector );
	pnv.push_back( PropertyName::OnHideEffector );
	pnv.push_back( PropertyName::OnEnableEffector );
	pnv.push_back( PropertyName::OnDisableEffector );
	pnv.push_back( PropertyName::MarginHotSpot );
	
	Unicode::String tmp;
	{
		for (UIPropertyNameVector::const_iterator it = pnv.begin (); it != pnv.end (); ++it)
		{
			tmp.clear ();
			const UILowerString & key = (*it);
			if (GetProperty (key, tmp))
				widget.SetProperty (key, tmp);
		}
	}
	
	if (mRectangleStyles)
	{
		pnv.clear ();
		mRectangleStyles->GetPropertyNames (pnv);
		
		{
			for (UIPropertyNameVector::const_iterator it = pnv.begin (); it != pnv.end (); ++it)
			{
				tmp.clear ();
				const UILowerString & key = (*it);
				if (mRectangleStyles->GetProperty (widget, key, tmp))
					widget.SetProperty (key, tmp);
				
			}
		}
	}
	
	widget.Link ();
}

//----------------------------------------------------------------------

