#include "_precompile.h"

#include "UICanvas.h"
#include "UIImage.h"
#include "UIImageFrame.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIOutputStream.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <vector>

const char *UIImage::TypeName                       = "Image";

//----------------------------------------------------------------------

const UILowerString UIImage::PropertyName::SourceRect       = UILowerString ("SourceRect");
const UILowerString UIImage::PropertyName::SourceResource   = UILowerString ("SourceResource");
const UILowerString UIImage::PropertyName::Stretch          = UILowerString ("Stretch");
const UILowerString UIImage::PropertyName::Style            = UILowerString ("Style");

//======================================================================================
#define _TYPENAME UIImage

namespace UIImageNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(SourceRect, "", T_rect),
		_DESCRIPTOR(SourceResource, "", T_object),
		_DESCRIPTOR(Stretch, "", T_bool),
		_DESCRIPTOR(Style, "", T_object),
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UIImageNamespace;
//======================================================================================

UIImage::UIImage () :
UIWidget         (),
mStyle           (0),
mSourceCanvas    (0),
mStretch         (true),
mSourceRectSet   (false),
mCanvasRectDirty (false)
{
	SetGetsInput (false);
}

//----------------------------------------------------------------------

UIImage::~UIImage()
{
	SetStyle(0);
	SetCanvas(0);
}

//----------------------------------------------------------------------

bool UIImage::IsA( const UITypeID QueriedType ) const
{
	return QueriedType == TUIImage || UIWidget::IsA( QueriedType );
}

//----------------------------------------------------------------------

const char *UIImage::GetTypeName( void ) const
{
	return TypeName;
}

//----------------------------------------------------------------------

UIBaseObject *UIImage::Clone( void ) const
{
	return new UIImage;
}

//----------------------------------------------------------------------

bool UIImage::ProcessMessage( const UIMessage &msg )
{
	return UIWidget::ProcessMessage( msg );
}

//-----------------------------------------------------------------

void UIImage::Render( UICanvas &DestinationCanvas ) const
{
	UIWidget::Render (DestinationCanvas);

	if( !mStyle && !mSourceCanvas )
		return;

	const long animationState = GetAnimationState ();

	const UIImageFragment * SourceFragment = 0;
	UIImageFrame * SourceFrame = 0;

	if (mStyle)
	{
		if (!mStyle->GetFrame (animationState, SourceFrame ))
		{
			SourceFragment = mStyle->GetFragmentSelf ();
		}
		else
			SourceFragment = SourceFrame;

		if (SourceFragment && !SourceFragment->IsReadyToRender ())
		{
			SourceFragment = 0;
			SourceFrame    = 0;
		}
	}

	if (mCanvasRectDirty && !mSourceRectSet && !SourceFragment)
		AutoSetCanvasRect ();
	
	const UISize ImageSize (SourceFragment ? SourceFragment->GetSize() : mSourceSize );

	const UISize mySize (GetSize ());

	if (mySize.x == 0 || mySize.y == 0 || ImageSize.x == 0 || ImageSize.y == 0)
		return;

	UIFloatPoint srcScale  (1.0f, 1.0f);
	bool isFiltered = false;

	if( mStretch && (ImageSize != GetSize()) )
	{
		if (mySize.x != ImageSize.x)
		{
			srcScale.x = static_cast<float>(ImageSize.x) / static_cast<float>(GetWidth());
		}

		if (mySize.y != ImageSize.y)
		{
			srcScale.y = static_cast<float>(ImageSize.y) / static_cast<float>(GetHeight());
		}

		isFiltered = true;
		DestinationCanvas.EnableFiltering( true );
	}

	DestinationCanvas.SetSourceScale (srcScale.x, srcScale.y);

	if( mStyle )
	{
		if (SourceFrame)
			mStyle->Render(*SourceFrame, DestinationCanvas, UIPoint::zero, GetSize() );
		else
			mStyle->Render(animationState, DestinationCanvas, UIPoint::zero, GetSize() );
	}
	else if( mSourceCanvas )
		DestinationCanvas.BltFrom( mSourceCanvas, mSourcePoint, UIPoint::zero, GetSize() );

	if (isFiltered)
	{
		DestinationCanvas.EnableFiltering( false );
		DestinationCanvas.SetSourceScale (1.0, 1.0);
	}
}

//----------------------------------------------------------------------

void UIImage::SetSourceRect( const UIRect *NewRect )
{
      if( NewRect )
      {
              mSourceRectSet = true;

              mSourcePoint.x = NewRect->left;
              mSourcePoint.y = NewRect->top;

              mSourceSize.x = NewRect->Width();
              mSourceSize.y = NewRect->Height();
      }
      else
      {
              mSourceRectSet = false;
      }
}

