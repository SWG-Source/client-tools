//======================================================================
//
// SwgCuiCraftCustomize.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCraftCustomize.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "UITextbox.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include "clientGame/Game.h"
#include "clientGame/ManufactureSchematicObject.h"
#include "clientGame/TangibleObject.h"
#include "clientUserInterface/CuiColorPicker.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsCraft.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "swgClientUserInterface/SwgCuiInventoryInfo.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

//======================================================================


namespace SwgCuiCraftCustomizeNamespace
{
}

using namespace SwgCuiCraftCustomizeNamespace;

//----------------------------------------------------------------------

SwgCuiCraftCustomize::SwgCuiCraftCustomize (UIPage & page) :
CuiMediator          ("SwgCuiCraftCustomize", page),
UIEventCallback      (),
m_buttonNext         (0),
m_buttonPractice     (0),
m_buttonCancel       (0),
m_buttonReset        (0),
m_transition         (false),
m_callback           (new MessageDispatch::Callback),
m_textboxName        (0),
m_textboxLimit       (0),
m_wasPrototype       (false),
m_comboAppearance    (0),
m_sliderLimit        (0),
m_compositeColor     (0),
m_pageCustom         (0),
m_viewer             (0),
m_checkPractice      (0),
m_objects            (new ObjectWatcherVector),
m_ignoreTextboxLimit (false),
m_practice           (false),
m_info               (0),
m_sessionEnded       (false)
{
	getCodeDataObject (TUIButton,  m_buttonNext,        "buttonNext");
	getCodeDataObject (TUIButton,  m_buttonPractice,    "buttonPractice");
	getCodeDataObject (TUITextbox, m_textboxName,       "textboxName");
	getCodeDataObject (TUITextbox, m_textboxLimit,      "textboxLimit");

	getCodeDataObject (TUIButton,    m_buttonCancel,     "buttonCancel");
	getCodeDataObject (TUIButton,    m_buttonReset,      "buttonReset");
	getCodeDataObject (TUIComboBox,  m_comboAppearance,  "comboAppearance");
	getCodeDataObject (TUISliderbar, m_sliderLimit,      "sliderLimit");
	getCodeDataObject (TUIComposite, m_compositeColor,   "compositeColor");
	getCodeDataObject (TUICheckbox,  m_checkPractice,    "checkPractice");

	getCodeDataObject (TUIPage, m_pageCustom, "pageCustom");

	for (int i = 0; i < NumColorPickers; ++i)
	{
		char buf [128];
		snprintf (buf, sizeof (buf), "colorPicker%d", i);
		UIPage * page = 0;
		getCodeDataObject (TUIPage, page,      buf);
		m_colorPicker [i] = new CuiColorPicker (*page);
		m_colorPicker [i]->fetch ();
	}

	{
		UIWidget * widget = 0;
		getCodeDataObject (TUIWidget, widget,      "viewer");
		m_viewer = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(widget));
		m_viewer->SetLocalTooltip (CuiStringIds::tooltip_viewer_3d_controls.localize ());
		m_viewer->setAutoZoomOutOnly       (false);
		m_viewer->setCameraZoomInWhileTurn (false);
		m_viewer->setCameraForceTarget     (false);
		m_viewer->setCameraLodBias         (3.0f);
		m_viewer->setCameraLodBiasOverride (true);
	}

	{
		UIPage * page = 0;
		getCodeDataObject (TUIPage, page,      "info");
		m_info = new SwgCuiInventoryInfo (*page);
		m_info->fetch ();
	}

	registerMediatorObject (*m_buttonNext,      true);
	registerMediatorObject (*m_buttonPractice,  true);
	registerMediatorObject (*m_comboAppearance, true);
	registerMediatorObject (*m_sliderLimit,     true);
	registerMediatorObject (*m_textboxLimit,    true);

	m_textboxLimit->SetNumericInteger  (true);
	m_textboxLimit->SetNumericNegative (true);

	setState   (MS_closeable);
}

//----------------------------------------------------------------------

SwgCuiCraftCustomize::~SwgCuiCraftCustomize ()
{
	m_textboxName     = 0;
	m_textboxLimit    = 0;
	m_buttonNext      = 0;
	m_buttonPractice  = 0;
	m_buttonCancel    = 0;
	m_buttonReset     = 0;
	m_comboAppearance = 0;
	m_sliderLimit     = 0;
	m_compositeColor  = 0;

	for (int i = 0; i < NumColorPickers; ++i)
	{
		m_colorPicker [i]->release ();
		m_colorPicker [i]    = 0;
	}

	m_info->release ();
	m_info            = 0;

	m_pageCustom = 0;
	m_viewer     = 0;

	delete m_callback;
	m_callback    = 0;

	delete m_objects;
	m_objects = 0;
}

