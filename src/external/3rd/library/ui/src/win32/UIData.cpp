#include "_precompile.h"

#include "UIData.h"
#include "UIManager.h"
#include "UiMemoryBlockManager.h"

#include <cassert>
#include <map>

//-----------------------------------------------------------------

const char * const UIData::TypeName = "Data";

//----------------------------------------------------------------------

UIData::UIData () :
UIBaseObject             (),
UINotificationServer     (),
mGUID                    (0),
mAccumulateNotifications (false),
mHasChanged              (false)
{
}

//-----------------------------------------------------------------

UIBaseObject *UIData::Clone( void ) const
{
	return new UIData;
}

//-----------------------------------------------------------------

bool UIData::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( !UIBaseObject::SetProperty( Name, Value ) )
		return false;

	if (!Name.equals ("local", 5))
	{
		static const std::string localString ("local");

		const UILowerString & localName = UILowerString (localString + Name.get ());

		UIString localized;
		UIManager::gUIManager ().CreateLocalizedString (Value, localized);

		if (localized != Value)
		{
			UIBaseObject::SetProperty (localName, localized);
		}
		else
		{
			UIBaseObject::RemoveProperty (localName);
		}
	}

	mHasChanged = true;

	if (!mAccumulateNotifications)
		SendNotification( UINotification::ObjectChanged, this );

	return true;
}

//-----------------------------------------------------------------

bool UIData::RemoveProperty( const UILowerString & Name )
{
	bool retval = false;

	// make sure localized versions of the property are removed as well

	if (!Name.equals ("local", 5))
	{ 
		static const std::string localString ("local");
		const UILowerString & localName = UILowerString (localString + Name.get ());
		retval = RemoveProperty (localName);
	}

	retval = UIBaseObject::RemoveProperty (Name) || retval;

	if (retval)
		mHasChanged = true;

	return retval;
}

//-----------------------------------------------------------------

void UIData::ResetLocalizedStrings ()
{
	setAccumulateNotifications (true);

	UIBaseObject::UIPropertyNameMap * propsPtr = GetPropertyMap ();

	if (propsPtr == 0)
		return;

	// make a copy of property map
	UIBaseObject::UIPropertyNameMap props = *propsPtr;

	const UIBaseObject::UIPropertyNameMap::const_iterator end = props.end ();
	for (UIBaseObject::UIPropertyNameMap::const_iterator iter = props.begin (); iter != end; ++iter)
	{
		const UILowerString & lname = (*iter).first;

		// let SetProperty attempt to localized this string
		if ( !lname.equals ("local", 5))
		{
			SetProperty (lname, (*iter).second);
		}
	}

	setAccumulateNotifications (false);
}

//----------------------------------------------------------------------

void UIData::setAccumulateNotifications (bool b)
{
	mAccumulateNotifications = b;
	if (b)
		mHasChanged = false;
}

//----------------------------------------------------------------------

void UIData::SetGUID (long In )
{ 
	mGUID = In;
}

//----------------------------------------------------------------------

void UIData::Link       ()
{
	ResetLocalizedStrings ();
}

//----------------------------------------------------------------------
