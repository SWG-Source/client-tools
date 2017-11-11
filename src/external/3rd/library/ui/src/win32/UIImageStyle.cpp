#include "_precompile.h"
#include "UIImageStyle.h"

#include "UICanvas.h"
#include "UIImageFrame.h"
#include "UINamespace.h"
#include "UIPage.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <cmath>
#include <cassert>
#include <vector>
#include <algorithm>

//----------------------------------------------------------------------

const char * const UIImageStyle::TypeName                            = "ImageStyle";
const UILowerString UIImageStyle::PropertyName::Loop                 = UILowerString ("Loop");
const UILowerString UIImageStyle::PropertyName::Reversing            = UILowerString ("Reversing");
const UILowerString UIImageStyle::PropertyName::PlayTime             = UILowerString ("Playtime");
const UILowerString UIImageStyle::PropertyName::OpacityRelativeMin   = UILowerString ("OpacityRelativeMin");

//======================================================================================
#define _TYPENAME UIImageStyle

namespace UIImageStyleNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Loop, "", T_bool),
		_DESCRIPTOR(Reversing, "", T_bool),
		_DESCRIPTOR(PlayTime, "", T_float),
		_DESCRIPTOR(OpacityRelativeMin, "", T_float)
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UIImageStyleNamespace;
//======================================================================================

//----------------------------------------------------------------------
// UIImageStyle
//----------------------------------------------------------------------

UIImageStyle::UIImageStyle() :
UIStyle             (),
mPlayTime           (1.0f),
mFrames             (0),
mFragmentSelf       (new UIImageFragment),
mOpacityRelativeMin (0.0f),
mReversing          (false),
mLooping            (true)
{
	mFragmentSelf->Attach (this);
}

//-----------------------------------------------------------------

UIImageStyle::~UIImageStyle()
{
	mFragmentSelf->Detach (this);
	mFragmentSelf = 0;

	if (mFrames)
	{
		std::for_each (mFrames->begin (), mFrames->end (), DetachFunctor (this));
		delete mFrames;
		mFrames = 0;
	}
}

//-----------------------------------------------------------------

bool UIImageStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUIImageStyle) || UIStyle::IsA( Type );
}

//-----------------------------------------------------------------

const char *UIImageStyle::GetTypeName( void ) const
{
	return TypeName;
}

//-----------------------------------------------------------------

UIBaseObject *UIImageStyle::Clone( void ) const
{
	return new UIImageStyle;
}

//-----------------------------------------------------------------

bool UIImageStyle::AddChild( UIBaseObject *ChildToAdd )
{
	if( ChildToAdd && ChildToAdd->IsA( TUIImageFrame ) )
	{
		if (!mFrames)
			mFrames = new UIImageFrameList;

		mFrames->push_back( reinterpret_cast<UIImageFrame *>( ChildToAdd ) );

		ChildToAdd->SetParent( this );
		ChildToAdd->Attach( this );
		return true;
	}
	return false;
}

//-----------------------------------------------------------------

