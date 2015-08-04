//======================================================================
//
// SwgCuiAvatarCreationHelper.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAvatarCreationHelper.h"

#include "UIData.h"
#include "UIDataSource.h"
#include "UIManager.h"
#include "UIPage.h"
#include "UnicodeUtils.h"
#include "clientAudio/Audio.h"
#include "clientGame/ClientImageDesignerManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/PlayerObject.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiBackdrop.h"
#include "clientUserInterface/CuiCachedAvatarManager.h"
#include "clientUserInterface/CuiCharacterHairManager.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSkillManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Watcher.h"
#include "sharedGame/WearableEntry.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"
#include "sharedNetworkMessages/NameErrors.h"
#include "sharedObject/Container.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedRandom/Random.h"
#include "sharedUtility/DataTable.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiSceneSelection.h"


#include <algorithm>

//======================================================================


namespace SwgCuiAvatarCreationHelperNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<bool, SwgCuiAvatarCreationHelper::Messages::CreationFinished > 
			creationFinished;
		MessageDispatch::Transceiver<const SwgCuiAvatarCreationHelper::Messages::RandomNameChanged::Payload &, SwgCuiAvatarCreationHelper::Messages::RandomNameChanged > 
			randomNameChanged;
		MessageDispatch::Transceiver<bool, SwgCuiAvatarCreationHelper::Messages::Aborted > 
			aborted;
		MessageDispatch::Transceiver<const SwgCuiAvatarCreationHelper::Messages::VerifyAndLockNameResponse::Payload &, SwgCuiAvatarCreationHelper::Messages::VerifyAndLockNameResponse > 
			verifyAndLockNameResponse;
	}

	namespace UnnamedMessages
	{
		const char * const ClientCreateCharacterSuccess    = "ClientCreateCharacterSuccess";  //lint !e578 //hides
		const char * const ClientCreateCharacterFailed     = "ClientCreateCharacterFailed";  //lint !e578 //hides
		const char * const ClientRandomNameResponse        = "ClientRandomNameResponse";
		const char * const ClientVerifyAndLockNameResponse = "ClientVerifyAndLockNameResponse";
	}

	//----------------------------------------------------------------------

	CuiMessageBox * m_messageBox = 0;
	CuiMessageBox * s_messageBoxWaitingName = 0;

	Watcher<CreatureObject> s_creature (0);

	bool s_waiting_for_creation = false;

	//----------------------------------------------------------------------
	
	struct Listener : public MessageDispatch::Receiver
	{
		
		Listener () :
		MessageDispatch::Receiver ()
		{
			connectToMessage(UnnamedMessages::ClientCreateCharacterSuccess);
			connectToMessage(UnnamedMessages::ClientCreateCharacterFailed);
			connectToMessage(UnnamedMessages::ClientRandomNameResponse);
			connectToMessage(UnnamedMessages::ClientVerifyAndLockNameResponse);
		}

		void receiveMessage (const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
		{
			if (message.isType (ClientCreateCharacterSuccess::MessageType))
			{
				Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
				const ClientCreateCharacterSuccess msg(ri);

				const NetworkId & id = msg.getNetworkId ();
				SwgCuiAvatarCreationHelper::onCompleted (true, id);
			}
			else if (message.isType (UnnamedMessages::ClientCreateCharacterFailed))
			{
				Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
				const ClientCreateCharacterFailed msg(ri);
				
				Unicode::String str;
				msg.getErrorMessage ().localize (str);
				IGNORE_RETURN (CuiMessageBox::createInfoBox (str));
				
				SwgCuiAvatarCreationHelper::onCompleted (false, NetworkId::cms_invalid);
			}
			else if (message.isType (UnnamedMessages::ClientRandomNameResponse))
			{
				if (s_messageBoxWaitingName)
					s_messageBoxWaitingName->closeMessageBox ();
				
				Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
				const ClientRandomNameResponse msg(ri);
				
				SwgCuiAvatarCreationHelper::onRandomNameReceived (msg.getCreatureTemplate (),  msg.getName (), msg.getErrorMessage() );
			}
			else if (message.isType (UnnamedMessages::ClientVerifyAndLockNameResponse))
			{
				Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
				ClientVerifyAndLockNameResponse const msg(ri);

				SwgCuiAvatarCreationHelper::onClientVerifyAndLockNameResponseReceived(msg.getCharacterName(), msg.getErrorMessage());
			}
			else
			{
				const CuiMessageBox::BoxMessage * const bm = dynamic_cast<const CuiMessageBox::BoxMessage *>(&message);
				
				if (bm)
				{
					if (message.isType (CuiMessageBox::Messages::CLOSED))
					{ 
						s_waiting_for_creation = false;

						if (bm->getMessageBox () == m_messageBox)
						{
							m_messageBox = 0;
						}
						else if (bm->getMessageBox () == s_messageBoxWaitingName)
						{
							s_messageBoxWaitingName = 0;
						}
					}
					else if (message.isType (CuiMessageBox::Messages::COMPLETED))
					{
						const CuiMessageBox::CompletedMessage * const cm = dynamic_cast<const CuiMessageBox::CompletedMessage *>(bm);

						if (cm)
						{
							if (s_waiting_for_creation)
							{
								s_waiting_for_creation = false;
								Transceivers::aborted.emitMessage (true);
							}
						}
					}
				}
			}
		}
	};
	
	Listener * s_listener = 0;

	const char * const s_hairSlot = "hair"; 

	std::string     s_profession;
	Unicode::String s_biography;

	std::string s_startingLocation;

	//----------------------------------------------------------------------

	SwgCuiAvatarCreationHelper::CreatureVector  * s_creaturePool;

	//----------------------------------------------------------------------

	Unicode::String s_randomName;

	bool s_creatureCustomized    = false;
	bool s_automaticFinish       = false;
	bool s_lastCreationAutomatic = false;

	Unicode::String s_automaticName;

	TangibleObject * s_pedestal = 0;

	SoundId s_musicId;

	bool s_installed = false;
}

