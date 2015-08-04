#ifndef __UINAMESPACE_H__
#define __UINAMESPACE_H__

#include "UIBaseObject.h"

#include <list>

class UINamespace : public UIBaseObject
{
public:
	
	static const char       *TypeName;
	
	UINamespace();
	virtual                 ~UINamespace();
	
	virtual bool             IsA( const UITypeID ) const;
	virtual const char      *GetTypeName () const;
	virtual UIBaseObject    *Clone () const;
	
	virtual bool             AddChild( UIBaseObject * );
	virtual bool             RemoveChild( UIBaseObject * );
	virtual UIBaseObject    *GetChild( const char *ChildName ) const;
	virtual void             GetChildren( UIObjectList & ) const;		
	virtual unsigned long    GetChildCount () const;
	virtual UIBaseObject *   GetChildByPositionLinear (int pos);
	const   UIObjectList &     GetChildrenRef () const;

	virtual bool             SetProperty( const UILowerString & Name, const UIString &Value );
	
	virtual bool             CanChildMove( UIBaseObject *, ChildMovementDirection );
	virtual bool             MoveChild( UIBaseObject *, ChildMovementDirection );
	
	virtual void             MinimizeResources ();
	
	virtual void             Link ();
	
	virtual void             ResetLocalizedStrings ();
private:
	
	UIObjectList             mChildren;
};

//-----------------------------------------------------------------

inline const UIBaseObject::UIObjectList & UINamespace::GetChildrenRef () const
{
	return mChildren;
}

//-----------------------------------------------------------------

#endif // __UINAMESPACE_H__