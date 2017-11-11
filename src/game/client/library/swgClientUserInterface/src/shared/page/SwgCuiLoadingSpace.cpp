//======================================================================
//
// SwgCuiLoadingSpace.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiLoadingSpace.h"

#include "StringId.h"
#include "UIButton.h"
#include "UIClock.h"
#include "UIImage.h"
#include "UIPage.h"
#include "UIPie.h"
#include "UIText.h"
#include "UIMessage.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/Scene.h"
#include "clientUserInterface/CuiLoadingManager.h"
#include "clientUserInterface/CuiManager.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Production.h"
#include "sharedMessageDispatch/Message.h"


//======================================================================

namespace SwgCuiLoadingSpaceNamespace
{
	char const * const cms_texturePre = "texture/";
	char const * const cms_texturePost = ".dds";
	StringId const     cms_defaultTextId("loading/tips", "city_map");
	StringId const     cms_defaultTitleId("loading/tipcms_n", "city_map");
	char const * const cms_defaultPicture = "loading/space/images/space_load_escapepod";
}

using namespace SwgCuiLoadingSpaceNamespace;

//======================================================================

SwgCuiLoadingSpace::SwgCuiLoadingSpace(UIPage & page) :
CuiMediator          ("SwgCuiLoadingSpace", page),
UIEventCallback      (),
UINotification       (),
m_textScreenshotName (NULL),
m_text               (NULL),
m_pie                (NULL),
m_backButton         (NULL),
m_escButton          (NULL),
m_image              (NULL),
m_textLoad           (NULL),
m_defaultBgPage      (NULL)
{
	getCodeDataObject (TUIText,     m_textScreenshotName, "screenshotname");
	getCodeDataObject (TUIText,     m_text,               "text");
	getCodeDataObject (TUIPie,      m_pie,                "pie");
	getCodeDataObject (TUIButton,   m_backButton,         "backbutton");
	getCodeDataObject (TUIButton,   m_escButton,          "buttonEsc");
	getCodeDataObject (TUIImage,    m_image,              "screenshot");
	getCodeDataObject (TUIText,     m_textLoad,           "textprogress");
	getCodeDataObject (TUIPage,     m_defaultBgPage,      "default");

	m_textScreenshotName->Clear();
	m_text->Clear();
	m_textLoad->Clear();
	
	registerMediatorObject (*m_backButton, true);
	registerMediatorObject (*m_escButton, true);
}

//----------------------------------------------------------------------

SwgCuiLoadingSpace::~SwgCuiLoadingSpace()
{
	m_textScreenshotName = NULL;
	m_text = NULL;
	m_pie = NULL;
	m_backButton = NULL;
	m_image = NULL;
	m_textLoad = NULL;
}

//----------------------------------------------------------------------

void SwgCuiLoadingSpace::performActivate()
{
	setKeyboardInputActive (true);

	// force ui to render once
	update();

	if (!Game::isPlayingCutScene())
	{
		CuiManager::forceRender ();
	}

	UIClock::gUIClock ().ListenPerFrame (this);
	GameNetwork::setAcceptSceneCommand (true);

	CuiLoadingManager::setLoadingScreenVisible(true);
	//mark that the first loading screen of this loading sequence has definitely been displayed
	CuiLoadingManager::setFirstLoadingScreenHasBeenShown();

	std::string const & planetName = CuiLoadingManager::getPlanetName();
	setupBackground(planetName);
}

//----------------------------------------------------------------------

void SwgCuiLoadingSpace::performDeactivate()
{
	setKeyboardInputActive (false);
	UIClock::gUIClock ().StopListening (this);
	CuiLoadingManager::setLoadingScreenVisible(false);
}

//----------------------------------------------------------------------

void SwgCuiLoadingSpace::setupBackground(std::string const & )
{
	m_defaultBgPage->SetVisible(true);
}

//----------------------------------------------------------------------

void SwgCuiLoadingSpace::OnButtonPressed(UIWidget * const context)
{
	if (context == m_backButton)
	{		
		deactivate ();
		GameNetwork::setAcceptSceneCommand (false);
		GameNetwork::disconnectConnectionServer ();
		CuiLoadingManager::setFullscreenLoadingEnabled    (false);

		Scene * const scene = Game::getScene ();
		if (scene) 
			scene->quit ();
	}
	else if(context == m_escButton)
	{
		if (Game::isPlayingCutScene())
		{
			Game::skipCutScene();
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiLoadingSpace::setupPage(std::string const & planetName, StringId const & textId, StringId const & titleId, std::string const & picture)
{
	setupBackground(planetName);
	UNREF(planetName);

	//make local copies of the data since we may need to override the values
	StringId localTextId(textId);
	StringId localTitleId(titleId);
	std::string localPicture(picture);

	if(localTextId.isInvalid() || localTitleId.isInvalid() || localPicture.empty())
	{
		localTextId     = cms_defaultTextId;
		localTitleId    = cms_defaultTitleId;
		localPicture    = cms_defaultPicture;
	}

	Unicode::String localizedText;

	//set title text
#if PRODUCTION
	if(!localTitleId.localize(localizedText))
		localizedText = Unicode::emptyString;
#else
	localizedText = localTitleId.localize();
#endif
	m_textScreenshotName->SetText(localizedText);

	//set tip text
#if PRODUCTION
	if(!localTextId.localize(localizedText))
		localizedText = Unicode::emptyString;
#else
	localizedText = localTextId.localize();
#endif
	m_text->SetText(localizedText);

	//set pictures
	std::string screenshot = localPicture;
	if(!TreeFile::exists((cms_texturePre + localPicture + cms_texturePost).c_str()))
		screenshot = cms_defaultPicture;
	IGNORE_RETURN(m_image->SetSourceResource(Unicode::narrowToWide(screenshot)));
}

//----------------------------------------------------------------------

void SwgCuiLoadingSpace::update()
{
	int const percent = CuiLoadingManager::getFullscreenLoadingPercent ();
	float const value = static_cast<float>(percent) / 100.0f;
	m_pie->SetValue(value);
}

//----------------------------------------------------------------------

void SwgCuiLoadingSpace::Notify( UINotificationServer * const , UIBaseObject * const , Code const )
{
	if (m_textLoad)
	{
		const Unicode::String & str = CuiLoadingManager::getFullscreenLoadingString ();
		if (str != m_text->GetText ())
			m_textLoad->SetText (str);
	}

	update();
}
//----------------------------------------------------------------------

