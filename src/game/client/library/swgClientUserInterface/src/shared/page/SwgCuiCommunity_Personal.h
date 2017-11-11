// ======================================================================
//
// SwgCuiCommunity_Personal.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommunity_Personal_H
#define INCLUDED_SwgCuiCommunity_Personal_H

#include "clientUserInterface/CuiMediator.h"
#include "sharedGame/MatchMakingId.h"
#include "swgClientUserInterface/SwgCuiCommunity.h"
#include "UIEventCallback.h"

class CuiMessageBox;
class UIButton;
class UICheckbox;
class UIComboBox;
class UIDataSource;
class UIList;
class UISliderbar;
class UIText;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------
class SwgCuiCommunity::Personal : public CuiMediator
                                , public UIEventCallback
{
public:

	Personal(UIPage &page, SwgCuiCommunity const &communityPage);

	virtual void performActivate();
	virtual void performDeactivate();

	virtual void OnGenericSelectionChanged(UIWidget *context);
	virtual void OnButtonPressed(UIWidget *context);
	virtual void OnSliderbarChanged(UIWidget *context);
	virtual void OnCheckboxSet(UIWidget *context);
	virtual void OnCheckboxUnset(UIWidget *context);

	void         onClearPreferenceList(const CuiMessageBox &messageBox);
	void         onClearProfileList(const CuiMessageBox &messageBox);

private:
	
	void          populateTypeComboBoxFromCategoryComboBox(UIComboBox &typeComboBox, UIComboBox &categoryComboBox, UIList &list) const;
	void          addItemToListFromCategoryAndType(UIList &list, UIComboBox &categoryComboBox, UIComboBox &typeComboBox);
	void          removeAllItemsFromList(UIList &list);
	void          removeSelectedItemsFromList(UIList &list);
	void          setMatchMakingId(UIList const &list, MatchMakingId const &matchMakingId);
	void          synchronizeMatchMakingPersonalId() const;
	void          removeCategory(MatchMakingId &matchMakingId, std::string const &categoryName);
	void          rebuildTypeComboBox(UIComboBox &categoryComboBox);
	void          rebuildList(UIList &list);
	void          highlightItem(UIList &list, std::string const &typeName);

	MatchMakingId const &getMatchMakingId(UIList const &list) const;

	UIButton *    m_preferenceButtonRemove;
	UIButton *    m_preferenceButtonAdd;
	UIButton *    m_preferenceButtonReset;
	UIComboBox *  m_preferenceComboCategory;
	UIComboBox *  m_preferenceComboType;
	UIComboBox *  m_preferenceComboTime;
	UIList *      m_preferenceList;

	UIButton *    m_profileButtonRemove;
	UIButton *    m_profileButtonAdd;
	UIButton *    m_profileButtonReset;
	UIComboBox *  m_profileComboCategory;
	UIComboBox *  m_profileComboType;
	UICheckbox *  m_profileCheckSearchable;
	UIText *      m_profileTextSensitive;
	UISliderbar * m_profileSliderbarSensitive;
	UIList *      m_profileList;
	MatchMakingId m_matchMakingPersonalId;

	SwgCuiCommunity const &     m_communityMediator;
	MessageDispatch::Callback * m_callBack;

	// Disabled

	~Personal();
	Personal(Personal const &rhs);
	Personal &operator =(Personal const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiCommunity_Personal_H
