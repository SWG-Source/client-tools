//========================================================================
//
// ClientFactoryObjectTemplate.h - A wrapper around SharedIntangibleObjectTemplate 
// to create IntangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ClientFactoryObjectTemplate_H
#define _INCLUDED_ClientFactoryObjectTemplate_H

#include "sharedGame/SharedFactoryObjectTemplate.h"

//----------------------------------------------------------------------

class ClientFactoryObjectTemplate : public SharedFactoryObjectTemplate
{
public:
	         ClientFactoryObjectTemplate(const std::string & filename);
	virtual ~ClientFactoryObjectTemplate();

	static void install(bool allowDefaultTemplateParams = false);

	// user functions
	virtual Object *createObject() const;

private:
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ClientFactoryObjectTemplate(const ClientFactoryObjectTemplate &);
	ClientFactoryObjectTemplate & operator =(const ClientFactoryObjectTemplate &);
};

//----------------------------------------------------------------------


#endif	// _INCLUDED_ClientFactoryObjectTemplate_H
