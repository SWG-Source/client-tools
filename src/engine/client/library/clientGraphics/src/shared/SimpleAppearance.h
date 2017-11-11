// ======================================================================
//
// SimpleAppearance.h
// Portions copyright 1999, bootprint entertainment
// Portions copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.

// ======================================================================

#ifndef INCLUDED_SimpleAppearance_H
#define INCLUDED_SimpleAppearance_H

#include "sharedObject/Appearance.h"

class ShaderPrimitive;

// Very, very simple appearance - just draws the primitive attached to it.

class SimpleAppearance : public Appearance
{
public:

	SimpleAppearance();
	virtual ~SimpleAppearance();

	virtual float alter ( float time );
	virtual void  render() const;

	// ----------

	virtual void    attachPrimitive	( ShaderPrimitive * primitive );
	
	virtual ShaderPrimitive *		getPrimitive	( void );
	virtual ShaderPrimitive const * getPrimitive	( void ) const;

protected:

	DPVS::Object * getDpvsObject() const = 0;

private:

	ShaderPrimitive * m_primitive;

	// ----------

	SimpleAppearance(const SimpleAppearance&);
	SimpleAppearance &operator=(const SimpleAppearance&);
};

// ======================================================================

#endif