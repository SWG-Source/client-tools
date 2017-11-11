#include "_precompile.h"

#include "UIPropertyDescriptor.h"
#include "UISliderplane.h"
#include "UISliderplaneStyle.h"
#include "UIImageStyle.h"
#include "UIUtils.h"
#include "UICanvas.h"
#include "UIMessage.h"
#include "UIEventCallback.h"
#include "UIButton.h"
#include "UIPage.h"

#include <cassert>
#include <list>
#include <vector>

const char *UISliderplane::TypeName	= "Sliderplane";

const UILowerString UISliderplane::PropertyName::LowerLimit = UILowerString ("LowerLimit");
const UILowerString UISliderplane::PropertyName::OnChange   = UILowerString ("OnChange");
const UILowerString UISliderplane::PropertyName::Style      = UILowerString ("Style");
const UILowerString UISliderplane::PropertyName::UpperLimit = UILowerString ("UpperLimit");
const UILowerString UISliderplane::PropertyName::Value      = UILowerString ("Value");
const UILowerString UISliderplane::PropertyName::ValueUpdateContinuous  = UILowerString ("ValueUpdateContinuous");

typedef UISliderplaneStyle::Image Image;

//======================================================================================
#define _TYPENAME UISliderplane

namespace UISliderplaneNamespace
{
	template <typename T> const T & UIClampRange (const T & min, const T & max, const T & val)
	{
		return std::max<T> (std::min<T> (max, val), min);
	}

	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(LowerLimit, "", T_point),
		_DESCRIPTOR(UpperLimit, "", T_point),
		_DESCRIPTOR(Style, "", T_object),
		_DESCRIPTOR(Value, "", T_point),
		_DESCRIPTOR(ValueUpdateContinuous, "", T_bool),
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================
	//================================================================
	// AdvancedBehavior category.
	_GROUPBEGIN(AdvancedBehavior)
		_DESCRIPTOR(OnChange, "", T_string),
	_GROUPEND(AdvancedBehavior, 2, int(UIPropertyCategories::C_AdvancedBehavior));
	//================================================================

}
using namespace UISliderplaneNamespace;
//======================================================================================

UISliderplane::UISliderplane() :
UIWidget (),
mStyle (0),
mLowerLimit (),
mUpperLimit (),
mValue (),
mDraggingSlider (false),
mDragMouseOrigin (),
mDragValueOrigin (),
mSliderButton (0),
mValueUpdateContinuous (true)
{
}

//-----------------------------------------------------------------

UISliderplane::~UISliderplane()
{
	SetStyle(0);

	if (mSliderButton)
		RemoveChild (mSliderButton);

	mSliderButton = 0;
}

//-----------------------------------------------------------------

bool UISliderplane::IsA( const UITypeID Type ) const
{
	return (Type == TUISliderplane) || UIWidget::IsA( Type );
}

//-----------------------------------------------------------------

const char *UISliderplane::GetTypeName( void ) const
{
	return TypeName;
}

//-----------------------------------------------------------------

UIBaseObject *UISliderplane::Clone( void ) const
{
	return new UISliderplane;
}

//----------------------------------------------------------------------

void UISliderplane::updateValueFromPoint (const UIPoint & point, bool notify)
{
	const UISize size   = GetSize ();
	const UISize bsize  = mSliderButton->GetSize ();

	UIPoint p = point - UISize (bsize.x / 2l, bsize.y / 2l);

	p.x = UIClampRange (0l, size.x - bsize.x, p.x);
	p.y = UIClampRange (0l, size.y - bsize.y, p.y);

	const UIPoint cur_bloc = mSliderButton->GetLocation ();

	if (cur_bloc != p)
	{
		mSliderButton->SetLocation (p);
		mValue = CalculateValueFromSliderLocation ();

		if (notify)
			SendCallback( &UIEventCallback::OnSliderplaneChanged, PropertyName::OnChange );
	}
}

//-----------------------------------------------------------------

