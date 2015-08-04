//======================================================================
//
// SwgCuiLoadingGround.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiLoadingGround.h"

#include "StringId.h"
#include "UIButton.h"
#include "UICheckbox.h"
#include "UIClock.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/Scene.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiLoadingManager.h"
#include "clientUserInterface/CuiManager.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Production.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedRandom/Random.h"

//======================================================================

namespace SwgCuiLoadingGroundNamespace
{
	const std::string planetPre("loading/");
	const std::string planetPost("/ui_load_");
	const std::string flagPre("loading/");
	const std::string flagPost1("/ui_load_");
	const std::string flagPost2("_flag");
	const std::string texturePre("texture/");
	const std::string texturePost(".dds");
	const std::string defaultFlag("loading/generic/ui_load_generic_flag");
	const std::string defaultPlanet("loading/tatooine/ui_load_tatooine");
	const std::string defaultImage("loading/tatooine/creatures/tatooine_jawa");
	const std::string genericRebel("loading/generic/generic_rebel");
	const std::string genericImperial("loading/generic/generic_imperial");


	const std::string s_defaultPlanetName("general");
	const StringId    s_defaultTextId("loading/tips", "city_map");
	const StringId    s_defaultTitleId("loading/tips_n", "city_map");
	const std::string s_defaultPicture("loading/generic/generic_overheadmap");
}

using namespace SwgCuiLoadingGroundNamespace;

//======================================================================

SwgCuiLoadingGround::SwgCuiLoadingGround (UIPage & page) :
CuiMediator          ("SwgCuiLoadingGround", page),
UIEventCallback      (),
UINotification       (),
MessageDispatch::Receiver(),
m_bar                (0),
m_text               (0),
m_textPlanetName     (0),
m_textScreenshotName (0),
m_textPercentScroll  (0),
m_flagImage          (0),
m_planetImage        (0),
m_genericImage       (0),
m_tipImage           (0),
m_idleValue          (0),
m_wasIdling          (false),
m_idleUp             (true),
m_buttonBack         (0),
m_buttonEsc          (0),
m_textLoad           (0),
m_checkServerObjects (0),
m_checkFileCaching        (0),
m_checkWorldLoaded        (0),
m_checkTerrainGenerated   (0),
m_checkPlayerReady        (0)
{
	getCodeDataObject (TUIWidget,   m_bar,                "progressbar");
	getCodeDataObject (TUIText,     m_text,               "textdesc");
	getCodeDataObject (TUIText,     m_textPlanetName,     "textplanet");
	getCodeDataObject (TUIImage,    m_flagImage,          "flagImage");
	getCodeDataObject (TUIImage,    m_planetImage,        "planetImage");
	getCodeDataObject (TUIImage,    m_genericImage,       "genericRight");
	getCodeDataObject (TUIImage,    m_tipImage,           "screenshot");
	getCodeDataObject (TUIText,     m_textScreenshotName, "screenshotname");
	getCodeDataObject (TUIText,     m_textPercentScroll,  "percentscroll");
	getCodeDataObject (TUIText,     m_textLoad,           "textProgress");
	getCodeDataObject (TUIButton,   m_buttonBack,         "buttonPrev");
	getCodeDataObject (TUIButton,   m_buttonEsc,          "buttonEsc");
	getCodeDataObject (TUICheckbox, m_checkServerObjects,    "checkserverobjects");
	getCodeDataObject (TUICheckbox, m_checkFileCaching,      "checkfilecaching");
	getCodeDataObject (TUICheckbox, m_checkWorldLoaded,      "checkworldloaded");
	getCodeDataObject (TUICheckbox, m_checkTerrainGenerated, "checkterraingenerated");
	getCodeDataObject (TUICheckbox, m_checkPlayerReady,      "checkplayerready");

	if(!ConfigClientUserInterface::getDisplayLoadingBoxes())
	{
		m_checkServerObjects->SetVisible(false);
		m_checkFileCaching->SetVisible(false);
		m_checkWorldLoaded->SetVisible(false);
		m_checkTerrainGenerated->SetVisible(false);
		m_checkPlayerReady->SetVisible(false);
	}

	m_bar->SetVisible(true);

	m_textPercentScroll->SetVisible(false);

	m_text->Clear();
	m_textPlanetName->Clear();
	m_textScreenshotName->Clear();
	m_textPercentScroll->Clear();
	m_textLoad->Clear();

	registerMediatorObject (*m_buttonBack, true);
	registerMediatorObject (*m_buttonEsc, true);

	clearAllLoadingChecks();
}

