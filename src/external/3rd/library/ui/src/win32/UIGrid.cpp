#include "_precompile.h"

#include "UICanvas.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIGrid.h"
#include "UIGridStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIRenderHelper.h"
#include "UITemplate.h"
#include "UITemplateCache.h"
#include "UIUtils.h"

const UILowerString UIGrid::PropertyName::CellCount						= UILowerString ("CellCount");
const UILowerString UIGrid::PropertyName::CellSize						= UILowerString ("CellSize");
const UILowerString UIGrid::PropertyName::DataSource					= UILowerString ("DataSource");
const UILowerString UIGrid::PropertyName::SelectedItem				= UILowerString ("SelectedItem");
const UILowerString UIGrid::PropertyName::Style								= UILowerString ("Style");

const char *UIGrid::TypeName = "Grid";

//======================================================================================
#define _TYPENAME UIGrid

namespace UIGridNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(CellCount, "", T_point),
		_DESCRIPTOR(CellSize, "", T_point),
		_DESCRIPTOR(DataSource, "", T_object),
		_DESCRIPTOR(Style, "", T_object),
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UIGridNamespace;
//======================================================================================

UIGrid::UIGrid() : mStyle(0), mDataSource(0), mSelectedDataObject(0), mCellCount(0,0), mCellSize(0,0), mCellSelectedState(0)
{
}

//-----------------------------------------------------------------

UIGrid::~UIGrid()
{
	SetStyle(0);
	SetDataSource(0);

	delete mCellSelectedState;

	for( UITemplateSet::iterator i = mAttachedTemplates.begin(); i != mAttachedTemplates.end(); ++i )
	{
		UITemplate *theTemplate = *i;

		theTemplate->StopListening( this );
		theTemplate->Detach( this );
	}
}

//-----------------------------------------------------------------

bool UIGrid::IsA( const UITypeID Type ) const
{
	return (Type == TUIGrid) || UIWidget::IsA( Type );
}

//-----------------------------------------------------------------

const char *UIGrid::GetTypeName( void ) const
{
	return TypeName;
}

//-----------------------------------------------------------------

UIBaseObject *UIGrid::Clone( void ) const
{
	return new UIGrid;
}

//-----------------------------------------------------------------

void UIGrid::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, UINotification::Code NotificationCode )
{
	if( NotifyingObject == mDataSource )
	{
		switch( NotificationCode )
		{
			case UINotification::ChildRemoved:
				if( ContextObject == mSelectedDataObject )
				{
					// Selected object was deleted from the data source
					// we'll need to update the cache and change the selection
					mTemplateCache.Remove( mSelectedDataObject );
					mSelectedDataObject = 0;
				}
				break;
		}
	}
	else
	{
		mTemplateCache.Clear();
	}

	mTemplateCache.Notify( NotifyingObject, ContextObject, NotificationCode );
}

//-----------------------------------------------------------------

void UIGrid::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	GetPropertyNames (In, false);
}

//----------------------------------------------------------------------

void UIGrid::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
}

//----------------------------------------------------------------------

void UIGrid::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back( PropertyName::CellCount );
	In.push_back( PropertyName::CellSize );
	In.push_back( PropertyName::DataSource );
	In.push_back( PropertyName::Style );

	UIWidget::GetPropertyNames( In, forCopy );
}

//-----------------------------------------------------------------

bool UIGrid::SetProperty( const UILowerString & Name, const UIString &Value )
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

	if( Name == PropertyName::CellCount )
	{
		if( UIUtils::ParsePoint( Value, mCellCount ) )
		{
			delete mCellSelectedState;
			mCellSelectedState = 0;

			long len = mCellCount.x * mCellCount.y;

			if( len > 0 )
			{
				mCellSelectedState = new bool[len];
				memset( mCellSelectedState, 0, len * sizeof( bool ) );
			}
		}
	}
	else if( Name == PropertyName::CellSize )
		return UIUtils::ParsePoint( Value, mCellSize );
	else if( Name == PropertyName::DataSource )
	{
		UIBaseObject *NewDataSource = GetObjectFromPath( Value.c_str(), TUIDataSource );

		if( NewDataSource || Value.empty() )
		{
			SetDataSource( reinterpret_cast<UIDataSource *>( NewDataSource ) );
			return true;
		}
	}
	else if( Name == PropertyName::Style )
	{
		UIGridStyle *NewStyle = reinterpret_cast<UIGridStyle *>( GetObjectFromPath( Value.c_str(), TUIGridStyle ) );

		if( NewStyle )
		{
			SetStyle( NewStyle );
			return true;
		}
	}

	return UIWidget::SetProperty( Name, Value );
}

