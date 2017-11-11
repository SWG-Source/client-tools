// ======================================================================
//
// ShaderTemplate.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/ShaderTemplate.h"

#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"

#include <string>

// ======================================================================

namespace ShaderTemplateNamespace
{
	const std::string ms_emptyString("");
}
using namespace ShaderTemplateNamespace;

// ======================================================================

ShaderTemplate::ShaderTemplate(CrcString const & name) :
	m_users(0),
	m_name(name)
#ifdef _DEBUG
	, m_debugName(0)
#endif
{
}

// ----------------------------------------------------------------------

ShaderTemplate::~ShaderTemplate()
{
	DEBUG_FATAL(m_users < 0, ("Negative user count"));

#ifdef _DEBUG
	m_debugName = 0;
#endif
}

// ----------------------------------------------------------------------

void ShaderTemplate::fetch() const
{
	ShaderTemplateList::enterCriticalSection();
		++m_users;
	ShaderTemplateList::leaveCriticalSection();
}

// ----------------------------------------------------------------------

void ShaderTemplate::release() const
{
	ShaderTemplateList::enterCriticalSection();

		if (--m_users <= 0)
		{
#ifdef _DEBUG
			// Clear out the debug name so getName() doesn't report the wrong shader template name to the list.
			m_debugName = 0;
#endif

			ShaderTemplateList::stopTracking(this);
			delete const_cast<ShaderTemplate*>(this);
		}

	ShaderTemplateList::leaveCriticalSection();
}

// ----------------------------------------------------------------------
/**
 * Retrieve the number of customization variables that influence
 * this ShaderTemplate instance.
 *
 * The default implementation returns zero.
 *
 * @return  the number of customization variables that influence
 *          this ShaderTemplate instance.
 */

int ShaderTemplate::getCustomizationVariableCount() const
{
	return 0;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the name of the specified customization variable associated
 * with this ShaderTemplate instance.
 *
 * The default implementation will DEBUG_FATAL and must be overridden
 * if the derived class supports customization.
 *
 * @param index  specifies the customization variable, must be in the
 *               range [0..getCustomizationVariableCount() - 1].
 *
 * @return  the name of customization variable.  Note entities using
 *          this ShaderTemplate may feel free to embed these variables
 *          in other directories by appending a directory structure
 *          before the variable name returned.
 */

const std::string &ShaderTemplate::getCustomizationVariableName(int index) const
{
	UNREF(index);

	DEBUG_FATAL(true, ("derived class implementation should override this function."));
	return ms_emptyString; //lint !e527 // Unreachable // Reachable in release.
}

// ----------------------------------------------------------------------
/**
 * Create a CustomizationVariable instance appropriate for the specified 
 * customization variable that influences this ShaderTemplate instance.
 *
 * A caller can use this function to create a CustomizationVariable instance
 * to be added to a CustomizationData instance.  This can be used as part
 * of an introspection mechanism when trying to establish which customization
 * variables influence given Appearance-related resources.  This facility
 * should be used primary by tools.  Object instances should already know
 * the variables that can be customized via the ObjectTemplate system.
 *
 * The default implementation will DEBUG_FATAL and must be overridden
 * if a derived class supports customization.
 *
 * @param index  specifies the customization variable, must be in the
 *               range [0..getCustomizationVariableCount() - 1].
 *
 * @return  a CustomizationVariable instance with a default value compatible
 *          with the specified customization variable.
 */

CustomizationVariable *ShaderTemplate::createCustomizationVariable(int index) const
{
	UNREF(index);

	DEBUG_FATAL(true, ("derived class implementation should override this function."));
	return NULL; //lint !e527 // Unreachable // Reachable in release.
}

// ----------------------------------------------------------------------

bool ShaderTemplate::containsPrecalculatedVertexLighting() const
{
	return false;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

/**
 * Set the debug name for this shader primitive.
 *
 * The debug name will be reported by getName() if the debug name is set.
 * The specified debugName parameter must live at least as long as this
 * ShaderTemplate instance.
 *
 * @param debugName  string to be reported by getName().  May be NULL.
 *                   If non-NULL, must live at least as long as this
 *                   instance.  This instance will not clean up this parameter.
 */

void ShaderTemplate::setDebugName(CrcString const *debugName) const
{
	m_debugName = debugName;
}

#endif

// ======================================================================
