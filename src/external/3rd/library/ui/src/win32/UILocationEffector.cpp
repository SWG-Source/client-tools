#include "_precompile.h"

#include "UIClock.h"
#include "UILocationEffector.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UIWidget.h"

#include <cfloat>
#include <cmath>
#include <vector>

const char *UILocationEffector::TypeName											= "LocationEffector";

//----------------------------------------------------------------------

const UILowerString UILocationEffector::PropertyName::Speed						= UILowerString ("Speed");
const UILowerString UILocationEffector::PropertyName::TargetLocation	= UILowerString ("TargetLocation");

//======================================================================================
#define _TYPENAME UILocationEffector

namespace UILocationEffectorNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Speed, "", T_point),
		_DESCRIPTOR(TargetLocation, "", T_point),
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UILocationEffectorNamespace;
//======================================================================================

//======================================================================================
UILocationEffector::UILocationEffector( void ) :
mTargetLocation(0,0),
mLocationSpeed(0,0),
mLocationChange(0,0),
mVerticalTime(0),
mVerticalTimePerPixel(FLT_MAX),
mHorizontalTime(0),
mHorizontalTimePerPixel(FLT_MAX)
{
	CalculateHeartbeatSkips();
	UIClock::gUIClock().Listen( this );
}

//======================================================================================
UILocationEffector::UILocationEffector( const UIPoint &TargetLocation, const UIPoint &LocationSpeed ) : 
mTargetLocation(TargetLocation),
mLocationSpeed(LocationSpeed),
mLocationChange(0,0),
mVerticalTime(0),
mVerticalTimePerPixel(FLT_MAX),
mHorizontalTime(0),
mHorizontalTimePerPixel(FLT_MAX)
{
	CalculateHeartbeatSkips();
	UIClock::gUIClock().Listen( this );
}

//======================================================================================
UILocationEffector::~UILocationEffector( void )
{
	UIClock::gUIClock().StopListening( this );
}

//======================================================================================
const char *UILocationEffector::GetTypeName( void ) const
{
	return TypeName;
}

//======================================================================================
UIBaseObject *UILocationEffector::Clone( void ) const
{
	return new UILocationEffector;
}

//======================================================================================

void UILocationEffector::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIEffector::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UILocationEffector::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back( PropertyName::Speed			);
	In.push_back( PropertyName::TargetLocation	);
	
	UIEffector::GetPropertyNames( In, forCopy );
}

//======================================================================================
bool UILocationEffector::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Speed )
	{
		if( !UIUtils::ParsePoint( Value, mLocationSpeed ) )
			return false;

		CalculateHeartbeatSkips();
		return true;
	}
	else if( Name == PropertyName::TargetLocation )
	{
		if( !UIUtils::ParsePoint( Value, mTargetLocation ) )
			return false;

		CalculateHeartbeatSkips();
		return true;
	}
	else
		return UIEffector::SetProperty( Name, Value );
}

//======================================================================================
bool UILocationEffector::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Speed )
		return UIUtils::FormatPoint( Value, mLocationSpeed );
	else if( Name == PropertyName::TargetLocation )
		return UIUtils::FormatPoint( Value, mTargetLocation );
	else
		return UIEffector::GetProperty( Name, Value );
}

//======================================================================================
UIEffector::EffectResult UILocationEffector::Effect( UIBaseObject *theObject )
{
	EffectResult ReturnCode( Stop );

	if( theObject->IsA( TUIWidget ) )
	{
		UIWidget *theWidget		= reinterpret_cast<UIWidget *>( theObject );
		UIPoint		CurrentLocation	= theWidget->GetLocation();

		if( CurrentLocation != mTargetLocation )
		{
			if( CurrentLocation.x < mTargetLocation.x )
			{
				CurrentLocation.x += mLocationChange.x;

				if( CurrentLocation.x > mTargetLocation.x )
					CurrentLocation.x = mTargetLocation.x;
				else
					ReturnCode = Continue;
			}
			else
			{
				CurrentLocation.x -= mLocationChange.x;

				if( CurrentLocation.x < mTargetLocation.x )
					CurrentLocation.x = mTargetLocation.x;
				else
					ReturnCode = Continue;
			}

			if( CurrentLocation.y < mTargetLocation.y )
			{
				CurrentLocation.y += mLocationChange.y;

				if( CurrentLocation.y > mTargetLocation.y )
					CurrentLocation.y = mTargetLocation.y;
				else
					ReturnCode = Continue;
			}
			else
			{
				CurrentLocation.y -= mLocationChange.y;

				if( CurrentLocation.y < mTargetLocation.y )
					CurrentLocation.y = mTargetLocation.y;
				else
					ReturnCode = Continue;
			}
			
			theWidget->SetLocation( CurrentLocation );
		}		
	}

	return ReturnCode;
}

//======================================================================================
void UILocationEffector::CalculateHeartbeatSkips( void )
{
	if( mLocationSpeed.x > 0 )
		mHorizontalTimePerPixel = 1.0f / (float)mLocationSpeed.x;
	else
		mHorizontalTimePerPixel = FLT_MAX;

	if( mLocationSpeed.y > 0 )
		mVerticalTimePerPixel = 1.0f / (float)mLocationSpeed.y;
	else
		mVerticalTimePerPixel = FLT_MAX;
}

//======================================================================================
void UILocationEffector::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode )
{
	UI_UNREF (NotificationCode);
	UI_UNREF (ContextObject);
	UI_UNREF (NotifyingObject);

	const float HeartbeatTime = 1.0f/60.0f;

	mHorizontalTime += HeartbeatTime;
	mVerticalTime		+= HeartbeatTime;	

	mLocationChange.x    = (int)floor( mHorizontalTime / mHorizontalTimePerPixel );
	mHorizontalTime -= mLocationChange.x * mHorizontalTimePerPixel;

	mLocationChange.y = (int)floor( mVerticalTime / mVerticalTimePerPixel );
	mVerticalTime -= mLocationChange.y * mVerticalTimePerPixel;
}