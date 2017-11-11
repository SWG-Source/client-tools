//======================================================================
//
// SwgCuiIncubator.cpp
// copyright (c) 2007 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiIncubator.h"

#include "clientGame/ClientIncubatorManager.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientUserInterface/CuiIconManagerCallback.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/IncubatorCommitMessage.h"
#include "sharedObject/Container.h"
#include "sharedObject/Controller.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "swgClientUserInterface/SwgCuiContainerProviderDefault.h"
#include "swgClientUserInterface/SwgCuiContainerProviderFilter.h"
#include "swgClientUserInterface/SwgCuiInventoryContainer.h"
#include "swgClientUserInterface/SwgCuiInventoryInfo.h"


#include "UIButton.h"
#include "UICheckbox.h"
#include "UIImage.h"
#include "UIMessage.h"
#include "UIText.h"
#include "UIVolumePage.h"

// ----------------------------------------------------------


class SwgCuiIncubator::ContainerFilter : 
public SwgCuiInventoryContainerFilter
{
public:

	bool overrideMessage (const UIWidget *, const UIMessage & msg) const
	{
		if (msg.Type == UIMessage::DragEnd || msg.Type == UIMessage::DragOver)
		{
//			return true;
		}

		return false;
	}

	class IconCallback : public CuiIconManagerCallback
	{
	public:
		void overrideTooltip (const ClientObject & obj, Unicode::String & str) const
		{
			Unicode::String suffix;
			SwgCuiIncubator::createObjectTooltipString (obj, suffix);
			
			if (!suffix.empty ())
			{
				str.push_back ('\n');
				str.append (suffix);
			}
		}

		bool overrideDoubleClick (const UIWidget & viewer) const
		{
			return NON_NULL (m_incubator)->overrideDoubleClick (viewer);
		}

		SwgCuiIncubator * m_incubator;

		IconCallback () : m_incubator (0) {}
	};

	ContainerFilter () :
	SwgCuiInventoryContainerFilter (new IconCallback)
	{
	}
	
	~ContainerFilter ()
	{
		if (iconCallback)
			delete iconCallback;
	}
};


//----------------------------------------------------------------------

class SwgCuiIncubator::ProviderFilter : public SwgCuiContainerProviderFilter
{
public:

	bool showObject      (const ClientObject & obj) const
	{
		if (m_incubator->m_checkFilter->IsChecked ())
		{
			if (obj.getGameObjectType () == SharedObjectTemplate::GOT_misc_container || 
				obj.getGameObjectType () == SharedObjectTemplate::GOT_misc_container_wearable)
			{
				//Show a container if it contains anything we can use
				const Container* c = ContainerInterface::getContainer(obj);
				if (c)
				{
					for (ContainerConstIterator i = c->begin(); i != c->end(); ++i)
					{
						Container::ContainedItem item = *i;
						ClientObject const * const o = safe_cast<ClientObject const *>(item.getObject());
						if (o && showObject(*o))
							return true;
					}
					return false;
				}
			}
			else if(obj.getGameObjectType() == SharedObjectTemplate::GOT_misc_enzyme ||
				obj.getGameObjectType() == SharedObjectTemplate::GOT_misc_enzyme_hydrolase ||
				obj.getGameObjectType() == SharedObjectTemplate::GOT_misc_enzyme_isomerase ||
				obj.getGameObjectType() == SharedObjectTemplate::GOT_misc_enzyme_lyase )
			{
				return true;
			}

			return false;
		}
		else
			return true;
	}

	SwgCuiIncubator * m_incubator;
};

namespace SwgCuiIncubatorNamespace
{
	static const int ms_numCheckboxes = 10;
	static const int ms_maxNutrientTemperatureGaugeValue = 10; // 0 -> 10
	static const int ms_middleNutrientTemperatureGaugeValue = 5;
	static const char * ms_survivalCodeDataString = "checkbox_survival%d";
	static const char * ms_beastialResilienceCodeDataString = "checkbox_beastialresilience%d";
	static const char * ms_cunningCodeDataString = "checkbox_cunning%d";
	static const char * ms_intelligenceCodeDataString = "checkbox_intelligence%d";
	static const char * ms_aggressionCodeDataString = "checkbox_aggression%d";
	static const char * ms_huntersInstinctCodeDataString = "checkbox_huntersinstinct%d";

	static std::string ms_attribNameSurvival = "survival";
	static std::string ms_attribNameBeastialResilience = "beastialresilience";
	static std::string ms_attribNameCunning = "cunning";
	static std::string ms_attribNameIntelligence = "intelligence";
	static std::string ms_attribNameAggression = "aggression";
	static std::string ms_attribNameHuntersInstinct = "huntersinstinct";

	static std::string ms_enzymePointsAttribName = "enzyme_purity";

	static const int ms_numSlots = 4;
	static const char * ms_slotViewerDataString = "enzyme%dviewer";

	std::string convertServerObjectTemplateNameToSharedObjectTemplateName (std::string const & serverObjectTemplateName)
	{
		size_t const sharedpos = serverObjectTemplateName.rfind ('/');
		if (sharedpos != std::string::npos)
		{
			return serverObjectTemplateName.substr (0, sharedpos + 1) + "shared_" + serverObjectTemplateName.substr (sharedpos + 1);
		}

		WARNING (true, ("bad template name"));
		return std::string ();
	}
};

using namespace  SwgCuiIncubatorNamespace;



//----------------------------------------------------------------------

SwgCuiIncubator::SwgCuiIncubator (UIPage & page) :
CuiMediator                     ("SwgCuiIncubator", page),
UIEventCallback                 (),
m_callback                      (new MessageDispatch::Callback),
m_terminalId(),
m_buttonCancel(NULL),
m_buttonCommit(NULL),
m_pointsDefensiveSkills(NULL),
m_pointsIntelligenceSkills(NULL),
m_pointsOffensiveSkills(NULL),
m_powerBar(NULL),
m_heatBar(NULL),
m_heatBarBack(NULL),
m_nutrientBar(NULL),
m_nutrientBarBack(NULL),
m_buttonAddHeat(NULL),
m_buttonSubtractHeat(NULL),
m_buttonAddNutrients(NULL),
m_buttonSubtractNutrients(NULL),
m_viewer(NULL),
m_creatureObject(NULL),
m_inventoryInfo(NULL),
m_containerInventory(NULL),
m_containerProviderInventory(new SwgCuiContainerProviderDefault),
m_containerFilter(new ContainerFilter),
m_providerFilter(new ProviderFilter),
m_checkFilter(NULL),
m_pointsToSpend(0),
m_newCreatureColorIndex(0),
m_committed(false)
{
	IGNORE_RETURN(setState(MS_closeable));
	safe_cast<ContainerFilter::IconCallback *>(m_containerFilter->iconCallback)->m_incubator = this;
	m_providerFilter->m_incubator = this;
	m_containerProviderInventory->setFilter (m_providerFilter);

	getCodeDataObject(TUIButton, m_buttonCancel, "buttonCancel");
	getCodeDataObject(TUIButton, m_buttonCommit, "buttonCommit");
	getCodeDataObject(TUIText, m_pointsDefensiveSkills, "pointsDefensiveSkills");
	getCodeDataObject(TUIText, m_pointsIntelligenceSkills, "pointsIntelligenceSkills");
	getCodeDataObject(TUIText, m_pointsOffensiveSkills, "pointsOffensiveSkills");
	getCodeDataObject(TUIImage, m_powerBar, "powerBar");
	getCodeDataObject(TUIPage, m_heatBar, "heatBar");
	getCodeDataObject(TUIPage, m_heatBarBack, "heatBarBack");
	getCodeDataObject(TUIPage, m_nutrientBar, "nutrientBar");
	getCodeDataObject(TUIPage, m_nutrientBarBack, "nutrientBarBack");
	getCodeDataObject(TUIButton, m_buttonAddHeat, "buttonAddHeat");
	getCodeDataObject(TUIButton, m_buttonSubtractHeat, "buttonSubtractHeat");
	getCodeDataObject(TUIButton, m_buttonAddNutrients, "buttonAddNutrients");
	getCodeDataObject(TUIButton, m_buttonSubtractNutrients, "buttonSubtractNutrients");
	getCodeDataObject(TUICheckbox,   m_checkFilter,    "checkFilter");
	
	m_checkFilter->SetChecked         (true);

	// viewer setup
	UIWidget * viewerWidget = 0;
	getCodeDataObject (TUIWidget, viewerWidget,     "viewer");
	m_viewer = safe_cast<CuiWidget3dObjectListViewer *>(viewerWidget);

	if (m_viewer)
	{
		m_viewer->setCameraLodBias (2.0f);
		m_viewer->setAutoZoomOutOnly       (false);
		m_viewer->setCameraZoomInWhileTurn (false);
		m_viewer->setAlterObjects          (true);
		m_viewer->setCameraLookAtCenter    (true);
		m_viewer->setDragYawOk             (true);
		m_viewer->setPaused                (false);
		m_viewer->SetDragable              (false);	
		m_viewer->SetContextCapable        (true, false);
		m_viewer->setRotateSpeed           (1.0f);
		m_viewer->setCameraForceTarget     (false);
		m_viewer->setCameraTransformToObj  (true);
		m_viewer->setCameraLodBias         (3.0f);
		m_viewer->setCameraLodBiasOverride (true);
	}
	// end viewer setup

	// checkboxes
	int i;
	char buff[512];
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		sprintf(buff, ms_survivalCodeDataString,i);
		getCodeDataObject(TUICheckbox, m_checkboxSurvival[i], buff );
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		sprintf(buff, ms_beastialResilienceCodeDataString, i);
		getCodeDataObject(TUICheckbox, m_checkboxBeastialResilience[i], buff );
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		sprintf(buff, ms_cunningCodeDataString, i);
		getCodeDataObject(TUICheckbox, m_checkboxCunning[i], buff );
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		sprintf(buff, ms_intelligenceCodeDataString, i);
		getCodeDataObject(TUICheckbox, m_checkboxIntelligence[i], buff);
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		sprintf(buff, ms_aggressionCodeDataString, i);
		getCodeDataObject(TUICheckbox, m_checkboxAggression[i], buff);
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		sprintf(buff, ms_huntersInstinctCodeDataString, i);
		getCodeDataObject(TUICheckbox, m_checkboxHuntersInstinct[i], buff);
	}

	{				
		UIPage * infoPage = 0;
		getCodeDataObject (TUIPage, infoPage, "inventoryInfo");
		m_inventoryInfo      = new SwgCuiInventoryInfo (*infoPage);
		m_inventoryInfo->fetch ();
		
		{
			UIPage * containerPage = 0;
			getCodeDataObject (TUIPage, containerPage, "containerInventory");
			m_containerInventory = new SwgCuiInventoryContainer (*containerPage, SwgCuiInventoryContainer::T_craftIngredients);
			m_containerInventory->setInventoryType(SwgCuiInventory::IT_CRAFTING);
			m_containerInventory->fetch ();			
			m_containerInventory->setContainerProvider (m_containerProviderInventory);
		}

		//-- connect the info mediator to selection changes in the container mediator
		m_inventoryInfo->connectToSelectionTransceiver (m_containerInventory->getTransceiverSelection ());

	}

	// slot setup
	for (i = 0; i < ms_numSlots; ++i)
	{
		sprintf(buff,ms_slotViewerDataString,i+1);
		UIWidget * slotViewerWidget = 0;
		getCodeDataObject (TUIWidget, slotViewerWidget, buff);
		m_slotViewer[i] = safe_cast<CuiWidget3dObjectListViewer *> (slotViewerWidget);
		m_slotViewer[i]->setCameraLookAtCenter (true);
		m_slotViewer[i]->setPaused (false);
		m_slotViewer[i]->setCameraForceTarget (true);
		m_slotViewer[i]->SetEnabled(true);
		m_slotViewer[i]->SetActivated(true);
		m_slotViewer[i]->SetGetsInput(true);
		m_slotViewer[i]->SetDragable(false);

	}


 // ..........

	m_containerInventory->setFilter (m_containerFilter);

	registerMediatorObject (*m_buttonCancel, true);
	registerMediatorObject (*m_buttonCommit, true);
	registerMediatorObject (*m_buttonAddHeat, true);
	registerMediatorObject (*m_buttonSubtractHeat, true);
	registerMediatorObject (*m_buttonAddNutrients, true);
	registerMediatorObject (*m_buttonSubtractNutrients, true);
	registerMediatorObject (*m_checkFilter, true);
	
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		registerMediatorObject(*(m_checkboxSurvival[i]), true);
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		registerMediatorObject(*(m_checkboxBeastialResilience[i]), true);
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		registerMediatorObject(*(m_checkboxCunning[i]),true);
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		registerMediatorObject(*(m_checkboxIntelligence[i]),true);
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		registerMediatorObject(*(m_checkboxAggression[i]),true);
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		registerMediatorObject(*(m_checkboxHuntersInstinct[i]),true);
	}

}

