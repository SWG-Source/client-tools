//========================================================================
//
// ClientManufactureSchematicObjectTemplate.h - A wrapper around SharedManufactureSchematicObjectTemplate 
// to create TangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ClientManufactureSchematicObjectTemplate_H
#define _INCLUDED_ClientManufactureSchematicObjectTemplate_H

#include "sharedGame/SharedManufactureSchematicObjectTemplate.h"

//----------------------------------------------------------------------

class ClientManufactureSchematicObjectTemplate : 
public SharedManufactureSchematicObjectTemplate
{
public:
	         ClientManufactureSchematicObjectTemplate(const std::string & filename);
	virtual ~ClientManufactureSchematicObjectTemplate();

	static void install(bool allowDefaultTemplateParams = false);

	// user functions
	virtual Object *createObject() const;

private:
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ClientManufactureSchematicObjectTemplate(const ClientManufactureSchematicObjectTemplate &);
	ClientManufactureSchematicObjectTemplate & operator =(const ClientManufactureSchematicObjectTemplate &);
};

//----------------------------------------------------------------------


#endif	// _INCLUDED_ClientManufactureSchematicObjectTemplate_H
