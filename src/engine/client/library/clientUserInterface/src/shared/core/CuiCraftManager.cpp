//======================================================================
//
// CuiCraftManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiCraftManager.h"

#include "UnicodeUtils.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientCommandQueueEntry.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/FactoryObject.h"
#include "clientGame/Game.h"
#include "clientGame/ManufactureSchematicObject.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/ResourceContainerObject.h"
#include "clientUserInterface/CuiCraftManagerSimulator.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMediatorTypes.h"
#include "clientUserInterface/CuiMenuInfoTypes.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringIdsCraft.h"
#include "clientUserInterface/CuiStringTables.h"
#include "clientUserInterface/CuiSystemMessageManager.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueCraftCustomization.h"
#include "sharedNetworkMessages/MessageQueueCraftEmptySlot.h"
#include "sharedNetworkMessages/MessageQueueCraftExperiment.h"
#include "sharedNetworkMessages/MessageQueueCraftIngredients.h"
#include "sharedNetworkMessages/MessageQueueCraftRequestSession.h"
#include "sharedNetworkMessages/MessageQueueCraftSelectSchematic.h"
#include "sharedNetworkMessages/MessageQueueDraftSchematics.h"
#include "sharedNetworkMessages/MessageQueueDraftSlots.h"
#include "sharedNetworkMessages/MessageQueueGeneric.h"
#include "sharedNetworkMessages/MessageQueueGenericIntResponse.h"
#include "sharedNetworkMessages/MessageQueueGenericResponse.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedRandom/Random.h"
#include <vector>

//- this stuff is for testing clientside hoppers
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/ObjectTemplate.h"

//lint -esym(641, Crafting::CraftingStage) // convert enum to int
//lint -esym(641, GameControllerMessage) // convert enum to int

//======================================================================

namespace
{

	bool s_installed = false;

	//----------------------------------------------------------------------

	const std::pair<int, const StringId *> s_errorCodeValues[] = 
	{
		std::make_pair(Crafting::CE_noOwner,                 &CuiStringIdsCraft::err_no_owner),
		std::make_pair(Crafting::CE_notAssemblyStage,        &CuiStringIdsCraft::err_not_assembly_stage),
		std::make_pair(Crafting::CE_notCustomizeStage,       &CuiStringIdsCraft::err_not_customize_stage),
		std::make_pair(Crafting::CE_noDraftSchematic,        &CuiStringIdsCraft::err_no_draft_schematic),
		std::make_pair(Crafting::CE_noCraftingTool,          &CuiStringIdsCraft::err_no_crafting_tool),
		std::make_pair(Crafting::CE_noManfSchematic,         &CuiStringIdsCraft::err_no_manf_schematic),
		std::make_pair(Crafting::CE_invalidSlot,             &CuiStringIdsCraft::err_invalid_slot),
		std::make_pair(Crafting::CE_invalidSlotOption,       &CuiStringIdsCraft::err_invalid_slot_option),
		std::make_pair(Crafting::CE_invalidIngredientSize,   &CuiStringIdsCraft::err_invalid_ingredient_size),
		std::make_pair(Crafting::CE_slotFull,                &CuiStringIdsCraft::err_slot_full),
		std::make_pair(Crafting::CE_invalidIngredient,       &CuiStringIdsCraft::err_invalid_ingredient),
		std::make_pair(Crafting::CE_ingredientNotInInventory,&CuiStringIdsCraft::err_ingredient_not_in_inventory),
		std::make_pair(Crafting::CE_cantRemoveResource,      &CuiStringIdsCraft::err_cant_remove_resource),
		std::make_pair(Crafting::CE_wrongResource,           &CuiStringIdsCraft::err_wrong_resource),
		std::make_pair(Crafting::CE_damagedComponent,        &CuiStringIdsCraft::err_damaged_component),
		std::make_pair(Crafting::CE_cantTransferComponent,   &CuiStringIdsCraft::err_cant_transfer_component),
		std::make_pair(Crafting::CE_wrongComponent,          &CuiStringIdsCraft::err_wrong_component),
		std::make_pair(Crafting::CE_noInventory,             &CuiStringIdsCraft::err_no_inventory),
		std::make_pair(Crafting::CE_badTargetHopper,         &CuiStringIdsCraft::err_bad_target_hopper),
		std::make_pair(Crafting::CE_badTargetContainer,      &CuiStringIdsCraft::err_bad_target_container),
		std::make_pair(Crafting::CE_emptySlot,               &CuiStringIdsCraft::err_empty_slot),
		std::make_pair(Crafting::CE_cantCreateResourceCrate, &CuiStringIdsCraft::err_cant_create_resource_crate),
		std::make_pair(Crafting::CE_emptySlotAssembly,       &CuiStringIdsCraft::err_empty_slot_assembly),
		std::make_pair(Crafting::CE_partialSlotAssembly,     &CuiStringIdsCraft::err_partial_slot_assembly),
		std::make_pair(Crafting::CE_noPrototype,             &CuiStringIdsCraft::err_no_prototype),
		std::make_pair(Crafting::CE_invalidCraftedName,      &CuiStringIdsCraft::err_invalid_crafted_name),
		std::make_pair(Crafting::CE_readOnlyDraftSchematic,  &CuiStringIdsCraft::err_read_only_draft_schematic),
		std::make_pair(Crafting::CE_invalidBioLink,          &CuiStringIdsCraft::err_invalid_bio_link),
		std::make_pair(Crafting::CE_stackedLoot,             &CuiStringIdsCraft::err_stacked_loot),
	};
	
	const uint32 START_CRAFTING_COMMAND            = Crc::normalizeAndCalculate("requestCraftingSession");
	const uint32 CANCEL_CRAFTING_SESSION_COMMAND   = Crc::normalizeAndCalculate("cancelCraftingSession");
	const uint32 RESTART_CRAFTING_SESSION_COMMAND  = Crc::normalizeAndCalculate("restartCraftingSession");
	const uint32 SELECT_SCHEMATIC_COMMAND          = Crc::normalizeAndCalculate("selectDraftSchematic");
	const uint32 NEXT_CRAFTING_STAGE_COMMAND       = Crc::normalizeAndCalculate("nextCraftingStage");
	const uint32 CREATE_PROTOTYPE_COMMAND          = Crc::normalizeAndCalculate("createPrototype");
	const uint32 CREATE_MANF_SCHEMATIC_COMMAND     = Crc::normalizeAndCalculate("createManfSchematic");
	
	const int s_numErrorCodes = sizeof(s_errorCodeValues) / sizeof(s_errorCodeValues[0]);

	typedef std::map<int, const StringId *> CraftingErrorCodeMap;
	CraftingErrorCodeMap s_errorCodeMap(&s_errorCodeValues[0], &s_errorCodeValues[s_numErrorCodes]);

	std::pair<uint32, uint32> s_currentDraftSchematicCrc;
	//----------------------------------------------------------------------

	namespace Mediators
	{
		const char * const Draft = "WS_CraftDraft";
	}

	//----------------------------------------------------------------------

	namespace Transceivers
	{
		MessageDispatch::Transceiver<const CuiCraftManager::Messages::DraftSchematicsReceived::Payload &,   CuiCraftManager::Messages::DraftSchematicsReceived > 
			draftSchematics;

		MessageDispatch::Transceiver<const CuiCraftManager::Messages::DraftSlotsReceived::Payload &,        CuiCraftManager::Messages::DraftSlotsReceived > 
			draftSlots;

		MessageDispatch::Transceiver<const CuiCraftManager::Messages::StageChange::Payload &,               CuiCraftManager::Messages::StageChange > 
			stageChange;

		MessageDispatch::Transceiver<const CuiCraftManager::Messages::Assembling::Payload &,                CuiCraftManager::Messages::Assembling > 
			assembling;

