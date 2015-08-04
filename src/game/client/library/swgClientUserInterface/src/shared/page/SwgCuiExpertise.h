//======================================================================
//
// SwgCuiExpertise.h
// copyright (c) 2006 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiExpertise_H
#define INCLUDED_SwgCuiExpertise_H

//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiExpertise.h"

#include "clientUserInterface/CuiMediator.h"
#include "sharedMessageDispatch/Receiver.h"
#include "UIEventCallback.h"

class CreatureObject;
class CuiMessageBox;
class PlayerObject;
class SkillObject;
class UIImage;
class UIText;

namespace MessageDispatch
{
	class Emitter;
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiExpertise :
public CuiMediator,
public UIEventCallback,
public MessageDispatch::Receiver
{

public:
	static const int MAX_NUM_EXPERTISE_RANKS = 4;
	static const int MAX_NUM_SKILL_MODS_PER_EXPERTISE = 4;

	explicit            SwgCuiExpertise(UIPage & page);

	virtual bool        OnMessage                    (UIWidget * context, const UIMessage & msg);
	void                OnButtonPressed              (UIWidget * context);
	virtual void        OnTabbedPaneChanged          (UIWidget * context);
	virtual void        receiveMessage               (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);	
			void        update                       (float deltaTimeSecs);

	        void        onLevelChanged               (const CreatureObject & creature);
			void        onSkillsChanged              (const CreatureObject & creature);
			void        onSkillTemplateChanged       (const std::string & creature);

protected:
	
	virtual void        performActivate              ();
	virtual void        performDeactivate            ();
	
private:
	                    SwgCuiExpertise          ();
	                    SwgCuiExpertise          (const SwgCuiExpertise &);
	SwgCuiExpertise &operator=                   (const SwgCuiExpertise &);

	                   ~SwgCuiExpertise          ();

	        void        displayExpertisePoints   ();
	        void        displayExpertiseTabs     ();
	        void        displayExpertiseTree     ();
	        void        displayExpertiseTreeIcon      (UIPage * box, SkillObject const * skill, bool useBright);
	        void        displayExpertiseTreeComponent (UIPage * box, std::string const & path,  bool useBright);
			void		updateExpertiseTreeComponentHighlight (UIPage * box, std::string const & path,  bool useBright);
			void        updateExpertiseIconHighlighting();

	        void        onSkillIconClicked       (UIWidget * context, bool isRightMouseButton);

			void		displaySkillSelectBorder (UIWidget * context, bool display);
			void		updateExpertiseDisplayName();

			void        updateExpertiseDescriptionSection();
			void        updateExpertiseRequirementsSection();

			void        updateButtonStatus();
			void		onMessageBoxClosed(const CuiMessageBox & box);

	UITabbedPane *               m_treeTabs;
	long                         m_currentTab;

	UIText *                     m_spentPointsText;
	UIText *                     m_allocatedPointsText;

	UIButton *                   m_acceptButton;
	UIButton *                   m_clearTreeButton;
	UIButton *                   m_clearAllButton;
	UIButton *                   m_switchNumbersButton;

	UIPage *                     m_treeGrid;
	UIPage *                     m_sampleIconPage;
	UIPage *                     m_sampleComponentPage;

	MessageDispatch::Callback *	 m_callback;

	UIWidget *                   m_lastExpertiseClickedOn;   // Saves widget that has border displayed

	UIText *                     m_expertiseDisplayName;     // The widget that shows the name of the expertise
	UIText *                     m_expertiseDisplayDescription;   // The widget that shows the description of the expertise
	UIText *                     m_expertiseDisplayRequirements; // The widget that shows what the expertise requires

	std::string                  m_currentSkillName;         // The skill name associated with the current context expertise 
	std::string                  m_currentBaseSkillName;     // The base version of that skill

	UIText *                     m_currentRankText;

	UIImage *                    m_mainIconImage;            // The icon that sits next to the main title in the upper right

	//Only one of these 3 will be visible at once
	UIPage *                     m_commandType;              // Description section for a command-based expertise
	UIImage *                    m_commandTypeIcon;
	UIText *                     m_commandTypeName;
	UIText *                     m_commandTypeDescription;

	UIPage *                     m_schematicType;            // Description section for a schematic-based expertise
	UIPage *                     m_skillModType;             // Description section for a skill-mod-based expertise
	UIPage *                     m_skillModTypeMain[MAX_NUM_SKILL_MODS_PER_EXPERTISE];
	UIText *                     m_skillModTypeNames[MAX_NUM_SKILL_MODS_PER_EXPERTISE];     
	UIText *                     m_skillModTypeDescriptions[MAX_NUM_SKILL_MODS_PER_EXPERTISE];
	UIText *                     m_skillModTypeAmounts[MAX_NUM_SKILL_MODS_PER_EXPERTISE][MAX_NUM_EXPERTISE_RANKS];
	UIPage *                     m_skillModTypeMarkers[MAX_NUM_SKILL_MODS_PER_EXPERTISE][MAX_NUM_EXPERTISE_RANKS];

	int m_numberMode;

	CuiMessageBox *              m_messageBoxTrainExpertises;

	UIImage *                    m_treeBackgroundImage;

};
//======================================================================

#endif
