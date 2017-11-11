//======================================================================
//
// SwgCuiImageDesignerRecipient.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiImageDesignerRecipient.h"

#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/ClientImageDesignerManager.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/PlayerMoneyManagerClient.h"
#include "clientGame/ProsePackageManagerClient.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientGame/TangibleObject.h"
#include "clientUserInterface/CuiCharacterHairManager.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringIdsImageDesigner.h"
#include "clientUserInterface/CuiStringIdsTrade.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/CustomizationManager.h"
#include "sharedGame/CustomizationManager_MorphParameter.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/SharedImageDesignerManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Container.h"
#include "sharedObject/Controller.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/SlottedContainer.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"
#include "swgClientUserInterface/SwgCuiAvatarCustomizationBase.h"
#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComposite.h"
#include "UIText.h"
#include "UITextbox.h"

// =====================================================================

namespace SwgCuiImageDesignerRecipientNamespace
{
	namespace Properties
	{
		const UILowerString DefaultViewerPitch = UILowerString ("DefaultViewerPitch");
		const UILowerString DefaultViewerYaw   = UILowerString ("DefaultViewerYaw");
	}

	bool const cms_verboseOutput = false;

	float const cms_maxTimeOutsideTent = 30.0f;
	float const cms_warnTimeIncrement = 5.0f;
	float ms_timeOutsideTent = 0.0f;
	float ms_lastWarnTime = 0.0f;
}

using namespace SwgCuiImageDesignerRecipientNamespace;

// =====================================================================

