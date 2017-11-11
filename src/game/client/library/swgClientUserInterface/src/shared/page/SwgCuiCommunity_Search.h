// ======================================================================
//
// SwgCuiCommunity_Search.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommunity_Search_H
#define INCLUDED_SwgCuiCommunity_Search_H

#include "clientGame/PlayerCreatureController.h"
#include "clientUserInterface/CuiMediator.h"
#include "swgClientUserInterface/SwgCuiCommunity.h"
#include "UIEventCallback.h"

class UIButton;
class UIText;
class UITextbox;
class SwgCuiLfg;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiCommunity::Search : public CuiMediator, public UIEventCallback
{
public:

	Search(UIPage &page, SwgCuiCommunity const &communityMediator);

	virtual void performActivate();
	virtual void performDeactivate();

	virtual void OnButtonPressed(UIWidget *context);
	virtual void OnTextboxChanged(UIWidget *context);
	virtual void OnGenericSelectionChanged(UIWidget * context);

	virtual void saveSettings() const;
	virtual void loadSettings();

	void onCharacterMatchRetrieved(PlayerCreatureController::Messages::CharacterMatchRetrieved::MatchResults const &matchResults);

private:
	
	void sendTellToSelection() const;
	void sendInviteToSelection() const;
	void updateButtonStates(); 

	SwgCuiCommunity const &          m_communityMediator;
	SwgCuiLfg *						 m_Lfg;
	UITable *                        m_table;
	UIButton *                       m_searchButton;
	UIButton *                       m_buttonTell;
	UIButton *                       m_buttonInvite;
	UIButton *                       m_buttonTellAndInvite;
	UITextbox *                      m_tellTextbox;
	UIText *                         m_bioTextbox;
	UIText *                         m_tooManyResultsText;

	MessageDispatch::Callback *      m_callBack;

	// Disabled
 	~Search();
	Search(Search const &rhs);
	Search &operator =(Search const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiCommunity_Search_H
