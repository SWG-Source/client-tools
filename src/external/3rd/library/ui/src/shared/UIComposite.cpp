// ======================================================================
//
// UIComposite.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "_precompile.h"
#include "UIComposite.h"

#include "UIPacking.h"
#include "UIPropertyDescriptor.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"

#include <cassert>
#include <list>
#include <vector>

#include <math.h>


// ======================================================================

const char * const  UIComposite::TypeName                    = "Composite";

//----------------------------------------------------------------------

const UILowerString  UIComposite::PropertyName::Orientation   = UILowerString ("Orientation");
const UILowerString  UIComposite::PropertyName::Spacing       = UILowerString ("Spacing");
const UILowerString  UIComposite::PropertyName::SpacingType   = UILowerString ("SpacingType");
const UILowerString  UIComposite::PropertyName::Alignment     = UILowerString ("Alignment");
const UILowerString  UIComposite::PropertyName::Padding       = UILowerString ("Padding");
const UILowerString  UIComposite::PropertyName::FillRemainder = UILowerString ("FillRemainder");

const std::string UIComposite::OrientationNames::Horizontal = "Horizontal";
const std::string UIComposite::OrientationNames::Vertical   = "Vertical";

const std::string UIComposite::AlignmentNames::Front        = "Front";
const std::string UIComposite::AlignmentNames::Center       = "Center";
const std::string UIComposite::AlignmentNames::Back         = "Back";

const std::string UIComposite::SpacingTypeNames::Fill       = "Fill";
const std::string UIComposite::SpacingTypeNames::Spread     = "Spread";
const std::string UIComposite::SpacingTypeNames::Constant   = "Constant";
const std::string UIComposite::SpacingTypeNames::Skinned   = "Skinned";

const std::string UIComposite::PaddingNames::Internal       = "Internal";
const std::string UIComposite::PaddingNames::External       = "External";
const std::string UIComposite::PaddingNames::Both           = "Both";

//======================================================================================
#define _TYPENAME UIComposite

namespace UICompositeNamespace
{
	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Orientation, "", T_string), // ENUM
		_DESCRIPTOR(Spacing, "", T_int),
		_DESCRIPTOR(SpacingType, "", T_string), // ENUM
		_DESCRIPTOR(Padding, "", T_string), // ENUM
		_DESCRIPTOR(FillRemainder, "", T_bool),
		_DESCRIPTOR(Alignment, "", T_string) // ENUM
	_GROUPEND(Basic, 3, 0);
	//================================================================

	size_t const s_maxNumberOfWidgets = 512;

}
using namespace UICompositeNamespace;

//======================================================================================

UIComposite::UIComposite() :
UIPage         (),
mInPacking     (false),
mOrientation   (O_vertical),
mSpacing       (0L),
mAlignment     (A_front),
mSpacingType   (ST_spread),
mPadding       (P_internal),
mFillRemainder (true)
{
}

//-----------------------------------------------------------------

UIComposite::~UIComposite()
{
}

//-----------------------------------------------------------------

bool UIComposite::IsA( const UITypeID type) const
{
	return type == TUIComposite || UIPage::IsA (type);
}

//-----------------------------------------------------------------

const char * UIComposite::GetTypeName() const
{
	return TypeName;
}

//-----------------------------------------------------------------

UIBaseObject * UIComposite::Clone() const
{
	return new UIComposite;
}

//----------------------------------------------------------------------

void UIComposite::GetVisibleWidgets   (WidgetVector & wv) const
{
	const UIObjectList & olist = GetChildrenRef ();

	wv.clear ();
	wv.reserve (olist.size ());
		
	for (UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIBaseObject * const obj = *it;
		
		if (obj->IsA (TUIWidget))
		{
			UIWidget * const wid = static_cast<UIWidget *>(obj);
			
			if (wid->WillDraw())
				wv.push_back (wid);
		}
	}
}

//----------------------------------------------------------------------

int UIComposite::CountVisibleWidgets () const
{
	WidgetVector wv;
	GetVisibleWidgets(wv);
	return static_cast<int>(wv.size());
}

//----------------------------------------------------------------------

