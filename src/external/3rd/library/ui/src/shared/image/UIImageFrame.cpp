//======================================================================
//
// UIImageFrame.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UIImageFrame.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include <list>
#include <vector>
#include <cassert>

//======================================================================
// UIImageFrame
// ------------------------------------------

const char *UIImageFrame::TypeName												= "ImageFrame";

//----------------------------------------------------------------------

const UILowerString UIImageFrame::PropertyName::Duration					= UILowerString ("Duration");

//======================================================================================
#define _TYPENAME UIImageFrame

namespace UIImageFrameNamespace
{
	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Duration, "", T_int),
	_GROUPEND(Basic, 2, 0);
	//================================================================
}
using namespace UIImageFrameNamespace;
//======================================================================================

UIImageFrame::UIImageFrame( void )
{
	mDuration  = 1;
	mFragments = 0;
}

//----------------------------------------------------------------------

UIImageFrame::~UIImageFrame()
{
	if( mFragments )
	{
		for( UIImageFragmentVector::iterator i = mFragments->begin(); i != mFragments->end(); ++i )
			(*i)->Detach( this );

		delete mFragments;
	}
}

//----------------------------------------------------------------------

bool UIImageFrame::IsA( const UITypeID Type ) const
{
	return (Type == TUIImageFrame) || UIImageFragment::IsA( Type );
}

//----------------------------------------------------------------------

const char *UIImageFrame::GetTypeName( void ) const
{
	return TypeName;
}

//----------------------------------------------------------------------

UIBaseObject *UIImageFrame::Clone( void ) const
{
	return new UIImageFrame;
}

//-----------------------------------------------------------------


void UIImageFrame::Render( UICanvas &DestinationCanvas, const UISize & size ) const
{
	if( mFragments )
	{
		for( UIImageFragmentVector::iterator i = mFragments->begin(); i != mFragments->end(); ++i )
			(*i)->Render( DestinationCanvas, size );
	}
	else
		UIImageFragment::Render( DestinationCanvas, size );
}

//-----------------------------------------------------------------

void UIImageFrame::Render( UICanvas &DestinationCanvas ) const
{
	if( mFragments )
	{
		for( UIImageFragmentVector::iterator i = mFragments->begin(); i != mFragments->end(); ++i )
			(*i)->Render( DestinationCanvas);
	}
	else
		UIImageFragment::Render( DestinationCanvas);
}

//----------------------------------------------------------------------

void UIImageFrame::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIImageFragment::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//-----------------------------------------------------------------

void UIImageFrame::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::Duration );

	UIImageFragment::GetPropertyNames( In, forCopy );
}

//-----------------------------------------------------------------

bool UIImageFrame::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Duration )
	{
		long Duration;

		if( !UIUtils::ParseLong( Value, Duration ) )
			return false;

		if( Duration < 1 )
			Duration = 1;

		mDuration = Duration;
		return true;
	}	
	else
		return UIImageFragment::SetProperty( Name, Value );
}

//-----------------------------------------------------------------

bool UIImageFrame::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Duration )
		return UIUtils::FormatLong( Value, mDuration );

	return UIImageFragment::GetProperty( Name, Value );
}

//-----------------------------------------------------------------

bool UIImageFrame::AddChild( UIBaseObject *NewChild )
{
	if( NewChild && NewChild->IsA( TUIImageFragment ) )
	{
		if( !mFragments )
			mFragments = new UIImageFragmentVector;			

		mFragments->push_back( reinterpret_cast<UIImageFragment *>( NewChild ) );

		NewChild->SetParent( this );
		NewChild->Attach( this );
		
		UIRect Extent( INT_MAX, INT_MAX, -INT_MAX, -INT_MAX );

		for( UIImageFragmentVector::iterator i = mFragments->begin(); i != mFragments->end(); ++i )
			UIUtils::UnionRect( Extent, (*i)->GetSourceRect() );

		SetSize( Extent.right - Extent.left, Extent.bottom - Extent.top );
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

bool UIImageFrame::RemoveChild( UIBaseObject *ChildToRemove )
{
	if( ChildToRemove->IsA( TUIImageFragment ) )
	{
		if( mFragments )
		{
			for( UIImageFragmentVector::iterator i = mFragments->begin(); i != mFragments->end(); ++i )
			{
				if( *i == ChildToRemove )
				{
					mFragments->erase(i);
					ChildToRemove->Detach( this );

					if( mFragments->size() == 0 )
					{
						delete mFragments;
						mFragments = 0;
					}
					return true;
				}
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------

void UIImageFrame::GetChildren( UIObjectList &Out ) const
{
	if( mFragments )
	{
		for( UIImageFragmentVector::iterator i = mFragments->begin(); i != mFragments->end(); ++i )
			Out.push_back( *i );
	}
}

//-----------------------------------------------------------------

unsigned long	UIImageFrame::GetChildCount( void ) const
{
	if( mFragments )
		return mFragments->size();
	else
		return 0;
}

//======================================================================
