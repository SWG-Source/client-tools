//======================================================================
//
// CuiMenuInfoHelper.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiMenuInfoHelper.h"

#include "UIData.h"
#include "UIDataSource.h"
#include "UIDataSourceContainer.h"
#include "UIPopupMenu.h"
#include "UIRadialMenu.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "sharedGame/RadialMenuManager.h"
#include "sharedNetworkMessages/ObjectMenuRequestData.h"
#include "sharedObject/Object.h"

#include <cstdio>
#include <vector>

//======================================================================

namespace
{
	struct IdComparator
	{
		IdComparator (int id) : m_id (id) {}
		bool operator () (const ObjectMenuRequestData & rhs)
		{
			return (rhs.m_id == m_id);
		}

		int m_id;
	};

	struct MenuItemTypeComparator
	{
		MenuItemTypeComparator (int menuItemType) : m_menuItemType (menuItemType) {}
		bool operator () (const ObjectMenuRequestData & rhs)
		{
			return (rhs.m_menuItemType == m_menuItemType);
		}

		int m_menuItemType;
	};

	//----------------------------------------------------------------------

	void makeLabels (const ObjectMenuRequestData & omrd, Unicode::String & actualLabel, Unicode::String & label, int got, int rootIndex)
	{
		if (!omrd.m_label.empty ())
			actualLabel = StringId::decodeString (omrd.m_label);
		else if (omrd.m_menuItemType < Cui::MenuInfoTypes::CLIENT_MENU_LAST)
			actualLabel = Cui::MenuInfoTypes::getLocalizedLabel (static_cast<Cui::MenuInfoTypes::Type>(omrd.m_menuItemType), got);

		if (!actualLabel.empty ())
		{
			if (rootIndex >= 0)
			{
				char nameBuf     [64];
				snprintf (nameBuf, sizeof (nameBuf), "%d) ", rootIndex);	
				label = Unicode::narrowToWide (nameBuf) + actualLabel;
			}
			else
				label = actualLabel;
		}
	}

	const UILowerString s_prop_rootIndex = UILowerString ("RootIndex");

	const UIColor OUT_OF_RANGE_COLOR(128, 128, 128);
	const UIColor IN_RANGE_COLOR(255, 255, 255);
};

//----------------------------------------------------------------------

const UILowerString CuiMenuInfoHelper::PROP_SERVER_NOTIFY           = UILowerString ("ServerNotify");
const UILowerString CuiMenuInfoHelper::DataProperties::ActualLabel  = UILowerString ("ActualLabel");

//----------------------------------------------------------------------

CuiMenuInfoHelper::CuiMenuInfoHelper () :
m_dataVector (new DataVector),
m_nextItemId (1)
{
}

//----------------------------------------------------------------------

CuiMenuInfoHelper::CuiMenuInfoHelper (const DataVector & dv) :
m_dataVector (new DataVector (dv)),
m_nextItemId (1)
{
}

//----------------------------------------------------------------------

CuiMenuInfoHelper::CuiMenuInfoHelper (const CuiMenuInfoHelper & rhs) :
m_dataVector (new DataVector (*rhs.m_dataVector)),
m_nextItemId (rhs.m_nextItemId)
{
}

//----------------------------------------------------------------------

CuiMenuInfoHelper & CuiMenuInfoHelper::operator= (const CuiMenuInfoHelper & rhs)
{
	*m_dataVector = *rhs.m_dataVector;
	m_nextItemId  = rhs.m_nextItemId;

	return *this;
}

//----------------------------------------------------------------------

void CuiMenuInfoHelper::clear ()
{
	m_dataVector->clear ();
	m_nextItemId = 1;
}

//----------------------------------------------------------------------

CuiMenuInfoHelper::~CuiMenuInfoHelper ()
{
	delete m_dataVector;
	m_dataVector = 0;
}

//----------------------------------------------------------------------

