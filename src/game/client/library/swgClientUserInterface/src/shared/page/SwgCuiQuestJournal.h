//======================================================================
//
// SwgCuiQuestJournal.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiQuestJournal_H
#define INCLUDED_SwgCuiQuestJournal_H

//======================================================================

#include "clientGame/ClientObject.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class ClientObject;
class CreatureObject;
class CuiMessageBox;
class CuiWidget3dObjectListViewer;
class PlayerObject;
class Quest;
class UICheckbox;
class UIComboBox;
class UIComposite;
class UIText;
class UITreeView;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

/**
 * Implements a quest journal UI that allows the player to interact and abandon quests.
 */
class SwgCuiQuestJournal : public CuiMediator, public UIEventCallback
{
public:

	/**
	 * Used to provide sort selection types for the visible quests
	 */
	enum SortType
	{
		ST_category,
		ST_level,
		ST_type
	};

public:
	static std::string       getQuestColor(uint32 questCrc);

public:
	SwgCuiQuestJournal       (UIPage & page);
	~SwgCuiQuestJournal      ();

	void                     onPlayerSetup            (const CreatureObject & creature);
	void                     onActiveQuestsChanged    (const PlayerObject & player);
	void                     onCompletedQuestsChanged (const PlayerObject & player);
	void                     onQuestsChanged          (const PlayerObject & player);
	void                     onContainerChange        (ClientObject::Messages::ContainerMsg const & msg);
	void                     onCountersChanged        (const bool & payload);
	void                     onTimersChanged          (const bool & payload);
	void                     onQuestAbandonResponse   (const CuiMessageBox & messageBox);

	virtual void             OnGenericSelectionChanged (UIWidget * context);
	virtual void             OnButtonPressed           (UIWidget * context);
	virtual void             OnCheckboxSet             (UIWidget *Context);
	virtual void             OnCheckboxUnset           (UIWidget *Context);
	virtual void             OnTreeRowExpansionToggled (UIWidget * context, int row);

	void                     saveSettings       () const;
	void                     loadSettings       ();

	void                     update             (float deltaTimeSecs);

protected:
	virtual void             performActivate   ();
	virtual void             performDeactivate ();

private:
	//disabled
	                         SwgCuiQuestJournal  ();
	                         SwgCuiQuestJournal  (const SwgCuiQuestJournal & rhs);
	SwgCuiQuestJournal &     operator=           (const SwgCuiQuestJournal & rhs);

	void                     updateAll ();
	void                     updateSelection ();
	void                     updateDescription();
	void                     updateDecriptionTasks(Quest const & quest);
	void                     updateDecriptionReward(Quest const & quest);
	void                     updateQuestTree ();
	std::string const        getSelectedQuest () const;
	uint32 const             getSelectedQuestCrc() const;
	std::string const        getQuestAtRow(int const row) const;
	bool                     isSelectedQuestAMission() const;
	NetworkId                getSelectedMission() const;
	void                     deleteTaskData();
	void                     updateTimers(float deltaTimeSecs);
	void                     clearTaskPages();
	void                     expandAllCategories(bool isExpanded);
	void                     storeRowExpanded(bool isExpanded, int row);
	void                     storeRowExpanded(bool isExpanded, std::string const & rowName);
	void                     updateQuestDescription();
	void                     clearQuestDescription();
	void                     updateMissionDescription();
	void                     updateSpecificMissionDescriptionData(std::string const & missionType);
	void                     requestOpenCompletionWindow(uint32 questCrc);
	bool                     selectedQuestCanClaimReward() const;

	NetworkId				 getSelectedPlayerQuest() const;
	void				     updatePlayerQuestDescription();
	void					 clearPlayerQuestTasks();

	MessageDispatch::Callback * m_callback;
	UICheckbox *                m_showCompletedCheckbox;
	UIText *                    m_description;
	UIText *                    m_titleText;
	UIText *                    m_experienceRewardText;
	UIText *                    m_factionRewardText;
	UIText *                    m_moneyRewardText;
	UIText *                    m_rewardLabel;
	UITreeView *                m_questTree;
	UIButton *                  m_okButton;
	boolean                     m_firstTime;
	UIComposite *               m_descriptionComp;
	UIButton *                  m_buttonAbandonQuest;
	UIComposite *               m_tasksSample;
	UIText *                    m_descLabel;
	UIComboBox *                m_sortCombo;
	SortType                    m_sortType;
	UIText *                    m_levelText;
	UIText *                    m_levelLabel;
	UIText *                    m_postTaskSpacer;
	UIText *                    m_buttonExpandAll;
	UIPage *                    m_typePage;
	UIText *                    m_typeText;
	UIText *                    m_inclusiveText;
	UIText *                    m_exclusiveText;
	UIText *                    m_chooseOneText;
	UIPage *                    m_rewardItemPages[10];
	CuiWidget3dObjectListViewer * m_rewardItemViewers[10];
	UIText *                      m_rewardItemNames[10];
	UIText *                      m_rewardItemTexts[10];
	UIPage *                      m_rewardItemSelectionBoxes[10];
	UIButton *                    m_abandonMissionButton;
	UIPage *                      m_questDescriptionPage;
	UIPage *                      m_missionDescriptionPage;

	UIText *                      m_missionTitleText;
	UIText *                      m_missionDescriptionText;
	UIText *                      m_missionMoneyRewardText;
	UIComposite *                 m_missionDescriptionComp;
	UIText *                      m_missionLevelText;
	UIText *                      m_missionTypeText;
	UIText *                      m_missionCreatorText;
	UIText *                      m_missionTargetText;
	UIText *                      m_missionStartLocText;
	UIText *                      m_missionDestLocText;
	UIText *                      m_missionEfficiencyText;

	UIText *					  m_playerQuestTitleText;
	UIText *					  m_playerQuestDescriptionText;
	UIComposite *				  m_playerQuestDescriptionComp;
	UIText *					  m_playerQuestCreatorText;
	UIPage *					  m_playerQuestDescriptionPage;
	UIPage *					  m_playerQuestTaskSample;
	UIComposite *				  m_playerQuestTaskComposite;
	UIText *					  m_playerQuestDifficultyText;
};

// ======================================================================

#endif

