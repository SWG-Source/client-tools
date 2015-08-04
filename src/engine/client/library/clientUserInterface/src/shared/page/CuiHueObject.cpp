//======================================================================
//
// CuiHueObject.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiHueObject.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIImage.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UIVolumePage.h"
#include "UnicodeUtils.h"
#include "clientGame/TangibleObject.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"

#include <list>

//======================================================================

namespace
{
	namespace PathPrefixes
	{
		const std::string shared_owner = "/shared_owner/";
		const std::string priv         = "/private/";
	}

	CustomizationVariable * findVariable (CustomizationData & cdata, const std::string & partialName, CuiHueObject::PathFlags flags)
	{		
		CustomizationVariable * cv = 0;
		
//		cv = cdata.findVariable (partialName);
		
		if (!cv)
		{
			if ((flags & CuiHueObject::PF_shared) != 0)
				cv = cdata.findVariable (PathPrefixes::shared_owner + partialName);
			
			if (!cv && (flags & CuiHueObject::PF_private) != 0)
				cv = cdata.findVariable (PathPrefixes::priv + partialName);
		}
		
		return cv;
	}		
	
	typedef CuiHueObject::StringIntMap StringIntMap;
	StringIntMap s_paletteColumnData;
	
	std::string getBasename (const std::string & path)
	{
		std::string basename (path);
		
		const size_t slashpos = basename.rfind ('/');
		const size_t dotpos   = basename.rfind ('.');
		
		if (slashpos == std::string::npos)
			return basename.substr (0, dotpos);
		else if (dotpos == std::string::npos)
			return basename.substr (slashpos + 1);
		else
			return basename.substr (slashpos + 1, dotpos - slashpos - 1);
	}
}

//----------------------------------------------------------------------

const UILowerString CuiHueObject::Properties::AutoSizePaletteCells = UILowerString ("AutoSizePaletteCells");

//----------------------------------------------------------------------

const UILowerString CuiHueObject::DataProperties::SelectedIndex   = UILowerString ("SelectedIndex");
const UILowerString CuiHueObject::DataProperties::TargetNetworkId = UILowerString ("TargetNetworkId");
const UILowerString CuiHueObject::DataProperties::TargetRange     = UILowerString ("TargetRange");
const UILowerString CuiHueObject::DataProperties::TargetValue     = UILowerString ("TargetValue");
const UILowerString CuiHueObject::DataProperties::TargetVariable  = UILowerString ("TargetVariable");

//----------------------------------------------------------------------

CuiHueObject::CuiHueObject (UIPage & page) :
CuiMediator            ("CuiHueObject", page),
UIEventCallback        (),
m_volumePage           (0),
m_buttonCancel         (0),
m_buttonRevert         (0),
m_originalSelection    (0),
m_range                (0),
m_palette              (0),
m_targetObject         (new CachedNetworkId),
m_targetVariable       (),
m_linkedObjects        (new CachedNetworkIdVector),
m_autoSizePaletteCells (false),
m_forceColumns         (0),
m_autoForceColumns     (false)
{
	getCodeDataObject (TUIVolumePage, m_volumePage,    "volume1.volume");
	getCodeDataObject (TUIButton,     m_buttonCancel,  "btnCancel",  true);
	getCodeDataObject (TUIButton,     m_buttonRevert,  "btnRevert",  true);

	IGNORE_RETURN (getPage ().GetPropertyBoolean (Properties::AutoSizePaletteCells, m_autoSizePaletteCells));
}

//----------------------------------------------------------------------

CuiHueObject::~CuiHueObject ()
{
	if (m_palette)
	{
		m_palette->release ();
		m_palette = 0;
	}

	reset ();

	m_volumePage        = 0;
	m_buttonCancel      = 0;
	m_buttonRevert      = 0;
//	m_pageSample        = 0;
//	m_text              = 0;

	delete m_targetObject;
	m_targetObject = 0;

	delete m_linkedObjects;
	m_linkedObjects = 0;
}

//----------------------------------------------------------------------

void CuiHueObject::performActivate   ()
{
	std::string str;
	if (getPage ().GetPropertyNarrow (DataProperties::TargetNetworkId, str))
		*m_targetObject = NetworkId (str);
	else
		*m_targetObject = NetworkId::cms_invalid;
	
	m_targetVariable.clear ();

	getPage ().GetPropertyNarrow (DataProperties::TargetVariable, m_targetVariable);
	getPage ().GetPropertyInteger (DataProperties::TargetRange, m_range);

	setTarget (*m_targetObject, m_maxIndex1, m_maxIndex2);

	if (m_buttonCancel)
		m_buttonCancel->AddCallback (this);
	if (m_buttonRevert)
		m_buttonRevert->AddCallback (this);

	m_volumePage->AddCallback (this);

	m_volumePage->SetSelectionIndex (m_originalSelection);
}

