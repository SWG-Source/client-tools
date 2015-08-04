// ======================================================================
//
// ClientBattlefieldMarkerObjectTemplate.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_ClientBattlefieldMarkerObjectTemplate_H
#define	_INCLUDED_ClientBattlefieldMarkerObjectTemplate_H

// ======================================================================

#include "sharedGame/SharedBattlefieldMarkerObjectTemplate.h"

// ======================================================================

class ClientBattlefieldMarkerObjectTemplate: public SharedBattlefieldMarkerObjectTemplate
{
public:
	ClientBattlefieldMarkerObjectTemplate(std::string const &filename);
	virtual ~ClientBattlefieldMarkerObjectTemplate();

	static void install();

	virtual Object *createObject() const;

private:
	ClientBattlefieldMarkerObjectTemplate(ClientBattlefieldMarkerObjectTemplate const &);
	ClientBattlefieldMarkerObjectTemplate &operator=(ClientBattlefieldMarkerObjectTemplate const &);

	static ObjectTemplate *create(std::string const &filename);
};

// ======================================================================

#endif	// _INCLUDED_ClientBattlefieldMarkerObjectTemplate_H

