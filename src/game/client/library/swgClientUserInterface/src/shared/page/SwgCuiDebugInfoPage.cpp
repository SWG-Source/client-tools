//======================================================================
//
// SwgCuiDebugInfoPage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiDebugInfoPage.h"

#include "UIButton.h"
#include "UIClock.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIImage.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UIVolumePage.h"
#include "UnicodeUtils.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/GameNetworkConnection.h"
#include "clientGame/GroundScene.h"
#include "clientGame/PlayerCreatureController.h"
#include "clientGame/ShipObject.h"
#include "clientGame/ShipProjectile.h"
#include "clientGame/TangibleObject.h"
#include "clientGraphics/Graphics.h"
#include "clientObject/GameCamera.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"
#include "clientSkeletalAnimation/TrackAnimationController.h"
#include "clientSkeletalAnimation/TrackAnimationController_Track.h"
#include "clientSkeletalAnimation/TrackAnimationController_TrackTemplate.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiLoginManagerClusterInfo.h"
#include "clientUserInterface/CuiLoginManagerClusterPing.h"
#include "clientUserInterface/CuiSharedPageManager.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Production.h"
#include "sharedNetworkMessages/GalaxyLoopTimesResponse.h"
#include "sharedNetworkMessages/RequestGalaxyLoopTimes.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/PortalProperty.h"
#include "sharedTerrain/TerrainObject.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include <cstdio>

//----------------------------------------------------------------------
namespace PlayerCreatureControllerNamespace
{
#if PRODUCTION == 0
	extern bool s_usePlayerServerSpeed;
#endif
	extern float ms_playerServerMovementSpeed;
}

namespace SwgCuiDebugInfoPageNamespace
{
	const int s_maxAnimTracks = 7;
}

using namespace SwgCuiDebugInfoPageNamespace;
using namespace PlayerCreatureControllerNamespace;
//----------------------------------------------------------------------

