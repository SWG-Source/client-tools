#include "_precompile.h"

#include "UICanvas.h"
#include "UIImageStyle.h"
#include "UIPage.h"
#include "UIPropertyDescriptor.h"
#include "UIRenderHelper.h"
#include "UITabSet.h"
#include "UITabSetStyle.h"
#include "UITemplate.h"
#include <cassert>
#include <list>
#include <vector>

const char *UITabSet::TypeName = "UITabSet";

const UILowerString UITabSet::PropertyName::Style		 = UILowerString ("Style");
const UILowerString UITabSet::PropertyName::Template = UILowerString ("Template");

//======================================================================================
#define _TYPENAME UITabSet

namespace UITabSetNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Style, "", T_object),
		_DESCRIPTOR(Template, "", T_object),
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UITabSetNamespace;
//======================================================================================

UITabSet::UITabSet( void ) : mStyle(0), mTemplate(0), mActivePage(0)
{
}

//----------------------------------------------------------------------

UITabSet::~UITabSet( void )
{
	SetStyle(0);
	SetTemplate(0);
}

//----------------------------------------------------------------------

bool UITabSet::IsA( const UITypeID Type ) const
{
	return (Type == TUITabSet) || UIWidget::IsA( Type );
}

const char *UITabSet::GetTypeName( void ) const
{
	return TypeName;
}

UIBaseObject *UITabSet::Clone( void ) const
{
	return new UITabSet;
}

//----------------------------------------------------------------------
void UITabSet::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	UIWidget::GetLinkPropertyNames (In);
}

//----------------------------------------------------------------------

void UITabSet::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UITabSet::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::Style );
	In.push_back( PropertyName::Template );

	UIWidget::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UITabSet::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Style )
	{
		UITabSetStyle *NewStyle = reinterpret_cast<UITabSetStyle *>( GetObjectFromPath( Value, TUITabSetStyle ) );

		if( NewStyle || Value.empty() )
			SetStyle( NewStyle );
	}
	else if( Name == PropertyName::Template )
	{
		UITemplate *NewTemplate = reinterpret_cast<UITemplate *>( GetObjectFromPath( Value, TUITemplate ) );

		if( NewTemplate || Value.empty() )
			SetTemplate( NewTemplate );
	}

	return UIWidget::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UITabSet::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Style )
	{
		if( mStyle )
		{
			GetPathTo( Value, mStyle );
			return true;
		}
	}
	else if( Name == PropertyName::Template )
	{
		if( mTemplate )
		{
			GetPathTo( Value, mTemplate );
			return true;
		}
	}

	return UIWidget::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UITabSet::ProcessMessage( const UIMessage & )
{
	return true;
}

void UITabSet::Render( UICanvas &DestinationCanvas ) const
{
	if( !mStyle )
		return;

	UIImageStyle *InactiveTabStartCap = mStyle->GetImage( UITabSetStyle::InactiveTabStartCap );
	int           TabHeight;

	if( InactiveTabStartCap )
		TabHeight = InactiveTabStartCap->GetHeight();
	else
		TabHeight = 0;

	/*
	Fill2D( DestinationCanvas,
					UIRect( 0, TabHeight, GetWidth(), GetHeight() ),
					mStyle->GetImage( UITabSetStyle::Image::BodyTopLeft ),
					mStyle->GetImage( UITabSetStyle::Image::BodyTop ),
					mStyle->GetImage( UITabSetStyle::Image::BodyTopRight ),
					mStyle->GetImage( UITabSetStyle::Image::BodyRight ),
					mStyle->GetImage( UITabSetStyle::Image::BodyBottomRight ),
					mStyle->GetImage( UITabSetStyle::Image::BodyBottom ),
					mStyle->GetImage( UITabSetStyle::Image::BodyBottomLeft ),
					mStyle->GetImage( UITabSetStyle::Image::BodyLeft ),
					mStyle->GetImage( UITabSetStyle::Image::BodyFill ) );
*/
	if( mActivePage )
	{
		UIWidgetVector w;

		w.push_back( mActivePage );

		DestinationCanvas.PushState();
		//DestinationCanvas.Translate(LeftEdgeSize, TopEdgeSize);
		UIRenderHelper::RenderObjects( DestinationCanvas, w);
		DestinationCanvas.PopState();
	}
}

bool UITabSet::AddChild( UIBaseObject *NewChild )
{
	if( NewChild->IsA( TUIPage ) )
	{
		mPages.push_back( reinterpret_cast<UIPage *>( NewChild ) );
		return true;
	}
	else
		return false;
}

