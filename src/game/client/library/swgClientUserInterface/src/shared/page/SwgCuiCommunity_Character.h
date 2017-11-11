// ======================================================================
//
// SwgCuiCommunity_Character.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommunity_Character_H
#define INCLUDED_SwgCuiCommunity_Character_H

#include "clientGame/PlayerCreatureController.h"
#include "clientGame/PlayerObject.h"
#include "clientUserInterface/CuiMediator.h"
#include "sharedGame/MatchMakingId.h"
#include "sharedMessageDispatch/Receiver.h"
#include "swgClientUserInterface/SwgCuiCommunity.h"
#include "UIEventCallback.h"

class SwgCuiLfg;
class UIButton;
class UICheckbox;
class UIComboBox;
class UIList;
class UIText;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiCommunity::Character : public CuiMediator
                                 , public UIEventCallback
{
public:

	Character(UIPage &page, SwgCuiCommunity const &communityMediator);

	virtual void performActivate();
	virtual void performDeactivate();

	virtual void update(float deltaTimeSecs);
	virtual void OnCheckboxSet(UIWidget *context);
	virtual void OnCheckboxUnset(UIWidget *context);
	virtual void OnGenericSelectionChanged(UIWidget *context);

	void onBiographyRetrieved(PlayerCreatureController::Messages::BiographyRetrieved::BiographyOwner const &biographyOwner);
	void onWhoStatusChanged(MatchMakingId const &matchMakingId);
	void onSpokenLanguageChanged(PlayerObject::Messages::SpokenLangaugeChanged::Language const &language);

private:
	
	void populateLanguages();
	void setComboTitleToCurrentTitle();

	SwgCuiCommunity const &    m_communityMediator;
	SwgCuiLfg *				   m_Lfg;
	UIText *                   m_nameText;
	UIText *                   m_birthDateText;
	UIText *                   m_speciesText;
	UIText *                   m_bioText;
	UICheckbox *               m_lookingForGroupCheckBox;
	UICheckbox *               m_searchableByCtsSourceGalaxyCheckBox;
	UICheckbox *               m_displayLocationInSearchResultsCheckBox;
	UICheckbox *               m_characterSearchableCheckBox;
	UICheckbox *               m_helperCheckBox;
	UICheckbox *               m_rolePlayCheckBox;
	UICheckbox *			   m_lookingForWorkCheckBox;
	UICheckbox *               m_awayFromKeyBoardCheckBox;
	UICheckbox *               m_displayingFactionRankCheckBox;
	UIComboBox *               m_titleComboBox;
	UIComboBox *               m_languageComboBox;
	MessageDispatch::Callback *m_callBack;
	Unicode::String            m_biography;
	std::string                m_currentTitle;

	// Disabled

	~Character();
	Character(Character const &rhs);
	Character &operator =(Character const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiCommunity_Character_H
