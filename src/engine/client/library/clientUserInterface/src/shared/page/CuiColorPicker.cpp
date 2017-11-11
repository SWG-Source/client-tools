//======================================================================
//
// CuiColorPicker.cpp
// copyright(c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiColorPicker.h"

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
#include "sharedGame/CustomizationManager.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"

#include <list>
#include <map>

//======================================================================

namespace CuiColorPickerNamespace
{
	namespace PathPrefixes
	{
		const std::string shared_owner = "/shared_owner/";
		const std::string priv = "/private/";
	}

	CustomizationVariable * findVariable(CustomizationData & cdata, const std::string & partialName, CuiColorPicker::PathFlags flags)
	{
		CustomizationVariable * cv = cdata.findVariable(partialName);

		if (!cv)
		{
			if ((flags & CuiColorPicker::PF_shared) != 0)
				cv = cdata.findVariable(PathPrefixes::shared_owner + partialName);

			if (!cv &&(flags & CuiColorPicker::PF_private) != 0)
				cv = cdata.findVariable(PathPrefixes::priv + partialName);
		}

		return cv;
	}

//	typedef CuiColorPicker::StringIntMap StringIntMap;
//	StringIntMap s_paletteColumnData;
	std::map<std::string, CustomizationManager::PaletteColumns> s_paletteColumnDataTableData;

	std::string getBasename(const std::string & path)
	{
		std::string basename(path);

		const size_t slashpos = basename.rfind('/');
		const size_t dotpos = basename.rfind('.');

		if (slashpos == std::string::npos) //lint !e650 !e737
			return basename.substr(0, dotpos);
		else if (dotpos == std::string::npos) //lint !e650 !e737
			return basename.substr(slashpos + 1);
		else
			return basename.substr(slashpos + 1,(dotpos - slashpos) - 1);
	}
}

using namespace CuiColorPickerNamespace;

//----------------------------------------------------------------------

const UILowerString CuiColorPicker::Properties::AutoSizePaletteCells = UILowerString("AutoSizePaletteCells");

//----------------------------------------------------------------------

const UILowerString CuiColorPicker::DataProperties::SelectedIndex = UILowerString("SelectedIndex");
const UILowerString CuiColorPicker::DataProperties::TargetNetworkId = UILowerString("TargetNetworkId");
const UILowerString CuiColorPicker::DataProperties::TargetRangeMin = UILowerString("TargetRangeMin");
const UILowerString CuiColorPicker::DataProperties::TargetRangeMax = UILowerString("TargetRangeMax");
const UILowerString CuiColorPicker::DataProperties::TargetValue = UILowerString("TargetValue");
const UILowerString CuiColorPicker::DataProperties::TargetVariable = UILowerString("TargetVariable");


//----------------------------------------------------------------------

CuiColorPicker::CuiColorPicker(UIPage & page) :
CuiMediator("CuiColorPicker", page),
UIEventCallback(),
m_volumePage(0),
m_buttonCancel(0),
m_buttonRevert(0),
m_buttonClose(0),
m_pageSample(0),
m_originalSelection(0),
m_rangeMin(0),
m_rangeMax(0),
m_palette(0),
m_targetObject(new ObjectWatcher),
m_targetVariable(),
m_sampleElement(0),
m_linkedObjects(new ObjectWatcherVector),
m_autoSizePaletteCells(false),
m_text(0),
m_forceColumns(0),
m_autoForceColumns(false),
m_changed(false),
m_lastSize(),
m_paletteSource(PS_target)
{
	getCodeDataObject(TUIVolumePage, m_volumePage, "volumePage");
	getCodeDataObject(TUIButton, m_buttonCancel, "buttonCancel", true);
	getCodeDataObject(TUIButton, m_buttonRevert, "buttonRevert", true);
	getCodeDataObject(TUIPage, m_pageSample, "pageSample", true);
	getCodeDataObject(TUIWidget, m_sampleElement, "sampleElement", true);
	getCodeDataObject(TUIWidget, m_text, "text", true);

	if(getButtonClose())
		m_buttonClose = getButtonClose();
	
	m_volumePage->Clear();

	IGNORE_RETURN(setState(MS_closeable));
}

//----------------------------------------------------------------------

CuiColorPicker::~CuiColorPicker()
{
	revert();
	reset();

	m_volumePage = 0;
	m_buttonCancel = 0;
	m_buttonRevert = 0;
	m_buttonClose = 0;
	m_pageSample = 0;
	m_text = 0;

	if (m_palette)
	{
		m_palette->release();
		m_palette = 0;
	}

	delete m_targetObject;
	m_targetObject = 0;

	delete m_linkedObjects;
	m_linkedObjects = 0;
}

