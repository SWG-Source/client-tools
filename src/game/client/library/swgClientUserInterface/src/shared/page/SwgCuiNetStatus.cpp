//======================================================================
//
// SwgCuiNetStatus.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiNetStatus.h"

#include "UIText.h"
#include "UIPage.h"
#include "UIData.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Production.h"
#include "sharedNetworkMessages/GalaxyLoopTimesResponse.h"
#include "sharedNetworkMessages/RequestGalaxyLoopTimes.h"
#include "clientGraphics/Graphics.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiLoginManagerClusterInfo.h"
#include "clientUserInterface/CuiLoginManagerClusterPing.h"
#include "clientUserInterface/CuiStringIdsServer.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/ConnectionServerConnection.h"
#include "clientGame/Game.h"

//======================================================================

SwgCuiNetStatus::SwgCuiNetStatus  (UIPage & page) :
SwgCuiLockableMediator   ("SwgCuiNetStatus", page),
MessageDispatch::Receiver(),
m_textPing               (0),
m_textPacketLoss         (0),
m_textBandwidth          (0),
m_textFps                (0),
m_textActivity           (0),
m_cachedPing             (-1),
m_cachedPacketLoss       (-1),
m_cachedFps              (-1),
m_elapsedUpdate          (0.0f),
m_performanceClear       (true),
m_timeToRequestGalaxyLoopTime(0)
{
	getCodeDataObject (TUIText, m_textPing,       "textPing");
	getCodeDataObject (TUIText, m_textPacketLoss, "textPacketLoss");
	getCodeDataObject (TUIText, m_textBandwidth,  "textBandwidth");
	getCodeDataObject (TUIText, m_textFps,        "textFps");
	getCodeDataObject (TUIText, m_textActivity,   "textActivity");

	m_textPing->SetPreLocalized       (true);
	m_textPacketLoss->SetPreLocalized (true);
	m_textBandwidth->SetPreLocalized  (true);
	m_textFps->SetPreLocalized        (true);

	if (m_textActivity)
		m_textActivity->SetPreLocalized(true);

	setStickyVisible (true);

	setState (MS_closeable);
	setState (MS_closeDeactivates);

	connectToMessage ("GalaxyLoopTimesResponse");

	registerMediatorObject(page, true);
}

//----------------------------------------------------------------------

SwgCuiNetStatus::~SwgCuiNetStatus ()
{
}

//----------------------------------------------------------------------

void SwgCuiNetStatus::performActivate ()
{
	setIsUpdating (true);
	m_performanceClear = true;
	m_textPing->Clear ();
	m_textPacketLoss->Clear ();
	m_textBandwidth->Clear ();
	CuiLoginManager::setPingEnabled(CuiLoginManager::getConnectedClusterId(), true);
}

//----------------------------------------------------------------------

void SwgCuiNetStatus::performDeactivate ()
{
	setIsUpdating (false);
	CuiLoginManager::setAllPingsDisabled();
}

//----------------------------------------------------------------------