		MessageDispatch::Transceiver<const CuiCraftManager::Messages::DoneAssembling::Payload &,            CuiCraftManager::Messages::DoneAssembling > 
			doneAssembling;

		MessageDispatch::Transceiver<const CuiCraftManager::Messages::Complete::Payload &,                  CuiCraftManager::Messages::Complete > 
			complete;

		MessageDispatch::Transceiver<const CuiCraftManager::Messages::Customize::Payload &,                 CuiCraftManager::Messages::Customize > 
			customize;

		MessageDispatch::Transceiver<const CuiCraftManager::Messages::Experiment::Payload &,                CuiCraftManager::Messages::Experiment > 
			experiment;

		MessageDispatch::Transceiver<const CuiCraftManager::Messages::SessionEnded::Payload &,              CuiCraftManager::Messages::SessionEnded > 
			sessionEnded;

		MessageDispatch::Transceiver<const CuiCraftManager::Messages::SlotTransferStatusChanged::Payload &, CuiCraftManager::Messages::SlotTransferStatusChanged > 
			slotTransferStatusChanged;		
	}

	//----------------------------------------------------------------------

	Crafting::CraftingStage s_lastStage = Crafting::CS_none;

	//----------------------------------------------------------------------

	namespace Slots
	{
		const std::string  hopper_output     = "anythingCanMod1";
		const char * const ingredient_hopper = "ingredient_hopper";
	}

	//----------------------------------------------------------------------

	typedef CuiCraftManager::SlotVector SlotVector;
	typedef CuiCraftManager::SchematicVector SchematicVector;

	SlotVector                  s_slotVector;
	SchematicVector             s_schematicVector;
	CuiCraftManager::FinalState s_finalState;
	bool                        s_isPractice;
	bool                        s_customizationSet = false;
	bool                        s_canManufacture = true;
	bool                        s_isBioLinked = false;

	Unicode::String             s_localizedAssemblyResult;

	//----------------------------------------------------------------------

	namespace Sequence
	{
		uint8 requestAssemble        = 0;
		uint8 requestStartCrafting   = 0;
		uint8 requestCustomize       = 0;
		uint8 requestRestartCrafting = 0;
		uint8 waitingTransfer        = 0;
		uint8 requestComplete        = 0;
		uint8 requestFinish          = 0;
		uint8 requestExperiment      = 0;
	}

	//----------------------------------------------------------------------

	namespace MessageBoxes
	{
		CuiMessageBox * requestAssemble        = 0;
		CuiMessageBox * requestComplete        = 0;
		CuiMessageBox * requestCustomize       = 0;
		CuiMessageBox * confirmComplete        = 0;
		CuiMessageBox * confirmDestroy         = 0;
		CuiMessageBox * confirmAssemble        = 0;
		CuiMessageBox * confirmCancel          = 0;
		CuiMessageBox * waitingTransfer        = 0;
		CuiMessageBox * requestStartCrafting   = 0;
		CuiMessageBox * waitingDraftSlots      = 0;
		CuiMessageBox * requestRestartCrafting = 0;
		CuiMessageBox * requestFinish          = 0;
		CuiMessageBox * requestExperiment      = 0;
		CuiMessageBox * waitingManfSchem       = 0;
		CuiMessageBox * requestSendCustomizationData = 0;
	}		

	//----------------------------------------------------------------------

	class TransferPendingInfo
	{
	public:
		CachedNetworkId item;
		int             slot;
		int             option;

		TransferPendingInfo (const NetworkId & _item, int _slot, int _option) :
		item   (_item),
		slot   (_slot),
		option (_option)
		{
		}

		TransferPendingInfo (const CachedNetworkId & _item, int _slot, int _option) :
		item   (_item),
		slot   (_slot),
		option (_option)
		{
		}

		TransferPendingInfo () :
		item   (),
		slot   (0),
		option (0)
		{
		}
	};

	typedef stdmap<uint8, TransferPendingInfo>::fwd TransferPendingMap;

	TransferPendingMap s_transferToSlotPending;
	TransferPendingMap s_removeFromSlotPending;

	//----------------------------------------------------------------------

	CuiCraftManager::IntVector s_slotTransferStatus;

	//----------------------------------------------------------------------

	/**
	* Increment and skip zero
	*/
	void incrementSequence(uint8 & val)
	{
		if (++val == 0)
			++val;
	}

	//----------------------------------------------------------------------

	Controller * findController ()
	{
		return Game::getPlayer () ? Game::getPlayer ()->getController () : 0;
	}

	//----------------------------------------------------------------------

	struct Listener : public MessageDispatch::Receiver 
	{
		Listener () :
			MessageDispatch::Receiver (),
			m_boxPtrs ()
		{			
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::requestAssemble,        &Sequence::requestAssemble));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::requestComplete,        &Sequence::requestComplete));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::requestCustomize,       &Sequence::requestCustomize));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::requestFinish,          &Sequence::requestFinish));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::confirmDestroy,         0));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::confirmAssemble,        0));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::confirmComplete,        0));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::waitingTransfer,        &Sequence::waitingTransfer));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::requestStartCrafting,   &Sequence::requestStartCrafting));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::requestRestartCrafting, &Sequence::requestRestartCrafting));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::waitingDraftSlots,      0));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::requestExperiment,      &Sequence::requestExperiment));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::waitingManfSchem,       0));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::confirmCancel,          0));
			m_boxPtrs.push_back (BoxPtr (&MessageBoxes::requestSendCustomizationData, 0));
		}

		//-----------------------------------------------------------------

		void cancelRequest (CuiMessageBox * const & box)
		{
			for (BoxPtrs::iterator it = m_boxPtrs.begin (); it != m_boxPtrs.end (); ++it)
			{
				if ((*it).first == &box)
				{
					if (box)
						box->closeMessageBox ();
					if ((*it).second)
						incrementSequence(*(*it).second);
				}
			}			
		}
		 
		//-----------------------------------------------------------------

		void receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
		{
			const CuiMessageBox::BoxMessage * const abm = dynamic_cast<const CuiMessageBox::BoxMessage *>(&message);
			
			if (abm)
			{
				if (message.isType (CuiMessageBox::Messages::COMPLETED))
				{
					const CuiMessageBox::CompletedMessage * const cmsg = dynamic_cast<const CuiMessageBox::CompletedMessage *>(&message);
					NOT_NULL (cmsg);

					if (cmsg->getMessageBox ()->completedAffirmative ())
					{
						if (abm->getMessageBox () == MessageBoxes::confirmAssemble)
							CuiCraftManager::assemble (true);
						else if (abm->getMessageBox () == MessageBoxes::confirmComplete)
							CuiCraftManager::complete (true, s_isPractice);
						else if (abm->getMessageBox () == MessageBoxes::confirmCancel)
							CuiCraftManager::stopCrafting (true, true);

					}
					else
					{
						for (BoxPtrs::iterator it = m_boxPtrs.begin (); it != m_boxPtrs.end (); ++it)
						{
							if (*(*it).first == abm->getMessageBox () && (*it).second)
								incrementSequence (*(*it).second);
						}
					}
				}
				else if (message.isType (CuiMessageBox::Messages::CLOSED))
				{
					for (BoxPtrs::iterator it = m_boxPtrs.begin (); it != m_boxPtrs.end (); ++it)
					{
						if (*(*it).first == abm->getMessageBox ())
							*(*it).first = 0;
					}
				}
			}
		}

		typedef std::pair<CuiMessageBox **, uint8 *> BoxPtr;
		typedef std::vector <BoxPtr> BoxPtrs;
		BoxPtrs	m_boxPtrs;

	};

	//----------------------------------------------------------------------
	
	Listener * s_listener = 0;
	
	//----------------------------------------------------------------------
	
	class MyCallback : 
	public MessageDispatch::Callback
	{
	public:

		MyCallback () : MessageDispatch::Callback () {}	

		//----------------------------------------------------------------------

		void onManfSchemReady (const ManufactureSchematicObject & manf)
		{
			if (manf.getNetworkId () == CuiCraftManager::Ids::manufactureSchematic)
			{
				if (s_listener)
					s_listener->cancelRequest (MessageBoxes::waitingManfSchem);

				Transceivers::draftSlots.emitMessage (s_slotVector);
			}
		} //lint !e1762

	};
	
	MyCallback s_callback;
	
	//----------------------------------------------------------------------

}

