//========================================================================
//
// ClientInstallationObjectTemplate.h - A wrapper around SharedInstallationObjectTemplate 
// to create TangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ClientInstallationObjectTemplate_H
#define _INCLUDED_ClientInstallationObjectTemplate_H

#include "sharedGame/SharedInstallationObjectTemplate.h"


class ClientInstallationObjectTemplate : public SharedInstallationObjectTemplate
{
public:
	         ClientInstallationObjectTemplate(const std::string & filename);
	virtual ~ClientInstallationObjectTemplate();

	static void install(bool allowDefaultTemplateParams = false);

	// user functions
	virtual Object *createObject(void) const;

private:
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ClientInstallationObjectTemplate(const ClientInstallationObjectTemplate &);
	ClientInstallationObjectTemplate & operator =(const ClientInstallationObjectTemplate &);
};


#endif	// _INCLUDED_ClientInstallationObjectTemplate_H
