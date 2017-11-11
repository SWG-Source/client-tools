//======================================================================
//
// SwgCuiImageDesignerDesigner.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiImageDesignerDesigner.h"
#include "swgClientUserInterface/SwgCuiAvatarCustomizationBase_CustomizationGroup.h"

#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Os.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ImageDesignChangeMessage.h"
#include "sharedGame/CustomizationManager.h"
#include "sharedGame/CustomizationManager_MorphParameter.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/SharedImageDesignerManager.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/Controller.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "clientGame/ClientImageDesignerManager.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientGame/TangibleObject.h"
#include "clientUserInterface/CuiColorPicker.h"
#include "clientUserInterface/CuiCharacterHairManager.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsImageDesigner.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"
#include "swgClientUserInterface/SwgCuiImageDesignerRecipient.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include "UIBaseObject.h"
#include "UIButton.h"
#include "UICheckbox.h"
#include "UISliderbar.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UIVolumePage.h"
#include "UIWidget.h"

// =====================================================================

namespace SwgCuiImageDesignerDesignerNamespace
{
	float ms_timeSinceUpdate = 0.0f;

	namespace Properties
	{
		UILowerString const holoemote("holoemote");
		UILowerString const IsOnHair("IsOnHair");
	}
	const char * const cms_noviceImageDesignerSkillName = "social_imagedesigner_novice";

	bool const cms_verboseOutput = false;

	float const cms_maxTimeOutsideTent = 30.0f;
	float const cms_warnTimeIncrement = 5.0f;
	float ms_timeOutsideTent = 0.0f;
	float ms_lastWarnTime = 0.0f;
}

using namespace SwgCuiImageDesignerDesignerNamespace;

// =====================================================================

