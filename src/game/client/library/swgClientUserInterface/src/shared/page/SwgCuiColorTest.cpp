//======================================================================
//
// SwgCuiColorTest.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiColorTest.h"

#include "UIComposite.h"
#include "UIData.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include "clientGame/TangibleObject.h"
#include "clientUserInterface/CuiColorPicker.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "sharedFoundation/Watcher.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include <algorithm>

//======================================================================

namespace SwgCuiColorTestNamespace
{
	namespace Properties
	{
		const UILowerString VariablePath ("VariablePath");
	}
}

using namespace SwgCuiColorTestNamespace;

//----------------------------------------------------------------------

SwgCuiColorTest::SwgCuiColorTest (UIPage & page) :
CuiMediator     ("SwgCuiColorTest", page),
m_pickerSample  (0),
m_sliderSample  (0),
m_composite     (0),
m_textCaption   (0),
m_pickers       (new PickerVector),
m_sliders       (new SliderbarVector),
m_target        (new ObjectWatcher)
{
	getCodeDataObject (TUIPage,      m_pickerSample, "pickerSample");
	getCodeDataObject (TUISliderbar, m_sliderSample, "sliderSample");
	getCodeDataObject (TUIComposite, m_composite,    "composite");
	getCodeDataObject (TUIText,      m_textCaption,  "textCaption");

	m_pickerSample->SetVisible (false);
	m_sliderSample->SetVisible (false);

	m_textCaption->SetPreLocalized (true);

	setState (MS_closeable);
}

//----------------------------------------------------------------------

SwgCuiColorTest::~SwgCuiColorTest ()
{
	clear ();

	m_sliderSample = 0;

	delete m_pickers;
	m_composite = 0;
	m_pickers = 0;
	m_pickerSample = 0;
	m_sliderSample = 0;

	delete m_sliders;
	m_sliders = 0;

	delete m_target;
	m_target = 0;
}

//----------------------------------------------------------------------

void SwgCuiColorTest::performActivate   ()
{
	TangibleObject * const tangible = dynamic_cast<TangibleObject *>(m_target->getPointer ());

	if (tangible)
		setObject (*tangible);
}

//----------------------------------------------------------------------

void SwgCuiColorTest::performDeactivate ()
{
	clear ();
}

//----------------------------------------------------------------------

void  SwgCuiColorTest::setObject (ClientObject & obj)
{
	clear ();

	TangibleObject * const tangible = dynamic_cast<TangibleObject *>(&obj);
	
	m_textCaption->Clear ();

	if (tangible)
	{
		char buf [1024];
		snprintf (buf, sizeof (buf), "%s [%s]", tangible->getNetworkId ().getValueString ().c_str (), tangible->getObjectTemplateName ());
		m_textCaption->SetLocalText (Unicode::narrowToWide (buf));

		*m_target = tangible;

		CustomizationData * const cdata = tangible->fetchCustomizationData ();
		if (cdata)
		{
			cdata->iterateOverVariables (customizationIteratorCallback, this);
			cdata->release ();
		}
	}
	else
		*m_target = 0;
}

//----------------------------------------------------------------------

void SwgCuiColorTest::clear ()
{
	std::for_each (m_pickers->begin (), m_pickers->end (), CuiMediator::ReleaserFunctor ());
	m_pickers->clear ();
	m_composite->Clear ();

	for (SliderbarVector::const_iterator it = m_sliders->begin (); it != m_sliders->end (); ++it)
	{
		UISliderbar * const slider = *it;
		slider->RemoveCallback (this);
		slider->Detach (0);
	}
	m_sliders->clear ();
}

//----------------------------------------------------------------------

