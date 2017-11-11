// NetworkScene.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_NetworkScene_H
#define	_NetworkScene_H

//-----------------------------------------------------------------------

#include "sharedMessageDispatch/Receiver.h"
#include "clientGame/Scene.h"

//-----------------------------------------------------------------------

class Object;
class Timer;
class Camera;

//-----------------------------------------------------------------------
/**
	The NetworkScene inherits messageHandler and defines the 
	onMessageReceived interface for derived scenes. When a remote has a
	scene-related message to send, it will arrive via a derived 
	NetworkScene's onMessageReceived() implementation. E.g. a server
	is creating an object, the object package is sent to the scene for
	instantiation.
*/
class NetworkScene : 
public Scene,
public MessageDispatch::Receiver
{
public:
	explicit                NetworkScene      (const char * debugName, Object * newPlayer=0);
	virtual                ~NetworkScene      ();
	virtual IoResult        processEvent      (IoEvent *event);
	virtual void            quit              (void); 
	
	const Object*    getPlayer         (void) const;
	Object*          getPlayer         (void);
	
	void                    setDrawNetGraph   (bool drawNetGraph);

protected:
	void                    setPlayer         (Object* newPlayer);
	void                    start             (void);
	void                    toggleNetGraph    (bool on);
	void                    drawNetworkIds    (const Camera & ) const;
	
private:
	Object*          m_player;
	
#if 0
	GraphWidget *           m_netGraphBytesSentBeforeCompression;
	GraphWidget *           m_netGraphBytesSentAfterCompression;
	GraphWidget *           m_netGraphBytesReceivedAfterDecompression;
	GraphWidget *           m_netGraphBytesReceivedBeforeDecompression;
	
	Widget *                m_netLabelBytesSentBeforeCompression;
	Widget *                m_netLabelBytesSentAfterCompression;
	Widget *                m_netLabelBytesReceivedBeforeDecompression;
	Widget *                m_netLabelBytesReceivedAfterDecompression;
	Widget *                m_netLabelTotalBytesSentBeforeCompression;
	Widget *                m_netLabelTotalBytesSentAfterCompression;
	Widget *                m_netLabelTotalBytesReceivedBeforeDeCompression;
	Widget *                m_netLabelTotalBytesReceivedAfterDeCompresssion;
#endif

	uint32                  m_netBytesSentBeforeCompressionPeakValue;
	uint32                  m_netBytesSentAfterCompressionPeakValue;
	uint32                  m_netBytesReceivedAfterDecompressionPeakValue;
	uint32                  m_netBytesReceivedBeforeDecompressionPeakValue;
	uint32                  m_lastBytesReceivedAfterDecompression;
	uint32                  m_lastBytesReceivedBeforeDecompression;
	uint32                  m_lastBytesSentAfterCompression;
	uint32                  m_lastBytesSentBeforeCompression;
	bool                    m_drawNetGraph;
	Timer *                 m_netGraphUpdateTimer;
	bool                    m_started;
	
private:                    
	NetworkScene        ();
	NetworkScene        (const NetworkScene & other);
	NetworkScene &      operator=            (const NetworkScene & other);
	void                updateNetStatistics  (const real elapsedTime);
};

//-----------------------------------------------------------------------

inline const Object* NetworkScene::getPlayer(void) const
{
	return m_player;
}

//-------------------------------------------------------------------

inline Object* NetworkScene::getPlayer(void) 
{
	return m_player;
}

//-------------------------------------------------------------------

inline void NetworkScene::setPlayer (Object* newPlayer)
{
	m_player = newPlayer;
}

//-------------------------------------------------------------------

#endif	// _NetworkScene_H