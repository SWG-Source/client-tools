#include "_precompile.h"

#include "UICanvas.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIDropdownbox.h"
#include "UIDropdownboxStyle.h"
#include "UIEventCallback.h"
#include "UIImageStyle.h"
#include "UIListbox.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPropertyDescriptor.h"
#include "UIRenderHelper.h"
#include "UIScrollbar.h"
#include "UITemplate.h"
#include "UIUtils.h"

#include <vector>

const char *UIDropdownbox::TypeName													= "Dropdownbox";

//----------------------------------------------------------------------

const UILowerString UIDropdownbox::PropertyName::DataSource					= UILowerString ("DataSource");
const UILowerString UIDropdownbox::PropertyName::OnSelect						= UILowerString ("OnSelect");
const UILowerString UIDropdownbox::PropertyName::SelectedItem				= UILowerString ("SelectedItem");
const UILowerString UIDropdownbox::PropertyName::SelectedItemIndex	= UILowerString ("SelectedItemIndex");
const UILowerString UIDropdownbox::PropertyName::Style							= UILowerString ("Style");
const UILowerString UIDropdownbox::PropertyName::Template						= UILowerString ("RowTemplate");
const UILowerString UIDropdownbox::PropertyName::OpenSound					= UILowerString ("OpenSound");
const UILowerString UIDropdownbox::PropertyName::CloseSound					= UILowerString ("CloseSound");

//======================================================================================
#define _TYPENAME UIDropdownbox

namespace UIDropdownboxNamespace
{
	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(DataSource, "", T_object),
		_DESCRIPTOR(Style, "", T_object),
		_DESCRIPTOR(SelectedItemIndex, "", T_int),
		_DESCRIPTOR(Template, "", T_object),
	_GROUPEND(Basic, 2, 0);
	//================================================================

	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(OpenSound, "", T_string),
		_DESCRIPTOR(CloseSound, "", T_string),
	_GROUPEND(Appearance, 2, 1);
	//================================================================

	//================================================================
	// Behavior category
	_GROUPBEGIN(Behavior)
		_DESCRIPTOR(OnSelect, "", T_object),
	_GROUPEND(Behavior, 2, 2);
	//================================================================

}
using namespace UIDropdownboxNamespace;
//======================================================================================

typedef UIDropdownboxStyle::Image Image;

UIDropdownbox::UIDropdownbox()
{
	mDataSource = 0;
	mTemplate   = 0;
	mStyle			= 0;

	mListbox		= 0;
	mScrollbar  = 0;
	mSelection  = 0;
}

UIDropdownbox::~UIDropdownbox()
{
	SetDataSource(0);
	SetTemplate(0);
	SetStyle(0);

	if( mListbox )
		mListbox->Detach( this );

	if( mScrollbar )
		mScrollbar->Detach( this );
}

bool UIDropdownbox::IsA( const UITypeID Type ) const
{
	return (Type == TUIDropdownbox) || UIWidget::IsA( Type );
}

const char *UIDropdownbox::GetTypeName( void ) const
{
	return TypeName;
}

UIBaseObject *UIDropdownbox::Clone( void ) const
{
	return new UIDropdownbox;
}

void UIDropdownbox::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, UINotification::Code NotificationCode )
{
	UI_UNREF (NotificationCode);
	UI_UNREF (ContextObject);

	if( NotifyingObject == mDataSource )
	{
		mTemplateCache.Clear();
	}
	else if( NotifyingObject == mTemplate )
	{
		mTemplateCache.Clear();
	}
}

void UIDropdownbox::SetSelected( const bool b )
{
	UIWidget::SetSelected( b );

	if( !b && mListbox )
	{
		int OldSelection = mSelection;

		mSelection = mListbox->GetSelectionIndex();

		if( mSelection < 0 )
			mSelection = 0;

		if( mSelection != OldSelection )
			SendCallback( &UIEventCallback::OnDropdownboxSelectionChanged, PropertyName::OnSelect );

		DestroyPopup();
	}
}

