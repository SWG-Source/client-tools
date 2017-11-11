//======================================================================
//
// ClientSharedObjectTemplateInterface.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientSharedObjectTemplateInterface_H
#define INCLUDED_ClientSharedObjectTemplateInterface_H

//======================================================================

#include "sharedGame/SharedObjectTemplateInterface.h"

class Object;
class SharedObjectTemplate;

//----------------------------------------------------------------------

class ClientSharedObjectTemplateInterface : public SharedObjectTemplateInterface
{
public:

	ClientSharedObjectTemplateInterface();
	virtual ~ClientSharedObjectTemplateInterface();

	virtual SharedObjectTemplate const * getSharedTemplate(Object const * object) const;
	virtual bool isShipObject(Object const * object) const;
	virtual uint32 getShipChassisType(Object const * object) const;
	virtual uint32 getShipComponentCrc(Object const * object, int chassisSlot) const;

private:

	ClientSharedObjectTemplateInterface(ClientSharedObjectTemplateInterface const & copy);
	ClientSharedObjectTemplateInterface & operator=(ClientSharedObjectTemplateInterface const & copy);
};

//======================================================================

#endif
