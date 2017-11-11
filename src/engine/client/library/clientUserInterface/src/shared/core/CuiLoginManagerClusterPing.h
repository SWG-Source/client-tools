//======================================================================
//
// CuiLoginManagerClusterPing.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiLoginManagerClusterPing_H
#define INCLUDED_CuiLoginManagerClusterPing_H

//======================================================================

class CuiLoginManagerClusterPing
{
public:
	enum 
	{
		DefaultWindowSize = 100
	};

	typedef stdmap<uint32, uint32>::fwd PingMap;

	explicit                      CuiLoginManagerClusterPing  (int windowSize = DefaultWindowSize);
	                              CuiLoginManagerClusterPing (const CuiLoginManagerClusterPing & rhs);
	CuiLoginManagerClusterPing &  operator= (const CuiLoginManagerClusterPing & rhs);
	                             ~CuiLoginManagerClusterPing ();

	uint32   onSendingPing  ();
	void     onReceivedPing (uint32 timestamp);

	uint32   getLatency       () const;
	float    getPacketSuccess () const;

	bool     isEnabled        () const;
	void     setEnabled       (bool b);

private:

	void    recompute () const;

	uint32            m_windowSize;
	PingMap *         m_pingMap;
	mutable uint32    m_latency;
	mutable float     m_packetSuccess;
	mutable bool      m_dirty;

	bool              m_enabled;
};

//----------------------------------------------------------------------

inline uint32  CuiLoginManagerClusterPing::getLatency       () const
{
	if (m_dirty)
		recompute ();

	return m_latency;
}

//----------------------------------------------------------------------

inline float   CuiLoginManagerClusterPing::getPacketSuccess () const
{
	if (m_dirty)
		recompute ();

	return m_packetSuccess;
}

//----------------------------------------------------------------------

inline bool CuiLoginManagerClusterPing::isEnabled        () const
{
	return m_enabled;
}

//----------------------------------------------------------------------

inline void CuiLoginManagerClusterPing::setEnabled       (bool b)
{
	m_enabled = b;
}

//======================================================================

#endif
