//======================================================================
//
// UIImageFragment.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UIImageFragment.h"

#include "UICanvas.h"
#include "UIManager.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include <vector>
#include <cassert>

//----------------------------------------------------------------------

const char * const UIImageFragment::TypeName                         = "ImageFragment";

const UILowerString UIImageFragment::PropertyName::Offset             = UILowerString ("Offset");
const UILowerString UIImageFragment::PropertyName::OffsetProportional = UILowerString ("OffsetProportional");
const UILowerString UIImageFragment::PropertyName::SourceRect         = UILowerString ("SourceRect");
const UILowerString UIImageFragment::PropertyName::SourceResource     = UILowerString ("Source");

//======================================================================================
#define _TYPENAME UIImageFragment

namespace UIImageFragmentNamespace
{
	const UIPoint s_pct (100,100);

	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Offset, "", T_point),
		_DESCRIPTOR(OffsetProportional, "", T_point),
		_DESCRIPTOR(SourceRect, "", T_rect),
		_DESCRIPTOR(SourceResource, "", T_string),
	_GROUPEND(Basic, 1, 0);
	//================================================================
}
using namespace UIImageFragmentNamespace;
//======================================================================================

// UIImageFragment
// ------------------------------------------

UIImageFragment::UIImageFragment( void ) :
mSourceCanvas       (0),
mSourcePoint        (),
mSize               (),
mOffset             (),
mOffsetProportional (),
mSourceRectSet      (false)
{
}

//-----------------------------------------------------------------

UIImageFragment::~UIImageFragment( void )
{
	SetCanvas( 0 );
}

//-----------------------------------------------------------------

bool UIImageFragment::IsA( const UITypeID Type ) const
{
	return (Type == TUIImageFragment) || UIBaseObject::IsA( Type );
}

//-----------------------------------------------------------------

const char *UIImageFragment::GetTypeName( void ) const
{
	return TypeName;
}

//-----------------------------------------------------------------

UIBaseObject *UIImageFragment::Clone( void ) const
{
	return new UIImageFragment;
}

//-----------------------------------------------------------------

void UIImageFragment::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::SourceResource );

	UIBaseObject::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UIImageFragment::GetStaticPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category)
{
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIImageFragment::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIBaseObject::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIImageFragment::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::Offset );
	In.push_back( PropertyName::OffsetProportional );
	In.push_back( PropertyName::SourceRect );
	In.push_back( PropertyName::SourceResource );

	UIBaseObject::GetPropertyNames( In, forCopy );
}

//-----------------------------------------------------------------

bool UIImageFragment::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Offset )
		return UIUtils::ParsePoint( Value, mOffset );
	else if( Name == PropertyName::OffsetProportional )
		return UIUtils::ParsePoint( Value, mOffsetProportional );
	else if( Name == PropertyName::SourceRect )
	{
		UIRect SourceRect;

		if( !UIUtils::ParseRect( Value, SourceRect ) )
			return false;

		mSourceRectSet = true;

		mSourcePoint.x = SourceRect.left;
		mSourcePoint.y = SourceRect.top;
		mSize.x				 = SourceRect.right - SourceRect.left;
		mSize.y				 = SourceRect.bottom - SourceRect.top;

		return true;
	}
	else if( Name == PropertyName::SourceResource )
	{
		if (SetSourceResource( Value ))
			return true;
	}

	return UIBaseObject::SetProperty( Name, Value );
}

//-----------------------------------------------------------------

bool UIImageFragment::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Offset )
		return UIUtils::FormatPoint( Value, mOffset );
	else if( Name == PropertyName::OffsetProportional )
		return UIUtils::FormatPoint( Value, mOffsetProportional );
	else if( Name == PropertyName::SourceRect )
	{
		if( mSourceRectSet )
			UIUtils::FormatRect( Value, UIRect( mSourcePoint.x, mSourcePoint.y, mSourcePoint.x + mSize.x, mSourcePoint.y + mSize.y ) );
		else
			Value.erase ();

		return true;
	}
	else if( Name == PropertyName::SourceResource )
	{
		if( mSourceCanvas )
		{
			Value = Unicode::narrowToWide (mSourceCanvas->GetName());
			return true;
		}
	}

	return UIBaseObject::GetProperty( Name, Value );
}

//-----------------------------------------------------------------

void UIImageFragment::Render( UICanvas &DestinationCanvas, const UISize & size ) const
{
	if( !mSourceRectSet )
		mSourceCanvas->GetSize( const_cast<UIImageFragment *>( this )->mSize );

	const UIPoint pt (mOffset + (mOffsetProportional * size / s_pct));
	DestinationCanvas.BltFrom( mSourceCanvas, mSourcePoint, pt, size );
}

//-----------------------------------------------------------------

void UIImageFragment::Render( UICanvas &DestinationCanvas ) const
{
	if( !mSourceRectSet )
		mSourceCanvas->GetSize( const_cast<UIImageFragment *>( this )->mSize );

	const UIPoint pt (mOffset + (mOffsetProportional * mSize / s_pct));
	DestinationCanvas.BltFrom( mSourceCanvas, mSourcePoint, pt, mSize );
}

//-----------------------------------------------------------------

void UIImageFragment::SetCanvas( UICanvas *NewCanvas )
{
	if( NewCanvas )
		NewCanvas->Attach( this );

	if( mSourceCanvas )
		mSourceCanvas->Detach( this );

	mSourceCanvas = NewCanvas;
}

//-----------------------------------------------------------------

void UIImageFragment::SetSourceRect( const UIRect &NewSourceRect )
{
	mSourcePoint.x = NewSourceRect.left;
	mSourcePoint.y = NewSourceRect.top;

	mSize.x = NewSourceRect.Width();
	mSize.y = NewSourceRect.Height();

	mSourceRectSet = true;
}

//-----------------------------------------------------------------

bool UIImageFragment::SetSourceResource( const UIString &NewResourceName )
{
	UICanvas * const theCanvas = UIManager::isUIReady() ? UIManager::gUIManager().GetCanvas( NewResourceName ) : NULL;

	if (theCanvas)
		SetCanvas (theCanvas);
	else
	{
		SetCanvas (0);
		if( NewResourceName.empty() )
			return true;
	}

	return false;
}
//======================================================================
