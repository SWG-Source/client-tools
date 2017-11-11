//======================================================================
//
// SwgCuiRoadmap_Profession.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiRoadmap_Profession_H
#define INCLUDED_SwgCuiRoadmap_Profession_H

//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiRoadmap_Profession.h"

#include "clientAudio/SoundId.h"
#include "clientUserInterface/CuiMediator.h"
#include "swgClientUserInterface/SwgCuiRoadmap.h"
#include "UIEventCallback.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedObject/CachedNetworkId.h"
#include "UITypes.h"
#include "UILowerString.h"

class ClientNotificationBoxMessage;
class CreatureObject;
class PlayerObject;
class UIPie;

namespace MessageDispatch
{
	class Emitter;
	class Callback;
};

//----------------------------------------------------------------------

class UIImage;
class UIText;
class SwgCuiRoadmap;

class SwgCuiRoadmap_Profession :
public CuiMediator,
public UIEventCallback,
public MessageDispatch::Receiver
{

public:
	explicit            SwgCuiRoadmap_Profession (UIPage & page, SwgCuiRoadmap *parentMediator);

	virtual void        receiveMessage               (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
			void        update                       (float deltaTimeSecs);

	void                OnButtonPressed              (UIWidget * context);

	void                onSkillsChanged              (const CreatureObject & creature);
	void                onPlayerExpChanged           (const PlayerObject & player);
			
	void                getCompletedSkillsInPhase    (int phase, int &completedSkills, int &uncompletedSkills);
	int                 getPlayersCurrentPhase       ();
	int                 getShowingPhase              ();

	void                setInnerExperienceBarValue   (float value);
	void                setInnerExperienceBarColor   (UIColor color);
	void                setCenterXPTextValue         (float value);
	void                setCenterSkill               (std::string const &skillName);
	void                convertPhaseToTrackAndPhase   (int phase, int &track, int &phaseOut);  

	void                showCategory                  (SwgCuiRoadmap::TabType category);
	void			    showZoomedInPhase             (int phase);

	void				SetStorytellerXPBar			 (int xpCurrent, int xpMax, int previousMax);
	void				HideStorytellerXPBar		 ();
	
protected:
	
	virtual void        performActivate              ();
	virtual void        performDeactivate            ();
	
private:
	
	                    SwgCuiRoadmap_Profession          ();
	                    SwgCuiRoadmap_Profession          (const SwgCuiRoadmap_Profession &);
	SwgCuiRoadmap_Profession &operator=                   (const SwgCuiRoadmap_Profession &);

	                   ~SwgCuiRoadmap_Profession          ();

	void               updateIconStatus              ();
	void               updateChicletStatus           ();
	void               updatePhaseStatus             ();
	void               hideBar                       (UIPage *bar);
	void               setProgressBar                (UIPage *bar, float val);

	void               layoutIconsUsing              (UIPage *rootPage, UIPage *samplePage, UIPage *sampleSmallPage, const std::string &roadmapName, int phase);
	void               layoutIconsUsing              (UIPage *rootPage, UIPage *samplePage, UIPage *sampleSmallPage, const std::string &templateName);
	void               layoutIconsUsing              (UIPage *rootPage, UIPage *samplePage, UIPage *sampleSmallPage, const stdvector<std::string>::fwd &skillVector, const stdvector<int>::fwd &xpVector, const int xpToFirstSkill);

	void               layoutIconsLevelBased         (UIPage *rootPage, UIPage *samplePage, const int startLevel, const int endLevel);

	void               erasePreviousLayoutIcons      (UIPage *rootPage);

	void               positionLayoutPage            (UIPage *page, int i, int n);
	void               positionChicletPage           (UIPage *page, int i, float c, int n);
	void               positionChicletText           (UIText *page, int i, float c, int n);
	void               positionChicletLinker         (UIImage *page, int i, float c, int n);
	void               positionLayoutLine            (UIPage *page, int i, float c, int n);
	
	void               updateContextRoadmap          ();

	void               updateBranchDisplay           ();

	bool               isFinalPhaseOfTrack           (int phase);
	int                getNextPhase                  (int phase);
	int                getPreviousPhase              (int phase); 
	
	UIString           getSkillTooltip               (std::string const &skillName);
	UIString           getPhaseTooltip               (std::string const &roadmapName, int phase);
	UIString           getPhaseTitle                 (std::string const &roadmapName, int phase);


	int                getNumberOfPhases             (int numberOfBranches);

	void               updateMainArc                 ();


					   UIPage **                     m_zoomedInPages;
					   UIPage **                     m_zoomedInRootPages;
					   UIPie **                      m_swooshes;
					   UIText **                     m_phaseTitles;
					   UIText **                     m_roadmapTitles;
					   UIText **                     m_phaseNumbers;
					   UIPage **                     m_needlePages;
					   UIImage **                    m_professionImages;
					   UIPage **                     m_phasePages;
					   
                       UIButton *                    m_phaseForwardButton;
                       UIButton *                    m_phaseBackwardButton;

					   int                           m_showingPhase; //if -1, zoomed out

					   UIPage *                      m_highlightPage;
					   UIPage *                      m_selectedPage;

					   UIImage **                    m_backgroundImages;
					   UIImage **					 m_levelBackgroundImages;
					   UIText **					 m_levelLabels;

					   SwgCuiRoadmap *               m_parentMediator;

					   int                           m_completedSkillsThisPhase;
					   int                           m_uncompletedSkillsThisPhase;

					   int							 m_playersCurrentPhase;

					   UIPage *                      m_sampleIconPage;
					   UIPage *                      m_sampleSmallIconPage;
					   UIPage **                     m_sampleFilledChicletPages;
					   UIPage **                     m_sampleEmptyChicletPages;
					   UIText *                      m_sampleChicletTextPage;
					   UIImage *                     m_sampleChicletLinkerImage;
					   UIPage *                      m_sampleLinePage;
					   UIImage *                     m_centerIcon;
					   UIText *                      m_centerText;
					   UIText *                      m_centerXPText;
					   UIPage *						 m_storytellerExpPage;
					   UIPage *						 m_storytellerExpBar;

					   float                         m_experienceValue;

					   int                           m_curNumBranches;
					   
					   bool                          m_okToSetUnchecked;

					   float                         m_innerXpValue;

					   SwgCuiRoadmap::TabType        m_currentCategory;
					   bool *                        m_usingSmallIcons;
	
};
//======================================================================

#endif