//----------------------------------------------------------------------

void CuiHueObject::performDeactivate ()
{

	if (m_buttonCancel)
		m_buttonCancel->RemoveCallback (this);
	if (m_buttonRevert)
		m_buttonRevert->RemoveCallback (this);

	m_volumePage->RemoveCallback (this);

	*m_targetObject = NetworkId::cms_invalid;

	getPage ().SetProperty (DataProperties::TargetVariable,  Unicode::narrowToWide (m_targetVariable));
	getPage ().SetProperty (DataProperties::TargetNetworkId, Unicode::narrowToWide (m_targetObject->getValueString ()));
	Unicode::String rangeStr;
	UIUtils::FormatInteger (rangeStr, m_range);
	getPage ().SetProperty (DataProperties::TargetRange,     rangeStr);

	if (m_palette)
	{
		m_palette->release ();
		m_palette = 0;
	}

	reset ();
}

//----------------------------------------------------------------------

void CuiHueObject::OnButtonPressed (UIWidget *context)
{
	if (context == m_buttonCancel)
	{
		revert ();
	}
	else if (context == m_buttonRevert)
	{
		revert ();
	}
}

//----------------------------------------------------------------------

void CuiHueObject::OnVolumePageSelectionChanged (UIWidget * context)
{
	if (context == m_volumePage)
	{
		const int index = m_volumePage->GetLastSelectedIndex ();
		
		updateValue (index);
	}
}

//----------------------------------------------------------------------

void CuiHueObject::reset ()
{
	if (m_palette)
	{
		const size_t count = std::min (m_range, m_palette->getEntryCount ());

		UIBaseObject::UIObjectList olist;
		m_volumePage->GetChildren (olist);
		const size_t num_children = olist.size ();
/*
		{
			UIBaseObject::UIObjectList::iterator it = olist.begin ();
			for (size_t i = 0; i < count; ++i)	
			{
				const PackedArgb     & pargb = m_palette->getEntry (i);
				
				UIWidget * element = 0;
				
				if (i >= num_children)
				{
					if (m_sampleElement)
						element = dynamic_cast<UIWidget *>(m_sampleElement->DuplicateObject ());
					else
					{
						element = new UIImage;
						element->SetGetsInput         (true);
						element->SetBackgroundOpacity (1.0f);
						element->SetBackgroundColor   (UIColor::white);
					}

					NOT_NULL (element);
					m_volumePage->AddChild (element);
				}
				else
					element = dynamic_cast<UIWidget *>(*(it++));
				
				NOT_NULL (element);
				const UIColor tint (CuiUtils::convertColor (pargb));
				element->SetBackgroundTint  (tint);
				
				element->SetEnabled    (true);
				element->SetGetsInput  (true);
				element->SetVisible    (true);
			}
		}
*/
		if (count < num_children)
		{
			UIBaseObject::UIObjectList::iterator it = olist.begin ();
			std::advance (it, count);

			for (; it != olist.end (); ++it)
			{
				m_volumePage->RemoveChild (*it);
			}
		}
		
		updateCellSizes ();

		m_volumePage->Link ();
	}
	
	else
		m_volumePage->Clear ();
}

//----------------------------------------------------------------------

int CuiHueObject::getValue ()
{
	TangibleObject * const object = safe_cast<TangibleObject *>(m_targetObject->getObject ());
	
	if (object)
	{
		CustomizationData * const cdata = object->fetchCustomizationData ();
		if (cdata)
		{
			PaletteColorCustomizationVariable * const var = dynamic_cast<PaletteColorCustomizationVariable *>(findVariable (*cdata, m_targetVariable, PF_any));
			
			if (var)
			{
				return var->getValue ();
			}
		}
	}

	return -1;
}

//----------------------------------------------------------------------

void CuiHueObject::updateValue (TangibleObject & obj, int index, PathFlags flags)
{
	CustomizationData * const cdata = obj.fetchCustomizationData ();
	if (cdata)
	{
		PaletteColorCustomizationVariable * const var = dynamic_cast<PaletteColorCustomizationVariable *>(findVariable (*cdata, m_targetVariable, flags));
		
		if (var)
		{
			var->setValue (index);
		}
		cdata->release ();
	}
}

//----------------------------------------------------------------------

