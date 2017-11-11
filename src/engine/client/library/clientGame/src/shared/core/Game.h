//
// Game.h
// asommers 6-7-99
//
// copyright 1999, bootprint entertainment
//

//-------------------------------------------------------------------

#ifndef GAME_H
#define GAME_H

//-------------------------------------------------------------------

class Camera;
class ClientObject;
class CreatureObject;
class InputMap;
class MessageQueue;
class NetworkId;
class Object;
class PlayerObject;
class ShipObject;
class Scene;
class Timer;
class Vector;

namespace MessageDispatch
{
	class Emitter;
}

namespace GameNamespace
{
	class SceneCreator;
}

//-------------------------------------------------------------------

class Game
{
public:

	typedef std::basic_string<unsigned short, std::char_traits<unsigned short>, std::allocator<unsigned short> > UnicodeString;
	struct Messages
	{
		static const char * const SCENE_CHANGED;

		struct ChatStartInput
		{
			typedef Unicode::String Payload;
		};

		struct DebugPrintUi
		{
			typedef UnicodeString Payload;
		};

		struct SceneChanged
		{
			typedef bool Payload;
		};

		struct CollectionServerFirstChanged
		{
			typedef std::string Payload;
		};

		struct CollectionShowServerFirstOptionChanged
		{
			typedef bool Payload;
		};
	};

	enum Application
	{
		A_client,
		A_godClient,
		A_particleEditor,
		A_animationEditor,
		A_npcEditor
	};

	enum SceneType
	{
		ST_ground,
		ST_space,
		ST_reset,
		ST_numTypes
	};

public:

	static void                install(Application const application);
	static void                remove();
	static void                run();

	static void                quit();
	static void                startExitTimer ();

	static void                cleanupScene();

	static void                runGameLoopOnce(bool presentToWindow, HWND hwnd, int width, int height);

	static Scene*              getScene ();
	static const Scene*        getConstScene ();

	static void setScene(
		bool            immediately,
		const char    * terrainFilename, 
		const char    * playerFilename, 
		CreatureObject* customizedPlayer
	);

	static void setScene(
		bool            immediately,
		const char*     terrainFilename, 
		const NetworkId& playerNetworkId, 
		const char* const templateName, 
		const Vector& startPosition, 
		const float startYaw, 
		float timeInSeconds, 
		bool disableSnapshot
	);

	static void                _setScene (Scene* newScene);

	static bool                playCutScene(const char *fileName, bool doReplayCheck);
	static void                endCutScene(bool immediate=false);
	static void                skipCutScene(); // polite user request to skip ahead.
	static bool                isPlayingCutScene();
	static bool                playProfessionMovie(const char *professionName);

	static bool                isOver ();
	static bool                getSinglePlayer ();
	static void                setSinglePlayer (bool b);

	static bool                isClient ();
	static bool                isGodClient ();
	static bool                isParticleEditor ();
	static bool                isAnimationEditor ();

	static void                startChatInput (const Unicode::String & str);

	static void                debugPrintUi (const char * str);
	static void                debugPrintUi (const UnicodeString & str);

	static MessageDispatch::Emitter & getEmitter    ();

	static Object *            getPlayer            ();
	static NetworkId           getPlayerNetworkId   ();
	static const Object *      getConstPlayer       ();
	static PlayerObject *      getPlayerObject      ();
	static const PlayerObject *getConstPlayerObject ();
	// @todo this method should go away when networkscene properly stores a ClientObject (Ships are not ClientObject)
	static ClientObject *      getClientPlayer      ();
	static CreatureObject *    getPlayerCreature    ();
	static ShipObject *        getPlayerPilotedShip ();
	static ShipObject const *  getConstPlayerPilotedShip();
	static ShipObject *        getPlayerContainingShip();
	static bool                isPlayerSquelched    ();
	static bool                isPlayerPermanentlySquelched();

	static bool                getPlayerPath        (std::string & loginId, std::string & clusterName, Unicode::String & playerName, NetworkId & id);
	static void                getLastPlayerInfo    (std::string & loginId, std::string & clusterName, NetworkId & id);
	static bool                playerIsLastPlayer   ();

	static MessageQueue *      getGameMessageQueue  ();
	static InputMap *          getGameInputMap      ();

	static float               getElapsedTime       ();

	static Camera *            getCamera            ();
	static Camera const *      getConstCamera       ();

	static const std::string & getSceneId           ();
	static const std::string & getNonInstanceSceneId();
	static std::string         calculateNonInstanceSceneId(std::string const & scene);

	static bool                isSpace              ();
	static bool                isSpaceSceneName     (std::string const & sceneName);

	static bool                isTutorial           ();

	static bool                isViewFirstPerson    ();
	static bool                isViewFreeCamera();

	static bool                isSceneLoading       ();

	static int                 getLoopCount         ();

	static void                setProfanityFiltered (bool profanityFiltered);
	static bool                isProfanityFiltered  ();

	typedef void (*GameOptionChangedCallback) ();
	static void                setGameOptionChangedCallback (GameOptionChangedCallback const callback);
	static void                gameOptionChanged            ();

	static int                 getReceivedUncompressedBytesPerSecond();
	static int                 getReceivedHistoryUncompressedBytesPerSecond();
	static int                 getReceivedCompressedBytesPerSecond();
	static int                 getReceivedHistoryCompressedBytesPerSecond();
	static int                 getSentUncompressedBytesPerSecond();
	static int                 getSentHistoryUncompressedBytesPerSecond();
	static int                 getSentCompressedBytesPerSecond();
	static int                 getSentHistoryCompressedBytesPerSecond();