//----------------------------------------------------------------------

void CuiColorPicker::performActivate()
{
	handleMediatorPropertiesChanged();
	setIsUpdating(true);
}

//----------------------------------------------------------------------

void CuiColorPicker::performDeactivate()
{
	setIsUpdating(false);

	if (m_buttonCancel)
		m_buttonCancel->RemoveCallback(this);
	if (m_buttonRevert)
		m_buttonRevert->RemoveCallback(this);

	m_volumePage->RemoveCallback(this);

	storeProperties();

	reset();
}

//----------------------------------------------------------------------

void CuiColorPicker::OnButtonPressed(UIWidget *context)
{
	if (context == m_buttonCancel || context == m_buttonClose)
	{
		revert();
	}
	else if (context == m_buttonRevert)
	{
		revert();
	}
}

//----------------------------------------------------------------------

void CuiColorPicker::OnVolumePageSelectionChanged(UIWidget * context)
{
	if (context == m_volumePage)
	{
		const int index = m_volumePage->GetLastSelectedIndex();

		updateValue(index);

		if (checkAndResetChanged())
			m_userChanged = true;
	}
}

//----------------------------------------------------------------------

void CuiColorPicker::reset()
{
	if (m_palette)
	{
		m_rangeMax = std::min (m_rangeMax, m_palette->getEntryCount ());
		UIBaseObject::UIObjectList olist;
		m_volumePage->GetChildren(olist);

		char buf[64];

		const int num_children = static_cast<int>(olist.size());
		int count = 0;

		{
			UIBaseObject::UIObjectList::iterator it = olist.begin();
			for (int i = m_rangeMin; i < m_rangeMax; ++i, ++count)
			{
				bool error = false;
				const PackedArgb & pargb = m_palette->getEntry(i, error);
				WARNING(error, ("CuiColorPicker reset error"));

				UIWidget * element = 0;

				if (count >= num_children)
				{
					if (m_sampleElement)
						element = safe_cast<UIWidget *>(m_sampleElement->DuplicateObject());
					else
					{
						element = new UIImage;
					}

					NOT_NULL(element);
					m_volumePage->AddChild(element);
				}
				else
					element = dynamic_cast<UIWidget *>(*(it++));

				NOT_NULL (element);

				snprintf (buf, sizeof (buf), "%d", i);

				element->SetName              (buf);
				element->SetGetsInput         (true);
				element->SetBackgroundOpacity (1.0f);
				element->SetBackgroundColor   (UIColor::white);
				element->SetEnabled           (true);
				element->SetGetsInput         (true);
				element->SetVisible           (true);

				const UIColor & tint = CuiUtils::convertColor (pargb);
				element->SetBackgroundTint    (tint);
			}
		}

		if (count < num_children)
		{
			UIBaseObject::UIObjectList::iterator it = olist.begin();
			std::advance(it, count);

			for (; it != olist.end(); ++it)
			{
				m_volumePage->RemoveChild(*it);
			}
		}

		updateCellSizes ();

		m_volumePage->Link ();
	}

	else
		m_volumePage->Clear();
}

//----------------------------------------------------------------------

int CuiColorPicker::getValue() const
{
	TangibleObject * const object = m_targetObject->getPointer();

	if (object)
	{
		CustomizationData * const cdata = object->fetchCustomizationData();
		if (cdata)
		{
			const PaletteColorCustomizationVariable * const var = dynamic_cast<PaletteColorCustomizationVariable *>(findVariable(*cdata, m_targetVariable, PF_any));
			int value = -1;

			if (var)
				value = var->getValue();

			cdata->release();
			return value;
		}
	}
	else if (m_paletteSource == PS_palette)
	{
		return m_volumePage->GetLastSelectedIndex();
	}

	return -1;
}

//----------------------------------------------------------------------

void CuiColorPicker::updateValue(TangibleObject & obj, int index, PathFlags flags)
{
	CustomizationData * const cdata = obj.fetchCustomizationData();
	if (cdata)
	{
		PaletteColorCustomizationVariable * const var = dynamic_cast<PaletteColorCustomizationVariable *>(findVariable(*cdata, m_targetVariable, flags));

		if (var)
		{
			if (var->getValue() != index)
			{
				var->setValue(index);
				m_changed = true;
			}

		}
		cdata->release();
	}

	//store the property so that the subscription can grab it
	char buf [256];
	_itoa(index, buf, 10);
	std::string b = buf;
	getPage().SetProperty(DataProperties::SelectedIndex, Unicode::narrowToWide(b));
} //lint !e1762 //not const

//----------------------------------------------------------------------

