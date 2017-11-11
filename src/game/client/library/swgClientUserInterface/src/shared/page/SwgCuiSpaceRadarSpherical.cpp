//======================================================================
//
// SwgCuiSpaceRadarSpherical.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceRadarSpherical.h"
#include "swgClientUserInterface/SwgCuiSpaceRadarSpherical_BlipPane.h"

#include "clientGame/Game.h"
#include "clientGame/ShipObject.h"
#include "clientUserInterface/CuiAction.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiSettings.h"
#include "clientUserInterface/CuiWorkspaceIcon.h"
#include "sharedFoundation/Misc.h"
#include "sharedMessageDispatch/Transceiver.h"

#include "UIButton.h"
#include "UIColorEffector.h"
#include "UIData.h"
#include "UIEllipse.h"
#include "UIImage.h"
#include "UIMessage.h"

#include "UnicodeUtils.h"

//======================================================================

namespace SwgCuiSpaceRadarSphericalNamespace
{
}

using namespace SwgCuiSpaceRadarSphericalNamespace;

//======================================================================

SwgCuiSpaceRadarSpherical::SwgCuiSpaceRadarSpherical(UIPage & page) :
CuiMediator("SwgCuiSpaceRadarSpherical", page),
UIEventCallback(),
m_blipPane(NULL),
m_pageSquare(NULL),
m_lastCoord(LONG_MIN, LONG_MIN),
m_lastSize(),
m_frontShields(new EllipseVector),
m_backShields(new EllipseVector),
m_frontArmor(new EllipseVector),
m_backArmor(new EllipseVector),
m_shieldOverDriveEffector(NULL),
m_shieldPage(NULL),
m_isShieldEffectorEnabled(false)
{
	getCodeDataObject(TUIPage,     m_pageSquare,         "square");

	UIImage * arrowImage = 0;
	UIImage * arrowWaypointImage = 0;
	UIImage * arrowBracket       = 0;
	UIImage * arrowGroupImage    = 0;
	
	getCodeDataObject(TUIImage,     arrowImage,               "arrow");
	getCodeDataObject(TUIImage,     arrowWaypointImage,       "arrowWaypoint");
	getCodeDataObject(TUIImage,     arrowBracket,             "arrowBracket");
	getCodeDataObject(TUIImage,     arrowGroupImage,          "arrowGroup");

	
	UIColorEffector * effectorWaypoint = 0;
	getCodeDataObject(TUIColorEffector,     effectorWaypoint, "effectorWaypoint");
	
	arrowWaypointImage->ExecuteEffector(*effectorWaypoint);
	
	arrowImage->SetVisible(false);
	arrowWaypointImage->SetVisible(false);
	arrowBracket->SetVisible(false);
	arrowGroupImage->SetVisible(false);
	
	getPage().SetVisible(true); 
	
	setLockedAspectRatio(UIFloatPoint::one);
	
	UIPage * const blipPaneParent = safe_cast<UIPage *>(arrowImage->GetParentWidget());
	blipPaneParent->SetVisible(false);
	

	Unicode::String tmp;
	blipPaneParent->GetProperty(UIWidget::PropertyName::PackSize, tmp);
	blipPaneParent->GetProperty(UIWidget::PropertyName::PackLocation, tmp);
	
	{
		UIImage * blip          = 0;
		UIImage * blipCorpse    = 0;
		UIImage * blipWaypoint  = 0;
		UIImage * blipStructure = 0;
		UIImage * blipBracket   = 0;
		UIImage * blipGroup     = 0;
		
		getCodeDataObject(TUIImage, blip,          "blipDefault");
		getCodeDataObject(TUIImage, blipCorpse,    "blipCorpse");
		getCodeDataObject(TUIImage, blipWaypoint,  "blipWaypoint");
		getCodeDataObject(TUIImage, blipStructure, "blipStructure");
		getCodeDataObject(TUIImage, blipBracket,   "blipBracket");
		getCodeDataObject(TUIImage, blipGroup,     "blipGroup");
		
		blipWaypoint->ExecuteEffector(*effectorWaypoint);
		
		m_blipPane = new BlipPane(*blip, *blipCorpse, *blipWaypoint, *blipStructure, *blipBracket, *blipGroup);
		IGNORE_RETURN(m_pageSquare->InsertChildAfter(m_blipPane, blipPaneParent));
		blipPaneParent->GetProperty(UIWidget::PropertyName::PackSize, tmp);
		IGNORE_RETURN(m_blipPane->SetProperty(UIWidget::PropertyName::PackSize, tmp));
		blipPaneParent->GetProperty(UIWidget::PropertyName::PackLocation, tmp);
		IGNORE_RETURN(m_blipPane->SetProperty(UIWidget::PropertyName::PackLocation, tmp));
		m_blipPane->SetRect(blipPaneParent->GetRect());
		m_blipPane->Attach(0);
	}


	{
		UIEllipse * ellipse = NULL;

		getCodeDataObject(TUIEllipse, ellipse, "shieldsFrontLeft");
		m_frontShields->push_back(ellipse);

		getCodeDataObject(TUIEllipse, ellipse, "shieldsFrontRight");
		m_frontShields->push_back(ellipse);


		getCodeDataObject(TUIEllipse, ellipse, "shieldsBackLeft");
		m_backShields->push_back(ellipse);

		getCodeDataObject(TUIEllipse, ellipse, "shieldsBackRight");
		m_backShields->push_back(ellipse);


		getCodeDataObject(TUIEllipse, ellipse, "armorFrontLeft");
		m_frontArmor->push_back(ellipse);
		
		getCodeDataObject(TUIEllipse, ellipse, "armorFrontRight");
		m_frontArmor->push_back(ellipse);

	
		getCodeDataObject(TUIEllipse, ellipse, "armorBackLeft");
		m_backArmor->push_back(ellipse);
		
		getCodeDataObject(TUIEllipse, ellipse, "armorBackRight");
		m_backArmor->push_back(ellipse);
	}

	getCodeDataObject(TUIEffector, m_shieldOverDriveEffector, "shieldOverDriveEffector");
	getCodeDataObject(TUIPage, m_shieldPage, "shieldPage");

	registerMediatorObject(getPage(), true);
	registerMediatorObject(*m_shieldPage, true);
}