SwgCuiIncubator::~SwgCuiIncubator ()
{
	delete m_callback;
	m_callback      = 0;

	m_buttonCancel = NULL;
	m_buttonCommit = NULL;
	m_powerBar = NULL;
	m_heatBar = NULL;
	m_heatBarBack = NULL;
	m_nutrientBar = NULL;
	m_nutrientBarBack = NULL;
	m_buttonAddHeat = NULL;
	m_buttonSubtractHeat = NULL;
	m_buttonAddNutrients = NULL;
	m_buttonSubtractNutrients = NULL;
	m_viewer = NULL;

	if(m_creatureObject)
	{
		delete m_creatureObject;
	}
	m_creatureObject = 0;

	if (m_inventoryInfo)
	{
		if (m_containerInventory)
		{
			m_inventoryInfo->disconnectFromSelectionTransceiver (m_containerInventory->getTransceiverSelection ());
		}
	}

	if (m_containerInventory)
	{
		m_containerInventory->setFilter (0);
	}

	delete m_containerFilter;
	m_containerFilter = 0;

	delete m_providerFilter;
	m_providerFilter = 0;

	if(m_inventoryInfo)
	{
		m_inventoryInfo->release ();
		m_inventoryInfo = 0;
	}

	if(m_containerInventory)
	{
		m_containerInventory->release ();
		m_containerInventory = 0;
	}

	delete m_containerProviderInventory;
	m_containerProviderInventory = 0;

	

	int i;
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		m_checkboxSurvival[i] = NULL;
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		m_checkboxBeastialResilience[i] = NULL;
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		m_checkboxCunning[i] = NULL;
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		m_checkboxIntelligence[i] = NULL;
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		m_checkboxAggression[i] = NULL;
	}
	for (i = 0; i < ms_numCheckboxes; ++i)
	{
		m_checkboxHuntersInstinct[i] = NULL;
	}

	for (i = 0; i < ms_numSlots; ++i)
	{
		m_slotViewer[i] = NULL;
	}
}

//----------------------------------------------------------------------
// accessors
void SwgCuiIncubator::SetSessionNumber(int val)
{
	m_sessionNumber = val;
}
void SwgCuiIncubator::SetTerminalId(const NetworkId& val)
{
	m_terminalId = val;
}
void SwgCuiIncubator::SetPowerGaugeValue(int val)
{
	m_powerGaugeValue = val;
}
void SwgCuiIncubator::SetInitialPointsSurvival(int val)
{
	m_initialPointsSurvival = val;
}
void SwgCuiIncubator::SetInitialPointsBeastialResilience(int val)
{
	m_initialPointsBeastialResilience = val;
}
void SwgCuiIncubator::SetInitialPointsCunning(int val)
{
	m_initialPointsCunning = val;
}
void SwgCuiIncubator::SetInitialPointsIntelligence(int val)
{
	m_initialPointsIntelligence = val;
}
void SwgCuiIncubator::SetInitialPointsAggression(int val)
{
	m_initialPointsAggression = val;
}
void SwgCuiIncubator::SetInitialPointsHuntersInstinct(int val)
{
	m_initialPointsHuntersInstinct = val;
}
void SwgCuiIncubator::SetTemperatureGaugeValue(int val)
{
	if(val < 0)
	{
		val = 0;
	}
	else if(val > ms_maxNutrientTemperatureGaugeValue)
	{
		val = ms_maxNutrientTemperatureGaugeValue;
	}

	m_temperatureGaugeValue = val;
}
void SwgCuiIncubator::SetNutrientGaugeValue(int val)
{
	if(val < 0)
	{
		val = 0;
	}
	else if(val > ms_maxNutrientTemperatureGaugeValue)
	{
		val = ms_maxNutrientTemperatureGaugeValue;
	}

	m_nutrientGaugeValue = val;
}
void SwgCuiIncubator::SetInitialCreatureColorIndex(int val)
{
	m_initialCreatureColorIndex = val;
}
void SwgCuiIncubator::SetCreatureTemplateName(const std::string& val)
{
	m_creatureTemplateName = val;
}

//----------------------------------------------------------------------

int SwgCuiIncubator::getDefensiveSkillPointsLeft()
{
	UIString pointsLeftString;
	m_pointsDefensiveSkills->GetText(pointsLeftString);
	return atoi(Unicode::wideToNarrow(pointsLeftString).c_str());
}
void SwgCuiIncubator::setDefensiveSkillPointsLeft(int val)
{
	char buf[16];
	_itoa(val, buf, 10);
	m_pointsDefensiveSkills->SetText(Unicode::narrowToWide(buf));
	updateButtonStates();

}
int SwgCuiIncubator::getIntelligenceSkillPointsLeft()
{
	UIString pointsLeftString;
	m_pointsIntelligenceSkills->GetText(pointsLeftString);
	return atoi(Unicode::wideToNarrow(pointsLeftString).c_str());
}
void SwgCuiIncubator::setIntelligenceSkillPointsLeft(int val)
{
	char buf[16];
	_itoa(val, buf, 10);
	m_pointsIntelligenceSkills->SetText(Unicode::narrowToWide(buf));
	updateButtonStates();
}
int SwgCuiIncubator::getOffensiveSkillPointsLeft()
{
	UIString pointsLeftString;
	m_pointsOffensiveSkills->GetText(pointsLeftString);
	return atoi(Unicode::wideToNarrow(pointsLeftString).c_str());
}
void SwgCuiIncubator::setOffensiveSkillPointsLeft(int val)
{
	char buf[16];
	_itoa(val, buf, 10);
	m_pointsOffensiveSkills->SetText(Unicode::narrowToWide(buf));
	updateButtonStates();
}

//----------------------------------------------------------------------