bool UISliderplane::ProcessMessage( const UIMessage &msg )
{
	if( UIWidget::ProcessMessage( msg ) )
		return true;

	switch( msg.Type )
	{
	case UIMessage::LeftMouseDown:
	case UIMessage::RightMouseDown:
	case UIMessage::MiddleMouseDown:
		mDraggingSlider = true;
		updateValueFromPoint (msg.MouseCoords, mValueUpdateContinuous);
		return true;

	case UIMessage::LeftMouseUp:
	case UIMessage::RightMouseUp:
	case UIMessage::MiddleMouseUp:
		if (mDraggingSlider)
		{
			if (!mValueUpdateContinuous)
				SendCallback( &UIEventCallback::OnSliderplaneChanged, PropertyName::OnChange );
			mDraggingSlider = false;
		}
		break;

	case UIMessage::MouseMove:
		if (mDraggingSlider)
			updateValueFromPoint (msg.MouseCoords, mValueUpdateContinuous);

		break;
	}

	return false;
}

//-----------------------------------------------------------------

void UISliderplane::Render( UICanvas &DestinationCanvas ) const
{
	UIWidget::Render (DestinationCanvas);

	if (mSliderButton)
	{
		DestinationCanvas.Translate (mSliderButton->GetLocation ());
		mSliderButton->Render (DestinationCanvas);
	}
}

//-----------------------------------------------------------------

void UISliderplane::SetStyle( UISliderplaneStyle *NewStyle )
{
	if( NewStyle )
		NewStyle->Attach( this );

	if( mStyle )
		mStyle->Detach( this );

	mStyle = NewStyle;
}

//-----------------------------------------------------------------

UIStyle *UISliderplane::GetStyle( void ) const
{
	return mStyle;
};

//-----------------------------------------------------------------

void UISliderplane::SetValue( UIPoint In, bool notify )
{
	In.x = UIClampRange (mLowerLimit.x, mUpperLimit.x, In.x);
	In.y = UIClampRange (mLowerLimit.y, mUpperLimit.y, In.y);

	if( mValue != In )
	{
		mValue = In;

		if (mSliderButton)
		{
			mSliderButton->SetLocation (CalculateSliderLocationFromValue ());
		}

		if (notify)
			SendCallback( &UIEventCallback::OnSliderplaneChanged, PropertyName::OnChange );
	}
}

//-----------------------------------------------------------------

void UISliderplane::SetUpperLimit( const UIPoint & In )
{
	mLowerLimit.x = std::min (In.x, mLowerLimit.x);
	mLowerLimit.y = std::min (In.y, mLowerLimit.y);

	mUpperLimit = In;

	if( mValue.x > mUpperLimit.x || mValue.y > mUpperLimit.y )
	{
		mValue = mUpperLimit;

		if (mSliderButton)
		{
			mSliderButton->SetLocation (CalculateSliderLocationFromValue ());
		}

		SendCallback( &UIEventCallback::OnSliderplaneChanged, PropertyName::OnChange );
	}
}

//-----------------------------------------------------------------

void UISliderplane::SetLowerLimit( const UIPoint & In )
{
	mUpperLimit.x = std::max (In.x, mUpperLimit.x);
	mUpperLimit.y = std::max (In.y, mUpperLimit.y);

	mLowerLimit = In;

	if( mValue.y < mLowerLimit.x || mValue.y < mLowerLimit.y )
	{
		mValue = mLowerLimit;

		if (mSliderButton)
		{
			mSliderButton->SetLocation (CalculateSliderLocationFromValue ());
		}

		SendCallback( &UIEventCallback::OnSliderplaneChanged, PropertyName::OnChange );
	}
}

//-----------------------------------------------------------------
void UISliderplane::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::Style );

	UIWidget::GetLinkPropertyNames( In );

}

//----------------------------------------------------------------------

void UISliderplane::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(AdvancedBehavior, category, o_groups);
}

//----------------------------------------------------------------------

void UISliderplane::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::LowerLimit );
	In.push_back( PropertyName::OnChange );
	In.push_back( PropertyName::Style );
	In.push_back( PropertyName::UpperLimit );
	In.push_back( PropertyName::Value );
	In.push_back( PropertyName::ValueUpdateContinuous );

	UIWidget::GetPropertyNames( In, forCopy );
}

