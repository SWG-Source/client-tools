// ======================================================================
//
// UIDataSourceContainer.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "_precompile.h"
#include "UIDataSourceContainer.h"

#include <cassert>
#include <list>

// ======================================================================

const char * const  UIDataSourceContainer::TypeName = "DataSourceContainer";

UIDataSourceContainer::UIDataSourceContainer() :
mDataSourceBaseList (new DataSourceBaseList)
{
}

//-----------------------------------------------------------------

UIDataSourceContainer::~UIDataSourceContainer()
{
	for (DataSourceBaseList::iterator it = mDataSourceBaseList->begin (); it != mDataSourceBaseList->end (); ++it)
	{
		(*it)->StopListening( this );
		(*it)->SetParent (0);
		(*it)->Detach( this );
		(*it) = 0;
	}

	mDataSourceBaseList->clear ();

	delete mDataSourceBaseList;
	mDataSourceBaseList = 0;
}

//-----------------------------------------------------------------

bool UIDataSourceContainer::IsA           ( const UITypeID type ) const
{
	return type == TUIDataSourceContainer || UIDataSourceBase::IsA (type);
}

//-----------------------------------------------------------------

UIBaseObject   *   UIDataSourceContainer::Clone         ( void ) const
{ 
	return new UIDataSourceContainer;
}

//-----------------------------------------------------------------

bool UIDataSourceContainer::AddChild      ( UIBaseObject * obj )
{
	if (obj->IsA (TUIDataSourceBase))
	{	
		UIDataSourceBase * const base = static_cast<UIDataSourceBase *>( obj);
		
		base->Attach( this );
		base->SetParent( this );
		base->Listen( this );
		
		mDataSourceBaseList->push_back( base );
		
		SendNotification( UINotification::ChildAdded, base );
		return true;
	}

	assert (false);
	return false;
}

//-----------------------------------------------------------------

bool UIDataSourceContainer::RemoveChild   ( UIBaseObject * obj )
{
	if (obj->IsA (TUIDataSourceBase) && obj->GetParent () == this)
	{	
		UIDataSourceBase * const base = static_cast<UIDataSourceBase *>( obj);
		
		assert (std::find (mDataSourceBaseList->begin (), mDataSourceBaseList->end (), base) != mDataSourceBaseList->end ());
		mDataSourceBaseList->remove (base);

		base->StopListening( this );
		SendNotification( UINotification::ChildRemoved, base );
		base->SetParent( 0 );
		base->Detach( this );		
		
		return true;
	}

	assert (false);
	return false;
}

//-----------------------------------------------------------------

