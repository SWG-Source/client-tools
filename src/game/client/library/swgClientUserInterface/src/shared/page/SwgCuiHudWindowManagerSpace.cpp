//======================================================================
//
// SwgCuiHudWindowManagerSpace.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHudWindowManagerSpace.h"

#include "clientGame/Game.h"
#include "clientGame/TangibleObject.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiWorkspace.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedNetworkMessages/NewbieTutorialEnableHudElement.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedTerrain/TerrainObject.h"
#include "swgClientUserInterface/SwgCuiAllTargetsSpace.h"
#include "swgClientUserInterface/SwgCuiHudSpace.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiShipComponentDetail.h"
#include "swgClientUserInterface/SwgCuiShipReticle.h"
#include "swgClientUserInterface/SwgCuiSpaceFlyOutPage.h"
#include "swgClientUserInterface/SwgCuiSpaceGroup.h"
#include "swgClientUserInterface/SwgCuiSpaceMissileCount.h"
#include "swgClientUserInterface/SwgCuiSpaceRadar.h"
#include "swgClientUserInterface/SwgCuiStatusFactory.h"
#include "swgClientUserInterface/SwgCuiStatusSpace.h"
#include "swgClientUserInterface/SwgCuiTargets.h"

#include "UIData.h"
#include "UIPage.h"

//======================================================================

namespace SwgCuiHudWindowManagerSpaceNamespace
{
	std::string const cms_newbieTutorialEnableHudElementRadar("radar");
	std::string const cms_newbieTutorialEnableHudElementAll("all");
}

using namespace SwgCuiHudWindowManagerSpaceNamespace;

//----------------------------------------------------------------------


SwgCuiHudWindowManagerSpace::SwgCuiHudWindowManagerSpace(SwgCuiHud const & hud, CuiWorkspace & workspace) :
SwgCuiHudWindowManager(hud, workspace),
m_spaceRadarMediator(NULL),
m_spaceFlyOutPage(NULL),
m_spaceMissileCount(NULL),
m_shipReticle(NULL),
m_targetsMediator(NULL)
{
	UIPage * mediatorPage = NULL;

	{
		hud.getCodeDataObject(TUIPage, mediatorPage, "RadarPage");
		mediatorPage->SetEnabled(true);
		mediatorPage->SetEnabled(false);
		m_spaceRadarMediator = new SwgCuiSpaceRadar(*mediatorPage);
		m_spaceRadarMediator->setSettingsAutoSizeLocation(true, true);
		m_spaceRadarMediator->setStickyVisible(true);
		m_spaceRadarMediator->fetch();
		m_spaceRadarMediator->activate();
		getWorkspace().addMediator(*m_spaceRadarMediator);
	}

	{
		hud.getCodeDataObject(TUIPage, mediatorPage, "AllTargets");
		mediatorPage->SetEnabled(false);
		SwgCuiAllTargets * const allTargets = new SwgCuiAllTargetsSpace(*mediatorPage);
		allTargets->setStickyVisible(true);
		allTargets->activate();
		getWorkspace().addMediator(*allTargets);
	} //lint !e429 custodial pointer not freed or returned.  The Workspace owns it.


	{
		hud.getCodeDataObject(TUIPage, mediatorPage, "ShipReticle");
		mediatorPage->SetEnabled(true);
		m_shipReticle = new SwgCuiShipReticle(*mediatorPage);
		m_shipReticle->setStickyVisible(true);
		m_shipReticle->fetch();
		m_shipReticle->activate();
		getWorkspace().addMediator(*m_shipReticle);
	} //lint !e429 custodial pointer not freed or returned.  The Workspace owns it.

	{
		hud.getCodeDataObject(TUIPage, mediatorPage, "FlyOut");
		mediatorPage->SetEnabled(false);
		mediatorPage->SetEnabled(true);
		m_spaceFlyOutPage = new SwgCuiSpaceFlyOutPage(*mediatorPage);
		m_spaceFlyOutPage->setStickyVisible(true);
		m_spaceFlyOutPage->fetch();
		m_spaceFlyOutPage->activate();
		getWorkspace().addMediator(*m_spaceFlyOutPage);
	} //lint !e429 custodial pointer not freed or returned.  The Workspace owns it.

	//-----------------------------------------------------------------
	{
		hud.getCodeDataObject(TUIPage, mediatorPage, "GroupStatus");
		mediatorPage->SetEnabled(false);
		mediatorPage->SetEnabled(true);
		SwgCuiSpaceGroup * const group = new SwgCuiSpaceGroup(*mediatorPage);
		group->setSettingsAutoSizeLocation(true, true);
		group->setStickyVisible(true);
		group->activate();
		getWorkspace().addMediator(*group);
	} //lint !e429 custodial pointer not freed or returned.  The Workspace owns it.

	{
		hud.getCodeDataObject(TUIPage, mediatorPage, "MissileCount");
		mediatorPage->SetEnabled(false);
		mediatorPage->SetEnabled(true);
		m_spaceMissileCount = new SwgCuiSpaceMissileCount(*mediatorPage);
		m_spaceMissileCount->setStickyVisible(true);
		m_spaceMissileCount->fetch();
		m_spaceMissileCount->activate();
		getWorkspace().addMediator(*m_spaceMissileCount);
	} //lint !e429 custodial pointer not freed or returned.  The Workspace owns it.

	//-----------------------------------------------------------------
	{
		hud.getCodeDataObject(TUIPage, mediatorPage, "TargetsPage");
		mediatorPage->SetEnabled(false);
		m_targetsMediator = new SwgCuiTargets(*mediatorPage);
		m_targetsMediator->setSettingsAutoSizeLocation(true, true);
		m_targetsMediator->setStickyVisible(true);
		m_targetsMediator->fetch();
		m_targetsMediator->activate();
		getWorkspace().addMediator(*m_targetsMediator);
	}


	//-- Activate the target info if necessary.
	if (CuiPreferences::getUseSpaceDebugWindows())
	{
		CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_ShipSelfInfo);
		CuiMediatorFactory::activateInWorkspace(CuiMediatorTypes::WS_ShipTargetInfo);
	}
}