//----------------------------------------------------------------------

SwgCuiLoadingGround::~SwgCuiLoadingGround ()
{
	m_bar  = 0;
	m_text = 0;
	m_textLoad = 0;

	m_flagImage = 0;
	m_planetImage = 0;
	m_tipImage = 0;
}

//----------------------------------------------------------------------

void SwgCuiLoadingGround::performActivate        ()
{
	setPointerInputActive  (true);
	setKeyboardInputActive (true);
	setInputToggleActive   (false);

	getPage ().AddCallback (this);
	UIManager::gUIManager ().DrawCursor (false);

	// - - - - - - - - - - - - - - - - - - - - - - - - - 
	// force ui to render once
	update();
	
	if (!Game::isPlayingCutScene())
	{
		CuiManager::forceRender();
	}

	UIClock::gUIClock ().ListenPerFrame (this);

	GameNetwork::setAcceptSceneCommand (true);

	connectToMessage(CuiLoadingManager::Messages::FullscreenBackButtonEnabled);

	clearAllLoadingChecks();
	//mark that the first loading screen of this loading sequence has definitely been displayed
	CuiLoadingManager::setLoadingScreenVisible(true);
	CuiLoadingManager::setFirstLoadingScreenHasBeenShown();
}

//----------------------------------------------------------------------

void SwgCuiLoadingGround::performDeactivate      ()
{
	setPointerInputActive     (false);
	setKeyboardInputActive    (false);
	setInputToggleActive      (false);

	getPage ().RemoveCallback (this);

	UIClock::gUIClock ().StopListening (this);

	disconnectFromMessage(CuiLoadingManager::Messages::FullscreenBackButtonEnabled);

	clearAllLoadingChecks();
	CuiLoadingManager::setLoadingScreenVisible(false);
}

//----------------------------------------------------------------------