//----------------------------------------------------------------------

void UIImage::SetStyle( UIImageStyle *NewStyle )
{
	RemoveProperty (PropertyName::Style);

	if(NewStyle != mStyle)
	{
		if( NewStyle )
			NewStyle->Attach( this );
		
		if( mStyle )
			mStyle->Detach( this );
		
		mStyle = NewStyle;
	}
}

//----------------------------------------------------------------------

UIStyle *UIImage::GetStyle( void ) const
{
	return mStyle;
};

//----------------------------------------------------------------------

void UIImage::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::SourceResource );
	In.push_back( PropertyName::Style );

	UIWidget::GetLinkPropertyNames( In );

}

//----------------------------------------------------------------------

void UIImage::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIImage::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back( PropertyName::SourceRect );
	In.push_back( PropertyName::SourceResource );
	In.push_back( PropertyName::Stretch );
	In.push_back( PropertyName::Style );

	UIWidget::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

void UIImage::GetPropertiesInCategory (UIPropertyCategories::Category category, UIPropertyNameVector & In) const
{
	if(category == UIPropertyCategories::C_Appearance)
	{						
		In.push_back( PropertyName::SourceRect );
		In.push_back( PropertyName::SourceResource );
		In.push_back( PropertyName::Stretch );
		In.push_back( PropertyName::Style );	
	}

	UIWidget::GetPropertiesInCategory(category, In);
}

//----------------------------------------------------------------------

bool UIImage::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::SourceRect )
	{
		UIRect SourceRect;

		if( Value.empty() )
		{
			mSourceRectSet = false;
			SetCanvas( mSourceCanvas );
			return true;
		}

		if( !UIUtils::ParseRect( Value, SourceRect ) )
			return false;

		mSourceRectSet = true;

		mSourcePoint.x = SourceRect.left;
		mSourcePoint.y = SourceRect.top;
		mSourceSize.x	 = SourceRect.right - SourceRect.left;
		mSourceSize.y	 = SourceRect.bottom - SourceRect.top;
		return true;
	}
	else if( Name == PropertyName::SourceResource )
	{
		if (SetSourceResource( Value ))
			return true;
	}
	else if( Name == PropertyName::Stretch )
	{
		bool stretch = false;

		if (!UIUtils::ParseBoolean(Value, stretch))
			return false;

		mStretch = stretch;
		return true;
	}
	else if( Name == PropertyName::Style )
	{
		UIImageStyle * const NewStyle = static_cast<UIImageStyle *> (GetObjectFromPath( Value.c_str(), TUIImageStyle ));

		if( NewStyle || Value.empty() )
		{
			SetStyle (NewStyle);
			return true;
		}
	}

	return UIWidget::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UIImage::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::SourceRect )
	{
		if( mSourceRectSet )
			UIUtils::FormatRect( Value, UIRect( mSourcePoint.x, mSourcePoint.y,
				mSourcePoint.x + mSourceSize.x, mSourcePoint.y + mSourceSize.y ) );
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
	else if( Name == PropertyName::Stretch )
	{
		return UIUtils::FormatBoolean( Value, mStretch );
	}
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

void UIImage::SetCanvas( UICanvas *NewCanvas )
{
	if (NewCanvas != mSourceCanvas)
	{
		if( NewCanvas )
			NewCanvas->Attach( this );
		
		if( mSourceCanvas )
			mSourceCanvas->Detach( this );
		
		mSourceCanvas = NewCanvas;
		
		mCanvasRectDirty = true;
	}
}

//----------------------------------------------------------------------

void UIImage::AutoSetCanvasRect () const
{
	mCanvasRectDirty = false;
	mSourcePoint.x = 0;
	mSourcePoint.y = 0;
	mSourceSize.x	 = mSourceCanvas->GetWidth();
	mSourceSize.y	 = mSourceCanvas->GetHeight();
	mSourceRectSet = true;
}

//----------------------------------------------------------------------

bool UIImage::SetSourceResource( const UIString &NewResourceName )
{
	if( NewResourceName.empty() )
	{
		RemoveProperty(PropertyName::SourceResource);
		RemoveProperty(PropertyName::SourceRect);
		SetCanvas(0);
	}
	else
	{

		UICanvas * const theCanvas = UIManager::isUIReady() ? UIManager::gUIManager().GetCanvas( NewResourceName.c_str() ) : NULL;
		SetCanvas (theCanvas);

		if (!theCanvas)
			return false;
	}

	return true;
}

//-----------------------------------------------------------------
