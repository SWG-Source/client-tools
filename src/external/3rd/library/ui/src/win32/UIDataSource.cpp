#include "_precompile.h"

#include "UIDataSource.h"
#include "UIData.h"
#include "UiMemoryBlockManager.h"

#include <cassert>
#include <list>

//======================================================================================

const char *UIDataSource::TypeName = "DataSource";

//======================================================================================

UIDataSource::UIDataSource() :
UIDataSourceBase (),
mData (new UIDataList)
{
}

//======================================================================================

UIDataSource::~UIDataSource()
{
	while( !mData->empty() )
	{
		mData->back()->StopListening( this );
		mData->back ()->SetParent (0);
		mData->back()->Detach( this );
		mData->pop_back();
	}

	delete mData;
	mData = 0;
}

//======================================================================================

bool UIDataSource::IsA( const UITypeID Type ) const
{
	return (Type == TUIDataSource) || UIDataSourceBase::IsA( Type );
}

//======================================================================================

const char *UIDataSource::GetTypeName( void ) const
{
	return TypeName;
}

//======================================================================================

UIBaseObject *UIDataSource::Clone( void ) const
{
	return new UIDataSource;
}

//======================================================================================

bool UIDataSource::AddChild( UIBaseObject *ChildToAdd )
{
	if( ChildToAdd && ChildToAdd->IsA( TUIData ) )
	{
		UIData *DataToAdd = static_cast<UIData *>( ChildToAdd );

		DataToAdd->Attach( this );
		DataToAdd->SetParent( this );
		DataToAdd->Listen( this );

		mData->push_back( DataToAdd );
		
		SendNotification( UINotification::ChildAdded, ChildToAdd );
		return true;
	}
	return false;
}

//======================================================================================

bool UIDataSource::RemoveChild( UIBaseObject *ChildToRemove )
{
	for( UIDataList::iterator i = mData->begin(); i != mData->end(); ++i )
	{
		if( *i == ChildToRemove )
		{
			(*i)->StopListening( this );

			mData->erase(i);
			SendNotification( UINotification::ChildRemoved, ChildToRemove );

			ChildToRemove->SetParent ( 0);
			ChildToRemove->Detach( this );
			return true;
		}
	}
	return false;
}

//======================================================================================

UIBaseObject *UIDataSource::GetChild( const char *ChildName ) const
{
	assert (ChildName);

	for( UIDataList::const_iterator i = mData->begin(); i != mData->end(); ++i )
	{
		UIData * const data = *i;
		assert (data);

		if (!_stricmp (data->GetName ().c_str (), ChildName))
			return data;
	}
	return 0;
}

//======================================================================================

void UIDataSource::GetChildren( UIObjectList &Out ) const
{
	Out.insert (Out.end (), mData->begin (), mData->end ());
}

//======================================================================================
unsigned long UIDataSource::GetChildCount( void ) const
{
	return mData->size();
}

//======================================================================================
bool UIDataSource::CanChildMove( UIBaseObject *ObjectToMove, ChildMovementDirection MoveDirection )
{
	if( !ObjectToMove )
		return false;

	for( UIDataList::const_iterator i = mData->begin(); i != mData->end(); ++i )
	{
		if( *i == ObjectToMove )
		{
			if( i == mData->begin() )
			{
				if( (MoveDirection == Up) || (MoveDirection == Top) )
					return false;
			}
			else if( ++i == mData->end() )
			{
				if( (MoveDirection == Down) || (MoveDirection == Bottom) )
					return false;
			}
			return true;
		}			
	}	
	return false;
}