using namespace SwgCuiAvatarCreationHelperNamespace;

bool SwgCuiAvatarCreationHelper::ms_isCreatingJedi = false;

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::install ()
{
	DEBUG_FATAL (s_installed, ("installed"));
	s_listener = new Listener;
	s_creaturePool = new CreatureVector;
	s_creatureCustomized = false;
	s_installed = true;

	s_startingLocation = ConfigClientGame::getDefaultStartLocation ();
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	delete s_listener;
	s_listener = 0;

	delete s_creaturePool;
	s_creaturePool = 0;
	s_installed = false;

	if (s_pedestal)
	{
		delete s_pedestal;
		s_pedestal = 0;
	}
}

//----------------------------------------------------------------------

bool SwgCuiAvatarCreationHelper::finishCreation (bool automatic)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	if (m_messageBox)
		return false;

	if (!s_creature && !automatic)
	{
		WARNING (true, ("Cannot finish creation without a creature!"));
		return false;
	}

	//-- if we're multi-player, we need to send a create-player message
	if (!Game::getSinglePlayer())
	{
		Unicode::String  name;
		std::string      customizationString;
		std::string      serverTemplateName;
		std::string      serverHairObjectTemplateName;
		std::string      hairCustomizationString;
		float            scaleFactor = 1.0f;

		if (s_creature)
		{
			s_automaticFinish = false;
			CustomizationData * const cdata = s_creature->fetchCustomizationData ();
			WARNING(!cdata, ("could not save customizations"));		
			customizationString = cdata ? cdata->writeLocalDataToString () : std::string ();
		
			//-- release local reference
			if (cdata)
				cdata->release();

			serverTemplateName = convertSharedToServer (s_creature->getObjectTemplateName ());
		
			TangibleObject * const hair = dynamic_cast<TangibleObject *>(s_creature->getEquippedObject (s_hairSlot));

			if (hair)
			{		
				serverHairObjectTemplateName = convertSharedToServer (hair->getObjectTemplateName ());

				CustomizationData * const hair_cdata = hair->fetchCustomizationData ();
				if (hair_cdata)
				{
					hairCustomizationString = hair_cdata->writeLocalDataToString ();
					hair_cdata->release();
				}
				else
					WARNING(true, ("could not save hair customizations"));
			}

			name = s_creature->getObjectName ();

			scaleFactor = s_creature->getScaleFactor ();

			s_lastCreationAutomatic = false;
			s_automaticName.clear ();
		}
		else
		{
			if (s_randomName.empty ())
			{
				s_automaticFinish = true;
				requestRandomName (false);
				return false;
			}

			const std::string &avatarName = ConfigClientGame::getAvatarName ();

			if (!avatarName.empty ())
				name               = Unicode::narrowToWide (avatarName);
			else
				name               = s_randomName;

			serverTemplateName = "object/creature/player/human_male.iff";

			if (s_profession.empty ())
				SwgCuiAvatarCreationHelper::setProfession ("artisan");

			s_lastCreationAutomatic = true;
			s_automaticName         = name;

			s_automaticFinish = false;
		}

		const ClientCreateCharacter creationMessage(name,
			serverTemplateName,
			scaleFactor,
			s_startingLocation,
			customizationString,
			serverHairObjectTemplateName,
			hairCustomizationString,
			s_profession,
			ms_isCreatingJedi,
			s_biography,
			CuiPreferences::getUseNewbieTutorial (),
			CuiSkillManager::getSkillTemplate(),
			CuiSkillManager::getWorkingSkill());
		
		GameNetwork::send (creationMessage, true); 
		
		s_waiting_for_creation = true;
		//-- create the message box
		m_messageBox = CuiMessageBox::createMessageBox (CuiStringIds::avatar_wait_confirm_create.localize ());
		m_messageBox->setRunner (true);
		m_messageBox->connectToMessages (*s_listener);
		return false;
	}
	else
	{
		//-- single player selects the scene now
		const SwgCuiSceneSelection *const sceneSelector = 
			NON_NULL (dynamic_cast<SwgCuiSceneSelection *> (CuiMediatorFactory::get (CuiMediatorTypes::SceneSelection)));
		
		// START THE SCENE
		if (sceneSelector->startScene (s_creature->getObjectTemplateName (), s_creature))
		{
			//-- the scene now owns the player's creature
			IGNORE_RETURN (s_creaturePool->erase (std::remove (s_creaturePool->begin (), s_creaturePool->end (), s_creature), s_creaturePool->end ()));
			s_creature = 0;
			purgePool (true);
			return true;
		}

		return false;
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::VerifyAndLockName()
{
	std::string templateName;
	Unicode::String characterName;

	if (s_creature)
	{
		templateName = convertSharedToServer (s_creature->getObjectTemplateName ());
		characterName = s_creature->getObjectName ();
	}
	else
	{
		const std::string &avatarName = ConfigClientGame::getAvatarName ();

		if (!avatarName.empty ())
			characterName               = Unicode::narrowToWide (avatarName);
		else
			characterName               = s_randomName;

		templateName = "object/creature/player/human_male.iff";
	}
	
	const ClientVerifyAndLockNameRequest req (templateName, characterName);
	GameNetwork::send (req, true); 
}

//----------------------------------------------------------------------

CreatureObject * SwgCuiAvatarCreationHelper::setCreature (CreatureObject & obj)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	CreatureObject * const old = s_creature.getPointer ();

	if (std::find (s_creaturePool->begin (), s_creaturePool->end (), &obj) == s_creaturePool->end ())
		s_creaturePool->push_back (&obj);

	s_creature = &obj;

	return old;
}

