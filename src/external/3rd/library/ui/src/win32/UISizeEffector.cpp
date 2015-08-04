#include "_precompile.h"

#include "UIClock.h"
#include "UIPropertyDescriptor.h"
#include "UISizeEffector.h"
#include "UIUtils.h"
#include "UIWidget.h"

#include <cfloat>
#include <cmath>
#include <vector>

const char *UISizeEffector::TypeName									= "SizeEffector";
const UILowerString UISizeEffector::PropertyName::Speed				= UILowerString ("Speed");
const UILowerString UISizeEffector::PropertyName::TargetSize	= UILowerString ("TargetSize");

//======================================================================================
#define _TYPENAME UISizeEffector

namespace UISizeEffectorNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Speed, "", T_point),
		_DESCRIPTOR(TargetSize, "", T_point),
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UISizeEffectorNamespace;
//======================================================================================

//======================================================================================

UISizeEffector::UISizeEffector( void ) : mTargetSize(0,0),
mSizeSpeed(0,0),
mSizeChange(0,0),
mVerticalTime(0),
mVerticalTimePerPixel(FLT_MAX),
mHorizontalTime(0),
mHorizontalTimePerPixel(FLT_MAX)
{
	CalculateHeartbeatSkips();
	UIClock::gUIClock().Listen( this );
}

//======================================================================================

UISizeEffector::UISizeEffector( const UISize &TargetSize, const UISize &SizeSpeed ) :
mTargetSize(TargetSize),
mSizeSpeed(SizeSpeed),
mSizeChange(0,0),
mVerticalTime(0),
mVerticalTimePerPixel(FLT_MAX),
mHorizontalTime(0),
mHorizontalTimePerPixel(FLT_MAX)
{
	CalculateHeartbeatSkips();
	UIClock::gUIClock().Listen( this );
}

//======================================================================================

UISizeEffector::~UISizeEffector( void )
{
	UIClock::gUIClock().StopListening( this );
}

//======================================================================================

const char *UISizeEffector::GetTypeName( void ) const
{
	return TypeName;
}

//======================================================================================

UIBaseObject *UISizeEffector::Clone( void ) const
{
	return new UISizeEffector;
}

//======================================================================================

void UISizeEffector::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIEffector::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UISizeEffector::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back( PropertyName::Speed			);
	In.push_back( PropertyName::TargetSize	);
	
	UIEffector::GetPropertyNames( In, forCopy );
}

//======================================================================================

bool UISizeEffector::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Speed )
	{
		if( !UIUtils::ParsePoint( Value, mSizeSpeed ) )
			return false;

		CalculateHeartbeatSkips();
		return true;
	}
	else if( Name == PropertyName::TargetSize )
	{
		if( !UIUtils::ParsePoint( Value, mTargetSize ) )
			return false;

		CalculateHeartbeatSkips();
		return true;
	}
	else
		return UIEffector::SetProperty( Name, Value );
}

//======================================================================================

bool UISizeEffector::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Speed )
		return UIUtils::FormatPoint( Value, mSizeSpeed );
	else if( Name == PropertyName::TargetSize )
		return UIUtils::FormatPoint( Value, mTargetSize );
	else
		return UIEffector::GetProperty( Name, Value );
}

//======================================================================================

UIEffector::EffectResult UISizeEffector::Effect( UIBaseObject *theObject )
{
	EffectResult ReturnCode( Stop );
	
	if( theObject->IsA( TUIWidget ) )
	{
		UIWidget *theWidget		= reinterpret_cast<UIWidget *>( theObject );
		UISize		CurrentSize	= theWidget->GetSize();
		
		if( CurrentSize != mTargetSize )
		{
			if( CurrentSize.x < mTargetSize.x )
			{
				CurrentSize.x += mSizeChange.x;
				
				if( CurrentSize.x > mTargetSize.x )
					CurrentSize.x = mTargetSize.x;
				else
					ReturnCode = Continue;
			}
			else
			{
				CurrentSize.x -= mSizeChange.x;
				
				if( CurrentSize.x < mTargetSize.x )
					CurrentSize.x = mTargetSize.x;
				else
					ReturnCode = Continue;
			}
			
			if( CurrentSize.y < mTargetSize.y )
			{
				CurrentSize.y += mSizeChange.y;
				
				if( CurrentSize.y > mTargetSize.y )
					CurrentSize.y = mTargetSize.y;
				else
					ReturnCode = Continue;
			}
			else
			{
				CurrentSize.y -= mSizeChange.y;
				
				if( CurrentSize.y < mTargetSize.y )
					CurrentSize.y = mTargetSize.y;
				else
					ReturnCode = Continue;
			}
			
			theWidget->SetSize( CurrentSize );
		}		
	}
	
	return ReturnCode;
}

//======================================================================================

void UISizeEffector::CalculateHeartbeatSkips( void )
{
	if( mSizeSpeed.x > 0 )
		mHorizontalTimePerPixel = 1.0f / (float)mSizeSpeed.x;
	else
		mHorizontalTimePerPixel = FLT_MAX;

	if( mSizeSpeed.y > 0 )
		mVerticalTimePerPixel = 1.0f / (float)mSizeSpeed.y;
	else
		mVerticalTimePerPixel = FLT_MAX;
}

//======================================================================================

void UISizeEffector::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode )
{

	UI_UNREF (NotificationCode);
	UI_UNREF (ContextObject);
	UI_UNREF (NotifyingObject);

	const float HeartbeatTime = 1.0f/60.0f;

	mHorizontalTime += HeartbeatTime;
	mVerticalTime		+= HeartbeatTime;	

	mSizeChange.x    = (int)floor( mHorizontalTime / mHorizontalTimePerPixel );
	mHorizontalTime -= mSizeChange.x * mHorizontalTimePerPixel;

	mSizeChange.y = (int)floor( mVerticalTime / mVerticalTimePerPixel );
	mVerticalTime -= mSizeChange.y * mVerticalTimePerPixel;
}

//----------------------------------------------------------------------

void UISizeEffector::SetSpeed (const UISize & speed)
{
	mSizeSpeed = speed;
	CalculateHeartbeatSkips();
}

//----------------------------------------------------------------------