void SwgCuiLoadingGround::OnButtonPressed        (UIWidget *context)
{
	if (context == m_buttonBack)
	{
		deactivate ();
		GameNetwork::setAcceptSceneCommand (false);
		GameNetwork::disconnectConnectionServer ();
		CuiLoadingManager::setFullscreenLoadingEnabled    (false);

		Scene * const scene = Game::getScene ();
		if (scene)
			scene->quit ();		
	}
	else if (context == m_buttonEsc)
	{
		if (Game::isPlayingCutScene())
		{
			Game::skipCutScene();
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiLoadingGround::setupPage(
	const std::string & planetName, 
	const StringId &    textId, 
	const StringId &    titleId, 
	const std::string & picture
)
{
	//make local copies of the data since we may need to override the values
	std::string localPlanetName = planetName;
	StringId    localTextId     = textId;
	StringId    localTitleId    = titleId;
	std::string localPicture    = picture;

	bool generalTip = CuiLoadingManager::getIsGeneralTip();

	if (localTextId.isInvalid() || localTitleId.isInvalid() || localPicture.empty())
	{
		localPlanetName = s_defaultPlanetName;
		localTextId     = s_defaultTextId;
		localTitleId    = s_defaultTitleId;
		localPicture    = s_defaultPicture;
	}

	if (generalTip || planetName == "general")
	{
		//set the generic flag
		IGNORE_RETURN(m_flagImage->SetSourceResource(Unicode::narrowToWide(defaultFlag)));

		//show a generic image instead of the planet
		m_genericImage->SetVisible(true);
		int result = Random::random(0, 1);
		if(result)
			IGNORE_RETURN(m_genericImage->SetSourceResource(Unicode::narrowToWide(genericRebel)));
		else
			IGNORE_RETURN(m_genericImage->SetSourceResource(Unicode::narrowToWide(genericImperial)));
	}
	else
	{
		m_genericImage->SetVisible(false);
		
		//set flag image
		std::string flagImageRes = flagPre + localPlanetName + flagPost1 + localPlanetName + flagPost2;
		if (!TreeFile::exists((texturePre + flagImageRes + texturePost).c_str()))
		{
			flagImageRes = defaultFlag;
		}
		IGNORE_RETURN(m_flagImage->SetSourceResource(Unicode::narrowToWide(flagImageRes)));

		//set planet picture
		std::string planetImageRes = planetPre + localPlanetName + planetPost + localPlanetName;
		if (!TreeFile::exists((texturePre + planetImageRes + texturePost).c_str()))
		{
			planetImageRes = defaultPlanet;
		}
		IGNORE_RETURN(m_planetImage->SetSourceResource(Unicode::narrowToWide(planetImageRes)));
	}

	Unicode::String planetNameWide;
	if (localPlanetName.empty() || localPlanetName == "general")
	{
		planetNameWide = Unicode::narrowToWide("Loading..."); //TODO stringid this
	}
	else
	{
		const StringId s ("planet_n", localPlanetName);
#if PRODUCTION
		if(!s.localize(planetNameWide))
			planetNameWide = Unicode::emptyString;
#else
		planetNameWide = s.localize();
#endif
	}
	//set picture text
	m_textPlanetName->SetText(planetNameWide);

	//set tip picture
	std::string screenshot = localPicture;
	if(!TreeFile::exists((texturePre + localPicture + texturePost).c_str()))
		screenshot = defaultImage;
	IGNORE_RETURN(m_tipImage->SetSourceResource(Unicode::narrowToWide(screenshot)));

	//set tip text
	Unicode::String localText;
#if PRODUCTION
	if(!localTextId.localize(localText))
	{
		localText = Unicode::emptyString;
	}
#else
	localText = localTextId.localize();
#endif
	m_text->SetText(localText);

	//set title text
#if PRODUCTION
	if(!localTitleId.localize(localText))
	{
		localText = Unicode::emptyString;
	}
#else
	localText = localTitleId.localize();
#endif

	m_textScreenshotName->SetText(localText);
}

//----------------------------------------------------------------------

void SwgCuiLoadingGround::update()
{
	if (m_bar)
	{
		const UIWidget * const parent = m_bar->GetParentWidget ();
		if (!parent)
			return;

		const long parentLength = parent->GetWidth ();
		const int percent = CuiLoadingManager::getFullscreenLoadingPercent ();

		if (percent < 0)
		{
			if (!m_wasIdling)
				m_idleValue = 0;

			m_wasIdling = true;

			if (m_idleValue > 48)
				m_idleUp = false;
			else if (m_idleValue <= 0)
				m_idleUp = true;

			if (m_idleUp)
				++m_idleValue;
			else
				--m_idleValue;

			const long barLen = 8L;

			const long slideLen = parentLength - (barLen * 2);

			const long pos = slideLen * m_idleValue / 48;
			m_bar->SetLocation (pos, 0L);
			m_bar->SetWidth (barLen);
		}
		
		else
		{
			m_wasIdling = false;
			m_bar->SetWidth    (parentLength * percent / 100);
			m_bar->SetLocation (UIPoint::zero);
		}
	}

	//update check boxes
	if(m_checkServerObjects)
		m_checkServerObjects->SetChecked(CuiLoadingManager::getServerObjectsReceived());
	if(m_checkFileCaching)
		m_checkFileCaching->SetChecked(CuiLoadingManager::getFileCachingComplete());
	if(m_checkWorldLoaded)
		m_checkWorldLoaded->SetChecked(CuiLoadingManager::getWorldLoaded());
	if(m_checkTerrainGenerated)
		m_checkTerrainGenerated->SetChecked(CuiLoadingManager::getTerrainGenerated());
	if(m_checkPlayerReady)
		m_checkPlayerReady->SetChecked(CuiLoadingManager::getPlayerReady());
}

//----------------------------------------------------------------------

void SwgCuiLoadingGround::Notify( UINotificationServer *, UIBaseObject *, Code )
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

void SwgCuiLoadingGround::receiveMessage(const MessageDispatch::Emitter & /*source*/, const MessageDispatch::MessageBase & message)
{
	if (message.isType (CuiLoadingManager::Messages::FullscreenBackButtonEnabled))
	{
		const MessageDispatch::Message<bool>* const loadingMessage = safe_cast<const MessageDispatch::Message<bool>*> (&message);
		if (loadingMessage)
		{
			//-- always allow back button
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiLoadingGround::clearAllLoadingChecks()
{
	CuiLoadingManager::setFileCachingComplete(false);
	CuiLoadingManager::setPlayerReady(false);
	CuiLoadingManager::setServerObjectsReceived(false);
	CuiLoadingManager::setTerrainGenerated(false);
	CuiLoadingManager::setWorldLoaded(false);

	if(m_checkServerObjects)
		m_checkServerObjects->SetChecked(false);
	if(m_checkFileCaching)
		m_checkFileCaching->SetChecked(false);
	if(m_checkWorldLoaded)
		m_checkWorldLoaded->SetChecked(false);
	if(m_checkTerrainGenerated)
		m_checkTerrainGenerated->SetChecked(false);
	if(m_checkPlayerReady)
		m_checkPlayerReady->SetChecked(false);
}

//======================================================================
