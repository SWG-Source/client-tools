//========================================================================
//
// ClientCreatureObjectTemplate.h - A wrapper around SharedCreatureObjectTemplate 
// to create TangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ClientCreatureObjectTemplate_H
#define _INCLUDED_ClientCreatureObjectTemplate_H

#include "sharedGame/SharedCreatureObjectTemplate.h"


class ClientCreatureObjectTemplate : public SharedCreatureObjectTemplate
{
public:
	         ClientCreatureObjectTemplate(const std::string & filename);
	virtual ~ClientCreatureObjectTemplate();

	static void install(bool allowDefaultTemplateParams = false);

	// user functions
	virtual Object *createObject(void) const;

private:
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ClientCreatureObjectTemplate(const ClientCreatureObjectTemplate &);
	ClientCreatureObjectTemplate & operator =(const ClientCreatureObjectTemplate &);
};


#endif	// _INCLUDED_ClientCreatureObjectTemplate_H
