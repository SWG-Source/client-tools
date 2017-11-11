#include "_precompile.h"

#include "UICanvas.h"
#include "UIClock.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIEventCallback.h"
#include "UIGridStyle.h"
#include "UIImageStyle.h"
#include "UIListbox.h"
#include "UIListboxStyle.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPropertyDescriptor.h"
#include "UIRenderHelper.h"
#include "UITemplate.h"
#include "UITooltipStyle.h"
#include "UIUtils.h"
#include <cmath>
#include <vector>

const char *UIListbox::TypeName													= "Listbox";

const UILowerString UIListbox::PropertyName::DataSource					= UILowerString ("DataSource");
const UILowerString UIListbox::PropertyName::OnSelect						= UILowerString ("OnSelect");
const UILowerString UIListbox::PropertyName::OnDoubleClick						= UILowerString ("OnDoubleClick");
const UILowerString UIListbox::PropertyName::SelectedItem				= UILowerString ("SelectedItem");
const UILowerString UIListbox::PropertyName::SelectedItemIndex	= UILowerString ("SelectedItemIndex");
const UILowerString UIListbox::PropertyName::Style							= UILowerString ("Style");
const UILowerString UIListbox::PropertyName::Template						= UILowerString ("RowTemplate");
const UILowerString UIListbox::PropertyName::AutoScrollSpeed		= UILowerString ("AutoScrollSpeed");
//======================================================================================
#define _TYPENAME UIListbox

namespace UIListboxNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(AutoScrollSpeed, "", T_float),
		_DESCRIPTOR(DataSource, "", T_object),
		_DESCRIPTOR(SelectedItemIndex, "", T_int),
		_DESCRIPTOR(Style, "", T_object),
		_DESCRIPTOR(Template, "", T_object),
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================
	//================================================================
	// AdvancedBehavior category.
	_GROUPBEGIN(AdvancedBehavior)
		_DESCRIPTOR(OnSelect, "", T_string),
		_DESCRIPTOR(OnDoubleClick, "", T_string),
	_GROUPEND(AdvancedBehavior, 2, int(UIPropertyCategories::C_AdvancedBehavior));
	//================================================================
}
using namespace UIListboxNamespace;
//======================================================================================

UIListbox::UIListbox() :
UIWidget (),
mMouseDown (false),
mSelectedItemsAllocatedSize(0),
mSelectedItems(0)
{
	mTemplate				    =  0;
	mDataSource			    =  0;
	mStyle					    =  0;
	mSelection			    = -1;
	mSelectedDataObject =  0;
	mAutoScroll			    =  0;
	mAutoScrollSpeed    = 10;
}

UIListbox::~UIListbox()
{
	SetTemplate(0);
	SetDataSource(0);
	SetStyle(0);
	delete [] mSelectedItems;
}

bool UIListbox::IsA( const UITypeID Type ) const
{
	return Type == TUIListbox || UIWidget::IsA( Type );
}

const char *UIListbox::GetTypeName( void ) const
{
	return TypeName;
}

UIBaseObject *UIListbox::Clone( void ) const
{
	return new UIListbox;
}

//-----------------------------------------------------------------

void UIListbox::GetScrollSizes( UISize &PageSize, UISize &LineSize ) const
{
	UIWidget::GetScrollSizes (PageSize, LineSize);
	if (mTemplate)
		LineSize.y = mTemplate->GetHeight ();
}

//-----------------------------------------------------------------