//----------------------------------------------------------------------

SwgCuiSpaceRadarSpherical::~SwgCuiSpaceRadarSpherical()
{	
	if (m_blipPane)
	{
		m_blipPane->Detach(0);
		m_blipPane = NULL;
	}
	
	m_pageSquare = NULL;

	delete m_frontShields;
	m_frontShields = NULL;

	delete m_backShields;
	m_backShields = NULL;

	delete m_frontArmor;
	m_frontArmor = NULL;

	delete m_backArmor;
	m_backArmor = NULL;

	m_shieldOverDriveEffector = NULL;
	m_shieldPage = NULL;
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadarSpherical::performActivate()
{
	setIsUpdating(true);
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadarSpherical::performDeactivate()
{
	setIsUpdating(false);
	enableShieldEffector(false);
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarSpherical::update(float const deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);

	updateShieldEffector();

	const UISize & size = getPage().GetSize();
	if (size != m_lastSize)
	{
		const UIWidget * const parent = NON_NULL(m_pageSquare->GetParentWidget());
		const UISize & parentSize = parent->GetSize();
		const long min = std::min(parentSize.x, parentSize.y);
		m_pageSquare->SetSize(UISize(min, min));
		m_pageSquare->Center ();
		m_pageSquare->ForcePackChildren();
		m_lastSize = size;
	}

	if (m_blipPane)
	{
		m_blipPane->update(deltaTimeSecs);
	}

	//-- Update the ship shields and armor readouts.
	ShipObject * const playerShip = Game::getPlayerContainingShip();
	if (playerShip)
	{
		updateEllipseVector(m_frontShields, playerShip->getShieldHitpointsFrontCurrent(), playerShip->getShieldHitpointsFrontMaximum());
		updateEllipseVector(m_backShields, playerShip->getShieldHitpointsBackCurrent(), playerShip->getShieldHitpointsBackMaximum());
		
		updateEllipseVector(m_frontArmor, playerShip->getComponentArmorHitpointsCurrent(ShipChassisSlotType::SCST_armor_0), 
			playerShip->getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_0));
		
		updateEllipseVector(m_backArmor, playerShip->getComponentArmorHitpointsCurrent(ShipChassisSlotType::SCST_armor_1), 
			playerShip->getComponentArmorHitpointsMaximum(ShipChassisSlotType::SCST_armor_1));
	}
	else
	{
		// Hide pages if we don't have a ship.
		updateEllipseVector(m_frontShields, 0.0f, 0.0f);
		updateEllipseVector(m_backShields, 0.0f, 0.0f);
		updateEllipseVector(m_frontArmor, 0.0f, 0.0f);
		updateEllipseVector(m_backArmor, 0.0f, 0.0f);
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarSpherical::updateEllipseVector(EllipseVector *vec, float num, float den)
{
	NOT_NULL(vec);

	float percent = den > FLT_MIN ? num / den : 0.0f;
	percent = clamp(0.f, percent, 1.0f);

	for (EllipseVector::iterator it = vec->begin(); it != vec->end(); ++it)
	{
		UIEllipse * const ellipse = *it;
		if (ellipse)
		{
			ellipse->SetVisible(num > 0.0f && den > 0.0f);
			ellipse->SetPercent(percent);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarSpherical::enableShieldEffector(bool const enabled)
{
	if (!boolEqual(enabled, m_isShieldEffectorEnabled))
	{
		m_isShieldEffectorEnabled = enabled;
		
		if (enabled)
		{
			m_shieldPage->ExecuteEffector(*m_shieldOverDriveEffector);
		}
		else
		{
			m_shieldPage->CancelEffector(*m_shieldOverDriveEffector);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiSpaceRadarSpherical::updateShieldEffector()
{
	bool shouldEnableShieldEffector = false;

	ShipObject * const playerShip = Game::getPlayerContainingShip();
	if (playerShip)
	{
		shouldEnableShieldEffector = (playerShip->getShieldHitpointsFrontMaximum() > 0.0f && playerShip->getShieldHitpointsFrontCurrent() > playerShip->getShieldHitpointsFrontMaximum()) ||
									(playerShip->getShieldHitpointsBackMaximum() > 0.0f && playerShip->getShieldHitpointsBackCurrent() > playerShip->getShieldHitpointsBackMaximum());
	}

	enableShieldEffector(shouldEnableShieldEffector);
}


//======================================================================
