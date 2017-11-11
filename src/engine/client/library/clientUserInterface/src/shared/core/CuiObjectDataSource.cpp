//======================================================================
//
// CuiObjectDataSource.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiObjectDataSource.h"

#include "UIData.h"
#include "UIDataSource.h"
#include "clientUserInterface/CuiObjectDataSourceCallback.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplate.h"

#include <vector>
#include <list>

//======================================================================

const UILowerString CuiObjectDataSource::Properties::ObjectTemplate        = UILowerString ("ObjectTemplate");
const UILowerString CuiObjectDataSource::Properties::AppearanceTemplate    = UILowerString ("AppearanceTemplate");
const UILowerString CuiObjectDataSource::Properties::ObjectId              = UILowerString ("ObjectId");
const UILowerString CuiObjectDataSource::Properties::Position              = UILowerString ("Position");
const UILowerString CuiObjectDataSource::Properties::Yaw                   = UILowerString ("Yaw");
const UILowerString CuiObjectDataSource::Properties::Pitch                 = UILowerString ("Pitch");

//----------------------------------------------------------------------

CuiObjectDataSource::CuiObjectDataSource () :
m_dataSource (0),
m_objectWatcherVector (new ObjectWatcherVector)
{
}

//----------------------------------------------------------------------

CuiObjectDataSource::~CuiObjectDataSource ()
{
	setDataSource (0, 0);
	delete m_objectWatcherVector;
	m_objectWatcherVector = 0;
}

//----------------------------------------------------------------------

void CuiObjectDataSource::setDataSource (UIDataSource * ds, CuiObjectDataSourceCallback * callback)
{
	if (m_dataSource == ds)
		return;

	if (ds)
		ds->Attach (0);

	if (m_dataSource)
	{
		m_dataSource->Detach (0);
	}

	m_dataSource = ds;
	updateList (callback);
}

//----------------------------------------------------------------------

void CuiObjectDataSource::updateList (CuiObjectDataSourceCallback * callback)
{
	if (!m_objectWatcherVector->empty ())
		removeOldObjects (callback);

	if (!m_dataSource)
		return;

	const UIDataList & dlist = m_dataSource->GetData ();

	for (UIDataList::const_iterator it = dlist.begin (); it != dlist.end (); ++it)
	{
		UIData * const data = *it;
		NOT_NULL (data);

		ObjectWatcherData owd (ObjectWatcher (0), true);

		std::string objStr;
		if (data->GetPropertyNarrow (Properties::ObjectId, objStr))
		{
			owd.second = false;

			const NetworkId id (objStr);
			Object * const obj = NetworkIdManager::getObjectById (id);
			if (obj)
			{
				owd.first = obj;
			}
			else
			{
				WARNING (true, ("CuiObjectDataSource object %s does not exist", objStr.c_str ()));
			}
		}
		else if (data->GetPropertyNarrow (Properties::ObjectTemplate, objStr))
		{
			Object * const obj = ObjectTemplate::createObject (objStr.c_str ());
			if (obj)
				owd.first = obj;
			else
				WARNING (true, ("CuiObjectDataSource object template %s does not exist", objStr.c_str ()));
		}
		else if (data->GetPropertyNarrow (Properties::AppearanceTemplate, objStr))
		{
			Appearance * const app = AppearanceTemplateList::createAppearance (objStr.c_str ());

			if (app)
			{
				Object * const obj = new Object;
				obj->setAppearance (app);
				owd.first = obj;
			}
			else
				WARNING (true, ("CuiObjectDataSource appearance template %s does not exist", objStr.c_str ()));
		}
		
		Object * const obj = owd.first.getPointer ();
		
		if (obj)
		{
			Unicode::String posStr;
			if (data->GetProperty (Properties::Position, posStr))
			{
				Vector v;
				if (CuiUtils::ParseVector (posStr, v))
					obj->setPosition_w (v);
				else
					WARNING (true, ("CuiObjectDataSource position [%s] invalid", Unicode::wideToNarrow (posStr).c_str ()));
			}
			
			float yaw = 0.0f;
			if (data->GetPropertyFloat (Properties::Yaw, yaw))
				obj->yaw_o (yaw);
			
			float pitch = 0.0f;
			if (data->GetPropertyFloat (Properties::Pitch, pitch))
				obj->pitch_o (pitch);
		}
		
		m_objectWatcherVector->push_back (owd);
	}

	if (!m_objectWatcherVector->empty ())
		addNewObjects (callback);
}

//----------------------------------------------------------------------

void CuiObjectDataSource::removeOldObjects (CuiObjectDataSourceCallback * callback)
{
	for (ObjectWatcherVector::iterator it = m_objectWatcherVector->begin (); it != m_objectWatcherVector->end (); ++it)
	{
		ObjectWatcherData & owd = *it;
		ObjectWatcher & ow      = owd.first;
		const bool ownsObject   = owd.second;

		Object * const obj = ow.getPointer ();
		if (obj)
		{
			if (callback)
				callback->removeObject (*obj);

			if (ownsObject)
				delete obj;
		}
	}

	m_objectWatcherVector->clear ();
}

//----------------------------------------------------------------------


void CuiObjectDataSource::addNewObjects    (CuiObjectDataSourceCallback * callback)
{
	for (ObjectWatcherVector::iterator it = m_objectWatcherVector->begin (); it != m_objectWatcherVector->end (); ++it)
	{
		ObjectWatcherData & owd = *it;
		ObjectWatcher & ow = owd.first;
		Object * const obj = ow.getPointer ();
		if (obj)
		{
			if (callback)
				callback->addObject (*obj);
		}
	}
}

//======================================================================