void UIListbox::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, UINotification::Code NotificationCode )
{
	if( NotifyingObject == mDataSource )
	{
		SendCallback( &UIEventCallback::OnListboxDataSourceChanged, UILowerString::null );

		switch( NotificationCode )
		{
			case UINotification::ChildRemoved:
				if( ContextObject == mSelectedDataObject )
				{
					// Selected object was deleted from the data source
					// we'll need to update the cache and change the selection
					mTemplateCache.Remove( mSelectedDataObject );
					UpdateSelectionDataItemFromIndex();
					SendCallback( &UIEventCallback::OnListboxSelectionChanged, PropertyName::OnSelect );
				}
				else
				{
					// Data may have been deleted that preceeded our selection,
					// requiring us to move the selection index
					UpdateSelectionIndexFromDataItem();
					SendCallback( &UIEventCallback::OnListboxSelectionChanged, PropertyName::OnSelect );
				}
				break;

			case UINotification::ChildAdded:
			{
				// Data may have been added that preceeded our selection,
				// requiring us to move the selection index
				UpdateSelectionDataItemFromIndex();
				break;
			}
		}
	}
	else if( NotifyingObject == mTemplate )
	{
		mTemplateCache.Clear();
	}
	else if( NotifyingObject == &UIClock::gUIClock() )
	{
		if( mTemplate && mStyle && mAutoScroll != 0  )
		{
			UISize  TemplateSize;
			const UISize & Size = GetSize ();
			UISize  ScrollExtent;

			long OldSelection = mSelection;

			mTemplate->GetSize( TemplateSize );

			GetScrollExtent( ScrollExtent );
			UIPoint ScrollLocation (GetScrollLocation());

			if( mStyle->GetLayout() == UIStyle::L_vertical )
			{
				ScrollLocation.y += mAutoScroll;

				if( ScrollLocation.y > ScrollExtent.y - Size.y )
					ScrollLocation.y = ScrollExtent.y - Size.y;

				if( ScrollLocation.y < 0 )
					ScrollLocation.y = 0;

				mSelection = (long)floor( (float)(mLastMousePosition.y + ScrollLocation.y ) / (float)TemplateSize.y );
			}
			else
			{
				ScrollLocation.x += mAutoScroll;

				if( ScrollLocation.x > ScrollExtent.x - Size.x )
					ScrollLocation.x = ScrollExtent.x - Size.x;

				if( ScrollLocation.x < 0 )
					ScrollLocation.x = 0;

				mSelection = (long)floor( (float)(mLastMousePosition.x + ScrollLocation.x ) / (float)TemplateSize.x );
			}

			SetScrollLocation( ScrollLocation );

			if( mSelection != OldSelection )
			{
				UpdateSelectionDataItemFromIndex();
				SendCallback( &UIEventCallback::OnListboxSelectionChanged, PropertyName::OnSelect );
			}
		}
	}

	mTemplateCache.Notify( NotifyingObject, ContextObject, NotificationCode );
}

//----------------------------------------------------------------------

