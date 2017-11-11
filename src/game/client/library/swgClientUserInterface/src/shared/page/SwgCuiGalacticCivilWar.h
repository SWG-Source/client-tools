//======================================================================
//
// SwgCuiGalacticCivilWar.h
// copyright (c) 2009 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiGalacticCivilWar_H
#define INCLUDED_SwgCuiGalacticCivilWar_H

//======================================================================
#include "clientGame/GuildObject.h"
#include "clientGame/PlanetMapManagerClient.h"
#include "clientUserInterface/CuiMediator.h"

#include "UIEventCallback.h"
//----------------------------------------------------------------------

class UIPage;
class UIText;
class UIButton;
class UIColorEffector;
class UIComboBox;
class UIComposite;
class UITabbedPane;
class UITreeView;

#include <vector>
#include <map>
#include <string>
//----------------------------------------------------------------------
namespace MessageDispatch
{
	class Callback;
}

class SwgCuiGalacticCivilWar: public CuiMediator, public UIEventCallback
{
public:

	class Messages
	{
	public:
		struct GcwEventLocationsReceived
		{
			typedef std::pair<bool, std::pair<bool, bool> > Payload;
		};

		struct GcwEventLocationsRequested
		{
			typedef bool Payload;
		};
	};

	explicit SwgCuiGalacticCivilWar(UIPage & page);
	~SwgCuiGalacticCivilWar();

	virtual void update(float delta);
	virtual void OnTabbedPaneChanged(UIWidget * Context);
	virtual void OnButtonPressed(UIWidget * Context );
	virtual void OnGenericSelectionChanged (UIWidget * context);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

	void         onGCWScoreUpdatedThisGalaxy(GuildObject::Messages::GCWScoreUpdatedThisGalaxy::Payload const & obj);
	void         onGCWGroupScoreUpdatedThisGalaxy(GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy::Payload const & obj);

	void		 onGCWScoreUpdatedOtherGalaxies(GuildObject::Messages::GCWScoreUpdatedOtherGalaxies::Payload const & obj);
	void         onGCWGroupScoreUpdatedOtherGalaxies(GuildObject::Messages::GCWGroupScoreUpdatedOtherGalaxies::Payload const & obj);

	void         onGCWRegionEntriesReceived(PlanetMapManagerClient::Messages::GcwEventLocationsReceived::Payload const & obj);

private:
	//disabled
	SwgCuiGalacticCivilWar(const SwgCuiGalacticCivilWar & rhs);
	SwgCuiGalacticCivilWar & operator= (const SwgCuiGalacticCivilWar & rhs);

private:
	void updateGalaxyWideScore();
	void updateServerSpecificScore();
	void updateGalaxyMapIcons();
	void updateHistoryGraph();
	void updateGalaxyScoreDropDownList();

	void AdvanceHistoryGraph();
	void ResetHistoryGraph();

	void updateMainPage();
	void updatePlanetRegions(std::string & planetName);
	void updateCategoryTree(std::string & rootCategory);

	void populateServerComboBox();

	UIPage * m_galaxyMap;
	UIPage * m_planetMap;
	UIPage * m_planetNamePage;

	UIPage * m_galaxyWideRebelBar;
	UIPage * m_galaxyWideImperialBar;

	UIText * m_galaxyWideRebelText;
	UIText * m_galaxyWideImperialText;

	UIComboBox * m_serverComboBox;

	UIPage * m_serverRebelBar;
	UIPage * m_serverImperialBar;

	UIText * m_serverRebelText;
	UIText * m_serverImperialText;

	UIText * m_serverNameText;

	UIButton * m_galaxyWideWinnerIcon;
	UIButton * m_serverWinnerIcon;

	UIComposite * m_planetTabComposite;
	UITabbedPane * m_planetTabs;
	UIPage * m_mapCatalog;

	UIButton * m_gcwNeutralMarker;

	UIButton * m_closeButton;

	typedef std::vector<UIButton *> ButtonVector;
	typedef std::vector<UIText *> TextVector;
	ButtonVector m_regionButtons;
	TextVector   m_regionTextLabels;

	UIPage * m_markerPage;

	UITreeView * m_categoryTree;

	UIText * m_sampleText;
	UIButton * m_refreshButton;

	UIColorEffector * m_effector;

	MessageDispatch::Callback * m_callback;

	std::map<std::string, std::pair<UIButton *, UIButton *> > m_galaxyButtons;

	std::vector<std::pair<UIPage *, UIPage *> > m_historyBars;

	float m_historyUpdateTimer;
	std::string m_currentHistoryCategory;
	std::string m_lastSelectedCategory;
	bool        m_lastSelectionIsGroupCategory;

	UIText * m_historyLabel;

	UIImageStyle * m_regionNeutralIconStyle;
	UIImageStyle * m_regionRebelIconStyle;
	UIImageStyle * m_regionImperialIconStyle;
};

//======================================================================

#endif