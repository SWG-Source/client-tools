// ======================================================================
//
// SwgCuiInventoryInfo.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiInventoryInfo.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UIMessage.h"
#include "UIScrollbar.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWaypointObject.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/DraftSchematicInfo.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/ManufactureSchematicObject.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/ResourceContainerObject.h"
#include "clientGame/TangibleObject.h"
#include "clientGame/WeaponObject.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiActions.h"
#include "clientUserInterface/CuiIconManagerObjectProperties.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedGame/TextManager.h"
#include "sharedObject/Container.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/GuildRequestMessage.h"
#include "sharedNetworkMessages/GuildResponseMessage.h"
#include "sharedObject/NetworkIdManager.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"
#include "swgClientUserInterface/SwgCuiInventoryContainer.h"

// ======================================================================

namespace SwgCuiInventoryInfoNamespace
{
	namespace Properties
	{
		const UILowerString AutoEnableContent ("AutoEnableContent");
	}

	const Unicode::String keyColor = Unicode::narrowToWide ("\\#pcontrast1 ");
	const Unicode::String indent   = Unicode::narrowToWide ("  \\>024\\#.");
	const Unicode::String unindent = Unicode::narrowToWide ("\\>000\n");

	bool ms_showBadgeText;

	CreatureObject * ms_dupedCreatureNoAppearance = NULL;

	void CleanupDupedCreature()
	{
		if(ms_dupedCreatureNoAppearance)
		{
			SlottedContainer * const container = ContainerInterface::getSlottedContainer(*ms_dupedCreatureNoAppearance);
			if(container)
			{
				for (ContainerIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
				{
					CachedNetworkId const & id = *containerIterator;

					if(id.getObject())
					{
						ClientObject * const obj = id.getObject ()->asClientObject();
						if(obj)
							delete obj;
					}
				}
			}
			delete ms_dupedCreatureNoAppearance;
		}
		ms_dupedCreatureNoAppearance = NULL;
	}
}

using namespace SwgCuiInventoryInfoNamespace;

//----------------------------------------------------------------------


SwgCuiInventoryInfo::SwgCuiInventoryInfo (UIPage & page, bool alterObjects, bool isExamine) :
CuiMediator              ("SwgCuiInventoryInfo", page),
UIEventCallback          (),
MessageDispatch::Receiver(),
m_viewer                (0),
m_label                 (0),
m_content               (0),
m_textAttribs           (0),
m_textDesc              (0),
m_noTrade(0),
m_callback              (new MessageDispatch::Callback),
m_autoEnableContent     (false),
m_defaultViewerPitch    (0.0f),
m_watcher               (new ObjectWatcher),
m_objectProperties      (new CuiIconManagerObjectProperties),
m_isPlayer(false),
m_tier(NULL),
m_unique(0),
m_buttonCollections(0),
m_hideAppearanceItems(0),
m_container(0)
{
	{
		UIWidget * widget = 0;
		getCodeDataObject (TUIWidget, widget, "viewer", true);
		if (widget)
		{
			m_viewer = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(widget)); 
		}
	}
	
	getCodeDataObject (TUIComposite,    m_content,       "content");

	m_content->SetScrollSizes (UISize (100, 100), UISize (18, 18));

	getCodeDataObject (TUIText,         m_label,         "label");
	getCodeDataObject (TUIText,         m_textAttribs,   "textAttribs");
	getCodeDataObject (TUIText,         m_textDesc,      "textDesc");
	getCodeDataObject (TUIText,         m_noTrade,       "noTrade", true);
	getCodeDataObject (TUIText,         m_tier,          "Tier", true);
	getCodeDataObject (TUIText,         m_unique,        "unique", true);
	getCodeDataObject (TUIButton,       m_buttonCollections, "buttonCollections", true);

