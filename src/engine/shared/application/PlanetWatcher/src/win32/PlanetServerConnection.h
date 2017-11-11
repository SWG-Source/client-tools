// ======================================================================
//
// PlanetServerConnection.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PlanetServerConnection_H
#define INCLUDED_PlanetServerConnection_H

// ======================================================================

#include "sharedNetwork/Connection.h"

class PlanetWatcher;

// ======================================================================

class PlanetServerConnection : public Connection
{
public:
	PlanetServerConnection   (const std::string & remoteAddress, const unsigned short port, PlanetWatcher *watcher);
	~PlanetServerConnection  ();

	void          onConnectionClosed      ();
	void          onConnectionOpened      ();

	virtual void  onReceive               (const Archive::ByteStream & bs);

	bool          gotUpdates              () const;
	void          clearUpdatesFlag        ();

private:
	PlanetServerConnection (const PlanetServerConnection&);
	PlanetServerConnection& operator= (const PlanetServerConnection&);

	PlanetWatcher *m_watcher;
	bool           m_gotUpdates;
};


inline bool PlanetServerConnection::gotUpdates() const
{
	return m_gotUpdates;
}

inline void PlanetServerConnection::clearUpdatesFlag()
{
	m_gotUpdates = false;
}

// ======================================================================

#endif
