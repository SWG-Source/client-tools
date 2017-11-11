//===================================================================
//
// ClientNoBuildNotification.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientNoBuildNotification_H
#define INCLUDED_ClientNoBuildNotification_H

//===================================================================

#include "sharedGame/NoBuildNotification.h"

//===================================================================

class ClientNoBuildNotification : public NoBuildNotification
{
public:

	static void install ();
	static ClientNoBuildNotification& getInstance ();

public:

	ClientNoBuildNotification ();
	virtual ~ClientNoBuildNotification ();

	virtual void addToWorld (Object& object) const;
	virtual void removeFromWorld (Object& object) const;

private:

	ClientNoBuildNotification (const ClientNoBuildNotification&);
	ClientNoBuildNotification& operator= (const ClientNoBuildNotification&);

private:

	static ClientNoBuildNotification ms_instance;
};

//===================================================================

#endif