bool UIDropdownbox::ProcessMessage( const UIMessage &msg )
{
	if( UIWidget::ProcessMessage( msg ) )
		return true;

	long OldSelection = mSelection;

	switch( msg.Type )
	{
		case UIMessage::KeyDown:
		case UIMessage::KeyRepeat:
			if( msg.Keystroke == UIMessage::Space )
				CreatePopup();
			else if( mListbox )
				mListbox->ProcessMessage( msg );
			else if( msg.Keystroke == UIMessage::UpArrow )
			{
				if( mSelection > 0 )
					--mSelection;
			}
			else if( msg.Keystroke == UIMessage::DownArrow )
			{
				if( mSelection < (long)(mDataSource->GetChildCount() - 1) )
					++mSelection;
			}
			break;

		case UIMessage::KeyUp:
			if( mListbox && (msg.Keystroke != UIMessage::Space) )
				mListbox->ProcessMessage( msg );

			break;

		case UIMessage::LeftMouseDown:
		case UIMessage::RightMouseDown:
		case UIMessage::MiddleMouseDown:
			CreatePopup();
			break;

		case UIMessage::LeftMouseUp:
		case UIMessage::RightMouseUp:
		case UIMessage::MiddleMouseUp:
			break;
	}

	if( mSelection != OldSelection )
		SendCallback( &UIEventCallback::OnDropdownboxSelectionChanged, PropertyName::OnSelect );

	return true;
}

void UIDropdownbox::Render( UICanvas &DestinationCanvas ) const
{
	if( !mStyle )
		return;

	UIImageStyle *StartCap;
	UIImageStyle *Background;
	UIImageStyle *DownArrow;

	if( mStyle )
	{
		if( IsEnabled() )
		{
			if( mListbox )
			{
				StartCap   = mStyle->GetImage( UIDropdownboxStyle::PressedStartCap );
				Background = mStyle->GetImage( UIDropdownboxStyle::PressedBackground );
				DownArrow	 = mStyle->GetImage( UIDropdownboxStyle::PressedDownArrow );
			}
			else if( IsSelected() )
			{
				StartCap   = mStyle->GetImage( UIDropdownboxStyle::SelectedStartCap );
				Background = mStyle->GetImage( UIDropdownboxStyle::SelectedBackground );
				DownArrow	 = mStyle->GetImage( UIDropdownboxStyle::SelectedDownArrow );
			}
			else
			{
				StartCap   = mStyle->GetImage( UIDropdownboxStyle::NormalStartCap );
				Background = mStyle->GetImage( UIDropdownboxStyle::NormalBackground );
				DownArrow	 = mStyle->GetImage( UIDropdownboxStyle::NormalDownArrow );
			}
		}
		else
		{
			StartCap   = mStyle->GetImage( UIDropdownboxStyle::DisabledStartCap );
			Background = mStyle->GetImage( UIDropdownboxStyle::DisabledBackground );
			DownArrow	 = mStyle->GetImage( UIDropdownboxStyle::DisabledDownArrow );
		}
	}
	else
	{
		StartCap   = 0;
		Background = 0;
		DownArrow	 = 0;
	}

	if( Background )
	{
		int CenterWidth	= Background->GetWidth();

		if( CenterWidth > 0 )
		{
			long PixelOffset = 0;

			while( PixelOffset < GetWidth() )
			{
				Background->Render( GetAnimationState(), DestinationCanvas, UIPoint(PixelOffset,0) );
				PixelOffset += CenterWidth;
			}
		}
	}

	if( StartCap )
		StartCap->Render( GetAnimationState(), DestinationCanvas, UIPoint(0,0) );

	if( DownArrow )
		DownArrow->Render( GetAnimationState(), DestinationCanvas, UIPoint(GetWidth() - DownArrow->GetWidth(),0) );

	if( !mDataSource || !mTemplate )
		return;

	UIDataList & CurrentData = mDataSource->GetData();

	DestinationCanvas.Flush();
	long rownum = 0;

	UIRect Padding;
	mStyle->GetPadding( Padding );

	for( UIDataList::iterator i = CurrentData.begin(); i != CurrentData.end(); ++i, ++rownum )
	{
		if( rownum == mSelection )
		{
			UIWidgetVector *TopLevelObjects = mTemplateCache.Get( (UIData *)*i );

			if( !TopLevelObjects )
			{
				const_cast<UIDropdownbox *>( this )->mTemplateCache.Add( *i, mTemplate, const_cast<UIDropdownbox *>( this ) );
				TopLevelObjects = mTemplateCache.Get( *i );
			}

			DestinationCanvas.PushState();
			DestinationCanvas.Translate( Padding.left, Padding.top );
			DestinationCanvas.Clip( UIRect( 0, 0, GetWidth() - Padding.right - Padding.left,
				GetHeight() - Padding.top - Padding.bottom ) );

			UIRenderHelper::RenderObjects( DestinationCanvas, *TopLevelObjects );

			DestinationCanvas.PopState();
			return;
		}
	}
}

void UIDropdownbox::MinimizeResources( void )
{
	UIWidget::MinimizeResources();
	mTemplateCache.Clear();
}

void UIDropdownbox::SetDataSource( UIDataSource *NewDataSource )
{
	if( NewDataSource )
		NewDataSource->Attach( this );

	// Template cache depends on the contents of mDataSource, so clear it first
	mTemplateCache.Clear();

	if( mDataSource )
		mDataSource->Detach( this );

	mDataSource = NewDataSource;
}