bool UIListbox::ProcessMessage( const UIMessage &msg )
{
	if( UIWidget::ProcessMessage( msg ) )
		return true;

	const long OldSelection = mSelection;

	bool handled = false;

	switch( msg.Type )
	{
		case UIMessage::KeyDown:
		case UIMessage::KeyRepeat:
		{
			if( mStyle->GetLayout() == UIStyle::L_vertical )
			{
				if( msg.Keystroke == UIMessage::UpArrow )
				{
					if( mSelection > 0 )
						--mSelection;
				}
				else if( msg.Keystroke == UIMessage::DownArrow )
				{
					if( mSelection < (long)mDataSource->GetChildCount() - 1 )
						++mSelection;
				}
			}
			else
			{
				if( msg.Keystroke == UIMessage::LeftArrow )
				{
					if( mSelection > 0 )
						--mSelection;
				}
				else if( msg.Keystroke == UIMessage::RightArrow )
				{
					if( mSelection < (long)mDataSource->GetChildCount() - 1 )
						++mSelection;
				}
			}

			if( mStyle && (OldSelection != mSelection) )
			{
				UIPoint OldScrollLocation;
				UIPoint ScrollLocation (GetScrollLocation ());
				UISize  TemplateSize;
				const UISize  & Size = GetSize ();

				mTemplate->GetSize( TemplateSize );

				OldScrollLocation = ScrollLocation;

				if( mStyle->GetLayout() == UIStyle::L_vertical )
				{
					if( (ScrollLocation.y + Size.y - TemplateSize.y) < (long)mSelection * TemplateSize.y )
						ScrollLocation.y = mSelection * TemplateSize.y - Size.y + TemplateSize.y;
					else if( ScrollLocation.y > (long)mSelection * TemplateSize.y )
						ScrollLocation.y = mSelection * TemplateSize.y;
				}
				else
				{
					if( (ScrollLocation.x + Size.x - TemplateSize.x) < (long)mSelection * TemplateSize.x )
						ScrollLocation.x = mSelection * TemplateSize.x - Size.x + TemplateSize.x;
					else if( ScrollLocation.x > (long)mSelection * TemplateSize.x )
						ScrollLocation.x = mSelection * TemplateSize.x;
				}

				if( ScrollLocation != OldScrollLocation )
				{
					SetScrollLocation( ScrollLocation );
				}
			}
			break;
		}
		case UIMessage::LeftMouseDown:

			mMouseDown = true;

			if( mTemplate && mStyle )
			{
				const UIPoint & ScrollLocation = GetScrollLocation ();
				UISize	TemplateSize;

				mLastMousePosition = msg.MouseCoords;

				mTemplate->GetSize( TemplateSize );

				if( mStyle->GetLayout() == UIStyle::L_vertical )
					mSelection = (long)floor( (float)(msg.MouseCoords.y + ScrollLocation.y ) / (float)TemplateSize.y );
				else
					mSelection = (long)floor( (float)(msg.MouseCoords.x + ScrollLocation.x ) / (float)TemplateSize.x );

				UIClock::gUIClock().Listen(this);
			}
			break;

		case UIMessage::LeftMouseDoubleClick:
			SendCallback (&UIEventCallback::OnListboxDoubleClicked, PropertyName::OnDoubleClick );
			handled = true;
			break;

		case UIMessage::MouseMove:
		{
			if( mTemplate && mStyle )
			{
				const UIPoint & ScrollLocation = GetScrollLocation();

				mAutoScroll				 = 0;
				mLastMousePosition = msg.MouseCoords;

				UISize	TemplateSize;

				mTemplate->GetSize( TemplateSize );

				if( GetTooltipStyle() )
				{
					long MousedOverItem;

					if( mStyle->GetLayout() == UIStyle::L_vertical )
						MousedOverItem = (long)floor( (float)(msg.MouseCoords.y + ScrollLocation.y ) / (float)TemplateSize.y );
					else
						MousedOverItem = (long)floor( (float)(msg.MouseCoords.x + ScrollLocation.x ) / (float)TemplateSize.x );

					GetTooltipStyle()->SetItemInDataSource( MousedOverItem );
				}

				if( mAutoScrollSpeed > 0.0f && mMouseDown && msg.Modifiers.LeftMouseDown )
				{
					if( mStyle->GetLayout() == UIStyle::L_vertical )
					{
						mSelection = (long)floor( (float)(msg.MouseCoords.y + ScrollLocation.y ) / (float)TemplateSize.y );

						if( msg.MouseCoords.y < 0 )
						{
							mAutoScroll = (long)floor((float)msg.MouseCoords.y / mAutoScrollSpeed );
						}
						else if( msg.MouseCoords.y > GetHeight() )
						{
							mAutoScroll = (long)ceil((float)(msg.MouseCoords.y - GetHeight()) / mAutoScrollSpeed );
						}
					}
					else
					{
						mSelection = (long)floor( (float)(msg.MouseCoords.x + ScrollLocation.x ) / (float)TemplateSize.x );

						if( msg.MouseCoords.x < 0 )
						{
							mAutoScroll = (long)floor((float)msg.MouseCoords.x / mAutoScrollSpeed );
						}
						else if( msg.MouseCoords.x > GetWidth() )
						{
							mAutoScroll = (long)ceil((float)(msg.MouseCoords.x - GetWidth()) / mAutoScrollSpeed );
						}
					}

					handled = true;
				}
			}
			break;
		}

		case UIMessage::LeftMouseUp:
		{
			mMouseDown = false;
			UIBaseObject *Parent = GetParent();

			mLastMousePosition = msg.MouseCoords;
			mAutoScroll				 = 0;

			if( Parent && Parent->IsA( TUIWidget ) )
				static_cast<UIWidget *>( Parent )->ProcessChildNotificationMessage( this, msg );

			UIClock::gUIClock().StopListening(this);

			break;
		}

		case UIMessage::MouseExit:
			mMouseDown = false;
			break;
	}

	if( mSelection != OldSelection )
	{
		UpdateSelectionDataItemFromIndex();
		SendCallback( &UIEventCallback::OnListboxSelectionChanged, PropertyName::OnSelect );
		handled = true;
	}

	const UIData * const data = GetSelectedItem ();

	if (data)
	{
		if (msg.IsMouseMessage ())
		{
			UIWidgetVector *topLevelObjects = mTemplateCache.Get( data );

			if (topLevelObjects)
			{
				UIObjectList olist (topLevelObjects->begin (), topLevelObjects->end ());

				UIMessage translatedMsg (msg);

				const UISize TemplateSize    = mTemplate->GetSize ();
				const UIPoint ScrollLocation = GetScrollLocation ();

				if( mStyle->GetLayout() == UIStyle::L_vertical )
					translatedMsg.MouseCoords.y -= GetSelectionIndex () * TemplateSize.y - ScrollLocation.y;
				else
					translatedMsg.MouseCoords.x -= GetSelectionIndex () * TemplateSize.x - ScrollLocation.x;

				bool dummyChildWasHit = false;
				return static_cast<UIPage *>(GetParent ())->ProcessMouseMessageUsingControlSet (translatedMsg, olist, dummyChildWasHit);

			}
		}
	}

	return handled;
}