void CuiColorPicker::updateValue(int index)
{
	if (index >= 0)
	{
		TangibleObject * const object = m_targetObject->getPointer();

		if (object)
		{
			updateValue(*object, index);
		}
		else if (m_paletteSource == PS_palette)
		{
			m_changed = true;
		}

		for (ObjectWatcherVector::iterator it = m_linkedObjects->begin(); it != m_linkedObjects->end(); ++it)
		{
			TangibleObject * const linked_object = *it;
			if (linked_object)
				updateValue(*linked_object, index, PF_private);
		}

		if (m_palette && m_pageSample)
		{
			bool error = false;
			const PackedArgb & pargb = m_palette->getEntry(index, error);
			WARNING(error, ("CuiColorPicker updateValue error"));
			const UIColor tint(CuiUtils::convertColor(pargb));
			m_pageSample->SetBackgroundTint(tint);
		}

		if (m_buttonRevert)
			m_buttonRevert->SetEnabled(m_originalSelection != index);

		Unicode::String str;
		UIUtils::FormatInteger(str, index);
		getPage().SetProperty(DataProperties::TargetValue, str);

	}
}

//----------------------------------------------------------------------

void CuiColorPicker::revert()
{
	updateValue(m_originalSelection);
}

//----------------------------------------------------------------------

void CuiColorPicker::setTarget(const NetworkId & id, const std::string & var, int rangeMin, int rangeMax)
{
	setTarget(NetworkIdManager::getObjectById(id), var, rangeMin, rangeMax);
}

//----------------------------------------------------------------------

void CuiColorPicker::setTarget(Object * obj, const std::string & var, int rangeMin, int rangeMax)
{
	m_paletteSource = PS_target;

	*m_targetObject = dynamic_cast<TangibleObject *>(obj);


	if (m_palette)
	{
		m_palette->release();
		m_palette = 0;
	}

	int actualRangeMin = 0;
	int actualRangeMax = 0;

	m_targetVariable = var;

	if (!m_targetVariable.empty ())
	{
		TangibleObject * const object = m_targetObject->getPointer ();

		if (object)
		{
			WARNING(rangeMax <= rangeMin,("CuiColorPicker::setTarget invalid range for [%s]:[%s], rangeMin=%d, rangeMax=%d", Unicode::wideToNarrow(object->getLocalizedName()).c_str(), var.c_str(), rangeMin, rangeMax));

			CustomizationData * const cdata = object->fetchCustomizationData ();
			if (cdata)
			{
				PaletteColorCustomizationVariable * const cvar = dynamic_cast<PaletteColorCustomizationVariable *>(findVariable (*cdata, m_targetVariable, PF_any));
				if (cvar)
				{
					m_palette           = cvar->fetchPalette ();
					m_originalSelection = cvar->getValue ();
					cvar->getRange (actualRangeMin, actualRangeMax);
				}
				else
					WARNING (true, ("color picker could not find variable '%s'", m_targetVariable.c_str ()));

				cdata->release ();
			}
		}
	}

	m_rangeMin = std::max (rangeMin, actualRangeMin);
	m_rangeMax = std::min (rangeMax, actualRangeMax);

	storeProperties();

	m_linkedObjects->clear();

	reset();

	m_volumePage->SetSelectionIndex(m_originalSelection);

	updateValue(m_originalSelection);

	const UIWidget * const child = m_volumePage->GetLastSelectedChild();

	if (child)
		m_volumePage->CenterChild(*child);
	else
		m_volumePage->SetScrollLocation(UIPoint(0L, 0L));
}

//----------------------------------------------------------------------

void CuiColorPicker::setLinkedObjects(const ObjectWatcherVector & v, bool doUpdate)
{
	*m_linkedObjects = v;
	const int index = m_volumePage->GetLastSelectedIndex();
	if (doUpdate)
		updateValue(index);
}

//----------------------------------------------------------------------

void CuiColorPicker::setPalette(const PaletteArgb *palette)
{
	// Clean up the previous palette

	if (m_palette)
	{
		m_palette->release();
		m_palette = 0;
	}

	m_paletteSource = PS_palette;

	// Set the new palette

	palette->fetch();
	m_palette = palette;

	if (m_autoForceColumns && palette)
	{
		m_forceColumns = 0;
	}

	m_autoSizePaletteCells = true;

	// What does all this code do?

	if (palette != NULL)
	{
		m_rangeMin = 0;
		m_rangeMax = palette->getEntryCount();
	}

	storeProperties();

	m_linkedObjects->clear();

	reset();

	m_volumePage->SetSelectionIndex(m_originalSelection);

	const UIWidget * const child = m_volumePage->GetLastSelectedChild();

	if (child)
		m_volumePage->CenterChild(*child);
	else
		m_volumePage->SetScrollLocation(UIPoint(0L, 0L));
}

