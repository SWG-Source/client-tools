//===================================================================
//
// SwgCuiHyperspaceMap.cpp
// copyright 2004, sony online entertainment
//
//===================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiHyperspaceMap.h"

#include "UIButton.h"
#include "UIManager.h"
#include "UIPopupMenu.h"
#include "UIText.h"
#include "sharedGame/HyperspaceManager.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientStringIds.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GuildObject.h"
#include "clientGame/PlayerShipController.h"
#include "clientGame/ShipObject.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiSystemMessageManager.h"

#include <vector>

//===================================================================

namespace SwgCuiHyperspaceMapNamespace
{
	UIString formatSystemLocation(HyperspaceManager::HyperspaceLocation const & location)
	{
		UIString result = StringId("planet_n", location.sceneName).localize();
		char buffer[256];
		sprintf (buffer, " (%.0f, %.0f, %.0f)", location.location.x, location.location.y, location.location.z);
		result += Unicode::narrowToWide(buffer);
		return result;
	}

	const char *ORD_MANTELL_ZONE = "space_ord_mantell";

	//
	// *****WARNING WARNING WARNING WARNING WARNING*****
	// this list must be kept in sync in
	// PvpNamespace::loadGcwScoreCategoryTable()
	// SwgCuiSpaceZoneMap::SwgCuiSpaceZoneMap()
	// SwgCuiHyperspaceMap::SwgCuiHyperspaceMap()
	// and gcw.scriptlib (validScenes and defaultRegions)
	// *****WARNING WARNING WARNING WARNING WARNING*****
	//
	typedef std::map<std::string, UIButton *> GalacticGCWButtonMap;
	GalacticGCWButtonMap ms_gcwButtons;

	Unicode::String const ms_gcwRebelIconStyle = Unicode::narrowToWide("/Styles.Icon.map.galactic_gcw_rebel");
	Unicode::String const ms_gcwImperialIconStyle = Unicode::narrowToWide("/Styles.Icon.map.galactic_gcw_imperial");
	Unicode::String const ms_gcwNeutralIconStyle = Unicode::narrowToWide("/Styles.Icon.map.galactic_gcw_neutral");
}

using namespace SwgCuiHyperspaceMapNamespace;

//===================================================================

