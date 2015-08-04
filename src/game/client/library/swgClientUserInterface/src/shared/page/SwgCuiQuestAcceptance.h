//======================================================================
//
// SwgCuiQuestAcceptance.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiQuestAcceptance_H
#define INCLUDED_SwgCuiQuestAcceptance_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

//----------------------------------------------------------------------

class CuiWidget3dObjectListViewer;
class Quest;
class UIButton;
class UIText;

//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiQuestAcceptance : public CuiMediator, public UIEventCallback
{
public:
	enum WindowType
	{
		WT_requestAccept,
		WT_requestComplete,
		WT_forceAccept
	};

public:
	SwgCuiQuestAcceptance                            (UIPage & page);
	~SwgCuiQuestAcceptance                           ();
	void setData                                     (WindowType type, uint32 questCrcString);
	virtual void             OnButtonPressed         (UIWidget * context);
	void                     update                  (float deltaTimeSecs);
	void                     onAttributesChanged     (const std::string & staticItemName);

protected:
	virtual void             performActivate         ();
	virtual void             performDeactivate       ();

private:
	//disabled
	                         SwgCuiQuestAcceptance   ();
	                         SwgCuiQuestAcceptance   (SwgCuiQuestAcceptance const & rhs);
	SwgCuiQuestAcceptance &  operator=               (SwgCuiQuestAcceptance const & rhs);

private:
	void                     updateDecriptionReward  (Quest const & quest);
	std::string const &      getRewardSelection      () const;

private:
	MessageDispatch::Callback * m_callback;
	UIText *                    m_titleText;
	UIText *                    m_descriptionText;
	UIButton *                  m_acceptButton;
	UIButton *                  m_okButton;
	UIButton *                  m_completeButton;
	UIButton *                  m_cancelButton;
	UIText *                    m_levelText;
	UIText *                    m_typeText;
	UIPage *                    m_typePage;
	UIText *                    m_windowCaptionText;
	uint32                      m_questCrc;
	UIText *                    m_experienceRewardText;
	UIText *                    m_factionRewardText;
	UIText *                    m_moneyRewardText;
	UIText *                    m_rewardLabel;
	UIText *                    m_inclusiveText;
	UIText *                    m_exclusiveText;
	UIText *                    m_chooseOneText;
	UIPage *                    m_rewardItemPages[10];
	CuiWidget3dObjectListViewer * m_rewardItemViewers[10];
	UIText *                      m_rewardItemNames[10];
	UIText *                      m_rewardItemTexts[10];
	UIPage *                      m_rewardItemSelectionBoxes[10];
	UIButton *                    m_rewardItemSelectionButtons[10];
	UIPage *                      m_completionPanel;
};

// ======================================================================

#endif

