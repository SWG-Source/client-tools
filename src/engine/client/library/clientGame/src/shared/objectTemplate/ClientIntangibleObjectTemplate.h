//========================================================================
//
// ClientIntangibleObjectTemplate.h - A wrapper around SharedIntangibleObjectTemplate 
// to create IntangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ClientIntangibleObjectTemplate_H
#define _INCLUDED_ClientIntangibleObjectTemplate_H

#include "sharedGame/SharedIntangibleObjectTemplate.h"

//----------------------------------------------------------------------

class ClientIntangibleObjectTemplate : public SharedIntangibleObjectTemplate
{
public:
	         ClientIntangibleObjectTemplate(const std::string & filename);
	virtual ~ClientIntangibleObjectTemplate();

	static void install(bool allowDefaultTemplateParams = false);

	// user functions
	virtual Object *createObject() const;

private:
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ClientIntangibleObjectTemplate(const ClientIntangibleObjectTemplate &);
	ClientIntangibleObjectTemplate & operator =(const ClientIntangibleObjectTemplate &);
};

//----------------------------------------------------------------------


#endif	// _INCLUDED_ClientIntangibleObjectTemplate_H
