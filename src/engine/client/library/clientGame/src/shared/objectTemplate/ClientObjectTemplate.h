//========================================================================
//
// ClientObjectTemplate.h - A wrapper around SharedObjectTemplate to create 
// ClientObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ClientObjectTemplate_H
#define _INCLUDED_ClientObjectTemplate_H

#include "sharedGame/SharedObjectTemplate.h"


class ClientObjectTemplate : public SharedObjectTemplate
{
public:
	         ClientObjectTemplate(const std::string & filename);
	virtual ~ClientObjectTemplate();

	static void install(bool allowDefaultTemplateParams = false);

	// user functions
	virtual Object *createObject(void) const;

private:
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ClientObjectTemplate(const ClientObjectTemplate &);
	ClientObjectTemplate & operator =(const ClientObjectTemplate &);
};


#endif	// _INCLUDED_ClientObjectTemplate_H
