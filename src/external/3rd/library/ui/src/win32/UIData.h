#ifndef __UIDATA_H__
#define __UIDATA_H__

#include "UIBaseObject.h"
#include "UINotification.h"
#include "UITypes.h"

//----------------------------------------------------------------------

class UIData : public UIBaseObject, public UINotificationServer
{
public:

	static const char * const TypeName;

	                         UIData                     ();

	bool                     IsA                        (const UITypeID ) const;
	const char *             GetTypeName                () const;
	UIBaseObject *           Clone                      () const;
	virtual void             Link                       ();

	bool                     SetProperty                (const UILowerString & Name, const UIString &Value );
	bool                     RemoveProperty             (const UILowerString & Name );

	void                     SetGUID                    (long In );
	long                     GetGUID                    () const;
	void                     ResetLocalizedStrings      ();

	void                     setAccumulateNotifications (bool b);
	bool                     getHasChanged              () const;


private:

	long                     mGUID;

	bool                     mAccumulateNotifications;
	bool                     mHasChanged;
};

//----------------------------------------------------------------------

inline bool UIData::getHasChanged () const
{
	return mHasChanged;
}

//----------------------------------------------------------------------

inline long UIData::GetGUID () const
{
	return mGUID;
};

//----------------------------------------------------------------------

inline bool UIData::IsA( const UITypeID Type ) const
{
	return (Type == TUIData) || UIBaseObject::IsA( Type );
}

//-----------------------------------------------------------------

inline const char *UIData::GetTypeName( void ) const
{
	return TypeName;
}

//----------------------------------------------------------------------

#endif // __UIDATA_H__