void UIDropdownbox::SetTemplate( UITemplate *NewTemplate )
{
	if( NewTemplate != mTemplate )
	{
		mTemplateCache.Clear();

		if( mTemplate )
		{
			mTemplate->StopListening( this );
			mTemplate->Detach( this );
		}

		mTemplate = NewTemplate;

		if( mTemplate )
		{
			mTemplate->Listen( this );
			mTemplate->Attach( this );
		}
	}
}

void UIDropdownbox::SetStyle( UIDropdownboxStyle *NewStyle )
{
	if( NewStyle )
		NewStyle->Attach( this );

	if( mStyle )
		mStyle->Detach( this );

	mStyle = NewStyle;
}

UIStyle *UIDropdownbox::GetStyle( void ) const
{
	return mStyle;
};

void UIDropdownbox::CreatePopup( void )
{
	if( mListbox )
		return;

	UINarrowString SoundToPlay;

	if( !GetPropertyNarrow ( UIDropdownbox::PropertyName::OpenSound, SoundToPlay ) && mStyle )
		mStyle->GetPropertyNarrow ( UIDropdownboxStyle::PropertyName::OpenSound, SoundToPlay );

	if( !SoundToPlay.empty() )
		UIManager::gUIManager().PlaySound( SoundToPlay.c_str() );

	UIRect WorldRect;
	GetWorldRect(WorldRect);

	mListbox = new UIListbox;

	mListbox->Attach( this );
	mListbox->SetParent( this );
	mListbox->SetStyle( mStyle->GetListboxStyle() );
	mListbox->SetDataSource( mDataSource );
	mListbox->SetTemplate( mTemplate );

	UIScalar MaxDropBy = mStyle->GetDropBy();
	UIScalar DataSize  = INT_MAX;
	UIScalar DropBy;

	if( mTemplate && mDataSource )
	{
		DataSize = DropBy = mTemplate->GetHeight() * mDataSource->GetChildCount();

		if( DropBy > MaxDropBy )
			DropBy = MaxDropBy;
	}
	else
	{
		DropBy = MaxDropBy;
	}

//	UIScrollbarStyle *ScrollbarStyle = mStyle->GetScrollbarStyle();
	UIScalar          ScrollbarWidth = 0;

//	if( ScrollbarStyle && (DataSize > DropBy) )
//		ScrollbarWidth = ScrollbarStyle->GetThickness();
//	else
//		ScrollbarWidth = 0;

	mListbox->SetSize( UISize( GetWidth() - ScrollbarWidth, DropBy ) );

	mListbox->SetLocation( WorldRect.left, WorldRect.bottom );
	mListbox->SetVisible( true );
	mListbox->SetTransient( true );

	// Set the selection and scroll to it.  This has to happen after the SetSize
	// call so that the listbox can prevent being scrolled too far down.
	mListbox->SetSelectionIndex( mSelection );
	mListbox->ScrollTo( mSelection );

	UIPage *TopLevelPage = reinterpret_cast<UIPage *>( GetRoot() );
	TopLevelPage->AddOverlay( mListbox );

	if( ScrollbarWidth )
	{
		mScrollbar = new UIScrollbar;

		mScrollbar->Attach( this );
		mScrollbar->SetParent( this );

		//@todo fix this to use UISliderbarStyle instead
//		mScrollbar->SetStyle( ScrollbarStyle );
		mScrollbar->AttachToControl( mListbox );

		mScrollbar->SetSize( UISize( ScrollbarWidth, DropBy ) );
		mScrollbar->SetLocation( WorldRect.right - mScrollbar->GetWidth(), WorldRect.bottom );
		mScrollbar->SetVisible( true );
		mScrollbar->SetTransient( true );

		TopLevelPage->AddOverlay( mScrollbar );
	}
}

void UIDropdownbox::DestroyPopup( void )
{
	UIPage *TopLevelPage = reinterpret_cast<UIPage *>( GetRoot() );

	if( mListbox )
	{
		UINarrowString SoundToPlay;

		if( !GetPropertyNarrow ( UIDropdownbox::PropertyName::CloseSound, SoundToPlay ) && mStyle )
			mStyle->GetPropertyNarrow ( UIDropdownboxStyle::PropertyName::CloseSound, SoundToPlay );

		if( !SoundToPlay.empty() )
			UIManager::gUIManager().PlaySound( SoundToPlay.c_str() );

		TopLevelPage->RemoveOverlay( mListbox );
		mListbox->Detach( this );
		mListbox = 0;
	}

	if( mScrollbar )
	{
		TopLevelPage->RemoveOverlay( mScrollbar );
		mScrollbar->Detach( this );
		mScrollbar = 0;
	}
}

