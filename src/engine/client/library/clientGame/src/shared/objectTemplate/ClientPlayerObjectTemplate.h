//========================================================================
//
// ClientPlayerObjectTemplate.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_ClientPlayerObjectTemplate_H
#define	_INCLUDED_ClientPlayerObjectTemplate_H

//-----------------------------------------------------------------------

#include "sharedGame/SharedPlayerObjectTemplate.h"

//-----------------------------------------------------------------------

class ClientPlayerObjectTemplate : public SharedPlayerObjectTemplate
{
public:
	ClientPlayerObjectTemplate(const std::string & filename);
	virtual ~ClientPlayerObjectTemplate();

	static void install(bool allowDefaultTemplateParams = false);

	virtual Object * createObject() const;

private:
	ClientPlayerObjectTemplate & operator = (const ClientPlayerObjectTemplate & rhs);
	ClientPlayerObjectTemplate(const ClientPlayerObjectTemplate & source);

	static ObjectTemplate * create(const std::string & filename);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ClientPlayerObjectTemplate_H
