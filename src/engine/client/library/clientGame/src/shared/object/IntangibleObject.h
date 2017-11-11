//========================================================================
//
// IntangibleObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_IntangibleObject_H
#define INCLUDED_IntangibleObject_H

#include "clientGame/ClientObject.h"

class SharedIntangibleObjectTemplate;

//----------------------------------------------------------------------

/**
  * A IntangibleObject represents things that don't have a physical form in 
  * the game.
  */
class IntangibleObject : public ClientObject
{
public:

	explicit IntangibleObject(const SharedIntangibleObjectTemplate* newTemplate);
	virtual ~IntangibleObject();

	int  getCount(void) const;
	// @todo: why is this needed?
	void setCount(int count);

	void makeFlattenedTheater();

	virtual void getObjectInfo(stdmap<std::string, stdmap<std::string, Unicode::String>::fwd >::fwd & propertyMap) const;

protected:

private:
	IntangibleObject();
	IntangibleObject(const IntangibleObject& rhs);
	IntangibleObject&	operator=(const IntangibleObject& rhs);

private:

	Archive::AutoDeltaVariable<int> m_count;         // generic counter
};

//----------------------------------------------------------------------

inline int IntangibleObject::getCount(void) const
{
	return m_count.get();
}

inline void IntangibleObject::setCount(int count)
{
	m_count = count;
}


#endif	// INCLUDED_IntangibleObject_H