//----------------------------------------------------------------------

void UIListbox::GetScrollExtent( UISize &Out ) const
{
	if( !mTemplate || !mDataSource || !mStyle )
	{
		Out.x = 0;
		Out.y = 0;
	}
	else
	{
		UISize TemplateSize;
		const UISize & Size = GetSize ();

		mTemplate->GetSize( TemplateSize );

		if( mStyle->GetLayout() == UIStyle::L_vertical )
		{
			if( Size.x > TemplateSize.x )
				Out.x = Size.x;
			else
				Out.x = TemplateSize.x;

			Out.y = TemplateSize.y * mDataSource->GetChildCount();

			if( Size.y > Out.y )
				Out.y = Size.y;
		}
		else
		{
			Out.x = TemplateSize.x * mDataSource->GetChildCount();

			if( Size.x > Out.x )
				Out.x = Size.x;

			if( Size.y > TemplateSize.y )
				Out.y = Size.y;
			else
				Out.y = TemplateSize.y;
		}
	}
}

void UIListbox::Render( UICanvas &DestinationCanvas ) const
{

	UIWidget::Render (DestinationCanvas);

	if( !mTemplate || !mDataSource || !mStyle )
		return;

	UIPoint			CellTranslation(0,0);
	UIPoint			CellTranslationStep;
	UIRect			ClippingRect;
	UISize			TemplateSize;
	long				d(0);
	long				step;
	long				LowerBound;
	long				UpperBound;

	UIDataList & CurrentData = mDataSource->GetData();

	mTemplate->GetSize( TemplateSize );

	if( mStyle->GetGridStyle() )
	{
		UISize Dimensions;

		if( mStyle->GetLayout() == UIStyle::L_vertical )
		{
			Dimensions.x = 1;
			Dimensions.y = CurrentData.size();
		}
		else
		{
			Dimensions.x = CurrentData.size();
			Dimensions.y = 1;
		}

		long  DataSize      = CurrentData.size();
		//bool *SelectedItems = new bool[DataSize];
		if(DataSize > mSelectedItemsAllocatedSize)
		{
			delete mSelectedItems;
			mSelectedItemsAllocatedSize = DataSize;
			mSelectedItems = new bool[mSelectedItemsAllocatedSize];
		}

		for( long i = 0; i < DataSize; ++i )
		{
			if( i == mSelection )
				mSelectedItems[i] = true;
			else
				mSelectedItems[i] = false;
		}

		mStyle->GetGridStyle()->Render( GetAnimationState(), DestinationCanvas, TemplateSize, Dimensions, mSelectedItems );
	}

	DestinationCanvas.GetClip( ClippingRect );

	if( mStyle->GetLayout() == UIStyle::L_vertical )
	{
		step = TemplateSize.y;

		CellTranslationStep.x = 0;
		CellTranslationStep.y = step;

		LowerBound = ClippingRect.top;
		UpperBound = ClippingRect.bottom;
	}
	else
	{
		step = TemplateSize.x;

		CellTranslationStep.x = step;
		CellTranslationStep.y = 0;

		LowerBound = ClippingRect.left;
		UpperBound = ClippingRect.right;
	}

	long rownum = 0;

	DestinationCanvas.PushState();
	DestinationCanvas.Flush();

	for( UIDataList::iterator i = CurrentData.begin(); i != CurrentData.end() && (d < UpperBound); ++i, ++rownum )
	{
		if( (d + step > LowerBound) )
		{
			UIData				 *theData					= *i;
			UIWidgetVector *TopLevelObjects = mTemplateCache.Get( theData );

			if( !TopLevelObjects )
			{
				const_cast<UIListbox *>( this )->mTemplateCache.Add( theData, mTemplate, const_cast<UIListbox *>( this ) );
				TopLevelObjects = mTemplateCache.Get( theData );
			}

			DestinationCanvas.Translate( CellTranslation );
			UIRenderHelper::RenderObjects( DestinationCanvas, *TopLevelObjects );
			DestinationCanvas.RestoreTransform();
		}
		CellTranslation += CellTranslationStep;
		d += step;
	}

	DestinationCanvas.PopState();
}