UIPoint UIComposite::PackFill ()
{
	WidgetVector wv;
	GetVisibleWidgets   (wv);

	int widgetCount = static_cast<int>(wv.size ());

	if (!widgetCount)
		return UIPoint::zero;
	
	long totalLength       = 0L;
	long segmentLength     = 0L;
	long segmentLengthLast = 0L;
	
	if (mOrientation == O_vertical)
		totalLength = GetHeight ();
	else
		totalLength = GetWidth ();
	
	UIPoint startPt;
	if (mPadding == P_external || mPadding == P_both)
	{
		if (mOrientation == O_vertical)
			startPt.y += mSpacing / 2L;
		else
			startPt.x += mSpacing / 2L;

		totalLength -= mSpacing;
	}

	if (mPadding == P_internal || mPadding == P_both)
		 totalLength -= mSpacing * (widgetCount - 1);

	segmentLength     = totalLength / widgetCount;
	segmentLengthLast = segmentLength;

	if (mFillRemainder)
		segmentLengthLast += (totalLength % widgetCount);
	else
		totalLength -= (totalLength % widgetCount);
	
	WidgetVector sizeableWidgets (wv);

	int totalFilled = 0;

	{
		int index = 0;
		for (WidgetVector::iterator it = sizeableWidgets.begin (); it != sizeableWidgets.end ();)
		{
			UIWidget * const wid = *it;
			
			++index;
			
			long desiredLength = (index == widgetCount) ? segmentLengthLast : segmentLength;
			long actualDesiredLength = desiredLength;

			const UIPacking::SizeInfo * const packSize = (mOrientation == O_vertical) ? wid->getPackSize (1) : wid->getPackSize (0);
			if (packSize && packSize->m_type == UIPacking::SizeInfo::UserDefined)
				actualDesiredLength = static_cast<long>(static_cast<float>(desiredLength) * packSize->m_userProportion);

			long actualLength = 0L;
			if (mOrientation == O_vertical)
			{
				wid->SetHeight (actualDesiredLength);
				actualLength = wid->GetHeight ();
			}
			else
			{
				wid->SetWidth  (actualDesiredLength);
				actualLength = wid->GetWidth ();
			}
						
			if (actualLength < desiredLength)
				it = sizeableWidgets.erase (it);
			else
				++it;

			totalFilled += actualLength;
		}
	}
	
	if (totalFilled != totalLength)
	{
		const int sizeableWidgetCount = static_cast<int>(sizeableWidgets.size ());
		
		if (sizeableWidgetCount)
		{
			int deltaLength            = totalLength - totalFilled;
			int segmentDeltaLength     = deltaLength / sizeableWidgetCount;
			int segmentDeltaLengthLast = segmentDeltaLength + (deltaLength % sizeableWidgetCount);
			
			int index = 0;
			for (WidgetVector::const_iterator it = sizeableWidgets.begin (); it != sizeableWidgets.end (); ++it)
			{
				UIWidget * const wid = *it;
				
				++index;
				
				const long desiredDeltaLength = (index == sizeableWidgetCount) ? segmentDeltaLengthLast : segmentDeltaLength;
				long actualDesiredDeltaLength = desiredDeltaLength;

				const UIPacking::SizeInfo * const packSize = (mOrientation == O_vertical) ? wid->getPackSize (1) : wid->getPackSize (0);
				if (packSize && packSize->m_type == UIPacking::SizeInfo::UserDefined)
					actualDesiredDeltaLength = static_cast<long>(static_cast<float>(actualDesiredDeltaLength) * packSize->m_userProportion);

				long actualDeltaLength = 0L;
				
				if (mOrientation == O_vertical)
				{
					actualDeltaLength = wid->GetHeight ();
					wid->SetHeight (wid->GetHeight () + actualDesiredDeltaLength);
					actualDeltaLength = wid->GetHeight () - actualDeltaLength;
				}
				else
				{
					actualDeltaLength = wid->GetWidth ();
					wid->SetWidth  (wid->GetWidth () + actualDesiredDeltaLength);
					actualDeltaLength = wid->GetWidth () - actualDeltaLength;
				}

				if (actualDeltaLength != desiredDeltaLength)
				{
					const int numWidgetsRemaining = (sizeableWidgetCount - index);

					if (numWidgetsRemaining)
					{
						const long diff = desiredDeltaLength - actualDeltaLength;
						segmentDeltaLength     += diff / numWidgetsRemaining;
						segmentDeltaLengthLast += (diff / numWidgetsRemaining) + (diff % numWidgetsRemaining);
					}
				}
			}
		}
	}

	UIPoint pt (startPt);
	pt = startPt;
	
	{
		int index = 0;
		for (WidgetVector::const_iterator it = wv.begin (); it != wv.end (); ++it)
		{
			UIWidget * const wid = *it;
			++index;
			
			if (mOrientation == O_vertical)
			{
				pt.x = wid->GetLocation ().x;
				wid->SetLocation (pt);
				pt.y += wid->GetHeight ();
				if ((mPadding == P_internal || mPadding == P_both) && index != widgetCount)
					pt.y += mSpacing;
			}
			else
			{
				pt.y = wid->GetLocation ().y;
				wid->SetLocation (pt);
				pt.x += wid->GetWidth ();
				if ((mPadding == P_internal || mPadding == P_both) && index != widgetCount)
					pt.x += mSpacing;
			}
		}
	}

	return pt;
}

