//======================================================================
//
// CuiMediatorFactory_ConstructorBase.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiMediatorFactory_ConstructorBase_H
#define INCLUDED_CuiMediatorFactory_ConstructorBase_H

#include "clientUserInterface/CuiMediatorFactory.h"

//======================================================================

class CuiMediatorFactory::ConstructorBase
{
public:
	virtual       CuiMediator * get             (bool create = true) = 0;
	virtual       CuiMediator * create          (UIPage & page) const = 0;
	virtual       CuiMediator * createInto      (UIPage & page) const = 0;
	virtual const type_info &   getTypeId       () const = 0;
	
	virtual                    ~ConstructorBase () = 0 {};
	virtual void                detachMediator  () = 0;
	ConstructorBase (bool duplicateOnly) : m_duplicateOnly (duplicateOnly) {}
	
	bool                        isDuplicateOnly () const { return m_duplicateOnly; }
	
protected:
	bool                        m_duplicateOnly;
	
private:
	ConstructorBase &           operator=       (const ConstructorBase & rhs);
	ConstructorBase (const ConstructorBase & rhs);
};

//======================================================================

#endif
