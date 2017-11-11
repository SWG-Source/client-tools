#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/SimpleAppearance.h"

#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "sharedObject/AlterResult.h"

#include "dpvsObject.hpp"

// ----------------------------------------------------------------------

SimpleAppearance::SimpleAppearance()
: Appearance(NULL),
  m_primitive(NULL)
{
}

// ----------------------------------------------------------------------

SimpleAppearance::~SimpleAppearance()
{
	delete m_primitive;
	m_primitive = NULL;
}

// ----------------------------------------------------------------------

void SimpleAppearance::render() const
{
	if(m_primitive)
		ShaderPrimitiveSorter::add(*m_primitive);
}

// ----------------------------------------------------------------------

float SimpleAppearance::alter(float time)
{
	if(m_primitive)
		return m_primitive->alter(time);

	// @todo figure out what this should really return.
	return AlterResult::cms_alterNextFrame;
}

// ----------------------------------------------------------------------

void SimpleAppearance::attachPrimitive( ShaderPrimitive * primitive )
{
	if(primitive != m_primitive)
	{
		delete m_primitive;
		m_primitive = primitive;
	}
}

// ----------------------------------------------------------------------

ShaderPrimitive * SimpleAppearance::getPrimitive ( void )
{
	return m_primitive;
}

// ----------------------------------------------------------------------

ShaderPrimitive const * SimpleAppearance::getPrimitive ( void ) const
{
	return m_primitive;
}

// ----------------------------------------------------------------------