//----------------------------------------------------------------------

SwgCuiHudWindowManagerSpace::~SwgCuiHudWindowManagerSpace()
{
	getWorkspace().removeMediator(*m_spaceRadarMediator);
	m_spaceRadarMediator->release();
	m_spaceRadarMediator = 0;

	getWorkspace().removeMediator(*m_spaceFlyOutPage);
	m_spaceFlyOutPage->release();
	m_spaceFlyOutPage = 0;

	getWorkspace().removeMediator(*m_spaceMissileCount);
	m_spaceMissileCount->release();
	m_spaceMissileCount = 0;

	getWorkspace().removeMediator(*m_shipReticle);
	m_shipReticle->release();
	m_shipReticle = 0;

	getWorkspace().removeMediator(*m_targetsMediator);
	m_targetsMediator->release();
	m_targetsMediator = 0;
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerSpace::receiveMessage(MessageDispatch::Emitter const & emitter, MessageDispatch::MessageBase const & message)
{
	if (message.isType(NewbieTutorialEnableHudElement::cms_name))
	{
		//-- what type of request is it?
		Archive::ReadIterator ri = NON_NULL(safe_cast<const GameNetworkMessage*>(&message))->getByteStream().begin();
		NewbieTutorialEnableHudElement const newbieTutorialEnableHudElement(ri);

		if (newbieTutorialEnableHudElement.getName() == cms_newbieTutorialEnableHudElementRadar || newbieTutorialEnableHudElement.getName() == cms_newbieTutorialEnableHudElementAll)
		{
			setBlinkingMediator(*m_spaceRadarMediator, newbieTutorialEnableHudElement.getBlinkTime());

			if (newbieTutorialEnableHudElement.getEnable())
			{
				if (!m_spaceRadarMediator->isActive())
					m_spaceRadarMediator->activate();
			}
			else
			{
				if (m_spaceRadarMediator->isActive())
					m_spaceRadarMediator->deactivate();
			}
		}
	}

	SwgCuiHudWindowManager::receiveMessage(emitter, message);
}

//----------------------------------------------------------------------

#ifdef ENABLE_FORMATIONS
void SwgCuiHudWindowManagerSpace::spawnSpaceAssignPlayerFormation() const
{
	CuiMediatorFactory::toggleInWorkspace(CuiMediatorTypes::WS_SpaceAssignPlayerFormation);
}
#endif

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerSpace::spawnShipComponentDetail(NetworkId const & shipId) const
{
	Object * const o = NetworkIdManager::getObjectById(shipId);
	ClientObject * const co = o ? o->asClientObject() : NULL;
	ShipObject * const ship = co ? co->asShipObject() : NULL;
	if(ship)
	{
		SwgCuiShipComponentDetail * const mediator = safe_cast<SwgCuiShipComponentDetail * const>(CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_ShipComponentDetail));
		mediator->setShip(ship);
	}
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerSpace::spawnHyperspaceMap() const
{
	CuiMediatorFactory::toggleInWorkspace(CuiMediatorTypes::WS_HyperspaceMap);
}

//----------------------------------------------------------------------

void SwgCuiHudWindowManagerSpace::setReticleVisible(bool b)
{
	m_shipReticle->setReticleVisible(b);
}

//======================================================================