SwgCuiDebugInfoPage::SwgCuiDebugInfoPage(UIPage & page)
:
CuiMediator               ("SwgCuiDebugInfoPage", page),
UINotification            (),
UIEventCallback           (),
MessageDispatch::Receiver (),
m_camera_pText            (0),
m_camera_wText            (0),
m_player_pText            (0),
m_player_wText            (0),
m_fpsText                 (0),
m_terrainText             (0),
m_pingTimeText            (0),
m_bandwidthText           (0),
m_versionText             (0),
m_serverLoopTimeText      (0),
m_playerAnimInfoText	  (0),
m_playerServerSpeed       (0),
m_playerServerSpeedEnabled (0),
m_lastCameraCellProperty  (0),
m_lastCameraTransform_p   (),
m_lastPlayerCellProperty  (0),
m_lastPlayerTransform_p   (),
m_lastFps                 (0.0f),
m_lastFrameNumber         (0),
m_cachedPing              (0),
m_serverLoopTimeRequested (false)
{
	getCodeDataObject (TUIText, m_camera_pText,       "camera_p");
	getCodeDataObject (TUIText, m_camera_wText,       "camera_w");
	getCodeDataObject (TUIText, m_player_pText,       "player_p");
	getCodeDataObject (TUIText, m_player_wText,       "player_w");
	getCodeDataObject (TUIText, m_fpsText,            "fps");
	getCodeDataObject (TUIText, m_terrainText,        "terrain");
	getCodeDataObject (TUIText, m_pingTimeText,       "pingTime");
	getCodeDataObject (TUIText, m_bandwidthText,      "bandwidth");
	getCodeDataObject (TUIText, m_versionText,        "version");
	getCodeDataObject (TUIText, m_serverLoopTimeText, "serverLoopTime");
	getCodeDataObject (TUIText, m_playerAnimInfoText, "playerAnimInfo");
	m_playerAnimInfoText->SetPreLocalized(true);

	char buff[256];
	for(int i = 0; i < s_maxAnimTracks; ++i)
	{
		sprintf(buff,"playerAnimTrack%d",i);
		getCodeDataObject(TUIText, m_playerAnimTrackText[i],buff);
		m_playerAnimTrackText[i]->SetPreLocalized(true);
	}

	getCodeDataObject(TUIText, m_playerServerSpeed, "serverSpeed");
	getCodeDataObject(TUIText, m_playerServerSpeedEnabled, "serverSpeedEnabled");

	m_camera_pText->SetPreLocalized (true);
	m_camera_wText->SetPreLocalized (true);
	m_player_pText->SetPreLocalized (true);
	m_player_wText->SetPreLocalized (true);
	m_fpsText->SetPreLocalized (true);
	m_terrainText->SetPreLocalized (true);
	m_pingTimeText->SetPreLocalized (true);
	m_bandwidthText->SetPreLocalized (true);
	m_versionText->SetPreLocalized (true);
	m_serverLoopTimeText->SetPreLocalized (true);
	m_playerServerSpeed->SetPreLocalized(true);
	m_playerServerSpeedEnabled->SetPreLocalized(true);

	const char * const version = ApplicationVersion::getPublicVersion();
	NOT_NULL (version);

#if PRODUCTION == 0
	connectToMessage ("GalaxyLoopTimesResponse");
#endif

	m_versionText->SetLocalText (Unicode::narrowToWide (version));

	//hide server loop time and animtrack stuff in release
#if PRODUCTION == 1
	{
		m_serverLoopTimeText->SetVisible(false);
		UIText * serverLoopTimeLabel = NULL;
		getCodeDataObject (TUIText, serverLoopTimeLabel, "labelServerLoopTime");
		if (serverLoopTimeLabel)
			serverLoopTimeLabel->SetVisible(false);

		m_playerAnimInfoText->SetVisible(false);
		UIText * labelPlayerAnimInfo = NULL;
		getCodeDataObject (TUIText, labelPlayerAnimInfo, "labelPlayerAnimInfo");
		if(labelPlayerAnimInfo)
			labelPlayerAnimInfo->SetVisible(false);
		char buff[256];
		for(int i = 0; i < s_maxAnimTracks; ++i)
		{
			m_playerAnimTrackText[i]->SetVisible(false);

			UIText * labelPlayerAnimTrack = NULL;
			sprintf(buff,"labelPlayerAnimTrack%d",i);
			getCodeDataObject(TUIText,labelPlayerAnimTrack,buff);
			if(labelPlayerAnimTrack)
				labelPlayerAnimTrack->SetVisible(false);
		}
	
		UIText* labelPlayerServerSpeed = NULL;
		getCodeDataObject(TUIText, labelPlayerServerSpeed, "playerServerSpeed");
		if(labelPlayerServerSpeed)
			labelPlayerServerSpeed->SetVisible(false);
		m_playerServerSpeed->SetVisible(false);

		UIText* labelPlayerServerSpeedEnabled = NULL;
		getCodeDataObject(TUIText, labelPlayerServerSpeedEnabled, "playerServerSpeedEnabled");
		if(labelPlayerServerSpeedEnabled)
			labelPlayerServerSpeedEnabled->SetVisible(false);
		m_playerServerSpeedEnabled->SetVisible(false);
	}

#endif

	UIPage * const parent = safe_cast<UIPage *>(getPage ().GetParent ());
	NOT_NULL (parent);
	CuiSharedPageManager::registerMediatorType (parent->GetName (), CuiMediatorTypes::DebugInfoPage);

	connectToMessage (Game::Messages::SCENE_CHANGED);
}

//----------------------------------------------------------------------

