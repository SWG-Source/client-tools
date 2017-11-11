// ClientWaypointObjectTemplate.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ClientWaypointObjectTemplate_H
#define	_INCLUDED_ClientWaypointObjectTemplate_H

//-----------------------------------------------------------------------

#include "sharedGame/SharedWaypointObjectTemplate.h"

//-----------------------------------------------------------------------

class ClientWaypointObjectTemplate : public SharedWaypointObjectTemplate
{
public:
	ClientWaypointObjectTemplate(const std::string & filename);
	virtual ~ClientWaypointObjectTemplate();

	virtual Object *  createObject  () const;
	static void       install       ();

private:
	ClientWaypointObjectTemplate & operator = (const ClientWaypointObjectTemplate & rhs);
	ClientWaypointObjectTemplate(const ClientWaypointObjectTemplate & source);
	static ObjectTemplate *  create(const std::string & filename);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ClientWaypointObjectTemplate_H
