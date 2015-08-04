//========================================================================
//
// ClientVehicleObjectTemplate.h - A wrapper around SharedVehicleObjectTemplate 
// to create TangibleObjects.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ClientVehicleObjectTemplate_H
#define _INCLUDED_ClientVehicleObjectTemplate_H

#include "sharedGame/SharedVehicleObjectTemplate.h"


class ClientVehicleObjectTemplate : public SharedVehicleObjectTemplate
{
public:
	         ClientVehicleObjectTemplate(const std::string & filename);
	virtual ~ClientVehicleObjectTemplate();

	static void install(bool allowDefaultTemplateParams = false);

	// user functions
	virtual Object *createObject(void) const;

private:
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ClientVehicleObjectTemplate(const ClientVehicleObjectTemplate &);
	ClientVehicleObjectTemplate & operator =(const ClientVehicleObjectTemplate &);
};


#endif	// _INCLUDED_ClientVehicleObjectTemplate_H
