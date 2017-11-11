//======================================================================
//
// SwgCuiCustomize.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCustomize.h"

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
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/Controller.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "swgClientUserInterface/SwgCuiInventoryInfo.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

//======================================================================


namespace SwgCuiCustomizeNamespace
{
}

using namespace SwgCuiCustomizeNamespace;

//----------------------------------------------------------------------

SwgCuiCustomize::SwgCuiCustomize (UIPage & page) :
CuiMediator          ("SwgCuiCustomize", page),
UIEventCallback      (),
m_buttonOK           (0),
m_buttonCancel       (0),
m_buttonClose        (0),
m_callback           (new MessageDispatch::Callback),
m_compositeColor     (0),
m_pageCustom         (0),
m_viewer             (0),
m_objects            (new ObjectWatcherVector),
m_info               (0),
m_sessionEnded       (false),
m_currentCustomizationObject(),
m_currentCustomVarName1(),
m_minVar1(-1),
m_maxVar1(-1),
m_currentCustomVarName2(),
m_minVar2(-1),
m_maxVar2(-1),
m_currentCustomVarName3(),
m_minVar3(-1),
m_maxVar3(-1),
m_currentCustomVarName4(),
m_minVar4(-1),
m_maxVar4(-1)
{
	getCodeDataObject (TUIButton,  m_buttonOK,          "buttonOK");

	getCodeDataObject (TUIButton,    m_buttonCancel,     "buttonCancel");
	getCodeDataObject (TUIComposite, m_compositeColor,   "compositeColor");

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

	registerMediatorObject (*m_buttonOK,        true);
	registerMediatorObject (*m_buttonCancel,    true);

	if(getButtonClose())
	{
		m_buttonClose = getButtonClose();
		registerMediatorObject(*m_buttonClose, true);
	}

	setState   (MS_closeable);
}

//----------------------------------------------------------------------

SwgCuiCustomize::~SwgCuiCustomize ()
{
	m_buttonOK        = 0;
	m_buttonCancel    = 0;
	m_buttonClose     = 0;
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

void SwgCuiCustomize::performActivate   ()
{
	m_sessionEnded = false;

	CuiManager::requestPointer (true);

	m_buttonOK->SetEnabled(true);
	
	m_info->activate ();

	setupCustomizations ();
}

//----------------------------------------------------------------------

void SwgCuiCustomize::performDeactivate ()
{
	CuiManager::requestPointer (false);

	for (int i = 0; i < NumColorPickers; ++i)
	{
		std::string empty;
		m_colorPicker[i]->setTarget(NULL, empty, 0, 0);
	}

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
}

//----------------------------------------------------------------------

void SwgCuiCustomize::OnButtonPressed( UIWidget *context )
{
	if (context == m_buttonOK)
	{

		//Construct string value
		std::string value;
		if(!m_currentCustomVarName1.empty())
		{
			value.append(m_currentCustomVarName1);
			value.append(" ");
			char tmp[256];
			sprintf(tmp, "%d ", m_colorPicker [0]->getValue());
			value.append(tmp);
		}
		if(!m_currentCustomVarName2.empty())
		{
			value.append(m_currentCustomVarName2);
			value.append(" ");
			char tmp[256];
			sprintf(tmp, "%d ", m_colorPicker [1]->getValue());
			value.append(tmp);
		}
		if(!m_currentCustomVarName3.empty())
		{
			value.append(m_currentCustomVarName3);
			value.append(" ");
			char tmp[256];
			sprintf(tmp, "%d ", m_colorPicker [2]->getValue());
			value.append(tmp);
		}
		if(!m_currentCustomVarName4.empty())
		{
			value.append(m_currentCustomVarName4);
			value.append(" ");
			char tmp[256];
			sprintf(tmp, "%d ", m_colorPicker [3]->getValue());
			value.append(tmp);
		}
		Game::getPlayer()->getController()->appendMessage (
			CM_customizeFinished, 
			0.0f, 
			new MessageQueueGenericValueType<std::pair<NetworkId, std::string> >(std::make_pair(m_currentCustomizationObject, value)), 
			GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);

		deactivate();
	}
	if(context == m_buttonCancel || context == m_buttonClose)
	{
		for (int i = 0; i < NumColorPickers; ++i)
		{
			if (m_colorPicker[i])
			{
				m_colorPicker[i]->OnButtonPressed(context);
			}
		}

		Game::getPlayer()->getController()->appendMessage (
			CM_customizeFinished, 
			0.0f, 
			new MessageQueueGenericValueType<std::pair<NetworkId, std::string> >(std::make_pair(m_currentCustomizationObject, std::string())), 
			GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);

		deactivate();
	}
}


//----------------------------------------------------------------------

void SwgCuiCustomize::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);
}

//----------------------------------------------------------------------

