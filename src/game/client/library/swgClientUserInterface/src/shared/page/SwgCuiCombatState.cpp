//======================================================================
//
// SwgCuiCombatState.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCombatState_Action.h"

#include "UIButton.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UIOpacityEffector.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CachedNetworkId.h"
#include "swgSharedUtility/Postures.def"

#include <list>

//======================================================================

SwgCuiCombatState::SwgCuiCombatState (UIPage & page) :
CuiMediator               ("SwgCuiCombatState", page),
UIEventCallback           (),
m_effectorBgFadeFull      (0),
m_effectorBgFadeOut       (0),
m_action                  (0),
m_lastPosture             (0),
m_callback                (new MessageDispatch::Callback)
{
	zeroArrays ();
		
	getCodeDataObject (TUIOpacityEffector, m_effectorBgFadeFull,    "effector.FadeFull");
	getCodeDataObject (TUIOpacityEffector, m_effectorBgFadeOut,     "effector.FadeOut");
	
	{
		getCodeDataObject (TUIButton, m_postureButton [0], "stand");
		getCodeDataObject (TUIButton, m_postureButton [1], "kneel");
		getCodeDataObject (TUIButton, m_postureButton [2], "prone");
		getCodeDataObject (TUIButton, m_postureButton [3], "sit");
	}
	
	m_action = new Action (*this);
}

//----------------------------------------------------------------------

SwgCuiCombatState::~SwgCuiCombatState ()
{
	delete m_callback;
	m_callback = 0;

	delete m_action;
	m_action = 0;
		
	m_effectorBgFadeFull    = 0;
	m_effectorBgFadeOut  = 0;
	
	zeroArrays ();
}

//----------------------------------------------------------------------

void        SwgCuiCombatState::performActivate ()
{	
	updatePosture ();
	
	int i = 0;
	
	for (i = 0; i < NUM_POSTURES; ++i)
	{
		if (m_postureButton [i])
			m_postureButton [i]->AddCallback (this);
		
	}

	m_callback->connect (*this, &SwgCuiCombatState::onPostureChanged,        static_cast<CreatureObject::Messages::VisualPostureChanged *>     (0));
}

//----------------------------------------------------------------------

void        SwgCuiCombatState::performDeactivate ()
{
	int i = 0;
	
	for (i = 0; i < NUM_POSTURES; ++i)
	{
		if (m_postureButton [i])
			m_postureButton [i]->RemoveCallback (this);
	}

	m_callback->disconnect (*this, &SwgCuiCombatState::onPostureChanged,        static_cast<CreatureObject::Messages::VisualPostureChanged *>     (0));
}

//----------------------------------------------------------------------

void SwgCuiCombatState::requestPostureChange (int posture)
{
	DEBUG_FATAL (posture < 0 || posture >= Postures::NumberOfPostures, ("bad posture"));

	CreatureObject * const player = Game::getPlayerCreature ();

	player->requestServerPostureChange (static_cast<uint8>(posture));
}

//----------------------------------------------------------------------

void        SwgCuiCombatState::OnButtonPressed      (UIWidget * )
{
	/*
	if (context == m_postureButton [0])
	{
		CuiMessageQueueManager::executeCommandByName ("CMD_postureStand");
	}
	else if (context == m_postureButton [1])
	{
		CuiMessageQueueManager::executeCommandByName ("CMD_postureKneel");
	}
	else if (context == m_postureButton [2])
	{
		CuiMessageQueueManager::executeCommandByName ("CMD_postureProne");
	}
	else if (context == m_postureButton [3])
	{
		CuiMessageQueueManager::executeCommandByName ("CMD_postureSit");
	}
	*/
}

//----------------------------------------------------------------------

void SwgCuiCombatState::updatePosture ()
{
	CreatureObject * const player = Game::getPlayerCreature ();
	
	if (!player)
		return;

	const int8 posture  = player->getVisualPosture ();
		
	int activeButton = -1;
	
	switch (posture)
	{
	case Postures::Upright:
	case Postures::Blocking:
	case Postures::Sneaking:
		activeButton = 0;
		break;
	case Postures::Crouched:
		activeButton = 1;
		break;
	case Postures::Prone:
		activeButton = 2;
		break;
	case Postures::Sitting:
		activeButton = 3;
		break;
	default:
		break;
	}
	
	for (int i = 0; i < NUM_POSTURES; ++i)
	{	
		m_postureButton [i]->CancelEffector (*m_effectorBgFadeFull);
		m_postureButton [i]->CancelEffector (*m_effectorBgFadeOut);
		
		if (i == activeButton)
			m_postureButton [i]->ExecuteEffector (*m_effectorBgFadeFull);
		else
			m_postureButton [i]->ExecuteEffector (*m_effectorBgFadeOut);
	}
	
	m_lastPosture  = posture;
}

//----------------------------------------------------------------------

void SwgCuiCombatState::zeroArrays ()
{
	int i = 0;
	for (i = 0; i < NUM_POSTURES; ++i)
	{		
		m_postureButton  [i] = 0;
	}
}

//----------------------------------------------------------------------

void SwgCuiCombatState::onPostureChanged (const CreatureObject & creature)
{
	if (static_cast<const Object *>(Game::getPlayer ()) == &creature)
		updatePosture ();
}

//======================================================================