//----------------------------------------------------------------------

CreatureObject * SwgCuiAvatarCreationHelper::getCreature ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	return s_creature.getPointer ();
}

//----------------------------------------------------------------------

bool SwgCuiAvatarCreationHelper::areAllDetailLevelsAvailable (const CreatureObject & creature)
{
	bool available = true;
	
	const Container * const container = creature.getContainerProperty ();
	
	if (container)
	{
		for (ContainerConstIterator cit = container->begin (); cit != container->end (); ++cit)
		{
			const Object * const contained = (*cit).getObject ();
			NOT_NULL (contained);
			const Appearance * const app = contained->getAppearance ();
			if (app)
			{
				const SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
				if (skelApp)
				{
					//-- preload all lod assets
					const int count = skelApp->getDetailLevelCount ();
					for (int i = 0; i < count; ++i)
					{
						if (!skelApp->isDetailLevelAvailable (i))
							available = false;
					}
				}
			}
		}
	}

	const Appearance * const app = creature.getAppearance ();
	if (app)
	{
		const SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
		if (skelApp)
		{
			//-- preload all lod assets
			const int count = skelApp->getDetailLevelCount ();
			for (int i = 0; i < count; ++i)
			{
				if (!skelApp->isDetailLevelAvailable (i))
					available = false;
			}
			
			if (available)
				const_cast<SkeletalAppearance2 *>(skelApp)->rebuildIfDirtyAndAvailable ();
		}
	}

	return available;
}

