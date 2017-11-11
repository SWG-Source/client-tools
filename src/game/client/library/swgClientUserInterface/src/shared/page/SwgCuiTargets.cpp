//======================================================================
//
// SwgCuiTargets.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiTargets.h"

#include "UIButton.h"
#include "UIData.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiAction.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedGame/SharedInstallationObjectTemplate.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "swgClientUserInterface/SwgCuiActions.h"
#include "swgClientUserInterface/SwgCuiStatusFactory.h"

//======================================================================

class SwgCuiTargets::SwgCuiTargetsAction : public CuiAction
{
public:
	
	explicit SwgCuiTargetsAction (SwgCuiTargets & targets) : CuiAction (), m_targets (targets) {}
	
	bool  performAction (const std::string & id, const Unicode::String &) const
	{
		UNREF (id);
		
		return false;
	}
	
private:
	SwgCuiTargets & m_targets;
	SwgCuiTargetsAction ();
	SwgCuiTargetsAction const & operator=(SwgCuiTargetsAction const & rhs);
};

//----------------------------------------------------------------------
SwgCuiTargets::SwgCuiTargets (UIPage & page) :
CuiMediator                ("SwgCuiTargets", page),
UIEventCallback            (),
m_action                   (0),
m_mfdStatus                (0),
m_buttonCollapse           (0),
m_buttonExpand             (0),
m_pageToggle               (0),
m_sceneType(Game::getHudSceneType())
{
	//----------------------------------------------------------------------
	//-- setup the icon

	CuiWorkspaceIcon * const icon = new CuiWorkspaceIcon (this);
	icon->SetName ("TargetsIcon");
	icon->SetSize (UISize (32, 32));
	icon->SetBackgroundColor (UIColor (0, 0, 0, 50));
	icon->SetLocation (64, 0);

	setIcon (icon);

	getCodeDataObject (TUIButton, m_buttonCollapse, "buttoncollapse");
	getCodeDataObject (TUIButton, m_buttonExpand,   "buttonexpand");
	getCodeDataObject (TUIPage,   m_pageToggle,     "pagetoggle");

	UIPage * statusPage = 0;
	getCodeDataObject (TUIPage, statusPage, "pagestatus");

	m_mfdStatus = SwgCuiStatusFactory::createStatusPage(static_cast<Game::SceneType>(m_sceneType), *statusPage);
	m_mfdStatus->fetch ();

	m_mfdStatus->setShowRange(m_mfdStatus->getTarget().getObject() != Game::getPlayer());

	m_action = new SwgCuiTargetsAction (*this);

	registerMediatorObject (*m_buttonCollapse, true);
	registerMediatorObject (*m_buttonExpand,   true);
	
	m_mfdStatus->setPageToLock(&getPage());
}  //lint !e429 // custodial pointer that is owned by the interface.

//----------------------------------------------------------------------

SwgCuiTargets::~SwgCuiTargets ()
{
	CuiActionManager::removeAction (m_action);
	delete m_action;
	m_action = 0;

	m_mfdStatus->release ();
	m_mfdStatus = 0;

	m_buttonCollapse = 0;
	m_buttonExpand   = 0;
}

//-----------------------------------------------------------------

void SwgCuiTargets::performActivate()
{
	m_mfdStatus->activate ();
	setIsUpdating (true);
	update (0.0f);
}

//-----------------------------------------------------------------

void SwgCuiTargets::performDeactivate()
{
	m_mfdStatus->deactivate ();
	setIsUpdating (false);
}

//----------------------------------------------------------------------

void SwgCuiTargets::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	const CreatureObject * const player = Game::getPlayerCreature ();
	
	if (!player)
		return;

	const CachedNetworkId & target = player->getLookAtTarget ();
	const CachedNetworkId & oldId = m_mfdStatus->getTarget ();

	CachedNetworkId combatTarget;
	const bool isInCombat = CuiCombatManager::isInCombat (player, combatTarget);

	if (isInCombat)
		m_pageToggle->SetEnabled (true);
	else
		m_pageToggle->SetEnabled (false);

	if (oldId != target && target.getObject () && target.getObject ()->asClientObject () && target.getObject ()->asClientObject ()->asTangibleObject ())
	{
		m_mfdStatus->setTarget (target.getObject ()->asClientObject ()->asTangibleObject ());
	}

	if (target != CachedNetworkId::cms_invalid)
	{
		if (!getPage ().IsVisible ())
			getPage ().SetVisible (true);
		
		TangibleObject * tangible = 0;
		CreatureObject * creature = 0;

		Object * const targetObject = target.getObject ();
		
		if (targetObject)
		{
			if (targetObject->getObjectType () == SharedCreatureObjectTemplate::SharedCreatureObjectTemplate_tag)
			{
				creature = safe_cast<CreatureObject *>(targetObject);
				tangible = creature;
			}
			else if (targetObject->getObjectType () == SharedCreatureObjectTemplate::SharedTangibleObjectTemplate_tag ||
				targetObject->getObjectType () == SharedInstallationObjectTemplate::SharedInstallationObjectTemplate_tag)
			{
				tangible = safe_cast<TangibleObject *>(targetObject);
			}
			else
				tangible = dynamic_cast<TangibleObject *>(targetObject);
		}

		if (tangible && !tangible->isInvulnerable () && (creature || tangible->getMaxHitPoints () != 0) && (!creature || !creature->isDead ()))
		{
			m_mfdStatus->setDisplayStates (true, true, true);
			m_mfdStatus->update (0.0f);
		}
		else
			m_mfdStatus->setDisplayStates (false, false, false);
	}
	else
	{
		if (getPage ().IsVisible ())
			getPage ().SetVisible (false);
	}
}

//----------------------------------------------------------------------

void SwgCuiTargets::OnButtonPressed (UIWidget *context)
{
	if (context == m_buttonCollapse)
	{
		IGNORE_RETURN(CuiActionManager::performAction (SwgCuiActions::combatQueueCollapse, Unicode::emptyString));
		m_buttonCollapse->SetVisible (false);
		m_buttonExpand->SetVisible   (true);
	}
	else if (context == m_buttonExpand)
	{
		IGNORE_RETURN(CuiActionManager::performAction (SwgCuiActions::combatQueueExpand, Unicode::emptyString));
		m_buttonExpand->SetVisible   (false);
		m_buttonCollapse->SetVisible (true);
	}
}

//======================================================================