void UIDropdownbox::ProcessChildNotificationMessage( UIWidget *w, const UIMessage &msg )
{
	if( w->IsA( TUIListbox ) )
	{
		UIListbox *theListbox		= reinterpret_cast<UIListbox *>( w );
		long			 OldSelection = mSelection;

		switch( msg.Type )
		{
			case UIMessage::LeftMouseUp:
			case UIMessage::RightMouseUp:
			case UIMessage::MiddleMouseUp:

				mSelection = theListbox->GetSelectionIndex();

				if( mSelection < 0 )
					mSelection = 0;

				DestroyPopup();
				break;
		}

		if( mSelection != OldSelection )
			SendCallback( &UIEventCallback::OnDropdownboxSelectionChanged, PropertyName::OnSelect );
	}
}

//----------------------------------------------------------------------

void UIDropdownbox::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	GetPropertyNames (In, false);
}

//----------------------------------------------------------------------

void UIDropdownbox::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
	GROUP_TEST_AND_PUSH(Behavior, category, o_groups);
}

//----------------------------------------------------------------------

void UIDropdownbox::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	// PropertyName::SelectedItem is a special property
	// and is not returned by this function

	In.push_back( PropertyName::DataSource					);
	In.push_back( PropertyName::OnSelect						);
	In.push_back( PropertyName::SelectedItemIndex	);
	In.push_back( PropertyName::Style							);
	In.push_back( PropertyName::Template						);
	In.push_back( PropertyName::OpenSound					);
	In.push_back( PropertyName::CloseSound					);

	UIWidget::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UIDropdownbox::SetProperty( const UILowerString & Name, const UIString &Value )
{
	const char * const NamePtr = Name.c_str ();

	const char *pSeparator = strchr( NamePtr, '.' );

	if( pSeparator )
	{
		int len = pSeparator - NamePtr;

		if( !_strnicmp( NamePtr, PropertyName::SelectedItem.c_str (), len ) )
		{
			if( mDataSource )
			{
				UIData *SelectedDataObject = mDataSource->GetChildByPosition( mSelection );

				if( SelectedDataObject )
					return SelectedDataObject->SetProperty( UILowerString (pSeparator + 1), Value );
			}
			return false;
		}
	}

	if( Name == PropertyName::DataSource )
	{
		UIBaseObject *NewDataSource = GetObjectFromPath( Value, TUIDataSource );

		if( NewDataSource || Value.empty() )
		{
			SetDataSource( reinterpret_cast<UIDataSource *>( NewDataSource ) );
			return true;
		}
	}
	else if( Name == PropertyName::Style )
	{
		UIBaseObject *NewStyle = GetObjectFromPath( Value, TUIDropdownboxStyle );

		if( NewStyle || Value.empty() )
		{
			SetStyle( reinterpret_cast<UIDropdownboxStyle *>( NewStyle ) );
			return true;
		}
	}
	else if( Name == PropertyName::SelectedItemIndex )
	{
		return UIUtils::ParseLong( Value, mSelection );
	}
	else if( Name == PropertyName::Template )
	{
		UIBaseObject *NewTemplate = GetObjectFromPath( Value, TUITemplate );

		if( NewTemplate || Value.empty() )
		{
			SetTemplate( reinterpret_cast<UITemplate *>( NewTemplate ) );
			return true;
		}
	}

	return UIWidget::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UIDropdownbox::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	const char * const NamePtr = Name.c_str ();
	const char *pSeparator = strchr( NamePtr, '.' );

	if( pSeparator )
	{
		int len = pSeparator - NamePtr;

		if( !_strnicmp( NamePtr, PropertyName::SelectedItem.c_str (), len ) )
		{
			if( mDataSource )
			{
				UIData *SelectedDataObject = mDataSource->GetChildByPosition( mSelection );

				if( SelectedDataObject )
					return SelectedDataObject->GetProperty( UILowerString (pSeparator + 1), Value );
			}
			return false;
		}
	}

	if( Name == PropertyName::DataSource )
	{
		if( mDataSource )
		{
			GetPathTo( Value, mDataSource );
			return true;
		}
	}
	else if( Name == PropertyName::SelectedItemIndex )
	{
		return UIUtils::FormatLong( Value, mSelection );
	}
	else if( Name == PropertyName::Style )
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

void UIDropdownbox::SetSelectedItemIndex( long NewSelection )
{
	mSelection = NewSelection;
}

UIData *UIDropdownbox::GetSelectedItem( void ) const
{
	if( !mDataSource )
		return 0;

	return mDataSource->GetChildByPosition( mSelection );
}