//----------------------------------------------------------------------

void SwgCuiCraftCustomize::performActivate   ()
{
	m_sessionEnded = false;

	if (!CuiCraftManager::isCrafting () && !Game::getSinglePlayer ())
	{
		onSessionEnded (true);
		return;
	}

	CuiManager::requestPointer (true);

	m_buttonPractice->SetEnabled(true);
	m_buttonNext->SetEnabled(true);

	const CuiCraftManager::FinalState fs = CuiCraftManager::getFinalState ();

	m_textboxLimit->SetVisible (fs == CuiCraftManager::FS_schem);

	m_checkPractice->SetChecked (!m_practice);
	m_checkPractice->SetChecked (m_practice);

	m_checkPractice->SetVisible (fs == CuiCraftManager::FS_proto);
	if (fs == CuiCraftManager::FS_schem)
	{
		m_buttonNext->SetVisible (true);
		m_buttonPractice->SetVisible (false);
	}
	
	Unicode::String numStr;

	const int manf_limit = 1000;
	UIUtils::FormatInteger (numStr, manf_limit);
	m_textboxLimit->SetLocalText (numStr);
	m_sliderLimit->SetLowerLimit (0);
	m_sliderLimit->SetUpperLimit (manf_limit);
	m_sliderLimit->SetValue      (manf_limit, false);

	m_transition = false;

	m_info->activate ();

	setupCustomizations ();

	m_callback->connect (*this, &SwgCuiCraftCustomize::onComplete,         static_cast<CuiCraftManager::Messages::Complete*>     (0));
	m_callback->connect (*this, &SwgCuiCraftCustomize::onSessionEnded,     static_cast<CuiCraftManager::Messages::SessionEnded *>     (0));
}

//----------------------------------------------------------------------

void SwgCuiCraftCustomize::performDeactivate ()
{
	m_practice = m_checkPractice->IsChecked ();

	CuiManager::requestPointer (false);

	for (ObjectWatcherVector::iterator it = m_objects->begin (); it != m_objects->end (); ++it)
	{
		Object * const obj = (*it);
		if (obj)
			delete obj;
	}

	m_objects->clear ();

	m_viewer->clearObjects ();

	m_viewer->setPaused (true);

	setIsUpdating (false);

	m_callback->disconnect (*this, &SwgCuiCraftCustomize::onComplete,         static_cast<CuiCraftManager::Messages::Complete*>     (0));
	m_callback->disconnect (*this, &SwgCuiCraftCustomize::onSessionEnded,     static_cast<CuiCraftManager::Messages::SessionEnded *>     (0));
}

//----------------------------------------------------------------------

void SwgCuiCraftCustomize::OnButtonPressed( UIWidget *context )
{
	if (context == m_buttonPractice || context == m_buttonNext)
	{
//		m_buttonPractice->SetEnabled(false);
//		m_buttonNext->SetEnabled(false);
		bool practice = false;
		if (context == m_buttonPractice)
			practice = true;

		const Unicode::String & str = m_textboxName->GetLocalText ();

		//prevent players from putting '@' as the first character of the name
		//so it is not interpreted as a code string
		int i = 0;
		int nameLength = str.length ();
		while(i < nameLength && str[i] == '@')
		{
			++i;
		}

		Unicode::String customName = str.substr (i, nameLength);

		const int limit = m_textboxLimit->GetNumericIntegerValue ();

		if (limit <= 0 || limit > 1000)
		{
			CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_cust_bad_limit.localize ());
			return;
		}
		
		storeCustomizationInfo (customName, limit);

		CuiCraftManager::complete (false, practice);
	}
}

//----------------------------------------------------------------------