//-----------------------------------------------------------------

bool UISliderplane::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Value)
	{
		UIPoint NewValue;

		if( UIUtils::ParsePoint( Value, NewValue ) )
		{
			SetValue( NewValue, true );
			return true;
		}
		return false;
	}
	else if(Name == PropertyName::Style )
	{
		UIBaseObject *NewStyle = GetObjectFromPath( Value, TUISliderplaneStyle );

		if( NewStyle || Value.empty() )
		{
			SetStyle( reinterpret_cast<UISliderplaneStyle *>( NewStyle ) );
			return true;
		}
	}
	else if( Name == PropertyName::LowerLimit )
		return UIUtils::ParsePoint( Value, mLowerLimit );
	else if( Name == PropertyName::UpperLimit )
		return UIUtils::ParsePoint( Value, mUpperLimit );
	else if ( Name == PropertyName::ValueUpdateContinuous )
		return UIUtils::ParseBoolean( Value, mValueUpdateContinuous);

	return UIWidget::SetProperty( Name, Value );
}

//-----------------------------------------------------------------

bool UISliderplane::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Value )
		return UIUtils::FormatPoint( Value, mValue );
	else if( Name == PropertyName::LowerLimit )
		return UIUtils::FormatPoint( Value, mLowerLimit );
	else if( Name == PropertyName::UpperLimit )
		return UIUtils::FormatPoint( Value, mUpperLimit );
	else if( Name == PropertyName::Style )
	{
		if( mStyle )
		{
			GetPathTo( Value, mStyle );
			return true;
		}
	}
	else if (Name == PropertyName::ValueUpdateContinuous )
		return UIUtils::FormatBoolean( Value, mValueUpdateContinuous);

	return UIWidget::GetProperty( Name, Value );
}

//-----------------------------------------------------------------

const UIPoint UISliderplane::GetSliderPosition( void ) const
{
	return UIPoint ();
	/*

	UIImageStyle *Slider;
	long					Range;

	if( !mStyle )
		return 0;

	Slider = mStyle->GetImage( Image::Slider );

	if( !Slider )
	{
		Range = GetWidth();
	}
	else if( mStyle->GetLayout() == UIStyle::Layout::Horizontal )
	{
		Range = GetWidth() - Slider->GetWidth();
	}
	else
	{
		Range = GetHeight() - Slider->GetHeight();
	}

	return (long)(((float)(mValue - mLowerLimit) / (float)(mUpperLimit - mLowerLimit)) * Range);
	*/
}
//-----------------------------------------------------------------

UIWidget *UISliderplane::GetWidgetFromPoint( const UIPoint &PointToTest, bool mustGetInput ) const
{
	if (mSliderButton)
	{
		const UIPoint p = PointToTest - mSliderButton->GetLocation () + GetScrollLocation ();

		if (mSliderButton->WillDraw () && mSliderButton->HitTest (p))
			return mSliderButton->GetWidgetFromPoint (p, mustGetInput);
	}

	return UIWidget::GetWidgetFromPoint( PointToTest, mustGetInput );
}

//-----------------------------------------------------------------
void UISliderplane::Link( void )
{
	UIWidget::Link();

	if (mSliderButton)
		mSliderButton->Link ();
}
//-----------------------------------------------------------------
bool UISliderplane::AddChild( UIBaseObject * o)
{
	assert (o);

	if (!o->IsA (TUIButton))
		return false;

	if (mSliderButton)
	{
		mSliderButton->Detach (this);
	}

	mSliderButton = static_cast<UIButton *>(o);
	mSliderButton->AddCallback (this);
	o->SetParent( this );
	o->Attach (this);

	//-----------------------------------------------------------------
	//-- pop button to center

	mValue.x = (mUpperLimit.x + mLowerLimit.x) / 2;
	mValue.y = (mUpperLimit.y + mLowerLimit.y) / 2;

	mSliderButton->SetLocation (CalculateSliderLocationFromValue ());
	SendCallback( &UIEventCallback::OnSliderplaneChanged, PropertyName::OnChange );

	return true;
}
//-----------------------------------------------------------------