void CuiHueObject::updateValue (int index)
{
	if (index >= 0)
	{
		TangibleObject * const object = safe_cast<TangibleObject *>(m_targetObject->getObject ());
		
		if (object)
			updateValue (*object, index);

		for (CachedNetworkIdVector::iterator it = m_linkedObjects->begin (); it != m_linkedObjects->end (); ++it)
		{
			CachedNetworkId & id = *it;
			TangibleObject * const linked_object = dynamic_cast<TangibleObject *>(id.getObject ());
			if (linked_object)
				updateValue (*linked_object, index, PF_private);
		}

/*
		if (m_palette && m_pageSample)
		{
			const PackedArgb     & pargb = m_palette->getEntry (index);
			const UIColor tint (CuiUtils::convertColor (pargb));
			m_pageSample->SetBackgroundTint (tint);
		}
*/

		if (m_buttonRevert)
			m_buttonRevert->SetEnabled (m_originalSelection != index);

		Unicode::String str;
		UIUtils::FormatInteger  (str, index);
		getPage ().SetProperty (DataProperties::TargetValue, str);

	}
}

//----------------------------------------------------------------------

void CuiHueObject::revert ()
{
	updateValue (m_originalSelection);
}

//----------------------------------------------------------------------

void CuiHueObject::setTarget (const NetworkId & id, int maxIndex1, int maxIndex2)
{
	*m_targetObject = CachedNetworkId (id);
	m_maxIndex1 = maxIndex1;
	m_maxIndex2 = maxIndex2;
/*	
	if (m_palette)
	{
		m_palette->release ();
		m_palette = 0;
	}

	int maxRange = 0;
	m_targetVariable = var;

	if (!m_targetVariable.empty ())
	{
		TangibleObject * const object = safe_cast<TangibleObject *>(m_targetObject->getObject ());
		
		if (object)
		{
			CustomizationData * const cdata = object->fetchCustomizationData ();
			if (cdata)
			{
				PaletteColorCustomizationVariable * const var = dynamic_cast<PaletteColorCustomizationVariable *>(findVariable (*cdata, m_targetVariable, PF_any));
				if (var)
				{
					m_palette           = var->fetchPalette ();
					m_originalSelection = var->getValue ();
					int dummy = 0;
					var->getRange (dummy, maxRange);

					//-- update force columns
					if (m_autoForceColumns && m_palette)
					{
						const std::string paletteName (getBasename (m_palette->getName ().getString ()));
						
						const StringIntMap::const_iterator it = s_paletteColumnData.find (paletteName);
						
						if (it != s_paletteColumnData.end ())
							m_forceColumns = (*it).second;
						else
							m_forceColumns = 0;
					}
				}
				else
					WARNING (true, ("color picker could not find variable '%s'", m_targetVariable.c_str ()));

				cdata->release ();
			}
		}
	}

	m_range = std::min (maxRange, range);

	getPage ().SetProperty (DataProperties::TargetNetworkId, Unicode::narrowToWide (id.getValueString ()));
	getPage ().SetProperty (DataProperties::TargetVariable,  Unicode::narrowToWide (m_targetVariable));

	UIString tmp;
	UIUtils::FormatInteger (tmp, m_range);
	getPage ().SetProperty (DataProperties::TargetRange, tmp);

	m_linkedObjects->clear ();

	reset ();

	m_volumePage->SetSelectionIndex (m_originalSelection);

	const UIWidget * const child = m_volumePage->GetLastSelectedChild ();

	if (child)
		m_volumePage->CenterChild (*child);
	else
		m_volumePage->SetScrollLocation (UIPoint (0L, 0L));
*/
}

