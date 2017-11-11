// ClientMissionObjectTemplate.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ClientMissionObjectTemplate_H
#define	_INCLUDED_ClientMissionObjectTemplate_H

//-----------------------------------------------------------------------

#include "sharedGame/SharedMissionObjectTemplate.h"

class Object;

//-----------------------------------------------------------------------

class ClientMissionObjectTemplate : public SharedMissionObjectTemplate
{
public:
	ClientMissionObjectTemplate(const std::string & filename);
	virtual ~ClientMissionObjectTemplate();

	static void       install       ();
	virtual Object *  createObject  () const;

private:
	ClientMissionObjectTemplate & operator = (const ClientMissionObjectTemplate & rhs);
	ClientMissionObjectTemplate(const ClientMissionObjectTemplate & source);

	static ObjectTemplate *  create(const std::string & filename);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ClientMissionObjectTemplate_H