SwgCuiImageDesignerDesigner::SwgCuiImageDesignerDesigner (UIPage & page)
: SwgCuiAvatarCustomizationBase("SwgCuiImageDesignerDesigner", page),
  m_callback(new MessageDispatch::Callback),
  m_timeRemaining(NULL),
  m_buttonCancel(NULL),
  m_buttonRevert(NULL),
  m_buttonCommit(NULL),
  m_recipientId(),
  m_originalCustomizations(),
  m_forceUpdate(false),
  m_paperDollViewer(NULL),
  m_paperDoll(NULL),
  m_costTextBox(NULL),
  m_offeredMoney(NULL),
  m_holoEmoteBeehiveCheck(NULL),
  m_holoEmoteBrainstormCheck(NULL),
  m_holoEmoteImperialCheck(NULL),
  m_holoEmoteRebelCheck(NULL),
  m_holoEmoteBubbleheadCheck(NULL),
  m_holoEmoteHologlitterCheck(NULL),
  m_holoEmoteHolonotesCheck(NULL),
  m_holoEmoteSparkyCheck(NULL),
  m_holoEmoteBullhornsCheck(NULL),
  m_holoEmoteChampagneCheck(NULL),
  m_holoEmoteTechnokittyCheck(NULL),
  m_holoEmotePhonytailCheck(NULL),
  m_holoEmoteBlossomCheck(NULL),
  m_holoEmoteButterfliesCheck(NULL),
  m_holoEmoteHauntedCheck(NULL),
  m_holoEmoteHeartsCheck(NULL),
  m_holoEmoteAllCheck(NULL),
  m_holoEmoteNoneCheck(NULL),
  m_morphPageOverlay(NULL),
  m_slider1PageOverlay(NULL),
  m_slider2PageOverlay(NULL),
  m_slider3PageOverlay(NULL),
  m_slider4PageOverlay(NULL),
  m_slider5PageOverlay(NULL),
  m_slider6PageOverlay(NULL),
  m_slider7PageOverlay(NULL),
  m_colorpicker0PageOverlay(NULL),
  m_colorpicker1PageOverlay(NULL),
  m_hairPageOverlay(NULL),
  m_nameText(NULL),
  m_terminalId(),
  m_currentHoloEmote(),
  m_holoEmoteCheckboxes(),
  m_committed(false),
  m_originalHairId(),
  m_originalHairCustomizations()
{
	m_objectViewer->setPaused                (true);
	m_objectViewer->setCameraTransformToObj  (false);
	m_objectViewer->setAlterObjects        (true);
	m_objectViewer->setCameraForceTarget   (true);

	ms_timeSinceUpdate = 0.0f;

	IGNORE_RETURN(setState(MS_closeable));

	getCodeDataObject (TUIText, m_timeRemaining, "timeRemaining");
	getCodeDataObject (TUIButton, m_buttonCancel, "buttonCancel");
	getCodeDataObject (TUIButton, m_buttonRevert, "buttonRevert");
	getCodeDataObject (TUIButton, m_buttonCommit, "buttonCommit");
	getCodeDataObject (TUITextbox, m_costTextBox, "costTextBox");
	getCodeDataObject (TUIText, m_offeredMoney, "offeredMoney");
	getCodeDataObject (TUICheckbox, m_holoEmoteBeehiveCheck, "holoEmoteBeehive");
	getCodeDataObject (TUICheckbox, m_holoEmoteBrainstormCheck, "holoEmoteBrainstorm");
	getCodeDataObject (TUICheckbox, m_holoEmoteImperialCheck, "holoEmoteImperial");
	getCodeDataObject (TUICheckbox, m_holoEmoteRebelCheck, "holoEmoteRebel");
	getCodeDataObject (TUICheckbox, m_holoEmoteBubbleheadCheck, "holoEmoteBubblehead");
	getCodeDataObject (TUICheckbox, m_holoEmoteHologlitterCheck, "holoEmoteHologlitter");
	getCodeDataObject (TUICheckbox, m_holoEmoteHolonotesCheck, "holoEmoteHolonotes");
	getCodeDataObject (TUICheckbox, m_holoEmoteSparkyCheck, "holoEmoteSparky");
	getCodeDataObject (TUICheckbox, m_holoEmoteBullhornsCheck, "holoEmoteBullhorns");
	getCodeDataObject (TUICheckbox, m_holoEmoteChampagneCheck, "holoEmoteChampagne");
	getCodeDataObject (TUICheckbox, m_holoEmoteTechnokittyCheck, "holoEmoteTechnokitty");
	getCodeDataObject (TUICheckbox, m_holoEmotePhonytailCheck, "holoEmotePhonytail");
	getCodeDataObject (TUICheckbox, m_holoEmoteBlossomCheck, "holoEmoteBlossom");
	getCodeDataObject (TUICheckbox, m_holoEmoteButterfliesCheck, "holoEmoteButterflies");
	getCodeDataObject (TUICheckbox, m_holoEmoteHauntedCheck, "holoEmoteHaunted");
	getCodeDataObject (TUICheckbox, m_holoEmoteHeartsCheck, "holoEmoteHearts");
	getCodeDataObject (TUICheckbox, m_holoEmoteAllCheck, "holoEmoteAll");
	getCodeDataObject (TUICheckbox, m_holoEmoteNoneCheck, "holoEmoteNone");
	getCodeDataObject (TUIPage, m_morphPageOverlay, "overlaymorphpage");
	getCodeDataObject (TUIPage, m_slider1PageOverlay, "overlayslider1");
	getCodeDataObject (TUIPage, m_slider2PageOverlay, "overlayslider2");
	getCodeDataObject (TUIPage, m_slider3PageOverlay, "overlayslider3");
	getCodeDataObject (TUIPage, m_slider4PageOverlay, "overlayslider4");
	getCodeDataObject (TUIPage, m_slider5PageOverlay, "overlayslider5");
	getCodeDataObject (TUIPage, m_slider6PageOverlay, "overlayslider6");
	getCodeDataObject (TUIPage, m_slider7PageOverlay, "overlayslider7");
	getCodeDataObject (TUIPage, m_colorpicker0PageOverlay, "overlaycolorpicker0");
	getCodeDataObject (TUIPage, m_colorpicker1PageOverlay, "overlaycolorpicker1");
	getCodeDataObject (TUIPage, m_hairPageOverlay, "overlayhairpage");
	getCodeDataObject (TUIText, m_nameText, "nameText");

	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteBeehiveCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteBrainstormCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteImperialCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteRebelCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteBubbleheadCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteHologlitterCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteHolonotesCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteSparkyCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteBullhornsCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteChampagneCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteTechnokittyCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmotePhonytailCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteBlossomCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteButterfliesCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteHauntedCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteHeartsCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteAllCheck));
	IGNORE_RETURN(m_holoEmoteCheckboxes.insert(m_holoEmoteNoneCheck));

	ms_timeOutsideTent = 0.0f;
	ms_lastWarnTime = 0.0f;

	//set up holo emote check boxes
	for(std::set<UICheckbox *>::iterator i = m_holoEmoteCheckboxes.begin(); i != m_holoEmoteCheckboxes.end(); ++i)
	{
		(*i)->SetChecked(false);
		registerMediatorObject(*(*i), true);
	}
	m_holoEmoteNoneCheck->SetChecked(true);

	//tell the base class not to yaw the camera when selecting customizations
	setYawOnCustomizationSelection(false);

	//initialize UI fields
	m_nameText->Clear();
	m_costTextBox->SetText(Unicode::narrowToWide("0"));
	m_offeredMoney->SetText(Unicode::narrowToWide("0"));

	m_morphPageOverlay->SetVisible(false);
	m_slider1PageOverlay->SetVisible(false);
	m_slider2PageOverlay->SetVisible(false);
	m_slider3PageOverlay->SetVisible(false);
	m_slider4PageOverlay->SetVisible(false);
	m_slider5PageOverlay->SetVisible(false);
	m_slider6PageOverlay->SetVisible(false);
	m_slider7PageOverlay->SetVisible(false);
	m_colorpicker0PageOverlay->SetVisible(false);
	m_colorpicker1PageOverlay->SetVisible(false);
	m_hairPageOverlay->SetVisible(false);

	SetDisableSubPagesWhenNotSelected(true);
	//update even when not visible to count down timer
	setIsUpdating (true);
	updateTimeLeft(0);

	registerMediatorObject (*m_buttonCancel, true);
	registerMediatorObject (*m_buttonRevert, true);
	registerMediatorObject (*m_buttonCommit, true);
	registerMediatorObject (*m_costTextBox, true);
	registerMediatorObject (*m_buttonCancel, true);
	registerMediatorObject (*m_buttonRevert, true);
	registerMediatorObject (*m_buttonCommit, true);
	registerMediatorObject (*m_offeredMoney, true);

	m_callback->connect (*this, &SwgCuiImageDesignerDesigner::onImageDesignerChangeReceived, static_cast<PlayerCreatureController::Messages::ImageDesignerChangeReceived *>(0));
	m_callback->connect (*this, &SwgCuiImageDesignerDesigner::onImageDesignerCancelReceived, static_cast<PlayerCreatureController::Messages::ImageDesignerCancelReceived *>(0));
}