//----------------------------------------------------------------------

CachedNetworkId CuiCraftManager::Ids::craftingTool;
CachedNetworkId CuiCraftManager::Ids::craftingToolPending;
CachedNetworkId CuiCraftManager::Ids::craftingToolHopperInput;
CachedNetworkId CuiCraftManager::Ids::manufactureSchematic;
CachedNetworkId CuiCraftManager::Ids::schematicPrototype;

//----------------------------------------------------------------------

void CuiCraftManager::install ()
{
	DEBUG_FATAL (s_installed, ("installed"));
	s_callback.connect (s_callback, &MyCallback::onManfSchemReady,             static_cast<ManufactureSchematicObject::Messages::Ready *>     (0));

	s_listener = new Listener;
	s_installed = true;
}

//----------------------------------------------------------------------

void CuiCraftManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	s_callback.disconnect (s_callback, &MyCallback::onManfSchemReady,             static_cast<ManufactureSchematicObject::Messages::Ready *>     (0));

	delete s_listener;
	s_listener = 0;
	s_installed = false;
}

//----------------------------------------------------------------------

void CuiCraftManager::receiveSessionEnded  (bool normalExit)
{	
	if (!normalExit)
	{
		s_finalState = FS_none;
		Ids::craftingToolPending = CachedNetworkId::cms_invalid;
		Ids::craftingTool        = CachedNetworkId::cms_invalid;
		s_schematicVector.clear ();
		s_currentDraftSchematicCrc = std::make_pair(0, 0);
		s_slotVector.clear ();
		s_transferToSlotPending.clear ();
		s_removeFromSlotPending.clear ();
		s_slotTransferStatus.clear    ();

		CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIdsCraft::session_ended.localize ());
		Transceivers::sessionEnded.emitMessage (true);
		s_customizationSet = false;
	}
}

//----------------------------------------------------------------------

void CuiCraftManager::receiveDraftSchematics (const MessageQueueDraftSchematics & schematics)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	if (schematics.getToolId() != Ids::craftingToolPending)
	{
		if (Ids::craftingToolPending != NetworkId::cms_invalid)
	{
		WARNING (true, ("receiveDraftSchematics got tool %s but expected tool %s\n",
			schematics.getToolId().getValueString().c_str(),
			Ids::craftingToolPending.getValueString().c_str()));
		WARNING (true, ("discarding stale MessageQueueDraftSchematics response"));
		return;
	}
		else
		{
			TangibleObject* tool = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(schematics.getToolId()));
			if (tool && !tool->getClientSynchronizedUi())
			{
				// We haven't started crafting with this tool, so start now
				tool->startCrafting();
			}
		}
	}

	s_listener->cancelRequest (MessageBoxes::requestStartCrafting );

	//----------------------------------------------------------------------
	//-- no valid schematics
	if (schematics.getSchematics ().empty ())
	{
		stopCrafting (true);
		CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_no_draft_schematics.localize ());
		return;
	}


	Ids::craftingTool         = schematics.getToolId();
	Ids::craftingToolPending  = NetworkId::cms_invalid;
	Ids::manufactureSchematic = NetworkId::cms_invalid;
	Ids::schematicPrototype   = NetworkId::cms_invalid;
	s_finalState	          = FS_none;
	s_customizationSet        = false;

	s_schematicVector = schematics.getSchematics ();
	//@todo: receive hopper input networkid
	//Ids::craftingStationHopperInput = ???

	CuiMediatorFactory::activateInWorkspace (Mediators::Draft);

	Transceivers::draftSchematics.emitMessage (s_schematicVector);
}

//----------------------------------------------------------------------

void CuiCraftManager::receiveDraftSlots      (const MessageQueueDraftSlots & slots)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	if (slots.getToolId () != Ids::craftingTool)
	{
		WARNING (true, ("discarding stale requestDraftSlots response"));
		return;
	}

	s_listener->cancelRequest (MessageBoxes::waitingDraftSlots );
	s_slotVector              = slots.getSlots ();
	Ids::manufactureSchematic = slots.getManfSchemId ();
	Ids::schematicPrototype   = slots.getPrototypeId ();
	s_slotTransferStatus.clear    ();
	s_slotTransferStatus.resize (s_slotVector.size (), 0);
	s_canManufacture = slots.canManufacture();
	s_isBioLinked = false;

	bool wait = true;	
	if (Ids::manufactureSchematic.getObject ())
	{
		const ManufactureSchematicObject * const schematic = safe_cast<const ManufactureSchematicObject *>(Ids::manufactureSchematic.getObject());
		if (schematic->isReadyToCraft())
		{
			Transceivers::draftSlots.emitMessage (s_slotVector);
			wait = false;
		}
	}
	if (wait)
	{
		MessageBoxes::waitingManfSchem = CuiMessageBox::createMessageBox (CuiStringIdsCraft::wait_manf_schem.localize ());
		MessageBoxes::waitingManfSchem->setRunner (true);
		MessageBoxes::waitingManfSchem->connectToMessages (*s_listener);
	}
}

//----------------------------------------------------------------------

void CuiCraftManager::receiveNextCraftingStageResult(const MessageQueueGenericIntResponse & resp)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	const Crafting::CraftingStage stage = findCurrentStage ();
	int craftingResult = resp.getResponse();

	switch (s_lastStage)
	{
	case Crafting::CS_assembly:
		{
			if (resp.getSequenceId () != Sequence::requestAssemble)
				WARNING (true, ("discarding stale assemble response"));
			else
			{
				s_listener->cancelRequest (MessageBoxes::requestAssemble );
				
				if (findCraftingLevel() >= 2)
				{
					if (stage != Crafting::CS_experiment)
						WARNING (true, ("expected stage experiment %d, got %d", Crafting::CS_experiment, stage));
				}
				else
				{
					if (stage != Crafting::CS_customize)
						WARNING (true, ("expected stage customize %d, got %d", Crafting::CS_customize, stage));
				}
				
				if (craftingResult >= 0 && Ids::schematicPrototype.getObject())
				{
					reportAssemblyResult (static_cast<Crafting::CraftingResult>(craftingResult));
					if (craftingResult != Crafting::CR_criticalFailure)
						Transceivers::stageChange.emitMessage (stage);
					else
						Transceivers::doneAssembling.emitMessage (0);
				}
				else
				{
					CraftingErrorCodeMap::const_iterator error = s_errorCodeMap.find(-craftingResult);
					if (error != s_errorCodeMap.end())
						CuiMessageBox::createInfoBox ((*error).second->localize());
					else
						CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_assembly.localize ());
				}
			}
		}
		break;
		
	case Crafting::CS_experiment:
		{
			if (resp.getSequenceId () != Sequence::requestCustomize)
				WARNING (true, ("discarding stale customize response"));
			else
			{
				s_listener->cancelRequest (MessageBoxes::requestCustomize );

				if (stage != Crafting::CS_customize)
					WARNING (true, ("expected stage customize %d, got %d", Crafting::CS_customize, stage));

				else if (craftingResult != Crafting::CR_success)
					CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_customize.localize ());
				else
				{
					Transceivers::customize.emitMessage (stage);
				}
			}
		}
		break;
	
	case Crafting::CS_customize:
		{
			if (resp.getSequenceId () != Sequence::requestFinish)
				WARNING (true, ("discarding stale finish response"));
			else
			{
				s_listener->cancelRequest (MessageBoxes::requestFinish );

				if (stage != Crafting::CS_finish)
					WARNING (true, ("expected stage finish %d, got %d", Crafting::CS_finish, stage));

				else if (craftingResult != Crafting::CR_success)
					CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_finish.localize ());
				else
					CuiCraftManager::complete (true, s_isPractice);
			}
		}
		break;

	case Crafting::CS_none:
	case Crafting::CS_selectDraftSchematic:
	case Crafting::CS_finish:
		break;

	default:
		break;
	}
}