void UIListbox::MinimizeResources( void )
{
	UIWidget::MinimizeResources();
	mTemplateCache.Clear();
}

void UIListbox::SetDataSource( UIDataSource *NewDataSource )
{
	if( NewDataSource != mDataSource )
	{
		if( NewDataSource )
		{
			NewDataSource->Listen( this );
			NewDataSource->Attach( this );
		}

		// Template cache depends on the contents of mDataSource, so clear it first
		mTemplateCache.Clear();

		if( mDataSource )
		{
			mDataSource->StopListening( this );
			mDataSource->Detach( this );
		}

		mDataSource = NewDataSource;
	}
}

void UIListbox::SetTemplate( UITemplate *NewTemplate )
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

void UIListbox::SetStyle( UIListboxStyle *NewStyle )
{
	if( NewStyle )
		NewStyle->Attach( this );

	if( mStyle )
		mStyle->Detach( this );

	mStyle = NewStyle;
}

UIStyle *UIListbox::GetStyle( void ) const
{
	return mStyle;
};

void UIListbox::SetSelectionIndex( long In )
{
	long OldSelection = mSelection;

	mSelection = In;
	UpdateSelectionDataItemFromIndex();

	if( OldSelection != mSelection )
		SendCallback( &UIEventCallback::OnListboxSelectionChanged, PropertyName::OnSelect );
}

void UIListbox::ScrollTo( long In )
{
	if( !mTemplate )
		return;

	const UIPoint & ScrollLocation = GetScrollLocation();
	UISize	 TemplateSize;
	int			 ItemCount;
	int			 ItemsPerPage;

	mTemplate->GetSize( TemplateSize );

	if( mDataSource )
		ItemCount = mDataSource->GetChildCount();
	else
		ItemCount = 0;

	if( mStyle->GetLayout() == UIStyle::L_vertical )
		ItemsPerPage = GetHeight() / TemplateSize.y;
	else
		ItemsPerPage = GetWidth() / TemplateSize.x;

	if( (ItemCount - In) < ItemsPerPage )
		In = ItemCount - ItemsPerPage;

	if( In < 0 )
		In = 0;

	if( mStyle->GetLayout() == UIStyle::L_vertical )
	{
		if( ScrollLocation.y != (In * TemplateSize.y) )
			SetScrollLocation( UIPoint( ScrollLocation.x, In * TemplateSize.y ) );
	}
	else
	{
		if( ScrollLocation.x != (In * TemplateSize.x) )
			SetScrollLocation( UIPoint( In * TemplateSize.x, ScrollLocation.y ) );
	}
}

UIData *UIListbox::GetSelectedItem( void )
{
	if( mSelection < 0 || !mDataSource )
		return 0;

	return mDataSource->GetChildByPosition( mSelection );
}

//----------------------------------------------------------------------

void UIListbox::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	GetPropertyNames (In, false);
}

//----------------------------------------------------------------------

void UIListbox::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(AdvancedBehavior, category, o_groups);
}

//----------------------------------------------------------------------