//----------------------------------------------------------------------

UIPoint UIComposite::PackConstant (long spacing)
{
	int widgetCount = CountVisibleWidgets ();

	if (!widgetCount)
		return UIPoint::zero;
	
	UIPoint pt;

	if (mPadding == P_external || mPadding == P_both)
	{
		if (mOrientation == O_vertical)
			pt.y += spacing / 2L;
		else
			pt.x += spacing / 2L;
	}

	const UIObjectList & olist = GetChildrenRef ();
	int index = 0;
	for (UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIBaseObject * const obj = *it;
		
		if (obj->IsA (TUIWidget))
		{
			UIWidget * const wid = static_cast<UIWidget *>(obj);
			
			if (!wid->WillDraw())
				continue;
			
			++index;
			
			if (wid->IsA (TUIText) ||wid->IsA (TUIComposite))
			{
				if (wid->IsA (TUIComposite))
					static_cast<UIComposite *>(wid)->Pack ();
				if (mOrientation == O_horizontal)
				{
					wid->SetWidth (0L);
					if (wid->IsA (TUIText))
					{
						UIText * const text = static_cast<UIText *>(wid);
						wid->SetWidth  (16384L);
						const UIRect & textMargin = text->GetMargin ();
						const UISize & textExtent = text->GetTextExtent ();
						wid->SetWidth (textExtent.x + textMargin.left + textMargin.right);
					}
				}
				else
				{
					if (wid->IsA (TUIText))
						wid->SetHeight (0L);
				}
			}
			
			UISize extent;			
			wid->GetScrollExtent (extent);
			
			if (mOrientation == O_vertical)
			{
				wid->SetLocation (wid->GetLocation ().x, pt.y);
				wid->SetHeight (extent.y);
				pt.y += wid->GetHeight ();
				if ((mPadding == P_internal || mPadding == P_both) && index != widgetCount)
					pt.y += spacing;
			}
			else
			{
				wid->SetLocation (pt.x, wid->GetLocation ().y);
				wid->SetWidth (extent.x);
				pt.x += wid->GetWidth ();
				if ((mPadding == P_internal || mPadding == P_both) && index != widgetCount)
					pt.x += spacing;
			}
		}
	}
	
	if (mPadding == P_external || mPadding == P_both)
	{
		if (mOrientation == O_vertical)
			pt.y += spacing / 2L;
		else
			pt.x += spacing / 2L;
	}
	
	return pt;
}

//----------------------------------------------------------------------