SwgCuiDebugInfoPage::~SwgCuiDebugInfoPage()
{
	disconnectFromMessage (Game::Messages::SCENE_CHANGED);

	m_camera_pText = 0;
	m_camera_wText = 0;
	m_player_pText = 0;
	m_player_wText = 0;
	m_fpsText = 0;
	m_terrainText = 0;
	m_pingTimeText = 0;
	m_bandwidthText = 0;
	m_versionText = 0;
	m_serverLoopTimeText = 0;
	m_playerAnimInfoText = 0;
	m_playerServerSpeed = 0;
	m_playerServerSpeedEnabled = 0;
	for(int i = 0; i < s_maxAnimTracks; ++i)
	{
		m_playerAnimTrackText[i] = 0;
	}

	m_lastCameraCellProperty = 0;
	m_lastPlayerCellProperty = 0;
}

//----------------------------------------------------------------------

void SwgCuiDebugInfoPage::performActivate()
{
	UIPage * const parent = safe_cast<UIPage *>(getPage ().GetParent ());
	if (parent)
		CuiSharedPageManager::checkPageVisibility (parent->GetName (), *parent, &getPage (), CuiMediatorTypes::DebugInfoPage, true);

	CuiLoginManager::setPingEnabled (CuiLoginManager::getConnectedClusterId (), true);

	UIClock::gUIClock ().ListenPerFrame (this);
}

//----------------------------------------------------------------------

void SwgCuiDebugInfoPage::performDeactivate()
{
	UIClock::gUIClock ().StopListening (this);

	UIPage * const parent = safe_cast<UIPage *>(getPage ().GetParent ());
	if (parent)
		CuiSharedPageManager::checkPageVisibility (parent->GetName (), *parent, &getPage (), CuiMediatorTypes::DebugInfoPage, false);

	CuiLoginManager::setAllPingsDisabled ();
}

//----------------------------------------------------------------------

void SwgCuiDebugInfoPage::OnButtonPressed(UIWidget * /*context*/)
{
}

//----------------------------------------------------------------------

void SwgCuiDebugInfoPage::Notify (UINotificationServer *, UIBaseObject *, UINotification::Code )
{
	updateBandwidth ();
	updateFps ();
	updatePositions ();
	updateTerrain ();
	updateServerInfo ();
	updateAnimInfo ();
}

//----------------------------------------------------------------------

void SwgCuiDebugInfoPage::updateAnimInfo()
{
// don't show anim info in release client
#if PRODUCTION == 0
	Object* object = Game::getPlayer();
	Appearance *appearance = NULL;
	SkeletalAppearance2* skeletalAppearance2 = NULL;
	TransformAnimationController* transformAnimationController = NULL;
	StateHierarchyAnimationController* stateHierarchyAnimationController = NULL;

	if(object && object->asClientObject() && object->asClientObject()->asCreatureObject() 
		&& object->asClientObject()->asCreatureObject()->isRidingMount())
	{
		object = object->asClientObject()->asCreatureObject()->getMountedCreature();
	}

	// drill down to the controller
	if(object)
	{
		appearance = object->getAppearance();
		if(appearance)
		{
			skeletalAppearance2 = dynamic_cast<SkeletalAppearance2 *>(appearance);
			if(skeletalAppearance2)
			{
				transformAnimationController = skeletalAppearance2->getAnimationController();
				if(transformAnimationController)
				{
					stateHierarchyAnimationController = transformAnimationController->asStateHierarchyAnimationController();
				}
			}
		}
	}

	if(stateHierarchyAnimationController)
	{
		TrackAnimationController& trackAnimationController = stateHierarchyAnimationController->getTrackAnimationController();
		int numTrackTemplates = trackAnimationController.getTrackTemplateCount();
		numTrackTemplates = std::min(numTrackTemplates,s_maxAnimTracks);
		char buffer[256];
			
		_snprintf(buffer, sizeof(buffer), "numTrackTemplate == %d",numTrackTemplates);
		m_playerAnimInfoText->SetLocalText (Unicode::narrowToWide(buffer));

		for(int i = 0; i < numTrackTemplates; ++i)
		{
			const TrackAnimationController::TrackTemplate& trackTemplate = trackAnimationController.getTrackTemplate(i);
			AnimationTrackId animationTrackId = trackTemplate.getTrackId();

			const TrackAnimationController::Track* track = trackAnimationController.getTrackConstPtr(animationTrackId);
			const SkeletalAnimation* skeletalAnimation = track ? track->fetchCurrentAnimation() : NULL;
			std::string animationName = "-none-";
			if(skeletalAnimation)
			{
				animationName = skeletalAnimation->getLeafAnimationTemplateName().getString();
				skeletalAnimation->release();
			}
							
			sprintf(buffer,"(%d)(%s)[%s]",
				i,
				trackTemplate.getTrackName().getString(),
				animationName.c_str()
			);
			m_playerAnimTrackText[i]->SetLocalText (Unicode::narrowToWide(buffer));
		}
	}
#endif
// end - don't show anim info in release client
}