SwgCuiHyperspaceMap::SwgCuiHyperspaceMap (UIPage& page) :
UIEventCallback(),
CuiMediator("SwgCuiHyperspaceMap",page),
m_hyperspaceButton(NULL),
m_cancelButton(NULL),
m_buttonCorellia(NULL),
m_buttonDantooine(NULL),
m_buttonDathomir(NULL),
m_buttonEndor(NULL),
m_buttonLok(NULL),
m_buttonNaboo(NULL),
m_buttonRori(NULL),
m_buttonTalus(NULL),
m_buttonTatooine(NULL),
m_buttonYavin4(NULL),
m_buttonKashyyyk(NULL),
m_buttonOrdMantell(NULL),
m_hyperspacePointName(NULL),
m_hyperspacePointDescription(NULL),
m_hyperspacePointSystemLocation(NULL),
m_selectedHyperspacePoint()
{
	getCodeDataObject (TUIButton,   m_hyperspaceButton,       "buttonHyperspace");
	getCodeDataObject (TUIButton,   m_cancelButton,           "buttonCancel");
	getCodeDataObject (TUIButton,   m_buttonCorellia,         "buttonCorellia");
	getCodeDataObject (TUIButton,   m_buttonDantooine,        "buttonDantooine");
	getCodeDataObject (TUIButton,   m_buttonDathomir,         "buttonDathomir");
	getCodeDataObject (TUIButton,   m_buttonEndor,            "buttonEndor");
	getCodeDataObject (TUIButton,   m_buttonLok,              "buttonLok");
	getCodeDataObject (TUIButton,   m_buttonNaboo,            "buttonNaboo");
	getCodeDataObject (TUIButton,   m_buttonRori,             "buttonRori");
	getCodeDataObject (TUIButton,   m_buttonTalus,            "buttonTalus");
	getCodeDataObject (TUIButton,   m_buttonTatooine,         "buttonTatooine");
	getCodeDataObject (TUIButton,   m_buttonYavin4,           "buttonYavin4");
	getCodeDataObject (TUIButton,   m_buttonKashyyyk,         "buttonKashyyyk");
	getCodeDataObject (TUIButton,   m_buttonOrdMantell,       "buttonOrdMantell");
	getCodeDataObject (TUIText,     m_hyperspacePointName,           "hyperspacePointName");
	getCodeDataObject (TUIText,     m_hyperspacePointDescription,    "hyperspacePointDescription");
	getCodeDataObject (TUIText,     m_hyperspacePointSystemLocation, "hyperspacePointSystemLocation");

	registerMediatorObject (*m_hyperspaceButton,       true);
	registerMediatorObject (*m_cancelButton,           true);
	registerMediatorObject (*m_buttonCorellia,         true);
	registerMediatorObject (*m_buttonDantooine,        true);
	registerMediatorObject (*m_buttonDathomir,         true);
	registerMediatorObject (*m_buttonEndor,            true);
	registerMediatorObject (*m_buttonLok,              true);
	registerMediatorObject (*m_buttonNaboo,            true);
	registerMediatorObject (*m_buttonRori,             true);
	registerMediatorObject (*m_buttonTalus,            true);
	registerMediatorObject (*m_buttonTatooine,         true);
	registerMediatorObject (*m_buttonYavin4,           true);
	registerMediatorObject (*m_buttonKashyyyk,         true);
	registerMediatorObject (*m_buttonOrdMantell,       true);

	m_hyperspaceButton->SetEnabled(false);
	m_hyperspacePointName->Clear();
	m_hyperspacePointDescription->Clear();
	m_hyperspacePointSystemLocation->Clear();

	setState(MS_closeable);
	setState(MS_closeDeactivates);

	//
	// *****WARNING WARNING WARNING WARNING WARNING*****
	// this list must be kept in sync in
	// PvpNamespace::loadGcwScoreCategoryTable()
	// SwgCuiSpaceZoneMap::SwgCuiSpaceZoneMap()
	// SwgCuiHyperspaceMap::SwgCuiHyperspaceMap()
	// and gcw.scriptlib (validScenes and defaultRegions)
	// *****WARNING WARNING WARNING WARNING WARNING*****
	//
	{
		ms_gcwButtons.clear();

		UIButton * gcwButton = NULL;
		getCodeDataObject(TUIButton, gcwButton, "gcwTatooine");
		if (!gcwButton)
			DEBUG_WARNING(true, ("Failed to find Galactic Map GCW Button for space zone [space_tatooine] using codeData value [gcwTatooine]"));
		else
			ms_gcwButtons.insert(std::make_pair("gcw_region_tatooine_12", gcwButton));

		gcwButton = NULL;
		getCodeDataObject(TUIButton, gcwButton, "gcwCorellia");
		if (!gcwButton)
			DEBUG_WARNING(true, ("Failed to find Galactic Map GCW Button for space zone [space_corellia] using codeData value [gcwCorellia]"));
		else
			ms_gcwButtons.insert(std::make_pair("gcw_region_corellia_14", gcwButton));

		gcwButton = NULL;
		getCodeDataObject(TUIButton, gcwButton, "gcwDantooine");
		if (!gcwButton)
			DEBUG_WARNING(true, ("Failed to find Galactic Map GCW Button for space zone [space_dantooine] using codeData value [gcwDantooine]"));
		else
			ms_gcwButtons.insert(std::make_pair("gcw_region_dantooine_17", gcwButton));

		gcwButton = NULL;
		getCodeDataObject(TUIButton, gcwButton, "gcwDathomir");
		if (!gcwButton)
			DEBUG_WARNING(true, ("Failed to find Galactic Map GCW Button for space zone [space_dathomir] using codeData value [gcwDathomir]"));
		else
			ms_gcwButtons.insert(std::make_pair("gcw_region_dathomir_12", gcwButton));

		gcwButton = NULL;
		getCodeDataObject(TUIButton, gcwButton, "gcwEndor");
		if (!gcwButton)
			DEBUG_WARNING(true, ("Failed to find Galactic Map GCW Button for space zone [space_endor] using codeData value [gcwEndor]"));
		else
			ms_gcwButtons.insert(std::make_pair("gcw_region_endor_16", gcwButton));

		gcwButton = NULL;
		getCodeDataObject(TUIButton, gcwButton, "gcwLok");
		if (!gcwButton)
			DEBUG_WARNING(true, ("Failed to find Galactic Map GCW Button for space zone [space_lok] using codeData value [gcwLok]"));
		else
			ms_gcwButtons.insert(std::make_pair("gcw_region_lok_14", gcwButton));

		gcwButton = NULL;
		getCodeDataObject(TUIButton, gcwButton, "gcwNaboo");
		if (!gcwButton)
			DEBUG_WARNING(true, ("Failed to find Galactic Map GCW Button for space zone [space_naboo] using codeData value [gcwNaboo]"));
		else
			ms_gcwButtons.insert(std::make_pair("gcw_region_naboo_14", gcwButton));

		gcwButton = NULL;
		getCodeDataObject(TUIButton, gcwButton, "gcwYavin4");
		if (!gcwButton)
			DEBUG_WARNING(true, ("Failed to find Galactic Map GCW Button for space zone [space_yavin4] using codeData value [gcwYavin4]"));
		else
			ms_gcwButtons.insert(std::make_pair("gcw_region_yavin4_18", gcwButton));
	}
}

