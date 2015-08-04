//======================================================================
//
// SwgCuiRoadmap.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiRoadmap_H
#define INCLUDED_SwgCuiRoadmap_H

//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiRoadmap.h"

#include "clientAudio/SoundId.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedObject/CachedNetworkId.h"
#include "UITypes.h"
#include "UILowerString.h"

class ClientNotificationBoxMessage;
class CreatureObject;
class CuiWidget3dObjectListViewer;
class Object;
class PlayerObject;
class UIImage;
class UIPie;
class UITable;
class UITableModelDefault;
class UIText;

namespace MessageDispatch
{
	class Emitter;
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiRoadmap_Profession;

class SwgCuiRoadmap :
public CuiMediator,
public UIEventCallback,
public MessageDispatch::Receiver
{

public:

	enum TabType
	{
			TT_GROUND = 0,
			TT_POLITICIAN,
			TT_SPACE,
			TT_STORYTELLER,
			TT_NUM_CATEGORIES
	};

	explicit            SwgCuiRoadmap (UIPage & page);

	virtual bool        OnMessage                    (UIWidget *context, const UIMessage & msg );
	void                OnButtonPressed              (UIWidget * context);
	virtual void        receiveMessage               (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);	
	        void        onSkillsChanged              (const CreatureObject & creature);
		    void        onPlayerExpChanged           (const PlayerObject & player);		
		    void        onLevelChanged               (const CreatureObject & player);				
	virtual void        OnTabbedPaneChanged          (UIWidget * Context);
			void        update                       (float deltaTimeSecs);

			void        setContextSkill              (std::string const & skillName);
			void        setContextTemplate           (std::string const & templateName);
			void        setContextSpaceTemplate      (std::string const & spaceTemplateName);

			std::string const &getStorytellerTemplate();
			std::string const &getPoliticianTemplate();
			std::string const &getContextSpaceTemplate();
			std::string const &getContextTemplate    ();
			std::string const &getContextRoadmap     ();
			std::string const &getContextSkill       ();

			void        setExperienceBar             (float val);
			float       getMainlineXpValue           ();

			TabType     getCurrentTabType            ();

			void        configPhaseIconDisplay       (int numPhaseIconsDisplayed, float *phaseProgress, int selectedPhase);
			void        disablePhaseIconDisplay      ();

			bool        isCurrentTemplateLevelBased  ();

protected:
	
	virtual void        performActivate              ();
	virtual void        performDeactivate            ();
	
private:
	typedef stdvector<std::string>::fwd StringVector;

	                    SwgCuiRoadmap          ();
	                    SwgCuiRoadmap          (const SwgCuiRoadmap &);
	SwgCuiRoadmap &operator=                   (const SwgCuiRoadmap &);

	                   ~SwgCuiRoadmap          ();

	void				updateExperienceBar    ();
	void                updateRewardsTable     ();
	void                updateItemReward       ();
	void                addSchematicGroups     (const StringVector & sv, UITableModelDefault & model);

	void                showZoomedOutDisplay   ();
	float               getXpValue             (const std::string &workingSkill);

	

	SwgCuiRoadmap_Profession *                 m_professionMediator;	

	UIText *                                   m_nextActivityName;
	std::string                                m_skillName;
	UIText *                                   m_nextActivityNameAurabesh;
	std::string                                m_spaceTemplateName;
	std::string                                m_templateName;
	std::string                                m_roadmapName;
	UIText *                                   m_nextActivityDescription;
	UIPage *                                   m_experienceBar;

	UITable *                                  m_rewardsTable;
	
	Unicode::String                            m_iconPathDraftGranted;
	UIImageStyle *                             m_stylePhaseComplete;
	UIImageStyle *                             m_stylePhaseIncomplete;
	UIImageStyle *                             m_stylePhaseCompleteSelected;
	UIImageStyle *                             m_stylePhaseIncompleteSelected;


	UIImage *                                  m_activityIcon;

	int                                        m_numProfessions;

	stdvector<std::string>::fwd                m_professionList;

	UIText *                                   m_titleText;
	
	MessageDispatch::Callback *		           m_callback;

	UIPage *                                   m_itemRewardPage;
	UIText *                                   m_itemRewardText;
	CuiWidget3dObjectListViewer *              m_itemRewardViewer;

	Object *                                   m_itemRewardObject;

	UITabbedPane *                             m_categoryTabs;
	TabType                                    m_currentTabType;

	UIImage **                                 m_phaseIcons;
	int                                        m_phaseIconsStartingY;
	UIPie **                                   m_phaseSwooshes;
	UIButton **                                m_phaseButtons;
	UIText **                                  m_phaseNumbers;
	
	bool                                       m_showingSpaceTab;
	bool                                       m_showingPoliticianTab;
	bool									   m_showingStorytellerTab;

};
//======================================================================

#endif