UIDataSource * CuiMenuInfoHelper::addItemToDataSourceContainer (UIDataSourceContainer & dsc, const ObjectMenuRequestData & omrd, int got)
{
	char nameBuf[64];
	_itoa (omrd.m_menuItemType, nameBuf, 10);
	
	UIDataSourceBase * const dsb = safe_cast<UIDataSourceBase *>(dsc.GetChild (nameBuf));
	
	DEBUG_FATAL (dsb && !dsb->IsA (TUIDataSource), ("bad popup"));
	
	UIDataSource * ds = safe_cast<UIDataSource *>(dsb);
	
	if (!ds)
	{
		ds = new UIDataSource;
		ds->SetName (nameBuf);
		dsc.AddChild (ds);
	}
	
	//-- do not overwrite labels with empty strings.
	//-- the client specifies empty strings when sending its menu to the server,
	//-- and the server returns empty strings in this case
	
	Unicode::String       label;
	Unicode::String actualLabel;
	makeLabels (omrd, actualLabel, label, got, -1);
	ds->SetProperty (UIPopupMenu::DataProperties::Text, label);
	ds->SetProperty (DataProperties::ActualLabel,       actualLabel);
	
	//-- update or set the enabled property of the root menu item
	ds->SetPropertyBoolean (UIWidget::PropertyName::Enabled, omrd.isEnabled ());	
	//-- update or set the enabled property of the root menu item
	ds->SetPropertyBoolean (PROP_SERVER_NOTIFY, omrd.isServerNotify ());

	ds->SetPropertyFloat   (UIWidget::PropertyName::Opacity, omrd.isOutOfRange () ? 0.95f : 1.0f);
	if(omrd.isOutOfRange())
		ds->SetPropertyColor (UIWidget::PropertyName::Color, OUT_OF_RANGE_COLOR);
	else
		ds->SetPropertyColor (UIWidget::PropertyName::Color, IN_RANGE_COLOR);

	return ds;
}

//----------------------------------------------------------------------

void CuiMenuInfoHelper::populatePopupDataSourceContainer (UIDataSourceContainer & dsc, const int id, bool putSelf, int got)
{
	UNREF (putSelf);

	if (id && putSelf)
	{
		const ObjectMenuRequestData * self = findItem (id);
		if (!self)
			WARNING (true, ("No self?"));
		else
		{
			addItemToDataSourceContainer (dsc, *self, got);
		}
	}

	DataVector rootItems;
	getChildItems (id, rootItems);
	
	UIBaseObject::UIObjectVector objectsToMoveToBottom;
	{
		
		for (DataVector::const_iterator it = rootItems.begin (); it != rootItems.end (); ++it)
		{
			const ObjectMenuRequestData & omrd = *it;
			
			UIDataSource * const ds = addItemToDataSourceContainer (dsc, omrd, got);
			NOT_NULL (ds);
			
			DataVector childItems;
			getChildItems (omrd.m_id, childItems);
			
			if (!childItems.empty ())
			{
				char nameBuf[64];
				_itoa (omrd.m_menuItemType, nameBuf, 10);
				
				const std::string subName = std::string (nameBuf) + "_subMenu";
				
				ds->SetPropertyNarrow (UIPopupMenu::DataProperties::SubMenu, subName);
				
				UIDataSourceContainer * subDsc = safe_cast<UIDataSourceContainer *>(dsc.GetChild (subName.c_str ()));
				
				if (!subDsc)
				{
					subDsc = new UIDataSourceContainer;
					subDsc->SetName (subName);
					dsc.AddChild   (subDsc);
				}
				else
				{
					//-- always repopulate conversation responses to avoid fuxxoring em up,
					//-- when updated from the client side
					
					if (omrd.m_menuItemType == Cui::MenuInfoTypes::CONVERSE_RESPOND)
					{
						//-- the server is not allowd to modify this menu
						if (omrd.m_label.empty ())
							continue;
						
						subDsc->Clear ();
					}
				}
				
				populatePopupDataSourceContainer (*subDsc, omrd.m_id, true, got);
				
				objectsToMoveToBottom.push_back (subDsc);
			}
		}
	}

	for (UIBaseObject::UIObjectVector::const_iterator it = objectsToMoveToBottom.begin (); it != objectsToMoveToBottom.end (); ++it)
	{
		UIBaseObject * const obj = *it;
		dsc.MoveChild (obj, UIBaseObject::Bottom);
	}

	//-- todo: move all UIDataSourceContainer to the bottom
}

//----------------------------------------------------------------------

void CuiMenuInfoHelper::updatePopupMenu (UIPopupMenu & popup, int got)
{
	UIDataSourceContainer * dsc = popup.GetDataSourceContainer ();

	if (!dsc)
	{
		dsc = new UIDataSourceContainer;
		dsc->SetName ("CuiMenuInfoHelperDataSourceContainer");
		popup.SetDataSourceContainer (dsc);
	}

	populatePopupDataSourceContainer (*dsc, 0, false, got);

	popup.RecreateButtons ();
}

//----------------------------------------------------------------------

