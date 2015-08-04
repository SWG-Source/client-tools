//======================================================================
//
// SwgCuiNpeContinuation.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiNpeContinuation.h"

#include "clientGame/Game.h"
#include "clientGame/PlayerObject.h"
#include "clientGame/RoadmapManager.h"
#include "clientUserInterface/CuiManager.h"
#include "UIButton.h"

namespace SwgCuiNpeContinuationNamespace
{

}

using namespace SwgCuiNpeContinuationNamespace;

//----------------------------------------------------------------------

SwgCuiNpeContinuation::SwgCuiNpeContinuation (UIPage & page) :
CuiMediator         ("SwgCuiNpeContinuation", page),
UIEventCallback     (),
m_previewButton(0),
m_continueButton(0),
m_returnButton(0)
{
	getCodeDataObject (TUIButton, m_previewButton,     "preview", true);
	getCodeDataObject (TUIButton, m_continueButton,    "continue", true);
	getCodeDataObject (TUIButton, m_returnButton,      "return", true);
	getCodeDataObject (TUIButton, m_closeButton,       "close", true);

	registerMediatorObject(*m_previewButton, true);
	registerMediatorObject(*m_continueButton, true);
	registerMediatorObject(*m_returnButton, true);
	registerMediatorObject(*m_closeButton, true);

	//page can be closed
	setState(MS_closeable);
	setState(MS_closeDeactivates);
}


//----------------------------------------------------------------------

SwgCuiNpeContinuation::~SwgCuiNpeContinuation ()
{
	m_previewButton = 0;
	m_continueButton = 0;
	m_returnButton = 0;
	m_closeButton = 0;
}

//----------------------------------------------------------------------

void SwgCuiNpeContinuation::performActivate()
{
	CuiManager::requestPointer (true);
}

//----------------------------------------------------------------------

void SwgCuiNpeContinuation::performDeactivate()
{	
	CuiManager::requestPointer (false);
}

//----------------------------------------------------------------------

void SwgCuiNpeContinuation::OnButtonPressed   (UIWidget *context)
{
	if (context == m_previewButton)
	{
		PlayerObject const * player = Game::getPlayerObject();
		std::string const &templ = player->getSkillTemplate();
		Game::playProfessionMovie( RoadmapManager::getRoadmapNameForTemplateName(templ).c_str() );
	}
	else if (context == m_continueButton)
	{
		Game::externalCommand( "npe_continue" );
	}
	else if (context == m_returnButton)
	{
		closeThroughWorkspace ();
	}
	else if (context == m_closeButton)
	{
		closeThroughWorkspace ();
	}
}

