//========================================================================
//
// ClientCellObjectTemplate.h - A wrapper around SharedCellObjectTemplate 
// to create CellObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ClientCellObjectTemplate_H
#define _INCLUDED_ClientCellObjectTemplate_H

#include "sharedGame/SharedCellObjectTemplate.h"

class ClientCellObjectTemplate : public SharedCellObjectTemplate
{
public:
	         ClientCellObjectTemplate(const std::string & filename);
	virtual ~ClientCellObjectTemplate();

	static void install(bool allowDefaultTemplateParams = false);

	// user functions
	virtual Object *createObject(void) const;

private:
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ClientCellObjectTemplate(const ClientCellObjectTemplate &);
	ClientCellObjectTemplate & operator =(const ClientCellObjectTemplate &);
};


#endif	// _INCLUDED_ClientCellObjectTemplate_H