bool UISliderplane::RemoveChild( UIBaseObject * o)
{
	if (o != mSliderButton)
		return false;

	o->SetParent( 0 );
	mSliderButton->RemoveCallback (this);
	mSliderButton->Detach (this);
	mSliderButton = 0;

	return true;
}
//-----------------------------------------------------------------
void UISliderplane::SelectChild( UIBaseObject * o)
{
	assert (o);

	if (o != mSliderButton)
		return;

	mSliderButton->SetSelected (true);
	return;
}
//-----------------------------------------------------------------
UIBaseObject * UISliderplane::GetChild( const char * ObjectName ) const
{
	assert (ObjectName);

	if (mSliderButton && !_stricmp (mSliderButton->GetName ().c_str (), ObjectName))
		return mSliderButton;

	return 0;
}
//-----------------------------------------------------------------
void UISliderplane::GetChildren( UIObjectList & list) const
{
	if (mSliderButton)
		list.push_back (mSliderButton);
}
//-----------------------------------------------------------------
unsigned long UISliderplane::GetChildCount( void ) const
{
	return mSliderButton ? 1 : 0;
}
//-----------------------------------------------------------------
bool UISliderplane::OnMessage( UIWidget *Context, const UIMessage & msg )
{
	UI_UNREF (msg);
	UI_UNREF (Context);

	return true;
}

//-----------------------------------------------------------------
const UIPoint UISliderplane::CalculateSliderLocationFromValue () const
{
	const UISize size   = GetSize ();
	const UISize bsize  = mSliderButton->GetSize ();

	const UIFloatPoint range (static_cast<float>(size.x - bsize.x), static_cast<float>(size.y - bsize.y));

	const UIFloatPoint valueRange (static_cast<float>(mUpperLimit.x - mLowerLimit.x),
	                               static_cast<float>(mUpperLimit.y - mLowerLimit.y));

	if (valueRange.x == 0 || valueRange.y == 0)
		return UIPoint ();

	const UIFloatPoint valueRatio (static_cast<float>(mValue.x - mLowerLimit.x) / valueRange.x,
	                               static_cast<float>(mValue.y - mLowerLimit.y) / valueRange.y);

	//-----------------------------------------------------------------
	//-- invert the y axis

	const UIPoint p (static_cast<long>(valueRatio.x * range.x), size.y - bsize.y - static_cast<long>(valueRatio.y * range.y));

	return UIPoint (UIClampRange (0l, size.x - bsize.x, p.x), UIClampRange (0l, size.y - bsize.y, p.y));
}
//-----------------------------------------------------------------
const UIPoint UISliderplane::CalculateValueFromSliderLocation () const
{
	const UISize size    = GetSize ();
	const UISize bsize   = mSliderButton->GetSize ();

	//-----------------------------------------------------------------
	//-- invert the y axis

	const UIPoint a_bloc = mSliderButton->GetLocation ();
	const UIPoint bloc (a_bloc.x, size.y - bsize.y - a_bloc.y);

	const UIFloatPoint range (static_cast<float>(size.x - bsize.x), static_cast<float>(size.y - bsize.y));

	if (range.x == 0 || range.y == 0)
		return UIPoint ();

	const UIFloatPoint valueRange (static_cast<float>(mUpperLimit.x - mLowerLimit.x),
	                               static_cast<float>(mUpperLimit.y - mLowerLimit.y));

	if (valueRange.x == 0 || valueRange.y == 0)
		return UIPoint ();

	const UIPoint p (mLowerLimit.x + static_cast<long>(static_cast<float>(bloc.x * (mUpperLimit.x - mLowerLimit.x)) / range.x),
	                 mLowerLimit.y + static_cast<long>(static_cast<float>(bloc.y * (mUpperLimit.y - mLowerLimit.y)) / range.y));

	return UIPoint (UIClampRange (mLowerLimit.x, mUpperLimit.x, p.x), UIClampRange (mLowerLimit.y, mUpperLimit.y, p.y));
}
//-----------------------------------------------------------------
