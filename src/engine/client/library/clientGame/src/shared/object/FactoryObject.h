//========================================================================
//
// FactoryObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_FactoryObject_H
#define INCLUDED_FactoryObject_H

#include "clientGame/TangibleObject.h"

class SharedFactoryObjectTemplate;

//----------------------------------------------------------------------

/**
  * A FactoryObject represents things that don't have a physical form in 
  * the game.
  */
class FactoryObject : public TangibleObject
{
public:

	explicit FactoryObject(const SharedFactoryObjectTemplate* newTemplate);
	virtual ~FactoryObject();

	const char *           getContainedTemplateName() const;
	const ObjectTemplate * getContainedObjectTemplate() const;
	const ClientObject *   getContainedObject() const;

protected:

private:
	FactoryObject();
	FactoryObject(const FactoryObject& rhs);
	FactoryObject&	operator=(const FactoryObject& rhs);

private:

};

//----------------------------------------------------------------------

#endif	// INCLUDED_FactoryObject_H