//-----------------------------------------------------------------

bool UIGrid::GetProperty( const UILowerString & Name, UIString &Value ) const
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

	if( Name == PropertyName::CellCount )
		return UIUtils::FormatPoint( Value, mCellCount );
	else if( Name == PropertyName::CellSize )
		return UIUtils::FormatPoint( Value, mCellSize );
	else if( Name == PropertyName::DataSource )
	{
		if( mDataSource )
		{
			GetPathTo( Value, mDataSource );
			return true;
		}
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

//-----------------------------------------------------------------

bool UIGrid::ProcessMessage( const UIMessage &theMessage )
{
	return UIWidget::ProcessMessage( theMessage );
}

//-----------------------------------------------------------------

void UIGrid::Render( UICanvas &DestinationCanvas ) const
{
	UIWidget::Render (DestinationCanvas);

	if( mStyle )
	{
		mStyle->Render( GetAnimationState(), DestinationCanvas, mCellSize, mCellCount, mCellSelectedState );
		DestinationCanvas.Flush();
	}

	if( mDataSource )
	{
		UIRect      RenderRect( 0, 0, GetWidth(), GetHeight() );
		UIRect			UpdateRect( INT_MAX, INT_MAX, -INT_MAX, -INT_MAX );
		UIDataList & theDataSet = mDataSource->GetData();

		RenderRect += GetScrollLocation();

		for( UIDataList::iterator i = theDataSet.begin(); i != theDataSet.end(); ++i )
		{
			UIData	*theData = *i;
			UIString theLocationProperty;
			UIString theExtentProperty;
			UIString theTemplateProperty;
			UIPoint  theLocation;
			UISize   theExtent;

			static const UILowerString LocationPropertyName     ("L");
			static const UILowerString ExtentPropertyName       ("E");
			static const UILowerString TemplatePropertyName ("T");

			if( !theData->GetProperty(LocationPropertyName, theLocationProperty ) || !UIUtils::ParsePoint( theLocationProperty.c_str(), theLocation ) )
			{
				UIUtils::RunTimeDiagnosticDump( this, "Encountered a data item without a location tag (L)", theData );
				continue;
			}

			if( !theData->GetProperty(ExtentPropertyName, theExtentProperty ) || !UIUtils::ParsePoint( theExtentProperty.c_str(), theExtent ) )
			{
				UIUtils::RunTimeDiagnosticDump( this, "Encountered a data item without an extent tag (E)", theData );
				continue;
			}

			if( !theData->GetProperty(TemplatePropertyName, theTemplateProperty ) )
			{
				UIUtils::RunTimeDiagnosticDump( this, "Encountered a data item without a template tag (T)", theData );
				continue;
			}

			UIRect ItemRect;

			ItemRect.left   = theLocation.x * mCellSize.x;
			ItemRect.top    = theLocation.y * mCellSize.y;
			ItemRect.right  = ItemRect.left + theExtent.x * mCellSize.x;
			ItemRect.bottom = ItemRect.top  + theExtent.y * mCellSize.y;

			if( UIUtils::RectanglesIntersect( ItemRect, RenderRect ) )
			{
				if( UIUtils::RectanglesIntersect( ItemRect, UpdateRect ) )
				{
					UpdateRect.Set( INT_MAX, INT_MAX, -INT_MAX, -INT_MAX );
					DestinationCanvas.Flush();
				}
				else
				{
					UIUtils::UnionRect( UpdateRect, ItemRect );
				}

				UIWidgetVector *theTemplateInstanciation = mTemplateCache.Get( theData );

				if( !theTemplateInstanciation )
				{
					UITemplate *theTemplate = reinterpret_cast<UITemplate *>( GetObjectFromPath( theTemplateProperty.c_str(), TUITemplate ) );

					if( !theTemplate )
					{
						UIUtils::RunTimeDiagnosticDump( this, "Encountered a data item which specified an undefined template", theData );
						continue;
					}

					UIGrid *NonConstThis = const_cast<UIGrid *>( this );

					if( mAttachedTemplates.find( theTemplate ) == mAttachedTemplates.end() )
					{
						theTemplate->Attach( this );
						theTemplate->Listen( NonConstThis );
						NonConstThis->mAttachedTemplates.insert( theTemplate );
					}

					NonConstThis->mTemplateCache.Add( theData, theTemplate, NonConstThis );
					theTemplateInstanciation = mTemplateCache.Get( theData );

					TranslateTemplateInstanceToCell( theTemplateInstanciation, theLocation );
				}

				UIRenderHelper::RenderObjects( DestinationCanvas, *theTemplateInstanciation );
			}
		}
	}
}

//-----------------------------------------------------------------

UIWidget *UIGrid::GetWidgetFromPoint( const UIPoint &PointToTest, bool mustGetInput ) const
{
	if( mDataSource )
	{
		const UIDataList & theDataSet = mDataSource->GetData();

		for( UIDataList::const_iterator i = theDataSet.begin(); i != theDataSet.end(); ++i )
		{
			const UIData *theData = *i;
			UIString			theLocationProperty;
			UIString			theExtentProperty;
			UIPoint				theLocation;
			UISize				theExtent;

			static const UILowerString LocationPropertyName     ("L");
			static const UILowerString ExtentPropertyName       ("E");

			if( !theData->GetProperty(LocationPropertyName, theLocationProperty ) || !UIUtils::ParsePoint( theLocationProperty.c_str(), theLocation ) )
				continue;

			if( !theData->GetProperty( ExtentPropertyName, theExtentProperty ) || !UIUtils::ParsePoint( theExtentProperty.c_str(), theExtent ) )
				continue;

			UIRect ItemRect;

			ItemRect.left   = theLocation.x * mCellSize.x;
			ItemRect.top    = theLocation.y * mCellSize.y;
			ItemRect.right  = ItemRect.left + theExtent.x * mCellSize.x;
			ItemRect.bottom = ItemRect.top  + theExtent.y * mCellSize.y;

			if( ItemRect.ContainsPoint( PointToTest ) )
			{
				UIWidgetVector *theTemplateInstanciation = mTemplateCache.Get( theData );

				if( !theTemplateInstanciation )
					continue;

				for( UIWidgetVector::const_iterator j = theTemplateInstanciation->begin(); j != theTemplateInstanciation->end(); ++j )
				{
					UIWidget *w = *j;
					const UIPoint p (PointToTest - w->GetLocation () + GetScrollLocation());

					if( w->WillDraw() && w->HitTest( p ) )
						return w->GetWidgetFromPoint( p, mustGetInput );
				}
			}
		}
	}
	return UIWidget::GetWidgetFromPoint( PointToTest, mustGetInput );
}

//-----------------------------------------------------------------

void UIGrid::SetStyle( UIGridStyle *NewStyle )
{
	if( NewStyle )
		NewStyle->Attach( this );

	if( mStyle )
		mStyle->Detach( this );

	mStyle = NewStyle;
}

//-----------------------------------------------------------------

UIStyle *UIGrid::GetStyle( void ) const
{
	return mStyle;
};

//-----------------------------------------------------------------

void UIGrid::SetDataSource( UIDataSource *NewDataSource )
{
	if( mDataSource != NewDataSource )
	{
		if( NewDataSource )
		{
			NewDataSource->Listen( this );
			NewDataSource->Attach( this );
		}

		mTemplateCache.Clear();

		if( mDataSource )
		{
			mDataSource->StopListening( this );
			mDataSource->Detach( this );
		}

		mDataSource = NewDataSource;
	}
}

//-----------------------------------------------------------------

void	UIGrid::TranslateTemplateInstanceToCell( UIWidgetVector *theTemplateInstance, const UIPoint &CellLocation ) const
{
	UIPoint Offset( CellLocation.x * mCellSize.x, CellLocation.y * mCellSize.y );

	for( UIWidgetVector::iterator i = theTemplateInstance->begin(); i != theTemplateInstance->end(); ++i )
	{
		UIWidget *w = *i;
		w->SetLocation( w->GetLocation() + Offset );
	}
}

//-----------------------------------------------------------------