SwgCuiImageDesignerRecipient::SwgCuiImageDesignerRecipient (UIPage & page)
: CuiMediator("SwgCuiImageDesignerRecipient", page),
  UIEventCallback(),
  m_callback(new MessageDispatch::Callback),
  m_designerId(),
  m_viewerBefore(NULL),
  m_viewerAfter(NULL),
  m_beforeDoll(NULL),
  m_afterDoll(NULL),
  m_rejectButton(NULL),
  m_acceptButton(NULL),
  m_originalCustomizations(),
  m_timeLeft(NULL),
  m_acceptedByDesigner(NULL),
  m_cost(NULL),
  m_purchasedHoloEmote(NULL),
  m_offeredMoneyTextBox(NULL),
  m_bodyCheckBox(NULL),
  m_faceCheckBox(NULL),
  m_dataComposite(NULL),
  m_terminalId(),
  m_currentHoloEmote(),
  m_acceptedByDesignerBool(false),
  m_committed(false),
  m_afterDollOriginalHairId(),
  m_originalHairCustomizations(),
  m_caption(NULL)
{
	IGNORE_RETURN(setState(MS_closeable));

	//get the before viewer
	UIWidget * viewer = 0;
	getCodeDataObject(TUIWidget, viewer, "viewerBefore");
	m_viewerBefore = NON_NULL(dynamic_cast<CuiWidget3dObjectListViewer *>(viewer));
	m_viewerBefore->SetLocalTooltip(CuiStringIds::tooltip_viewer_3d_controls.localize ());
	m_viewerBefore->SetPropertyFloat(Properties::DefaultViewerPitch, m_viewerBefore->getCameraPitch ());
	m_viewerBefore->SetPropertyFloat(Properties::DefaultViewerYaw,   m_viewerBefore->getCameraYaw   ());

	//get the after viewer
	getCodeDataObject(TUIWidget,viewer, "viewerAfter");
	m_viewerAfter = NON_NULL(dynamic_cast<CuiWidget3dObjectListViewer *>(viewer));
	m_viewerAfter->SetLocalTooltip(CuiStringIds::tooltip_viewer_3d_controls.localize ());
	m_viewerAfter->SetPropertyFloat(Properties::DefaultViewerPitch, m_viewerAfter->getCameraPitch ());
	m_viewerAfter->SetPropertyFloat(Properties::DefaultViewerYaw,   m_viewerAfter->getCameraYaw   ());

	getCodeDataObject(TUIButton, m_rejectButton, "buttonReject");
	getCodeDataObject(TUIButton, m_acceptButton, "buttonAccept");
	getCodeDataObject (TUIText, m_timeLeft, "labelTimeLeft");
	getCodeDataObject (TUIText, m_acceptedByDesigner, "labelAcceptedByDesigner");
	getCodeDataObject (TUIText, m_cost, "labelCost");
	getCodeDataObject (TUIText, m_purchasedHoloEmote, "labelPurchasedHoloEmotes");
	getCodeDataObject (TUITextbox, m_offeredMoneyTextBox, "paymentTextBox");
	getCodeDataObject (TUICheckbox, m_bodyCheckBox, "bodyCheckBox");
	getCodeDataObject (TUICheckbox, m_faceCheckBox, "faceCheckBox");
	getCodeDataObject (TUIComposite, m_dataComposite, "dataComposite");
	getCodeDataObject (TUIText, m_caption, "caption");

	//clear out the UI elements
	m_timeLeft->SetText(Unicode::narrowToWide("0"));
	m_acceptedByDesigner->SetText(SharedStringIds::no.localize());
	m_cost->SetText(Unicode::narrowToWide("0"));
	m_purchasedHoloEmote->Clear();
	m_offeredMoneyTextBox->SetText(Unicode::narrowToWide("0"));

	//create a "before" copy of the avatar
	m_beforeDoll = SwgCuiAvatarCreationHelper::duplicateCreatureWithClothesAndCustomization(*Game::getPlayerCreature());
	if(m_beforeDoll)
	{
		Appearance * const app = m_beforeDoll->getAppearance();
		if(app)
		{
			SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2();
			if(skelApp)
			{
				skelApp->setDetailLevel(0);
			}
		}

		m_viewerBefore->addObject(*m_beforeDoll);
		m_viewerBefore->setCameraLookAtBone("root");
		m_viewerBefore->setCameraCompensateScale(true);
		m_viewerBefore->setViewDirty(true);
		m_viewerBefore->setCameraLookAtBoneDirty(true);
		m_viewerBefore->setCameraForceTarget(true);
		m_viewerBefore->recomputeZoom();
		m_viewerBefore->setCameraForceTarget(false);
	}

	//create an "after" copy of the avatar
	m_afterDoll = SwgCuiAvatarCreationHelper::duplicateCreatureWithClothesAndCustomization(*Game::getPlayerCreature());
	if(m_afterDoll)
	{
		Appearance * const app = m_afterDoll->getAppearance();
		if(app)
		{
			SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2();
			if(skelApp)
			{
				skelApp->setDetailLevel(0);
			}
		}
		
		m_viewerAfter->addObject(*m_afterDoll);
		m_viewerAfter->setCameraLookAtBone("root");
		m_viewerAfter->setCameraCompensateScale(true);
		m_viewerAfter->setViewDirty(true);
		m_viewerAfter->setCameraLookAtBoneDirty(true);
		m_viewerAfter->setCameraForceTarget(true);
		m_viewerAfter->recomputeZoom();
		m_viewerAfter->setCameraForceTarget(false);

		ClientObject * const hair = m_afterDoll->getHairObject();
		m_afterDollOriginalHairId = hair ? hair->getNetworkId() : NetworkId::cms_invalid;
		if(m_afterDollOriginalHairId != NetworkId::cms_invalid)
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

	//update even when not visible to count down timer
	setIsUpdating (true);
	updateTimeLeft(0);

	setViewersView("root", 1.0f);

	m_callback->connect (*this, &SwgCuiImageDesignerRecipient::onImageDesignerChangeReceived, static_cast<PlayerCreatureController::Messages::ImageDesignerChangeReceived *>(0));
	m_callback->connect (*this, &SwgCuiImageDesignerRecipient::onImageDesignerCancelReceived, static_cast<PlayerCreatureController::Messages::ImageDesignerCancelReceived *>(0));

	registerMediatorObject (*m_rejectButton, true);
	registerMediatorObject (*m_acceptButton, true);
	registerMediatorObject (*m_offeredMoneyTextBox, true);
	registerMediatorObject (*m_bodyCheckBox, true);
	registerMediatorObject (*m_faceCheckBox, true);
}

//----------------------------------------------------------------------

SwgCuiImageDesignerRecipient::~SwgCuiImageDesignerRecipient()
{
	m_callback->disconnect (*this, &SwgCuiImageDesignerRecipient::onImageDesignerCancelReceived, static_cast<PlayerCreatureController::Messages::ImageDesignerCancelReceived *>(0));
	m_callback->disconnect (*this, &SwgCuiImageDesignerRecipient::onImageDesignerChangeReceived, static_cast<PlayerCreatureController::Messages::ImageDesignerChangeReceived *>(0));

	delete m_callback;
	m_callback = NULL;

	m_viewerBefore = NULL;
	m_viewerAfter = NULL;

	//delete the before doll and all of it's items
	if(m_beforeDoll)
	{
		SlottedContainer * const container = ContainerInterface::getSlottedContainer(*m_beforeDoll);
		if(container)
		{
			for (ContainerIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
			{
				CachedNetworkId const & id = *containerIterator;
				ClientObject * const obj = dynamic_cast<ClientObject *>(id.getObject ());
				delete obj;
			}
		}
		delete m_beforeDoll;
		m_beforeDoll = 0;
	}

	//delete the after doll and all of it's items
	if(m_afterDoll)
	{
		SlottedContainer * const container = ContainerInterface::getSlottedContainer(*m_afterDoll);
		if(container)
		{
			for (ContainerIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
			{
				CachedNetworkId const & id = *containerIterator;
				ClientObject * const obj = dynamic_cast<ClientObject *>(id.getObject ());
				delete obj;
			}
		}
		delete m_afterDoll;
		m_afterDoll = NULL;
	}

	m_rejectButton = NULL;
	m_acceptButton = NULL;
	m_offeredMoneyTextBox = NULL;
	m_bodyCheckBox = NULL;
	m_faceCheckBox = NULL;
	m_dataComposite = NULL;
	m_caption = NULL;
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerRecipient::performActivate()
{
	CuiManager::requestPointer (true);
	float f = 0.0f;
	if (m_viewerBefore->GetPropertyFloat (Properties::DefaultViewerPitch, f))
		m_viewerBefore->setCameraPitch (f);
	if (m_viewerBefore->GetPropertyFloat (Properties::DefaultViewerYaw, f))
		m_viewerBefore->setCameraYaw (f, true);
	m_viewerBefore->setPaused (false);

	f = 0.0f;
	if (m_viewerAfter->GetPropertyFloat (Properties::DefaultViewerPitch, f))
		m_viewerAfter->setCameraPitch (f);
	if (m_viewerAfter->GetPropertyFloat (Properties::DefaultViewerYaw, f))
		m_viewerAfter->setCameraYaw (f, true);
	m_viewerAfter->setPaused (false);
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerRecipient::performDeactivate()
{
	CuiManager::requestPointer (false);
	m_viewerBefore->setPaused (true);
	m_viewerAfter->setPaused (true);
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerRecipient::OnButtonPressed(UIWidget * const context)
{
	//tell the server to cancel the session if necessary
	if(context == m_rejectButton)
	{
		closeThroughWorkspace ();
	}
	//send the update packet
	else if(context == m_acceptButton)
	{
		buildAndSendUpdateToServer(true);
		m_committed = true;
		closeThroughWorkspace ();
	}
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerRecipient::OnTextboxChanged(UIWidget * const context)
{
	//update the offered money, send server update if necessary
	if (context == m_offeredMoneyTextBox)
	{
		SharedImageDesignerManager::Session session;
		bool const result = SharedImageDesignerManager::getSession(getDesignerId(), session);
		if(result)
		{
			int value = m_offeredMoneyTextBox->GetNumericIntegerValue();
			int finalValue = checkMoney();
			if(value != finalValue)
			{
				char buffer[256];
				_itoa(finalValue, buffer, 10);
				m_offeredMoneyTextBox->SetLocalText(Unicode::narrowToWide(buffer));
			}
			session.recipientPaidCredits = value;
			SharedImageDesignerManager::updateSession(session);
			buildAndSendUpdateToServer(false);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerRecipient::OnCheckboxSet(UIWidget * const context)
{
	if(context == m_bodyCheckBox)
	{
		setViewersView("root", 1.0f);
	}
	else if(context == m_faceCheckBox)
	{
		setViewersView("head", 1.1f);
	}
}

//----------------------------------------------------------------------

bool SwgCuiImageDesignerRecipient::close()
{
	CreatureObject * const player = Game::getPlayerCreature();

	//cancel the session if it's not committed
	if(!m_committed && player)
	{
		SharedImageDesignerManager::Session session;
		bool const result = SharedImageDesignerManager::getSession(getDesignerId(), session);
		if(result)
		{
			SharedImageDesignerManager::endSession(session.designerId);
			Object * const recipientObj = NetworkIdManager::getObjectById(session.recipientId);
			if(recipientObj && recipientObj == player)
			{
				ImageDesignChangeMessage * const msg = new ImageDesignChangeMessage;
				SharedImageDesignerManager::populateChangeMessage(session, *msg);
				msg->setOrigin(ImageDesignChangeMessage::O_RECIPIENT);
				msg->setAccepted(false);
				recipientObj->getController()->appendMessage (CM_imageDesignerCancel, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
	}

	return CuiMediator::close();
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerRecipient::update (float const deltaTimeSecs)
{
	CreatureObject * const player = Game::getPlayerCreature ();
	if(!player)
		return;

	CreatureObject * const designerObj = getDesignerCreature();
	if(!designerObj)
		return;

	CachedNetworkId target;
	if(CuiCombatManager::isInCombat(player, target) || CuiCombatManager::isInCombat(designerObj, target))
	{
		CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIdsImageDesigner::canceled_session_by_entering_combat.localize ());
		closeThroughWorkspace();
		return;
	}

	time_t timeLeft = SharedImageDesignerManager::getTimeRemaining(designerObj->getNetworkId());

	//update the time left UI
	if(timeLeft < 0)
		timeLeft = 0;
	updateTimeLeft(timeLeft);

	SharedImageDesignerManager::Session session;
	bool result = SharedImageDesignerManager::getSession(designerObj->getNetworkId(), session);
	if(result)
	{
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

	CuiMediator::update(deltaTimeSecs);
}

//----------------------------------------------------------------------

/** Set both viewers to the given parameters
*/
void SwgCuiImageDesignerRecipient::setViewersView(std::string const & rootBone, float const zoomFactor)
{
	m_viewerBefore->setCameraLookAtBone(rootBone);
	m_viewerBefore->setFitDistanceFactor(zoomFactor);
	if(rootBone == "root")
		m_viewerBefore->setCameraCompensateScale(true);
	else
		m_viewerBefore->setCameraCompensateScale(false);
	m_viewerBefore->recomputeZoom();

	m_viewerAfter->setCameraLookAtBone(rootBone);
	m_viewerAfter->setFitDistanceFactor(zoomFactor);
	if(rootBone == "root")
		m_viewerAfter->setCameraCompensateScale(true);
	else
		m_viewerAfter->setCameraCompensateScale(false);
	m_viewerAfter->recomputeZoom();
}

//----------------------------------------------------------------------

/** Set the designer for the UI, setup data
*/
void SwgCuiImageDesignerRecipient::setDesigner(NetworkId const & designerId)
{
	CreatureObject * const player = Game::getPlayerCreature();
	if(!player)
		return;

	m_designerId = designerId;

	//add the designer's name to the window caption
	CreatureObject * const designer = getDesignerCreature();
	if(designer)
	{
		Unicode::String newCaption = CuiStringIdsImageDesigner::recipient_caption.localize();
		newCaption += designer->getLocalizedName();
		m_caption->SetPreLocalized(true);
		m_caption->SetLocalText(newCaption);
	}

	//start a session.  This data will get overridden by designer-sent data once they start making changes
	SharedImageDesignerManager::Session newSession;
	newSession.designerId = getDesignerId();
	newSession.recipientId = player->getNetworkId();
	newSession.terminalId = m_terminalId;
	newSession.startingTime = Os::getRealSystemTime();
	SharedImageDesignerManager::startSession(newSession);

	CustomizationData const * const customizationData = player->fetchCustomizationData();
	if(customizationData)
	{
		m_originalCustomizations = customizationData->writeLocalDataToString();
		customizationData->release();
	}

	setAssociatedObjectId(designerId);
	setMaxRangeFromObject(16.0f);
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerRecipient::setTerminal(NetworkId const & terminalId)
{
	m_terminalId = terminalId;
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerRecipient::setCurrentHoloemote(std::string const & currentHoloEmote)
{
	m_currentHoloEmote = currentHoloEmote;
}

//----------------------------------------------------------------------

NetworkId const & SwgCuiImageDesignerRecipient::getDesignerId() const
{
	return m_designerId;
}

//----------------------------------------------------------------------

CreatureObject * SwgCuiImageDesignerRecipient::getDesignerCreature() const
{
	Object * const o = NetworkIdManager::getObjectById(getDesignerId());
	ClientObject * const co = o ? o->asClientObject() : NULL;
	return co ? co->asCreatureObject() : NULL;
}

//----------------------------------------------------------------------

/** When receiving a change packet, update the UI
*/
void SwgCuiImageDesignerRecipient::onImageDesignerChangeReceived(PlayerCreatureController::Messages::ImageDesignerChangeReceived::Payload const & payload)
{
	CreatureObject * const player = Game::getPlayerCreature();
	if(!player)
		return;

	if(player->getNetworkId() != payload.recipientId)
		return;

	if(!m_afterDoll)
		return;

	//override the given recipientPaidCredits with our value (since it's the correct one)
	int value = m_offeredMoneyTextBox->GetNumericIntegerValue();
	if(value < 0)
		value = 0;
	SharedImageDesignerManager::Session session = payload;
	session.recipientPaidCredits = value;

	//get our current session, to ensure that we don't use the server-sent starting time
	SharedImageDesignerManager::Session clientSession;
	bool const result = SharedImageDesignerManager::getSession(session.designerId, clientSession);
	if(result)
		session.startingTime = clientSession.startingTime;

	SharedImageDesignerManager::updateSession(session);

	DEBUG_REPORT_LOG(cms_verboseOutput, ("Received session...\n"));
	for(std::map<std::string, float>::iterator i = session.morphChanges.begin(); i != session.morphChanges.end(); ++i)
		DEBUG_REPORT_LOG(cms_verboseOutput, ("[%s]=%.3f\n", i->first.c_str(), i->second));
	for(std::map<std::string, int>::iterator j = session.indexChanges.begin(); j != session.indexChanges.end(); ++j)
		DEBUG_REPORT_LOG(cms_verboseOutput, ("[%s]=%d\n", j->first.c_str(), j->second));

	CustomizationData * const customizationData = m_afterDoll->fetchCustomizationData();
	if(customizationData)
	{
		customizationData->loadLocalDataFromString(m_originalCustomizations);
		customizationData->release();
		Object * const hair = NetworkIdManager::getObjectById(m_afterDollOriginalHairId);
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
			CuiCharacterHairManager::replaceHair(*m_afterDoll, tangibleHair, m_afterDoll->getHairObject() != tangibleHair);
		}
		else
			CuiCharacterHairManager::replaceHair(*m_afterDoll, NULL, true);
	}

	if(session.designerId == getDesignerId() && session.recipientId == player->getNetworkId())
	{
		for(std::map<std::string, float>::const_iterator k = session.morphChanges.begin(); k != session.morphChanges.end(); ++k)
		{
			ClientImageDesignerManager::setMorphCustomization(*m_afterDoll, k->first, k->second);
		}

		for(std::map<std::string, int>::const_iterator l = session.indexChanges.begin(); l != session.indexChanges.end(); ++l)
		{
			ClientImageDesignerManager::setIndexCustomization(*m_afterDoll, l->first, l->second);
		}
	}

	if(session.newHairSet)
	{
		ClientObject const * const currentHair = m_afterDoll->getHairObject();
		Object * const originalHair = NetworkIdManager::getObjectById(m_afterDollOriginalHairId);
		ClientObject * const clientOriginalHair = originalHair ? originalHair->asClientObject() : NULL;
		TangibleObject * const tangibleOriginalHair = clientOriginalHair ? clientOriginalHair->asTangibleObject() : NULL;
		std::string const sharedHairTemplateName = CustomizationManager::convertServerHairTemplateNameToSharedHairTemplateName(session.newHairAsset);
		SharedObjectTemplate const * const newHairOt = safe_cast<SharedObjectTemplate const *>(ObjectTemplateList::fetch(sharedHairTemplateName));
		if(newHairOt)
		{
			TangibleObject * const newHair = safe_cast<TangibleObject *>(newHairOt->createObject());
			newHairOt->releaseReference();
			if (newHair)
			{
				newHair->setNetworkId(ClientImageDesignerManager::getNextClientNetworkId());
				if(tangibleOriginalHair)
				{
					CustomizationData const * const sourceData = tangibleOriginalHair->fetchCustomizationData();
					if (sourceData)
					{
						newHair->setAppearanceData (sourceData->writeLocalDataToString());
						sourceData->release();
					}
				}
				newHair->endBaselines();
				CuiCharacterHairManager::replaceHair(*m_afterDoll, newHair, currentHair != tangibleOriginalHair);
			}
		}
		else
			CuiCharacterHairManager::replaceHair(*m_afterDoll, NULL, currentHair != tangibleOriginalHair);
	}

	if(session.accepted)
	{
		m_acceptedByDesigner->SetText(SharedStringIds::yes.localize());
		m_acceptedByDesignerBool = true;
	}
	else
	{
		m_acceptedByDesigner->SetText(SharedStringIds::no.localize());
		m_acceptedByDesignerBool = false;
	}

	int const creditsRequiredByDesigner = session.designerRequiredCredits;
	char buf[256];
	_itoa(creditsRequiredByDesigner, buf, 10);
	m_cost->SetText(Unicode::narrowToWide(buf));

	m_purchasedHoloEmote->Clear();
	if(!session.holoEmote.empty())
	{
		StringId const s("ui_imagedesigner", session.holoEmote);
		UIString const & oldString = m_purchasedHoloEmote->GetText();
		UIString newString = oldString;
		if(!newString.empty())
			newString.push_back('\n');
		newString += s.localize();
		m_purchasedHoloEmote->SetText(newString);
	}
	m_dataComposite->Pack();
}

//----------------------------------------------------------------------

/** When receiving a cancel packet, close the UI
*/
void SwgCuiImageDesignerRecipient::onImageDesignerCancelReceived(PlayerCreatureController::Messages::ImageDesignerChangeReceived::Payload const & payload)
{
	CreatureObject * const player = Game::getPlayerCreature();
	if(!player)
		return;

	if(player->getNetworkId() != payload.recipientId)
		return;

	SharedImageDesignerManager::endSession(payload.designerId);
	//no need to tell server
	m_committed = true;
	CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIdsImageDesigner::designer_cancelled_session.localize());
	closeThroughWorkspace();
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerRecipient::updateTimeLeft(time_t const timeLeft)
{
	if(timeLeft <= 0)
	{
		m_timeLeft->SetText(CuiStringIdsImageDesigner::ready.localize());
	}
	else
	{
		Unicode::String str;
		CuiUtils::FormatTimeDuration(str, static_cast<unsigned int>(timeLeft), false, false, true, true, true);
		m_timeLeft->SetText(str);
	}

	if(timeLeft > 0 || !isEnoughMoneyOffered() ||  !m_acceptedByDesignerBool)
		m_acceptButton->SetEnabled(false);
	else
		m_acceptButton->SetEnabled(true);
}

//----------------------------------------------------------------------

/** Build a packet and send it to the server
*/
void SwgCuiImageDesignerRecipient::buildAndSendUpdateToServer(bool const accepted) const
{
	CreatureObject * const designer = getDesignerCreature();
	if(designer)
	{
		SharedImageDesignerManager::Session session;
		bool const result = SharedImageDesignerManager::getSession(designer->getNetworkId(), session);
		if(result)
		{
			ImageDesignChangeMessage * const msg = new ImageDesignChangeMessage;
			SharedImageDesignerManager::populateChangeMessage(session, *msg);
			msg->setOrigin(ImageDesignChangeMessage::O_RECIPIENT);
			msg->setAccepted(accepted);
			Game::getPlayer()->getController()->appendMessage (CM_imageDesignerChange, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);

			//if accepted, also send message to designer, so they forward on the designer's session to the recipient
			if(accepted)
			{
				ImageDesignChangeMessage * const msg2 = new ImageDesignChangeMessage;
				SharedImageDesignerManager::populateChangeMessage(session, *msg2);
				msg->setOrigin(ImageDesignChangeMessage::O_RECIPIENT);
				msg->setAccepted(true);
				Controller * const designerController = designer->getController();
				if(designerController)
					designerController->appendMessage (CM_imageDesignerChange, 0.0f, msg2, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
	}
}

//----------------------------------------------------------------------

/** Determine if enough money has been offered by the recipient
*/
bool SwgCuiImageDesignerRecipient::isEnoughMoneyOffered() const
{
	int const offered = m_offeredMoneyTextBox->GetNumericIntegerValue();
	std::string const requiredStr = Unicode::wideToNarrow(m_cost->GetLocalText());
	int const required = atoi(requiredStr.c_str());
	return (offered >= required);
}

//----------------------------------------------------------------------

void SwgCuiImageDesignerRecipient::setAccepted(bool const accepted)
{
	m_committed = accepted;
}

//----------------------------------------------------------------------

int SwgCuiImageDesignerRecipient::checkMoney() const
{
	int val = m_offeredMoneyTextBox->GetNumericIntegerValue ();
	if(val < 0)
		val = 0;

	int cash = 0;
	int bank = 0;

	IGNORE_RETURN(PlayerMoneyManagerClient::getPlayerMoney (cash, bank));

	if ((bank + cash) < val)
	{
		CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIdsTrade::err_not_enough_money.localize ());
		m_offeredMoneyTextBox->Ding ();
		return cash + bank;
	}

	return val;
}

//======================================================================