	static void                garbageCollect               (bool immediate);

	static SceneType           getHudSceneType();
	static SceneType           getLastHudSceneType();
	static bool                hudSceneTypeIsLastHudSceneType();
	static void                updateHudSceneType();
	static void                emitSceneChange();
	static bool                isHudSceneTypeSpace();

	static std::string         getSceneIdFromTerrainFilename(const char *terrainFilename);

	static void                setRadarRange(float range);
	static float               getRadarRange();

	// Note: only call this for tools apps which want to identify themselves w/o calling Game::install
	static void                setApplication      (Application application);

	typedef void (*ExternalCommandHandler)(const char *);
	static void                setExternalCommandHandler(ExternalCommandHandler const handler);
	static void                externalCommand(const char* name);

	static void resetHud();

	static float getBrightness();
	static void setBrightness(float brightness);
	static float getDefaultBrightness();
	static float getContrast();
	static void setContrast(float contrast);
	static float getDefaultContrast();
	static float getGamma();
	static void setGamma(float gamma);
	static float getDefaultGamma();

	static int getGameFeatureBits();
	static int getSubscriptionFeatureBits();
	static void setGameFeatureBits(int bits);
	static void setSubscriptionFeatureBits(int bits);

	static int getServerSideGameFeatureBits();
	static int getServerSideSubscriptionFeatureBits();
	static void setServerSideFeatureBits(int gameFeatureBits, int subscriptionFeatureBits);

	static int getConnectionServerId();
	static void setConnectionServerId(int id);

	static void requestCollectionServerFirstList();
	static void responseCollectionServerFirstList(const std::string & collectionServerFirstListVersion, const stdset<std::pair<std::pair<int32, std::string>, std::pair<NetworkId, Unicode::String> > >::fwd & collectionServerFirst);
	static void handleCollectionShowServerFirstOptionChanged(bool enabled);

#if PRODUCTION == 0
	static void videoCaptureConfig(int resolution, int seconds, int quality, const char* filename);
	static void videoCaptureStart();
	static void videoCaptureStop();
#endif // PRODUCTION

private:

	Game();
	Game(const Game &);
	Game &operator =(const Game &);

private:

	static void                 _setScene(GameNamespace::SceneCreator &sc, bool immediately);
	static void                 _getCutSceneFromTerrainFilename(char *o_name, const char *terrainFileName, const Vector &startPosition);
	static void                 _startScene();
	static void                 _endCutScene();
	static bool                 _startCutScene(const char *fileName, bool doReplayCheck);

	static Scene*               ms_scene;
	static bool                 ms_singlePlayer;
	static MessageDispatch::Emitter *  ms_emitter;
	static Application          ms_application;

	static int                  ms_loops;
	static bool                 ms_done;

	static float                ms_elapsedTime;
	static std::string          ms_sceneId;
	static std::string          ms_nonInstanceSceneId;

	static bool                 ms_useExitTimer;
	static Timer                ms_exitTimer;

	static NetworkId            ms_lastPlayerId;
	static std::string          ms_lastPlayerCluster;
	static std::string          ms_lastPlayerLoginId;
	static bool                 ms_profanityFiltered;

	static SceneType            ms_lastHudSceneType;

	static float                ms_radarRange;

	static int                  ms_gameFeatureBits;
	static int                  ms_subscriptionFeatureBits;

	static int                  ms_serverSideGameFeatureBits;
	static int                  ms_serverSideSubscriptionFeatureBits;

	static int                  ms_connectionServerId;
};

//-------------------------------------------------------------------

inline Scene* Game::getScene ()
{
	return ms_scene;
}

//-------------------------------------------------------------------

inline const Scene* Game::getConstScene ()
{
	return ms_scene;
}

//-------------------------------------------------------------------

inline bool Game::getSinglePlayer ()
{
	return ms_singlePlayer;
}

//-----------------------------------------------------------------

inline void Game::setSinglePlayer (bool b)
{
	ms_singlePlayer = b;
}

//-----------------------------------------------------------------

inline void Game::setApplication(Application application)
{
	ms_application = application;
}

//-----------------------------------------------------------------

inline bool Game::isClient ()
{
	return ms_application == A_client;
}

//-----------------------------------------------------------------

inline bool Game::isGodClient ()
{
	return ms_application == A_godClient;
}

//-----------------------------------------------------------------

inline bool Game::isParticleEditor ()
{
	return ms_application == A_particleEditor;
}

//-----------------------------------------------------------------

inline bool Game::isAnimationEditor ()
{
	return ms_application == A_animationEditor;
}

//-----------------------------------------------------------------

inline MessageDispatch::Emitter & Game::getEmitter ()
{
	NOT_NULL (ms_emitter);
	return *ms_emitter;
}

//----------------------------------------------------------------------

inline float Game::getElapsedTime ()
{
	return ms_elapsedTime;
}

//----------------------------------------------------------------------

inline const std::string & Game::getSceneId ()
{
	return ms_sceneId;
}

//----------------------------------------------------------------------

inline const std::string & Game::getNonInstanceSceneId()
{
	return ms_nonInstanceSceneId;
}

//----------------------------------------------------------------------

inline int Game::getLoopCount   ()
{
	return ms_loops;
}

// ---------------------------------------------------------------------

inline int Game::getConnectionServerId()
{
	return ms_connectionServerId;
}

// ---------------------------------------------------------------------

inline void Game::setConnectionServerId(int id)
{
	ms_connectionServerId = id;
}

//-----------------------------------------------------------------

#endif
