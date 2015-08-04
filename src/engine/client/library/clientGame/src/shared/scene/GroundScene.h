//
// GroundScene.h
// Copyright 2000-2002 Sony Online Entertainment Inc.
// All Rights Reserved

#ifndef INCLUDED_GroundScene_H
#define INCLUDED_GroundScene_H

//-------------------------------------------------------------------

#include "clientGame/NetworkScene.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/Vector.h"

//-------------------------------------------------------------------

class CellProperty;
class ClientObject;
class ClientPathObject;
class ClientWeaponObjectTemplate;
class CockpitCamera;
class CreatureObject;
class DebugPortalCamera;
class FlyByCamera;
class FreeCamera;
class FreeChaseCamera;
class GameCamera;
class GraphWidget;
class Iff;
class IndexedTriangleList;
class InputMap;
class MessageQueue;
class MouseCursor;
class OverheadMap;
class Shader;
class ShipTurretCamera;
class SpaceTargetBracketOverlay;
class StructurePlacementCamera;
class Vector2d;
class Vector;

//-------------------------------------------------------------------

class GroundScene : public NetworkScene
{
public:

	static void install ();

	typedef void (*RenderDetailLevelFunction) (const Object* object);
	static void setRenderDetailLevelFunction (RenderDetailLevelFunction renderDetailLevelFunction);

public:

	enum CameraIds
	{
		CI_cockpit,
		CI_shipTurret,
		CI_freeChase,
		CI_free,
		CI_flyBy,
		CI_debugPortal,
		CI_structurePlacement,

		CI_COUNT
	};

private:

	typedef void (*ModeCallback) (void* context);
	void setModeCallback (ModeCallback modeCallback, void* context);
	bool isFinishedLoading() const;
	void _onFinishedLoading();
	void updateCuiLoading();
	void update(float elapsedTime);
	void updateLoading();

private:

	//-- input
	InputMap*               m_inputMap;
	InputMap*               m_debugPortalCameraInputMap;
	InputMap*               m_structurePlacementCameraInputMap;

	//-- free camera input
	InputMap*               m_freeCameraInputMap;
	MouseCursor*            m_mouseCursor;
	Vector2d*               m_lastYawPitchMod;

	//-- cameras
	CockpitCamera *         m_cockpitCamera;
	ShipTurretCamera *      m_shipTurretCamera;
	FreeChaseCamera*        m_freeChaseCamera;
	FreeCamera*             m_freeCamera;
	DebugPortalCamera*      m_debugPortalCamera;
	StructurePlacementCamera* m_structurePlacementCamera;
	FlyByCamera * m_flyByCamera;

	GameCamera*             m_cameras [CI_COUNT];
	int                     m_currentView;

	bool                    m_disableWorldSnapshot;

	bool                    m_usingGodClientCamera;
	bool                    m_usingGodClientInteriorCamera;

	bool                    m_loading;
	bool                    m_sentSceneChannel;
	bool                    m_receivedSceneReady;
	bool                    m_noDraw;
	int                     m_currentLoadCount;

	int                     m_debugKeyContext;
	std::string             m_debugKeySubContext;
	const ClientWeaponObjectTemplate*   m_debugKeyContextWeaponObjectTemplate;

	unsigned int            m_serverTimeOffset;

	//-- overhead map support
	OverheadMap* const      m_overheadMap;

	ModeCallback            m_modeCallback;
	void*                   m_context;
	bool                    m_currentMode;

	SpaceTargetBracketOverlay * const m_spaceTargetBracketOverlay;

	//-- client path (for /find)
	ClientPathObject *      m_clientPathObject;

	//-- debugging
	typedef stdvector<Vector>::fwd PointList;
	PointList* const m_debugPointList;

	struct Line
	{
		Vector m_start_w;
		Vector m_end_w;
		PackedArgb m_color;
	};

	typedef stdvector<Line>::fwd LineList;
	LineList * const m_debugLineList;

	bool m_isTutorial;

	typedef stdset<NetworkId>::fwd DestroyObjectSet;
	DestroyObjectSet * const m_destroyObjectSet;
	Timer m_destroyObjectTimer;

private:

	//-- explicitly disable the default constructor
	GroundScene ();
	GroundScene (const GroundScene&);
	GroundScene& operator= (const GroundScene&);

	void         load_0008 (Iff& iff);
	void         load_0009 (Iff& iff);
	void         load_0010 (Iff& iff);