void CuiMenuInfoHelper::updateRadialMenu (UIRadialMenu & radial, int got)
{
	UIDataSource * ds = radial.GetDataSource ();

	if (!ds)
	{
		ds = new UIDataSource;
		ds->SetName ("CuiMenuInfoHelperDataSource");
		radial.SetDataSource (ds);
	}

	DataVector rootItems;
	getChildItems (0, rootItems);

	Unicode::String label;
	Unicode::String actualLabel;

	int rootIndex = 1;

	for (DataVector::const_iterator it = rootItems.begin (); it != rootItems.end (); ++it, ++rootIndex)
	{
		const ObjectMenuRequestData & omrd = *it;

		char dataNameBuf [64];
		_itoa (omrd.m_menuItemType, dataNameBuf, 10);

		UIData * data = safe_cast<UIData *>(ds->GetChild (dataNameBuf));

		int actual_rootIndex = 0;

		if (!data)
		{
			data = new UIData;
			data->SetName (dataNameBuf);

			ds->AddChild (data);

			data->SetPropertyInteger (s_prop_rootIndex, rootIndex);
			actual_rootIndex = rootIndex;
		}
		else
		{
			data->GetPropertyInteger (s_prop_rootIndex, actual_rootIndex);
		}
		
		//-- do not overwrite labels with empty strings.
		//-- the client specifies empty strings when sending its menu to the server,
		//-- and the server returns empty strings in this case
		
		label.clear ();
		actualLabel.clear ();

		makeLabels (omrd, actualLabel, label, got,            actual_rootIndex);
		data->SetProperty (UIPopupMenu::DataProperties::Text, label);
		data->SetProperty (DataProperties::ActualLabel,       actualLabel);
 
		//-- update or set the enabled property of the root menu item
		data->SetPropertyBoolean (UIWidget::PropertyName::Enabled, omrd.isEnabled ());

		//-- update or set the enabled property of the root menu item
		data->SetPropertyBoolean (PROP_SERVER_NOTIFY, omrd.isServerNotify ());

		data->SetPropertyFloat   (UIWidget::PropertyName::Opacity, omrd.isOutOfRange () ? 0.95f : 1.0f);
		if(omrd.isOutOfRange())
			data->SetPropertyColor (UIWidget::PropertyName::Color, OUT_OF_RANGE_COLOR);
		else
			data->SetPropertyColor (UIWidget::PropertyName::Color, IN_RANGE_COLOR);

		DataVector childItems;
		getChildItems (omrd.m_id, childItems);

		if (!childItems.empty ())
		{
			UIDataSourceContainer * const popupDataSource = radial.GetPopupDataSourceContainer (dataNameBuf, true);

			//-- GetPopupDataSource should always create a new data source
			NOT_NULL (popupDataSource);
			if (!popupDataSource)
				continue;

			//-- always repopulate conversation responses to avoid fuxxoring em up,
			//-- when updated from the client side

			if (omrd.m_menuItemType == Cui::MenuInfoTypes::CONVERSE_RESPOND)
			{
				//-- the server is not allowd to modify this menu
				if (omrd.m_label.empty ())
					continue;

				popupDataSource->Clear ();
			}

			size_t childIndex = 0;
			for (DataVector::const_iterator cit = childItems.begin (); cit != childItems.end (); ++cit, ++childIndex)
			{
				const ObjectMenuRequestData & child = *cit;

				char childNameBuf[64];

				//-- child responses are always added afresh, so just make sure the names are unique
				if (child.m_menuItemType != Cui::MenuInfoTypes::CONVERSE_RESPONSE || omrd.m_menuItemType != Cui::MenuInfoTypes::CONVERSE_RESPOND)
					_itoa (child.m_menuItemType, childNameBuf, 10);
				else
					_snprintf (childNameBuf, 64, "%d %d", child.m_menuItemType, childIndex);

				//@ add popupdatasourceitem does not add if it already exists...

				UIDataSource * const dataSource = UIRadialMenu::AddPopupDataSourceItem (popupDataSource, childNameBuf, child.m_label, child.isEnabled ());

				NOT_NULL (dataSource);

				if (dataSource)
				{
					dataSource->SetPropertyBoolean (UIWidget::PropertyName::Enabled, child.isEnabled ());

					dataSource->SetPropertyBoolean (PROP_SERVER_NOTIFY, child.isServerNotify ());

					dataSource->SetPropertyFloat   (UIWidget::PropertyName::Opacity, omrd.isOutOfRange () ? 0.95f : 1.0f);
					if(omrd.isOutOfRange())
						dataSource->SetPropertyColor (UIWidget::PropertyName::Color, OUT_OF_RANGE_COLOR);
					else
						dataSource->SetPropertyColor (UIWidget::PropertyName::Color, IN_RANGE_COLOR);

					if (child.m_label.empty () && child.m_menuItemType < Cui::MenuInfoTypes::CLIENT_MENU_LAST)
					{
						label.clear ();
						actualLabel.clear ();

						makeLabels (child, actualLabel, label, got, -1);
						dataSource->SetProperty (UIPopupMenu::DataProperties::Text, label);
						dataSource->SetProperty (DataProperties::ActualLabel,       actualLabel);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void CuiMenuInfoHelper::purgeLabels ()
{
	for (DataVector::iterator it = m_dataVector->begin (); it != m_dataVector->end (); ++it)
		(*it).m_label.clear ();
}

//----------------------------------------------------------------------

void CuiMenuInfoHelper::getChildItems (int parent, DataVector & dv) const
{
	for (DataVector::const_iterator it = m_dataVector->begin (); it != m_dataVector->end (); ++it)
	{
		if ((*it).m_parent == parent)
			dv.push_back (*it);
	}
}

//----------------------------------------------------------------------

const ObjectMenuRequestData * CuiMenuInfoHelper::findItem      (int id) const
{
	for (DataVector::const_iterator it = m_dataVector->begin (); it != m_dataVector->end (); ++it)
	{
		if ((*it).m_id == id)
			return &(*it);
	}

	return 0;
}

//----------------------------------------------------------------------

int CuiMenuInfoHelper::addMenuInternal (int parent, Cui::MenuInfoTypes::Type type, const Unicode::String & label, bool serverNotify)
{
	if (type == Cui::MenuInfoTypes::UNKNOWN)
		return 0;

	//-- multiple CONVERSE_RESPONSE are allowed
	if (type != Cui::MenuInfoTypes::CONVERSE_RESPONSE)
	{
		//-- type already in vector
		if (std::find_if (m_dataVector->begin (), m_dataVector->end (), MenuItemTypeComparator (type)) != m_dataVector->end ())
			return 0;
	}

	//-- parent not present
	if (parent > 0 && std::find_if (m_dataVector->begin (), m_dataVector->end (), IdComparator (parent)) == m_dataVector->end ())
		return 0;

	ObjectMenuRequestData omrd;
	omrd.m_menuItemType = static_cast<uint16>(type);
	omrd.m_label  = label;
	omrd.m_id     = static_cast<uint8>(m_nextItemId);
	omrd.m_parent = static_cast<uint8>(parent);	
	omrd.setServerNotify(serverNotify);

	m_dataVector->push_back (omrd);
	return m_nextItemId++;
}

//----------------------------------------------------------------------

int CuiMenuInfoHelper::addRootMenu (Cui::MenuInfoTypes::Type type, int gameObjectType, bool serverNotify)
{
	return addMenuInternal (0, type, Cui::MenuInfoTypes::getLocalizedLabel (type, gameObjectType), serverNotify); 
}

//----------------------------------------------------------------------

int CuiMenuInfoHelper::addRootMenu (Cui::MenuInfoTypes::Type type, const char * label)
{
	return addMenuInternal (0, type, Unicode::narrowToWide (label));
}

//----------------------------------------------------------------------

int CuiMenuInfoHelper::addRootMenu (Cui::MenuInfoTypes::Type type, const Unicode::String & label, bool serverNotify)
{
	return addMenuInternal (0, type, label, serverNotify);
}

//----------------------------------------------------------------------

int CuiMenuInfoHelper::addSubMenu (int parent, Cui::MenuInfoTypes::Type type,  int gameObjectType)
{
	return addMenuInternal (parent, type, Cui::MenuInfoTypes::getLocalizedLabel (type, gameObjectType));
}

//----------------------------------------------------------------------

int CuiMenuInfoHelper::addSubMenu (int parent, Cui::MenuInfoTypes::Type type, const char * label)
{
	return addMenuInternal (parent, type, Unicode::narrowToWide (label));
}

//----------------------------------------------------------------------

int CuiMenuInfoHelper::addSubMenu (int parent, Cui::MenuInfoTypes::Type type, const Unicode::String & label )
{
	return addMenuInternal (parent, type, label);
}

//----------------------------------------------------------------------

const ObjectMenuRequestData * CuiMenuInfoHelper::getDefault () const
{
	NOT_NULL (m_dataVector);
	return m_dataVector->empty () ? 0 : &(m_dataVector->front ());
}

//----------------------------------------------------------------------

const ObjectMenuRequestData * CuiMenuInfoHelper::findDataByType (int type) const
{
	for (DataVector::const_iterator it = m_dataVector->begin (); it != m_dataVector->end (); ++it)
	{
		const ObjectMenuRequestData & omrd = *it;
		if (omrd.m_menuItemType == type)
			return &omrd;
	}

	return 0;
}

//----------------------------------------------------------------------

bool CuiMenuInfoHelper::updateRanges (float range, const Object & object)
{
	bool found = false;

	for (DataVector::iterator it = m_dataVector->begin (); it != m_dataVector->end (); ++it)
	{
		ObjectMenuRequestData & omrd = *it;
		float acceptableRange = 0.0f;
		if (RadialMenuManager::getRangeForMenuType (omrd.m_menuItemType, acceptableRange))
		{
			if (acceptableRange < range)
			{
				if (!omrd.isOutOfRange ())
				{
					if(object.isInWorld())
						omrd.setOutOfRange (true);
					found = true;
				}
			}
			else
			{
				if (omrd.isOutOfRange ())
				{
					omrd.setOutOfRange (false);
					found = true;
				}
			}
		}
	}

	return found;
}

//======================================================================