void UIComposite::PackAlignment (long diff, int widgetCount)
{
//	if (mSpacingType == ST_constant
	{
		if (widgetCount && mAlignment != A_front)
		{
			if (mAlignment == A_center)
				diff /= 2L;
			else if (mAlignment == A_back)
			{
				// nothing to do
			}				
			
			const UIObjectList & olist = GetChildrenRef ();
			for (UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
			{
				UIBaseObject * const obj = *it;
				
				if (obj->IsA (TUIWidget))
				{
					UIWidget * const wid = static_cast<UIWidget *>(obj);
					
					if (wid->WillDraw())
					{					
						UIPoint loc = wid->GetLocation ();
						if (mOrientation == O_vertical)
							loc.y += diff;
						else
							loc.x += diff;
						
						wid->SetLocation (loc);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void UIComposite::PackSpread (long diff, int widgetCount)
{
	if (widgetCount < 1 || (mPadding == P_internal && widgetCount < 2))
		return;

	long externalSpace     = 0L;
	long externalSpaceLast = 0L;
	long internalSpace     = 0L;
	long internalSpaceLast = 0L;

	if (mPadding == P_external)
	{
		externalSpace     = diff / 2L;
		externalSpaceLast = externalSpace + (diff % 2L);
	}
	else if (mPadding == P_internal)
	{
		if (widgetCount < 2)
			return;

		internalSpace     = diff / (widgetCount - 1L);
		internalSpaceLast = internalSpace + diff % (widgetCount - 1L);
	}
	else if (mPadding == P_both)
	{
		internalSpace     = diff / widgetCount;
		internalSpaceLast = internalSpace;
		externalSpace     = internalSpace / 2L;
		externalSpaceLast = externalSpace + (internalSpace % 2L);
	}
	
	UIPoint pt;
	
	if (mOrientation == O_vertical)
		pt.y += externalSpace;
	else
		pt.x += externalSpace;

	long index = 0;
	const UIObjectList & olist = GetChildrenRef ();
	for (UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIBaseObject * const obj = *it;
		
		if (obj->IsA (TUIWidget))
		{
			UIWidget * const wid = static_cast<UIWidget *>(obj);
			
			if (!wid->WillDraw())
				continue;
			
			if (mOrientation == O_vertical)
			{
				if (widgetCount > 1)
				{
					if (index == widgetCount - 1L)
						pt.y += internalSpaceLast;
					else if (index > 0)
						pt.y += internalSpace;
				}

				wid->SetLocation (wid->GetLocation ().x, pt.y);
				pt.y += wid->GetHeight ();
			}
			else
			{
				if (widgetCount > 1)
				{
					if (index == widgetCount - 1L)
						pt.x += internalSpaceLast;
					else if (index > 0)
						pt.x += internalSpace;
				}

				wid->SetLocation (pt.x, wid->GetLocation ().y);
				
				pt.x += wid->GetWidth ();
			}
			
			++index;
		}
	}
	
	if (mOrientation == O_vertical)
		pt.y += externalSpaceLast;
	else
		pt.x += externalSpaceLast;
}

//-----------------------------------------------------------------

void UIComposite::Pack()
{
	if (mInPacking)
		return;

	mInPacking = true;

	UIPoint pt;

	UIPage::Pack();

	switch(mSpacingType) 
	{
	case ST_fill:
		pt = PackFill();
		break;

	case ST_spread:
		pt = PackConstant(0);
		break;

	case ST_constant:
		pt = PackConstant(mSpacing);
		break;
		
	case ST_skinned:
		pt = PackSkinned();
		break;

	default:
		pt = PackFill();
		break;
	}

	if (mSpacingType != ST_skinned) 
	{
		UISize extent;
		GetScrollExtent (extent);
		
		if (mOrientation == O_vertical)
			extent.y = pt.y;
		else 
			extent.x = pt.x;
		
		SetScrollExtent (extent);
		
		UISize actualExtent;
		GetScrollExtent (actualExtent);
		long diff      = 0L;
		
		if (mOrientation == O_vertical)
			diff = actualExtent.y - extent.y;
		else
			diff = actualExtent.x - extent.x;
		
		if (diff > 0)
		{
			const int widgetCount = CountVisibleWidgets();
			
			if (mSpacingType == ST_constant || mSpacingType == ST_fill)
				PackAlignment(diff, widgetCount);
			else
				PackSpread(diff, widgetCount);
		}
	}

	mInPacking = false;
}

//-----------------------------------------------------------------

bool UIComposite::AddChild( UIBaseObject * child)
{
	if (UIPage::AddChild (child))
	{
		SetPackDirty (true);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

bool UIComposite::InsertChildBefore (UIBaseObject * childToAdd, const UIBaseObject * childToPrecede)
{
	if (UIPage::InsertChildBefore (childToAdd, childToPrecede))
	{
		SetPackDirty (true);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

bool UIComposite::InsertChildAfter  (UIBaseObject * childToAdd, const UIBaseObject * childTosucceed)
{
	if (UIPage::InsertChildAfter (childToAdd, childTosucceed))
	{
		SetPackDirty (true);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

bool UIComposite::RemoveChild( UIBaseObject * child)
{
	if (UIPage::RemoveChild (child))
	{
		SetPackDirty (true);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

bool UIComposite::MoveChild( UIBaseObject *child , ChildMovementDirection dir)
{
	if (UIPage::MoveChild (child, dir))
	{
		SetPackDirty (true);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

void UIComposite::Link( void )
{
	UIPage::Link();
	SetPackDirty (true);
}

//----------------------------------------------------------------------

void UIComposite::SetOrientation (Orientation t)
{
	if (t != mOrientation)
	{
		mOrientation = t;
		SetPackDirty (true);
	}
}

//----------------------------------------------------------------------

void UIComposite::SetAlignment (Alignment t)
{
	if (t != mAlignment)
	{
		mAlignment = t;
		SetPackDirty (true);
	}
}

//----------------------------------------------------------------------

void UIComposite::SetSpacing (long s)
{
	if (s != mSpacing)
	{
		mSpacing = s;
		SetPackDirty (true);
	}
}

//----------------------------------------------------------------------

void UIComposite::SetSpacingType (SpacingType type)
{
	if (type != mSpacingType)
	{
		mSpacingType = type;
		SetPackDirty (true);
	}
}

//----------------------------------------------------------------------

void UIComposite::SetPadding (Padding type)
{
	if (type != mPadding)
	{
		mPadding = type;
		SetPackDirty (true);
	}
}

//----------------------------------------------------------------------

void UIComposite::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIPage::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIComposite::GetPropertyNames( UIPropertyNameVector & in, bool forCopy) const
{
	if (!forCopy)
	{
		in.push_back (PropertyName::Orientation );
		in.push_back (PropertyName::Spacing     );
		in.push_back (PropertyName::SpacingType );
		in.push_back (PropertyName::Padding );
		in.push_back (PropertyName::FillRemainder);
		in.push_back (PropertyName::Alignment   );
	}

	UIPage::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

bool UIComposite::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::Orientation       )
	{
		if (Unicode::caseInsensitiveCompare (Value, OrientationNames::Horizontal, 0, Value.size ()))
			SetOrientation (O_horizontal);
		else if (Unicode::caseInsensitiveCompare (Value, OrientationNames::Vertical, 0, Value.size ()))
			SetOrientation (O_vertical);
		else
			return false;

		return true;
	}
	else if (Name == PropertyName::Alignment       )
	{
		if (Unicode::caseInsensitiveCompare (Value, AlignmentNames::Front, 0, Value.size ()))
			SetAlignment (A_front);
		else if (Unicode::caseInsensitiveCompare (Value, AlignmentNames::Center, 0, Value.size ()))
			SetAlignment (A_center);
		else if (Unicode::caseInsensitiveCompare (Value, AlignmentNames::Back, 0, Value.size ()))
			SetAlignment (A_back);
		else
			return false;

		return true;
	}
	else if (Name == PropertyName::SpacingType       )
	{
		if (Unicode::caseInsensitiveCompare (Value, SpacingTypeNames::Fill, 0, Value.size ()))
			SetSpacingType (ST_fill);
		else if (Unicode::caseInsensitiveCompare (Value, SpacingTypeNames::Spread, 0, Value.size ()))
			SetSpacingType (ST_spread);
		else if (Unicode::caseInsensitiveCompare (Value, SpacingTypeNames::Constant, 0, Value.size ()))
			SetSpacingType (ST_constant);
		else if (Unicode::caseInsensitiveCompare(Value, SpacingTypeNames::Skinned, 0, Value.size ()))
			SetSpacingType (ST_skinned);
		else
			return false;

		return true;
	}
	else if (Name == PropertyName::Padding)
	{
		if (Unicode::caseInsensitiveCompare (Value, PaddingNames::Internal, 0, Value.size ()))
			SetPadding (P_internal);
		else if (Unicode::caseInsensitiveCompare (Value, PaddingNames::External, 0, Value.size ()))
			SetPadding (P_external);
		else if (Unicode::caseInsensitiveCompare (Value, PaddingNames::Both, 0, Value.size ()))
			SetPadding (P_both);
		else
			return false;

		return true;
	}
	else if (Name == PropertyName::FillRemainder)
	{
		if (UIUtils::ParseBoolean (Value, mFillRemainder))
		{
			SetPackDirty (true);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::Spacing)
	{
		long tmp = 0L;
		if (UIUtils::ParseLong (Value, tmp))
		{
			SetSpacing (tmp);
			return true;
		}
		return false;
	}
	return UIPage::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool UIComposite::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::Orientation       )
	{
		if (mOrientation == O_horizontal)
			Value = Unicode::narrowToWide (OrientationNames::Horizontal);
		else
			Value = Unicode::narrowToWide (OrientationNames::Vertical);
		return true;
	}
	else if (Name == PropertyName::Alignment       )
	{
		if (mAlignment == A_front)
			Value = Unicode::narrowToWide (AlignmentNames::Front);
		else if (mAlignment == A_center)
			Value = Unicode::narrowToWide (AlignmentNames::Center);
		else
			Value = Unicode::narrowToWide (AlignmentNames::Back);
		return true;
	}
	else if (Name == PropertyName::SpacingType       )
	{
		if (mSpacingType == ST_fill)
			Value = Unicode::narrowToWide (SpacingTypeNames::Fill);
		else if (mSpacingType == ST_spread)
			Value = Unicode::narrowToWide (SpacingTypeNames::Spread);
		else if (mSpacingType == ST_constant)
			Value = Unicode::narrowToWide (SpacingTypeNames::Constant);
		else if (mSpacingType == ST_skinned)
			Value = Unicode::narrowToWide (SpacingTypeNames::Skinned);
		else
			return false;

		return true;
	}
	else if (Name == PropertyName::Padding)
	{
		if (mPadding == P_internal)
			Value = Unicode::narrowToWide (PaddingNames::Internal);
		else if (mPadding == P_external)
			Value = Unicode::narrowToWide (PaddingNames::External);
		else if (mPadding == P_both)
			Value = Unicode::narrowToWide (PaddingNames::Both);
		else
			return false;

		return true;
	}
	else if (Name == PropertyName::FillRemainder)
	{
		return UIUtils::FormatBoolean (Value, mFillRemainder);
	}
	else if (Name == PropertyName::Spacing)
	{
		return UIUtils::FormatLong (Value, GetSpacing ());
	}
	return UIPage::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

void UIComposite::CopyPropertiesFrom(const UIBaseObject & rhs)
{
	UIPage::CopyPropertiesFrom(rhs);

	if (rhs.IsA(TUIComposite))
	{
		UIComposite const & rhs_composite = static_cast<UIComposite const &>(rhs);		
		SetOrientation(rhs_composite.GetOrientation());
		SetAlignment(rhs_composite.GetAlignment());
		SetSpacingType(rhs_composite.GetSpacingType());
		SetPadding(rhs_composite.GetPadding());
		mFillRemainder = rhs_composite.mFillRemainder;
		SetSpacing(rhs_composite.GetSpacing());	
	}
}

//----------------------------------------------------------------------

UIPoint UIComposite::PackSkinned()
{
	// get widget vector.
	UISmartObjectVector wv;
	GetVisibleChildren(wv);

	// if we have a couple widgets
	size_t const widgetCount = wv.size();
	if(widgetCount == 0)
		return UIPoint::zero;

	// resize the widgets based on orientation.
	{
		UISize compositeSize(GetSize());
		
		for(UISmartObjectVector::iterator it = wv.begin(); it != wv.end(); ++it)
		{
			UIWidget * const w = UI_ASOBJECT(UIWidget, *it);
			if (w) 
			{
				if (mOrientation == O_vertical)
				{
					w->SetWidth(compositeSize.x);
				}
				else
				{
					w->SetHeight(compositeSize.y);
				}
			}
		}
	}


	// if we have one widget, set its location and return.
	if (widgetCount == 1) 
	{
		UIWidget * const leftWidget = UI_ASOBJECT(UIWidget, wv.front());
		leftWidget->SetLocation(UIPoint::zero);
		return leftWidget->GetSize();
	}

	// set the left widget to the 
	UIWidget * const leftWidget = UI_ASOBJECT(UIWidget, wv.front());
	if (leftWidget) 
	{
		leftWidget->SetLocation(UIPoint::zero);
	}

	// set the right widget.
	UIWidget * const rightWidget = UI_ASOBJECT(UIWidget, wv.back());
	if (rightWidget)
	{
		UIPoint nextLocation(GetSize() - rightWidget->GetSize());
		
		if (mOrientation == O_vertical)
		{
			nextLocation.x = 0;
		}
		else
		{
			nextLocation.y = 0;
		}
		
		rightWidget->SetLocation(nextLocation);
	}

	// get filler info.
	UIWidget const * const fillerWidget = GetFillerWidget();
	if (fillerWidget) 
	{
		size_t const currentFillerWidgetCount = widgetCount - 2;
		UISize const fillerSpaceAvailable = GetSize() - (leftWidget->GetSize() + rightWidget->GetSize());
		UISize const fillerObjectSize = fillerWidget->GetSize();
		if (fillerObjectSize.x > 0 && fillerObjectSize.y > 0) 
		{
			size_t requiredFillerWidgetCount = 0;
			if (mOrientation == O_vertical)
			{
				requiredFillerWidgetCount = static_cast<size_t>(ceilf(static_cast<float>(fillerSpaceAvailable.y) / fillerObjectSize.y));
			}
			else
			{
				requiredFillerWidgetCount = static_cast<size_t>(ceilf(static_cast<float>(fillerSpaceAvailable.x) / fillerObjectSize.x));
			}

			requiredFillerWidgetCount = std::min(requiredFillerWidgetCount, s_maxNumberOfWidgets);

			// add or clear out filler objects.
			if (requiredFillerWidgetCount > currentFillerWidgetCount) 
			{
				size_t const additionalFillerPageCount = requiredFillerWidgetCount - currentFillerWidgetCount;
				for (size_t fillerCount = 0; fillerCount < additionalFillerPageCount; ++fillerCount)
				{
					UIWidget * const newFiller = UI_ASOBJECT(UIWidget, fillerWidget->DuplicateObject());
					
					newFiller->SetVisible(true);
					newFiller->SetTransient(true);
					newFiller->SetGetsInput(false);
					newFiller->SetAbsorbsInput(false);
					
					InsertChildAfter(newFiller, leftWidget);

					newFiller->SetName("fillerObject");
					newFiller->Link();
				}
			}
			else if (requiredFillerWidgetCount < currentFillerWidgetCount)
			{
				size_t const removeFillerPageCount = currentFillerWidgetCount - requiredFillerWidgetCount;
				for (size_t fillerCount = 0; fillerCount < removeFillerPageCount; ++fillerCount)
				{
					UIWidget * const oldFiller = UI_ASOBJECT(UIWidget, wv[fillerCount + 1]);
					if (oldFiller) 
					{
						oldFiller->SetVisible(false);
						RemoveChild(oldFiller);
					}
				}
			}

			// space out the filler objects.
			if (requiredFillerWidgetCount) 
			{
				wv.clear();
				GetVisibleChildren(wv);
				
				UISize const fillerStepSize = fillerWidget->GetSize();
				UISize compositeSize(GetSize());
				UIPoint const startFillerLocation(leftWidget->GetSize());

				for (size_t fillerCount = 0; fillerCount < requiredFillerWidgetCount; ++fillerCount) 
				{
					UIPoint fillerLocation = startFillerLocation + UIPoint(fillerStepSize.x * fillerCount, fillerStepSize.y * fillerCount);
					UIWidget * const filler = UI_ASOBJECT(UIWidget, wv[fillerCount + 1]);
					if (filler) 
					{
						if (mOrientation == O_vertical)
						{
							fillerLocation.x = 0;
							filler->SetSize(UISize(compositeSize.x, fillerStepSize.y));
						}
						else
						{
							fillerLocation.y = 0;
							filler->SetSize(UISize(fillerStepSize.y, compositeSize.y));
						}

						filler->SetLocation(fillerLocation);
					}
				}
			}
		}
	}

	return GetSize();
}

//----------------------------------------------------------------------

UIWidget * UIComposite::GetFillerWidget()
{
	UIObjectList const & olist = GetChildrenRef();
	
	for (UIObjectList::const_iterator it = olist.begin(); it != olist.end (); ++it)
	{
		UIWidget * const page = UI_ASOBJECT(UIWidget, *it);
		if (page && !page->IsVisible() && page != olist.front() && page != olist.back())
		{
			return page;
		}
	}

	return NULL;
}

// ======================================================================

