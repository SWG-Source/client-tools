//===================================================================
//
// ClientLotManagerNotification.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientLotManagerNotification_H
#define INCLUDED_ClientLotManagerNotification_H

//===================================================================

#include "sharedGame/LotManagerNotification.h"

//===================================================================

class ClientLotManagerNotification : public LotManagerNotification
{
public:

	static ClientLotManagerNotification& getInstance ();

public:

	ClientLotManagerNotification ();
	virtual ~ClientLotManagerNotification ();

	virtual void addToWorld (Object& object) const;

private:

	ClientLotManagerNotification (const ClientLotManagerNotification&);
	ClientLotManagerNotification& operator= (const ClientLotManagerNotification&);

private:

	static ClientLotManagerNotification ms_instance;
};

//===================================================================

#endif
