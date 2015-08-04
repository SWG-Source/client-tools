// ======================================================================
//
// SwgCuiCommunity.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommunity_H
#define INCLUDED_SwgCuiCommunity_H

#include "clientUserInterface/CuiMediator.h"
#include "sharedSkillSystem/SkillObject.h"
#include "UIEventCallback.h"

class CreatureObject;
class MatchMakingId;
class PlayerCreatureController;
class PlayerObject;
class UIButton;
class UIComboBox;
class UIDataSource;
class UIList;
class UITabbedPane;
class UITable;

//-----------------------------------------------------------------

class SwgCuiCommunity : public CuiMediator, public UIEventCallback
{
public:

	typedef stdmap<Unicode::String, std::string>::fwd UnicodeStringToStringIdMap;
	typedef stdvector<std::string>::fwd               StringVector;
	typedef stdvector<Unicode::String>::fwd           UnicodeStringVector;

	explicit SwgCuiCommunity(UIPage & page);

	virtual void              performActivate();
	virtual void              performDeactivate();

	static SwgCuiCommunity *createInto(UIPage *parent);

	virtual void OnTabbedPaneChanged(UIWidget *context);

	int          getMatchMakingTypeIndex(std::string const &type) const;
	void         getProfessionTitles(SkillObject::SkillVector const &skillBoxes, UnicodeStringToStringIdMap &listItems, bool onlySearchableTitles) const;

	void         removeSelectedItemsFromList(UIList &list, StringVector &names) const;
	void         removeSelectedItemsFromList(UIList &list, UnicodeStringVector &labels) const;
	void         addItemToSortedList(UIList &list, Unicode::String const &localizedString, std::string const &nameString) const;
	void         addItem(UIDataSource &dataSource, Unicode::String const &localText, std::string const &name) const;

	PlayerCreatureController *getLocalPlayerCreatureController() const;
	CreatureObject *          getLocalCreatureObject() const;

private:

	virtual void OnButtonPressed(UIWidget *context);

	class Personal;
	class History;
	class Character;
	class Search;
	class Friend;
	class Ignore;

	enum Mediator
	{
		M_personal,
		M_history,
		M_character,
		M_search,
		M_friend,
		M_ignore,
		M_count
	};

	CuiMediator * m_mediators[M_count];
	UIButton *    m_doneButton;
	Personal *    m_personalMediator;
	History *     m_historyMediator;
	Character *   m_characterMediator;
	Search *      m_searchMediator;
	Friend *      m_friendMediator;
	Ignore *      m_ignoreMediator;
	UITabbedPane *m_tabs;

	typedef stdmap<std::string, int>::fwd StringToIntMap;

	StringToIntMap *m_matchMakingTypeIndexes;

	// Disabled

	~SwgCuiCommunity();
	SwgCuiCommunity(SwgCuiCommunity const &rhs);
	SwgCuiCommunity &operator =(SwgCuiCommunity const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiCommunity_H
