#include "_precompile.h"

#include "UINamespace.h"
#include <cassert>
#include <list>

//-----------------------------------------------------------------

const char *UINamespace::TypeName = "Namespace";

//-----------------------------------------------------------------

UINamespace::UINamespace() :
UIBaseObject (),
mChildren ()
{
}

//-----------------------------------------------------------------

UINamespace::~UINamespace()
{
	while( !mChildren.empty() )
	{
		UIBaseObject * const obj = mChildren.front ();
		assert (obj);
		obj->SetParent (0);
		obj->Detach( this );
		mChildren.pop_front();
	}
}

//-----------------------------------------------------------------

bool UINamespace::IsA( const UITypeID Type ) const
{
	return (Type == TUINamespace) || UIBaseObject::IsA( Type );
}

//-----------------------------------------------------------------

const char *UINamespace::GetTypeName( void ) const
{
	return TypeName;
}

//-----------------------------------------------------------------

UIBaseObject *UINamespace::Clone( void ) const
{
	return new UINamespace;
}
		
//-----------------------------------------------------------------

bool UINamespace::AddChild( UIBaseObject *ChildToAdd )
{
	if( ChildToAdd && !ChildToAdd->IsA( TUIWidget ) )
	{
		mChildren.push_back( ChildToAdd );
		ChildToAdd->SetParent( this );
		ChildToAdd->Attach( this );
		return true;
	}	
	return false;
}

//-----------------------------------------------------------------

bool UINamespace::RemoveChild( UIBaseObject *ObjectToRemove )
{
	for( UIObjectList::iterator i = mChildren.begin(); i != mChildren.end(); ++i )
	{
		if( *i == ObjectToRemove )
		{
			mChildren.erase(i);
			ObjectToRemove->SetParent (0);
			ObjectToRemove->Detach( this );
			return true;
		}		
	}
	return false;
}

//-----------------------------------------------------------------

void UINamespace::GetChildren( UIObjectList &Out ) const
{
	Out.insert (Out.end (), mChildren.begin (), mChildren.end ());
}

//-----------------------------------------------------------------

unsigned long UINamespace::GetChildCount( void ) const
{
	return mChildren.size();
}

//-----------------------------------------------------------------

UIBaseObject * UINamespace::GetChildByPositionLinear (int pos)
{
	if (pos < 0 || pos >= static_cast<int>(mChildren.size ()))
		return 0;

	UIObjectList::iterator it = mChildren.begin ();

	std::advance (it, pos);
	assert (it != mChildren.end ());

	return *it;
}

//-----------------------------------------------------------------

UIBaseObject *UINamespace::GetChild( const char *ChildName ) const
{	
	const char *pSeparator = strchr( ChildName, '.' );
	int					len;

	if( pSeparator )
		len = pSeparator - ChildName;
	else
		len = strlen( ChildName );

	for ( ; ; )
	{
		const UIObjectList::const_iterator end = mChildren.end();
		for( UIObjectList::const_iterator i = mChildren.begin(); i != end; ++i )
		{
			UIBaseObject *o = *i;

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

//-----------------------------------------------------------------

bool UINamespace::CanChildMove( UIBaseObject *ObjectToMove, ChildMovementDirection MoveDirection )
{
	if( !ObjectToMove )
		return false;

	for( UIObjectList::const_iterator i = mChildren.begin(); i != mChildren.end(); ++i )
	{
		if( *i == ObjectToMove )
		{
			if( i == mChildren.begin() )
			{
				if( (MoveDirection == Up) || (MoveDirection == Top) )
					return false;
			}
			else if( ++i == mChildren.end() )
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

bool UINamespace::MoveChild( UIBaseObject * ObjectToMove, ChildMovementDirection MoveDirection)
{
	if( !ObjectToMove )
		return false;

	UIObjectList::iterator i = std::find (mChildren.begin(), mChildren.end(), ObjectToMove);

	if (i == mChildren.end())
	{
		assert (false);
		return false;
	}
	
	UIBaseObject *o = *i;
		
	if( (MoveDirection == Up) )
	{
		if( i != mChildren.begin() )
		{
			--i;
			mChildren.insert( i, o );
			mChildren.erase( ++i );
			return true;
		}
	}
	else if( (MoveDirection == Top) )
	{
		if( i != mChildren.begin() )
		{
			mChildren.insert( mChildren.begin(), o );
			mChildren.erase(i);
			return true;
		}
	}
	else if( (MoveDirection == Down) )
	{		
		UIObjectList::iterator InsertPoint = i;
		++InsertPoint;

		if (InsertPoint != mChildren.end ())
			++InsertPoint;

		mChildren.insert( InsertPoint, o );
		mChildren.erase( i );
		return true;
	}

	else if( (MoveDirection == Bottom) )
	{
		if( o != mChildren.back() )
		{
			mChildren.insert( mChildren.end(), o );
			mChildren.erase( i ); 
			return true;
		}
	}
	else
		assert( false );
	
	return false;
}

//-----------------------------------------------------------------

void UINamespace::MinimizeResources( void )
{
	UIBaseObject::MinimizeResources();

	for( UIObjectList::const_iterator i = mChildren.begin(); i != mChildren.end(); ++i )
		(*i)->MinimizeResources();
}

//-----------------------------------------------------------------

void UINamespace::Link( void )
{
	UIBaseObject::Link();

	for( UIObjectList::const_iterator i = mChildren.begin(); i != mChildren.end(); ++i )
		(*i)->Link();
}

//-----------------------------------------------------------------

void UINamespace::ResetLocalizedStrings (void)
{
	for( UIObjectList::iterator i = mChildren.begin(); i != mChildren.end(); ++i )
	{
		(*i)->ResetLocalizedStrings ();
	}
}

//----------------------------------------------------------------------

bool UINamespace::SetProperty( const UILowerString & Name, const UIString &Value )
{
	const size_t pSeparator = Name.get ().find ('.');
	
	if( pSeparator != std::string::npos)
	{
		const int len = pSeparator;		

		for( UIObjectList::iterator o = mChildren.begin(); o != mChildren.end(); ++o )
		{
			UIBaseObject *theObject = *o;

			if( theObject->IsName( Name.c_str (), len ) )
				return theObject->SetProperty( UILowerString (Name.c_str () + pSeparator + 1), Value );
		}
		// Fall through
	}
	
	return UIBaseObject::SetProperty( Name, Value );
}
//-----------------------------------------------------------------