//----------------------------------------------------------------------

SwgCuiHyperspaceMap::~SwgCuiHyperspaceMap ()
{
	m_hyperspaceButton = NULL;
	m_cancelButton = NULL;
	m_buttonCorellia = NULL;
	m_buttonDantooine = NULL;
	m_buttonDathomir = NULL;
	m_buttonEndor = NULL;
	m_buttonLok = NULL;
	m_buttonNaboo = NULL;
	m_buttonRori = NULL;
	m_buttonTalus = NULL;
	m_buttonTatooine = NULL;
	m_buttonYavin4 = NULL;
	m_buttonKashyyyk = NULL;
	m_buttonOrdMantell = NULL;
	m_hyperspacePointName = NULL;
	m_hyperspacePointDescription = NULL;
	m_hyperspacePointSystemLocation = NULL;
}

//-------------------------------------------------------------------

void SwgCuiHyperspaceMap::performActivate()
{	
	CuiMediator::performActivate();
	CuiManager::requestPointer(true);
	bool isTutorialZone = (_stricmp(Game::getSceneId().c_str(), ORD_MANTELL_ZONE) == 0);
	if (isTutorialZone)
	{			
		CuiSystemMessageManager::sendFakeSystemMessage(ClientStringIds::no_hyperspace_in_tutorial_zone.localize());
		deactivate();
	}
	else
	{
		// update GCW score icons
		GuildObject const * const go = GuildObject::getGuildObject();
		if (go)
		{
			std::map<std::string, int> const & gcwScoreMap = go->getGcwImperialScorePercentileThisGalaxy();
			std::map<std::string, int>::const_iterator iterFindScore;
			for (GalacticGCWButtonMap::const_iterator iter = ms_gcwButtons.begin(); iter != ms_gcwButtons.end(); ++iter)
			{
				iterFindScore = gcwScoreMap.find(iter->first);
				if (iterFindScore != gcwScoreMap.end())
				{
					iter->second->SetVisible(true);

					if (iterFindScore->second > 50)
						iter->second->SetProperty(UIButton::PropertyName::Icon, ms_gcwImperialIconStyle);
					else if (iterFindScore->second < 50)
						iter->second->SetProperty(UIButton::PropertyName::Icon, ms_gcwRebelIconStyle);
					else
						iter->second->SetProperty(UIButton::PropertyName::Icon, ms_gcwNeutralIconStyle);
				}
				else
				{
					DEBUG_WARNING(true, ("HyperspaceMap - Could not find GCW score for [%s]", iter->first.c_str()));
					iter->second->SetVisible(false);
				}
			}
		}
	}
}

