// ======================================================================
//
// UIDataSourceContainer.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UIDataSourceContainer_H
#define INCLUDED_UIDataSourceContainer_H

// ======================================================================

#include "UIDataSourceBase.h"

//-----------------------------------------------------------------

class UIDataSourceContainer : public UIDataSourceBase
{
public:

	typedef ui_stdlist<UIDataSourceBase *>::fwd DataSourceBaseList;

	static const char * const  TypeName;

	                           UIDataSourceContainer();
	virtual                   ~UIDataSourceContainer();

	virtual bool               IsA           ( const UITypeID ) const;
	virtual const char     *   GetTypeName   () const { return TypeName; }
	virtual UIBaseObject   *   Clone         () const;
	virtual bool               AddChild      ( UIBaseObject * );
	virtual bool               RemoveChild   ( UIBaseObject * );
	virtual UIBaseObject   *   GetChild      ( const char * ) const;
	virtual void               GetChildren   ( UIObjectList & ) const;
	virtual unsigned long      GetChildCount () const;
	virtual bool               CanChildMove  ( UIBaseObject *, ChildMovementDirection );
	virtual bool               MoveChild     ( UIBaseObject *, ChildMovementDirection );

	        void               Clear( void );
	        UIDataSourceBase  *GetChildByPositionLinear ( unsigned long thePosition ) const;
	        bool               SetChildPositionByGUID( long GUID, long Position );
	        void               AddChildByPosition( UIDataSourceBase * child, long thePosition );

	const DataSourceBaseList & GetDataSourceBaseList() const;

	virtual void              ResetLocalizedStrings ();

	virtual bool              SetProperty( const UILowerString & Name, const UIString &Value );

private:
	UIDataSourceContainer (const UIDataSourceContainer & rhs);
	UIDataSourceContainer & operator= (const UIDataSourceContainer & rhs);

	DataSourceBaseList *         mDataSourceBaseList;

};

//-----------------------------------------------------------------

inline const UIDataSourceContainer::DataSourceBaseList & UIDataSourceContainer::GetDataSourceBaseList( void ) const
{
	return *mDataSourceBaseList;
}

// ======================================================================

#endif