UIBaseObject   *   UIDataSourceContainer::GetChild      ( const char * name) const
{
	if (!name)
	{
		assert (false);
		return 0;
	}

	const char *pSeparator = strchr( name, '.' );
	int					len;

	if( pSeparator )
		len = pSeparator - name;
	else
		len = strlen( name );

	for ( ; ; )
	{		
		for (DataSourceBaseList::const_iterator it = mDataSourceBaseList->begin (); it != mDataSourceBaseList->end (); ++it)
		{
			UIBaseObject * const o = *it;

			if( o->IsName( name, len ) )
			{
				if( pSeparator )
				{
					UIBaseObject *Child = o->GetChild( name + len + 1 );

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
				len = pSeparator - name;
			else
				len = strlen( name );
		}
		else
			return 0;
	}

	return 0;
}

//-----------------------------------------------------------------

void               UIDataSourceContainer::GetChildren   ( UIObjectList & objectList) const
{
	objectList.insert (objectList.end (), mDataSourceBaseList->begin (), mDataSourceBaseList->end ());
}

//-----------------------------------------------------------------

unsigned long      UIDataSourceContainer::GetChildCount ( void ) const
{
	return mDataSourceBaseList->size ();
}

//-----------------------------------------------------------------

bool               UIDataSourceContainer::CanChildMove  ( UIBaseObject * child, ChildMovementDirection dir)
{
	if (!child || mDataSourceBaseList->empty ())
	{
		assert (false);
		return false;
	}

	switch (dir)
	{
	case UIBaseObject::Up:
	case UIBaseObject::Top:
		return (child != mDataSourceBaseList->front ());
		
	case UIBaseObject::Down:
	case UIBaseObject::Bottom:

		return (child != mDataSourceBaseList->back ());

	default:
		assert (false);
		return false;	
	}
}

//-----------------------------------------------------------------

bool               UIDataSourceContainer::MoveChild     ( UIBaseObject * child , ChildMovementDirection dir)
{
	if (!child || mDataSourceBaseList->empty () || !child->IsA (TUIDataSourceBase))
	{
		assert (false);
		return false;
	}

	UIDataSourceBase * const base = static_cast<UIDataSourceBase *>(child);

	DataSourceBaseList::iterator it = mDataSourceBaseList->end ();

	switch (dir)
	{
	case UIBaseObject::Up:
		if (base == mDataSourceBaseList->front ())
			return false;

		it = std::find (mDataSourceBaseList->begin (), mDataSourceBaseList->end (), base);
		assert (it != mDataSourceBaseList->end ());
		mDataSourceBaseList->erase (it--);
		break;

	case UIBaseObject::Top:
		
		if (base == mDataSourceBaseList->front ())
			return false;

		mDataSourceBaseList->remove (base);
		it = mDataSourceBaseList->begin ();
		break;

	case UIBaseObject::Down:
		if (base == mDataSourceBaseList->back ())
			return false;
		it = std::find (mDataSourceBaseList->begin (), mDataSourceBaseList->end (), base);
		
		assert (it != mDataSourceBaseList->end ());
		
		mDataSourceBaseList->erase (it++);

		break;

	case UIBaseObject::Bottom:
		if (base == mDataSourceBaseList->back ())
			return false;

		mDataSourceBaseList->remove (base);
		it = mDataSourceBaseList->end ();
		break;

	default:
		assert (false);
		return false;	
	}

	if (it == mDataSourceBaseList->end ())
		mDataSourceBaseList->push_back (base);
	else
		mDataSourceBaseList->insert (it, base);

	SendNotification( UINotification::ObjectChanged, this );

	return true;
}

//-----------------------------------------------------------------

void UIDataSourceContainer::Clear( void )
{
	for (DataSourceBaseList::iterator it = mDataSourceBaseList->begin (); it != mDataSourceBaseList->end (); ++it)
	{
		(*it)->StopListening( this );
		(*it)->SetParent (0);
		(*it)->Detach( this );
		(*it) = 0;
	}

	mDataSourceBaseList->clear ();

	SendNotification( UINotification::ObjectChanged, this );
}

//-----------------------------------------------------------------

UIDataSourceBase  *UIDataSourceContainer::GetChildByPositionLinear( unsigned long pos ) const
{
	if (pos >= mDataSourceBaseList->size ())
		return 0;

	DataSourceBaseList::const_iterator it = mDataSourceBaseList->begin ();

	std::advance (it, pos);

	return *it;
}


//-----------------------------------------------------------------

void UIDataSourceContainer::AddChildByPosition( UIDataSourceBase *obj, long pos )
{
	if (pos < 0 || obj->GetParent ())
	{
		assert (false);
		return;
	}

	if (pos >= static_cast<long>(mDataSourceBaseList->size ()))
	{
		mDataSourceBaseList->push_back (obj);
		SendNotification( UINotification::ChildAdded, obj );
	}
	else
	{
		DataSourceBaseList::iterator it = mDataSourceBaseList->begin ();
		std::advance (it, pos);

		obj->Attach( this );
		obj->SetParent( this );
		obj->Listen( this );

		mDataSourceBaseList->insert (it, obj);
		
		SendNotification( UINotification::ChildAdded, obj );
	}
}

//-----------------------------------------------------------------

void UIDataSourceContainer::ResetLocalizedStrings ()
{
	const DataSourceBaseList::const_iterator end = mDataSourceBaseList->end ();
	for (DataSourceBaseList::const_iterator iter = mDataSourceBaseList->begin (); iter != end; ++iter)
	{
		(*iter)->ResetLocalizedStrings ();
	}
}

//----------------------------------------------------------------------

bool UIDataSourceContainer::SetProperty( const UILowerString & Name, const UIString &Value )
{
	const size_t pSeparator = Name.get ().find ('.');
	
	if( pSeparator != std::string::npos)
	{
		const int len = pSeparator;		

		for( DataSourceBaseList::iterator o = mDataSourceBaseList->begin(); o != mDataSourceBaseList->end(); ++o )
		{
			UIDataSourceBase *theObject = *o;

			if( theObject->IsName( Name.c_str (), len ) )
				return theObject->SetProperty( UILowerString (Name.c_str () + pSeparator + 1), Value );
		}
		// Fall through
	}

	if (UIBaseObject::SetProperty (Name, Value))
		SendNotification( UINotification::ObjectChanged, this);

	return false;
}

// ======================================================================