	if(isExamine)
	{
		getCodeDataObject (TUICheckbox,     m_hideAppearanceItems, "checkHideAppearance");

		registerMediatorObject(*m_hideAppearanceItems, true);
	}

	
	getCodeData()->GetPropertyBoolean(UILowerString("showBadgeText"), ms_showBadgeText);
	
	const UIData * const codeData = getCodeData ();
	
	codeData->GetPropertyBoolean (Properties::AutoEnableContent, m_autoEnableContent);
	
	if (m_viewer)
	{
		m_viewer->setCameraLodBias (2.0f);
		m_defaultViewerPitch = m_viewer->getCameraPitch ();
		m_viewer->SetLocalTooltip    (CuiStringIds::tooltip_viewer_3d_controls.localize ());
		m_viewer->setAutoZoomOutOnly       (false);
		m_viewer->setCameraZoomInWhileTurn (false);
		m_viewer->setAlterObjects          (alterObjects);
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
	
	m_label->SetPreLocalized          (true);
	m_textAttribs->SetPreLocalized    (true);
	m_textDesc->SetPreLocalized       (true);

	m_label->Clear ();
	m_textAttribs->Clear ();
	m_textDesc->Clear ();

	if (m_autoEnableContent)
		m_content->SetEnabled (false);

	updateAttributeFlags();

	m_content->AddCallback(this);

}

//-----------------------------------------------------------------

SwgCuiInventoryInfo::~SwgCuiInventoryInfo ()
{
	m_content->RemoveCallback (this);

	delete m_callback;
	m_callback      = 0;

	m_viewer        = 0;

	m_content       = 0;
	m_label         = 0;
	m_textAttribs   = 0;
	m_textDesc      = 0;
	m_noTrade = 0;
	m_unique = 0;
	m_buttonCollections = 0;
	
	delete m_watcher;
	m_watcher = 0;

	delete m_objectProperties;
	m_objectProperties = 0;

	ObjectAttributeManager::stopWatching(this);

	if(ms_dupedCreatureNoAppearance)
	{
		CleanupDupedCreature();
	}
}

//-----------------------------------------------------------------

void SwgCuiInventoryInfo::performActivate ()
{
	m_content->ScrollToPoint (UIPoint (0,0));
	m_callback->connect (*this, &SwgCuiInventoryInfo::onAttributesChanged,          static_cast<ObjectAttributeManager::Messages::AttributesChanged *> (0));
	m_callback->connect (*this, &SwgCuiInventoryInfo::onDraftSchematicInfoChanged,  static_cast<DraftSchematicInfo::Messages::Changed *> (0));
	m_callback->connect (*this,  &SwgCuiInventoryInfo::onBiographyRetrieved,        static_cast<PlayerCreatureController::Messages::BiographyRetrieved *>(0));
	connectToMessage(GuildResponseMessage::MessageType);
	setIsUpdating (true);

	if (m_buttonCollections)
		m_buttonCollections->AddCallback(this);
}

//-----------------------------------------------------------------

void SwgCuiInventoryInfo::performDeactivate ()
{
	disconnectFromMessage(GuildResponseMessage::MessageType);
	m_callback->disconnect (*this, &SwgCuiInventoryInfo::onBiographyRetrieved,        static_cast<PlayerCreatureController::Messages::BiographyRetrieved *>(0));
	m_callback->disconnect (*this, &SwgCuiInventoryInfo::onAttributesChanged,         static_cast<ObjectAttributeManager::Messages::AttributesChanged *> (0));
	m_callback->disconnect (*this, &SwgCuiInventoryInfo::onDraftSchematicInfoChanged, static_cast<DraftSchematicInfo::Messages::Changed *> (0));

	ObjectAttributeManager::stopWatching(this);

	setInfoObject (0, false);
	setIsUpdating (false);
	if (m_buttonCollections)
		m_buttonCollections->RemoveCallback(this);
}

//-----------------------------------------------------------------

void SwgCuiInventoryInfo::setInfoObject (Object * object, bool requestAttributeUpdate)
{
	m_isPlayer = false;
	ObjectAttributeManager::stopWatching(this);
	updateAttributeFlags();

	if (m_buttonCollections)
		m_buttonCollections->SetVisible(false);
	if (m_hideAppearanceItems)
	{
		m_hideAppearanceItems->SetVisible(false);
	}
	
	// If this is a player object, route the request appropriately
	CreatureObject * const creatureObj = dynamic_cast<CreatureObject *>(object);
	if(creatureObj)
	{
		const PlayerObject* const playerObj = creatureObj->getPlayerObject();
		if(playerObj)
		{
			if (m_buttonCollections)
				m_buttonCollections->SetVisible(true);
			if (m_hideAppearanceItems)
				m_hideAppearanceItems->SetVisible(true);
			setPlayerInfo(creatureObj);
			return;
		}
	}

	if (object && requestAttributeUpdate && object->getNetworkId () != NetworkId::cms_invalid)
		ObjectAttributeManager::requestUpdate (object->getNetworkId ());

	ClientObject * const clientObject = object ? object->asClientObject (): 0;
	
	const bool changingObject = object != m_watcher->getPointer ();

	if (changingObject && clientObject)
		m_objectProperties->updateFromObject (*clientObject);

	*m_watcher = object;

	if (m_viewer)
	{
		if (changingObject)
			m_viewer->setCameraPitch          (m_defaultViewerPitch);

		m_viewer->setCameraForceTarget   (true);
		m_viewer->setObject              (object);

		if (changingObject)
		{
			m_viewer->setRotateSpeed          (1.0f);
			m_viewer->recomputeZoom ();
		}

		m_viewer->setCameraForceTarget   (false);
	}

	m_textAttribs->Clear();
	m_textDesc->Clear();
	
	if (clientObject)
	{
		Unicode::String header;
		Unicode::String desc;
		Unicode::String attribs;

		bool descOk = false;

		NetworkId const & objId = object->getNetworkId();
		if ((objId != NetworkId::cms_invalid) && (Game::getSinglePlayer() || !ClientObject::isFakeNetworkId(objId)))
		{
			descOk = ObjectAttributeManager::formatDescription (objId, header, desc, attribs, false);
		}
		//-- it may be a draft schematic
		else if (!object->isInWorld ())
		{
			int dummy = 0;
			descOk = DraftSchematicManager::formatDescriptionIfNewer (*clientObject, header, desc, attribs, false, dummy);
		}


		ClientWaypointObject const * const wp = dynamic_cast<const ClientWaypointObject* const>(clientObject);		
		if(wp)
		{
			const StringId & siddesc = wp->getDescription();
			if(siddesc.isValid())
			{
				desc = siddesc.localize();
				descOk = true;
			}
		}

		if (descOk)
		{
			m_label->SetLocalText       (header);
			m_textAttribs->SetLocalText (attribs);

			m_textDesc->SetLocalText    (desc);
		}
		else
		{
			m_label->SetLocalText(clientObject->getLocalizedName());
		}
	}

	//-- it's not a client object, just print the debug info
	else if (object)
	{
		Unicode::String debugInfo;
		ObjectAttributeManager::formatDebugInfo (*object, debugInfo);
		m_label->SetLocalText (Unicode::narrowToWide (object->getDebugName ()));
		m_textAttribs->SetLocalText   (debugInfo);
		m_textDesc->Clear ();
	}


	updateAttributeFlags();

	m_content->Pack ();

	if (m_autoEnableContent)
		m_content->SetEnabled (object != 0);
}

//----------------------------------------------------------------------

void SwgCuiInventoryInfo::setPlayerInfo(CreatureObject * creatureObj)
{
	if(!creatureObj)
	{
		DEBUG_REPORT_LOG(true, ("SwgCuiInventoryInfo::setPlayerInfo() Invalid player object: %s\n", (creatureObj == NULL) ? "NULL" : creatureObj->getObjectTemplateName()));

		return;
	}

	const PlayerObject* playerObj = creatureObj->getPlayerObject();
	const bool changingObject = creatureObj != m_watcher->getPointer ();

	m_currentGuild  = CuiStringIds::examine_fetchingguild.localize();
	m_currentBadges = CuiStringIds::examine_fetchingbadges.localize();
	m_currentTitle  = CuiStringIds::examine_fetchingtitle.localize();
	if (playerObj == NULL || playerObj != Game::getPlayerObject() || !playerObj->haveBiography())
		m_currentBio = CuiStringIds::examine_fetchingbio.localize();
	else
		m_currentBio = playerObj->getBiography();
	
	if(playerObj)
	{
		m_currentBadges.clear();

		std::vector<CollectionsDataTable::CollectionInfoSlot const *> earnedBadges;
		IGNORE_RETURN(playerObj->getCompletedCollectionSlotCountInBook("badge_book", &earnedBadges));

		for(std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator i = earnedBadges.begin(); i != earnedBadges.end(); ++i)
		{
			m_currentBadges += CollectionsDataTable::localizeCollectionDescription((*i)->name);
			m_currentBadges.append(1, '\n');
		}

		if(m_currentBadges == Unicode::emptyString)
		{
			m_currentBadges = CuiStringIds::examine_nobadges.localize();
		}

		playerObj->requestBiography();

		m_currentTitle = playerObj->getLocalizedTitle();
		if (m_currentTitle.empty())
			m_currentTitle = CuiStringIds::examine_notitle.localize();

		GuildRequestMessage grm(creatureObj->getNetworkId());
		GameNetwork::send (grm, true); 
	}

	*m_watcher = creatureObj;
	m_isPlayer = true;

	if (m_viewer)
	{
		if(ms_dupedCreatureNoAppearance)
		{
			CleanupDupedCreature();
		}

		if(m_hideAppearanceItems)
		{
			ms_dupedCreatureNoAppearance = SwgCuiAvatarCreationHelper::duplicateCreatureWithClothesAndCustomization(*creatureObj, false);
			m_hideAppearanceItems->SetChecked(false, false);
		}

		if (changingObject)
			m_viewer->setCameraPitch          (m_defaultViewerPitch);

		m_viewer->setCameraForceTarget   (true);
		m_viewer->setObject              (creatureObj);

		if (changingObject)
		{
			m_viewer->setRotateSpeed          (1.0f);
			m_viewer->recomputeZoom ();
		}

		m_viewer->setCameraForceTarget   (false);

		if(m_hideAppearanceItems && ms_dupedCreatureNoAppearance)
		{
			if(CuiPreferences::getDefaultExamineHideAppearance())
			{
				m_hideAppearanceItems->SetChecked(true, false);
				m_viewer->setCameraPitch(m_defaultViewerPitch);

				m_viewer->setCameraForceTarget(true);
				m_viewer->setObject(ms_dupedCreatureNoAppearance);

				m_viewer->setRotateSpeed(1.0f);
				m_viewer->recomputeZoom();

				m_viewer->setCameraForceTarget(false);
				m_viewer->setAlterObjects(true);
			}

			if(ms_dupedCreatureNoAppearance == m_viewer->getLastObject())
				m_viewer->setAlterObjects(true);
			else
				m_viewer->setAlterObjects(false);
		}
	}

	updatePlayerData();

	m_content->Pack ();

	if (m_autoEnableContent)
		m_content->SetEnabled (true);

	updateAttributeFlags();
}

//----------------------------------------------------------------------

void SwgCuiInventoryInfo::update (float)
{
	if (!m_watcher->getPointer())
	{
		ObjectAttributeManager::stopWatching(this);
		return;
	}

	if (!m_isPlayer) 
	{
		ObjectAttributeManager::startWatching(this, m_watcher->getPointer()->getNetworkId());
	}

	ClientObject * const clientObject = m_watcher->getPointer ()->asClientObject ();	
	if (!clientObject)
		return;

	if (m_objectProperties->updateAndCompareFromObject (*clientObject))
		setInfoObject (clientObject, true);

	if(clientObject->asCreatureObject() && clientObject->asCreatureObject()->getDupedCreaturesDirty() && m_hideAppearanceItems)
	{
		if(ms_dupedCreatureNoAppearance)
		{
			CleanupDupedCreature();
		}

		ms_dupedCreatureNoAppearance = SwgCuiAvatarCreationHelper::duplicateCreatureWithClothesAndCustomization(*clientObject->asCreatureObject(), false);
		if(m_hideAppearanceItems->IsChecked() && ms_dupedCreatureNoAppearance)
		{
			m_viewer->setCameraPitch(m_defaultViewerPitch);

			m_viewer->setCameraForceTarget(true);
			m_viewer->setObject(ms_dupedCreatureNoAppearance);

			m_viewer->setRotateSpeed(1.0f);
			m_viewer->recomputeZoom();

			m_viewer->setCameraForceTarget(false);
			m_viewer->setAlterObjects(true);
		}

		clientObject->asCreatureObject()->setDupedCreaturesDirty(false);
	}
}

//-----------------------------------------------------------------------

void SwgCuiInventoryInfo::updatePlayerData()
{
	m_textAttribs->Clear ();
	m_textDesc->Clear ();

	Unicode::String descData;
	descData.append(keyColor);
	descData.append(CuiStringIds::examine_title.localize());
	descData.append(indent);
	descData.append(m_currentTitle);
	descData.append(unindent);
	descData.append(keyColor);
	descData.append(CuiStringIds::examine_guild.localize());
	descData.append(indent);
	descData.append(m_currentGuild);
	descData.append(unindent);

	if (ms_showBadgeText)
	{
		descData.append(keyColor);
		descData.append(CuiStringIds::examine_badges.localize());
		descData.append(indent);
		descData.append(m_currentBadges);
		descData.append(unindent);
	}

#if PRODUCTION == 0
	if (m_watcher != NULL)
	{
		Unicode::String header;
		Unicode::String desc;
		Unicode::String attribs;

		const NetworkId & objId = m_watcher->getPointer()->getNetworkId();
		if (objId != NetworkId::cms_invalid)
		{
			if (ObjectAttributeManager::formatDescription(objId, header, desc, attribs, false))
			{
				descData.insert(descData.begin(), attribs.begin(), attribs.end());
			}
		}
	}
#endif // PRODUCTION == 0

	m_textAttribs->SetLocalText(descData);

	m_textDesc->SetLocalText(m_currentBio);
	
	Object* obj = m_watcher->getPointer();
	CreatureObject* creatureObj = dynamic_cast<CreatureObject*>(obj);

	if(creatureObj)
		m_label->SetLocalText(creatureObj->getLocalizedName());

	m_content->Pack ();
}

//-----------------------------------------------------------------------

void SwgCuiInventoryInfo::receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
{
	if(message.isType("GuildResponseMessage"))
	{
		Archive::ReadIterator ri = NON_NULL(safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const GuildResponseMessage msg(ri);
		Object* player = m_watcher->getPointer();
		if (player && player->getNetworkId() == msg.getTargetId())
		{
			m_currentGuild = Unicode::narrowToWide(msg.getGuildName());
			if(m_currentGuild == Unicode::emptyString)
			{
				m_currentGuild = CuiStringIds::examine_unguilded.localize();
			}
			m_currentGuildTitle = Unicode::narrowToWide(msg.getMemberTitle());
			updatePlayerData();
		}
	}
}

//-----------------------------------------------------------------------

void SwgCuiInventoryInfo::onBiographyRetrieved  (PlayerCreatureController::Messages::BiographyRetrieved::BiographyOwner const &msg)
{
	Object const * const o = m_watcher->getPointer();
	ClientObject const * const clo = o ? o->asClientObject() : NULL;
	CreatureObject const * const co = clo ? clo->asCreatureObject() : NULL;
	if (co)
	{
		const PlayerObject * const playerObject = co->getPlayerObject();
		if (playerObject)
		{
			//make sure the bio response is for the person we're viewing
			const PlayerObject * const bioPlayer = msg.second;
			if (bioPlayer == playerObject)
			{
				m_currentBio = Game::isProfanityFiltered() ? TextManager::filterText(playerObject->getBiography()) : playerObject->getBiography();
				m_textDesc->SetLocalText (m_currentBio);
				m_content->Pack ();
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryInfo::connectToSelectionTransceiver       (CuiContainerSelectionChanged::TransceiverType & transceiver)
{
	m_callback->connect (transceiver, *this, &SwgCuiInventoryInfo::onSelectionChanged);
}

//----------------------------------------------------------------------

void SwgCuiInventoryInfo::disconnectFromSelectionTransceiver  (CuiContainerSelectionChanged::TransceiverType & transceiver)
{
	m_callback->disconnect (transceiver, *this, &SwgCuiInventoryInfo::onSelectionChanged);
}

//----------------------------------------------------------------------

void SwgCuiInventoryInfo::onSelectionChanged(const CuiContainerSelectionChanged::Payload & payload)
{
	ClientObject * const obj = payload.second;
	setInfoObject (obj, true);
}

//----------------------------------------------------------------------

void SwgCuiInventoryInfo::onAttributesChanged (const ObjectAttributeManager::Messages::AttributesChanged::Payload & id)
{
	Object * const o = NetworkIdManager::getObjectById (id);
	ClientObject * const obj = o ? o->asClientObject() : NULL;
	if (obj)
	{
		if (obj == m_watcher->getPointer ())
		{
			setInfoObject (obj, false);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryInfo::onDraftSchematicInfoChanged         (const DraftSchematicInfo & info)
{
	if (info.getClientObject () == m_watcher->getPointer ())
		setInfoObject (info.getClientObject (), false);
}

//----------------------------------------------------------------------

Object * SwgCuiInventoryInfo::getInfoObject   ()
{
	return m_watcher->getPointer ();
}

//----------------------------------------------------------------------

void SwgCuiInventoryInfo::setInventoryType(SwgCuiInventory::InventoryType type)
{
	UNREF(type);
}



//----------------------------------------------------------------------

void SwgCuiInventoryInfo::updateAttributeFlags()
{
	if(m_noTrade)
		m_noTrade->SetOpacity(0.0f);
	if(m_unique)
		m_unique->SetOpacity(0.0f);
	if(m_tier)
		m_tier->SetOpacity(0.0f);
	if (m_noTrade)
	{
		m_noTrade->SetPreLocalized(true);

		Object const * const object = getInfoObject();
		bool const isNoTradeVisible = (object && ObjectAttributeManager::isNoTrade(object->getNetworkId()));
		m_noTrade->SetOpacity(isNoTradeVisible ? 1.0f : 0.0f);

		if (isNoTradeVisible)
		{
			if (ObjectAttributeManager::isNoTradeShared(object->getNetworkId()))
			{
				m_noTrade->SetLocalText(StringId("object_usability", "no_trade_shared").localize());
				m_noTrade->SetTooltip(CuiStringIds::no_trade_shared_tooltip.localize());
			}
			else
			{
				m_noTrade->SetLocalText(StringId("object_usability", "no_trade").localize());
				m_noTrade->SetTooltip(CuiStringIds::no_trade_tooltip.localize());
			}
		}
		else
		{
			m_noTrade->SetTooltip(Unicode::emptyString);
		}
	}
	if (m_unique)
	{
		Object const * const object = getInfoObject();
		bool const isUniqueVisible = (object && ObjectAttributeManager::isUnique(object->getNetworkId()));
		m_unique->SetOpacity(isUniqueVisible ? 1.0f : 0.0f);

		if (isUniqueVisible)
			m_unique->SetTooltip(CuiStringIds::unique_tooltip.localize());
		else
			m_unique->SetTooltip(Unicode::emptyString);
	}
	if (m_tier)
	{
		m_tier->SetPreLocalized(true);

		Object const * const object = getInfoObject();
		NetworkId const & id = object ? object->getNetworkId() : NetworkId::cms_invalid;
		int const tier = ObjectAttributeManager::getTier(id);

		bool const isTierVisible = tier >= 0;
		m_tier->SetOpacity(isTierVisible ? 1.0f : 0.0f);

		if (isTierVisible) 
		{
			StringId const & tierId = ObjectAttributeManager::getTierStringId(tier);
			m_tier->SetLocalText(tierId.localize());

			StringId const & tierDesc = ObjectAttributeManager::getTierDescStringId(tier);
			m_tier->SetTooltip(tierDesc.localize());

			UIColor const & color = ObjectAttributeManager::getTierColor(tier);
			m_tier->SetColor(UIColor::white);
			m_tier->SetTextColor(color);
		}
		else
		{
			m_tier->SetTooltip(Unicode::emptyString);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryInfo::setDropThroughTarget(SwgCuiInventoryContainer *container)
{
	m_container = container;
}

//----------------------------------------------------------------------

bool SwgCuiInventoryInfo::OnMessage       (UIWidget *context, const UIMessage & msg )
{
	if (msg.Type == UIMessage::DragEnd)
	{
		if(m_container && msg.DragObject)
		{
			m_container->handleDropThrough(msg.DragObject);
			return false;
		}
	}
	else if (msg.Type == UIMessage::DragOver)
	{
		if(m_container)
			context->SetDropFlagOk (true);
	}
	return true;
}

//----------------------------------------------------------------------

void SwgCuiInventoryInfo::OnButtonPressed(UIWidget *context)
{
	if (m_buttonCollections && context == m_buttonCollections && getInfoObject())
	{
		CuiActionManager::performAction(CuiActions::collections, Unicode::narrowToWide(getInfoObject()->getNetworkId().getValueString()));
	}
}

//----------------------------------------------------------------------

CuiWidget3dObjectListViewer *      SwgCuiInventoryInfo::getViewer(void)
{
	return m_viewer;
}

//----------------------------------------------------------------------

UICheckbox * SwgCuiInventoryInfo::getHideAppearanceItemsCheckbox()
{
	return m_hideAppearanceItems;
}

//----------------------------------------------------------------------

void SwgCuiInventoryInfo::OnCheckboxSet(UIWidget *context)
{
	if(context == m_hideAppearanceItems && m_viewer)
	{
		if(ms_dupedCreatureNoAppearance)
		{
			m_viewer->setCameraPitch(m_defaultViewerPitch);

			m_viewer->setCameraForceTarget(true);
			m_viewer->setObject(ms_dupedCreatureNoAppearance);

			m_viewer->setRotateSpeed(1.0f);
			m_viewer->recomputeZoom();

			m_viewer->setCameraForceTarget(false);
			m_viewer->setAlterObjects(true);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiInventoryInfo::OnCheckboxUnset(UIWidget *context)
{
	if(context == m_hideAppearanceItems && m_viewer)
	{
		if(ms_dupedCreatureNoAppearance)
		{
			m_viewer->setCameraPitch(m_defaultViewerPitch);

			m_viewer->setCameraForceTarget(true);
			m_viewer->setObject(getInfoObject());

			m_viewer->setRotateSpeed(1.0f);
			m_viewer->recomputeZoom();

			m_viewer->setCameraForceTarget(false);
			m_viewer->setAlterObjects(false);
		}
	}
}

//======================================================================
