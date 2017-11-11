//======================================================================
//
// CuiObjectDataSource.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiObjectDataSource_H
#define INCLUDED_CuiObjectDataSource_H

//======================================================================

class UIDataSource;
class UILowerString;
class ObjectWatcherList;
class Object;
class CuiObjectDataSourceCallback;

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

//----------------------------------------------------------------------

class CuiObjectDataSource
{
public:

	class Properties
	{
	public:
		static const UILowerString ObjectTemplate;
		static const UILowerString AppearanceTemplate;
		static const UILowerString ObjectId;
		static const UILowerString Position;
		static const UILowerString Yaw;
		static const UILowerString Pitch;
	};

	CuiObjectDataSource ();
	~CuiObjectDataSource ();

	void                 setDataSource (UIDataSource * ds, CuiObjectDataSourceCallback * callback);
	const UIDataSource * getDataSource () const;

private:
	typedef Watcher<Object> ObjectWatcher;
	typedef std::pair<ObjectWatcher, bool> ObjectWatcherData;
	typedef stdvector<ObjectWatcherData>::fwd ObjectWatcherVector;
	

	void                     updateList       (CuiObjectDataSourceCallback * callback);
	void                     removeOldObjects (CuiObjectDataSourceCallback * callback);
	void                     addNewObjects    (CuiObjectDataSourceCallback * callback);

	UIDataSource *           m_dataSource;
	ObjectWatcherVector    * m_objectWatcherVector;
};

//----------------------------------------------------------------------

inline const UIDataSource * CuiObjectDataSource::getDataSource () const
{
	return m_dataSource;
}

//======================================================================

#endif
