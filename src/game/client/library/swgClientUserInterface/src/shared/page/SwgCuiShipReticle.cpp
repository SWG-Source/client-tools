//======================================================================
//
// SwgCuiShipReticle.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiShipReticle.h"

#include "UIBaseObject.h"
#include "UIButton.h"
#include "UIClock.h"
#include "UIColorEffector.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIMessage.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UIWidget.h"
#include "UnicodeUtils.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/ShipObject.h"
#include "clientUserInterface/CuiAction.h"
#include "clientUserInterface/CuiActionManager.h"
#include "clientUserInterface/CuiArcGauge.h"
#include "clientUserInterface/CuiIoWin.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedGame/ConfigSharedGame.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedUtility/Callback.h"

#include <list>

//======================================================================

namespace SwgCuiShipReticleNamespace
{
	bool s_disableReticleInThirdPerson = false;
	float const s_shipSpeedMultiplier = 10.0f; // Design request.
}

using namespace SwgCuiShipReticleNamespace;

//-----------------------------------------------------------------

SwgCuiShipReticle::SwgCuiShipReticle(UIPage & page) :
CuiMediator("SwgCuiShipReticle", page),
CallbackReceiver(),
m_arcSpeed(NULL),
m_arcWeapon(NULL),
m_arcBooster(NULL),
m_reticles(new ImageVector),
m_reticleCurrent(NULL)
{
	//--
	{
		UIPage * arcSpeedPage = NULL;
		getCodeDataObject(TUIPage, arcSpeedPage, "SpeedGauge");
		arcSpeedPage->SetEnabled(true);
		m_arcSpeed = new CuiArcGuage(*arcSpeedPage);
		m_arcSpeed->fetch();
		m_arcSpeed->setStickyVisible(true);
		m_arcSpeed->activate();
		m_arcSpeed->setTextDisplayMultiplier(s_shipSpeedMultiplier);
	}

	//--
	{
		UIPage * arcWeaponPage = NULL;
		getCodeDataObject(TUIPage, arcWeaponPage, "WeaponGauge");
		arcWeaponPage->SetEnabled(true);
		m_arcWeapon = new CuiArcGuage(*arcWeaponPage);
		m_arcWeapon->fetch();
		m_arcWeapon->setStickyVisible(true);
		m_arcWeapon->activate();
	}

	//--
	{
		UIPage * arcBoosterPage = NULL;
		getCodeDataObject(TUIPage, arcBoosterPage, "BoosterGauge");
		arcBoosterPage->SetEnabled(true);
		m_arcBooster = new CuiArcGuage(*arcBoosterPage);
		m_arcBooster->fetch();
		m_arcBooster->setStickyVisible(true);
		m_arcBooster->activate();
	}

	//--
	{
		//-- Get the images on the reticle page.
		UIPage * reticlePage = NULL;
		getCodeDataObject(TUIPage, reticlePage, "Reticles");
		UIBaseObject::UIObjectList children;
		reticlePage->GetChildren(children);

		for (UIBaseObject::UIObjectList::iterator reticleIterator = children.begin(); reticleIterator != children.end(); ++reticleIterator)
		{
			UIBaseObject * object = *reticleIterator;
			if (object && object->IsA(TUIImage))
			{
				m_reticles->push_back(safe_cast<UIImage *>(object));
			}
		}

		enableReticle(CuiPreferences::getReticleSelect());
		CuiPreferences::getReticleSelectCallback().attachReceiver(*this);
	}

	registerMediatorObject(getPage(), true);

	DebugFlags::registerFlag(s_disableReticleInThirdPerson, "SwgClientUserInterface", "disableReticleInThirdPerson");
}


//----------------------------------------------------------------------

SwgCuiShipReticle::~SwgCuiShipReticle()
{	
	CuiPreferences::getReticleSelectCallback().detachReceiver(*this);

	m_arcSpeed->release();
	m_arcSpeed = NULL;

	m_arcWeapon->release();
	m_arcWeapon = NULL;

	m_arcBooster->release();
	m_arcBooster = NULL;

	m_reticleCurrent = NULL;

	delete m_reticles;
	m_reticles = NULL;

}

