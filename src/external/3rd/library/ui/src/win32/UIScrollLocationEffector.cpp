#include "_precompile.h"


#include "UIClock.h"
#include "UIPropertyDescriptor.h"
#include "UIScrollLocationEffector.h"
#include "UIUtils.h"
#include "UIWidget.h"

#include <cfloat>
#include <cmath>
#include <vector>

const char *UIScrollLocationEffector::TypeName														= "ScrollLocationEffector";
const UILowerString UIScrollLocationEffector::PropertyName::Speed									= UILowerString ("Speed");
const UILowerString UIScrollLocationEffector::PropertyName::TargetScrollLocation	= UILowerString ("TargetScrollLocation");

//======================================================================================
#define _TYPENAME UIScrollLocationEffector

namespace UIScrollLocationEffectorNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Speed, "", T_point),
		_DESCRIPTOR(TargetScrollLocation, "", T_point),
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UIScrollLocationEffectorNamespace;
//======================================================================================

//======================================================================================
UIScrollLocationEffector::UIScrollLocationEffector() :
mTargetScrollLocation(0,0),
mScrollLocationSpeed(0,0),
mScrollLocationChange(0,0),
mVerticalTime(0),
mVerticalTimePerPixel(FLT_MAX),
mHorizontalTime(0),
mHorizontalTimePerPixel(FLT_MAX)
{
	CalculateHeartbeatSkips();
	UIClock::gUIClock().Listen( this );
}

//======================================================================================
UIScrollLocationEffector::UIScrollLocationEffector( const UIPoint &TargetScrollLocation, const UIPoint &ScrollLocationSpeed ) :
mTargetScrollLocation(TargetScrollLocation),
mScrollLocationSpeed(ScrollLocationSpeed),
mScrollLocationChange(0,0),
mVerticalTime(0),
mVerticalTimePerPixel(FLT_MAX),
mHorizontalTime(0),
mHorizontalTimePerPixel(FLT_MAX)
{
	CalculateHeartbeatSkips();
	UIClock::gUIClock().Listen( this );
}

//======================================================================================
UIScrollLocationEffector::~UIScrollLocationEffector( void )
{
	UIClock::gUIClock().StopListening( this );
}

//======================================================================================
const char *UIScrollLocationEffector::GetTypeName( void ) const
{
	return TypeName;
}

//======================================================================================
UIBaseObject *UIScrollLocationEffector::Clone( void ) const
{
	return new UIScrollLocationEffector;
}

//======================================================================================

void UIScrollLocationEffector::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIEffector::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIScrollLocationEffector::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back( PropertyName::Speed			);
	In.push_back( PropertyName::TargetScrollLocation	);
	
	UIEffector::GetPropertyNames( In, forCopy );
}

//======================================================================================
bool UIScrollLocationEffector::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Speed )
	{
		if( !UIUtils::ParsePoint( Value, mScrollLocationSpeed ) )
			return false;

		CalculateHeartbeatSkips();
		return true;
	}
	else if( Name == PropertyName::TargetScrollLocation )
	{
		if( !UIUtils::ParsePoint( Value, mTargetScrollLocation ) )
			return false;

		CalculateHeartbeatSkips();
		return true;
	}
	else
		return UIEffector::SetProperty( Name, Value );
}

//======================================================================================
bool UIScrollLocationEffector::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Speed )
		return UIUtils::FormatPoint( Value, mScrollLocationSpeed );
	else if( Name == PropertyName::TargetScrollLocation )
		return UIUtils::FormatPoint( Value, mTargetScrollLocation );
	else
		return UIEffector::GetProperty( Name, Value );
}

//======================================================================================
UIEffector::EffectResult UIScrollLocationEffector::Effect( UIBaseObject *theObject )
{
	EffectResult ReturnCode( Stop );

	if( theObject->IsA( TUIWidget ) )
	{
		UIWidget *theWidget							= reinterpret_cast<UIWidget *>( theObject );
		UIPoint		CurrentScrollLocation	= theWidget->GetScrollLocation();

		if( CurrentScrollLocation != mTargetScrollLocation )
		{
			if( CurrentScrollLocation.x < mTargetScrollLocation.x )
			{
				CurrentScrollLocation.x += mScrollLocationChange.x;

				if( CurrentScrollLocation.x > mTargetScrollLocation.x )
					CurrentScrollLocation.x = mTargetScrollLocation.x;
				else
					ReturnCode = Continue;
			}
			else
			{
				CurrentScrollLocation.x -= mScrollLocationChange.x;

				if( CurrentScrollLocation.x < mTargetScrollLocation.x )
					CurrentScrollLocation.x = mTargetScrollLocation.x;
				else
					ReturnCode = Continue;
			}

			if( CurrentScrollLocation.y < mTargetScrollLocation.y )
			{
				CurrentScrollLocation.y += mScrollLocationChange.y;

				if( CurrentScrollLocation.y > mTargetScrollLocation.y )
					CurrentScrollLocation.y = mTargetScrollLocation.y;
				else
					ReturnCode = Continue;
			}
			else
			{
				CurrentScrollLocation.y -= mScrollLocationChange.y;

				if( CurrentScrollLocation.y < mTargetScrollLocation.y )
					CurrentScrollLocation.y = mTargetScrollLocation.y;
				else
					ReturnCode = Continue;
			}
			
			theWidget->SetScrollLocation( CurrentScrollLocation );
		}		
	}

	return ReturnCode;
}

//======================================================================================
void UIScrollLocationEffector::CalculateHeartbeatSkips( void )
{
	if( mScrollLocationSpeed.x > 0 )
		mHorizontalTimePerPixel = 1.0f / (float)mScrollLocationSpeed.x;
	else
		mHorizontalTimePerPixel = FLT_MAX;

	if( mScrollLocationSpeed.y > 0 )
		mVerticalTimePerPixel = 1.0f / (float)mScrollLocationSpeed.y;
	else
		mVerticalTimePerPixel = FLT_MAX;
}

//======================================================================================
void UIScrollLocationEffector::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode )
{
	UI_UNREF (NotificationCode);
	UI_UNREF (ContextObject);
	UI_UNREF (NotifyingObject);

	const float HeartbeatTime = 1.0f/60.0f;

	mHorizontalTime += HeartbeatTime;
	mVerticalTime		+= HeartbeatTime;	

	mScrollLocationChange.x  = (int)floor( mHorizontalTime / mHorizontalTimePerPixel );
	mHorizontalTime					-= mScrollLocationChange.x * mHorizontalTimePerPixel;

	mScrollLocationChange.y  = (int)floor( mVerticalTime / mVerticalTimePerPixel );
	mVerticalTime						-= mScrollLocationChange.y * mVerticalTimePerPixel;
}