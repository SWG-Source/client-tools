// ClientPlayerQuestObjectTemplate.h
// Copyright 2009, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Boudreaux

#ifndef	_INCLUDED_ClientPlayerQuestObjectTemplate_H
#define	_INCLUDED_ClientPlayerQuestObjectTemplate_H

//-----------------------------------------------------------------------

#include "sharedGame/SharedPlayerQuestObjectTemplate.h"

class Object;

//-----------------------------------------------------------------------

class ClientPlayerQuestObjectTemplate : public SharedPlayerQuestObjectTemplate
{
public:
	ClientPlayerQuestObjectTemplate(const std::string & filename);
	virtual ~ClientPlayerQuestObjectTemplate();

	static void       install       ();
	virtual Object *  createObject  () const;

private:
	ClientPlayerQuestObjectTemplate & operator = (const ClientPlayerQuestObjectTemplate & rhs);
	ClientPlayerQuestObjectTemplate(const ClientPlayerQuestObjectTemplate & source);

	static ObjectTemplate *  create(const std::string & filename);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ClientPlayerQuestObjectTemplate_H