void SwgCuiColorTest::customizationIteratorCallback (const std::string &fullVariablePathName, CustomizationVariable *customizationVariable, void *context)
{
	PaletteColorCustomizationVariable * const colorvar = dynamic_cast<PaletteColorCustomizationVariable *>(customizationVariable);
	SwgCuiColorTest * const test                       = reinterpret_cast<SwgCuiColorTest *>(context);

	TangibleObject * const tangible = dynamic_cast<TangibleObject *>(test->m_target->getPointer ());
	if (!tangible)
		return;

	if (colorvar)
	{
		UIPage * const dupe = safe_cast<UIPage *>(test->m_pickerSample->DuplicateObject ());

		if (dupe)
		{
			test->m_composite->AddChild (dupe);
			dupe->Link ();

			CuiColorPicker * const picker = new CuiColorPicker (*dupe);
			picker->fetch ();
			test->m_pickers->push_back (picker);

			static const long margin = 8L;
			dupe->SetLocation (margin, margin);
			dupe->SetVisible  (true);
			dupe->SetWidth    (test->m_composite->GetWidth () - margin * 2L);
//			dupe->SetPropertyNarrow  (CuiColorPicker::DataProperties::TargetNetworkId,    tangible->getNetworkId ().getValueString ());
			dupe->SetPropertyNarrow  (CuiColorPicker::DataProperties::TargetVariable,     fullVariablePathName);
			dupe->SetPropertyInteger (CuiColorPicker::DataProperties::TargetRangeMax,     500);
			dupe->SetPropertyInteger (CuiColorPicker::DataProperties::TargetRangeMin,       0);
			picker->activate ();
			picker->setTarget (tangible, fullVariablePathName, 0, 500);

		}

		return;
	}

	RangedIntCustomizationVariable * const intvar = dynamic_cast<RangedIntCustomizationVariable *>(customizationVariable);

	if (intvar)
	{
		UISliderbar * const dupe = safe_cast<UISliderbar *>(test->m_sliderSample->DuplicateObject ());

		if (dupe)
		{
			test->m_composite->AddChild (dupe);
			dupe->SetVisible (true);
			dupe->Link ();

			dupe->SetPropertyNarrow (Properties::VariablePath, fullVariablePathName);

			int minRangeInclusive = 0;
			int maxRangeExclusive = 0;

			intvar->getRange(minRangeInclusive, maxRangeExclusive);

			dupe->SetLowerLimit (minRangeInclusive);
			dupe->SetUpperLimit (maxRangeExclusive - 1);
			dupe->SetValue      (intvar->getValue (), true);

			dupe->AddCallback (test);
		}
	}
}

//----------------------------------------------------------------------

SwgCuiColorTest * SwgCuiColorTest::createInto (UIPage & parent)
{
	UIPage * const page = NON_NULL (safe_cast<UIPage *>(parent.GetObjectFromPath ("/pda.colortest", TUIPage)));
	UIPage * const dupe = NON_NULL (safe_cast<UIPage *>(page->DuplicateObject ()));
	IGNORE_RETURN (parent.AddChild (dupe));
	IGNORE_RETURN (parent.MoveChild (dupe, UIBaseObject::Top));
	dupe->Link ();
//	dupe->SetSize (parent->GetSize () * 2 / 3);
	dupe->SetLocation (UIPoint::zero);
	
	SwgCuiColorTest * const test = new SwgCuiColorTest (*dupe);
	return test;
	
}

//----------------------------------------------------------------------

void SwgCuiColorTest::OnSliderbarChanged( UIWidget * context)
{
	TangibleObject * const tangible = dynamic_cast<TangibleObject *>(m_target->getPointer ());
	
	if (tangible)
	{
		UISliderbar * const slider = safe_cast<UISliderbar *>(context);
		
		std::string variablePath;
		if (slider->GetPropertyNarrow (Properties::VariablePath, variablePath))
		{
			CustomizationData * const cdata = tangible->fetchCustomizationData ();
			if (cdata)
			{
				RangedIntCustomizationVariable * const intvar = dynamic_cast<RangedIntCustomizationVariable *>(cdata->findVariable (variablePath));
				
				if (intvar)
					intvar->setValue (slider->GetValue ());

				cdata->release ();
			}
		}
	}
}

//======================================================================
