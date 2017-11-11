//========================================================================
//
// ClientStaticObjectTemplate.h - A wrapper around SharedStaticObjectTemplate 
// to create StaticObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ClientStaticObjectTemplate_H
#define _INCLUDED_ClientStaticObjectTemplate_H

#include "sharedGame/SharedStaticObjectTemplate.h"


class ClientStaticObjectTemplate : public SharedStaticObjectTemplate
{
public:
	         ClientStaticObjectTemplate(const std::string & filename);
	virtual ~ClientStaticObjectTemplate();

	static void install(bool allowDefaultTemplateParams = false);

	// user functions
	virtual Object *createObject(void) const;

private:
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ClientStaticObjectTemplate(const ClientStaticObjectTemplate &);
	ClientStaticObjectTemplate & operator =(const ClientStaticObjectTemplate &);
};


#endif	// _INCLUDED_ClientStaticObjectTemplate_H
