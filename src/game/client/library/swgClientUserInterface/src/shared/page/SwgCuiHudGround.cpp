//======================================================================
//
// SwgCuiHudGround.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHudGround.h"

#include "clientGame/ConfigClientGame.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientObject/GameCamera.h"
#include "clientUserInterface/CuiConversationMenu.h"
#include "swgClientUserInterface/SwgCuiHudActionGround.h"
#include "swgClientUserInterface/SwgCuiHudWindowManagerGround.h"

#include "UIManager.h"

//======================================================================

namespace SwgCuiHudGroundNamespace
{
	static const std::string name_hudPage         = "GroundHUD";
	static const std::string path_hudPage         = "/" + name_hudPage;
	static const std::string name_hudPageTemplate = "templateGroundHUD";
	static const std::string path_hudPageTemplate = "/" + name_hudPageTemplate;
}

using namespace SwgCuiHudGroundNamespace;

//----------------------------------------------------------------------

SwgCuiHudGround::SwgCuiHudGround(UIPage & page) :
SwgCuiHud(page),
m_convo(NULL)
{
	setWindowManager(new SwgCuiHudWindowManagerGround(*this, getWorkspace()));
	setAction(new SwgCuiHudActionGround(*this));
	setHudEnabled(!ConfigClientGame::getHudDisabled());
}

//----------------------------------------------------------------------

SwgCuiHudGround::~SwgCuiHudGround()
{
	deleteConvo();
}

//----------------------------------------------------------------------

void SwgCuiHudGround::createHudTemplate()
{
	internalCreateHudTemplate(name_hudPage, name_hudPageTemplate);
}

//----------------------------------------------------------------------

void SwgCuiHudGround::createFreshHud()
{
	internalCreateFreshHud(name_hudPage, name_hudPageTemplate);
}

//-----------------------------------------------------------------

void SwgCuiHudGround::update(float deltaTimeSecs)
{	
	SwgCuiHud::update(deltaTimeSecs);

	GroundScene * const gs = dynamic_cast<GroundScene *>(Game::getScene());
	Camera const * const camera = gs ? NON_NULL(gs->getCurrentCamera()) : 0;

	if (m_convo && m_convo->isValid() && m_convo->updatePosition(*camera))
		UIManager::gUIManager().RefreshMousePosition();
}

//-----------------------------------------------------------------

void SwgCuiHudGround::newConvo()
{
	deleteConvo();

	m_convo = new CuiConversationMenu(getPage());
}

//-----------------------------------------------------------------

void SwgCuiHudGround::deleteConvo()
{
	delete m_convo;
	m_convo = NULL;
}

//-----------------------------------------------------------------

void SwgCuiHudGround::performActivate()
{
	SwgCuiHud::performActivate();
	newConvo();
}

//-----------------------------------------------------------------

void SwgCuiHudGround::performDeactivate()
{
	deleteConvo();
	SwgCuiHud::performDeactivate();
}


//======================================================================