//----------------------------------------------------------------------

CreatureObject * SwgCuiAvatarCreationHelper::duplicateCreature (const CreatureObject & base)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	const bool asynchronousLoaderEnabled = AsynchronousLoader::isEnabled ();
	AsynchronousLoader::disable ();

	const ObjectTemplate * const ot = base.getObjectTemplate ();
	CreatureObject * const creature = dynamic_cast<CreatureObject *>(ot->createObject ());
	
	if (creature)
	{
		CreatureController* const controller = new CreatureController (creature);
		creature->setController (controller);
		creature->endBaselines ();
		
		creature->setScaleFactor(base.getScaleFactor());

		const float alterBySecs = Random::randomReal (0.1f, 3.0f);
		if (creature->alter    (alterBySecs))
			creature->conclude ();
		
		areAllDetailLevelsAvailable (*creature);
		
	} //lint !e429 //custodial controller ptr

	if (asynchronousLoaderEnabled)
		AsynchronousLoader::enable ();

	return creature;
} //lint !e1763 //no, we might need to copy customization here

//----------------------------------------------------------------------

CreatureObject * SwgCuiAvatarCreationHelper::duplicateCreatureWithClothesAndCustomization(CreatureObject const & basea, bool includeAppearanceItems)
{
	CreatureObject * const newCreature = duplicateCreature(basea);
	if(newCreature)
	{
		newCreature->setNetworkId(ClientImageDesignerManager::getNextClientNetworkId());
		CustomizationData const * const sourceCustomizationData = basea.fetchCustomizationData();
		if(sourceCustomizationData)
		{
			std::string const & customizationDataStr = sourceCustomizationData->writeLocalDataToString();
			sourceCustomizationData->release();

			CustomizationData * const targetCustomizationData = newCreature->fetchCustomizationData();
			if(targetCustomizationData)
			{
				targetCustomizationData->loadLocalDataFromString(customizationDataStr);
				targetCustomizationData->release();
			}
		}
	
		bool showBackpack = true;
		bool showHelmet = true;

		if(basea.getPlayerObject())
		{
			showBackpack = basea.getPlayerObject()->getShowBackpack();
			showHelmet = basea.getPlayerObject()->getShowHelmet();
		}

		if(includeAppearanceItems)
		{
			std::vector<WearableEntry> const & appearanceWearables = basea.getAppearanceWearableData();
			for(std::vector<WearableEntry>::const_iterator i = appearanceWearables.begin(); i != appearanceWearables.end(); ++i)
			{
				//Set up streamed wearables for non-player creatures
				SlottedContainer * container = ContainerInterface::getSlottedContainer(*newCreature);
				NOT_NULL(container);

				Container::ContainerErrorCode tmp = Container::CEC_Success;

				ConstCharCrcString const cc = ObjectTemplateList::lookUp(i->m_objectTemplate);
				if (cc.isEmpty())
				{
					WARNING(true, ("SceneCreateObjectByCrc crc %08x was not found in object template list", cc.getCrc()));
					return newCreature;
				}

				TangibleObject * const wearable = safe_cast<TangibleObject*> (ObjectTemplate::createObject (cc.getString()));
				if (wearable)
				{
					wearable->setNetworkId(ClientImageDesignerManager::getNextClientNetworkId());
					wearable->endBaselines();
					CustomizationData * const customizationData = wearable->fetchCustomizationData();
					if(customizationData)
					{
						if(!i->m_appearanceString.empty())
							customizationData->loadLocalDataFromString(i->m_appearanceString);
						customizationData->release();
					}

					if(!showHelmet && PlayerObject::isObjectAHelmet(wearable->getAppearance()))
					{
						wearable->setShouldBakeIntoMesh(false);
					}

					if(!showBackpack && PlayerObject::isObjectABackpack(wearable->getAppearance()))
					{
						wearable->setShouldBakeIntoMesh(false);
					}

					if(wearable->getGameObjectType() == SharedObjectTemplate::GOT_misc_appearance_only_invisible)
						wearable->setShouldBakeIntoMesh(false);

					container->add(*wearable, i->m_arrangement, tmp);
				}
				else
				{
					DEBUG_WARNING(true, ("Failed to create wearable %s", cc.getString()));
				}
			}
		}

		std::vector<WearableEntry> const & wearables = basea.getWearableData();
		for(std::vector<WearableEntry>::const_iterator i = wearables.begin(); i != wearables.end(); ++i)
		{
			//Set up streamed wearables for non-player creatures
			SlottedContainer * container = ContainerInterface::getSlottedContainer(*newCreature);
			NOT_NULL(container);
			
			Container::ContainerErrorCode tmp = Container::CEC_Success;

			ConstCharCrcString const cc = ObjectTemplateList::lookUp(i->m_objectTemplate);
			if (cc.isEmpty())
			{
				WARNING(true, ("SceneCreateObjectByCrc crc %08x was not found in object template list", cc.getCrc()));
				return newCreature;
			}

			TangibleObject * const wearable = safe_cast<TangibleObject*> (ObjectTemplate::createObject (cc.getString()));
			if (wearable)
			{
				wearable->setNetworkId(ClientImageDesignerManager::getNextClientNetworkId());
				wearable->endBaselines();
				CustomizationData * const customizationData = wearable->fetchCustomizationData();
				if(customizationData)
				{
					if(!i->m_appearanceString.empty())
						customizationData->loadLocalDataFromString(i->m_appearanceString);
					customizationData->release();
				}

				if(!showHelmet && PlayerObject::isObjectAHelmet(wearable->getAppearance()))
				{
					wearable->setShouldBakeIntoMesh(false);
				}
				
				if(!showBackpack && PlayerObject::isObjectABackpack(wearable->getAppearance()))
				{
					wearable->setShouldBakeIntoMesh(false);
				}

				if(container->mayAdd(*wearable, i->m_arrangement, tmp))
					container->add(*wearable, i->m_arrangement, tmp);
				else
					delete wearable;
			}
			else
			{
				DEBUG_WARNING(true, ("Failed to create wearable %s", cc.getString()));
			}
		}

		//copy hair customization (if hairs exist)
		ClientObject const * const sourceHair = basea.getHairObject();
		TangibleObject const * const tangibleSourceHair = sourceHair ? sourceHair->asTangibleObject() : NULL;
		ClientObject * const destHair = newCreature->getHairObject();
		TangibleObject * const tangibleDestHair = destHair ? destHair->asTangibleObject() : NULL;
		if(tangibleSourceHair && tangibleDestHair)
		{
			CustomizationData const * const sourceCustomizationData = tangibleSourceHair->fetchCustomizationData();
			if(sourceCustomizationData)
			{
				std::string const & customizationDataStr = sourceCustomizationData->writeLocalDataToString();
				sourceCustomizationData->release();

				CustomizationData * const targetCustomizationData = tangibleDestHair->fetchCustomizationData();
				if(targetCustomizationData)
				{
					targetCustomizationData->loadLocalDataFromString(customizationDataStr);
					targetCustomizationData->release();
				}
			}
		}
	}
	return newCreature;
}

