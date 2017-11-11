// ======================================================================
//
// UIDataSourceBase.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UIDataSourceBase_H
#define INCLUDED_UIDataSourceBase_H

#include "UIBaseObject.h"
#include "UINotification.h"

// ======================================================================

class UIDataSourceBase : 
public UIBaseObject,
public UINotification,
public UINotificationServer
{
public:

	static const char * const TypeName;

	                          UIDataSourceBase();
	virtual                  ~UIDataSourceBase() = 0;

	virtual bool              IsA( const UITypeID ) const;
	virtual const char       *GetTypeName( void ) const    { return TypeName; }

	virtual void              Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );

	virtual void              Clear( void ) = 0;

	virtual void              ResetLocalizedStrings () = 0;

	virtual bool              SetProperty( const UILowerString & Name, const UIString &Value ) = 0;

public:

private:
	UIDataSourceBase (const UIDataSourceBase & rhs);
	UIDataSourceBase & operator= (const UIDataSourceBase & rhs);
};

// ======================================================================

#endif
