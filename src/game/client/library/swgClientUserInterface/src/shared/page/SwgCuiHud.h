//======================================================================
//
// SwgCuiHud.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiHud_H
#define INCLUDED_SwgCuiHud_H

//======================================================================

#include "sharedMath/Vector.h"
#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "UITypes.h"

#include "clientObject/ReticleObject.h"

class Camera;
class ClientObject;
class CuiConversationMenu;
class CuiDragInfo;
class CuiWorkspace;
class NetworkId;
class Object;
class ReticleObject;
class SwgCuiChatWindow;
class SwgCuiHudAction;
class SwgCuiHudWindowManager;
class TangibleObject;
class TerrainObject;
class Transform;
class UICursor;
class UIImage;
class UIPage;

//----------------------------------------------------------------------

class SwgCuiHud :
public CuiMediator,
public UIEventCallback
{
public:

	friend SwgCuiHudWindowManager;
	friend SwgCuiHudAction;

	enum ReticleType
	{
		RT_default,
		RT_attack,
		RT_use,
		RT_equip,
		RT_drop,
		RT_pickup,
		RT_info,
		RT_numReticleTypes
	};

	explicit            SwgCuiHud                    (UIPage & page);
	virtual void        update                       (float deltaTimeSecs);

	//-----------------------------------------------------------------
	//-- UIEventCallback

	virtual bool        OnMessage                           (UIWidget *Context, const UIMessage & msg);

	//-----------------------------------------------------------------

	SwgCuiChatWindow *  getChatWindow                       ();

	void                setHudEnabled                       (bool b);
	bool                getHudEnabled                       () const;

	void                startConversingWithSelectedCreature () const;

	void                colorTest                           (TangibleObject & id);
	void                hueObjectTest                       (const NetworkId & id, int maxIndex1, int maxIndex2);

	void                manageTargetBoxes                   (const Object * foundObject, const Camera & camera, const TerrainObject & terrain);

	void                targetAtCursor                      (bool startStop);
	void                targetAtCursorModal                 (bool startStop);
	void                targetAtCursorModeless              (bool startStop);

	void				summonRadialMenu					(const Unicode::String & params);
	void				setIntendedAndSummonRadialMenu		(bool mouseUp, bool cancel=false);

	bool                getAppropriateScreenPointForObject  (const Object & obj, UIPoint & pt) const;
	std::string const & getPlayerIdentificationString       () const;
	static std::string  createPlayerIdentificationString    ();
	
	Object *            getLastSelectedObject               () const;

	static const CachedNetworkId &  getHudAssistId          ();

protected:
	virtual            ~SwgCuiHud                     ();
	void                performActivate               ();
	void                performDeactivate             ();

	void                setAction                     (SwgCuiHudAction * action);
	void                setWindowManager              (SwgCuiHudWindowManager * windowManager);
	CuiWorkspace &      getWorkspace                  ();

	SwgCuiHudWindowManager * getWindowManager() const;

	virtual bool shouldRenderReticle() const;

	static void         internalCreateHudTemplate     (std::string const & pageName, std::string const & templateName);
	static void         internalCreateFreshHud        (std::string const & pageName, std::string const & templateName);

	bool m_targetingEnabled;

private:
	                    SwgCuiHud                     ();
	                    SwgCuiHud                     (const SwgCuiHud &);
	SwgCuiHud &   operator=                           (const SwgCuiHud &);

	void                checkDropOk                         ();
	void                handleDrop                          ();

	CuiWorkspace *             m_workspace;

	UIImage *                  m_reticles [static_cast<size_t>(RT_numReticleTypes)];

	enum SelBoxTypes
	{
		SBT_intended,
		SBT_target,
		SBT_combat,
		SBT_assist,
		SBT_critical_begin,
		SBT_critical_end = SBT_critical_begin + 32,
		SBT_group_begin,
		SBT_group_end = SBT_group_begin + 20,
		SBT_numTypes
	};

	UIPage *                   m_selectionBoxPages [SBT_numTypes];

	Watcher<Object>            m_lastSelectedObject;

	SwgCuiHudAction *          m_action;

	UIPoint                    m_lastReticlePoint;

	SwgCuiHudWindowManager *   m_windowManager;

	CuiDragInfo *              m_lastDragInfo;

	bool                       m_hudEnabled;

	UICursor *                 m_reticleDefault;

	UIImage *                  m_reticleImage;

	Watcher<Object>            m_radialMenuObject;
	bool                       m_radialMenuCountingDown;
	float                      m_radialMenuTimeout;
	float                      m_radialMenuStartTime;
	UIPoint                    m_radialMenuTimerCursorPos;

	std::string m_playerIdentificationString;

	float                      m_fireDelay;

	Watcher<ReticleObject>            m_reticleObject;
	std::vector<Watcher<ReticleObject> > m_adjacentReticleObjects;
};

//-----------------------------------------------------------------

#endif