//----------------------------------------------------------------------

void CuiCraftManager::receiveExperimentResult        (const MessageQueueGenericIntResponse & resp)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	if (Sequence::requestExperiment != resp.getSequenceId ())
	{
		WARNING (true, ("discarding stale experiment response"));
		//-- emit message to make sure UIs update properly
		Transceivers::experiment.emitMessage (0);
		return;
	}
	
	s_listener->cancelRequest (MessageBoxes::requestExperiment);
	
	const Crafting::CraftingResult result = static_cast<Crafting::CraftingResult>(resp.getResponse ());

	if (result < Crafting::CR_internalFailure || result >= Crafting::CR_numResults)
	{
		WARNING (true, ("discarding invalid experiment result [%d]", result));
		return;
	}

	if (result == Crafting::CR_internalFailure)
		CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_experiment.localize ());

	reportExperimentationResult (result);

	Transceivers::experiment.emitMessage (0);
}

//----------------------------------------------------------------------

void CuiCraftManager::receiveGenericResponse (const MessageQueueGenericIntResponse & resp)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	
	switch (resp.getRequestId ())
	{
	case CM_requestCraftingSession:
		{
			if (Ids::craftingToolPending != NetworkId::cms_invalid)
			{
				s_listener->cancelRequest (MessageBoxes::requestStartCrafting );

				if (!resp.getResponse ())
				{
					CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_start.localize ());
			}
		}
			// if sequenceId is true, the request was made on a crafting station and failed.
			// see server/PlayerObject::requestCrafting session and CuiRadialMenuManager::performDefaultDoubleClickAction
			else if (!resp.getResponse() && resp.getSequenceId())  
			{
				CreatureObject const * const playerObject = Game::getPlayerCreature();
				CachedNetworkId const craftingObject(playerObject ? playerObject->getLookAtTarget() : CachedNetworkId::cms_cachedInvalid);
				if (playerObject && craftingObject.isValid())
				{
					// Examine instead of displaying an error.
					Cui::MenuInfoTypes::executeCommandForMenu(Cui::MenuInfoTypes::EXAMINE, craftingObject, 0);
				}
			}
		}
		break;
		
	case CM_restartCraftingSession:
		{
			if (resp.getSequenceId () != Sequence::requestRestartCrafting)
				WARNING (true, ("discarding stale CM_restartCraftingSession response"));
			else
			{
				s_listener->cancelRequest (MessageBoxes::requestRestartCrafting );
				
				s_slotVector.clear            ();
				s_transferToSlotPending.clear ();
				s_removeFromSlotPending.clear ();
				s_slotTransferStatus.clear    ();

				Ids::schematicPrototype   = NetworkId::cms_invalid;
				
				if (!resp.getResponse ())
					CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_restart.localize ());
				else
				{
					const Crafting::CraftingStage stage = findCurrentStage ();
					if (stage != Crafting::CS_selectDraftSchematic)
					{
						WARNING (true, ("bad stage in restart: %d", stage));
						Transceivers::stageChange.emitMessage (Crafting::CS_selectDraftSchematic);
					}
					else
						Transceivers::stageChange.emitMessage (stage);
					
					Transceivers::draftSchematics.emitMessage (s_schematicVector);
				}
			}
		}
		break;
		
	case CM_fillSchematicSlotMessage:
		{
			const uint8 sequence = resp.getSequenceId ();
			const TransferPendingMap::iterator it = s_transferToSlotPending.find (sequence);

			if (it == s_transferToSlotPending.end ())
			{
				WARNING (true, ("discarding stale transfer response [%d]", sequence));
				break;
			}
			
			//-- make a copy so it will live through the coming erase
			const TransferPendingInfo tpi = (*it).second;

			DEBUG_FATAL (tpi.slot >= static_cast<int>(s_slotTransferStatus.size ()), ("bad s_slotTransferStatus size [%d]", s_slotTransferStatus.size ()));
			if (s_slotTransferStatus [tpi.slot] != 1)
			{
				WARNING (true, ("CuiCraftManager::receiveGenericResponse CM_fillSchematicSlotMessage received for slot [%d] with slot transfer status [%d]", 
					tpi.slot, s_slotTransferStatus [tpi.slot]));
				break;
			}

			const int result              = resp.getResponse ();
			
			if (result != Crafting::CE_success)
			{
				CraftingErrorCodeMap::const_iterator error = s_errorCodeMap.find (result);
				if (error != s_errorCodeMap.end())
					CuiMessageBox::createInfoBox ((*error).second->localize ());
				else
					CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_transfer.localize ());
			}
			
			s_slotTransferStatus [tpi.slot] = 0;
			s_transferToSlotPending.erase (it);

			Transceivers::slotTransferStatusChanged.emitMessage (true);			
		}
		break;

	case CM_emptySchematicSlotMessage:
		{
			const uint8 sequence = resp.getSequenceId ();
			const TransferPendingMap::iterator it = s_removeFromSlotPending.find (sequence);

			if (it == s_removeFromSlotPending.end ())
			{
				WARNING (true, ("discarding stale remove response [%d]", sequence));
				break;
			}
			
			//-- make a copy so it will live through the coming erase
			const TransferPendingInfo tpi = (*it).second;			

			DEBUG_FATAL (tpi.slot >= static_cast<int>(s_slotTransferStatus.size ()), ("bad s_slotTransferStatus size [%d]", s_slotTransferStatus.size ()));
			if (s_slotTransferStatus [tpi.slot] != -1)
			{
				WARNING (true, ("CuiCraftManager::receiveGenericResponse CM_fillSchematicSlotMessage received for slot [%d] with slot transfer status [%d]", 
					tpi.slot, s_slotTransferStatus [tpi.slot]));
				break;
			}

			const int result              = resp.getResponse ();
			
			if (result != Crafting::CE_success)
			{
				CraftingErrorCodeMap::const_iterator error = s_errorCodeMap.find (result);
				if (error != s_errorCodeMap.end())
					CuiMessageBox::createInfoBox ((*error).second->localize ());
				else
					CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_transfer.localize ());
			}

			s_slotTransferStatus [tpi.slot] = 0;
			s_removeFromSlotPending.erase (it);

			Transceivers::slotTransferStatusChanged.emitMessage (true);			
		}
		break;
		
	case CM_createManfSchematic:
	case CM_createPrototype:
		{
			if (resp.getSequenceId () != Sequence::requestComplete)
				WARNING (true, ("discarding stale complete response"));
			else
			{
				s_listener->cancelRequest (MessageBoxes::requestComplete );
				
				if (!resp.getResponse ())
					CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_complete.localize ());
				else
				{
					if (resp.getRequestId () == CM_createPrototype)
						CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIdsCraft::completed_prototype.localize ());
					else
						CuiSystemMessageManager::sendFakeSystemMessage (CuiStringIdsCraft::completed_manf_schem.localize ());
					
					Transceivers::complete.emitMessage (resp.getRequestId () == CM_createPrototype);
				}
			}
		}
		break;
		
	case CM_setCustomizationData:
		{
			s_listener->cancelRequest (MessageBoxes::requestSendCustomizationData);

			if (resp.getResponse() != Crafting::CE_success)
			{
				CraftingErrorCodeMap::const_iterator error = s_errorCodeMap.find(resp.getResponse());
				if (error != s_errorCodeMap.end())
					CuiMessageBox::createInfoBox ((*error).second->localize ());
				else
					CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_customize.localize ());
			}
			else
			{
				s_customizationSet = true;
				CuiCraftManager::complete (true, s_isPractice);
			}
		}
		break;
		
	default: 
		break;
	}
}