void SwgCuiCustomize::setupCustomizations ()
{
	Object * const object = NetworkIdManager::getObjectById (m_currentCustomizationObject);
	if(!object)
		return;

	ClientObject * const objAsClient = object->asClientObject();
	if(!objAsClient)
		return;

	TangibleObject * const objAsTangible = objAsClient->asTangibleObject();
	if(!objAsTangible)
		return;

	CustomizationData * const cdata = objAsTangible->fetchCustomizationData ();
	if(!cdata)
		return;

	m_viewer->clearObjects ();

	DEBUG_FATAL (!m_objects->empty (), ("m_objects not empty"));

	m_info->setInfoObject (object);
	m_viewer->addObject (*object);
	m_viewer->setPaused (false);
	
	m_viewer->setViewDirty             (true);
	m_viewer->setCameraForceTarget     (true);
	m_viewer->recomputeZoom            ();
	m_viewer->setCameraForceTarget     (false);
	m_viewer->setViewDirty             (true);

	{
		for (int i = 0; i < NumColorPickers; ++i)
		{
			m_colorPicker [i]->deactivate ();
			m_colorPicker [i]->setLinkedObjects (*m_objects, true);
		}
	}
	
	
	{
		for (int i = 0; i < NumColorPickers; ++i)
		{
			if (m_colorPicker [i]->isActive ())
				m_colorPicker [i]->setLinkedObjects (*m_objects, true);
		}
	}

	//----------------------------------------------------------------------

	int customizationCount = 0;
	if(m_currentCustomVarName1.empty()) customizationCount = 0;
	else if(m_currentCustomVarName2.empty()) customizationCount = 1;
	else if(m_currentCustomVarName3.empty()) customizationCount = 2;
	else if(m_currentCustomVarName4.empty()) customizationCount = 3;
	else customizationCount = 4;

	int colorCount = 0;

	if (customizationCount)
	{
		for (int i = 0; i < customizationCount && i < NumColorPickers; ++i)
		{
			std::string name;
			int minIndex = 0;
			int index = 0;
			int maxIndex = 0;
			
			{
				int thisMinIndex = 0;
				int thisMaxIndex = 0;

				switch(i)
				{
				case 0:
					name = m_currentCustomVarName1;
					thisMinIndex = m_minVar1;
					thisMaxIndex = m_maxVar1;
					break;
				case 1:
					name = m_currentCustomVarName2;
					thisMinIndex = m_minVar2;
					thisMaxIndex = m_maxVar2;
					break;
				case 2:
					name = m_currentCustomVarName3;
					thisMinIndex = m_minVar3;
					thisMaxIndex = m_maxVar3;
					break;
				case 3:
					name = m_currentCustomVarName4;
					thisMinIndex = m_minVar4;
					thisMaxIndex = m_maxVar4;
					break;
				default:
					break;
				}

				CustomizationVariable * const cvar = cdata->findVariable (name);
				PaletteColorCustomizationVariable * const palColor = dynamic_cast<PaletteColorCustomizationVariable * >(cvar);

				palColor->getRange(minIndex, maxIndex);
				if((thisMinIndex != -1) && (minIndex < thisMinIndex))
					minIndex = thisMinIndex;
				if((thisMaxIndex != -1) && (maxIndex > thisMaxIndex))
					maxIndex = thisMaxIndex;
				index = palColor->getValue();
				if (palColor && colorCount < NumColorPickers)
				{
					palColor->setValue (index);
					CuiColorPicker * const picker = m_colorPicker [colorCount];
					picker->setAutoForceColumns (true);
					picker->setTarget (m_currentCustomizationObject, name, minIndex, maxIndex);
					picker->activate ();
					picker->setLinkedObjects (*m_objects, true);

					++colorCount;
				}
			}
		}
	}

	cdata->release ();

	for (int i = 0; i < NumColorPickers; ++i)
	{
		m_colorPicker [i]->updateCellSizes ();
	}

	m_viewer->setViewDirty             (true);
	m_viewer->setCameraForceTarget     (true);
	m_viewer->recomputeZoom            ();
	m_viewer->setCameraForceTarget     (false);
	m_viewer->setViewDirty             (true);
}

//----------------------------------------------------------------------

void SwgCuiCustomize::setCustomizationObject (const NetworkId &object, const std::string & customVarName1, int minVar1, int maxVar1,
											  const std::string & customVarName2, int minVar2, int maxVar2,
											  const std::string & customVarName3, int minVar3, int maxVar3,
											  const std::string & customVarName4, int minVar4, int maxVar4)
{
	m_currentCustomizationObject = object;
	m_currentCustomVarName1 = customVarName1;
	m_minVar1 = minVar1;
	m_maxVar1 = maxVar1;
	m_currentCustomVarName2 = customVarName2;
	m_minVar2 = minVar2;
	m_maxVar2 = maxVar2;
	m_currentCustomVarName3 = customVarName3;
	m_minVar3 = minVar3;
	m_maxVar3 = maxVar3;
	m_currentCustomVarName4 = customVarName4;
	m_minVar4 = minVar4;
	m_maxVar4 = maxVar4;
	setupCustomizations();
}

//======================================================================
