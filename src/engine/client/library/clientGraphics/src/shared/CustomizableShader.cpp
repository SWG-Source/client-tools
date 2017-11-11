// ======================================================================
//
// CustomizableShader.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/CustomizableShader.h"

#include "clientGraphics/CustomizableShaderTemplate.h"
#include "clientGraphics/ConfigClientGraphics.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/RangedIntCustomizationVariable.h"

#include <string>
#include <vector>

// ======================================================================

const std::string CustomizableShader::cms_privateVariablePathPrefix("/private/");
const std::string CustomizableShader::cms_sharedVariablePathPrefix("/shared_owner/");

// ======================================================================
// private inlines
// ======================================================================

inline const CustomizableShaderTemplate &CustomizableShader::getCustomizableShaderTemplate() const
{
	return *safe_cast<const CustomizableShaderTemplate*>(&getShaderTemplate());
}

// ======================================================================

Shader *CustomizableShader::convertToModifiableShader() const
{
	return const_cast<CustomizableShader *>(this);
}

// ----------------------------------------------------------------------

bool CustomizableShader::obeysCustomizationData() const
{
	return true;
}

// ----------------------------------------------------------------------

void CustomizableShader::setCustomizationData(CustomizationData *customizationData)
{
	//-- ignore this function call if the CustomizableShader is not influenced
	//   by any customization variables.
	if (!m_intValues || !m_intValues->size())
		return;

	//-- check for assignment of same CustomizationData instance.
	if (customizationData == m_customizationData)
		return;

	//-- release modification callback set on old CustomizationData
	if (m_customizationData)
	{
		m_customizationData->deregisterModificationListener(handleCustomizationModificationCallback, this);

		//-- release local reference
		m_customizationData->release();
	}

	//-- assign new customization data
	m_customizationData = customizationData;

	//-- attach modification callback on new CustomizationData
	if (m_customizationData)
	{
		m_customizationData->registerModificationListener(handleCustomizationModificationCallback, this);

		//-- handle any customizations as any variable in the new CustomizationData may contain a value different than what we have.
		handleCustomizationModification(*m_customizationData);

		// fetch local reference
		m_customizationData->fetch();
	}
}

// ----------------------------------------------------------------------

void CustomizableShader::addCustomizationVariables(CustomizationData &customizationData) const
{
	//-- create a CustomizationVariable for each CustomizableShaderTemplate variable, depending on the type
	const CustomizableShaderTemplate &shaderTemplate = getCustomizableShaderTemplate();
	const int variableCount = shaderTemplate.getCustomizationVariableCount();

	for (int i = 0; i < variableCount; ++i)
	{
		// Get the short variable name.
		const std::string &templateVariableName = shaderTemplate.getCustomizationVariableName(i);

		// Construct full variable path name.
		const std::string  fullVariableName = (shaderTemplate.isCustomizationVariablePrivate(i) ? cms_privateVariablePathPrefix : cms_sharedVariablePathPrefix) + templateVariableName;

		// check if it already exists in the customizationData
		const CustomizationVariable *const existingCustomizationVariable = customizationData.findConstVariable(fullVariableName);
		if (existingCustomizationVariable)
			continue;

		// create the CustomizableVariable
		customizationData.addVariableTakeOwnership(fullVariableName, shaderTemplate.createCustomizationVariable(i));
	}
}

// ----------------------------------------------------------------------

float CustomizableShader::alter(float time) const
{
	//-- alter the base shader
	IGNORE_RETURN(m_baseShader->alter(time));

	// This may change at any time.  For now, have it alter every frame.
	// Later have it notify the appearance when it will need a new update.
	// The appearance can then notify the object, which can get itself put
	// on the AlterScheduler.
	return AlterResult::cms_alterNextFrame;
}

// ----------------------------------------------------------------------

const StaticShader &CustomizableShader::prepareToView() const
{
	//-- apply changes any have occurred since last prepareToView
	const StaticShader &baseStaticShader = m_baseShader->prepareToView();

	if (m_isModified)
	{
		bool const ok = getCustomizableShaderTemplate().applyShaderSettings(*m_intValues, const_cast<StaticShader&>(baseStaticShader));
		m_isModified = false;

		if(!ok)
			WARNING(true, ("CustomizableShader::prepareToView() failed for %s", getCustomizableShaderTemplate().getName().getString()));
	}

	return baseStaticShader;
}

// ----------------------------------------------------------------------

bool CustomizableShader::usesVertexShader() const
{
	return m_baseShader->usesVertexShader();
}

// ----------------------------------------------------------------------

const StaticShader *CustomizableShader::getStaticShader() const
{
	return m_baseShader->getStaticShader();
}

// ----------------------------------------------------------------------