//----------------------------------------------------------------------

void CuiCraftManager::requestDraftSlots      (int draftIndex)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	Controller * const controller = findController ();
	
	if (!controller)
		return;
	
	if (draftIndex < 0 || draftIndex >= static_cast<int>(s_schematicVector.size ()))
		return;

	s_currentDraftSchematicCrc = std::make_pair(s_schematicVector[draftIndex].serverCrc, s_schematicVector[draftIndex].sharedCrc);

	s_listener->cancelRequest (MessageBoxes::waitingDraftSlots );

	if (Game::getSinglePlayer ())
	{
		CuiCraftManagerSimulator::requestDraftSlots ();
	}
	
	else
	{		
		//-- enqueue message
		char buffer[32];
		_itoa(draftIndex, buffer, 10);
		ClientCommandQueue::enqueueCommand(SELECT_SCHEMATIC_COMMAND, NetworkId::cms_invalid, Unicode::narrowToWide(buffer));
		
		MessageBoxes::waitingDraftSlots = CuiMessageBox::createMessageBox (CuiStringIdsCraft::wait_draft_slots.localize ());
		MessageBoxes::waitingDraftSlots->setRunner (true);
		MessageBoxes::waitingDraftSlots->connectToMessages (*s_listener);
	}
}

//----------------------------------------------------------------------

void CuiCraftManager::transferToSlot (const NetworkId & id, int slot, int option)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	DEBUG_FATAL (slot >= static_cast<int>(s_slotTransferStatus.size ()), ("bad s_slotTransferStatus size [%d]", s_slotTransferStatus.size ()));

	if (s_slotTransferStatus [slot])
	{
		WARNING (true, ("CuiCraftManager::transferToSlot cannot be called on slot [%d] with transferstatus [%d]", slot, s_slotTransferStatus [slot]));
		return;
	}

	ManufactureSchematicObject * const manf = getManufactureSchematic ();

	if (manf)
	{
		incrementSequence (Sequence::waitingTransfer);

		manf->transferToSlot (id, slot, option, Sequence::waitingTransfer);

		s_transferToSlotPending [Sequence::waitingTransfer] = TransferPendingInfo (id, slot, option);
		s_slotTransferStatus [slot] = 1;
		Transceivers::slotTransferStatusChanged.emitMessage (true);
	}
	else
		WARNING (true, ("Attempt to transferToSlot on null manf schem"));
}

//----------------------------------------------------------------------
	
void CuiCraftManager::removeFromSlot (int slot, const NetworkId & targetContainer)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	Controller * const controller = findController ();
	
	if (!controller)
		return;

	DEBUG_FATAL (slot >= static_cast<int>(s_slotTransferStatus.size ()), ("bad s_slotTransferStatus size [%d]", s_slotTransferStatus.size ()));

	if (s_slotTransferStatus [slot])
	{
		WARNING (true, ("CuiCraftManager::transferToSlot cannot be called on slot [%d] with transferstatus [%d]", slot, s_slotTransferStatus [slot]));
		return;
	}

	incrementSequence (Sequence::waitingTransfer);

	MessageQueueCraftEmptySlot * const msg = new MessageQueueCraftEmptySlot (slot, targetContainer, Sequence::waitingTransfer);

	//-- enqueue message
	controller->appendMessage (CM_emptySchematicSlotMessage, 0.0f, msg, 
		GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);

	s_removeFromSlotPending [Sequence::waitingTransfer] = TransferPendingInfo (targetContainer, slot, 0);
	s_slotTransferStatus [slot] = -1;
	Transceivers::slotTransferStatusChanged.emitMessage (true);
}

//----------------------------------------------------------------------

void CuiCraftManager::assemble (bool confirmed)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	if (!isReadyToAssemble ())
	{
		CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_assembly_slots.localize ());
		return;
	}

	if (confirmed || !CuiPreferences::getConfirmCrafting ())
	{
		Controller * const controller = findController ();
		
		if (!controller)
			return;
		
		s_listener->cancelRequest (MessageBoxes::requestAssemble );

		s_lastStage = findCurrentStage ();
		
		//-- enqueue message
		char buffer[32];
		_itoa(Sequence::requestAssemble, buffer, 10);
		ClientCommandQueue::enqueueCommand(NEXT_CRAFTING_STAGE_COMMAND, NetworkId::cms_invalid, Unicode::narrowToWide(buffer));
		
		MessageBoxes::requestAssemble = CuiMessageBox::createMessageBox (CuiStringIdsCraft::wait_assemble.localize ());
		MessageBoxes::requestAssemble->setRunner (true);
		MessageBoxes::requestAssemble->connectToMessages (*s_listener);

		Transceivers::assembling.emitMessage (0);

	} //lint !e429 //custodial msg ptr

	else
	{
		MessageBoxes::confirmAssemble = CuiMessageBox::createYesNoBox (CuiStringIdsCraft::confirm_assemble.localize ());
		MessageBoxes::confirmAssemble->connectToMessages (*s_listener);
	}
}

//----------------------------------------------------------------------

void CuiCraftManager::complete (bool confirmed, bool practice)
{
	s_isPractice = practice;

	DEBUG_FATAL (!s_installed, ("not installed"));

	ManufactureSchematicObject * const manf_schem = getManufactureSchematic ();

	if (!manf_schem)
	{
		WARNING (true, ("no schematic"));
		return;
	}


	if (confirmed || !CuiPreferences::getConfirmCrafting ())
	{
		Controller * const controller = findController ();
		
		if (!controller)
			return;
		
		s_listener->cancelRequest (MessageBoxes::requestAssemble );
	
		uint32 type = 0;

		if (s_finalState == FS_proto)
			type = CREATE_PROTOTYPE_COMMAND;
		else if (s_finalState == FS_schem)
			type = CREATE_MANF_SCHEMATIC_COMMAND;
		else
		{
			WARNING (true, ("bad type"));
			return;
		}
					
		const Crafting::CraftingStage stage = findCurrentStage ();

		if (stage == Crafting::CS_customize && !s_customizationSet)
		{
			// send customization data to the server
			{
				MessageQueueCraftCustomization * const msg = new MessageQueueCraftCustomization();
				msg->setName(manf_schem->getObjectName());

				const int custCount = manf_schem->getCustomizationCount ();
				for (int i = 0; i < custCount; ++i)
				{
					int  value = 0;

					//@todo: store the default value on the manf schem
					if (manf_schem->getCustomizationValue (i, value))
						msg->addCustomization(i, value);
				}

				const int selectedAppearance = manf_schem->getCustomizationAppearance ();
				if (selectedAppearance >= 0)
					msg->setAppearance (selectedAppearance);

				msg->setItemCount (manf_schem->getCount());
				controller->appendMessage (CM_setCustomizationData, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);
			}

			MessageBoxes::requestSendCustomizationData = CuiMessageBox::createMessageBox (CuiStringIdsCraft::wait_send_customization_data.localize ());
			MessageBoxes::requestSendCustomizationData->connectToMessages (*s_listener);
			return;
		}

		if (stage != Crafting::CS_finish)
		{
			CuiCraftManager::transitionToFinished ();
			return;
		}

		// tell the server to create the prototype or schematic
		{
			//-- enqueue message
			char buffer[32];
			_itoa(Sequence::requestComplete, buffer, 10);
			Unicode::String params = Unicode::narrowToWide(buffer);
			if (s_finalState == FS_proto)
			{
				params.push_back (' ');
				if (practice)
					params.push_back ('0');
				else
					params.push_back ('1');
			}

			ClientCommandQueue::enqueueCommand(type, NetworkId::cms_invalid, params);
		}
		
		if (s_finalState == FS_proto)
			MessageBoxes::requestComplete = CuiMessageBox::createMessageBox (CuiStringIdsCraft::wait_complete_prototype.localize ());
		else
			MessageBoxes::requestComplete = CuiMessageBox::createMessageBox (CuiStringIdsCraft::wait_complete_manf_schem.localize ());

		MessageBoxes::requestComplete->setRunner (true);
		MessageBoxes::requestComplete->connectToMessages (*s_listener);
	}

	else
	{
		MessageBoxes::confirmComplete = CuiMessageBox::createYesNoBox (CuiStringIdsCraft::confirm_complete.localize ());
		MessageBoxes::confirmComplete->connectToMessages (*s_listener);
	}
}

