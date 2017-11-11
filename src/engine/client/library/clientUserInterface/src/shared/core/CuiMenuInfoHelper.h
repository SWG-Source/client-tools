//======================================================================
//
// CuiMenuInfoHelper.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiMenuInfoHelper_H
#define INCLUDED_CuiMenuInfoHelper_H

//======================================================================

class UIDataSource;
class UIDataSourceContainer;
class UILowerString;
class UIPopupMenu;
class UIRadialMenu;
class Object;
struct ObjectMenuRequestData;

#include "clientUserInterface/CuiMenuInfoTypes.h"

//----------------------------------------------------------------------

class CuiMenuInfoHelper
{
public:

	static const UILowerString PROP_SERVER_NOTIFY;

	struct DataProperties
	{
		static const UILowerString ActualLabel;
	};

	typedef stdvector<ObjectMenuRequestData>::fwd DataVector;

	                   CuiMenuInfoHelper (const CuiMenuInfoHelper & rhs);
	CuiMenuInfoHelper & operator= (const CuiMenuInfoHelper & rhs);
	                   CuiMenuInfoHelper ();
	                   CuiMenuInfoHelper (const DataVector & dv);
	                  ~CuiMenuInfoHelper ();

	void               updateRadialMenu           (UIRadialMenu & radial, int got);
	void               updatePopupMenu            (UIPopupMenu &  popup, int got);

	const DataVector & getDataVector () const;

	int                addRootMenu  (Cui::MenuInfoTypes::Type type, int gameObjectType, bool serverNotify = false);
	int                addRootMenu  (Cui::MenuInfoTypes::Type type, const char * label);
	int                addRootMenu  (Cui::MenuInfoTypes::Type type, const Unicode::String & label, bool serverNotify = false);
	int                addSubMenu   (int parent, Cui::MenuInfoTypes::Type type, int gameObjectType);
	int                addSubMenu   (int parent, Cui::MenuInfoTypes::Type type, const char * label);
	int                addSubMenu   (int parent, Cui::MenuInfoTypes::Type type, const Unicode::String & label);

	bool               updateRanges (float range, const Object & object);

	//-- use with care
	void               purgeLabels ();

	const ObjectMenuRequestData * getDefault     () const;
	const ObjectMenuRequestData * findDataByType (int type) const;

	void               clear ();

private:

	int                           addMenuInternal (int parent, Cui::MenuInfoTypes::Type type, const Unicode::String & label, bool serverNotify = false);
	void                          getChildItems (int parent, DataVector & dv) const;
	const ObjectMenuRequestData * findItem      (int id) const ;
	void                          populatePopupDataSourceContainer (UIDataSourceContainer & dsc, const int id, bool putSelf, int got);
	UIDataSource *                addItemToDataSourceContainer     (UIDataSourceContainer & dsc, const ObjectMenuRequestData & omrd, int got);

	DataVector *                  m_dataVector;
	int                           m_nextItemId;
};

//----------------------------------------------------------------------

inline const CuiMenuInfoHelper::DataVector & CuiMenuInfoHelper::getDataVector () const
{
	NOT_NULL (m_dataVector);
	return *m_dataVector;
}

//======================================================================

#endif
