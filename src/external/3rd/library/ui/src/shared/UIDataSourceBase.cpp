// ======================================================================
//
// UIDataSourceBase.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "_precompile.h"
#include "UIDataSourceBase.h"
#include <cassert>

#if WIN32
#pragma warning (disable:4505)
#endif

// ======================================================================

const char * const UIDataSourceBase::TypeName = "DataSourceBase";

//-----------------------------------------------------------------

UIDataSourceBase::UIDataSourceBase() :
UIBaseObject (),
UINotification (),
UINotificationServer ()
{
}

//-----------------------------------------------------------------

UIDataSourceBase::~UIDataSourceBase()
{
}

//-----------------------------------------------------------------

bool UIDataSourceBase::IsA( const UITypeID type) const
{
	return type == TUIDataSourceBase || UIBaseObject::IsA (type);
}

//-----------------------------------------------------------------

void  UIDataSourceBase::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode )
{
	assert (NotifyingObject != this);

	UI_UNREF (NotificationCode );
	UI_UNREF (NotifyingObject);
//	if(NotificationCode == ObjectChanged)
	{
		SendNotification( UINotification::ChildChanged, ContextObject );
	}
}

// ======================================================================
