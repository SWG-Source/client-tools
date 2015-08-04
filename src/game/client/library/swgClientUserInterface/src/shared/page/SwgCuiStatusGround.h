//======================================================================
//
// SwgCuiStatusGround.h
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiStatusGround_H
#define INCLUDED_SwgCuiStatusGround_H

//======================================================================

#include "swgClientUserInterface/SwgCuiMfdStatus.h"

//----------------------------------------------------------------------

class Buff;
class ClientMfdStatusUpdateMessage;
class CreatureObject;
class PlayerObject;
class SwgCuiBuff;
class SwgCuiMfdStatusBar;
class Transform;
class UIImage;
class UIText;
class UIVolumePage;


//----------------------------------------------------------------------

namespace SwgCuiStatusGroundNamespace
{
	class BuffIconSettingsChangedCallback;
};


class SwgCuiStatusGround :
public SwgCuiMfdStatus
{
public:
	
	enum StatusType
	{
		ST_undefined, // Do not use this style.
		ST_player, // The player HAM bar in the upper left.
		ST_lookAtTarget, // The look-at target status window.
		ST_intendedTarget, // The intended target status window
		ST_target, // Any of the targeting pages on screen.
		ST_group, // The group window status pages.
		ST_pet, // pet status page
		ST_count
	};

	enum ConIconIndex
	{
		CII_lower,
		CII_even,
		CII_higher,
		CII_MAX
	};

	// Max pages per HAM bar.
	enum { HAMBarPageCount = 6 }; // Number of pages tracked.

	explicit SwgCuiStatusGround(UIPage & page, StatusType statusType);

	static void deliverClientMfdStatusUpdateMessage(const ClientMfdStatusUpdateMessage & msg);

	void setStatusType(StatusType st);
	int getBuffIconType();

	void setTarget(TangibleObject * tangible);
	void setTarget(const NetworkId & id);
	void clearTarget();
	void setShowRange(bool b);
	void update(float deltaTimeSecs);
	void update(ClientMfdStatusUpdateMessage const & msg);
	
	void setDisplayStates(bool stats, bool states, bool posture);
	
	void OnPopupMenuSelection (UIWidget * context);
	bool OnMessage (UIWidget *context, const UIMessage & msg);

	void onBuffsChanged(CreatureObject const & creature);
	void onSkillsChanged(const CreatureObject & creature);

	void setDisplayNameOnly(bool display);
	void setResizeTargetName(bool resize);
	void setPulseNameOnCombat(bool pulse);
	void setLookAtTarget(bool const isLookAt);
	void setIsHoverTarget(bool const isHover);

	void setShowOutOfRange(bool outOfRange);

	void showDebugInformation(bool const show);
	void setDebugInformation(std::string const & debugInfo);

	UIScalar getRenderHeight() const;

	bool isStatusBarVisibleAndOverlapping(SwgCuiStatusGround const * const otherStatus) const;

	void onBuffIconSettingsChangedCallback();

protected:

	void performActivate();
	void performDeactivate();

private:
	virtual ~SwgCuiStatusGround();
	SwgCuiStatusGround();
	SwgCuiStatusGround(const SwgCuiStatusGround &);
	SwgCuiStatusGround & operator=(const SwgCuiStatusGround &);


	bool addBuffIcon(const Buff & buff, UIBaseObject *& lastChild);
	void hideLevelInfo();

	void enableCombatEffector(bool enable);
	void refreshTargetInfo();

	void setTargetNamePrefix(const Unicode::String & prefix);
	void setTargetName(const Unicode::String & name);
	void setObjectName(const Unicode::String & name);

	void updateCombatEffector(CreatureObject const & player, CreatureObject const & creature);
	void updateCreatureLevel(CreatureObject const & creature, bool enableCreatureConLevel);
	void updateRoleIcon(PlayerObject const & player);
	void updateRoleIcon();  //Updates using existing id if player
	bool updateVisiblity();
	void updateTargetName(ClientObject const & obj);
	void updateDirectionArrow(Object const * const player, TangibleObject const & target);
	void updateDirectionArrow(Object const * const player, Vector const & target);
	void updateCreatureBuffs(CreatureObject const & creature);
	void updateDistance(Object const * const playerObject, float const distance);
	void updateTargetGuild(CreatureObject const & creature);
	
	void updatePlayerStatus(PlayerObject const & playerObject);
	void updatePlayerTitle(PlayerObject const & playerObject);
	void updatePlayerFaction(PlayerObject const & player, CreatureObject const & creature);
	
	bool updateTargetHam(CreatureObject const & obj, float deltaTimeSecs);
	bool updateTargetHam(TangibleObject const & obj, float deltaTimeSecs);
	bool updateAttackableTargetHam(TangibleObject const & obj, float deltaTimeSecs);
	bool updateHamBar(CreatureObject const & creature, int const styleIndex, float deltaTimeSecs);
	void hideHamBars();
	void updateHamBarToolTips(int barIndex, int current, int normalMax);
	void updateFaction(TangibleObject const & tangibleObject);
	void updateEliteStatus(CreatureObject const & creatureObject);
	void hideEliteStatus();

	void onNewTarget();

	void updateVoiceChatIcon(TangibleObject const & tangibleObject);

private:
	
	void setRoleIconInternal(int roleIconChoice);

	SwgCuiMfdStatusBar * m_bars[HAMBarPageCount];
	UIPage * m_hamBarParent;
	Attributes::Enumerator m_attributes[HAMBarPageCount];

	UIVolumePage * m_volumeStates;
	UIVolumePage * m_debuffStates;
	UIImage * m_sampleStateIcon;
	UIPage * m_sampleIconPage;

	bool m_displayStats;
	bool m_displayStates;
	bool m_displayPosture;
	bool m_displayNameOnly;
	bool m_resizeTargetName;
	bool m_pulseNameOnCombat;
	bool m_isLookAtTarget;
	bool m_isHoverTarget;
	bool m_showRange;
	bool m_outOfRange;
	bool m_isVehicle;
	bool m_isPlayerControlled;
	bool m_isThePlayer;
	bool m_isPlayerMounted;
	bool m_isJedi;

	Unicode::String m_targetNamePrefix;

	UIImage * m_iconDifficulty;
	UIImage * m_iconFaction;

	UIImage * m_eliteSkin;

	Unicode::String m_objectName;
	ClientMfdStatusUpdateMessage * m_lastClientMfdStatusUpdateMessage;

	UIPage * m_pageRole;
	UIImage * m_pageRoleIcon;

	UIImage * m_conIcons[CII_MAX];

	UIEffector * m_effectorBlink;

	UIEffector * m_effectorCombat;
	UIWidget * m_effectorCombatTarget;

	StatusType m_statusType;

	Vector m_lastKnownPosition;
	float m_lastKnownDistance;

	UIText * m_textLevel;
	UIText * m_textStatus;
	UIText * m_textTitle;
	UIText * m_textName;
	UIText * m_textDistance;
	UIText * m_textFaction;
	UIText * m_textGuild;
	UIText * m_textDebug;
	
	UIImageStyle * m_conStyles[4];

	UIImage * m_eliteLeftPage;
	UIImage * m_eliteRightPage;

	UIPage * m_compositePage;

	UIImage * m_speakingIcon;

	SwgCuiStatusGroundNamespace::BuffIconSettingsChangedCallback * m_buffIconSettingsChangedCallback;
};

//======================================================================

#endif
