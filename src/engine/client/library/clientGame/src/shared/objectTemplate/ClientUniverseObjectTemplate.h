//========================================================================
//
// ClientUniverseObjectTemplate.h - A wrapper around SharedUniverseObjectTemplate 
// to create UniverseObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ClientUniverseObjectTemplate_H
#define _INCLUDED_ClientUniverseObjectTemplate_H

#include "sharedGame/SharedUniverseObjectTemplate.h"

//----------------------------------------------------------------------

class ClientUniverseObjectTemplate : public SharedUniverseObjectTemplate
{
public:
	         ClientUniverseObjectTemplate(const std::string & filename);
	virtual ~ClientUniverseObjectTemplate();

	static void install(bool allowDefaultTemplateParams = false);

	// user functions
	virtual Object *createObject() const;

private:
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ClientUniverseObjectTemplate(const ClientUniverseObjectTemplate &);
	ClientUniverseObjectTemplate & operator =(const ClientUniverseObjectTemplate &);
};

//----------------------------------------------------------------------


#endif	// _INCLUDED_ClientUniverseObjectTemplate_H