//----------------------------------------------------------------------
	
const std::string & SwgCuiAvatarCreationHelper::getProfession ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	return s_profession;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::setProfession (const std::string & s)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	s_profession = s;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::copyHair (CreatureObject &src, CreatureObject &dest, bool assignClientId)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	TangibleObject * const srcHair  = safe_cast<TangibleObject *>(ContainerInterface::getObjectInSlot (src,  s_hairSlot));
	TangibleObject * copiedHair = 0;

	if (srcHair)
	{
		copiedHair = safe_cast<TangibleObject *>(srcHair->getObjectTemplate ()->createObject ());
		if (copiedHair)
		{
			if(assignClientId)
				copiedHair->setNetworkId(ClientImageDesignerManager::getNextClientNetworkId());
			//-- retrieve CustomizationData for source hair
			CustomizationData * const sourceData = srcHair->fetchCustomizationData ();
			
			if (sourceData)
			{
				copiedHair->setAppearanceData (sourceData->writeLocalDataToString ());
				sourceData->release ();
			}
			copiedHair->endBaselines ();
		}
	}

	CuiCharacterHairManager::replaceHair (dest, copiedHair, true);
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::purgeExtraPoolMembers    ()
{	
	CreatureObject * const baseCreature = s_creature.getPointer ();
	
	if (baseCreature)
	{
		//-- hack to make this piece of garbage work
		s_creaturePool->erase (std::remove (s_creaturePool->begin (), s_creaturePool->end (), baseCreature), s_creaturePool->end ());

		std::for_each (s_creaturePool->begin (), s_creaturePool->end (), PointerDeleter ());
		s_creaturePool->clear ();

		s_creaturePool->push_back (baseCreature);
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::getCreaturesFromPool (CreatureVector & cv, int n)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	DEBUG_FATAL (n < 0, ("bad n"));

	cv.clear ();

	if (s_creaturePool->empty ())
	{
		WARNING (true, ("Can't request creatures from pool until there is at least one within it."));
		return;
	}

	cv.reserve              (static_cast<size_t>(n));
	s_creaturePool->reserve (static_cast<size_t>(n));
	
	CreatureObject * const baseCreature = NON_NULL (s_creature.getPointer ());
	
	for (int i = 0; i < n; ++i)
	{
		if (i >= static_cast<int>(s_creaturePool->size ()))
		{
			CreatureObject * const newCreature = NON_NULL (duplicateCreature (*baseCreature));
			s_creaturePool->push_back (newCreature);
		}
		
		CreatureObject * const creature = NON_NULL ((*s_creaturePool) [static_cast<size_t>(i)]);

		if (creature != baseCreature)
		{
			copyHair (*baseCreature, *creature);
			
			creature->setObjectName (baseCreature->getObjectName ());
			
			//-- retrieve CustomizationData for source creature
			
			CustomizationData * const sourceData = baseCreature->fetchCustomizationData ();
			
			if (sourceData)
			{
				creature->setAppearanceData (sourceData->writeLocalDataToString ());

				/*
				//- shared customization data is broken
				//--

				//-- assign CustomizationData to new creature
				Appearance * const destAppearance = creature->getAppearance ();
				if (destAppearance)
					destAppearance->setCustomizationData (sourceData);				

				*/

				//-- release local reference
				sourceData->release ();
			}

			creature->setScaleFactor (baseCreature->getScaleFactor ());
			creature->setScale       (baseCreature->getScale ());
		}

		creature->resetRotateTranslate_o2p ();
		cv.push_back (creature);
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::purgePool (bool andThePedestalToo)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	std::for_each (s_creaturePool->begin (), s_creaturePool->end (), PointerDeleter ());
	s_creaturePool->clear ();
	s_creature = 0;
	s_profession.clear    ();
	s_randomName.clear    ();

	setBiography (Unicode::emptyString);

	if (s_pedestal && andThePedestalToo)
	{
		delete s_pedestal;
		s_pedestal = 0;
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::unlinkPoolCustomizations ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	CreatureObject * const baseCreature = s_creature.getPointer ();

	if (!baseCreature)
	{
		DEBUG_FATAL (!s_creaturePool->empty (), ("bad"));
		return;
	}

	for (CreatureVector::iterator it = s_creaturePool->begin (); it != s_creaturePool->end (); ++it)
	{
		CreatureObject * const creature = NON_NULL (*it);

		if (creature == baseCreature)
			continue;

	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::onCompleted (bool success, const NetworkId & networkId)
{
	if (success && s_creature)
	{
		const NetworkId oldId = s_creature->getNetworkId ();

		s_creature->setNetworkId (networkId);

		//-- save the avatar's appearance
		if (!CuiCachedAvatarManager::saveAvatar (*NON_NULL(s_creature.getPointer ()), networkId, GameNetwork::getUserName (), GameNetwork::getCentralServerName ()))
			WARNING (true, ("Unable to save avatar"));

		//-- insert avatar onto the avatar list
		CuiLoginManager::addAvatarToList (*NON_NULL (s_creature.getPointer ()), networkId);

		s_creature->setNetworkId (oldId);
	}
	
	if(m_messageBox)
		m_messageBox->closeMessageBox();

	if (s_messageBoxWaitingName)
		s_messageBoxWaitingName->closeMessageBox ();
	
	Transceivers::creationFinished.emitMessage (success);
}

//----------------------------------------------------------------------

const std::string & SwgCuiAvatarCreationHelper::getStartingPlanet        ()
{
	return s_startingLocation;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::setStartingPlanet        (const std::string & s)
{
	s_startingLocation = s;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::requestRandomName        (bool showDialog)
{
	CreatureObject * const creature          = SwgCuiAvatarCreationHelper::getCreature ();
	
	std::string templateName;
	if (s_automaticFinish)
	{
		templateName = "object/creature/player/human_male.iff";
	}
	else
	{
		if (!creature)
		{
			WARNING (true, ("need a creature for that"));
			return;
		}
		
		if (s_messageBoxWaitingName)
			s_messageBoxWaitingName->closeMessageBox ();
		
		if (showDialog)
		{
			//-- create the message box
			s_messageBoxWaitingName = CuiMessageBox::createMessageBox (CuiStringIds::avatar_wait_random_name.localize ());
			s_messageBoxWaitingName->setRunner (true);
			s_messageBoxWaitingName->connectToMessages (*s_listener);
		}
		
		templateName = convertSharedToServer (creature->getObjectTemplateName ());
	}
	
	const ClientRandomNameRequest req (templateName);
	GameNetwork::send (req, true); 
}

//----------------------------------------------------------------------

const Unicode::String &   SwgCuiAvatarCreationHelper::getRandomName            ()
{
	return s_randomName;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::onRandomNameReceived (const std::string & serverTemplateName, const Unicode::String & name, const StringId &errorMessage)
{
	if (s_messageBoxWaitingName)
		s_messageBoxWaitingName->closeMessageBox ();
	
	if (s_automaticFinish)
	{
		s_randomName = name;
		finishCreation (true);
	}
	else
	{
		CreatureObject * const creature          = SwgCuiAvatarCreationHelper::getCreature ();
		
		if (!creature)
		{
			s_randomName.clear ();
		}
		else
		{
			const std::string creatureServerTemplate = convertSharedToServer (creature->getObjectTemplateName ());
			
			if (!creature || creatureServerTemplate != serverTemplateName)
			{
				WARNING (true, ("Received random name '%s' for template '%s' but current template is '%s'", Unicode::wideToNarrow (name).c_str (), serverTemplateName.c_str (), creatureServerTemplate.c_str ()));
				s_randomName.clear ();
				return;
			}
			
			if (errorMessage != NameErrors::nameApproved)
			{
				Unicode::String errorText;
				errorMessage.localize(errorText);
				WARNING (true, ("Received name error '%s' for template '%s' but current template is '%s'", Unicode::wideToNarrow(errorText).c_str (), serverTemplateName.c_str (), creatureServerTemplate.c_str ()));
				s_randomName.clear ();
				return;
			}
			
			if (name.empty ())
			{
				WARNING (true, ("Received empty name"));
				return;
			}
			
			s_randomName = name;
		}

		Transceivers::randomNameChanged.emitMessage (name);
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::onClientVerifyAndLockNameResponseReceived(Unicode::String const & name, StringId const & errorMessage)
{
	Messages::VerifyAndLockNameResponse::Payload payload;
	payload.success = (errorMessage == NameErrors::nameApproved) ? true : false;
	payload.name = name;
	payload.errorMessage = errorMessage;

	Transceivers::verifyAndLockNameResponse.emitMessage(payload);
}

//----------------------------------------------------------------------

bool SwgCuiAvatarCreationHelper::getCreatureCustomized()
{
	return s_creatureCustomized;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::setCreatureCustomized(bool b)
{
	s_creatureCustomized = b;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::setBiography(const Unicode::String & str)
{
	s_biography = str;
}

//----------------------------------------------------------------------

const Unicode::String & SwgCuiAvatarCreationHelper::getBiography()
{
	return s_biography;
}

//----------------------------------------------------------------------

bool SwgCuiAvatarCreationHelper::wasLastCreationAutomatic(Unicode::String & name)
{
	if (s_lastCreationAutomatic)
	{
		name = s_automaticName;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

TangibleObject * SwgCuiAvatarCreationHelper::getPedestal              ()
{
	if (!s_pedestal)
	{	
		const bool asynchronousLoaderEnabled = AsynchronousLoader::isEnabled ();
		AsynchronousLoader::disable ();
		static const std::string pedestalTemplateName = "object/tangible/ui/shared_ui_char_cust_pedestal.iff";
		s_pedestal = safe_cast<TangibleObject *>(ObjectTemplate::createObject (pedestalTemplateName.c_str ()));
		if (asynchronousLoaderEnabled)
			AsynchronousLoader::enable ();
		s_pedestal->endBaselines ();
	}

	return s_pedestal;
}

//----------------------------------------------------------------------

float SwgCuiAvatarCreationHelper::getPedestalOffsetFromTop ()
{
	return -0.102509f;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::setCreatingJedi(bool b)
{
	ms_isCreatingJedi = b;
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::restartMusic             (bool onlyIfNotPlaying)
{
	PerformanceTimer performanceTimer;
	performanceTimer.start();
	float waitTimer = 0.0f;

	if (!onlyIfNotPlaying || !Audio::isSoundPlaying (s_musicId))
	{
		Audio::stopSound (s_musicId, 1.0f);
	}
	
	while (waitTimer < 1.0f)
	{
		performanceTimer.stop();
		float const deltaTime = performanceTimer.getElapsedTime();
		performanceTimer.start();
		
		waitTimer += deltaTime;
		Audio::alter (deltaTime, NULL);
		Sleep        (5);
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarCreationHelper::stopMusic                (float fadeout, bool restartMainMusic)
{
	PerformanceTimer performanceTimer;
	performanceTimer.start();
	float waitTimer = 0.0f;

	Audio::stopSound (s_musicId, fadeout);
	if (restartMainMusic)
		CuiManager::restartMusic (true);

	while (waitTimer < fadeout)
	{
		performanceTimer.stop();
		float const deltaTime = performanceTimer.getElapsedTime();
		performanceTimer.start();
		
		waitTimer += deltaTime;
		Audio::alter (deltaTime, NULL);
		Sleep        (5);
	}
}

//----------------------------------------------------------------------

std::string SwgCuiAvatarCreationHelper::convertSharedToServer (const std::string & objectTemplateName)
{
	const size_t sharedPos = objectTemplateName.find("shared_", 0);
	DEBUG_FATAL(sharedPos == std::string::npos, ("Cannot find \"shared_\" keyword in template name '%s'", objectTemplateName.c_str ()));
	return objectTemplateName.substr(0, sharedPos) + objectTemplateName.substr(sharedPos + strlen("shared_"));
}

//======================================================================
