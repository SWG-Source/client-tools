#include "_precompile.h"

#include "UIProgressbar.h"
#include "UIProgressbarStyle.h"
#include "UIImageStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UICanvas.h"

#include <cassert>
#include <vector>

const char *UIProgressbar::TypeName								= "Progressbar";

//----------------------------------------------------------------------

const UILowerString UIProgressbar::PropertyName::Progress = UILowerString ("Progress");
const UILowerString UIProgressbar::PropertyName::Style		= UILowerString ("Style");

//======================================================================================
#define _TYPENAME UIProgressbar

namespace UIProgressbarNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Style, "", T_object),
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UIProgressbarNamespace;
//======================================================================================

UIProgressbar::UIProgressbar()
{
	mStyle    = 0;
	mProgress = 0;
}

UIProgressbar::~UIProgressbar()
{
	SetStyle(0);
}

bool UIProgressbar::IsA( const UITypeID Type ) const
{
	return (Type == TUIProgressbar) || UIWidget::IsA( Type );
}

const char *UIProgressbar::GetTypeName( void ) const
{
	return TypeName;
}

UIBaseObject *UIProgressbar::Clone( void ) const
{
	return new UIProgressbar;
}

bool UIProgressbar::ProcessMessage( const UIMessage &msg )
{
	return UIWidget::ProcessMessage( msg );
}

//----------------------------------------------------------------------

void UIProgressbar::Render( UICanvas &DestinationCanvas ) const
{
	if( !mStyle )
		return;

	UIImageStyle *StartCap   = mStyle->GetImage( UIProgressbarStyle::StartCap );
	UIImageStyle *Background = mStyle->GetImage( UIProgressbarStyle::Background );
	UIImageStyle *EndCap     = mStyle->GetImage( UIProgressbarStyle::EndCap );

	UISize				StartCapSize;
	UISize				EndCapSize;

	if( StartCap )
		StartCapSize = StartCap->GetSize();
	else
	{
		StartCapSize.x = 0;
		StartCapSize.y = 0;
	}

	if( EndCap )
		EndCapSize = EndCap->GetSize();
	else
	{
		EndCapSize.x = 0;
		EndCapSize.y = 0;
	}


	switch( mStyle->GetLayout() )
	{
		case UIStyle::L_horizontal:
		{
			if( mStyle->IsInverted() )
			{
			}
			else
			{
				int Range			  = GetWidth() - (StartCapSize.x + EndCapSize.x);
				int BarPosition = int(mProgress * float(Range));

				if( BarPosition % mStyle->GetStepSize() )
					BarPosition += mStyle->GetStepSize() - (BarPosition % mStyle->GetStepSize());

				if( BarPosition > Range )
					BarPosition = Range;

				FillHorizontallyAndCap( DestinationCanvas, GetWidth(), StartCap, Background, EndCap );
				FillHorizontallyAndCap( DestinationCanvas, BarPosition + StartCapSize.x + EndCapSize.x,
					mStyle->GetImage( UIProgressbarStyle::BarStartCap ), mStyle->GetImage( UIProgressbarStyle::BarBackground ), mStyle->GetImage( UIProgressbarStyle::BarEndCap ) );
			}
			break;
		}
		case UIStyle::L_vertical:
		{
			if( mStyle->IsInverted() )
			{
				int Range				= GetHeight() - (StartCapSize.y + EndCapSize.y);
				int BarPosition = int(mProgress * float(Range));

				if( BarPosition % mStyle->GetStepSize() )
					BarPosition += mStyle->GetStepSize() - (BarPosition % mStyle->GetStepSize());

				if( BarPosition > Range )
					BarPosition = Range;

				FillVerticallyAndCap( DestinationCanvas, GetHeight(), StartCap, Background, EndCap );
				FillVerticallyAndCap( DestinationCanvas, BarPosition + StartCapSize.y + EndCapSize.y,
					mStyle->GetImage( UIProgressbarStyle::BarStartCap ), mStyle->GetImage( UIProgressbarStyle::BarBackground ), mStyle->GetImage( UIProgressbarStyle::BarEndCap ) );
			}
			else
			{
				int Range				= GetHeight() - (StartCapSize.y + EndCapSize.y);
				int BarPosition = int(mProgress * float(Range));

				if( BarPosition % mStyle->GetStepSize() )
					BarPosition += mStyle->GetStepSize() - (BarPosition % mStyle->GetStepSize());

				if( BarPosition > Range )
					BarPosition = Range;

				FillVerticallyAndCapInverted( DestinationCanvas, GetHeight(), StartCap, Background, EndCap );

				DestinationCanvas.PushState();
				DestinationCanvas.Translate( 0, Range - (BarPosition + StartCapSize.y + EndCapSize.y) );
				FillVerticallyAndCapInverted( DestinationCanvas,BarPosition + StartCapSize.y + EndCapSize.y,
					mStyle->GetImage( UIProgressbarStyle::BarStartCap ), mStyle->GetImage( UIProgressbarStyle::BarBackground ), mStyle->GetImage( UIProgressbarStyle::BarEndCap ) );
				DestinationCanvas.PopState();

			}
			break;
		}
		default:
			assert( false );
	}
}

//----------------------------------------------------------------------

void UIProgressbar::SetStyle( UIProgressbarStyle *NewStyle )
{
	if( NewStyle )
		NewStyle->Attach( this );

	if( mStyle )
		mStyle->Detach( this );

	mStyle = NewStyle;
}

//----------------------------------------------------------------------

UIStyle *UIProgressbar::GetStyle( void ) const
{
	return mStyle;
};

void UIProgressbar::SetProgress( float In )
{
	mProgress = In;
}

float UIProgressbar::GetProgress( void ) const
{
	return mProgress;
}

//----------------------------------------------------------------------

void UIProgressbar::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::Style );

	UIWidget::GetLinkPropertyNames( In );}

//----------------------------------------------------------------------

void UIProgressbar::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIProgressbar::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back( PropertyName::Progress );
	In.push_back( PropertyName::Style );

	UIWidget::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UIProgressbar::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Progress )
	{
		if( UIUtils::ParseFloat( Value, mProgress ) )
		{
			if( mProgress < 0.0f )
				mProgress = 0.0f;
			else if( mProgress > 1.0f )
				mProgress = 1.0f;

			return true;
		}
		return false;
	}
	else if( Name == PropertyName::Style )
	{
		UIBaseObject *NewStyle = GetObjectFromPath( Value.c_str(), TUIProgressbarStyle );

		if( NewStyle || Value.empty() )
		{
			SetStyle( reinterpret_cast<UIProgressbarStyle *>( NewStyle ) );
			return true;
		}
	}

	return UIWidget::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UIProgressbar::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Progress )
		return UIUtils::FormatFloat( Value, mProgress );
	else if( Name == PropertyName::Style )
	{
		if( mStyle )
		{
			GetPathTo( Value, mStyle );
			return true;
		}
	}

	return UIWidget::GetProperty( Name, Value );
}

//----------------------------------------------------------------------