//----------------------------------------------------------------------

void CuiCraftManager::transitionToFinished ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	Controller * const controller = findController ();
		
	if (!controller)
		return;
	
	const Crafting::CraftingStage stage = findCurrentStage ();
	
	if (stage != Crafting::CS_customize)
	{
		WARNING (true, ("Can't finish from here."));
		return;
	}

	s_lastStage = stage;

	s_listener->cancelRequest (MessageBoxes::requestFinish );
		
	//-- enqueue message
	char buffer[32];
	_itoa(Sequence::requestFinish, buffer, 10);
	ClientCommandQueue::enqueueCommand(NEXT_CRAFTING_STAGE_COMMAND, NetworkId::cms_invalid, Unicode::narrowToWide(buffer));
	
	MessageBoxes::requestFinish = CuiMessageBox::createMessageBox (CuiStringIdsCraft::wait_finish.localize ());
	MessageBoxes::requestFinish->setRunner (true);
	MessageBoxes::requestFinish->connectToMessages (*s_listener);
}

//----------------------------------------------------------------------

void CuiCraftManager::customize ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	const Crafting::CraftingStage stage = findCurrentStage ();

	if (stage == Crafting::CS_customize)
	{
		Transceivers::customize.emitMessage (0);
		return;
	}

	if (stage != Crafting::CS_experiment)
	{
		WARNING (true, ("bad stage"));
		return;
	}

	s_lastStage = stage;

	Controller * const controller = findController ();
	
	if (!controller)
		return;
	
	s_listener->cancelRequest (MessageBoxes::requestCustomize );
	
	//-- enqueue message
	char buffer[32];
	_itoa(Sequence::requestCustomize, buffer, 10);
	ClientCommandQueue::enqueueCommand(NEXT_CRAFTING_STAGE_COMMAND, NetworkId::cms_invalid, Unicode::narrowToWide(buffer));
	
	MessageBoxes::requestCustomize = CuiMessageBox::createMessageBox (CuiStringIdsCraft::wait_customize.localize ());
	MessageBoxes::requestCustomize->setRunner (true);
	MessageBoxes::requestCustomize->connectToMessages (*s_listener);
}

//-----------------------------------------------------------------

void CuiCraftManager::experiment (const IntPairVector & v, int corelevel)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	Controller * const controller = findController ();

	if (!controller)
		return;

	s_listener->cancelRequest (MessageBoxes::requestExperiment);

	if (findCurrentStage () != Crafting::CS_experiment)
	{
		WARNING (true, ("bad stage"));
		return;
	}

	// @todo: set sequence id for constructor
	MessageQueueCraftExperiment * const msg = new MessageQueueCraftExperiment (Sequence::requestExperiment);

	for (IntPairVector::const_iterator it = v.begin (); it != v.end (); ++it)
	{
		if ((*it).second > 0)
			msg->addExperiment ((*it).first, (*it).second);
	}
	
	msg->setCoreLevel(corelevel);

	//-- enqueue message
	controller->appendMessage (CM_experimentMessage, 0.0f, msg, 
		GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);
		
	MessageBoxes::requestExperiment = CuiMessageBox::createMessageBox (CuiStringIdsCraft::wait_experiment.localize ());
	MessageBoxes::requestExperiment->setRunner (true);
	MessageBoxes::requestExperiment->connectToMessages (*s_listener);
}

//----------------------------------------------------------------------

void CuiCraftManager::startCrafting (const NetworkId & toolId)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	//-- already crafting
	if (Ids::craftingTool.isValid ())
	{
		TangibleObject const * const oldTool = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(Ids::craftingTool));
		if (oldTool && oldTool->getClientSynchronizedUi())
		{
			CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_already_crafting.localize ());
			return;
		}
	}

	TangibleObject * const tool = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(toolId));

	if (!tool)
	{
		WARNING (true, ("CuiCraftManager::startCrafting no crafting tool"));
		return;
	}

	const ClientObject * const hopperOutputContent = findHopperOutputContent (*tool);

	if (hopperOutputContent)
	{
		CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_hopper_output_nonempty.localize ());
		return;
	}

	Controller * const controller = findController ();
	
	if (!controller)
		return;
	
	s_listener->cancelRequest (MessageBoxes::requestStartCrafting );

	s_finalState = FS_none;
	Ids::craftingToolPending = toolId;
	Ids::craftingTool        = CachedNetworkId::cms_invalid;
	s_schematicVector.clear ();
	s_currentDraftSchematicCrc = std::make_pair(0, 0);
	s_slotVector.clear ();
	s_transferToSlotPending.clear ();
	s_removeFromSlotPending.clear ();
	s_slotTransferStatus.clear    ();

	s_customizationSet = false;

	if (Game::getSinglePlayer ())
	{
		CuiCraftManagerSimulator::startCrafting ();
	}
	else
	{
		tool->startCrafting();

		//-- enqueue message
		ClientCommandQueue::enqueueCommand(START_CRAFTING_COMMAND, toolId, Unicode::emptyString);
		
		MessageBoxes::requestStartCrafting = CuiMessageBox::createMessageBox (CuiStringIdsCraft::wait_start_crafting.localize ());
		MessageBoxes::requestStartCrafting->setRunner (true);
		MessageBoxes::requestStartCrafting->connectToMessages (*s_listener);

		CuiInventoryManager::closeAllInstancesOfItem (*tool, CuiCraftManager::getHopperOutputSlotName ());

	} //lint !e429 //custodial msg ptr
}

//----------------------------------------------------------------------

void CuiCraftManager::stopCrafting (bool confirmed, bool emitEndedMessage)
{
	if (!isCrafting () && !Ids::craftingTool.isValid ())
		return;

	if (confirmed)
	{
		TangibleObject * const tool = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(Ids::craftingToolPending));

		s_currentDraftSchematicCrc = std::make_pair(0, 0);
		s_finalState = FS_none;
		Ids::craftingToolPending = CachedNetworkId::cms_invalid;
		Ids::craftingTool        = CachedNetworkId::cms_invalid;
		s_schematicVector.clear ();
		s_slotVector.clear ();
		s_customizationSet = false;
		s_transferToSlotPending.clear ();
		s_removeFromSlotPending.clear ();
		s_slotTransferStatus.clear    ();

		if (Game::getSinglePlayer ())
		{
			CuiCraftManagerSimulator::stopCrafting ();
		}
		else
		{
			if (tool != NULL)
				tool->stopCrafting();

			Controller * const controller = findController ();
			if (!controller)
				return;
			
			ClientCommandQueue::enqueueCommand(CANCEL_CRAFTING_SESSION_COMMAND, NetworkId::cms_invalid, Unicode::emptyString);
		}

		if (emitEndedMessage)
			Transceivers::sessionEnded.emitMessage (true);
	}
	else
	{
		MessageBoxes::confirmCancel = CuiMessageBox::createYesNoBox (CuiStringIdsCraft::confirm_cancel.localize ());
		MessageBoxes::confirmCancel->connectToMessages (*s_listener);
	}
}

