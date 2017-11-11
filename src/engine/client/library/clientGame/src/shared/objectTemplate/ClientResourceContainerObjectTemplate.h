//========================================================================
//
// ClientResourceContainerObjectTemplate.h - A wrapper around SharedResourceContainerObjectTemplate 
// to create TangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ClientResourceContainerObjectTemplate_H
#define _INCLUDED_ClientResourceContainerObjectTemplate_H

#include "sharedGame/SharedResourceContainerObjectTemplate.h"


class ClientResourceContainerObjectTemplate : public SharedResourceContainerObjectTemplate
{
public:
	         ClientResourceContainerObjectTemplate(const std::string & filename);
	virtual ~ClientResourceContainerObjectTemplate();

	static void install(bool allowDefaultTemplateParams = false);

	// user functions
	virtual Object *createObject(void) const;

private:
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ClientResourceContainerObjectTemplate(const ClientResourceContainerObjectTemplate &);
	ClientResourceContainerObjectTemplate & operator =(const ClientResourceContainerObjectTemplate &);
};


#endif	// _INCLUDED_ClientResourceContainerObjectTemplate_H