void SwgCuiNetStatus::update                        (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	static char buf[256];
	static const size_t buf_size = sizeof (buf);

	m_elapsedUpdate += deltaTimeSecs;

	updateBandwidth();

	if (m_elapsedUpdate < 0.3f)
		return;

	m_elapsedUpdate = 0.0f;

	const uint32 clusterId = CuiLoginManager::getConnectedClusterId();
	const CuiLoginManager::ClusterInfo * ci = NULL;
	if (clusterId)
	{
		ci = CuiLoginManager::findClusterInfo(clusterId);
		const int ping = static_cast<int>(ci->getLatencyMs());
		if (m_performanceClear || ping != m_cachedPing)
		{
			snprintf (buf, buf_size, "%3d (host id %d)", ping, Game::getConnectionServerId());
			m_textPing->SetLocalText (Unicode::narrowToWide (buf));
			m_cachedPing = ping;
		}
		
		const int packetLoss = 100 - static_cast<int>(ci->getPacketSuccess () * 100.0f);
		if (m_performanceClear || packetLoss != m_cachedPacketLoss)
		{
			snprintf (buf, buf_size, "%3d%%", packetLoss);
			m_textPacketLoss->SetLocalText (Unicode::narrowToWide (buf));
			m_cachedPacketLoss = packetLoss;
		}

		m_performanceClear = false;
	}
	else if (!m_performanceClear)
	{
		m_performanceClear = true;
		m_textPing->Clear ();
		m_textPacketLoss->Clear ();
	}
	
	const int fps = static_cast<int>(Clock::framesPerSecond () + 0.5);
		
	if (fps != m_cachedFps) //lint !e777 testing floats for equality, this is alright since we're only trying to determine if we need to paint new test, rounding errors don't matter
	{
		m_cachedFps = fps;
		
		snprintf (buf, buf_size, "%d", fps);
		m_textFps->SetLocalText (Unicode::narrowToWide (buf));
	}

	// periodically request server loop time
	const time_t timeNow = ::time(NULL);
	if (timeNow >= m_timeToRequestGalaxyLoopTime)
	{
		RequestGalaxyLoopTimes request;
		GameNetwork::send(request, true);
		m_timeToRequestGalaxyLoopTime = timeNow + 10;

		// make sure that ping to the cluster is enabled
		if (ci && !ci->getPing().isEnabled())
		{
			CuiLoginManager::setPingEnabled(clusterId, true);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiNetStatus::updateBandwidth()
{
	static int previousReceivedCompressedBytesPerSecond = 0;
	static int previousSentCompressedBytesPerSecond = 0;

	// Only update the meter when there is a change in the values

	if (   (previousReceivedCompressedBytesPerSecond != Game::getReceivedCompressedBytesPerSecond())
	    || (previousSentCompressedBytesPerSecond != Game::getSentCompressedBytesPerSecond()))
	{
		previousReceivedCompressedBytesPerSecond = Game::getReceivedCompressedBytesPerSecond();
		previousSentCompressedBytesPerSecond = Game::getSentCompressedBytesPerSecond();

		char text[256];
		snprintf(text, sizeof(text), "%1.1f (%1.1f)/%1.1f (%1.1f)",
		    Game::getReceivedCompressedBytesPerSecond() / 1024.0f,
		    Game::getReceivedHistoryCompressedBytesPerSecond() / 1024.0f,
		    Game::getSentCompressedBytesPerSecond() / 1024.0f,
		    Game::getSentHistoryCompressedBytesPerSecond() / 1024.0f);
		m_textBandwidth->SetLocalText(Unicode::narrowToWide(text));
	}
}

//----------------------------------------------------------------------

void SwgCuiNetStatus::receiveMessage(const MessageDispatch::Emitter &, const MessageDispatch::MessageBase & message)
{
	if (message.isType("GalaxyLoopTimesResponse"))
	{
		Archive::ReadIterator ri = dynamic_cast<const GameNetworkMessage &>(message).getByteStream().begin();
		const GalaxyLoopTimesResponse cg(ri);

		// loop time (in ms) * 3193
		// don't convert the obfuscated loop time value in the release
		// build, because it will expose the actual loop time
		const unsigned long serverLoopTime = cg.getLastFrameMilliseconds();
		Unicode::String serverLoad;

		// extremely heavy (>= 2000ms)
		if (serverLoopTime >= 6386000) // 6386000 = 2000ms * 3193
			serverLoad = CuiStringIdsServer::server_cluster_table_population_extremely_heavy.localize();
		
		// very heavy      (1500ms - 1999ms)
		else if (serverLoopTime >= 4789500) // 4789500 = 1500ms * 3193
			serverLoad = CuiStringIdsServer::server_cluster_table_population_very_heavy.localize();

		// heavy           (1000ms - 1499ms)
		else if (serverLoopTime >= 3193000) // 3193000 = 1000ms * 3193
			serverLoad = CuiStringIdsServer::server_cluster_table_population_heavy.localize();

		// medium          (667ms -  999ms)
		else if (serverLoopTime >= 2129731) // 2129731 = 667ms * 3193
			serverLoad = CuiStringIdsServer::server_cluster_table_population_medium.localize();

		// light           (334ms -  666ms)
		else if (serverLoopTime >= 1066462) // 1066462 = 334ms * 3193
			serverLoad = CuiStringIdsServer::server_cluster_table_population_light.localize();

		// very light      (<= 333ms)
		else
			serverLoad = CuiStringIdsServer::server_cluster_table_population_very_light.localize();

		// in debug client, include actual server loop time in display
#if PRODUCTION == 0
		char buf [256];
		_snprintf (buf, sizeof(buf), " (%lu ms)", (serverLoopTime / 3193)); // loop time (in ms) * 3193
		serverLoad += Unicode::narrowToWide(buf);
#endif

		if (m_textActivity)
			m_textActivity->SetLocalText(serverLoad);
	}
}

//======================================================================