//----------------------------------------------------------------------

void CuiCraftManager::restartCraftingBROKEN (bool confirmed)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	if (confirmed || !CuiPreferences::getConfirmCrafting ())
	{
		if (Game::getSinglePlayer ())
		{
		}
		else
		{
			s_listener->cancelRequest (MessageBoxes::requestRestartCrafting);

			Controller * const controller = findController ();
			
			if (!controller)
				return;
			
			//-- enqueue message
			char buffer[32];
			_itoa(Sequence::requestRestartCrafting, buffer, 10);
			ClientCommandQueue::enqueueCommand(RESTART_CRAFTING_SESSION_COMMAND, NetworkId::cms_invalid, Unicode::narrowToWide(buffer));

			MessageBoxes::requestRestartCrafting = CuiMessageBox::createMessageBox (CuiStringIdsCraft::wait_restart_crafting.localize ());
			MessageBoxes::requestRestartCrafting->setRunner (true);
			MessageBoxes::requestRestartCrafting->connectToMessages (*s_listener);
		}
	}
}

//----------------------------------------------------------------------

void CuiCraftManager::confirmDestroy ()
{

}

//----------------------------------------------------------------------

ClientObject * CuiCraftManager::getCraftingTool()
{
	return safe_cast<ClientObject *>(Ids::craftingTool.getObject ());
}

//----------------------------------------------------------------------

ClientObject * CuiCraftManager::getCraftingStation()
{
	ClientObject * station = NULL;
	
	const CreatureObject * const player     = dynamic_cast<CreatureObject*>(Game::getClientPlayer ());
	
	if (player)
	{
		const PlayerObject * const playerObject = player->getPlayerObject ();
		
		if (playerObject)
			station = safe_cast<ClientObject *>(playerObject->getCraftingStation().getObject());
	}

	return station;
}

//----------------------------------------------------------------------

ClientObject * CuiCraftManager::getCraftingStationHopperInput  ()
{
	const ClientObject * station = getCraftingStation();
	
	return getCraftingStationHopperInput(station);
}

//----------------------------------------------------------------------

ClientObject * CuiCraftManager::getCraftingStationHopperInput (const ClientObject * station)
{
	static const SlotId inputHopperId(SlotIdManager::findSlotId(CrcLowerString(Slots::ingredient_hopper)));

	ClientObject * hopper = NULL;
	if (station != NULL)
	{
		const SlottedContainer * container = ContainerInterface::getSlottedContainer(*station);
		if (container != NULL)
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			Object* tmpHopper = (container->getObjectInSlot(inputHopperId, tmp)).getObject();
			if (tmpHopper && tmp == Container::CEC_Success)
			{
				hopper = tmpHopper->asClientObject();
			}
		}
	}

	return hopper;
}

//----------------------------------------------------------------------

ClientObject * CuiCraftManager::findHopperOutputContent (ClientObject & tool)
{
	ClientObject * const hopper = ContainerInterface::getObjectInSlot (tool, Slots::hopper_output.c_str ());
	return hopper;
}


//----------------------------------------------------------------------

ManufactureSchematicObject *     CuiCraftManager::getManufactureSchematic     ()
{
	if (Game::getSinglePlayer ())
	{
		CuiCraftManagerSimulator::onGetManufacturingSchematic (Ids::manufactureSchematic);
	}

	return dynamic_cast<ManufactureSchematicObject *>(Ids::manufactureSchematic.getObject ());
}

//-----------------------------------------------------------------

ClientObject * CuiCraftManager::getSchematicPrototype ()
{
	if (Game::getSinglePlayer ())
	{
		CuiCraftManagerSimulator::onGetSchematicPrototype (Ids::schematicPrototype);
	}

	return safe_cast<ClientObject *>(Ids::schematicPrototype.getObject ());
}

//----------------------------------------------------------------------

const CuiCraftManager::SlotVector & CuiCraftManager::getSlots ()
{
	if (Game::getSinglePlayer ())
	{
		CuiCraftManagerSimulator::onGetSlots (s_slotVector);
	}

	return s_slotVector;
}

//----------------------------------------------------------------------

const CuiCraftManager::Slot * CuiCraftManager::getSlot (int index)
{
	if (index < 0 || index >= static_cast<int>(s_slotVector.size ()))
		return 0;

	return &s_slotVector [static_cast<size_t>(index)];
}

//----------------------------------------------------------------------

const CuiCraftManager::SchematicVector & CuiCraftManager::getSchematics ()
{
	if (Game::getSinglePlayer ())
	{
		CuiCraftManagerSimulator::onGetSchematics (s_schematicVector);
	}

	return s_schematicVector;
}

//----------------------------------------------------------------------

CuiCraftManager::FinalState CuiCraftManager::getFinalState ()
{
	return s_finalState;
}

//----------------------------------------------------------------------

void CuiCraftManager::setFinalState (FinalState fs)
{
	s_finalState = fs;
}

//----------------------------------------------------------------------

Crafting::CraftingStage CuiCraftManager::findCurrentStage ()
{
	const CreatureObject * const player = dynamic_cast<CreatureObject*>(Game::getClientPlayer ());
	return player ? player->getCraftingStage () : Crafting::CS_none;
}

//-----------------------------------------------------------------

int CuiCraftManager::findCraftingLevel ()
{
	int level = 0;

	const CreatureObject * const player = dynamic_cast<CreatureObject*>(Game::getClientPlayer ());
	if (player != NULL && player->getPlayerObject() != NULL)
	{
		level = player->getPlayerObject()->getCraftingLevel();
	}

	return level;
}

//-----------------------------------------------------------------

int CuiCraftManager::findPlayerCurrentExperimentPoints (bool randomizeSinglePlayer)
{
	if (Game::getSinglePlayer ())
	{
		return CuiCraftManagerSimulator::onFindPlayerCurrentExperimentPoints (randomizeSinglePlayer);
	}
	else
	{
		const CreatureObject * const player = dynamic_cast<CreatureObject*>(Game::getClientPlayer ());
		return player ? player->getExperimentPoints () : 0;
	}
}

//----------------------------------------------------------------------

bool CuiCraftManager::findManufactureSchematicAttributeValues (int index, StringId & id, float & min, float & max, float & cur, float & resourceMax)
{
	const ManufactureSchematicObject * const manf_schem = getManufactureSchematic ();
	if (!manf_schem)
		return false;

	return manf_schem->getExperimentAttribData(index, id, min, max, resourceMax, cur);
}

//----------------------------------------------------------------------

