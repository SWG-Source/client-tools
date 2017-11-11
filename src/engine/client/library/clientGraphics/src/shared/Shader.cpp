// ======================================================================
//
// Shader.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/Shader.h"

#include "clientGraphics/ShaderTemplate.h"
#include "sharedObject/AlterResult.h"

// ======================================================================

Shader::Shader(const ShaderTemplate &shaderTemplate)
:
	m_users(0),
	m_template(shaderTemplate)
{
	m_template.fetch();
}

// ----------------------------------------------------------------------

Shader::Shader(const Shader &shader)
:
	m_users(0),
	m_template(shader.getShaderTemplate())
{
	m_template.fetch();
}

// ----------------------------------------------------------------------

Shader::~Shader()
{
	DEBUG_FATAL(m_users < 0, ("Negative user count"));
	m_template.release();
}

// ----------------------------------------------------------------------

void Shader::release() const
{
	if (--m_users <= 0)
		delete this; //lint !e605 // Warning -- Increase in pointer capability
}

// ----------------------------------------------------------------------

const char *Shader::getName() const
{
	return m_template.getName().getString();
}

// ----------------------------------------------------------------------
/**
 * Set the CustomizationData instance to be associated with this
 * Shader instance.
 *
 * Implementers of derived classes: the default implementation does nothing.
 * Override it and do the appropriate thing if the derived class supports
 * customization.  The derived implementation is not required to chain down
 * to this implementation.
 *
 * @param customizationData  the new CustomizationData instance to be
 *                           associated with this Shader instance.
 *                           This value may be NULL.
 */

void Shader::setCustomizationData(CustomizationData * /* customizationData */)
{
}

// ----------------------------------------------------------------------
/**
 * Add all CustomizationData variables influencing this Shader instance
 * to the given CustomizationData instance.
 *
 * This is primarily useful as a mechanism for tools.  The game should already
 * know which customization variables it has enabled for objects via the
 * ObjectTemplate system.
 *
 * Derived classes do not need to chain down to this function.
 *
 * Implementers: there are no guarantees as to the order of calls between 
 * setCustomizationData() and this function.  In other words, the implementation 
 * should use the given CustomizationData arg to add variables.
 *
 * @param customizationData  the CustomizationData instance to which new
 *                           variables will be added.
 */

void Shader::addCustomizationVariables(CustomizationData & /* customizationData */) const
{
}

// ----------------------------------------------------------------------

float Shader::alter(float time) const
{
	UNREF(time);

	// No alter required by this class functionality.
	return AlterResult::cms_keepNoAlter;
}

// ======================================================================