//----------------------------------------------------------------------

SwgCuiImageDesignerDesigner::~SwgCuiImageDesignerDesigner()
{
	m_callback->disconnect (*this, &SwgCuiImageDesignerDesigner::onImageDesignerCancelReceived, static_cast<PlayerCreatureController::Messages::ImageDesignerCancelReceived *>(0));
	m_callback->disconnect (*this, &SwgCuiImageDesignerDesigner::onImageDesignerChangeReceived, static_cast<PlayerCreatureController::Messages::ImageDesignerChangeReceived *>(0));

	delete m_callback;
	m_callback = 0;

	if(m_paperDoll)
	{
		SlottedContainer * const container = ContainerInterface::getSlottedContainer(*m_paperDoll);
		if(container)
		{
			for (ContainerIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
			{
				CachedNetworkId const & id = *containerIterator;
				ClientObject * const obj = dynamic_cast<ClientObject *>(id.getObject ());
				delete obj;
			}
		}
		delete m_paperDoll;
		m_paperDoll = 0;
	}

	m_paperDollViewer = NULL;

	m_buttonCancel = NULL;
	m_buttonRevert = NULL;
	m_buttonCommit = NULL;
	m_offeredMoney = NULL;
	m_holoEmoteBeehiveCheck = NULL;
	m_holoEmoteBrainstormCheck = NULL;
	m_holoEmoteImperialCheck = NULL;
	m_holoEmoteRebelCheck = NULL;
	m_holoEmoteBubbleheadCheck = NULL;
	m_holoEmoteHologlitterCheck = NULL;
	m_holoEmoteHolonotesCheck = NULL;
	m_holoEmoteSparkyCheck = NULL;
	m_holoEmoteBullhornsCheck = NULL;
	m_holoEmoteChampagneCheck = NULL;
	m_holoEmoteTechnokittyCheck = NULL;
	m_holoEmotePhonytailCheck = NULL;
	m_holoEmoteBlossomCheck = NULL;
	m_holoEmoteButterfliesCheck = NULL;
	m_holoEmoteHauntedCheck = NULL;
	m_holoEmoteHeartsCheck = NULL;
	m_holoEmoteAllCheck = NULL;
	m_holoEmoteNoneCheck = NULL;
	m_morphPageOverlay = NULL;
	m_slider1PageOverlay = NULL;
	m_slider2PageOverlay = NULL;
	m_slider3PageOverlay = NULL;
	m_slider4PageOverlay = NULL;
	m_slider5PageOverlay = NULL;
	m_slider6PageOverlay = NULL;
	m_slider7PageOverlay = NULL;
	m_colorpicker0PageOverlay = NULL;
	m_colorpicker1PageOverlay = NULL;
	m_hairPageOverlay = NULL;
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::performActivate()
{
	CuiManager::requestPointer (true);
	SwgCuiAvatarCustomizationBase::performActivate();
	SharedImageDesignerManager::SkillMods const & skillMods = ClientImageDesignerManager::getSkillModsForDesigner(Game::getPlayer()->getNetworkId());
	setObject(m_paperDoll, skillMods.hairSkillMod);

	//we need to ensure that the hair viewers get their color updated, but the hair group has to be active to do that
	std::string const currentGroup = getGroup();
	CustomizationGroup const * const hairGroup = findHairCustomizationGroup();
	if(hairGroup)
	{
		setGroup(Unicode::wideToNarrow(hairGroup->name));
		setGroup(currentGroup);
	}

	setIsUpdating (true);
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::performDeactivate()
{
	CuiManager::requestPointer (false);
	setObject(NULL, 0);
	SwgCuiAvatarCustomizationBase::performDeactivate();

	//update even when not visible to check for leaving the tent
	setIsUpdating (true);
}

//-----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::OnButtonPressed(UIWidget * const context)
{
	SwgCuiAvatarCustomizationBase::OnButtonPressed(context);

	//revert the current changes but keep the session open
	if(context == m_buttonRevert)
	{
		resetDefaults();

		if(m_paperDoll)
		{
			CustomizationData * const customizationData = m_paperDoll->fetchCustomizationData();
			if(customizationData)
			{
				customizationData->loadLocalDataFromString(m_originalCustomizations);
				m_forceUpdate = true;
				customizationData->release();
			}

			Object * const hair = NetworkIdManager::getObjectById(m_originalHairId);
			ClientObject * const clientHair = hair ? hair->asClientObject() : NULL;
			TangibleObject * const tangibleHair = clientHair ? clientHair->asTangibleObject() : NULL;
			if(tangibleHair)
			{
				CustomizationData * const hairCustomizationData = tangibleHair->fetchCustomizationData();
				if(hairCustomizationData)
				{
					hairCustomizationData->loadLocalDataFromString(m_originalHairCustomizations);
					hairCustomizationData->release();
				}
				CuiCharacterHairManager::replaceHair(*m_paperDoll, tangibleHair, false);
			}
			else
				CuiCharacterHairManager::replaceHair(*m_paperDoll, NULL, false);
		}

		for(std::set<UICheckbox *>::iterator i = m_holoEmoteCheckboxes.begin(); i != m_holoEmoteCheckboxes.end(); ++i)
		{
			(*i)->SetChecked(false, false);
		}
		m_holoEmoteNoneCheck->SetChecked(true, false);

		SharedImageDesignerManager::Session session;
		bool const result = SharedImageDesignerManager::getSession(Game::getPlayer()->getNetworkId(), session);
		if(result)
		{
			session.designType = ImageDesignChangeMessage::DT_COSMETIC;
			session.newHairSet = false;
			session.newHairAsset = "";
			session.accepted = false;
			session.morphChanges.clear();
			session.indexChanges.clear();
			session.holoEmote.clear();
			SharedImageDesignerManager::updateSession(session);
		}

		//update hair color for all hair viewers
		updateSelectedHairColorState();

		m_forceUpdate = true;
	}
	//send the update packet
	else if(context == m_buttonCommit)
	{
		SharedImageDesignerManager::Session session;
		bool const result = SharedImageDesignerManager::getSession(Game::getPlayer()->getNetworkId(), session);
		if(result)
		{
			if(session.designerId == session.recipientId)
			{
				SwgCuiImageDesignerRecipient * const recipientUI = safe_cast<SwgCuiImageDesignerRecipient *>(CuiMediatorFactory::getInWorkspace (CuiMediatorTypes::WS_ImageDesignerRecipient));
				if(recipientUI)
				{
					recipientUI->setAccepted(true);
					recipientUI->closeThroughWorkspace();
				}
			}
		}

		buildAndSendUpdateToServer(true);
		m_committed = true;
		closeThroughWorkspace();
	}
	//cancel the session
	else if(context == m_buttonCancel)
	{
		closeThroughWorkspace();
	}
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::OnVolumePageSelectionChanged (UIWidget * const context)
{
	SwgCuiAvatarCustomizationBase::OnVolumePageSelectionChanged(context);
	if (context == m_hairVolumePage)
	{
		//the hair has already been swapped out in the base class call
		TangibleObject const * const hair = getCurrentHair();

		//bald (no hair) may be okay
		std::string hairTemplate;
		if(hair)
		{
			hairTemplate = hair->getTemplateName();
		}
		std::string const & hairName = CustomizationManager::convertSharedHairTemplateNameToServerHairTemplateName(hairTemplate);

		SharedImageDesignerManager::Session session;
		Object * const player = Game::getPlayer();
		if(player)
		{
			bool const result = SharedImageDesignerManager::getSession(player->getNetworkId(), session);
			if(result)
			{
				session.newHairSet = true;
				session.newHairAsset = hairName;
				SharedImageDesignerManager::updateSession(session);
				m_forceUpdate = true;
			}
		}

		checkColorPalettes();
	}
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::OnSliderbarChanged(UIWidget * const context)
{
	if(!context)
		return;

	if (!context->IsA (TUISliderbar))
		return;

	UISliderbar * const slider = NON_NULL (safe_cast<UISliderbar *>(context));

	bool isMorphH = false;
	bool isMorphV = false;

	CustomizationManager::MorphParameter * morphParam = 0;

	if (findMorphSlider (*safe_cast<UISliderbar *>(context), isMorphH, isMorphV, morphParam))
	{
		if (morphParam)
		{
			float value = 0.0f;
			value = dataValueToFloat (slider->GetValue (), static_cast<float>(morphParam->discreteRange));
			SharedImageDesignerManager::Session session;
			bool const result = SharedImageDesignerManager::getSession(Game::getPlayer()->getNetworkId(), session);
			if(result)
			{
				DEBUG_REPORT_LOG(cms_verboseOutput, ("[%s]=%.3f before\n", Unicode::wideToNarrow(morphParam->name).c_str(), session.morphChanges[Unicode::wideToNarrow(morphParam->name)]));
				session.morphChanges[Unicode::wideToNarrow(morphParam->name)] = value;
				CreatureObject * const recipient = getRecipientCreature();
				if(recipient)
				{
					SharedImageDesignerManager::updateDesignType(session, CustomizationManager::getSharedSpeciesGender(*recipient));
				}
				else
				{
					DEBUG_WARNING(true, ("SwgCuiImageDesignerDesigner::OnSliderbarChanged - unable to get recipient creature"));
				}
				SharedImageDesignerManager::updateSession(session);
				DEBUG_REPORT_LOG(cms_verboseOutput, ("[%s]=%.3f after\n", Unicode::wideToNarrow(morphParam->name).c_str(), session.morphChanges[Unicode::wideToNarrow(morphParam->name)]));
				m_forceUpdate = true;
			}
		}
	}

	SwgCuiAvatarCustomizationBase::OnSliderbarChanged(context);
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::OnTextboxChanged(UIWidget * const context)
{
	if (context == m_costTextBox)
	{
		SharedImageDesignerManager::Session session;
		bool const result = SharedImageDesignerManager::getSession(Game::getPlayer()->getNetworkId(), session);
		if(result)
		{
			int value = m_costTextBox->GetNumericIntegerValue();
			if(value < 0)
				value = 0;
			session.designerRequiredCredits = value;
			SharedImageDesignerManager::updateSession(session);
			m_forceUpdate = true;
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::OnCheckboxSet(UIWidget * const context)
{
	SharedImageDesignerManager::Session session;
	bool const result = SharedImageDesignerManager::getSession(Game::getPlayer()->getNetworkId(), session);
	if(!result)
		return;

	//check holo emote check boxes
	for(std::set<UICheckbox *>::iterator i = m_holoEmoteCheckboxes.begin(); i != m_holoEmoteCheckboxes.end(); ++i)
	{
		UICheckbox * const check = *i;
		if(context == check)
		{
			if(!check->GetPropertyNarrow(Properties::holoemote, session.holoEmote))
				session.holoEmote.clear();
			setSingleHoloEmoteCheckbox(check);
			SharedImageDesignerManager::updateSession(session);
			m_forceUpdate = true;
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::setSingleHoloEmoteCheckbox(UICheckbox * const checkbox) const
{
	//clear all holo emote check boxes
	for(std::set<UICheckbox *>::iterator i = m_holoEmoteCheckboxes.begin(); i != m_holoEmoteCheckboxes.end(); ++i)
	{
		(*i)->SetChecked(false, false);
	}

	checkbox->SetChecked(true, false);
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::OnCheckboxUnset(UIWidget * const context)
{
	SharedImageDesignerManager::Session session;
	bool const result = SharedImageDesignerManager::getSession(Game::getPlayer()->getNetworkId(), session);
	if(!result)
	{
		return;
	}

	//check holo emote check boxes
	for(std::set<UICheckbox *>::iterator i = m_holoEmoteCheckboxes.begin(); i != m_holoEmoteCheckboxes.end(); ++i)
	{
		if(context == *i)
		{
			session.holoEmote.clear();
			SharedImageDesignerManager::updateSession(session);
			m_forceUpdate = true;
		}
	}
}

//----------------------------------------------------------------------

bool SwgCuiImageDesignerDesigner::close()
{
	//tell the server to cancel the session if necessary
	if(!m_committed)
	{
		CreatureObject * const designer = Game::getPlayerCreature();
		if(designer)
		{
			SharedImageDesignerManager::Session session;
			bool const result = SharedImageDesignerManager::getSession(designer->getNetworkId(), session);
			if(result)
			{
				SharedImageDesignerManager::endSession(session.designerId);
				Object const * const designerObj = NetworkIdManager::getObjectById(session.designerId);
				if(designerObj && designerObj == designer)
				{
					ImageDesignChangeMessage * msg = new ImageDesignChangeMessage();
					SharedImageDesignerManager::populateChangeMessage(session, *msg);
					msg->setOrigin(ImageDesignChangeMessage::O_DESIGNER);
					msg->setAccepted(false);
					designer->getController()->appendMessage (CM_imageDesignerCancel, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
				}
			}
		}
	}

	return CuiMediator::close();
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::update (float const deltaTimeSecs)
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if(!player)
		return;

	CreatureObject * const recipientObj = getRecipientCreature();
	if(!recipientObj)
		return;

	CachedNetworkId target;
	if(CuiCombatManager::isInCombat(player, target) || CuiCombatManager::isInCombat(recipientObj, target))
	{
		CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIdsImageDesigner::canceled_session_by_entering_combat.localize ());
		closeThroughWorkspace();
		return;
	}

	time_t timeLeft = SharedImageDesignerManager::getTimeRemaining(player->getNetworkId());

	if(timeLeft < 0)
		timeLeft = 0;
	updateTimeLeft(timeLeft);

	SharedImageDesignerManager::Session session;
	bool result = SharedImageDesignerManager::getSession(player->getNetworkId(), session);
	if(result)
	{
		//check color changes every frame (since a colorpickers are shared between groups)
		for (int i = 0; i < 2; ++i)
		{
			if (m_colorPicker[i]->isActive () && m_colorPicker[i]->checkAndResetUSerChanged ())
			{
				//figure out what customization changed, and the new value, save it off for sending to recipient
				bool isOnHair = false;
				m_colorPicker[i]->getPage().GetPropertyBoolean(Properties::IsOnHair, isOnHair);
				std::string const & colorPickerVariables = m_colorPicker [i]->getTargetVariable();
				std::string const & customizationName = CustomizationManager::getCustomizationNameForVariables(CustomizationManager::getSharedSpeciesGender(*recipientObj), colorPickerVariables, isOnHair);
				if(!customizationName.empty())
				{
					CustomizationManager::Customization customization;
					result = CustomizationManager::getCustomization(CustomizationManager::getSharedSpeciesGender(*recipientObj), customizationName, customization);
					if(result)
					{
						bool queryCustomizationData = true;
						//use the hair if it's the one to be queried
						TangibleObject * objectToQuery = recipientObj;
						if(customization.isVarHairColor)
						{
							ClientObject * const hair = recipientObj->getHairObject();
							TangibleObject * const tangibleHair = hair ? hair->asTangibleObject() : NULL;
							if(tangibleHair)
								objectToQuery = tangibleHair;
							else
							{
								//we don't currently have a hair, but we have a hair color that we want to store
								DEBUG_REPORT_LOG(cms_verboseOutput, ("[%s]=%d before\n", customizationName.c_str(), session.indexChanges[customizationName]));
								session.indexChanges[customizationName] = m_colorPicker [i]->getValue();
								SharedImageDesignerManager::updateDesignType(session, CustomizationManager::getSharedSpeciesGender(*recipientObj));
								SharedImageDesignerManager::updateSession(session);
								DEBUG_REPORT_LOG(cms_verboseOutput, ("[%s]=%d after\n", customizationName.c_str(), session.indexChanges[customizationName]));
								m_forceUpdate = true;
								queryCustomizationData = false;
							}
						}

						if(queryCustomizationData)
						{
							CustomizationData * const customizationData = objectToQuery->fetchCustomizationData();
							if(customizationData)
							{
								CustomizationVariable * const cv = CustomizationManager::findVariable (*customizationData, customization.variables);
								PaletteColorCustomizationVariable * const paletteVariable = dynamic_cast<PaletteColorCustomizationVariable *>(cv);
								if(paletteVariable)
								{
									int const originalValue = paletteVariable->getValue();
									int const newValue = m_colorPicker [i]->getValue();
									m_forceUpdate = true;
									if(newValue != originalValue)
									{
										DEBUG_REPORT_LOG(cms_verboseOutput, ("[%s]=%d before\n", customizationName.c_str(), session.indexChanges[customizationName]));
										session.indexChanges[customizationName] = m_colorPicker [i]->getValue();
										SharedImageDesignerManager::updateDesignType(session, CustomizationManager::getSharedSpeciesGender(*recipientObj));
										SharedImageDesignerManager::updateSession(session);
										DEBUG_REPORT_LOG(cms_verboseOutput, ("[%s]=%d after\n", customizationName.c_str(), session.indexChanges[customizationName]));
									}
									else
									{
										IGNORE_RETURN(session.indexChanges.erase(customizationName));
										SharedImageDesignerManager::updateSession(session);
									}
								}
								else
								{
									//the worn hair doesn't have this customization, but we want it on the new hair
									DEBUG_REPORT_LOG(cms_verboseOutput, ("[%s]=%d before\n", customizationName.c_str(), session.indexChanges[customizationName]));
									session.indexChanges[customizationName] = m_colorPicker [i]->getValue();
									SharedImageDesignerManager::updateDesignType(session, CustomizationManager::getSharedSpeciesGender(*recipientObj));
									SharedImageDesignerManager::updateSession(session);
									DEBUG_REPORT_LOG(cms_verboseOutput, ("[%s]=%d after\n", customizationName.c_str(), session.indexChanges[customizationName]));
									m_forceUpdate = true;
								}
								customizationData->release();
							}
						}//end queryCustomizationData
					}//end result
				}//end !customizationName.empty()
			}//end colorpicker changed
		}//end for

		//if they're supposed to be in a building, but they aren't, cancel the session after a grace period
		if(session.terminalId != NetworkId::cms_invalid)
		{
			bool playerInCorrectCell = false;
			CellProperty const * const cellProp = player->getParentCell();
			if(cellProp)
			{
				PortalProperty const * const portalProp = cellProp->getPortalProperty();
				if(portalProp)
				{
					Object const & owner = portalProp->getOwner();
					if(owner.getNetworkId() == m_terminalId)
					{
						ms_timeOutsideTent = 0.0f;
						ms_lastWarnTime = 0.0f;
						playerInCorrectCell = true;
					}
				}
			}

			if(!playerInCorrectCell)
			{
				ms_timeOutsideTent += deltaTimeSecs;

				//if they've been outside tent too long, cancel the session
				if(ms_timeOutsideTent > cms_maxTimeOutsideTent)
				{
					CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIdsImageDesigner::canceled_session_by_leaving_tent.localize ());
					closeThroughWorkspace();
				}
				//warn the player to return to the tent
				else if(ms_timeOutsideTent > ms_lastWarnTime + cms_warnTimeIncrement)
				{
					ms_lastWarnTime += cms_warnTimeIncrement;
					ProsePackage pp;
					pp.stringId   = CuiStringIdsImageDesigner::return_to_tent_warning_prose;
					pp.digitInteger = static_cast<int>(cms_maxTimeOutsideTent - ms_lastWarnTime);
					Unicode::String resultStr;
					IGNORE_RETURN(ProsePackageManagerClient::appendTranslation(pp, resultStr));
					CuiSystemMessageManager::sendFakeSystemMessage (resultStr);
				}
			}
		}
	}

	if(m_forceUpdate)
	{
		buildAndSendUpdateToServer(false);
		m_forceUpdate = false;
	}

	SwgCuiAvatarCustomizationBase::update(deltaTimeSecs);
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::setupPage()
{
	CreatureObject const * const player = Game::getPlayerCreature ();
	DEBUG_FATAL(!player, ("No player in SwgCuiImageDesignerDesigner::setupPage"));
	if(!player) //lint !e774 bool always evalutates true (necessary for release build)
		return;

	SharedImageDesignerManager::Session newSession;
	newSession.designerId = player->getNetworkId();
	newSession.recipientId = m_recipientId;
	newSession.startingTime = Os::getRealSystemTime();
	newSession.terminalId = m_terminalId;
	SharedImageDesignerManager::SkillMods const & skillMods = ClientImageDesignerManager::getSkillModsForDesigner(player->getNetworkId());
	newSession.faceSkillMod = skillMods.faceSkillMod;
	newSession.bodySkillMod = skillMods.bodySkillMod;
	newSession.markingsSkillMod = skillMods.markingsSkillMod;
	newSession.hairSkillMod = skillMods.hairSkillMod;
	SharedImageDesignerManager::startSession(newSession);

	CreatureObject * const recipientCreature = getRecipientCreature();
	if(recipientCreature)
	{
		m_paperDoll = SwgCuiAvatarCreationHelper::duplicateCreatureWithClothesAndCustomization(*recipientCreature);
		if(!m_paperDoll)
			return;

		m_nameText->SetText(recipientCreature->getLocalizedName());
		CustomizationData const * const customizationData = recipientCreature->fetchCustomizationData();
		if(customizationData)
		{
			m_originalCustomizations = customizationData->writeLocalDataToString();
			customizationData->release();
		}

		ClientObject * const hair = m_paperDoll->getHairObject();
		m_originalHairId = hair ? hair->getNetworkId() : NetworkId::cms_invalid;
		if(m_originalHairId != NetworkId::cms_invalid)
		{
			TangibleObject * const tangibleHair = hair ? hair->asTangibleObject() : NULL;
			if(tangibleHair)
			{
				CustomizationData * const hairCustomizationData = tangibleHair->fetchCustomizationData();
				if(hairCustomizationData)
				{
					m_originalHairCustomizations = hairCustomizationData->writeLocalDataToString();
					hairCustomizationData->release();
				}
			}
		}
	}

	//check holo emote check boxes, block out any the designer doesn't have the skill to modify
	for(std::set<UICheckbox *>::iterator i = m_holoEmoteCheckboxes.begin(); i != m_holoEmoteCheckboxes.end(); ++i)
	{
		Unicode::String holoEmoteWide;
		IGNORE_RETURN((*i)->GetProperty (UILowerString("holoemote"), holoEmoteWide));
		int const requiredHairSkillMod = CustomizationManager::getRequiredHairSkillForHoloemote(Unicode::wideToNarrow(holoEmoteWide));
		if(requiredHairSkillMod > skillMods.hairSkillMod)
		{
			UIBaseObject const * const parent = (*i)->GetParent();
			if(parent)
			{
				UIWidget * const overlay = dynamic_cast<UIWidget *>(parent->GetChild("Overlay"));
				if(overlay)
				{
					overlay->SetVisible(true);
				}
			}
		}
	}

	if(Game::getSinglePlayer())
	{
		//single player
		setRecipient(player->getNetworkId());
	}
	else
	{
		//multi player
		//make sure player has SOME imagedesigner skillmod
		if(skillMods.bodySkillMod == 0 && skillMods.faceSkillMod == 0 && skillMods.markingsSkillMod == 0 && skillMods.hairSkillMod == 0)
		{
			CuiMessageBox::createInfoBox(CuiStringIdsImageDesigner::noSkill.localize());
			closeThroughWorkspace();
			return;
		}

		//make sure the designer has some target
		if(!getRecipientCreature())
		{
			CuiMessageBox::createInfoBox(CuiStringIdsImageDesigner::noTarget.localize());
			closeThroughWorkspace();
			return;
		}
	}

	setAssociatedObjectId(getRecipientId());
	setMaxRangeFromObject(16.0f);
	buildAndSendUpdateToServer(false);
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::setRecipient(NetworkId const & recipientId)
{
	m_recipientId = recipientId;
	setupPage();
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::setTerminal(NetworkId const & terminalId)
{
	m_terminalId = terminalId;
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::setCurrentHoloemote(std::string const & currentHoloEmote)
{
	m_currentHoloEmote = currentHoloEmote;
}

//----------------------------------------------------------------------

NetworkId const & SwgCuiImageDesignerDesigner::getRecipientId() const
{
	return m_recipientId;
}

//----------------------------------------------------------------------

CreatureObject * SwgCuiImageDesignerDesigner::getRecipientCreature() const
{
	Object * const o = NetworkIdManager::getObjectById(m_recipientId);
	ClientObject * const co = o ? o->asClientObject() : NULL;
	return co ? co->asCreatureObject() : NULL;
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::updateTimeLeft(time_t const timeLeft)
{
	if(timeLeft <= 0)
	{
		m_timeRemaining->SetText(CuiStringIdsImageDesigner::ready.localize());
	}
	else
	{
		Unicode::String str;
		CuiUtils::FormatTimeDuration(str, static_cast<unsigned int>(timeLeft), false, false, true, true, true);
		m_timeRemaining->SetText(str);
	}
	if(timeLeft > 0 || !isEnoughMoneyOffered())
		m_buttonCommit->SetEnabled(false);
	else
		m_buttonCommit->SetEnabled(true);
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::buildAndSendUpdateToServer(bool const accepted) const
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if(!player)
		return;

	SharedImageDesignerManager::Session session;
	bool const result = SharedImageDesignerManager::getSession(player->getNetworkId(), session);
	DEBUG_REPORT_LOG(cms_verboseOutput, ("Sending session...\n"));
	if(result)
	{
		for(std::map<std::string, float>::iterator i = session.morphChanges.begin(); i != session.morphChanges.end(); ++i)
			DEBUG_REPORT_LOG(cms_verboseOutput, ("[%s]=%.3f\n", i->first.c_str(), i->second));
		for(std::map<std::string, int>::iterator j = session.indexChanges.begin(); j != session.indexChanges.end(); ++j)
			DEBUG_REPORT_LOG(cms_verboseOutput, ("[%s]=%d\n", j->first.c_str(), j->second));

		ImageDesignChangeMessage * const msg = new ImageDesignChangeMessage;
		SharedImageDesignerManager::populateChangeMessage(session, *msg);
		msg->setOrigin(ImageDesignChangeMessage::O_DESIGNER);
		msg->setAccepted(accepted);
		player->getController()->appendMessage (CM_imageDesignerChange, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
	DEBUG_REPORT_LOG(cms_verboseOutput, ("End session\n"));
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::onImageDesignerChangeReceived(PlayerCreatureController::Messages::ImageDesignerChangeReceived::Payload const & payload)
{
	if(Game::getPlayer()->getNetworkId() != payload.designerId)
		return;

	char buf[256];
	_itoa(payload.recipientPaidCredits, buf, 10);
	m_offeredMoney->SetText(Unicode::narrowToWide(buf));
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::onImageDesignerCancelReceived(PlayerCreatureController::Messages::ImageDesignerChangeReceived::Payload const & payload)
{
	if(Game::getPlayer()->getNetworkId() != payload.designerId)
		return;

	SharedImageDesignerManager::endSession(payload.designerId);
	//no need to tell server
	m_committed = true;
	closeThroughWorkspace();
}

//----------------------------------------------------------------------

bool SwgCuiImageDesignerDesigner::isEnoughMoneyOffered() const
{
	std::string const & offeredStr = Unicode::wideToNarrow(m_offeredMoney->GetLocalText());
	int const offered = atoi(offeredStr.c_str());
	int const required = m_costTextBox->GetNumericIntegerValue();
	return (offered >= required);
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::updateSelectedHairColorState()
{
	SwgCuiAvatarCustomizationBase::updateSelectedHairColorState();
	setColorPickerColumnAndMaxIndexes();
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::setGroup(std::string const & groupName)
{
	SwgCuiAvatarCustomizationBase::setGroup(groupName);
	setColorPickerColumnAndMaxIndexes();

	CustomizationManager::CustomizationGroupSharedData const & groupSharedData = CustomizationManager::getCustomizationGroupSharedData(groupName);
	//block out hair page if it's not the hair group
	if(groupSharedData.isHair)
	{
		m_hairPageOverlay->SetVisible(false);
	}
	else
	{
		m_hairPageOverlay->SetVisible(true);
	}

	SharedImageDesignerManager::SkillMods const & designerSkills = ClientImageDesignerManager::getSkillModsForDesigner(Game::getPlayer()->getNetworkId());
	CreatureObject * const recipientObj = getRecipientCreature();
	if(!recipientObj)
		return;
	std::string const & recipientSpeciesGender = CustomizationManager::getSharedSpeciesGender(*recipientObj);
	UIWidget * overlay = NULL;

	//block out any slider that the designer doesn't have the skill to change
	for(SliderVector::const_iterator i = m_morphSlidersH->begin(); i != m_morphSlidersH->end(); ++i)
	{
		UISliderbar const * const slider = *i;
		if(slider)
		{
			UIBaseObject const * const parent = slider->GetParent();
			if(parent)
			{
				overlay = dynamic_cast<UIWidget *>(parent->GetChild("Overlay"));
				if(overlay)
				{
					overlay->SetVisible(true);
					bool isMorphH = false;
					bool isMorphV = false;
					CustomizationManager::MorphParameter * morphParam = 0;
					bool const result = findMorphSlider (*slider, isMorphH, isMorphV, morphParam, false);
					if(result && morphParam)
					{
						std::string const & customizationName = Unicode::wideToNarrow(morphParam->name);
						bool const validCustomizationWithSkill = SharedImageDesignerManager::isMorphChangeValid(customizationName, 0.0f, designerSkills, recipientSpeciesGender);
						if(validCustomizationWithSkill)
								overlay->SetVisible(false);
					}
				}
			}
		}
	}

	checkColorPalettes();
}

//----------------------------------------------------------------------

/** Use data from SharedImageDesignerManager to determine and set the correct number of columns and maximum index to show for the current colorpickers
*/
void SwgCuiImageDesignerDesigner::setColorPickerColumnAndMaxIndexes() const
{
	SharedImageDesignerManager::SkillMods const & designerSkills = ClientImageDesignerManager::getSkillModsForDesigner(Game::getPlayer()->getNetworkId());

	for(int i = 0; i < 2; ++i)
	{
		if(m_colorPicker[i])
		{
			PaletteArgb const * const palette = m_colorPicker[i]->getPalette();
			if(palette)
			{
				SharedImageDesignerManager::PaletteValues const paletteValues = SharedImageDesignerManager::getPaletteValuesForPaletteAndSkillMods(palette, designerSkills);
				m_colorPicker[i]->setForceColumns(paletteValues.columns);
				m_colorPicker[i]->setMaximumPaletteIndex(paletteValues.maxIndex);
				m_colorPicker[i]->reset();
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerDesigner::checkColorPalettes() const
{
	CreatureObject * const recipientObj = getRecipientCreature();
	if(!recipientObj)
		return;
	std::string const & recipientSpeciesGender = CustomizationManager::getSharedSpeciesGender(*recipientObj);
	SharedImageDesignerManager::SkillMods const & designerSkills = ClientImageDesignerManager::getSkillModsForDesigner(Game::getPlayer()->getNetworkId());

	//block out any color picker that the designer doesn't have the skill to change
	for(int j = 0; j < 2; ++j)
	{
		if(m_colorPicker[j])
		{
			UIPage const & page = m_colorPicker[j]->getPage();
			UIWidget * const overlay = dynamic_cast<UIWidget *>(page.GetChild("Overlay"));
			if(overlay)
			{
				overlay->SetVisible(true);
				bool isOnHair = false;
				m_colorPicker[j]->getPage().GetPropertyBoolean(Properties::IsOnHair, isOnHair);
				std::string const & colorPickerVariables = m_colorPicker[j]->getTargetVariable();
				std::string const & customizationName = CustomizationManager::getCustomizationNameForVariables(CustomizationManager::getSharedSpeciesGender(*recipientObj), colorPickerVariables, isOnHair);
				bool const validCustomizationWithSkill = SharedImageDesignerManager::hasSkillForCustomization(customizationName, recipientSpeciesGender, designerSkills);
				if(validCustomizationWithSkill)
					overlay->SetVisible(false);
			}
		}
	}
}

//======================================================================