//----------------------------------------------------------------------

void SwgCuiDebugInfoPage::updateServerInfo()
{
	const uint32 clusterId = CuiLoginManager::getConnectedClusterId();
	if (clusterId)
	{
		const CuiLoginManager::ClusterInfo * const ci = CuiLoginManager::findClusterInfo(clusterId);
		const int ping = static_cast<int>(ci->getLatencyMs());
		
		if (!ci->getPing().isEnabled())
		{
			CuiLoginManager::setPingEnabled(clusterId, true);
		}

		if (ping != m_cachedPing)
		{
			char buf[256];
			_snprintf (buf, sizeof(buf), "%3ims", ping);
			m_pingTimeText->SetLocalText (Unicode::narrowToWide (buf));
			m_cachedPing = ping;
		}
	}

#if PRODUCTION == 0
	if (!m_serverLoopTimeRequested)
	{
		RequestGalaxyLoopTimes request;
		GameNetwork::send(request, true);
		m_serverLoopTimeRequested = true;
	}
#endif
}

//----------------------------------------------------------------------

void SwgCuiDebugInfoPage::updatePositions (const Object* const object, UIText* const position_wText, UIText* const position_pText) const
{
	if (object)
	{
		char buffer [256];

		//-- position_w
		const Vector& position_w = object->getPosition_w ();
		const int     frameK_w   = static_cast<int> (convertRadiansToDegrees (object->getObjectFrameK_w ().theta ()));
		_snprintf (buffer, sizeof (buffer), "%5.1f %5.1f %5.1f %i  world", position_w.x, position_w.y, position_w.z, frameK_w);
		position_wText->SetLocalText (Unicode::narrowToWide (buffer));

		//-- position_p
		const Vector&             position_p    = object->getPosition_p ();
		const int                 frameK_p      = static_cast<int> (convertRadiansToDegrees (object->getObjectFrameK_p ().theta ()));
		const CellProperty* const cellProperty  = object->getParentCell ();
		char const * const accessAllowed = (cellProperty->getAccessAllowed()) ? "public" : "private";
		_snprintf (buffer, sizeof (buffer), "%5.1f %5.1f %5.1f %i  %s  %s  [%s] %s", position_p.x, position_p.y, position_p.z, frameK_p, cellProperty == CellProperty::getWorldCellProperty () ? "world" : cellProperty->getPortalProperty ()->getPobShortName (), cellProperty == CellProperty::getWorldCellProperty () ? "world" : cellProperty->getCellName (), cellProperty == CellProperty::getWorldCellProperty () ? "NA" : cellProperty->getOwner ().getNetworkId ().getValueString ().c_str (), accessAllowed);
		position_pText->SetLocalText (Unicode::narrowToWide (buffer));
	}
}

//----------------------------------------------------------------------

