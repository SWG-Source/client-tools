//======================================================================
//
// UICursorSet.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UICursorSet.h"

#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UICursor.h"

#include <cassert>
#include <vector>

//======================================================================

const char * const UICursorSet::TypeName                         = "CursorSet";

//----------------------------------------------------------------------

const UILowerString UICursorSet::PropertyName::Count = UILowerString ("Count");

//======================================================================================
#define _TYPENAME UICursorSet

namespace UICursorSetNamespace
{
	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Count, "", T_int),
	_GROUPEND(Basic, 1, 0);
	//================================================================
}
using namespace UICursorSetNamespace;
//======================================================================================

UICursorSet::UICursorSet () :
UIBaseObject (),
mCursors (0),
mCount (0)
{
}

//----------------------------------------------------------------------

UICursorSet::~UICursorSet ()
{
	for (int i = 0; i < mCount; ++i)
	{
		if (mCursors [i])
			mCursors [i]->Detach (this);
	}

	delete[] mCursors;
	mCursors = 0;
}

//-----------------------------------------------------------------

bool UICursorSet::IsA( const UITypeID Type ) const
{
	return (Type == TUICursorSet) || UIBaseObject::IsA (Type);
}

//----------------------------------------------------------------------

UICursor * UICursorSet::GetCursor (int index) const
{
	if (index < 0 || index >= mCount)
		return 0;

	return mCursors [index];
}

//----------------------------------------------------------------------

void UICursorSet::SetCursor (int index, UICursor * cursor)
{
	if (index < 0 || index >= mCount)
		return;

	if (cursor)
		cursor->Attach (this);

	if (mCursors [index])
		mCursors [index]->Detach (this);

	mCursors [index] = cursor;
}

//----------------------------------------------------------------------

void UICursorSet::GetLinkPropertyNames( UIPropertyNameVector & in) const
{
	in.push_back (PropertyName::Count);

	//-- this is required because the cursor names are stored in the base object property map
	UIBaseObject::GetPropertyNames (in, false);
}

//----------------------------------------------------------------------

void UICursorSet::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIBaseObject::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UICursorSet::GetPropertyNames( UIPropertyNameVector & in, bool forCopy) const
{
	in.push_back (PropertyName::Count);

	UIBaseObject::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

bool UICursorSet::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::Count)
	{
		long count;
		if (!UIUtils::ParseLong (Value, count))
			return false;

		if (count < 0)
			return false;

		SetCount (count);
		return true;
	}

	if (!Name.get ().empty () && isdigit (Name.get ()[0]))
	{
		const int index = atoi (Name.c_str ());

		if (index >= 0 && index < mCount)
		{
			UIBaseObject * const NewCursor = GetObjectFromPath( Value, TUICursor );

			if( NewCursor || Value.empty() )
			{
				SetCursor ( index, static_cast<UICursor *>(NewCursor));
			}
		}
		// fall through
	}

	return UIBaseObject::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool UICursorSet::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::Count)
	{
		return UIUtils::FormatLong (Value, mCount);
	}

	if (!Name.get ().empty () && isdigit (Name.get ()[0]))
	{
		const int index = atoi (Name.c_str ());
		UICursor * const cursor = GetCursor (index);

		if( cursor )
		{
			GetPathTo( Value, cursor );
			return true;
		}
		// fall through
	}

	return UIBaseObject::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

void UICursorSet::SetCount (int count)
{
	if (mCount == count || count < 0)
		return;

	UICursor ** newCursors = 0;

	if (count > 0)
	{
		newCursors = new UICursor * [count];
		{
			for (int i = 0; i < count; ++i)
				newCursors [i] = 0;
		}

		const int least = std::min (count, mCount);

		{
			for (int i = 0; i < least; ++i)
			{
				newCursors [i] = mCursors [i];
				if (newCursors [i])
					newCursors [i]->Attach (this);
			}
		}

		for (int i = 0; i < mCount; ++i)
		{
			if (mCursors [i])
				mCursors [i]->Detach (this);
		}

		mCount = count;
	}
	else
		mCount = 0;

	delete[] mCursors;
	mCursors = newCursors;
}

//----------------------------------------------------------------------

void UICursorSet::Link()
{
	UIBaseObject::Link ();

	for (int i = 0; i < mCount; ++i)
	{
		if (mCursors [i])
			mCursors [i]->Link ();
	}
}

//======================================================================
