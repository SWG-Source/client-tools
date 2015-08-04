// NetworkScene.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------
#include "clientGame/FirstClientGame.h"
#include "clientGame/NetworkScene.h"

#include "UnicodeUtils.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ClientWorld.h"
#include "clientGame/ConnectionManager.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiObjectTextManager.h"
#include "sharedFoundation/Timer.h"

//-----------------------------------------------------------------------

NetworkScene::NetworkScene(const char * debugName, Object * newPlayer) :
Scene(debugName),
m_player(newPlayer),
m_netBytesSentAfterCompressionPeakValue(1),
m_netBytesSentBeforeCompressionPeakValue(1),
m_netBytesReceivedAfterDecompressionPeakValue(1),
m_netBytesReceivedBeforeDecompressionPeakValue(1),
m_lastBytesReceivedAfterDecompression(0),
m_lastBytesReceivedBeforeDecompression(0),
m_lastBytesSentAfterCompression(0),
m_lastBytesSentBeforeCompression(0),
m_drawNetGraph(false),
m_netGraphUpdateTimer(new Timer(CONST_REAL(1.0))),
m_started(false)
{
	UNREF(newPlayer);
}

//-----------------------------------------------------------------------

NetworkScene::~NetworkScene()
{

	toggleNetGraph(false);
	delete m_netGraphUpdateTimer;
}

//-----------------------------------------------------------------------

IoResult NetworkScene::processEvent(IoEvent *event)
{
	switch (event->type)
	{
	case IOET_Update:
		{
			const real elapsedTime = event->arg3;
			updateNetStatistics(elapsedTime);
		}
		break;
	case IOET_Character:
		{
			if (event->arg2 == 'N' || event->arg2 == 'n')
			{
				toggleNetGraph(!m_drawNetGraph);
			}
		}
		break;
	default:
		break;
	}
	return IOR_Pass;
}

//-----------------------------------------------------------------------

void NetworkScene::start(void)
{
	toggleNetGraph(m_drawNetGraph);

	m_started = true;
}

//-----------------------------------------------------------------------

void NetworkScene::toggleNetGraph(bool on)
{
	if(m_started)
	{
		m_drawNetGraph = on;
	}
}

//-----------------------------------------------------------------------

void NetworkScene::quit(void)
{
//	GameNetwork::getConnectionManager().disconnectGameServer();

	Scene::quit();
}

//-----------------------------------------------------------------------

void NetworkScene::updateNetStatistics(const real elapsedTime)
{
	UNREF(elapsedTime);

/*
	if(netGraphUpdateTimer->updateZero(elapsedTime) && ConfigClientGame::getUseNetwork() && started)
	{
		uint32	delta;
		real	graphValue;

		char	labelTextC[32];
		EString	labelText;

		// client bytes sent to server before data is compressed
		delta = Net::getBytesSentBeforeCompression() - lastBytesSentBeforeCompression;
		if(delta > netBytesSentBeforeCompressionPeakValue)
			netBytesSentBeforeCompressionPeakValue = delta;
		graphValue = static_cast<float>(static_cast<float>(delta) / static_cast<float>(netBytesSentBeforeCompressionPeakValue));
		lastBytesSentBeforeCompression = Net::getBytesSentBeforeCompression();
		netGraphBytesSentBeforeCompression->addValue(graphValue);
		snprintf(labelTextC, 32, "SentPreCmp PK %i, CR %i", netBytesSentBeforeCompressionPeakValue, delta);
		labelText.replaceString(labelTextC);
		netLabelBytesSentBeforeCompression->setLabelText(&labelText);

		// server bytes sent to client after data is decompressed
		delta = Net::getBytesReceivedAfterDecompression() - lastBytesReceivedAfterDecompression;
		if(delta > netBytesReceivedAfterDecompressionPeakValue)
			netBytesReceivedAfterDecompressionPeakValue = delta;
		graphValue = static_cast<float>(static_cast<float>(delta) / static_cast<float>(netBytesReceivedAfterDecompressionPeakValue));
		lastBytesReceivedAfterDecompression = Net::getBytesReceivedAfterDecompression();
		netGraphBytesReceivedAfterDecompression->addValue(graphValue);
		snprintf(labelTextC, 32, "RecvPostDeCmp PK %i, CR %i", netBytesReceivedAfterDecompressionPeakValue, delta);
		labelText.replaceString(labelTextC);
		netLabelBytesReceivedAfterDecompression->setLabelText(&labelText);

		// server bytes sent to client after data is decompressed
		delta = Net::getBytesSentAfterCompression() - lastBytesSentAfterCompression;
		if(delta > netBytesSentAfterCompressionPeakValue)
			netBytesSentAfterCompressionPeakValue = delta;
		graphValue = static_cast<float>(static_cast<float>(delta) / static_cast<float>(netBytesSentAfterCompressionPeakValue));
		lastBytesSentAfterCompression = Net::getBytesSentAfterCompression();
		netGraphBytesSentAfterCompression->addValue(graphValue);
		snprintf(labelTextC, 32, "SentPostCmp PK %i, CR %i", netBytesSentAfterCompressionPeakValue, delta);
		labelText.replaceString(labelTextC);
		netLabelBytesSentAfterCompression->setLabelText(&labelText);

		// server bytes sent to client after data is decompressed
		delta = Net::getBytesReceivedBeforeDecompression() - lastBytesReceivedBeforeDecompression;
		if(delta > netBytesReceivedBeforeDecompressionPeakValue)
			netBytesReceivedBeforeDecompressionPeakValue = delta;
		graphValue = static_cast<float>(static_cast<float>(delta) / static_cast<float>(netBytesReceivedBeforeDecompressionPeakValue));
		lastBytesReceivedBeforeDecompression = Net::getBytesReceivedBeforeDecompression();
		netGraphBytesReceivedBeforeDecompression->addValue(graphValue);
		snprintf(labelTextC, 32, "RecvPreDeCmp PK %i, CR %i", netBytesReceivedBeforeDecompressionPeakValue, delta);
		labelText.replaceString(labelTextC);
		netLabelBytesReceivedBeforeDecompression->setLabelText(&labelText);

		snprintf(labelTextC, sizeof(labelTextC), "Total Sent PreComp %i", Net::getBytesSentBeforeCompression());
		labelText.replaceString(labelTextC);
		netLabelTotalBytesSentBeforeCompression->setLabelText(&labelText);

		
		snprintf(labelTextC, sizeof(labelTextC), "Total Sent PostComp %i", Net::getBytesSentAfterCompression());
		labelText.replaceString(labelTextC);
		netLabelTotalBytesSentAfterCompression->setLabelText(&labelText);

		snprintf(labelTextC, sizeof(labelTextC), "Total Recv PreDeComp %i", Net::getBytesReceivedBeforeDecompression());
		labelText.replaceString(labelTextC);
		netLabelTotalBytesReceivedBeforeDeCompression->setLabelText(&labelText);
		
		snprintf(labelTextC, sizeof(labelTextC), "Total Recv PostDeComp %i", Net::getBytesReceivedAfterDecompression());
		labelText.replaceString(labelTextC);
		netLabelTotalBytesReceivedAfterDeCompresssion->setLabelText(&labelText);
	}
*/
}

//-----------------------------------------------------------------

void NetworkScene::drawNetworkIds (const Camera & camera) const
{
	CuiObjectTextManager::drawObjectLabels (camera);
}

//-----------------------------------------------------------------------
