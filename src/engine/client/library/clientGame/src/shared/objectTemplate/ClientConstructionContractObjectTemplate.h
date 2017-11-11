// ClientConstructionContractObjectTemplate.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ClientConstructionContractObjectTemplate_H
#define	_INCLUDED_ClientConstructionContractObjectTemplate_H

//-----------------------------------------------------------------------

#include "sharedGame/SharedConstructionContractObjectTemplate.h"

//-----------------------------------------------------------------------

class ClientConstructionContractObjectTemplate : public SharedConstructionContractObjectTemplate
{
public:
	ClientConstructionContractObjectTemplate(const std::string & filename);
	~ClientConstructionContractObjectTemplate();

	static void install();

	virtual Object * createObject() const;

private:
	static ObjectTemplate * create(const std::string & filename);

	ClientConstructionContractObjectTemplate & operator = (const ClientConstructionContractObjectTemplate & rhs);
	ClientConstructionContractObjectTemplate(const ClientConstructionContractObjectTemplate & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ClientConstructionContractObjectTemplate_H