bool CuiCraftManager::getValidSlotOptions (const ClientObject & objectToInsert, int slotIndex, IntVector & validOptions)
{
	if (slotIndex < 0 || slotIndex >= static_cast<int>(s_slotVector.size ()))
		return false;

	const size_t oldVectorSize = validOptions.size ();

	const Slot & slot = s_slotVector [static_cast<size_t>(slotIndex)];

	int index = 0;
	
	for (Slot::OptionVector::const_iterator it = slot.options.begin (); it != slot.options.end (); ++it, ++index)
	{
		const Slot::Option & option = *it;
		const ResourceContainerObject * const resourceContainer = dynamic_cast<const ResourceContainerObject *>(&objectToInsert);

		if (option.type == Crafting::IT_resourceType)
		{
			if (resourceContainer)
			{
				if (resourceContainer->getResourceName () == option.ingredient)
					validOptions.push_back (index);
			}
		}
		else if (option.type == Crafting::IT_resourceClass)
		{
			if (resourceContainer)
			{
				if (resourceContainer->derivesFromResource (option.ingredient))
					validOptions.push_back (index);
			}
		}
		else if (option.type == Crafting::IT_template || option.type == Crafting::IT_templateGeneric ||
			option.type == Crafting::IT_schematic || option.type == Crafting::IT_schematicGeneric)
		{
			if (option.type != Crafting::IT_templateGeneric)
			{
				// only crafted item can be used
				const TangibleObject * const tangibleObject = objectToInsert.asTangibleObject();
				if (tangibleObject && !tangibleObject->isCrafted())
					continue;
			}

			const ObjectTemplate * objectTemplateToInsert = 0;
			const FactoryObject * const factory = dynamic_cast<const FactoryObject *>(&objectToInsert);
			if (factory != NULL)
			{
				objectTemplateToInsert = factory->getContainedObjectTemplate ();

				if (objectTemplateToInsert == NULL)
				{
					WARNING (true, ("factory object [%s] has null template", Unicode::wideToNarrow (objectToInsert.getLocalizedName ()).c_str ()));
					return false;
				}
			}
			else
				objectTemplateToInsert = objectToInsert.getObjectTemplate ();

			if (!objectTemplateToInsert)
			{
				WARNING (true, ("crafting ingredient [%s] has null template", Unicode::wideToNarrow (objectToInsert.getLocalizedName ()).c_str ()));
				return false;
			}

			const std::string & requiredIngredientName = Unicode::wideToNarrow (option.ingredient);

			//@todo: Crafting::IT_template should not check derivation
			const bool slotOk = objectTemplateToInsert->derivesFrom (requiredIngredientName);

			if (slotOk)
			{
				validOptions.push_back (index);
			}
		}
		else if (option.type == Crafting::IT_item)
		{
			if (objectToInsert.getLocalizedName () == option.ingredient)
				validOptions.push_back (index);
		}
	}

	return (oldVectorSize < validOptions.size ());
}

//----------------------------------------------------------------------

bool CuiCraftManager::objectDerivesFromTemplate (const Object & obj, const std::string & potentialAncestorName)
{
	const ObjectTemplate * const testTemplate = obj.getObjectTemplate ();
	if (testTemplate)
		return testTemplate->derivesFrom (potentialAncestorName);

	return false;
}

//----------------------------------------------------------------------

bool CuiCraftManager::isReadyToAssemble ()
{
	if (findCurrentStage () != Crafting::CS_assembly)
		return false;

	const ManufactureSchematicObject * const manf_schem = getManufactureSchematic ();
	if (!manf_schem)
		return false;

	for (SlotVector::const_iterator it = s_slotVector.begin (); it != s_slotVector.end (); ++it)
	{
		const Slot & slot = *it;
		if (slot.optional)
			continue;

		Crafting::IngredientSlot ingredient;
		if (!manf_schem->getIngredient(slot.name, ingredient) || ingredient.ingredients.empty ())
			return false;

		const MessageQueueDraftSlotsDataOption & option = slot.options [ingredient.draftSlotOption];

		if (option.type == Crafting::IT_template || option.type == Crafting::IT_templateGeneric ||
			option.type == Crafting::IT_schematic || option.type == Crafting::IT_schematicGeneric)
		{
			if (option.amountNeeded != static_cast<int>(ingredient.ingredients.size()))
				return false;
		}
		else if (option.type == Crafting::IT_resourceType || option.type == Crafting::IT_resourceClass)
		{
			int count = 0;
			for (Crafting::Ingredients::const_iterator iter = 
				ingredient.ingredients.begin(); iter != ingredient.ingredients.end(); 
				++iter)
			{
				count += (*iter)->count;
			}
			if (option.amountNeeded != count)
				return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

bool CuiCraftManager::canManufacture()
{
	return s_canManufacture && !s_isBioLinked;
}

//----------------------------------------------------------------------

bool CuiCraftManager::canExperiment()
{
	bool hasAttributes = false;
	ManufactureSchematicObject * const manf_schem = getManufactureSchematic ();
	if (manf_schem)
		hasAttributes = manf_schem->getExperimentAttribCount () != 0;

	return findCurrentStage() == Crafting::CS_experiment && hasAttributes;
}

//----------------------------------------------------------------------

const std::string & CuiCraftManager::getHopperOutputSlotName ()
{
	return Slots::hopper_output;
}

//----------------------------------------------------------------------

void CuiCraftManager::reportAssemblyResult (Crafting::CraftingResult craftingResult)
{
	std::string resultName;
	Crafting::getCraftingResultName (craftingResult, resultName);

	resultName += "_ass";
	const StringId sid (CuiStringTables::craft_results, resultName);

	s_localizedAssemblyResult = sid.localize();

	if (craftingResult == Crafting::CR_criticalFailure)
	{		
		CuiMessageBox::createInfoBox (s_localizedAssemblyResult);
		Transceivers::doneAssembling.emitMessage (0);
	}
}

//----------------------------------------------------------------------

void CuiCraftManager::reportExperimentationResult (Crafting::CraftingResult craftingResult)
{
	std::string resultName;
	Crafting::getCraftingResultName (craftingResult, resultName);

	resultName += "_exp";
	const StringId sid (CuiStringTables::craft_results, resultName);

	CuiSystemMessageManager::sendFakeSystemMessage (sid.localize ());
}

//----------------------------------------------------------------------

const std::pair<uint32, uint32> & CuiCraftManager::getCurrentDraftSchematicCrc()
{
	return s_currentDraftSchematicCrc;
}

//----------------------------------------------------------------------

const Unicode::String & CuiCraftManager::getLocalizedAssemblyResult()
{
	return s_localizedAssemblyResult;
}

//----------------------------------------------------------------------

const CuiCraftManager::IntVector & CuiCraftManager::getSlotTransferStatus                   ()
{
	return s_slotTransferStatus;
}

//----------------------------------------------------------------------

int CuiCraftManager::getSlotTransferStatus                   (int slot)
{
	if (slot < 0 || slot >= static_cast<int>(s_slotTransferStatus.size ()))
		return 0;

	return s_slotTransferStatus [slot];
}

//----------------------------------------------------------------------

bool CuiCraftManager::canStartCrafting(NetworkId const & station)
{
	DEBUG_FATAL(!s_installed, ("not installed"));
	
	if (Ids::craftingTool.isValid())
	{
		TangibleObject const * const oldTool = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(Ids::craftingTool));
		if (oldTool && oldTool->getClientSynchronizedUi())
			return false;
	}
	
	TangibleObject * const tool = dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(station));
	if (!tool)
		return false;

	if (tool->getGameObjectType() != SharedObjectTemplate::GOT_tool_crafting) 
		return false;
	
	ClientObject const * const hopperOutputContent = findHopperOutputContent(*tool);
	if (hopperOutputContent)
		return false;
	
	Controller * const controller = findController();
	if (!controller)
		return false;
	
	return true;
}

//----------------------------------------------------------------------

void CuiCraftManager::setCraftingBioLinkedItem(bool isBioLinked)
{
	s_isBioLinked = isBioLinked;
}

//----------------------------------------------------------------------
// Check to see if the craft manager is currently crafting this object or 
// if the object is the crafting tool used.

bool CuiCraftManager::isCrafting(NetworkId const & object)
{
	return  object == Ids::manufactureSchematic || object == Ids::schematicPrototype ||
			object == Ids::craftingTool || object == Ids::craftingToolPending;
}

//----------------------------------------------------------------------

bool CuiCraftManager::isCraftingStartCommand(uint32 commandHash)
{
	if(commandHash == START_CRAFTING_COMMAND)
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

void CuiCraftManager::abortCraftingStart()
{
	s_listener->cancelRequest (MessageBoxes::requestStartCrafting );
}

//======================================================================