void  SwgCuiCraftCustomize::OnGenericSelectionChanged (UIWidget * context)
{
	if (context == m_comboAppearance)
	{
		const int selectedIndex = m_comboAppearance->GetSelectedIndex ();

		if (selectedIndex < 0)
		{
			WARNING (true, ("wtf"));
		}
		else
		{
			m_viewer->clearObjects ();
			
			if (selectedIndex == 0)
			{
				TangibleObject * const proto = dynamic_cast<TangibleObject *>(CuiCraftManager::getSchematicPrototype ());
				if (!proto)
					WARNING (true, ("SwgCuiCraftCustomize proto was deleted while craft customizing"));
				else
				{
					m_viewer->addObject (*proto);
					m_textboxName->SetLocalText (proto->getLocalizedName ());
				}
			}
			else
			{
				if (selectedIndex > static_cast<int>(m_objects->size ()))
					WARNING (true, ("bad index"));
				else
				{
					TangibleObject * const obj = (*m_objects) [selectedIndex - 1];
					if (!obj)
						WARNING (true, ("bad obj"));
					else
					{
						m_viewer->addObject (*obj);
						m_textboxName->SetLocalText (obj->getLocalizedName());
					}
				}
			}

			m_viewer->setViewDirty             (true);
			m_viewer->setCameraForceTarget     (true);
			m_viewer->recomputeZoom            ();
			m_viewer->setCameraForceTarget     (false);
			m_viewer->setViewDirty             (true);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftCustomize::OnSliderbarChanged        (UIWidget * context)
{
	if (context == m_sliderLimit)
	{
		m_ignoreTextboxLimit = true;

		const long value = m_sliderLimit->GetValue ();
		static Unicode::String str;
		UIUtils::FormatLong (str, value);
		m_textboxLimit->SetLocalText (str);
		
		m_ignoreTextboxLimit = false;
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftCustomize::OnTextboxChanged          (UIWidget * context)
{
	if (context == m_textboxLimit)
	{
		if (m_ignoreTextboxLimit)
			return;
		
		const int value = m_textboxLimit->GetNumericIntegerValue ();
		
		m_sliderLimit->SetValue (value, true);
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftCustomize::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (m_transition)
	{
		setIsUpdating (false);
		m_transition = false;
		deactivate ();
		CuiCraftManager::stopCrafting (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftCustomize::onComplete (const CuiCraftManager::Messages::Complete::Payload & wasPrototype)
{
	m_transition    = true;
	m_wasPrototype  = wasPrototype;
	setIsUpdating (true);
	m_viewer->clearObjects ();
	m_viewer->setPaused (true);
}

//----------------------------------------------------------------------

void SwgCuiCraftCustomize::setupCustomizations ()
{
	ManufactureSchematicObject * const manf_schem = CuiCraftManager::getManufactureSchematic ();
	TangibleObject * const proto = dynamic_cast<TangibleObject *>(CuiCraftManager::getSchematicPrototype ());

	m_viewer->clearObjects ();
	UIDataSource * const ds = m_comboAppearance->GetDataSource ();
	NOT_NULL (ds);
	ds->Clear ();

	DEBUG_FATAL (!m_objects->empty (), ("m_objects not empty"));

	if (!manf_schem || !proto)
	{
		m_textboxName->SetLocalText (Unicode::emptyString);
		m_pageCustom->SetVisible (false);
		return;
	}

	m_info->setInfoObject (proto);
	m_viewer->addObject (*proto);
	m_viewer->setPaused (false);
	
	m_viewer->setViewDirty             (true);
	m_viewer->setCameraForceTarget     (true);
	m_viewer->recomputeZoom            ();
	m_viewer->setCameraForceTarget     (false);
	m_viewer->setViewDirty             (true);

	m_textboxName->SetLocalText (proto->getLocalizedName());

	{
		for (int i = 0; i < NumColorPickers; ++i)
		{
			m_colorPicker [i]->deactivate ();
			m_colorPicker [i]->setLinkedObjects (*m_objects, true);
		}
	}
	
	//----------------------------------------------------------------------

	const int appCount = manf_schem->getCustomAppearanceCount   ();

	const Appearance * const currentApp = NON_NULL (proto->getAppearance ());
	const std::string currentAppName = NON_NULL (currentApp->getAppearanceTemplateName ());

	int currentIndex = 0;

	if (appCount)
	{
		UIData * const d = new UIData;
		d->SetProperty (UITextbox::PropertyName::Text, proto->getLocalizedName ());
		ds->AddChild (d);

		for (int i = 0; i < appCount; ++i)
		{
			std::string appName;
			if (manf_schem->getCustomAppearanceName    (i, appName))
			{
				if (!_stricmp (currentAppName.c_str (), appName.c_str ()))
					currentIndex = i;

				UIData * const d = new UIData;

				ClientObject * clientObject = safe_cast<ClientObject *>(ObjectTemplate::createObject (appName.c_str ()));
				if (!clientObject)
				{
					d->SetPropertyNarrow (UITextbox::PropertyName::Text, appName);
					clientObject = safe_cast<ClientObject *>(proto->getObjectTemplate ()->createObject ());
					NOT_NULL (clientObject);
				}
				else
				{
					d->SetProperty (UITextbox::PropertyName::Text, clientObject->getLocalizedName ());
				}

				if (clientObject)
					clientObject->endBaselines ();

				m_objects->push_back (Watcher<TangibleObject>(dynamic_cast<TangibleObject *>(clientObject)));
				ds->AddChild (d);
			}
		}

		ds->Link ();
		m_comboAppearance->SetVisible (true);
		m_comboAppearance->SetSelectedIndex (0);
		
		{
			for (int i = 0; i < NumColorPickers; ++i)
			{
				if (m_colorPicker [i]->isActive ())
					m_colorPicker [i]->setLinkedObjects (*m_objects, true);
			}
		}
	}
	else
	{
		m_comboAppearance->SetVisible (false);
	}

	//----------------------------------------------------------------------

	const int customizationCount = manf_schem->getCustomizationCount ();
	int colorCount = 0;

	if (customizationCount)
	{
		CustomizationData * const cdata = proto->fetchCustomizationData ();
		
		if (!cdata)
			WARNING (true, ("no cdata"));
		else
		{
			for (int i = 0; i < customizationCount && i < NumColorPickers; ++i)
			{
				std::string name;
				int minIndex = 0;
				int index = 0;
				int maxIndex = 0;
				if (manf_schem->getCustomizationInfo (i, name, minIndex, index, maxIndex))
				{
					CustomizationVariable * const cvar = cdata->findVariable (name);
					PaletteColorCustomizationVariable * const palColor = dynamic_cast<PaletteColorCustomizationVariable * >(cvar);

					if (palColor && colorCount < NumColorPickers)
					{
						palColor->setValue (index);
						CuiColorPicker * const picker = m_colorPicker [colorCount];
						picker->setAutoForceColumns (true);
						picker->setTarget (proto->getNetworkId (), name, minIndex, maxIndex);
						picker->activate ();
						picker->setLinkedObjects (*m_objects, true);

						++colorCount;
					}
				}
			}

			cdata->release ();
		}
	}

	m_pageCustom->ForcePackChildren ();

	for (int i = 0; i < NumColorPickers; ++i)
	{
		m_colorPicker [i]->updateCellSizes ();
	}

	m_viewer->setViewDirty             (true);
	m_viewer->setCameraForceTarget     (true);
	m_viewer->recomputeZoom            ();
	m_viewer->setCameraForceTarget     (false);
	m_viewer->setViewDirty             (true);

	if (colorCount == 0 && appCount < 2)
		m_pageCustom->SetVisible (false);
	else
		m_pageCustom->SetVisible (true);
}

//----------------------------------------------------------------------

void SwgCuiCraftCustomize::storeCustomizationInfo (const Unicode::String & name, int limit)
{
	ManufactureSchematicObject * const manf_schem = CuiCraftManager::getManufactureSchematic ();
	if (!manf_schem)
	{
		WARNING (true, ("error, no manf schem"));
		return;
	}
		
	manf_schem->setObjectName (name);
	manf_schem->setCount (limit);

	for (int i = 0; i < NumColorPickers; ++i)
	{
		if (m_colorPicker [i]->isActive ())
		{
			const std::string & targetVariable = m_colorPicker [i]->getTargetVariable ();
			const int value   = m_colorPicker [i]->getValue ();
			manf_schem->setCustomizationInfo (targetVariable, value);
		}
		else
			break;
	}

	if (m_comboAppearance->IsVisible ())
	{
		const int selectedIndex = m_comboAppearance->GetSelectedIndex ();

		//- negative value indicates the default appearance
		manf_schem->setCustomizationAppearance (selectedIndex - 1);	
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftCustomize::onSessionEnded (const bool &)
{
	m_sessionEnded = true;
	closeNextFrame ();
}

//----------------------------------------------------------------------

bool SwgCuiCraftCustomize::close ()
{
	CuiCraftManager::stopCrafting (m_sessionEnded);
	return true;
}

//======================================================================