bool SwgCuiIncubator::close()
{
	CreatureObject * const player = Game::getPlayerCreature();
	if(player && m_terminalId.getObject())
	{
		if(m_committed)
		{
			IncubatorCommitMessage * const msg = new IncubatorCommitMessage;
			msg->setTerminalId(m_terminalId);
			msg->setSlot1Id(ClientIncubatorManager::getNetworkId(ClientIncubatorManager::slot_slot1));
			msg->setSlot2Id(ClientIncubatorManager::getNetworkId(ClientIncubatorManager::slot_slot2)); 
			msg->setSlot3Id(ClientIncubatorManager::getNetworkId(ClientIncubatorManager::slot_slot3)); 
			msg->setSlot4Id(ClientIncubatorManager::getNetworkId(ClientIncubatorManager::slot_slot4)); 
			msg->setCommitted(true);

			msg->setInitialPointsSurvival(m_initialPointsSurvival);
			msg->setInitialPointsBeastialResilience(m_initialPointsBeastialResilience);
			msg->setInitialPointsCunning(m_initialPointsCunning);
			msg->setInitialPointsIntelligence(m_initialPointsIntelligence);
			msg->setInitialPointsAggression(m_initialPointsAggression);
			msg->setInitialPointsHuntersInstinct(m_initialPointsHuntersInstinct);
			msg->setTotalPointsSurvival(getTotalCheckboxFamilyPoints(CB_survival)); 
			msg->setTotalPointsBeastialResilience(getTotalCheckboxFamilyPoints(CB_beastialResilience));
			msg->setTotalPointsCunning(getTotalCheckboxFamilyPoints(CB_cunning));
			msg->setTotalPointsIntelligence(getTotalCheckboxFamilyPoints(CB_intelligence));
			msg->setTotalPointsAggression(getTotalCheckboxFamilyPoints(CB_aggression));
			msg->setTotalPointsHuntersInstinct(getTotalCheckboxFamilyPoints(CB_huntersInstinct));
			msg->setTemperatureGauge(m_temperatureGaugeValue);
			msg->setNutrientGauge(m_nutrientGaugeValue);
			msg->setNewCreatureColorIndex(m_newCreatureColorIndex);
			
			player->getController()->appendMessage (CM_incubatorCommit, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
		else
		{
			
			IncubatorCommitMessage * const msg = new IncubatorCommitMessage;
			msg->setTerminalId(m_terminalId);
			msg->setCommitted(false);

			player->getController()->appendMessage (CM_incubatorCancel, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	
	ClientIncubatorManager::resetSlots();
	return CuiMediator::close();
}

//----------------------------------------------------------------------

void SwgCuiIncubator::performActivate   ()
{
	CuiManager::requestPointer (true);
	m_inventoryInfo->activate ();
	m_containerInventory->activate ();

	m_containerProviderInventory->setContentDirty (true);

	CreatureObject * const player = dynamic_cast<CreatureObject*>(Game::getClientPlayer ());
	if (player != NULL)
	{
		ClientObject * const inventory = player->getInventoryObject();
		if (inventory != NULL)
		{
			m_containerInventory->setContainerObject (inventory, std::string ());
		}
	}

	int i;
	for(i = 0; i < ms_numSlots; ++i)
	{
		m_slotViewer[i]->AddCallback (this);
	}

	m_callback->connect (m_containerInventory->getTransceiverSelection (),    *this, &SwgCuiIncubator::onSelectionChanged);

	setIsUpdating(true);
}

//----------------------------------------------------------------------

void SwgCuiIncubator::performDeactivate ()
{
	CuiManager::requestPointer (false);
	m_inventoryInfo->deactivate ();

	m_callback->disconnect (m_containerInventory->getTransceiverSelection (), *this, &SwgCuiIncubator::onSelectionChanged);

	m_containerInventory->deactivate ();
	if (m_containerInventory && m_containerInventory->getVolumePage ())
	{
		m_containerInventory->getVolumePage()->RemoveCallback (this);
	}

	int i;
	for(i = 0; i < ms_numSlots; ++i)
	{
		m_slotViewer[i]->RemoveCallback (this);
	}

	setIsUpdating(false);
}

//----------------------------------------------------------------------

void SwgCuiIncubator::OnButtonPressed(UIWidget *context )
{
// JU_TODO: debug
#if 0
	static int debugVal = 15;
#endif

	if(context == m_buttonCancel)
	{
		closeThroughWorkspace ();
	}
	else if(context == m_buttonCommit)
	{
		m_committed = true;
		closeThroughWorkspace ();
	}
	else if(context == m_buttonAddHeat)
	{
// JU_TODO: debug
#if 0
	++debugVal;
	m_pointsToSpend = debugVal;
#endif
	// JU_TODO: end debug

		SetTemperatureGaugeValue(m_temperatureGaugeValue + 1);
		updateTemperatureBar();
		updateCreatureColor();
		updateButtonStates();
	
	}
	else if(context == m_buttonSubtractHeat)
	{
// JU_TODO: debug
#if 0
	--debugVal;
	if(debugVal < 0)
		debugVal = 0;
	m_pointsToSpend = debugVal;
#endif
	// JU_TODO: end debug
		SetTemperatureGaugeValue(m_temperatureGaugeValue - 1);
		updateTemperatureBar();
		updateCreatureColor();
		updateButtonStates();
	}
	else if(context == m_buttonAddNutrients)
	{
		SetNutrientGaugeValue(m_nutrientGaugeValue + 1);
		updateNutrientBar();
		resetChecksToInitialPoints();
		distributePointsToSpend();
	}
	else if(context == m_buttonSubtractNutrients)
	{
		SetNutrientGaugeValue(m_nutrientGaugeValue - 1);
		updateNutrientBar();
		resetChecksToInitialPoints();
		distributePointsToSpend();
	}
}

//----------------------------------------------------------------------

void SwgCuiIncubator::OnCheckboxSet(UIWidget *context)
{
	std::string stringVal;
	if(context->GetPropertyNarrow(UILowerString("AttribType"),stringVal) )
	{
		UICheckbox * cb = safe_cast<UICheckbox *> (context);
		if(cb)
		{
			if(stringVal == ms_attribNameSurvival)
			{
				handleCheckbox(CB_survival,cb,true);
			}
			else if(stringVal == ms_attribNameBeastialResilience)
			{
				handleCheckbox(CB_beastialResilience,cb,true);
			}
			else if(stringVal == ms_attribNameCunning)
			{
				handleCheckbox(CB_cunning,cb,true);
			}
			else if(stringVal == ms_attribNameIntelligence)
			{
				handleCheckbox(CB_intelligence,cb,true);
			}
			else if(stringVal == ms_attribNameAggression)
			{
				handleCheckbox(CB_aggression,cb,true);
			}
			else if(stringVal == ms_attribNameHuntersInstinct)
			{
				handleCheckbox(CB_huntersInstinct,cb,true);
			}
		}
	}
	else
	{
		if (context == m_checkFilter)
		{
			m_containerProviderInventory->setContentDirty (true);
		}
	}
	
}

//----------------------------------------------------------------------

void SwgCuiIncubator::OnCheckboxUnset(UIWidget *context)
{
	std::string stringVal;
	if(context->GetPropertyNarrow(UILowerString("AttribType"),stringVal) )
	{
		UICheckbox * cb = safe_cast<UICheckbox *> (context);
		if(cb)
		{
			if(stringVal == ms_attribNameSurvival)
			{
				handleCheckbox(CB_survival,cb,false);
			}
			else if(stringVal == ms_attribNameBeastialResilience)
			{
				handleCheckbox(CB_beastialResilience,cb,false);
			}
			else if(stringVal == ms_attribNameCunning)
			{
				handleCheckbox(CB_cunning,cb,false);
			}
			else if(stringVal == ms_attribNameIntelligence)
			{
				handleCheckbox(CB_intelligence,cb,false);
			}
			else if(stringVal == ms_attribNameAggression)
			{
				handleCheckbox(CB_aggression,cb,false);
			}
			else if(stringVal == ms_attribNameHuntersInstinct)
			{
				handleCheckbox(CB_huntersInstinct,cb,false);
			}
		}
	}
	else
	{
		if (context == m_checkFilter)
		{
			m_containerProviderInventory->setContentDirty (true);
		}
	}
}

bool SwgCuiIncubator::OnMessage( UIWidget *context, const UIMessage & msg )
{
	if (msg.Type == UIMessage::DragOver)
	{
		context->SetDropFlagOk (false);
		
		CuiWidget3dObjectListViewer * const draggingSlotViewer = dynamic_cast<CuiWidget3dObjectListViewer *>(msg.DragObject);
		if (draggingSlotViewer)
		{
			Object * const obj = draggingSlotViewer->getLastObject ();
			if (obj)
			{
				CuiWidget3dObjectListViewer * const destinationSlotViewer = findSlotViewer (context);
				if (destinationSlotViewer)
				{
					ClientIncubatorManager::Slot slot = static_cast<ClientIncubatorManager::Slot>(findSlotViewerIndex (destinationSlotViewer));
					if(ClientIncubatorManager::canEnzymeGoInSlot(obj,slot))
					{
						context->SetDropFlagOk (true);
					}
					return true;
				}
			}
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::DragStart)
	{
		
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::DragCancel)
	{
		// drag cancel will remove an enzyme from a slot
		CuiWidget3dObjectListViewer * const slotSource = dynamic_cast<CuiWidget3dObjectListViewer *>(msg.DragSource);
		if (slotSource)
		{
			const int slotIndex = findSlotViewerIndex (slotSource);
			clearSlot(slotIndex);
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::DragEnd)
	{
		if (msg.DragSource == context)
		{
			const int slotTargetIndex = findSlotViewerIndex (msg.DragTarget);
			if(slotTargetIndex == -1)
			{
				const int slotSourceIndex = findSlotViewerIndex (msg.DragSource);
				clearSlot(slotSourceIndex);
				return true;
			}
			return false;
		}

		CuiWidget3dObjectListViewer * const slotViewer = findSlotViewer (context);
			
		if (slotViewer)
		{
			CuiWidget3dObjectListViewer * const dragViewer = dynamic_cast<CuiWidget3dObjectListViewer *>(msg.DragObject);
			if (dragViewer)
			{
				ClientObject * const obj = dynamic_cast<ClientObject *>(dragViewer->getLastObject ());
				if (obj)
				{
					const int slotIndex = findSlotViewerIndex (slotViewer);

					if(slotIndex != -1)
					{
						// see if it already lives in another slot and clear it out
						const ClientIncubatorManager::Slot oldSlot = ClientIncubatorManager::getSlotIndexForNetworkId(obj->getNetworkId());
						if(oldSlot != ClientIncubatorManager::slot_invalid)
						{
							clearSlot(oldSlot);
						}
						setSlot(slotIndex,obj);
						return true;
					}	
				}
			}
		}
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::LeftMouseUp)
	{
		
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::MouseMove)
	{
		
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::MouseEnter)
	{
		
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::MouseExit)
	{
		
	}

	//----------------------------------------------------------------------

	else if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		CuiWidget3dObjectListViewer * const viewer = dynamic_cast<CuiWidget3dObjectListViewer *>(context);
		if (viewer)
		{
			const int slotIndex = findSlotViewerIndex (viewer);
			clearSlot(slotIndex);
		}
	}
	
	//----------------------------------------------------------------------

	return true;
}


//----------------------------------------------------------------------

void SwgCuiIncubator::initializeControls()
{

	resetChecksToInitialPoints();

	UIScalar height = m_powerBar->GetHeight();
	m_powerBar->SetHeight(static_cast<long>(height * (m_powerGaugeValue/1000.0f)));

	setDefensiveSkillPointsLeft(0);
	setIntelligenceSkillPointsLeft(0);
	setOffensiveSkillPointsLeft(0);
	
	if(m_creatureObject)
	{
		delete m_creatureObject;
	}

	std::string sharedTemplateName("object/mobile/shared_gungan_s02_male.iff");
	if(!m_creatureTemplateName.empty())
	{
		std::string tempString = convertServerObjectTemplateNameToSharedObjectTemplateName(m_creatureTemplateName);
		if(!tempString.empty())
		{
			sharedTemplateName = tempString; 
		}
	}
	
	m_creatureObject = safe_cast<ClientObject *>(ObjectTemplateList::createObject( TemporaryCrcString( sharedTemplateName.c_str(),true)));
	
	if(m_creatureObject)
	{
		m_creatureObject->endBaselines ();
	}

	if(m_creatureObject)
	{
		if (m_viewer)
		{
			m_viewer->setCameraForceTarget   (true);
			m_viewer->setObject              (m_creatureObject);
			m_viewer->recomputeZoom ();
			m_viewer->setCameraForceTarget   (false);
		}
	}

	updateTemperatureBar();
	updateNutrientBar();
	updateCreatureColor();

	updateButtonStates();
}


void SwgCuiIncubator::createObjectTooltipString (const ClientObject & obj, Unicode::String & str)
{
	// JU_TODO: implement?
	UNREF(obj);
	UNREF(str);
}

bool SwgCuiIncubator::overrideDoubleClick (const UIWidget & viewerWidget)
{
	const CuiWidget3dObjectListViewer * const viewer = dynamic_cast<const CuiWidget3dObjectListViewer *>(&viewerWidget);
	if (viewer)
	{
		ClientObject * const obj = const_cast<ClientObject *>(dynamic_cast<const ClientObject *>(viewer->getLastObject ()));

		if(obj)
		{
			// if it's a container, display its contents
			if (obj->getGameObjectType () == SharedObjectTemplate::GOT_misc_container || obj->getGameObjectType () == SharedObjectTemplate::GOT_misc_container_wearable)
			{
				m_containerInventory->setContainerObject (obj, std::string ());
				return true;
			}
			// check to see if it is already in a slot
			else if(ClientIncubatorManager::getSlotIndexForNetworkId(obj->getNetworkId()) == ClientIncubatorManager::slot_invalid)
			{
				int i;
				for(i = 0; i < ms_numSlots; ++i)
				{
					// see if something is already in this slot
					ClientIncubatorManager::Slot networkIdSlot = static_cast<ClientIncubatorManager::Slot>(i);
					if(ClientIncubatorManager::getNetworkId(networkIdSlot) == NetworkId::cms_invalid)
					{
						// see if I can go into that slot
						if(ClientIncubatorManager::canEnzymeGoInSlot(obj,networkIdSlot))
						{
							setSlot(i,obj);
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

void SwgCuiIncubator::update(float deltaTimeSecs)
{
	Object const * const player = Game::getConstPlayer();
	Object const * const terminal = m_terminalId.getObject();
	if (player && terminal)
	{
		// dismiss window if player and terminal are not in the same structure
		Object const * const building = ContainerInterface::getTopmostContainer(*terminal, false);
		if (!building || !ContainerInterface::isNestedWithin(*player, building->getNetworkId()))
		{
			closeThroughWorkspace();
		}
		// dismiss window if player picked up the terminal
		else if (ContainerInterface::isNestedWithin(*terminal, player->getNetworkId()))
		{
			closeThroughWorkspace();
		}
		else
		{
			// remove any item that is no longer on the character
			const Object * o;
			for(int i = 0; i < ms_numSlots; ++i)
			{
				o = m_slotViewer[i]->getLastObject();
				if (o && !ContainerInterface::isNestedWithin(*o, player->getNetworkId()))
					clearSlot(i);
			}
		}
	}
	else
	{
		// dismiss window if player or incubator is no longer around
		closeThroughWorkspace();
	}

	CuiMediator::update(deltaTimeSecs);
}

//----------------------------------------------------------------------

void SwgCuiIncubator::onSelectionChanged (const CuiContainerSelectionChanged::Payload & payload)
{
	UNREF(payload);
}

void SwgCuiIncubator::handleCheckbox(CheckboxType type,UICheckbox *cb, bool check)
{
	UICheckbox** checkboxFamily = getCheckboxFamily(type);
	NOT_NULL(checkboxFamily);
	int checkboxIndex = getCheckboxIndex(type,cb);
	if(!cb->IsEnabled() || checkboxIndex == -1)
		return;

	int pointsLeft = 0;
	switch(type)
	{
	case CB_survival:
	case CB_beastialResilience:
		pointsLeft = getDefensiveSkillPointsLeft();
		break;
	case CB_cunning:
	case CB_intelligence:
		pointsLeft = getIntelligenceSkillPointsLeft();
		break;
	case CB_aggression:
	case CB_huntersInstinct:
		pointsLeft = getOffensiveSkillPointsLeft();
		break;

	};

	if(check)
	{
		bool hitFinalCheck = false;
		for(int i = 0; i <= checkboxIndex && pointsLeft; ++i)
		{
			if(!checkboxFamily[i]->IsChecked() || i == checkboxIndex)
			{
				--pointsLeft;
				checkboxFamily[i]->SetChecked(true,false);
				if(i == checkboxIndex)
				{
					hitFinalCheck = true;
				}
			}		
		}
		if(!hitFinalCheck)
		{
			cb->SetChecked(false,false);
		}
	}
	else
	{
		for(int i = checkboxIndex; i < ms_numCheckboxes; ++i)
		{
			if(checkboxFamily[i]->IsChecked() || i == checkboxIndex)
			{
				++pointsLeft;
				checkboxFamily[i]->SetChecked(false,false);
			}
		}
	}

	switch(type)
	{
	case CB_survival:
	case CB_beastialResilience:
		setDefensiveSkillPointsLeft(pointsLeft);
		break;
	case CB_cunning:
	case CB_intelligence:
		setIntelligenceSkillPointsLeft(pointsLeft);
		break;
	case CB_aggression:
	case CB_huntersInstinct:
		setOffensiveSkillPointsLeft(pointsLeft);
		break;

	};
}

int SwgCuiIncubator::getTotalCheckboxFamilyPoints(CheckboxType type)
{
	UICheckbox** checkboxFamily = getCheckboxFamily(type);
	NOT_NULL(checkboxFamily);

	int total = 0;
	int i;
	for(i = 0; i < ms_numCheckboxes; ++i)
	{
		if(!checkboxFamily[i]->IsChecked())
			break;
		++total;
	}
	return total;
}

int SwgCuiIncubator::getCheckboxIndex(CheckboxType type, UICheckbox *cb)
{
	UICheckbox** checkboxFamily = getCheckboxFamily(type);
	NOT_NULL(checkboxFamily);

	int i;
	for(i = 0; i < ms_numCheckboxes; ++i)
	{
		if(cb == checkboxFamily[i])
			return i;
	}
	return -1;
}

UICheckbox** SwgCuiIncubator::getCheckboxFamily(CheckboxType type)
{
	UICheckbox** checkboxFamily = NULL;
	switch(type)
	{
		case CB_survival:
			checkboxFamily = m_checkboxSurvival;
			break;
		case CB_beastialResilience:
			checkboxFamily = m_checkboxBeastialResilience;
			break;
		case CB_cunning:
			checkboxFamily = m_checkboxCunning;
			break;
		case CB_intelligence:
			checkboxFamily = m_checkboxIntelligence;
			break;
		case CB_aggression:
			checkboxFamily = m_checkboxAggression;
			break;
		case CB_huntersInstinct:
			checkboxFamily = m_checkboxHuntersInstinct;
			break;
	};

	return checkboxFamily;
}

void SwgCuiIncubator::resetChecksToInitialPoints()
{
		int i;

	const UIColor greenDisabled(0x7f,0xff,0x7f);
	const UIColor yellowDisabled(0xd0,0xd0,0x7f);
	const UIColor orangeDisabled(0xff,0x7f,0x7f);

	// check and disable checkboxes from initialpoints
	for(i = 0; i < ms_numCheckboxes/*m_initialPointsSurvival*/; ++i)
	{
		if(i < m_initialPointsSurvival)
		{
			m_checkboxSurvival[i]->SetChecked(true,false);
			m_checkboxSurvival[i]->SetEnabled(false);
			m_checkboxSurvival[i]->SetColor(greenDisabled);
		}
		else
		{
			m_checkboxSurvival[i]->SetChecked(false,false);
			m_checkboxSurvival[i]->SetEnabled(true);
		}
	}
	for(i = 0; i < ms_numCheckboxes/*m_initialPointsBeastialResilience*/; ++i)
	{
		if(i < m_initialPointsBeastialResilience)
		{
			m_checkboxBeastialResilience[i]->SetChecked(true,false);
			m_checkboxBeastialResilience[i]->SetEnabled(false);
			m_checkboxBeastialResilience[i]->SetColor(greenDisabled);
		}
		else
		{
			m_checkboxBeastialResilience[i]->SetChecked(false,false);
			m_checkboxBeastialResilience[i]->SetEnabled(true);
		}
	}
	for(i = 0; i < ms_numCheckboxes/*m_initialPointsCunning*/; ++i)
	{
		if(i < m_initialPointsCunning)
		{
			m_checkboxCunning[i]->SetChecked(true,false);
			m_checkboxCunning[i]->SetEnabled(false);
			m_checkboxCunning[i]->SetColor(yellowDisabled);
		}
		else
		{
			m_checkboxCunning[i]->SetChecked(false,false);
			m_checkboxCunning[i]->SetEnabled(true);
		}
	}
	for(i = 0; i < ms_numCheckboxes/*m_initialPointsIntelligence*/; ++i)
	{
		if(i < m_initialPointsIntelligence)
		{
			m_checkboxIntelligence[i]->SetChecked(true,false);
			m_checkboxIntelligence[i]->SetEnabled(false);
			m_checkboxIntelligence[i]->SetColor(yellowDisabled);
		}
		else
		{
			m_checkboxIntelligence[i]->SetChecked(false,false);
			m_checkboxIntelligence[i]->SetEnabled(true);
		}
	}
	for(i = 0; i < ms_numCheckboxes/*m_initialPointsAggression*/; ++i)
	{
		if(i < m_initialPointsAggression)
		{
			m_checkboxAggression[i]->SetChecked(true,false);
			m_checkboxAggression[i]->SetEnabled(false);
			m_checkboxAggression[i]->SetColor(orangeDisabled);
		}
		else
		{
			m_checkboxAggression[i]->SetChecked(false,false);
			m_checkboxAggression[i]->SetEnabled(true);
		}
	}
	for(i = 0; i < ms_numCheckboxes/*m_initialPointsHuntersInstinct*/; ++i)
	{
		if(i < m_initialPointsHuntersInstinct)
		{
			m_checkboxHuntersInstinct[i]->SetChecked(true,false);
			m_checkboxHuntersInstinct[i]->SetEnabled(false);
			m_checkboxHuntersInstinct[i]->SetColor(orangeDisabled);
		}
		else
		{
			m_checkboxHuntersInstinct[i]->SetChecked(false,false);
			m_checkboxHuntersInstinct[i]->SetEnabled(true);
		}
	}
}

void SwgCuiIncubator::distributePointsToSpend()
{
	// because of floating point inaccuracy between different CPUs,
	// use a pre-generated list of all possible point distribution
	// combination for all possible combination of points and gauge
	// value; only use the formula if there is no pre-generated
	// combination values;
	//
	// TODO: move the pre-generated list into a shared datatable
	// that can be accessed by the server as well so the server
	// can validate that the combination sent from the client is
	// a valid combination
	static std::map<std::pair<int, int>, std::pair<int, std::pair<int, int> > > sPregeneratedCombos;
	if (sPregeneratedCombos.empty())
	{
		sPregeneratedCombos[std::make_pair(0,10)] = std::make_pair(0, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(0,9)] = std::make_pair(0, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(0,8)] = std::make_pair(0, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(0,7)] = std::make_pair(0, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(0,6)] = std::make_pair(0, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(0,5)] = std::make_pair(0, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(0,4)] = std::make_pair(0, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(0,3)] = std::make_pair(0, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(0,2)] = std::make_pair(0, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(0,1)] = std::make_pair(0, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(0,0)] = std::make_pair(0, std::make_pair(0,0));

		sPregeneratedCombos[std::make_pair(1,10)] = std::make_pair(1, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(1,9)] = std::make_pair(1, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(1,8)] = std::make_pair(1, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(1,7)] = std::make_pair(1, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(1,6)] = std::make_pair(1, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(1,5)] = std::make_pair(0, std::make_pair(1,0));
		sPregeneratedCombos[std::make_pair(1,4)] = std::make_pair(0, std::make_pair(0,1));
		sPregeneratedCombos[std::make_pair(1,3)] = std::make_pair(0, std::make_pair(0,1));
		sPregeneratedCombos[std::make_pair(1,2)] = std::make_pair(0, std::make_pair(0,1));
		sPregeneratedCombos[std::make_pair(1,1)] = std::make_pair(0, std::make_pair(0,1));
		sPregeneratedCombos[std::make_pair(1,0)] = std::make_pair(0, std::make_pair(0,1));

		sPregeneratedCombos[std::make_pair(2,10)] = std::make_pair(2, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(2,9)] = std::make_pair(2, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(2,8)] = std::make_pair(2, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(2,7)] = std::make_pair(2, std::make_pair(0,0)); 
		sPregeneratedCombos[std::make_pair(2,6)] = std::make_pair(1, std::make_pair(1,0));
		sPregeneratedCombos[std::make_pair(2,5)] = std::make_pair(0, std::make_pair(2,0));
		sPregeneratedCombos[std::make_pair(2,4)] = std::make_pair(0, std::make_pair(1,1));
		sPregeneratedCombos[std::make_pair(2,3)] = std::make_pair(0, std::make_pair(0,2));
		sPregeneratedCombos[std::make_pair(2,2)] = std::make_pair(0, std::make_pair(0,2));
		sPregeneratedCombos[std::make_pair(2,1)] = std::make_pair(0, std::make_pair(0,2));
		sPregeneratedCombos[std::make_pair(2,0)] = std::make_pair(0, std::make_pair(0,2));

		sPregeneratedCombos[std::make_pair(3,10)] = std::make_pair(3, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(3,9)] = std::make_pair(3, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(3,8)] = std::make_pair(3, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(3,7)] = std::make_pair(3, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(3,6)] = std::make_pair(2, std::make_pair(1,0));
		sPregeneratedCombos[std::make_pair(3,5)] = std::make_pair(1, std::make_pair(1,1));
		sPregeneratedCombos[std::make_pair(3,4)] = std::make_pair(0, std::make_pair(1,2));
		sPregeneratedCombos[std::make_pair(3,3)] = std::make_pair(0, std::make_pair(0,3));
		sPregeneratedCombos[std::make_pair(3,2)] = std::make_pair(0, std::make_pair(0,3));
		sPregeneratedCombos[std::make_pair(3,1)] = std::make_pair(0, std::make_pair(0,3));
		sPregeneratedCombos[std::make_pair(3,0)] = std::make_pair(0, std::make_pair(0,3));

		sPregeneratedCombos[std::make_pair(4,10)] = std::make_pair(4, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(4,9)] = std::make_pair(4, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(4,8)] = std::make_pair(3, std::make_pair(1,0));
		sPregeneratedCombos[std::make_pair(4,7)] = std::make_pair(3, std::make_pair(1,0));
		sPregeneratedCombos[std::make_pair(4,6)] = std::make_pair(2, std::make_pair(1,1));
		sPregeneratedCombos[std::make_pair(4,5)] = std::make_pair(1, std::make_pair(2,1));
		sPregeneratedCombos[std::make_pair(4,4)] = std::make_pair(1, std::make_pair(1,2));
		sPregeneratedCombos[std::make_pair(4,3)] = std::make_pair(0, std::make_pair(1,3));
		sPregeneratedCombos[std::make_pair(4,2)] = std::make_pair(0, std::make_pair(1,3));
		sPregeneratedCombos[std::make_pair(4,1)] = std::make_pair(0, std::make_pair(0,4));
		sPregeneratedCombos[std::make_pair(4,0)] = std::make_pair(0, std::make_pair(0,4));

		sPregeneratedCombos[std::make_pair(5,10)] = std::make_pair(5, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(5,9)] = std::make_pair(5, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(5,8)] = std::make_pair(4, std::make_pair(1,0));
		sPregeneratedCombos[std::make_pair(5,7)] = std::make_pair(3, std::make_pair(2,0));
		sPregeneratedCombos[std::make_pair(5,6)] = std::make_pair(2, std::make_pair(2,1));
		sPregeneratedCombos[std::make_pair(5,5)] = std::make_pair(1, std::make_pair(3,1));
		sPregeneratedCombos[std::make_pair(5,4)] = std::make_pair(1, std::make_pair(2,2));
		sPregeneratedCombos[std::make_pair(5,3)] = std::make_pair(0, std::make_pair(2,3));
		sPregeneratedCombos[std::make_pair(5,2)] = std::make_pair(0, std::make_pair(1,4)); 
		sPregeneratedCombos[std::make_pair(5,1)] = std::make_pair(0, std::make_pair(0,5));
		sPregeneratedCombos[std::make_pair(5,0)] = std::make_pair(0, std::make_pair(0,5));

		sPregeneratedCombos[std::make_pair(6,10)] = std::make_pair(6, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(6,9)] = std::make_pair(5, std::make_pair(1,0));
		sPregeneratedCombos[std::make_pair(6,8)] = std::make_pair(4, std::make_pair(2,0));
		sPregeneratedCombos[std::make_pair(6,7)] = std::make_pair(3, std::make_pair(2,1));
		sPregeneratedCombos[std::make_pair(6,6)] = std::make_pair(2, std::make_pair(3,1));
		sPregeneratedCombos[std::make_pair(6,5)] = std::make_pair(1, std::make_pair(4,1));
		sPregeneratedCombos[std::make_pair(6,4)] = std::make_pair(1, std::make_pair(3,2));
		sPregeneratedCombos[std::make_pair(6,3)] = std::make_pair(1, std::make_pair(2,3));
		sPregeneratedCombos[std::make_pair(6,2)] = std::make_pair(0, std::make_pair(2,4));
		sPregeneratedCombos[std::make_pair(6,1)] = std::make_pair(0, std::make_pair(1,5));
		sPregeneratedCombos[std::make_pair(6,0)] = std::make_pair(0, std::make_pair(0,6));

		sPregeneratedCombos[std::make_pair(7,10)] = std::make_pair(7, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(7,9)] = std::make_pair(6, std::make_pair(1,0));
		sPregeneratedCombos[std::make_pair(7,8)] = std::make_pair(5, std::make_pair(2,0));
		sPregeneratedCombos[std::make_pair(7,7)] = std::make_pair(3, std::make_pair(3,1));
		sPregeneratedCombos[std::make_pair(7,6)] = std::make_pair(2, std::make_pair(4,1));
		sPregeneratedCombos[std::make_pair(7,5)] = std::make_pair(1, std::make_pair(5,1));
		sPregeneratedCombos[std::make_pair(7,4)] = std::make_pair(1, std::make_pair(4,2));
		sPregeneratedCombos[std::make_pair(7,3)] = std::make_pair(1, std::make_pair(3,3));
		sPregeneratedCombos[std::make_pair(7,2)] = std::make_pair(0, std::make_pair(2,5));
		sPregeneratedCombos[std::make_pair(7,1)] = std::make_pair(0, std::make_pair(1,6));
		sPregeneratedCombos[std::make_pair(7,0)] = std::make_pair(0, std::make_pair(0,7));

		sPregeneratedCombos[std::make_pair(8,10)] = std::make_pair(8, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(8,9)] = std::make_pair(7, std::make_pair(1,0));
		sPregeneratedCombos[std::make_pair(8,8)] = std::make_pair(6, std::make_pair(2,0));
		sPregeneratedCombos[std::make_pair(8,7)] = std::make_pair(4, std::make_pair(3,1));
		sPregeneratedCombos[std::make_pair(8,6)] = std::make_pair(2, std::make_pair(5,1));
		sPregeneratedCombos[std::make_pair(8,5)] = std::make_pair(1, std::make_pair(6,1));
		sPregeneratedCombos[std::make_pair(8,4)] = std::make_pair(1, std::make_pair(5,2));
		sPregeneratedCombos[std::make_pair(8,3)] = std::make_pair(1, std::make_pair(3,4));
		sPregeneratedCombos[std::make_pair(8,2)] = std::make_pair(0, std::make_pair(2,6));
		sPregeneratedCombos[std::make_pair(8,1)] = std::make_pair(0, std::make_pair(1,7));
		sPregeneratedCombos[std::make_pair(8,0)] = std::make_pair(0, std::make_pair(0,8));

		sPregeneratedCombos[std::make_pair(9,10)] = std::make_pair(9, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(9,9)] = std::make_pair(8, std::make_pair(1,0));
		sPregeneratedCombos[std::make_pair(9,8)] = std::make_pair(7, std::make_pair(2,0));
		sPregeneratedCombos[std::make_pair(9,7)] = std::make_pair(4, std::make_pair(4,1));
		sPregeneratedCombos[std::make_pair(9,6)] = std::make_pair(3, std::make_pair(5,1));
		sPregeneratedCombos[std::make_pair(9,5)] = std::make_pair(1, std::make_pair(7,1));
		sPregeneratedCombos[std::make_pair(9,4)] = std::make_pair(1, std::make_pair(5,3));
		sPregeneratedCombos[std::make_pair(9,3)] = std::make_pair(1, std::make_pair(4,4));
		sPregeneratedCombos[std::make_pair(9,2)] = std::make_pair(0, std::make_pair(2,7));
		sPregeneratedCombos[std::make_pair(9,1)] = std::make_pair(0, std::make_pair(1,8));
		sPregeneratedCombos[std::make_pair(9,0)] = std::make_pair(0, std::make_pair(0,9));

		sPregeneratedCombos[std::make_pair(10,10)] = std::make_pair(10, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(10,9)] = std::make_pair(9, std::make_pair(1,0));
		sPregeneratedCombos[std::make_pair(10,8)] = std::make_pair(7, std::make_pair(3,0));
		sPregeneratedCombos[std::make_pair(10,7)] = std::make_pair(5, std::make_pair(4,1));
		sPregeneratedCombos[std::make_pair(10,6)] = std::make_pair(3, std::make_pair(6,1));
		sPregeneratedCombos[std::make_pair(10,5)] = std::make_pair(1, std::make_pair(8,1));
		sPregeneratedCombos[std::make_pair(10,4)] = std::make_pair(1, std::make_pair(6,3));
		sPregeneratedCombos[std::make_pair(10,3)] = std::make_pair(1, std::make_pair(4,5));
		sPregeneratedCombos[std::make_pair(10,2)] = std::make_pair(0, std::make_pair(3,7));
		sPregeneratedCombos[std::make_pair(10,1)] = std::make_pair(0, std::make_pair(1,9));
		sPregeneratedCombos[std::make_pair(10,0)] = std::make_pair(0, std::make_pair(0,10));

		sPregeneratedCombos[std::make_pair(11,10)] = std::make_pair(11, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(11,9)] = std::make_pair(10, std::make_pair(1,0));
		sPregeneratedCombos[std::make_pair(11,8)] = std::make_pair(8, std::make_pair(3,0));
		sPregeneratedCombos[std::make_pair(11,7)] = std::make_pair(6, std::make_pair(4,1));
		sPregeneratedCombos[std::make_pair(11,6)] = std::make_pair(3, std::make_pair(7,1));
		sPregeneratedCombos[std::make_pair(11,5)] = std::make_pair(1, std::make_pair(9,1));
		sPregeneratedCombos[std::make_pair(11,4)] = std::make_pair(1, std::make_pair(7,3));
		sPregeneratedCombos[std::make_pair(11,3)] = std::make_pair(1, std::make_pair(4,6));
		sPregeneratedCombos[std::make_pair(11,2)] = std::make_pair(0, std::make_pair(3,8));
		sPregeneratedCombos[std::make_pair(11,1)] = std::make_pair(0, std::make_pair(1,10));
		sPregeneratedCombos[std::make_pair(11,0)] = std::make_pair(0, std::make_pair(0,11));

		sPregeneratedCombos[std::make_pair(12,10)] = std::make_pair(12, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(12,9)] = std::make_pair(10, std::make_pair(2,0));
		sPregeneratedCombos[std::make_pair(12,8)] = std::make_pair(9, std::make_pair(3,0));
		sPregeneratedCombos[std::make_pair(12,7)] = std::make_pair(6, std::make_pair(5,1));
		sPregeneratedCombos[std::make_pair(12,6)] = std::make_pair(4, std::make_pair(7,1));
		sPregeneratedCombos[std::make_pair(12,5)] = std::make_pair(1, std::make_pair(10,1));
		sPregeneratedCombos[std::make_pair(12,4)] = std::make_pair(1, std::make_pair(7,4));
		sPregeneratedCombos[std::make_pair(12,3)] = std::make_pair(1, std::make_pair(5,6));
		sPregeneratedCombos[std::make_pair(12,2)] = std::make_pair(0, std::make_pair(3,9));
		sPregeneratedCombos[std::make_pair(12,1)] = std::make_pair(0, std::make_pair(2,10));
		sPregeneratedCombos[std::make_pair(12,0)] = std::make_pair(0, std::make_pair(0,12));

		sPregeneratedCombos[std::make_pair(13,10)] = std::make_pair(13, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(13,9)] = std::make_pair(11, std::make_pair(2,0));
		sPregeneratedCombos[std::make_pair(13,8)] = std::make_pair(8, std::make_pair(4,1));
		sPregeneratedCombos[std::make_pair(13,7)] = std::make_pair(7, std::make_pair(5,1));
		sPregeneratedCombos[std::make_pair(13,6)] = std::make_pair(4, std::make_pair(8,1));
		sPregeneratedCombos[std::make_pair(13,5)] = std::make_pair(1, std::make_pair(11,1));
		sPregeneratedCombos[std::make_pair(13,4)] = std::make_pair(1, std::make_pair(8,4));
		sPregeneratedCombos[std::make_pair(13,3)] = std::make_pair(1, std::make_pair(5,7));
		sPregeneratedCombos[std::make_pair(13,2)] = std::make_pair(1, std::make_pair(4,8));
		sPregeneratedCombos[std::make_pair(13,1)] = std::make_pair(0, std::make_pair(2,11));
		sPregeneratedCombos[std::make_pair(13,0)] = std::make_pair(0, std::make_pair(0,13));

		sPregeneratedCombos[std::make_pair(14,10)] = std::make_pair(14, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(14,9)] = std::make_pair(12, std::make_pair(2,0));
		sPregeneratedCombos[std::make_pair(14,8)] = std::make_pair(9, std::make_pair(4,1));
		sPregeneratedCombos[std::make_pair(14,7)] = std::make_pair(6, std::make_pair(6,2));
		sPregeneratedCombos[std::make_pair(14,6)] = std::make_pair(3, std::make_pair(9,2));
		sPregeneratedCombos[std::make_pair(14,5)] = std::make_pair(2, std::make_pair(10,2));
		sPregeneratedCombos[std::make_pair(14,4)] = std::make_pair(2, std::make_pair(9,3));
		sPregeneratedCombos[std::make_pair(14,3)] = std::make_pair(2, std::make_pair(6,6));
		sPregeneratedCombos[std::make_pair(14,2)] = std::make_pair(1, std::make_pair(4,9));
		sPregeneratedCombos[std::make_pair(14,1)] = std::make_pair(0, std::make_pair(2,12));
		sPregeneratedCombos[std::make_pair(14,0)] = std::make_pair(0, std::make_pair(0,14));

		sPregeneratedCombos[std::make_pair(15,10)] = std::make_pair(15, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(15,9)] = std::make_pair(13, std::make_pair(2,0));
		sPregeneratedCombos[std::make_pair(15,8)] = std::make_pair(10, std::make_pair(4,1));
		sPregeneratedCombos[std::make_pair(15,7)] = std::make_pair(7, std::make_pair(6,2));
		sPregeneratedCombos[std::make_pair(15,6)] = std::make_pair(4, std::make_pair(9,2));
		sPregeneratedCombos[std::make_pair(15,5)] = std::make_pair(2, std::make_pair(11,2));
		sPregeneratedCombos[std::make_pair(15,4)] = std::make_pair(2, std::make_pair(9,4));
		sPregeneratedCombos[std::make_pair(15,3)] = std::make_pair(2, std::make_pair(6,7));
		sPregeneratedCombos[std::make_pair(15,2)] = std::make_pair(1, std::make_pair(4,10));
		sPregeneratedCombos[std::make_pair(15,1)] = std::make_pair(0, std::make_pair(2,13));
		sPregeneratedCombos[std::make_pair(15,0)] = std::make_pair(0, std::make_pair(0,15));

		sPregeneratedCombos[std::make_pair(16,10)] = std::make_pair(16, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(16,9)] = std::make_pair(14, std::make_pair(2,0));
		sPregeneratedCombos[std::make_pair(16,8)] = std::make_pair(10, std::make_pair(5,1));
		sPregeneratedCombos[std::make_pair(16,7)] = std::make_pair(7, std::make_pair(7,2));
		sPregeneratedCombos[std::make_pair(16,6)] = std::make_pair(4, std::make_pair(10,2));
		sPregeneratedCombos[std::make_pair(16,5)] = std::make_pair(2, std::make_pair(12,2));
		sPregeneratedCombos[std::make_pair(16,4)] = std::make_pair(2, std::make_pair(10,4));
		sPregeneratedCombos[std::make_pair(16,3)] = std::make_pair(2, std::make_pair(7,7));
		sPregeneratedCombos[std::make_pair(16,2)] = std::make_pair(1, std::make_pair(5,10));
		sPregeneratedCombos[std::make_pair(16,1)] = std::make_pair(0, std::make_pair(2,14));
		sPregeneratedCombos[std::make_pair(16,0)] = std::make_pair(0, std::make_pair(0,16));

		sPregeneratedCombos[std::make_pair(17,10)] = std::make_pair(17, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(17,9)] = std::make_pair(14, std::make_pair(3,0));
		sPregeneratedCombos[std::make_pair(17,8)] = std::make_pair(11, std::make_pair(5,1));
		sPregeneratedCombos[std::make_pair(17,7)] = std::make_pair(8, std::make_pair(7,2));
		sPregeneratedCombos[std::make_pair(17,6)] = std::make_pair(4, std::make_pair(11,2));
		sPregeneratedCombos[std::make_pair(17,5)] = std::make_pair(2, std::make_pair(13,2));
		sPregeneratedCombos[std::make_pair(17,4)] = std::make_pair(2, std::make_pair(11,4));
		sPregeneratedCombos[std::make_pair(17,3)] = std::make_pair(2, std::make_pair(7,8));
		sPregeneratedCombos[std::make_pair(17,2)] = std::make_pair(1, std::make_pair(5,11));
		sPregeneratedCombos[std::make_pair(17,1)] = std::make_pair(0, std::make_pair(3,14));
		sPregeneratedCombos[std::make_pair(17,0)] = std::make_pair(0, std::make_pair(0,17));

		sPregeneratedCombos[std::make_pair(18,10)] = std::make_pair(18, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(18,9)] = std::make_pair(15, std::make_pair(3,0));
		sPregeneratedCombos[std::make_pair(18,8)] = std::make_pair(12, std::make_pair(5,1));
		sPregeneratedCombos[std::make_pair(18,7)] = std::make_pair(8, std::make_pair(8,2));
		sPregeneratedCombos[std::make_pair(18,6)] = std::make_pair(5, std::make_pair(11,2));
		sPregeneratedCombos[std::make_pair(18,5)] = std::make_pair(2, std::make_pair(14,2));
		sPregeneratedCombos[std::make_pair(18,4)] = std::make_pair(2, std::make_pair(11,5));
		sPregeneratedCombos[std::make_pair(18,3)] = std::make_pair(2, std::make_pair(8,8));
		sPregeneratedCombos[std::make_pair(18,2)] = std::make_pair(1, std::make_pair(5,12));
		sPregeneratedCombos[std::make_pair(18,1)] = std::make_pair(0, std::make_pair(3,15));
		sPregeneratedCombos[std::make_pair(18,0)] = std::make_pair(0, std::make_pair(0,18));

		sPregeneratedCombos[std::make_pair(19,10)] = std::make_pair(19, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(19,9)] = std::make_pair(16, std::make_pair(3,0));
		sPregeneratedCombos[std::make_pair(19,8)] = std::make_pair(12, std::make_pair(6,1));
		sPregeneratedCombos[std::make_pair(19,7)] = std::make_pair(9, std::make_pair(8,2));
		sPregeneratedCombos[std::make_pair(19,6)] = std::make_pair(5, std::make_pair(12,2));
		sPregeneratedCombos[std::make_pair(19,5)] = std::make_pair(2, std::make_pair(15,2));
		sPregeneratedCombos[std::make_pair(19,4)] = std::make_pair(2, std::make_pair(12,5));
		sPregeneratedCombos[std::make_pair(19,3)] = std::make_pair(2, std::make_pair(8,9));
		sPregeneratedCombos[std::make_pair(19,2)] = std::make_pair(1, std::make_pair(6,12));
		sPregeneratedCombos[std::make_pair(19,1)] = std::make_pair(0, std::make_pair(3,16));
		sPregeneratedCombos[std::make_pair(19,0)] = std::make_pair(0, std::make_pair(0,19));

		sPregeneratedCombos[std::make_pair(20,10)] = std::make_pair(20, std::make_pair(0,0));
		sPregeneratedCombos[std::make_pair(20,9)] = std::make_pair(17, std::make_pair(3,0));
		sPregeneratedCombos[std::make_pair(20,8)] = std::make_pair(13, std::make_pair(6,1));
		sPregeneratedCombos[std::make_pair(20,7)] = std::make_pair(8, std::make_pair(9,3));
		sPregeneratedCombos[std::make_pair(20,6)] = std::make_pair(4, std::make_pair(13,3));
		sPregeneratedCombos[std::make_pair(20,5)] = std::make_pair(3, std::make_pair(14,3));
		sPregeneratedCombos[std::make_pair(20,4)] = std::make_pair(3, std::make_pair(13,4));
		sPregeneratedCombos[std::make_pair(20,3)] = std::make_pair(3, std::make_pair(9,8));
		sPregeneratedCombos[std::make_pair(20,2)] = std::make_pair(1, std::make_pair(6,13));
		sPregeneratedCombos[std::make_pair(20,1)] = std::make_pair(0, std::make_pair(3,17));
		sPregeneratedCombos[std::make_pair(20,0)] = std::make_pair(0, std::make_pair(0,20));

		// sanity checker
		int upper, middle, lower;
		int previousUpper, previousMiddle, previousLower;
		std::map<std::pair<int, int>, std::pair<int, std::pair<int, int> > >::const_iterator iterFind;
		for (int points = 0; points <= 20; ++points)
		{
			for (int gauge = 0; gauge <= ms_maxNutrientTemperatureGaugeValue; ++gauge)
			{			
				iterFind = sPregeneratedCombos.find(std::make_pair(points, gauge));

				// missing combination
				FATAL((iterFind == sPregeneratedCombos.end()), ("incubator points distribution for (points=%d, gauge=%d) - not defined", points, gauge));

				upper = iterFind->second.first;
				middle = iterFind->second.second.first;
				lower = iterFind->second.second.second;

				// upper + middle + lower doesn't equal points
				FATAL(((upper + middle + lower) != points), ("incubator points distribution for (points=%d, gauge=%d, upper=%d, middle=%d, lower=%d) - sum of upper, middle, and lower does not equal points", points, gauge, upper, middle, lower));

				if (gauge > 0)
				{
					iterFind = sPregeneratedCombos.find(std::make_pair(points, gauge-1));

					// missing combination
					FATAL((iterFind == sPregeneratedCombos.end()), ("incubator points distribution for (points=%d, gauge=%d) - not defined", points, (gauge-1)));

					previousUpper = iterFind->second.first;
					previousMiddle = iterFind->second.second.first;
					previousLower = iterFind->second.second.second;

					// upper value must be non-decreasing
					FATAL((upper < previousUpper), ("incubator points distribution for (points=%d, gauge=%d, upper=%d, middle=%d, lower=%d) - upper value decreased from previous upper value", points, gauge, upper, middle, lower));

					// middle value must be non-increasing "from the middle out"
					if (gauge <= ms_middleNutrientTemperatureGaugeValue)
					{
						FATAL((middle < previousMiddle), ("incubator points distribution for (points=%d, gauge=%d, upper=%d, middle=%d, lower=%d) - middle value decreased from previous middle value", points, gauge, upper, middle, lower));
					}
					else
					{
						FATAL((middle > previousMiddle), ("incubator points distribution for (points=%d, gauge=%d, upper=%d, middle=%d, lower=%d) - middle value increased from previous middle value", points, gauge, upper, middle, lower));
					}

					// lower value must be non-increasing
					FATAL((lower > previousLower), ("incubator points distribution for (points=%d, gauge=%d, upper=%d, middle=%d, lower=%d) - lower value increased from previous lower value", points, gauge, upper, middle, lower));
				}
			}

			// 0/10 symmetry check
			iterFind = sPregeneratedCombos.find(std::make_pair(points, 0));
			FATAL((iterFind == sPregeneratedCombos.end()), ("incubator points distribution for (points=%d, gauge=0) - not defined", points));
			upper = iterFind->second.first;
			middle = iterFind->second.second.first;
			lower = iterFind->second.second.second;

			iterFind = sPregeneratedCombos.find(std::make_pair(points, 10));
			FATAL((iterFind == sPregeneratedCombos.end()), ("incubator points distribution for (points=%d, gauge=10) - not defined", points));
			previousUpper = iterFind->second.first;
			previousMiddle = iterFind->second.second.first;
			previousLower = iterFind->second.second.second;

			FATAL((upper != previousLower), ("incubator points distribution for (points=%d) - gauge 0 upper/10 lower mismatch", points));
			FATAL((lower != previousUpper), ("incubator points distribution for (points=%d) - gauge 0 lower/10 upper mismatch", points));
			FATAL((middle != previousMiddle), ("incubator points distribution for (points=%d) - gauge 0 middle/10 middle mismatch", points));

			// 1/9 symmetry check
			iterFind = sPregeneratedCombos.find(std::make_pair(points, 1));
			FATAL((iterFind == sPregeneratedCombos.end()), ("incubator points distribution for (points=%d, gauge=1) - not defined", points));
			upper = iterFind->second.first;
			middle = iterFind->second.second.first;
			lower = iterFind->second.second.second;

			iterFind = sPregeneratedCombos.find(std::make_pair(points, 9));
			FATAL((iterFind == sPregeneratedCombos.end()), ("incubator points distribution for (points=%d, gauge=9) - not defined", points));
			previousUpper = iterFind->second.first;
			previousMiddle = iterFind->second.second.first;
			previousLower = iterFind->second.second.second;

			FATAL((upper != previousLower), ("incubator points distribution for (points=%d) - gauge 1 upper/9 lower mismatch", points));
			FATAL((lower != previousUpper), ("incubator points distribution for (points=%d) - gauge 1 lower/9 upper mismatch", points));
			FATAL((middle != previousMiddle), ("incubator points distribution for (points=%d) - gauge 1 middle/9 middle mismatch", points));

			// 2/8 symmetry check
			iterFind = sPregeneratedCombos.find(std::make_pair(points, 2));
			FATAL((iterFind == sPregeneratedCombos.end()), ("incubator points distribution for (points=%d, gauge=2) - not defined", points));
			upper = iterFind->second.first;
			middle = iterFind->second.second.first;
			lower = iterFind->second.second.second;

			iterFind = sPregeneratedCombos.find(std::make_pair(points, 8));
			FATAL((iterFind == sPregeneratedCombos.end()), ("incubator points distribution for (points=%d, gauge=8) - not defined", points));
			previousUpper = iterFind->second.first;
			previousMiddle = iterFind->second.second.first;
			previousLower = iterFind->second.second.second;

			FATAL((upper != previousLower), ("incubator points distribution for (points=%d) - gauge 2 upper/8 lower mismatch", points));
			FATAL((lower != previousUpper), ("incubator points distribution for (points=%d) - gauge 2 lower/8 upper mismatch", points));
			FATAL((middle != previousMiddle), ("incubator points distribution for (points=%d) - gauge 2 middle/8 middle mismatch", points));

			// 3/7 symmetry check
			iterFind = sPregeneratedCombos.find(std::make_pair(points, 3));
			FATAL((iterFind == sPregeneratedCombos.end()), ("incubator points distribution for (points=%d, gauge=3) - not defined", points));
			upper = iterFind->second.first;
			middle = iterFind->second.second.first;
			lower = iterFind->second.second.second;

			iterFind = sPregeneratedCombos.find(std::make_pair(points, 7));
			FATAL((iterFind == sPregeneratedCombos.end()), ("incubator points distribution for (points=%d, gauge=7) - not defined", points));
			previousUpper = iterFind->second.first;
			previousMiddle = iterFind->second.second.first;
			previousLower = iterFind->second.second.second;

			FATAL((upper != previousLower), ("incubator points distribution for (points=%d) - gauge 3 upper/7 lower mismatch", points));
			FATAL((lower != previousUpper), ("incubator points distribution for (points=%d) - gauge 3 lower/7 upper mismatch", points));
			FATAL((middle != previousMiddle), ("incubator points distribution for (points=%d) - gauge 3 middle/7 middle mismatch", points));

			// 4/6 symmetry check
			iterFind = sPregeneratedCombos.find(std::make_pair(points, 4));
			FATAL((iterFind == sPregeneratedCombos.end()), ("incubator points distribution for (points=%d, gauge=4) - not defined", points));
			upper = iterFind->second.first;
			middle = iterFind->second.second.first;
			lower = iterFind->second.second.second;

			iterFind = sPregeneratedCombos.find(std::make_pair(points, 6));
			FATAL((iterFind == sPregeneratedCombos.end()), ("incubator points distribution for (points=%d, gauge=6) - not defined", points));
			previousUpper = iterFind->second.first;
			previousMiddle = iterFind->second.second.first;
			previousLower = iterFind->second.second.second;

			FATAL((upper != previousLower), ("incubator points distribution for (points=%d) - gauge 4 upper/6 lower mismatch", points));
			FATAL((lower != previousUpper), ("incubator points distribution for (points=%d) - gauge 4 lower/6 upper mismatch", points));
			FATAL((middle != previousMiddle), ("incubator points distribution for (points=%d) - gauge 4 middle/6 middle mismatch", points));
		}
	}

	int iUpperValue=0,iMiddleValue=0,iLowerValue=0;

	// use pre-generated values if available
	std::map<std::pair<int, int>, std::pair<int, std::pair<int, int> > >::const_iterator iterFind = sPregeneratedCombos.find(std::make_pair(m_pointsToSpend, m_nutrientGaugeValue));
	if (iterFind != sPregeneratedCombos.end())
	{
		iUpperValue = iterFind->second.first;
		iMiddleValue = iterFind->second.second.first;
		iLowerValue = iterFind->second.second.second;
	}
	else
	{
		const double pointsToSpend = static_cast<double>(m_pointsToSpend);
		double weight = static_cast<double>(m_nutrientGaugeValue)/10.0;
		double pointsLeft = pointsToSpend;
		double upperValue,middleValue,lowerValue;
		
		double baseMultiplier = 0.15;
		if(m_pointsToSpend < 5)
			baseMultiplier = 0.35;
		else if(m_pointsToSpend < 7)
			baseMultiplier = 0.2;

		if(m_nutrientGaugeValue != ms_middleNutrientTemperatureGaugeValue)
		{
			double modWeight = weight;
			if(weight > 0.5)
				modWeight = 1.0 - weight;

			const double multiplier = (1.0 - modWeight/0.5);
			lowerValue = (multiplier * pointsLeft);
			pointsLeft -= lowerValue;

			const double multiplier2 = (1.0 - multiplier)/2.0;
			upperValue = std::min((multiplier2 * pointsLeft),baseMultiplier * pointsToSpend);
			pointsLeft -= upperValue;

			middleValue = std::min(pointsLeft,pointsToSpend - (baseMultiplier * 2.0 * pointsToSpend));

			iUpperValue = static_cast<int>(upperValue);
			iMiddleValue = static_cast<int>(middleValue);
			iLowerValue = static_cast<int>(lowerValue);

			int sum = iUpperValue + iMiddleValue + iLowerValue;
			if(sum < m_pointsToSpend)
			{
				iLowerValue += (m_pointsToSpend - sum);
			}

			if(weight > 0.5)
			{
				int swap = iLowerValue;
				iLowerValue = iUpperValue;
				iUpperValue = swap;
			}
			
		}
		else // == 0.5 middle
		{
			upperValue = (baseMultiplier * pointsToSpend);
			lowerValue = (baseMultiplier * pointsToSpend);
			pointsLeft -= (upperValue + lowerValue);
			
			middleValue = pointsLeft;

			iUpperValue = static_cast<int>(upperValue);
			iMiddleValue = static_cast<int>(middleValue);
			iLowerValue = static_cast<int>(lowerValue);

			int sum = iUpperValue + iMiddleValue + iLowerValue;
			if(sum < m_pointsToSpend)
			{
				iMiddleValue += (m_pointsToSpend - sum);
			}
		}

		// JU_TODO: debug
		#if 0
		DEBUG_REPORT_LOG(true,("incoming points = %f, distribution(%f,%f,%f) (%d,%d,%d)\n",
			pointsToSpend,
			upperValue,
			middleValue,
			lowerValue,
			iUpperValue,
			iMiddleValue,
			iLowerValue
			));
		#endif
		// JU_TODO: end debug
	}

	setDefensiveSkillPointsLeft(iUpperValue);
	setIntelligenceSkillPointsLeft(iMiddleValue);
	setOffensiveSkillPointsLeft(iLowerValue);
}

// progress bars
void SwgCuiIncubator::updateTemperatureBar()
{
	UIScalar backerHeight = m_heatBarBack->GetHeight();
	const float max = ms_maxNutrientTemperatureGaugeValue;
	m_heatBar->SetHeight(static_cast<long>(backerHeight * (m_temperatureGaugeValue/max)));
	m_heatBar->PackSelfLocation();
}

void SwgCuiIncubator::updateNutrientBar()
{
	UIScalar backerHeight = m_nutrientBarBack->GetHeight();
	const float max = ms_maxNutrientTemperatureGaugeValue;
	m_nutrientBar->SetHeight(static_cast<long>(backerHeight * (m_nutrientGaugeValue/max)));
	m_nutrientBar->PackSelfLocation();
}

void SwgCuiIncubator::updateCreatureColor()
{
	if(m_creatureObject)
	{
		CreatureObject* creatureObject = m_creatureObject->asCreatureObject();
		if(creatureObject)
		{
			CustomizationData * const customizationData = creatureObject->fetchCustomizationData ();
			if (customizationData)
			{
				PaletteColorCustomizationVariable * const var = dynamic_cast<PaletteColorCustomizationVariable *>(customizationData->findVariable("/private/index_color_1"));
				if(var)
				{					
					int min, max;
					var->getRange(min,max);
					if(max >= 20) 
					{   // this potentially has a mutant cold -> hot palette.  Shift range to last 20 entries
						min = max - 20;
					}

					if(m_sessionNumber == 1
						|| (m_initialCreatureColorIndex < min || m_initialCreatureColorIndex >= max)
					)
					{
						m_initialCreatureColorIndex = (min + max) / 2;
					}

					const int sliderOffset = m_temperatureGaugeValue - ms_middleNutrientTemperatureGaugeValue;
					m_newCreatureColorIndex = m_initialCreatureColorIndex + sliderOffset;
					
					if(m_newCreatureColorIndex < min)
					{
						m_newCreatureColorIndex = min;
					}
					else if(m_newCreatureColorIndex >= max)
					{
						m_newCreatureColorIndex = max - 1;
					}

// JU_TODO: debug
#if 0
					DEBUG_REPORT_LOG(true,("***\n"));
					DEBUG_REPORT_LOG(true,("   m_sessionNumber = %d\n",m_sessionNumber));
					DEBUG_REPORT_LOG(true,("   m_initialCreatureColorIndex = %d\n",m_initialCreatureColorIndex));
					DEBUG_REPORT_LOG(true,("   m_newCreatureColorIndex = %d\n",m_newCreatureColorIndex));
					DEBUG_REPORT_LOG(true,("   min = %d\n",min));
					DEBUG_REPORT_LOG(true,("   max = %d\n",max));
					DEBUG_REPORT_LOG(true,("   m_temperatureGaugeValue = %d\n",m_temperatureGaugeValue));
					DEBUG_REPORT_LOG(true,("   sliderOffset = %d\n",sliderOffset));
					DEBUG_REPORT_LOG(true,("***\n"));
#endif
// JU_TODO: end debug
					var->setValue(m_newCreatureColorIndex);

				}
				customizationData->release();
			}
		}
	}
}

//----------------------------------------------------------------------

// slot handling
CuiWidget3dObjectListViewer* SwgCuiIncubator::findSlotViewer(const UIWidget * context)
{
	for(int i = 0; i < ms_numSlots; ++i)
	{
		if(m_slotViewer[i] == context)
		{
			return m_slotViewer[i];
		}
	}
	
	return NULL;
}

int SwgCuiIncubator::findSlotViewerIndex (const UIWidget * context)
{
	for(int i = 0; i < ms_numSlots; ++i)
	{
		if(m_slotViewer[i] == context)
		{
			return i;
		}
	}
	
	return -1;
}

void SwgCuiIncubator::setSlot(int slot, Object *obj)
{
	if(slot < 0 || slot >= ms_numSlots)
		return;

	NetworkId networkId = NetworkId::cms_invalid;
	bool viewerDraggable = false;
	if(obj)
	{
		networkId = obj->getNetworkId();
		viewerDraggable = true;
	}

	
	if(slot == 3)
	{
		resetChecksToInitialPoints();
		m_pointsToSpend = 0;
		if(obj && obj->getNetworkId() != NetworkId::cms_invalid)
		{
			std::vector<std::pair<std::string, Unicode::String> > attribs;
			bool const result = ObjectAttributeManager::getAttributes(obj->getNetworkId(), attribs, false, true);
			if(result)
			{
				for(unsigned int i = 0; i < attribs.size(); ++i)
				{
					if(attribs[i].first == ms_enzymePointsAttribName)
					{
						m_pointsToSpend = Unicode::toInt(attribs[i].second);
						if(m_pointsToSpend < 0)
						{
							m_pointsToSpend = 0;
						}
						break;

					}
					
				}
			}
		}
		distributePointsToSpend();
	}

	m_slotViewer[slot]->setObject(obj);
	m_slotViewer[slot]->SetDragable(viewerDraggable);
	ClientIncubatorManager::Slot networkIdSlot = static_cast<ClientIncubatorManager::Slot>(slot);
	ClientIncubatorManager::setNetworkId(networkIdSlot,networkId);

	updateButtonStates();
}

void SwgCuiIncubator::clearSlot(int slot)
{
	setSlot(slot, NULL);
}

void SwgCuiIncubator::updateButtonStates()
{
	// heat buttons
	m_buttonAddHeat->SetEnabled(m_temperatureGaugeValue < ms_maxNutrientTemperatureGaugeValue);
	m_buttonSubtractHeat->SetEnabled(m_temperatureGaugeValue > 0);

	// nutrient buttons
	m_buttonAddNutrients->SetEnabled(m_nutrientGaugeValue < ms_maxNutrientTemperatureGaugeValue);
	m_buttonSubtractNutrients->SetEnabled(m_nutrientGaugeValue > 0);

	// commit button
	m_buttonCommit->SetEnabled(
		!getDefensiveSkillPointsLeft() 
		&& !getIntelligenceSkillPointsLeft() 
		&& !getOffensiveSkillPointsLeft()
		&& ClientIncubatorManager::getNetworkId(ClientIncubatorManager::slot_slot1) != NetworkId::cms_invalid
		&& ClientIncubatorManager::getNetworkId(ClientIncubatorManager::slot_slot2) != NetworkId::cms_invalid
		&& ClientIncubatorManager::getNetworkId(ClientIncubatorManager::slot_slot3) != NetworkId::cms_invalid
		&& ClientIncubatorManager::getNetworkId(ClientIncubatorManager::slot_slot4) != NetworkId::cms_invalid
	);
}

//----------------------------------------------------------------------