void UIListbox::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	// PropertyName::SelectedItem is a special property
	// and is not returned by this function

	In.push_back( PropertyName::AutoScrollSpeed );
	In.push_back( PropertyName::DataSource );
	In.push_back( PropertyName::OnSelect );
	In.push_back( PropertyName::OnDoubleClick );
	In.push_back( PropertyName::SelectedItemIndex );
	In.push_back( PropertyName::Style );
	In.push_back( PropertyName::Template );

	UIWidget::GetPropertyNames( In, forCopy );
}

bool UIListbox::SetProperty( const UILowerString & Name, const UIString &Value )
{
	const size_t pSeparator = Name.get ().find ('.');

	if( pSeparator != std::string::npos)
	{
		const int len = pSeparator;

		if( Name.equals (PropertyName::SelectedItem.c_str (), len))
		{
			if( mSelectedDataObject )
				mSelectedDataObject->SetProperty( UILowerString (Name.get ().substr (pSeparator + 1)), Value );

			return true;
		}
	}

	if( Name == PropertyName::AutoScrollSpeed )
	{
		return UIUtils::ParseFloat( Value, mAutoScrollSpeed );
	}
	else if( Name == PropertyName::DataSource )
	{
		UIBaseObject *NewDataSource = GetObjectFromPath( Value.c_str(), TUIDataSource );

		if( NewDataSource || Value.empty() )
		{
			SetDataSource( static_cast<UIDataSource *>( NewDataSource ) );
			return true;
		}
	}
	else if( Name == PropertyName::SelectedItemIndex )
	{
		if( UIUtils::ParseLong( Value, mSelection ) )
		{
			UpdateSelectionDataItemFromIndex();
			return true;
		}
		else
			return false;
	}
	else if( Name == PropertyName::Style )
	{
		UIBaseObject *NewStyle = GetObjectFromPath( Value.c_str(), TUIListboxStyle );

		if( NewStyle || Value.empty() )
		{
			SetStyle( static_cast<UIListboxStyle *>(NewStyle) );
			return true;
		}
	}
	else if(  Name == PropertyName::Template )
	{
		UIBaseObject *NewTemplate = GetObjectFromPath( Value.c_str(), TUITemplate );

		if( NewTemplate || Value.empty() )
		{
			SetTemplate( static_cast<UITemplate *>(NewTemplate) );
			return true;
		}
	}

	return UIWidget::SetProperty( Name, Value );
}

bool UIListbox::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	const size_t pSeparator = Name.get ().find ('.');

	if( pSeparator != std::string::npos)
	{
		const int len = pSeparator;

		if( Name.equals (PropertyName::SelectedItem.c_str (), len))
		{
			if( mSelectedDataObject )
				mSelectedDataObject->GetProperty( UILowerString (Name.get ().substr (pSeparator + 1)), Value );

			return true;
		}
	}

	if( Name == PropertyName::AutoScrollSpeed )
		return UIUtils::FormatFloat( Value, mAutoScrollSpeed );
	else if( Name == PropertyName::DataSource )
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

void UIListbox::UpdateSelectionDataItemFromIndex( void )
{
	if( mDataSource )
	{
		UIDataList & CurrentData = mDataSource->GetData();

		if( mSelection >= (long)CurrentData.size() )
			mSelection = CurrentData.size() - 1;

		long DataItemIndex = 0;

		for( UIDataList::iterator i = CurrentData.begin(); i != CurrentData.end(); ++i )
		{
			if( DataItemIndex == mSelection )
			{
				mSelectedDataObject = *i;
				return;
			}

			++DataItemIndex;
		}
	}

	mSelection					= -1;
	mSelectedDataObject =  0;
}

void UIListbox::UpdateSelectionIndexFromDataItem( void )
{
	if( mDataSource )
	{
		UIDataList & CurrentData   = mDataSource->GetData();
		long        DataItemIndex = 0;

		for( UIDataList::iterator i = CurrentData.begin(); i != CurrentData.end(); ++i )
		{
			if( mSelectedDataObject == *i )
			{
				mSelection = DataItemIndex;
				return;
			}

			++DataItemIndex;
		}
	}

	mSelection					= -1;
	mSelectedDataObject =  0;
}