	void         load (const char* terrainFilename, float timeInSeconds);
	void         preload (void);
	void         postload (void);

	void         handleInputMapEvent (IoEvent* event);
	void         handleInputMapScan (void);
	void         handleInputMapUpdate (void);
	void         scanInputMapForSceneMessages (InputMap* inputMap);

	void         init (const char * terrainFilename, CreatureObject * player, float timeInSeconds);
	void         setServerTimeOffset (unsigned int offset);

	void         handleDebugKeyContextKey1 ();
	void         handleDebugKeyContextKey2 ();

	void         handleEndBaselines (ClientObject *target);

public:

	static float getCameraFieldOfViewDegrees ();
	static void  setCameraFieldOfViewDegrees (float fieldOfViewDegrees);
	static float getCameraFarPlane           ();
	static void  setCameraFarPlane           (float farPlane);
	static float getCameraFarPlaneSpace();
	static void  setCameraFarPlaneSpace(float farPlane);
	static bool  getInvertMouseLook          ();
	static void  setInvertMouseLook          (bool b);
	static float getMouseSensitivity         ();
	static void  setMouseSensitivity         (float f);

	static bool  getListenerFollowsPlayer    ();
	static void  setListenerFollowsPlayer    (bool b);

public:

	GroundScene (const char* terrainFilename, const char* playerFilename, CreatureObject* customizedPlayer);
	GroundScene (const char* terrainFilename, const NetworkId& playerNetworkId, const char* const templateName, const Vector& startPosition, const float startYaw, float timeInSeconds, bool disableSnapshot);
	virtual ~GroundScene (void);

	virtual IoResult          processEvent(IoEvent *event);
	virtual void              draw (void) const;
	void                      drawOverlays (void) const;

	void                      setView (int newView, float value = 0.f);
	int                       getNumberOfViews () const;
	int                       getCurrentView () const;
	GameCamera*               getCamera (int view);
	const GameCamera*         getCamera (int view) const;
	GameCamera*               getCurrentCamera ();
	const GameCamera*         getCurrentCamera () const;

	void                      reloadTerrain ();

	void                      setDebugKeyContext (int debugKeyContext, const std::string& debugKeySubContext);
	void                      getDebugKeyContextHelp (std::string& result) const;

	MessageQueue *            getCurrentMessageQueue ();

	InputMap *                getInputMap ();

	void                      receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

	Object*                   findObject (int x, int y) const;
	int                       findObjects (real left, real top, real right, real bottom, ClientObject**& result) const;

	void                      pivotRotate    (real x, real y);
	void                      pivotTranslate (real x, real y);
	void                      pivotZoom      (real dz);

	void                      flyRotate      (real x, real y);
	void                      flyTranslate   (real x, real y, real z);

	void                      activateGodClientCamera ();
	void                      deactivateGodClientCamera ();

	void                      activateGodClientInteriorCamera ();
	void                      deactivateGodClientInteriorCamera ();

	FreeCamera *              getGodClientCamera ();
	InputMap *                getGodClientInputMap ();

	const FreeCamera *        getGodClientCamera () const;
	const InputMap *          getGodClientInputMap () const;

	bool                      isFirstPerson () const;

	bool                      isLoading () const;
	unsigned int              getServerTime () const;

	const Object* getSoundObject () const;

	void setMovePlayer (bool useMovePlayer, const Vector& direction_c = Vector::zero);
	bool getMovePlayer () const;

	void zoomCamera (bool in);
	bool isTutorial () const;

	void loadInputMap();

	void setNoDraw(bool noDraw);

	void turnOffOverheadMap();
};

//-------------------------------------------------------------------

inline InputMap * GroundScene::getInputMap ()
{
	return m_inputMap;
}

//-----------------------------------------------------------------

inline FreeCamera * GroundScene::getGodClientCamera ()
{
	return m_freeCamera;
}

//-----------------------------------------------------------------

inline InputMap * GroundScene::getGodClientInputMap ()
{
	return m_freeCameraInputMap;
}

//-----------------------------------------------------------------

inline const FreeCamera * GroundScene::getGodClientCamera () const
{
	return m_freeCamera;
}

//-----------------------------------------------------------------

inline const InputMap * GroundScene::getGodClientInputMap () const
{
	return m_freeCameraInputMap;
}

//-------------------------------------------------------------------

inline bool GroundScene::isTutorial () const
{
	return m_isTutorial;
}

//-------------------------------------------------------------------

#endif

