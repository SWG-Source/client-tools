#ifndef __UIDATASOURCE_H__
#define __UIDATASOURCE_H__

class UIData;
class UiMemoryBlockManager;

#include "UIDataSourceBase.h"
typedef ui_stdlist<UIData *>::fwd UIDataList;

//======================================================================================

class UIDataSource : public UIDataSourceBase
{
public:

	static const char      *TypeName;

	                        UIDataSource();
	virtual                ~UIDataSource();

	virtual bool            IsA( const UITypeID ) const;
	virtual const char     *GetTypeName( void ) const;
	virtual UIBaseObject   *Clone( void ) const;

	virtual bool            AddChild( UIBaseObject * );
	virtual bool            RemoveChild( UIBaseObject * );
	virtual UIBaseObject   *GetChild( const char * ) const;
	virtual void            GetChildren( UIObjectList & ) const;
	virtual unsigned long   GetChildCount( void ) const;
	virtual bool            CanChildMove( UIBaseObject *, ChildMovementDirection );
	virtual bool            MoveChild( UIBaseObject *, ChildMovementDirection );

//	virtual void            Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );

	        void            Clear( void );
	        bool            RemoveChildByGUID( long );
	        UIData         *GetChildByGUID( long theGUID ) const;
	        UIData         *GetChildByPosition( unsigned long thePosition ) const;
	        bool            SetChildPositionByGUID( long GUID, long Position );
	        void            AddChildByPosition( UIData *ChildToAdd, long thePosition );

	        void            UpdateChild( UIData *, const UIData * );
	        UIDataList &    GetData ();
	const   UIDataList &    GetData () const;
	virtual void            ResetLocalizedStrings ();

	virtual bool            SetProperty( const UILowerString & Name, const UIString &Value );

private:

	UIDataSource (const UIDataSource & rhs);
	UIDataSource & operator= (const UIDataSource & rhs);

	UIDataList *            mData;
};

//======================================================================================

inline UIDataList & UIDataSource::GetData ()
{
	return *mData;
}

//-----------------------------------------------------------------

inline const UIDataList & UIDataSource::GetData () const
{
	return *mData;
}

//-----------------------------------------------------------------

#endif // __UIDATASOURCE_H__