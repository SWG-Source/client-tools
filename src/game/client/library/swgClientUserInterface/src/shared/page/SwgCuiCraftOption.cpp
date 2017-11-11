//======================================================================
//
// SwgCuiCraftExperiment.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCraftOption.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIPage.h"
#include "clientGame/ClientObject.h"
#include "clientGame/DraftSchematicInfo.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientGame/ManufactureSchematicObject.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "clientUserInterface/CuiInventoryManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsCraft.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/VolumeContainer.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiCraftExperiment.h"

//======================================================================

SwgCuiCraftOption::SwgCuiCraftOption (UIPage & page) :
CuiMediator        ("SwgCuiCraftOption", page),
UIEventCallback    (),
m_buttonSchematic  (0),
m_buttonPrototype  (0),
m_buttonExperiment (0),
m_transition       (false),
m_callback         (new MessageDispatch::Callback),
m_sessionEnded     (false) 
{
	getCodeDataObject (TUIButton,  m_buttonSchematic,  "ButtonSchematic");
	getCodeDataObject (TUIButton,  m_buttonPrototype,  "buttonPrototype");
	getCodeDataObject (TUIButton,  m_buttonExperiment, "buttonExperiment");

	setState    (MS_closeable);

	registerMediatorObject (*m_buttonSchematic,  true);
	registerMediatorObject (*m_buttonPrototype,  true);
	registerMediatorObject (*m_buttonExperiment, true);
}

//----------------------------------------------------------------------

SwgCuiCraftOption::~SwgCuiCraftOption ()
{
	m_buttonSchematic = 0;
	m_buttonPrototype = 0;
	m_buttonExperiment = 0;

	delete m_callback;
	m_callback = 0;
}

//----------------------------------------------------------------------

void SwgCuiCraftOption::performActivate   ()
{
	m_sessionEnded = false;

	if (!CuiCraftManager::isCrafting ())
	{
		onSessionEnded (true);
		return;
	}

	m_buttonPrototype->SetEnabled(true);

	boolean canManufacture = CuiCraftManager::canManufacture();
	m_buttonSchematic->SetEnabled(canManufacture);
	m_buttonSchematic->SetVisible(canManufacture);

	CuiManager::requestPointer    (true);

	m_buttonExperiment->SetVisible (CuiCraftManager::canExperiment());


	m_transition = false;

	m_callback->connect (*this, &SwgCuiCraftOption::onCustomize,         static_cast<CuiCraftManager::Messages::Customize*>     (0));
	m_callback->connect (*this, &SwgCuiCraftOption::onSessionEnded,      static_cast<CuiCraftManager::Messages::SessionEnded *>     (0));

	setIsUpdating (true);
}

//----------------------------------------------------------------------

void SwgCuiCraftOption::performDeactivate ()
{
	CuiManager::requestPointer    (false);

	m_callback->disconnect (*this, &SwgCuiCraftOption::onCustomize,         static_cast<CuiCraftManager::Messages::Customize*>     (0));
	m_callback->disconnect (*this, &SwgCuiCraftOption::onSessionEnded,      static_cast<CuiCraftManager::Messages::SessionEnded *>     (0));

	setIsUpdating (false);
}

//----------------------------------------------------------------------

void SwgCuiCraftOption::OnButtonPressed( UIWidget *context )
{
	if (context == m_buttonSchematic)
	{
		const ClientObject * const datapad            = CuiInventoryManager::getPlayerDatapad ();
		const VolumeContainer * const volumeContainer = datapad ? datapad->getVolumeContainerProperty () : 0;
		const DraftSchematicInfo * const dsi          = DraftSchematicManager::findDraftSchematic (CuiCraftManager::getCurrentDraftSchematicCrc ());
		
		if (volumeContainer && dsi)
		{
			const int manfVolume      = dsi->getManufactureSchematicVolume ();
			const int volumeAvailable = volumeContainer->getTotalVolume () - volumeContainer->getCurrentVolume ();
			
			if (manfVolume > volumeAvailable)
			{
				Unicode::String result;
				CuiStringVariablesManager::process (CuiStringIdsCraft::err_datapad_full_prose, Unicode::emptyString, Unicode::emptyString, Unicode::emptyString, manfVolume, 0.0f, result);
				CuiMessageBox::createInfoBox (result);
				return;
			}
		}

		CuiCraftManager::setFinalState (CuiCraftManager::FS_schem);
		CuiCraftManager::customize ();
		m_buttonSchematic->SetEnabled(false);
	}
	else if (context == m_buttonPrototype)
	{
		CuiCraftManager::setFinalState (CuiCraftManager::FS_proto);
		CuiCraftManager::customize ();
		m_buttonPrototype->SetEnabled(false);
	}
	else if (context == m_buttonExperiment)
	{
		const Crafting::CraftingStage stage = CuiCraftManager::findCurrentStage ();

		if (stage != Crafting::CS_experiment)
		{
			WARNING (true, ("You can't do that."));
			return;
		}

		CuiCraftManager::setFinalState (CuiCraftManager::FS_none);
		deactivate ();
		SwgCuiCraftExperiment* experimentPage = dynamic_cast<SwgCuiCraftExperiment*>(CuiMediatorFactory::getInWorkspace(CuiMediatorTypes::WS_CraftExperiment));
		if(experimentPage)
			experimentPage->setInitializePage(true);
		CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_CraftExperiment);
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftOption::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if (m_transition)
	{
		m_transition = false;
		deactivate ();

		const Crafting::CraftingStage stage = CuiCraftManager::findCurrentStage ();

		if (stage == Crafting::CS_customize)
			CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_CraftCustomize);
		else
			WARNING (true, ("I don't know what to do"));
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftOption::onCustomize (const int & )
{
	m_transition = true;
};

//----------------------------------------------------------------------

void SwgCuiCraftOption::onSessionEnded (const bool &)
{
	m_sessionEnded = true;
	closeNextFrame ();
}

//----------------------------------------------------------------------

bool SwgCuiCraftOption::close ()
{
	CuiCraftManager::stopCrafting (m_sessionEnded);
	return true;
}

//======================================================================

