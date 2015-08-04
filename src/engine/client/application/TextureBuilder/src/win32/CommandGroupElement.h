// ======================================================================
//
// CommandGroupElement.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef COMMAND_GROUP_ELEMENT_H
#define COMMAND_GROUP_ELEMENT_H

// ======================================================================

#include "Element.h"

// ======================================================================

class CommandGroupElement: public Element
{
public:

	CommandGroupElement();
	virtual ~CommandGroupElement();

	virtual std::string   getLabel() const;
	virtual unsigned int  getTypeIndex() const;

private:
	// disabled
	CommandGroupElement(const CommandGroupElement&);
	CommandGroupElement &operator =(const CommandGroupElement&);
};

// ======================================================================

#endif
