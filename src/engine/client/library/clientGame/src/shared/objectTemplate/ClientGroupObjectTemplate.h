// ClientGroupObjectTemplate.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ClientGroupObjectTemplate_H
#define	_INCLUDED_ClientGroupObjectTemplate_H

//-----------------------------------------------------------------------

#include "sharedGame/SharedGroupObjectTemplate.h"

//-----------------------------------------------------------------------

class ClientGroupObjectTemplate : public SharedGroupObjectTemplate
{
public:
	ClientGroupObjectTemplate(const std::string & filename);
	virtual ~ClientGroupObjectTemplate();

	static void install();

	virtual Object * createObject() const;

private:
	ClientGroupObjectTemplate & operator = (const ClientGroupObjectTemplate & rhs);
	ClientGroupObjectTemplate(const ClientGroupObjectTemplate & source);

	static ObjectTemplate * create(const std::string & filename);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ClientGroupObjectTemplate_H
