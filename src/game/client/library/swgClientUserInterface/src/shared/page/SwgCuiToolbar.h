//======================================================================
//
// SwgCuiToolbar.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiToolbar_H
#define INCLUDED_SwgCuiToolbar_H

//======================================================================

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/Game.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedObject/CachedNetworkId.h"
#include "swgClientUserInterface/SwgCuiLockableMediator.h"
#include "UITypes.h"
#include "UILowerString.h"
#include "UITextStyleManager.h"

#include <map>
#include <vector>


//----------------------------------------------------------------------

class CreatureObject;
class CuiDragInfo;
class CuiWidget3dObjectListViewer;
class MessageQueueCommandTimer;
class PlayerObject;
class SwgCuiMfdStatus;
class UIButton;
class UIColorEffector;
class UIImage;
class UIPage;
class UIPie;
class UIScrollbar;
class UITabbedPane;
class UIVolumePage;
class UIText;
class WeaponObject;

namespace MessageDispatch
{
	class Emitter;
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiToolbar :
public SwgCuiLockableMediator,
public MessageDispatch::Receiver
{
public:

	explicit            SwgCuiToolbar (UIPage & page, Game::SceneType sceneType);

	static void install();
	static void performPrimaryAttack(NetworkId const & target);
	static void performSecondaryAttack(NetworkId const & target);
	static void performSpecialAttack(NetworkId const & target);
	
	virtual bool        OnMessage                    (UIWidget *context, const UIMessage & msg );
	virtual void        OnPopupMenuSelection         (UIWidget * context);
	virtual void        OnVolumePageSelectionChanged (UIWidget * context);
	virtual void        OnTabbedPaneChanged          (UIWidget * Context);

	virtual void        receiveMessage               (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

	        void        nextPane                     ();
	        void        prevPane                     ();

			void        switchToPane                 (int pane);
			int         getCurrentPane               ();

	        bool        performToolbarAction         (int slot, bool pet = false);

			void		selectNextAvailableSlot();
			void		selectPreviousAvailableSlot();

			void        update                       (float deltaTimeSecs);
	virtual void        saveSettings                 () const;
	virtual void        loadSettings                 ();

	void                onMacrosChangedReset         (const bool & );

	void                onPlayerSetup                (CreatureObject const & payload);
	void                updateKeyBindings            ();

	void                doWhiteFlash                 (const uint32 strCrc);
	void                onCommandTimerDataReceived   (const MessageQueueCommandTimer & commandTimerData);
		
	void                setCommandExecuting          (const uint32 strCrc);
	void                setCommandExecuting          (const uint32 strCrc, float currentExecTime, float maxExecTime);
	void                setCommandFailed             (const uint32 strCrc);

	static SwgCuiToolbar* getActiveToolbar           ();

	void                clearCommandDisplayStates    ();  // Clears active, pending, and failed boxes

	void setDefaultAction(int slot, bool activateActionIfNeeded = true);

	void                setCooldownTime              (int cooldownGroup, float currentTime, float maxTime) const;
	bool                isDoubleToolbar() const;
	void                startProcessingActions       ();
	void                stopProcessingActions        ();

	void				setPetBarVisible			 (const bool visible);
	void				toggleDefaultPetAction		 (int slot);
protected:
	virtual void        performActivate              ();
	virtual void        performDeactivate            ();

	void                setupDefaults                ();

private:
	                   ~SwgCuiToolbar                ();

	CuiDragInfo *       getToolbarItem               (int pane, int slot, bool pet = false);
	CuiDragInfo *       getToolbarItem               (int slot, bool pet = false);

	UIWidget *          createToolbarWidget          (const CuiDragInfo & item);

	int                 discoverToolbarSlot          (UIWidget * context, bool pet = false);

	void                setToolbarItem               (int pane, int slot, const CuiDragInfo & item, bool pet = false);

	UIWidget *          getToolbarItemWidget         (int slot, bool pet = false);
	UIWidget *          getToolbarItemShadeWidget    (int slot, bool pet = false);
	UIText*			    getToolbarCooldownTimer      (int slot, bool pet = false);
	UIWidget *          getToolbarItemBackgroundWidget (int slot, bool pet = false);
	void                populateSlot                 (int slot, bool pet = false);

	void                repopulateSlots              (bool pet = false);

	void                onCommandAdded               (const std::pair<CreatureObject *, std::string> &);
	void                onCommandRemoved             (const std::pair<CreatureObject *, std::string> &);
	
	void                onSceneChanged               (bool const &);

	void				onCurrentWeaponChanged		 (CreatureObject const & creature);
	void				onMaxRangeWeaponChanged		 (WeaponObject const & weapon);
	void				onMinRangeWeaponChanged		 (WeaponObject const & weapon);
	void                updateCommandRange           (int slot, bool pet = false);

	void				onPetCommandsChanged		 (PlayerObject const & player);
	void				onPetToggledCommandsChanged	 (PlayerObject const & player);
	UIImage	*			getPetToolbarHighlightImage  (int slot);
	void				updatePetToolbarToggle		 (int const slot);
	void                clearWidgets                 (bool pet = false);

	void                setupTabData                 ();
	void                updateFromSizes              ();
	
	UIPage *            getPageFromPool              (UIPage *original, stdvector<UIPage *>::fwd &duplicates, int &index, bool toOriginalParent = false);
	void                resetAllPagesInPool          (stdvector<UIPage *>::fwd const &duplicates, int &index);
	void                deleteAllPagesInPool         (stdvector<UIPage *>::fwd &duplicates);

	bool                getWeaponTypeIncompatible    (uint32 weaponTypesValid, uint32 weaponTypesInvalid, int weaponType);
	bool                isMeleeWeapon                (int weaponType);
	bool                isRangedWeapon               (int weaponType);
	bool                isLightsaberWeapon           (int weaponType);

	void                removeOldStyleIcons          ();

	bool                isCooldownFinished           (int cooldownGroup) const;
	unsigned long       getCooldownProgress          (int cooldownGroup) const;

	uint32				getTemplateCrcForObject       (const CuiDragInfo & item);

	void                processCooldownGroupOnlyMessage(const MessageQueueCommandTimer & commandTimerData);
	                    SwgCuiToolbar                ();
	                    SwgCuiToolbar                (const SwgCuiToolbar &);
	SwgCuiToolbar &     operator=                    (const SwgCuiToolbar &);

	std::string getActionString(int slot, bool pet = false);
	std::string getCommandName(int const slot, bool pet = false);
	ClientObject * getObjectFromSlot(int const slot, bool pet = false);
	void populateDefaultActionWindow();

	void updatePrimaryThrottleBar();

	Game::SceneType     getSceneType                  () const;

private:
	class SwgCuiToolbarAction;
	typedef stdvector<CuiDragInfo>::fwd     ToolbarItemPane;
	typedef stdvector<ToolbarItemPane>::fwd ToolbarItemPaneVector;

	UIPage *					  m_toolbarPage;
	UIPage *					  m_petToolbarPage;

	UIVolumePage *                m_volumePage;
	UIVolumePage *                m_volumeBackgroundPage;
	UIPage *                      m_volumeTimersPage;

	UIVolumePage *                m_petVolumePage;
	UIVolumePage *                m_petVolumeBackgroundPage;	
	UIVolumePage *                m_petVolumeHighlightsPage; // for toggle status
	UIPage *                      m_petVolumeTimersPage;

	UITabbedPane *                m_tabs;

	SwgCuiToolbarAction *         m_action;

	ToolbarItemPaneVector *       m_toolbarItemPanes;
	ToolbarItemPane *		      m_petToolbarItemPane;

	long                          m_draggingPane;
	long                          m_draggingSlot;
	long                          m_dragCounter;

	UIPoint                       m_popupPoint;
	long                          m_popupSlot;

	UIPage *                      m_sampleItemPage;

	MessageDispatch::Callback *   m_callback;

	UIRect                        m_iconMargin;

	UIColorEffector *             m_effectorUse;
	UIColorEffector *             m_effectorUseChild;

	bool                          m_sendEquipToolbar;

	UIVolumePage *                m_volumeKeyBindings;
	UIText *                      m_textPane;

	int                           m_mouseDownOnSlot;

	UISize                        m_lastSize;

	UILowerString                 m_palIconFlashColor;

	UIButton *                    m_sampleItemButton;

	bool                          m_playerIsSetup;

	bool						  m_toolbarHasLocalSettings;

	UIPage *                      m_currentActionPage;
	UIPage *                      m_failedActionPage;
	UIPage *                      m_whiteFlashPage;

	std::vector<UIPage *>         m_currentActionPages;
	std::vector<UIPage *>         m_failedActionPages;
	std::vector<UIPage *>         m_whiteFlashPages;

	int                           m_nextNextActionPage;
	int                           m_nextCurrentActionPage;
	int                           m_nextFailedActionPage;
	int                           m_nextWhiteFlashPage;

	float                         m_executeMaxTimer;
	float                         m_executeTimer;

	float                         m_warmupMaxTimer;
	float                         m_warmupTimer;
	
	Game::SceneType m_sceneType;

	uint32                        m_commandExecutingCrc;
	uint32                        m_commandFailedCrc;

	int                           m_lastCooldownGroupReceived;
	int                           m_executingGroup;

	int                           m_mouseOverSlot;
	UIPage *                      m_mouseOverPage;

	uint32                        m_clientOverrideCurrentActionCrc;
	uint32                        m_clientOverrideNextActionCommandCrc;
	int                           m_clientOverrideCooldownGroup;

	int                           m_defaultActionSlot;
	UIPage *                      m_defaultActionPage;
	int                           m_nextDefaultActionPage;

	UIPage *                      m_wrongWeaponTypePage;
	stdvector<UIPage *>::fwd      m_wrongWeaponTypePages;
	int                           m_nextWrongWeaponTypePage;
	
	UIEffector *         m_effectorCurrent;

	float *                       m_lastFadedHeight;

	bool                          m_removedOldIcons;

	UIButton *                    m_bigButton;
	CuiWidget3dObjectListViewer * m_sampleBigViewer;
	CuiWidget3dObjectListViewer * m_bigViewer;
	UIPie *                       m_bigShade;
	// 2 pages so that the art didn't need to be changed
	UIPage *                      m_bigViewerBackground1;
	UIPage *                      m_bigViewerBackground2;

	UIPage *                      m_throttleBarParent;
	UIPage *                      m_throttleBar;

	bool						  m_clickedInActionBar;
	bool						  m_doubleToolbar;

	UITextStyleManager*           m_textStyleManager;
};

//======================================================================

#endif
