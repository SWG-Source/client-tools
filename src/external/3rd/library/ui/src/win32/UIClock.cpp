#include "_precompile.h"

#include "UIClock.h"
#include "UIManager.h"

//-----------------------------------------------------------------

UIClock *UIClock::gSingleton = 0;

//-----------------------------------------------------------------

const char *UIClock::TypeName = "Clock";

//-----------------------------------------------------------------

UIClock::UIClock     () : 
UIBaseObject         (),
UINotificationServer (),
mTime                (0),
mLastFrameTime       (0)
{
	if (UIManager::isUIReady())
	{
		UIManager::gUIManager ().AddClock (this);
	}
}

//-----------------------------------------------------------------

UIClock::~UIClock ()
{
	if (UIManager::isUIReady())
	{
		UIManager::gUIManager ().RemoveClock (this);
	}
}

//-----------------------------------------------------------------

bool UIClock::IsA (const UITypeID Type) const
{
	return (Type == TUIClock) || UIBaseObject::IsA( Type );
}

//-----------------------------------------------------------------

const char *UIClock::GetTypeName () const
{
	return TypeName;
}

//-----------------------------------------------------------------

UIBaseObject *UIClock::Clone () const
{
	return new UIClock;
}

//-----------------------------------------------------------------

void UIClock::Advance( UITime AdvanceBy )
{
	if (!AdvanceBy)
		return;

	mLastFrameTime = AdvanceBy;

	mTime += AdvanceBy;

	SendNotification( UINotification::ObjectChanged, this, false );
}

//-----------------------------------------------------------------

void UIClock::Reset ()
{
	mTime          = 0;
	mLastFrameTime = 0;
};

//----------------------------------------------------------------------

void UIClock::ExplicitDestroy () 
{
	delete gSingleton; 
	gSingleton = 0;
};

//-----------------------------------------------------------------