StaticShader *CustomizableShader::getStaticShader()
{
	return m_baseShader->getStaticShader();
}

// ----------------------------------------------------------------------

bool CustomizableShader::getHueInfo(StringVector &variableNames, PackedArgbVector &colors) const
{
	const CustomizableShaderTemplate &shaderTemplate = getCustomizableShaderTemplate();
	return shaderTemplate.getHueInfo(*m_intValues, variableNames, colors);
}

// ======================================================================
/**
 * Handle reception of CustomizationData modifications and dispatch
 * to the associated CustomizableShader instance.
 *
 * @param customizationData  the CustomizationData instance modified.
 * @param context            a casted version of the CustomizableShader
 *                           instance pointer.
 */

void CustomizableShader::handleCustomizationModificationCallback(const CustomizationData &customizationData, const void *context)
{
	NOT_NULL(context);

	//-- convert context to SkeletalMeshGenerator instance
	CustomizableShader *const shader = const_cast<CustomizableShader*>(static_cast<const CustomizableShader*>(context));

	//-- call the instance handler
	shader->handleCustomizationModification(customizationData);
}

// ======================================================================

CustomizableShader::CustomizableShader(const CustomizableShaderTemplate &customizableShaderTemplate) :
	Shader(customizableShaderTemplate),
	m_baseShader(customizableShaderTemplate.getBaseShaderTemplate().fetchModifiableShader()),
	m_customizationData(0),
	m_intValues(new IntVector(static_cast<size_t>(customizableShaderTemplate.getCustomizationVariableCount()))),
	m_isModified(true)
{
}

// ----------------------------------------------------------------------

CustomizableShader::~CustomizableShader()
{
	//lint -esym(1740, CustomizableShader::m_baseShader) // neither freed nor zero'ed // it's okay, it is reference counted and released
	m_baseShader->release();

	delete m_intValues;

	if (m_customizationData)
	{
		m_customizationData->deregisterModificationListener(handleCustomizationModificationCallback, this);

		m_customizationData->release();
		m_customizationData = 0;
	}
}

// ----------------------------------------------------------------------
/**
 * Perform activities necessary when the associated CustomizationData
 * is modified.
 *
 * This function should only be called by handleCustomizationModificationCallback.
 *
 * @param customizationData  this is here for debug purposes --- it should
 *                           be the same as this instance's CustomizationData
 *                           variable.  If not, a DEBUG_FATAL will occur.
 */

void CustomizableShader::handleCustomizationModification(const CustomizationData &customizationData)
{
	//-- sanity check: the given customizationData should be the same as our internal m_customizationData.
	DEBUG_FATAL(&customizationData != m_customizationData, ("CustomizableShader instance is notified of a CustomizationData change not associated with this CustomizableShader instance"));
	UNREF(customizationData);
	NOT_NULL(m_intValues);
	NOT_NULL(m_customizationData);

	//-- Find values for each of the CustomizableShaderTemplates's variables.
	const CustomizableShaderTemplate &shaderTemplate = getCustomizableShaderTemplate();
	bool  modified = false;

	const int variableCount = shaderTemplate.getCustomizationVariableCount();
	DEBUG_FATAL(variableCount != static_cast<int>(m_intValues->size()), ("out of sync variable data storage"));

	for (int i = 0; i < variableCount; ++i)
	{
		//-- get variable name
		const std::string &templateVariableName = shaderTemplate.getCustomizationVariableName(i);

		//-- Build full customization variable name, based on privacy status of variable.
		const std::string  fullVariableName = (shaderTemplate.isCustomizationVariablePrivate(i) ? cms_privateVariablePathPrefix : cms_sharedVariablePathPrefix) + templateVariableName;

		//-- Get RangedIntCustomizationVariable for the given variable name.
		const RangedIntCustomizationVariable *const variable = safe_cast<const RangedIntCustomizationVariable*>(m_customizationData->findConstVariable(fullVariableName));
		if (!variable)
		{
			WARNING(ConfigClientGraphics::getLogBadCustomizationData(), ("CustomizableShader [%s]: no customization data variable for variable [%s].\n", shaderTemplate.getName().getString(), fullVariableName.c_str()));
			continue;
		}

		//-- assign new value if different from old value
		const int newValue = variable->getValue();
		int &localVariable = (*m_intValues)[static_cast<size_t>(i)];

		if (localVariable != newValue)
		{
			// assign new value
			localVariable = newValue;

			// keep track that this instance has been modified
			modified = true;
		}
	}

	//-- update flag that keeps track of when we need to rebuild.
	//   We do not just set it because we may
	//   have multiple CustomizationData changes, resulting in multiple
	//   calls to this function, prior to making changes.
	if (modified)
		m_isModified = true;
}

// ======================================================================