UIBaseObject *UITabSet::GetChild( const char *ChildName ) const
{
	const char *pSeparator = strchr( ChildName, '.' );
	int					len;

	if( pSeparator )
		len = pSeparator - ChildName;
	else
		len = strlen( ChildName );

	for ( ; ; )
	{
		for( UIPageList::const_iterator i = mPages.begin(); i != mPages.end(); ++i )
		{
			UIPage *o = *i;

			if( o->IsName( ChildName, len ) )
			{
				if( pSeparator )
				{
					UIBaseObject *Child = o->GetChild( ChildName + len + 1 );

					if( Child )
						return Child;
				}
				else
					return o;
			}
		}

		if( pSeparator )
		{
			pSeparator = strchr( pSeparator + 1, '.' );

			if( pSeparator )
				len = pSeparator - ChildName;
			else
				len = strlen( ChildName );
		}
		else
			return 0;
	}
}

bool UITabSet::RemoveChild( UIBaseObject *ChildToRemove )
{
	for( UIPageList::iterator i = mPages.begin(); i != mPages.end(); ++i )
	{
		if( *i == ChildToRemove )
		{
			mPages.erase( i );

			if( ChildToRemove == mActivePage )
			{
				if( !mPages.empty() )
					mActivePage = mPages.front();
				else
					mActivePage = 0;
			}

			return true;
		}
	}
	return false;
}

void UITabSet::SelectChild( UIBaseObject *NewSelectedChild )
{
	assert( NewSelectedChild->IsA( TUIPage ) );

	mActivePage = reinterpret_cast<UIPage *>( NewSelectedChild );
}

void UITabSet::GetChildren( UIObjectList &Out ) const
{
	for( UIPageList::const_iterator i = mPages.begin(); i != mPages.end(); ++i )
		Out.push_back( *i );
}

unsigned long	UITabSet::GetChildCount( void ) const
{
	return mPages.size();
}

bool UITabSet::CanChildMove( UIBaseObject *ObjectToMove, ChildMovementDirection theDirection )
{
	if( !ObjectToMove )
		return false;

	for( UIPageList::const_iterator i = mPages.begin(); i != mPages.end(); ++i )
	{
		if( *i == ObjectToMove )
		{
			if( i == mPages.begin() )
			{
				if( (theDirection == Up) || (theDirection == Top) )
					return false;
			}
			else if( ++i == mPages.end() )
			{
				if( (theDirection == Down) || (theDirection == Bottom) )
					return false;
			}
			return true;
		}
	}
	return false;
}

bool UITabSet::MoveChild( UIBaseObject *ObjectToMove, ChildMovementDirection theDirection )
{
	if( !ObjectToMove )
		return false;

	for( UIPageList::iterator i = mPages.begin(); i != mPages.end(); ++i )
	{
		UIPage *o = *i;

		if( o == ObjectToMove )
		{
			if( (theDirection == Up) )
			{
				if( i != mPages.begin() )
				{
					mPages.insert( --i, 1, o );
					mPages.erase( ++i );
					return true;
				}
			}
			else if( (theDirection == Top) )
			{
				if( i != mPages.begin() )
				{
					mPages.insert( mPages.begin(), 1, o );
					mPages.erase(i);
					return true;
				}
			}
			else if( (theDirection == Down) )
			{
				if( i != mPages.end() )
				{
					UIPageList::iterator InsertPoint = i;

					++InsertPoint;

					if( InsertPoint != mPages.end() )
						++InsertPoint;

					mPages.insert( InsertPoint, 1, o );
					mPages.erase( i );
					return true;
				}
			}
			else if( (theDirection == Bottom) )
			{
				if( i != mPages.end() )
				{
					mPages.insert( mPages.end(), 1, o );
					mPages.erase( i );
					return true;
				}
			}
			else
				assert( false );

			return false;
		}
	}
	return false;
}

UIWidget *UITabSet::GetWidgetFromPoint( const UIPoint & ) const
{
	return const_cast<UITabSet *>( this );
}

void UITabSet::SetTemplate( UITemplate *NewTemplate )
{
	if( NewTemplate )
		NewTemplate->Attach(this);

	if( mTemplate )
		mTemplate->Detach(this);

	mTemplate = NewTemplate;
}

void UITabSet::SetStyle( UITabSetStyle *NewStyle )
{
	if( NewStyle )
		NewStyle->Attach(this);

	if( mStyle )
		mStyle->Detach(this);

	mStyle = NewStyle;
}

UIStyle *UITabSet::GetStyle( void ) const
{
	return mStyle;
};