void SwgCuiDebugInfoPage::updatePositions ()
{
	//-- update player position
	{
		const Object* const player = Game::getPlayer ();
		if (player)
		{
			if (player->getParentCell () != m_lastPlayerCellProperty || player->getTransform_o2p () != m_lastPlayerTransform_p)
			{
				m_lastPlayerCellProperty = player->getParentCell ();
				m_lastPlayerTransform_p  = player->getTransform_o2p ();

				updatePositions (player, m_player_wText, m_player_pText);
			}

			char buffer[32];
			memset(buffer, 0, 32);
			_snprintf(buffer, sizeof(buffer), "%3.2f", ms_playerServerMovementSpeed);
			m_playerServerSpeed->SetLocalText(Unicode::narrowToWide(buffer));
#if PRODUCTION == 0
			m_playerServerSpeedEnabled->SetLocalText(s_usePlayerServerSpeed ? Unicode::narrowToWide("True") : Unicode::narrowToWide("False"));
#endif
		}
	}

	//-- update camera position
	{
		const Object* const camera = Game::getCamera ();
		if (camera)
		{
			if (camera->getParentCell () != m_lastCameraCellProperty || camera->getTransform_o2p () != m_lastCameraTransform_p)
			{
				m_lastCameraCellProperty = camera->getParentCell ();
				m_lastCameraTransform_p  = camera->getTransform_o2p ();

				updatePositions (camera, m_camera_wText, m_camera_pText);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiDebugInfoPage::updateFps ()
{
	//-- update FPS if needed, every 10 frames
	const int currentFrameNumber = Graphics::getFrameNumber ();
	if (currentFrameNumber > (m_lastFrameNumber + 10))
	{
		const float fps = Clock::framesPerSecond ();

		m_lastFrameNumber = currentFrameNumber;

		if (fps != m_lastFps) //lint !e777 testing floats for equality, this is alright since we're only trying to determine if we need to paint new test, rounding errors don't matter
		{
			m_lastFps = fps;

			const int numberOfMegabytesAllocated = MemoryManager::getCurrentNumberOfBytesAllocated () / (1024 * 1024);
			const int limit = MemoryManager::getLimit ();

			char buf[128];
			sprintf (buf, "%5.2f     %iMB/%iMB", fps, numberOfMegabytesAllocated, limit);
			UINarrowString str (buf);

			m_fpsText->SetLocalText (Unicode::narrowToWide (str));
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiDebugInfoPage::updateTerrain ()
{
	if (TerrainObject::getInstance ())
	{
		int hour = 0;
		int minute = 0;
		TerrainObject::getInstance ()->getTime (hour, minute);

		char buffer [64];
		_snprintf (buffer, sizeof(buffer), "Time: %02i:%02i     T=%i TNT=%i I=%i C=%i S=%i P=%i", hour, minute, ClientWorld::getNumberOfObjects (WOL_Tangible), ClientWorld::getNumberOfObjects (WOL_TangibleNotTargetable), ClientWorld::getNumberOfObjects (WOL_Intangible), CreatureObject::getNumberOfInstances (), ShipObject::getNumberOfInstances(), ShipProjectile::getNumberOfInstances());
		m_terrainText->SetLocalText (Unicode::narrowToWide (buffer));
	}
}

//----------------------------------------------------------------------

void SwgCuiDebugInfoPage::updateBandwidth ()
{
	char buffer [64];
	snprintf (buffer, sizeof(buffer), "R: %i  S: %i", Game::getReceivedCompressedBytesPerSecond(), Game::getSentCompressedBytesPerSecond());
	m_bandwidthText->SetLocalText (Unicode::narrowToWide (buffer));
}

//----------------------------------------------------------------------

void SwgCuiDebugInfoPage::receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
{
	if (message.isType("GalaxyLoopTimesResponse"))
	{
		Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const GalaxyLoopTimesResponse cg(ri);

#if PRODUCTION == 0
		char buf [256];
		_snprintf (buf, sizeof(buf), "%lu ms", (cg.getLastFrameMilliseconds() / 3193)); // loop time (in ms) * 3193
		m_serverLoopTimeText->SetLocalText (Unicode::narrowToWide (buf));
#else
		UNREF(cg);
#endif

		m_serverLoopTimeRequested = false;
	}

	//if we change scenes (i.e. disconnect/reconnect), make sure to start over with server requests
	else if (message.isType (Game::Messages::SCENE_CHANGED))
	{
		m_serverLoopTimeRequested = false;
	}
}

//======================================================================
