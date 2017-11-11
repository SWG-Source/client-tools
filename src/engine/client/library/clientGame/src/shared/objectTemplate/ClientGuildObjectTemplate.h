// ======================================================================
//
// ClientGuildObjectTemplate.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_ClientGuildObjectTemplate_H
#define	_INCLUDED_ClientGuildObjectTemplate_H

// ======================================================================

#include "sharedGame/SharedGuildObjectTemplate.h"

// ======================================================================

class ClientGuildObjectTemplate: public SharedGuildObjectTemplate
{
public:
	ClientGuildObjectTemplate(std::string const &filename);
	virtual ~ClientGuildObjectTemplate();

	static void install();

	virtual Object *createObject() const;

private:
	ClientGuildObjectTemplate(ClientGuildObjectTemplate const &);
	ClientGuildObjectTemplate &operator=(ClientGuildObjectTemplate const &);

	static ObjectTemplate *create(std::string const &filename);
};

// ======================================================================

#endif	// _INCLUDED_ClientGuildObjectTemplate_H