//-------------------------------------------------------------------

void SwgCuiHyperspaceMap::performDeactivate()
{
	CuiMediator::performDeactivate();
	CuiManager::requestPointer(false);
}

//-------------------------------------------------------------------

void SwgCuiHyperspaceMap::OnButtonPressed (UIWidget* const context)
{
	if(context == m_buttonCorellia)
	{
		createContextMenu("space_corellia");
	}
	else if(context == m_buttonDantooine)
	{
		createContextMenu("space_dantooine");
	}
	else if(context == m_buttonDathomir)
	{
		createContextMenu("space_dathomir");
	}
	else if(context == m_buttonEndor)
	{
		createContextMenu("space_endor");
	}
	else if(context == m_buttonLok)
	{
		createContextMenu("space_lok");
	}
	else if(context == m_buttonNaboo)
	{
		createContextMenu("space_naboo");
	}
	else if(context == m_buttonRori)
	{
		createContextMenu("space_rori");
	}
	else if(context == m_buttonTalus)
	{
		createContextMenu("space_talus");
	}
	else if(context == m_buttonTatooine)
	{
		createContextMenu("space_tatooine");
	}
	else if(context == m_buttonYavin4)
	{
		createContextMenu("space_yavin4");
	}
	else if(context == m_buttonKashyyyk)
	{
		createContextMenu("space_kashyyyk");
	}
	else if(context == m_buttonOrdMantell)
	{
		createContextMenu("space_nova_orion");
	}
	else if (context == m_hyperspaceButton)
	{
		if(!m_selectedHyperspacePoint.empty())
		{
			IGNORE_RETURN(ClientCommandQueue::enqueueCommand("hyperspace", NetworkId::cms_invalid, Unicode::narrowToWide(m_selectedHyperspacePoint)));

			closeThroughWorkspace();
		}
	}
	else if(context == m_cancelButton)
	{
		closeThroughWorkspace();
	}
}

//----------------------------------------------------------------------

void SwgCuiHyperspaceMap::createContextMenu (std::string const & starSystem)
{
	CreatureObject const * const player = Game::getPlayerCreature();
	if(!player)
		return;

	UIPopupMenu * const pop = new UIPopupMenu(&getPage());
	pop->SetStyle (getPage ().FindPopupStyle ());

	std::vector<HyperspaceManager::HyperspaceLocation> locations;
	bool const result = HyperspaceManager::getHyperspacePoints(starSystem, locations);
	if(result)
	{
		for(std::vector<HyperspaceManager::HyperspaceLocation>::const_iterator i = locations.begin(); i != locations.end(); ++i)
		{
			if(i->requiredCommand.empty() || player->hasCommand(i->requiredCommand))
				pop->AddItem(i->name, StringId("hyperspace_points_n", i->name).localize());
		}
		pop->SetVisible  (true);
		pop->AddCallback (this);
		pop->SetLocation (UIManager::gUIManager().GetLastMouseCoord());
		UIManager::gUIManager().PushContextWidget(*pop);
	}
} //lint !e429 pop not freed (UIManager gains ownership with PushContextWidget call)

//----------------------------------------------------------------------

void SwgCuiHyperspaceMap::OnPopupMenuSelection (UIWidget * context)
{
	UIPopupMenu * const pop = dynamic_cast<UIPopupMenu *>(context);
	if(pop)
	{
		m_selectedHyperspacePoint = pop->GetSelectedName ();
		if(!m_selectedHyperspacePoint.empty())
		{
			HyperspaceManager::HyperspaceLocation location;
			bool const result = HyperspaceManager::getHyperspacePoint(m_selectedHyperspacePoint, location);
			if(result)
			{
				m_hyperspacePointName->SetText(StringId("hyperspace_points_n", location.name).localize());
				m_hyperspacePointDescription->SetText(StringId("hyperspace_points_d", location.name).localize());
				m_hyperspacePointName->SetText(formatSystemLocation(location));
				m_hyperspaceButton->SetEnabled(true);
			}
		}
	}
}

//===================================================================

