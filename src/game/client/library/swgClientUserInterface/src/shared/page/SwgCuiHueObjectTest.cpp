//======================================================================
//
// SwgCuiHueObjectTest.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHueObjectTest.h"

#include "UIComposite.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include "clientGame/TangibleObject.h"
#include "clientUserInterface/CuiColorPicker.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"

#include <algorithm>

//======================================================================

SwgCuiHueObjectTest::SwgCuiHueObjectTest (UIPage & page) :
CuiMediator    ("SwgCuiHueObjectTest", page),
m_pickerSample (0),
m_composite    (0),
m_pickers      (new PickerVector),
m_temporaryNetworkId (new NetworkId),
m_maxIndex1(0),
m_maxIndex2(0)
{
	getCodeDataObject (TUIPage,      m_pickerSample, "pickerSample");
	getCodeDataObject (TUIComposite, m_composite,    "composite");

	m_pickerSample->SetVisible (false);

	setState (MS_closeable);
}

//----------------------------------------------------------------------

SwgCuiHueObjectTest::~SwgCuiHueObjectTest ()
{
	clear ();

	delete m_pickers;
	m_composite = 0;
	m_pickers = 0;
	m_pickerSample = 0;

	delete m_temporaryNetworkId;
	m_temporaryNetworkId = 0;
}

//----------------------------------------------------------------------

void SwgCuiHueObjectTest::performActivate   ()
{
}

//----------------------------------------------------------------------

void SwgCuiHueObjectTest::performDeactivate ()
{
	clear ();
}

//----------------------------------------------------------------------

void  SwgCuiHueObjectTest::setObject (ClientObject & obj)
{
	clear ();

	TangibleObject * const tangible = dynamic_cast<TangibleObject *>(&obj);
	
	if (tangible)
	{
		CustomizationData * const cdata = tangible->fetchCustomizationData ();
		if (cdata)
		{
			*m_temporaryNetworkId = tangible->getNetworkId ();
			cdata->iterateOverVariables (customizationIteratorCallback, this);
			cdata->release ();
		}
	}

	*m_temporaryNetworkId = NetworkId::cms_invalid;
}

//----------------------------------------------------------------------

void SwgCuiHueObjectTest::setMax1(int max1)
{
	m_maxIndex1 = max1;
}

//----------------------------------------------------------------------

void SwgCuiHueObjectTest::setMax2(int max2)
{
	m_maxIndex2 = max2;
}

//----------------------------------------------------------------------

void SwgCuiHueObjectTest::clear ()
{
	std::for_each (m_pickers->begin (), m_pickers->end (), CuiMediator::ReleaserFunctor ());
	m_pickers->clear ();
	m_composite->Clear ();
}

//----------------------------------------------------------------------

void SwgCuiHueObjectTest::customizationIteratorCallback (const std::string &fullVariablePathName, CustomizationVariable *customizationVariable, void *context)
{
	PaletteColorCustomizationVariable * const var = dynamic_cast<PaletteColorCustomizationVariable *>(customizationVariable);
	SwgCuiHueObjectTest * const testobj                  = reinterpret_cast<SwgCuiHueObjectTest *>(context);

	if (var)
	{
		UIPage * const dupe = dynamic_cast<UIPage *>(testobj->m_pickerSample->DuplicateObject ());

		if (dupe)
		{
			testobj->m_composite->AddChild (dupe);
			dupe->Link ();

			CuiColorPicker * const picker = new CuiColorPicker (*dupe);
			picker->fetch ();
			testobj->m_pickers->push_back (picker);

			static const long margin = 8L;
			dupe->SetLocation (margin, margin);
			dupe->SetVisible  (true);
			dupe->SetWidth    (testobj->m_composite->GetWidth () - margin * 2L);
			dupe->SetPropertyNarrow  (CuiColorPicker::DataProperties::TargetNetworkId, testobj->m_temporaryNetworkId->getValueString ());
			dupe->SetPropertyNarrow  (CuiColorPicker::DataProperties::TargetVariable,  fullVariablePathName);
			dupe->SetPropertyInteger (CuiColorPicker::DataProperties::TargetRangeMin,       0);
			dupe->SetPropertyInteger (CuiColorPicker::DataProperties::TargetRangeMax,     500);
			picker->activate ();
		}
	}
}

//----------------------------------------------------------------------

SwgCuiHueObjectTest * SwgCuiHueObjectTest::createInto (UIPage & parent)
{
	UIPage * const page = NON_NULL (safe_cast<UIPage *>(parent.GetObjectFromPath ("/script.hueobject", TUIPage)));
	UIPage * const dupe = NON_NULL (safe_cast<UIPage *>(page->DuplicateObject ()));
	IGNORE_RETURN (parent.AddChild (dupe));
	IGNORE_RETURN (parent.MoveChild (dupe, UIBaseObject::Top));
	dupe->Link ();
	dupe->SetLocation (UIPoint::zero);
	
	SwgCuiHueObjectTest * const testobj = new SwgCuiHueObjectTest (*dupe);
	return testobj;
	
}

//======================================================================