bool UIImageStyle::RemoveChild( UIBaseObject *ChildToRemove )
{
	if (!mFrames)
		return false;

	for( UIImageFrameList::iterator i = mFrames->begin(); i != mFrames->end(); ++i )
	{
		if( (*i) == ChildToRemove )
		{
			mFrames->erase(i);
			ChildToRemove->Detach( this );

			if (mFrames->empty ())
			{
				delete mFrames;
				mFrames = 0;
			}

			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------

UIBaseObject *UIImageStyle::GetChild( const char *ChildName ) const
{
	if (!mFrames)
		return 0;

	for( UIImageFrameList::const_iterator i = mFrames->begin(); i != mFrames->end(); ++i )
	{
		if( (*i)->IsName( ChildName ) )
			return (*i);
	}
	return 0;
}

//-----------------------------------------------------------------

bool UIImageStyle::CanChildMove( UIBaseObject *ObjectToMove, ChildMovementDirection MoveDirection )
{
	if( !mFrames || !ObjectToMove )
		return false;

	for( UIImageFrameList::const_iterator i = mFrames->begin(); i != mFrames->end(); ++i )
	{
		if( *i == ObjectToMove )
		{
			if( i == mFrames->begin() )
			{
				if( (MoveDirection == Up) || (MoveDirection == Top) )
					return false;
			}
			else if( ++i == mFrames->end() )
			{
				if( (MoveDirection == Down) || (MoveDirection == Bottom) )
					return false;
			}
			return true;
		}			
	}
	return false;
}

//-----------------------------------------------------------------

bool UIImageStyle::MoveChild( UIBaseObject *ObjectToMove, ChildMovementDirection MoveDirection )
{
	if( !mFrames || !ObjectToMove )
		return false;

	for( UIImageFrameList::iterator i = mFrames->begin(); i != mFrames->end(); ++i )
	{
		if( *i == ObjectToMove )
		{
			if( (MoveDirection == Up) )
			{
				if( i != mFrames->begin() )
				{
					mFrames->insert( --i, 1, *i );
					mFrames->erase( ++i );
					return true;
				}
			}
			else if( (MoveDirection == Top) )
			{
				if( i != mFrames->begin() )
				{
					mFrames->insert( mFrames->begin(), 1, *i );
					mFrames->erase(i);
					return true;
				}
			}
			else if( (MoveDirection == Down) )
			{
				if( i != mFrames->end() )
				{
					UIImageFrameList::iterator InsertPoint = i;

					++InsertPoint;

					if( InsertPoint != mFrames->end() )
						++InsertPoint;

					mFrames->insert( InsertPoint, 1, *i );
					mFrames->erase( i );
					return true;
				}
			}
			else if( (MoveDirection == Bottom) )
			{
				if( i != mFrames->end() )
				{
					mFrames->insert( mFrames->end(), 1, *i );
					mFrames->erase( i );
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

//-----------------------------------------------------------------

bool UIImageStyle::GetFrame( UITime DesiredTime, UIImageFrame *&Out ) const
{
	if (!mFrames)
		return false;

	if( mFrames->size() <= 1 )
	{
		if( mFrames->empty() )
			return false;

		Out = mFrames->front();
		return true;
	}

	float	DesiredTimeInSeconds = DesiredTime / 60.0f;
	
	if( DesiredTimeInSeconds >= mPlayTime )
	{
		if (mReversing)
		{
			DesiredTimeInSeconds = (float)fmod( DesiredTimeInSeconds, mPlayTime );
		}
		else if( mLooping )
		{
			DesiredTimeInSeconds = (float)fmod( DesiredTimeInSeconds, mPlayTime );
		}
		else
		{
			Out = mFrames->back();
			return true;
		}
	}

	UITime TotalTime = 0;

	{
		for( UIImageFrameList::const_iterator i = mFrames->begin(); i != mFrames->end(); ++i )
			TotalTime += (*i)->GetDuration();
	}

	UITime DesiredTimeInTicks = 0;
	

	const size_t frameCount = mFrames->size ();

	if (mReversing && frameCount > 2)
	{
		const float lengthModifier = 1.0f + static_cast<float>(frameCount - 2) / frameCount;

		DesiredTimeInTicks = static_cast<UITime>((DesiredTimeInSeconds * lengthModifier / mPlayTime) * static_cast<float>(TotalTime));
		if (DesiredTimeInTicks > TotalTime)
			DesiredTimeInTicks = static_cast<UITime>(lengthModifier * TotalTime - DesiredTimeInTicks);
	}
	else if (mLooping)
	{
		DesiredTimeInTicks = static_cast<UITime>((DesiredTimeInSeconds / mPlayTime) * static_cast<float>(TotalTime));
	}

	TotalTime = 0;

	{
		for( UIImageFrameList::const_iterator i = mFrames->begin(); i != mFrames->end(); ++i )
		{
			TotalTime += (*i)->GetDuration();

			if( TotalTime > DesiredTimeInTicks )
			{
				Out = *i;
				return true;
			}
		}	
	}
	return false;
}

//-----------------------------------------------------------------

void UIImageStyle::GetChildren( UIObjectList &ol ) const
{
	if (mFrames)
	{
		for( UIImageFrameList::const_iterator i = mFrames->begin(); i != mFrames->end(); ++i )
			ol.push_back( *i );
	}
}

//-----------------------------------------------------------------

unsigned long UIImageStyle::GetChildCount( void ) const
{	
	return mFrames ? mFrames->size() : 0;
}
 
//-----------------------------------------------------------------

void UIImageStyle::Render( UITime DesiredTime, UICanvas &DestinationCanvas, const UIPoint &Location) const
{
	if (mFrames)
	{
		UIImageFrame *SourceFrame = 0;
		if (GetFrame( DesiredTime, SourceFrame ) && SourceFrame)
			Render (*SourceFrame, DestinationCanvas, Location);

		return;
	}

	if (mFragmentSelf && mFragmentSelf->IsReadyToRender ())
	{
		const float oldOpacity    = DestinationCanvas.GetOpacity ();
		const float actualOpacity = UIWidget::ComputeRelativeOpacity (oldOpacity, mOpacityRelativeMin);
		DestinationCanvas.SetOpacity (actualOpacity);

		if( Location != UIPoint::zero)
		{
			DestinationCanvas.PushState();
			DestinationCanvas.Translate(Location);
			mFragmentSelf->Render( DestinationCanvas );
			DestinationCanvas.PopState();
		}
		else
			mFragmentSelf->Render( DestinationCanvas );

		DestinationCanvas.SetOpacity (oldOpacity);
	}
}

//-----------------------------------------------------------------

void UIImageStyle::Render( UITime DesiredTime, UICanvas &DestinationCanvas, const UIPoint &Location, const UISize & size ) const
{
	if (mFrames)
	{
		UIImageFrame *SourceFrame = 0;
		if (GetFrame( DesiredTime, SourceFrame ) && SourceFrame)
			Render (*SourceFrame, DestinationCanvas, Location, size);

		return;
	}

	if (mFragmentSelf && mFragmentSelf->IsReadyToRender ())
	{
		const float oldOpacity    = DestinationCanvas.GetOpacity ();
		const float actualOpacity = UIWidget::ComputeRelativeOpacity (oldOpacity, mOpacityRelativeMin);
		DestinationCanvas.SetOpacity (actualOpacity);
		
		if( Location != UIPoint::zero)
		{
			DestinationCanvas.PushState();
			DestinationCanvas.Translate(Location);
			mFragmentSelf->Render (DestinationCanvas, size);
			DestinationCanvas.PopState();
		}
		else
			mFragmentSelf->Render (DestinationCanvas, size);

		DestinationCanvas.SetOpacity (oldOpacity);
	}
}

//-----------------------------------------------------------------

void UIImageStyle::Render( UIImageFrame & SourceFrame, UICanvas &DestinationCanvas, const UIPoint &Location) const
{
	const float oldOpacity    = DestinationCanvas.GetOpacity ();
	const float actualOpacity = UIWidget::ComputeRelativeOpacity (oldOpacity, mOpacityRelativeMin);
	DestinationCanvas.SetOpacity (actualOpacity);

	if (Location != UIPoint::zero)
	{
		DestinationCanvas.PushState();
		DestinationCanvas.Translate(Location);
		SourceFrame.Render( DestinationCanvas );
		DestinationCanvas.PopState();
	}
	else
		SourceFrame.Render( DestinationCanvas );

	DestinationCanvas.SetOpacity (oldOpacity);
}

//-----------------------------------------------------------------

void UIImageStyle::Render( UIImageFrame & SourceFrame, UICanvas &DestinationCanvas, const UIPoint &Location, const UISize & size ) const
{
	const float oldOpacity    = DestinationCanvas.GetOpacity ();
	const float actualOpacity = UIWidget::ComputeRelativeOpacity (oldOpacity, mOpacityRelativeMin);
	DestinationCanvas.SetOpacity (actualOpacity);

	if (Location != UIPoint::zero)
	{
		DestinationCanvas.PushState();
		DestinationCanvas.Translate(Location);
		SourceFrame.Render( DestinationCanvas, size );
		DestinationCanvas.PopState();
	}
	else
		SourceFrame.Render( DestinationCanvas, size );

	DestinationCanvas.SetOpacity (oldOpacity);
}

//-----------------------------------------------------------------

const UISize & UIImageStyle::GetSize () const
{
	if (mFragmentSelf && mFragmentSelf->IsReadyToRender ())
		return mFragmentSelf->GetSize ();

	if (mFrames && ! mFrames->empty() )
		return mFrames->front()->GetSize();

	return UISize::zero;
}

//-----------------------------------------------------------------

long UIImageStyle::GetWidth () const
{
	if (mFragmentSelf && mFragmentSelf->IsReadyToRender ())
		return mFragmentSelf->GetWidth ();

	if (mFrames && !mFrames->empty() )
		return mFrames->front()->GetWidth();

	return 0;
}

//-----------------------------------------------------------------

long UIImageStyle::GetHeight () const
{
	if (mFragmentSelf && mFragmentSelf->IsReadyToRender ())
		return mFragmentSelf->GetHeight ();

	if (mFrames && !mFrames->empty() )
		return mFrames->front()->GetHeight();

	return 0;
}

//----------------------------------------------------------------------

void UIImageStyle::CopyPropertiesFrom    (const UIBaseObject & rhs)
{
	if (rhs.IsA(TUIImageStyle))
	{
		UIImageStyle const & rhs_imageStyle = static_cast<UIImageStyle const &>(rhs);
		
		mLooping = rhs_imageStyle.mLooping;
		mReversing = rhs_imageStyle.mReversing;
		mPlayTime = rhs_imageStyle.mPlayTime;
		mOpacityRelativeMin = rhs_imageStyle.mOpacityRelativeMin;
	}

	UIStyle::CopyPropertiesFrom(rhs);
}

//-----------------------------------------------------------------

void UIImageStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	UIImageFragment::GetStaticPropertyGroups(o_groups, category);
}

//-----------------------------------------------------------------

void UIImageStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy) const
{
	if (!forCopy)
	{
		In.push_back( PropertyName::Loop );
		In.push_back( PropertyName::Reversing );
		In.push_back( PropertyName::PlayTime );
		In.push_back( PropertyName::OpacityRelativeMin );	                    
	}

	if (mFragmentSelf)
		mFragmentSelf->GetPropertyNames (In, forCopy);
	
	UIStyle::GetPropertyNames( In, forCopy );
}

//-----------------------------------------------------------------

bool UIImageStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Loop )
	{
		bool looping = true;

		if (!UIUtils::ParseBoolean(Value, looping))
			return false;

		mLooping = looping;
		return true;
	}
	else if( Name == PropertyName::Reversing )
	{
		bool reversing = true;
		
		if (!UIUtils::ParseBoolean(Value, reversing))
			return false;
		
		mReversing = reversing;
		return true;
	}
	else if( Name == PropertyName::PlayTime )
		return UIUtils::ParseFloat( Value, mPlayTime );	
	else if (Name == PropertyName::OpacityRelativeMin)
		return UIUtils::ParseFloat( Value, mOpacityRelativeMin );	
	else if (Name == UIImageFragment::PropertyName::Offset ||
			Name == UIImageFragment::PropertyName::OffsetProportional ||
			Name == UIImageFragment::PropertyName::SourceRect ||
			Name == UIImageFragment::PropertyName::SourceResource)
	{
		if (mFragmentSelf)
			return mFragmentSelf->SetProperty (Name, Value);
	}

	return UIStyle::SetProperty( Name, Value );
}

//-----------------------------------------------------------------

bool UIImageStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Loop )
		return UIUtils::FormatBoolean( Value, mLooping );
	else if( Name == PropertyName::Reversing )
		return UIUtils::FormatBoolean( Value, mReversing );
	else if( Name == PropertyName::PlayTime )
		return UIUtils::FormatFloat( Value, mPlayTime );
	else if (Name == PropertyName::OpacityRelativeMin)
		return UIUtils::FormatFloat( Value, mOpacityRelativeMin );	
	else if (Name == UIImageFragment::PropertyName::Offset ||
			Name == UIImageFragment::PropertyName::OffsetProportional ||
			Name == UIImageFragment::PropertyName::SourceRect ||
			Name == UIImageFragment::PropertyName::SourceResource)
	{
		if (mFragmentSelf)
			return mFragmentSelf->GetProperty (Name, Value);
	}
	return UIStyle::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

const UICanvas  * UIImageStyle::GetSourceCanvas  () const
{
	if (mFragmentSelf && mFragmentSelf->IsReadyToRender ())
		return mFragmentSelf->GetCanvas ();

	if (mFrames && !mFrames->empty ())
		return mFrames->front ()->GetCanvas ();

	return 0;
}

//----------------------------------------------------------------------

const UIRect UIImageStyle::GetSourceRect    () const
{
	if (mFragmentSelf && mFragmentSelf->IsReadyToRender ())
		return mFragmentSelf->GetSourceRect ();

	if (mFrames && !mFrames->empty() )
		return mFrames->front()->GetSourceRect ();

	return UIRect::zero;
}

//-----------------------------------------------------------------
