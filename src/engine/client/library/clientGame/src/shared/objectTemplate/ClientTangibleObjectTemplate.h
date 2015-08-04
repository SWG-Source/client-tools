//========================================================================
//
// ClientTangibleObjectTemplate.h - A wrapper around SharedTangibleObjectTemplate 
// to create TangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ClientTangibleObjectTemplate_H
#define _INCLUDED_ClientTangibleObjectTemplate_H

#include "sharedGame/SharedTangibleObjectTemplate.h"

//========================================================================

class ClientTangibleObjectTemplate : public SharedTangibleObjectTemplate
{
public:

	static void install(bool allowDefaultTemplateParams = false);

	static void applyComponents (Object* object, const stdset<int>::fwd& componentList);

public:

	explicit ClientTangibleObjectTemplate(const std::string & filename);
	virtual ~ClientTangibleObjectTemplate();

	virtual Object *createObject(void) const;

private:

	static ObjectTemplate * create(const std::string & filename);

private:

	ClientTangibleObjectTemplate(const ClientTangibleObjectTemplate &);
	ClientTangibleObjectTemplate & operator =(const ClientTangibleObjectTemplate &);
};

//========================================================================

#endif	