//----------------------------------------------------------------------

PaletteArgb const * CuiColorPicker::getPalette() const
{
	return m_palette;
}

//----------------------------------------------------------------------

void CuiColorPicker::updateCellSizes()
{
	getPage().ForcePackChildren();

	if (m_autoSizePaletteCells && m_palette)
	{
		int const count = std::min(m_rangeMax - m_rangeMin, m_palette->getEntryCount());
		
		if (count) 
		{
			if (m_forceColumns)
			{
				long const width = m_volumePage->GetWidth();
				long const height = m_volumePage->GetHeight();
			
				if (!width || !height)
					return;
				UIPoint cellSize;
				
				int const ny = (count + m_forceColumns - 1) / m_forceColumns;
				
				cellSize.x = width / m_forceColumns;
				if (ny)
					cellSize.y = height / ny;
				else
					cellSize.y = 1;
				
				m_volumePage->SetCellSize(cellSize);
				m_volumePage->SetCellPadding(UISize::zero);
			}
			else
			{
				IGNORE_RETURN(m_volumePage->OptimizeChildSpacing(count));
			}
		}
	}
}

//----------------------------------------------------------------------

void CuiColorPicker::setText(const Unicode::String & str)
{
	if (m_text)
		m_text->SetText (str);
	else
		WARNING(true,("Attempt to set text on CuiColorPicker with no text widget."));
}

//----------------------------------------------------------------------

void CuiColorPicker::setForceColumns(int cols)
{
	m_forceColumns = cols;
}

//----------------------------------------------------------------------

void CuiColorPicker::setAutoForceColumns(bool b)
{
	m_autoForceColumns = b;
	m_forceColumns = 0;
}


//----------------------------------------------------------------------

void CuiColorPicker::setupPaletteColumnData(std::map<std::string, CustomizationManager::PaletteColumns> const & data)
{
	s_paletteColumnDataTableData = data;
}

//----------------------------------------------------------------------

void CuiColorPicker::storeProperties()
{
	getPage().SetPropertyNarrow(DataProperties::TargetVariable, m_targetVariable);
	std::string networkIdString;
	if (*m_targetObject != NULL)
		networkIdString = m_targetObject->getPointer()->getNetworkId().getValueString();

	getPage().SetPropertyNarrow(DataProperties::TargetNetworkId, networkIdString);
	getPage().SetPropertyInteger(DataProperties::TargetRangeMin, m_rangeMin);
	getPage().SetPropertyInteger(DataProperties::TargetRangeMax, m_rangeMax);
}

//----------------------------------------------------------------------

void CuiColorPicker::update(float deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);
	updateCellSizes();
}

//----------------------------------------------------------------------

void CuiColorPicker::setIndex(int index)
{
	m_volumePage->SetSelectionIndex(index);
}

//----------------------------------------------------------------------

void CuiColorPicker::setMaximumPaletteIndex(int const index)
{
	m_rangeMax = index;
}

//----------------------------------------------------------------------

void CuiColorPicker::handleMediatorPropertiesChanged()
{
	std::string str;
	IGNORE_RETURN(getPage().GetPropertyBoolean(Properties::AutoSizePaletteCells, m_autoSizePaletteCells));
	
	if (getPage().GetPropertyNarrow(DataProperties::TargetNetworkId, str))
		*m_targetObject = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(NetworkId(str)));
	else
		*m_targetObject = NULL;

	m_targetVariable.clear();

	getPage ().ForcePackChildren ();
	getPage().GetPropertyNarrow(DataProperties::TargetVariable, m_targetVariable);
	getPage().GetPropertyInteger(DataProperties::TargetRangeMin, m_rangeMin);
	m_rangeMin = std::max(0, m_rangeMin);
	getPage().GetPropertyInteger(DataProperties::TargetRangeMax, m_rangeMax);

	if (m_paletteSource == PS_target)
	{
		setTarget(m_targetObject->getPointer(), m_targetVariable, m_rangeMin, m_rangeMax);
	}
	else if (m_paletteSource == PS_palette)
	{
		setPalette(m_palette);
	}

	if (m_buttonCancel)
		m_buttonCancel->AddCallback(this);
	if (m_buttonRevert)
		m_buttonRevert->AddCallback(this);

	m_volumePage->AddCallback(this);

	m_volumePage->SetSelectionIndex(m_originalSelection);

	m_changed = m_userChanged = false;
	updateCellSizes ();
}

//======================================================================