//======================================================================================
bool UIDataSource::MoveChild( UIBaseObject *ObjectToMove, ChildMovementDirection MoveDirection )
{
	if( !ObjectToMove )
		return false;

	for( UIDataList::iterator i = mData->begin(); i != mData->end(); ++i )
	{
		UIData *o = *i;

		if( o == ObjectToMove )
		{
			if( (MoveDirection == Up) )
			{
				if( i != mData->begin() )
				{
					mData->insert( --i, 1, o );
					mData->erase( ++i );
					return true;
				}
			}
			else if( (MoveDirection == Top) )
			{
				if( i != mData->begin() )
				{
					mData->insert( mData->begin(), 1, o );
					mData->erase(i);
					return true;
				}
			}
			else if( (MoveDirection == Down) )
			{
				if( i != mData->end() )
				{
					UIDataList::iterator InsertPoint = i;

					++InsertPoint;

					if( InsertPoint != mData->end() )
						++InsertPoint;

					mData->insert( InsertPoint, 1, o );
					mData->erase( i );
					return true;
				}
			}
			else if( (MoveDirection == Bottom) )
			{
				if( i != mData->end() )
				{
					mData->insert( mData->end(), 1, o );
					mData->erase( i );
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

//======================================================================================
void UIDataSource::UpdateChild( UIData *Child, const UIData *NewData )
{
	Child->CopyPropertiesFrom( *NewData );
	SendNotification( UINotification::ChildChanged, Child );
}

//======================================================================================

void UIDataSource::Clear( void )
{
	while( !mData->empty() )
	{
		UIData *ChildToRemove = mData->front();
		mData->pop_front();

		ChildToRemove->StopListening( this );
		SendNotification( UINotification::ChildRemoved, ChildToRemove );
		ChildToRemove->SetParent (0);
		ChildToRemove->Detach( this );
	}
}

//======================================================================================

bool UIDataSource::RemoveChildByGUID( long theGUID )
{
	for( UIDataList::iterator i = mData->begin(); i != mData->end(); ++i )
	{
		if( (*i)->GetGUID() == theGUID )
		{
			UIData *ChildToRemove = *i;

			(*i)->StopListening( this );
			mData->erase( i );
			SendNotification( UINotification::ChildRemoved, ChildToRemove );
			ChildToRemove->SetParent (0);
			ChildToRemove->Detach( this );
			return true;
		}
	}
	return false;
}

//======================================================================================

UIData *UIDataSource::GetChildByGUID( long theGUID ) const
{
	for( UIDataList::const_iterator i = mData->begin(); i != mData->end(); ++i )
	{
		if( (*i)->GetGUID() == theGUID )
			return *i;
	}
	return 0;
}

//======================================================================================

UIData *UIDataSource::GetChildByPosition( unsigned long thePosition ) const
{
	if( thePosition >= mData->size() )
		return 0;

	UIDataList::const_iterator i = mData->begin();

	for( ; ; )
	{
		if( thePosition == 0 )
			return *i;

		++i;
		--thePosition;
	}
}

//======================================================================================

bool UIDataSource::SetChildPositionByGUID( long theGUID, long thePosition )
{
	UIData *theData = 0;

	{
		for( UIDataList::iterator i = mData->begin(); i != mData->end(); ++i )
		{
			if( (*i)->GetGUID() == theGUID )
			{
				theData = *i;
				mData->erase( i );
			}
		}
	}

	if( !theData )
		return false;

	{
		UIDataList::iterator i = mData->begin();
		for( ; i != mData->end() && thePosition != 0; ++i, --thePosition )
			{};

		mData->insert( i, theData );
	}
	return true;
}

//======================================================================================

void UIDataSource::AddChildByPosition( UIData *DataToAdd, long thePosition )
{
	UIDataList::iterator i = mData->begin();
	for( ; i != mData->end() && thePosition != 0; ++i, --thePosition )
		{};

	DataToAdd->Attach( this );
	DataToAdd->SetParent( this );
	DataToAdd->Listen( this );

	mData->insert( i, DataToAdd );

	SendNotification( UINotification::ChildAdded, DataToAdd );
}

//-----------------------------------------------------------------

void UIDataSource::ResetLocalizedStrings ()
{
	bool notify = false;

	const UIDataList::const_iterator end = mData->end ();
	for (UIDataList::const_iterator iter = mData->begin (); iter != end; ++iter)
	{
		UIData * const data = (*iter);
		data->ResetLocalizedStrings ();
		notify = notify || data->getHasChanged ();
	}

	if (notify)
		SendNotification (UINotification::ChildChanged, 0);
}

//----------------------------------------------------------------------

bool UIDataSource::SetProperty( const UILowerString & Name, const UIString &Value )
{
	const size_t pSeparator = Name.get ().find ('.');
	
	if( pSeparator != std::string::npos)
	{
		const int len = pSeparator;		

		for( UIDataList::iterator o = mData->begin(); o != mData->end(); ++o )
		{
			UIData *theObject = *o;

			if( theObject->IsName( Name.c_str (), len ) )
				return theObject->SetProperty( UILowerString (Name.c_str () + pSeparator + 1), Value );
		}
		// Fall through
	}

	if (UIBaseObject::SetProperty (Name, Value))
		SendNotification( UINotification::ObjectChanged, this);

	return false;
}

//-----------------------------------------------------------------