/*
void CuiHueObject::setTarget (const NetworkId & id, const std::string & var, int range)
{
	*m_targetObject = CachedNetworkId (id);
	
	if (m_palette)
	{
		m_palette->release ();
		m_palette = 0;
	}

	int maxRange = 0;
	m_targetVariable = var;

	if (!m_targetVariable.empty ())
	{
		TangibleObject * const object = safe_cast<TangibleObject *>(m_targetObject->getObject ());
		
		if (object)
		{
			CustomizationData * const cdata = object->fetchCustomizationData ();
			if (cdata)
			{
				PaletteColorCustomizationVariable * const var = dynamic_cast<PaletteColorCustomizationVariable *>(findVariable (*cdata, m_targetVariable, PF_any));
				if (var)
				{
					m_palette           = var->fetchPalette ();
					m_originalSelection = var->getValue ();
					int dummy = 0;
					var->getRange (dummy, maxRange);

					//-- update force columns
					if (m_autoForceColumns && m_palette)
					{
						const std::string paletteName (getBasename (m_palette->getName ().getString ()));
						
						const StringIntMap::const_iterator it = s_paletteColumnData.find (paletteName);
						
						if (it != s_paletteColumnData.end ())
							m_forceColumns = (*it).second;
						else
							m_forceColumns = 0;
					}
				}
				else
					WARNING (true, ("color picker could not find variable '%s'", m_targetVariable.c_str ()));

				cdata->release ();
			}
		}
	}

	m_range = std::min (maxRange, range);

	getPage ().SetProperty (DataProperties::TargetNetworkId, Unicode::narrowToWide (id.getValueString ()));
	getPage ().SetProperty (DataProperties::TargetVariable,  Unicode::narrowToWide (m_targetVariable));

	UIString tmp;
	UIUtils::FormatInteger (tmp, m_range);
	getPage ().SetProperty (DataProperties::TargetRange, tmp);

	m_linkedObjects->clear ();

	reset ();

	m_volumePage->SetSelectionIndex (m_originalSelection);

	const UIWidget * const child = m_volumePage->GetLastSelectedChild ();

	if (child)
		m_volumePage->CenterChild (*child);
	else
		m_volumePage->SetScrollLocation (UIPoint (0L, 0L));
}
*/
//----------------------------------------------------------------------

void CuiHueObject::setLinkedObjects (const CachedNetworkIdVector & v)
{
	*m_linkedObjects = v;
}

//----------------------------------------------------------------------

void CuiHueObject::updateCellSizes ()
{
	if (m_autoSizePaletteCells && m_palette)
	{
		const size_t count = std::min (m_range, m_palette->getEntryCount ());
		
		//-- auto size the palette cells if possible
		if (count)
		{
			getPage ().ForcePackChildren ();

			const long width  = m_volumePage->GetWidth ();
			const long height = m_volumePage->GetHeight ();

			if (!width || !height)
				return;
			
			UIPoint cellSize;

			if (m_forceColumns)
			{
				const int ny = (count + m_forceColumns - 1) / m_forceColumns;

				cellSize.x = width / m_forceColumns;
				if (ny)
					cellSize.y = height / ny;
			}
			else
			{
				const int ny = static_cast<int>(ceil (sqrt (static_cast<float>(count * height) / static_cast<float>(width))));

				const int nx = (count + ny - 1) / ny;

				if (nx)
					cellSize.x = width / nx;

				if (ny)
					cellSize.y = height / ny;
			}

			m_volumePage->SetCellSize (cellSize);
			m_volumePage->SetCellPadding (UISize::zero);

		}
	}
}

//----------------------------------------------------------------------

void CuiHueObject::setText (const Unicode::String &)
{
/*
	if (m_text)
		m_text->SetText (str);
	else
		WARNING (true, ("Attempt to set text on CuiHueObject with no text widget."));
*/
}

//----------------------------------------------------------------------

void CuiHueObject::setForceColumns              (int cols)
{
	m_forceColumns = cols;
}

//----------------------------------------------------------------------

void CuiHueObject::setAutoForceColumns          (bool b)
{
	m_autoForceColumns = b;
	m_forceColumns = 0;
}

//----------------------------------------------------------------------

void CuiHueObject::setupPaletteColumnData (const UIDataSource & ds)
{
	s_paletteColumnData.clear ();

	const UIDataList & dataList = ds.GetData ();

	for (UIDataList::const_iterator it = dataList.begin (); it != dataList.end (); ++it)
	{
		const UIData * data = NON_NULL (*it);

		static const UILowerString prop_columns      = UILowerString ("columns");

		std::string paletteName = data->GetName ();

		int columns = 0;
		if (!data->GetPropertyInteger (prop_columns, columns))
			WARNING (true, ("No property found for %s", prop_columns));

		s_paletteColumnData [paletteName] = columns;
	}
}

//----------------------------------------------------------------------

CuiHueObject * CuiHueObject::createInto (UIPage & parent)
{
	UIPage * const page = NON_NULL (safe_cast<UIPage *>(parent.GetObjectFromPath ("/script.hueobject", TUIPage)));
	UIPage * const dupe = NON_NULL (safe_cast<UIPage *>(page->DuplicateObject ()));
	IGNORE_RETURN (parent.AddChild (dupe));
	IGNORE_RETURN (parent.MoveChild (dupe, UIBaseObject::Top));
	dupe->Link ();
	dupe->SetLocation (UIPoint::zero);
	
	CuiHueObject * const test = new CuiHueObject (*dupe);
	return test;
	
}

//======================================================================