//-----------------------------------------------------------------

void SwgCuiShipReticle::performActivate()
{
	setIsUpdating(true);
}

//-----------------------------------------------------------------

void SwgCuiShipReticle::performDeactivate()
{
	setIsUpdating(false);
}

//-----------------------------------------------------------------

void SwgCuiShipReticle::enableReticle(int const index)
{
	m_reticleCurrent = NULL;

	if (m_reticles->size())
	{
		for (ImageVector::iterator imageIterator = m_reticles->begin(); imageIterator != m_reticles->end(); ++imageIterator)
		{
			UIImage * const image = *imageIterator;
			if (image)
			{
				image->SetVisible(false);
			}
		}
		
		int const maxReticles = static_cast<int>(m_reticles->size() - 1);
		int const clampedIndex = clamp(0, index, maxReticles);
		m_reticleCurrent = (*m_reticles)[static_cast<unsigned>(clampedIndex)];

		if (m_reticleCurrent)
		{
			m_reticleCurrent->SetVisible(true);
		}
	}
}

//----------------------------------------------------------------------


void SwgCuiShipReticle::update(float const deltaTimeSecs)
{
	//-- Center before the update to prevent frame lag.
	UIPoint screenCenter;
	CuiManager::getIoWin().getScreenCenter(screenCenter);
	getPage().SetLocation(screenCenter, true);

	CuiMediator::update(deltaTimeSecs);

	// In third person, you may want to disable the reticle.
	bool shouldShowReticle = !(s_disableReticleInThirdPerson && Game::isViewFreeCamera());
	
	ShipObject const * const playerShip = Game::getPlayerContainingShip();
	if (playerShip)
	{
		shouldShowReticle = shouldShowReticle && !playerShip->hasCondition(TangibleObject::C_docking);

		//-- Engine speed.
		float const maxSpeed = playerShip->getEngineSpeedMaximum() * (playerShip->getChassisSpeedMaximumModifier() * playerShip->getComponentEfficiencyGeneral(ShipChassisSlotType::SCST_engine));
		m_arcSpeed->setMax(maxSpeed);
		m_arcSpeed->setCurrent(playerShip->getCurrentSpeed());
		
		PlayerShipController const * const shipController = dynamic_cast<PlayerShipController const *>(playerShip->getController());
		float const speedTickValue = shipController ? (shipController->getThrottlePosition() * maxSpeed) : 0.0f;
		m_arcSpeed->setTick(speedTickValue);

		//-- Weapon capacitor.
		if (!playerShip->isComponentDemolished(ShipChassisSlotType::SCST_capacitor))
		{
			m_arcWeapon->setMax(playerShip->getCapacitorEnergyMaximum());
			m_arcWeapon->setCurrent(playerShip->getCapacitorEnergyCurrent());
			m_arcWeapon->setTick(playerShip->getCapacitorEnergyCurrent());
		}
		else
		{
			m_arcWeapon->setMax(0);
			m_arcWeapon->setCurrent(0);
			m_arcWeapon->setTick(0);
		}

		//-- Booster energy.
		m_arcBooster->getPage().SetVisible(playerShip->isSlotInstalled(ShipChassisSlotType::SCST_booster));
		m_arcBooster->setMax(playerShip->getBoosterEnergyMaximum());
		m_arcBooster->setCurrent(playerShip->getBoosterEnergyCurrent() * playerShip->getComponentEfficiencyGeneral(ShipChassisSlotType::SCST_booster));
		m_arcBooster->setTick(playerShip->getBoosterEnergyCurrent());
	}

	getPage().SetVisible(shouldShowReticle);
}

//----------------------------------------------------------------------

void SwgCuiShipReticle::performCallback()
{
	enableReticle(CuiPreferences::getReticleSelect());
}

//----------------------------------------------------------------------

void SwgCuiShipReticle::setReticleVisible(bool b)
{
	if (NULL != m_reticleCurrent)
		m_reticleCurrent->SetVisible(b);
}

//======================================================================
