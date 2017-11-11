// ======================================================================
//
// BlueprintTextureRenderer.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "clientTextureRenderer/FirstClientTextureRenderer.h"
#include "clientTextureRenderer/BlueprintTextureRenderer.h"

#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientTextureRenderer/BlueprintSharedTextureRenderer.h"
#include "clientTextureRenderer/BlueprintTextureRendererTemplate.h"
#include "clientTextureRenderer/ConfigClientTextureRenderer.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/RangedIntCustomizationVariable.h"

#include <string>
#include <vector>

// ======================================================================

const std::string BlueprintTextureRenderer::cms_privateVariableNamePrefix("/private/");
const std::string BlueprintTextureRenderer::cms_sharedVariableNamePrefix("/shared_owner/");

// ======================================================================

inline const BlueprintTextureRendererTemplate &BlueprintTextureRenderer::getBlueprintTextureRendererTemplate() const
{
	return *safe_cast<const BlueprintTextureRendererTemplate*>(&getTextureRendererTemplate());
}

// ======================================================================

bool BlueprintTextureRenderer::render()
{
	// this doesn't do anything anymore.
	DEBUG_WARNING(true, ("unexpected call to BlueprintTextureRenderer::render."));
	return true;
}

// ----------------------------------------------------------------------

void BlueprintTextureRenderer::setCustomizationData(CustomizationData *customizationData)
{
	//-- ignore this function call if the TextureRenderer is not influenced
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

		//-- handle any customizations as any variable may be new
		handleCustomizationModification(*m_customizationData);

		// fetch local reference
		m_customizationData->fetch();
	}
}

// ----------------------------------------------------------------------

void BlueprintTextureRenderer::addCustomizationVariables(CustomizationData &customizationData) const
{
	//-- create a CustomizationVariable for each BlueprintTextureRendererTemplate variable, depending on the type
	const BlueprintTextureRendererTemplate &trTemplate = getBlueprintTextureRendererTemplate();
	const int variableCount = trTemplate.getCustomizationVariableCount();

	for (int i = 0; i < variableCount; ++i)
	{
		//-- Get short variable name.
		const std::string &templateVariableName = trTemplate.getCustomizationVariableName(i);

		//-- Construct variable name as prefix + short variable name.
		std::string fullVariableName = (trTemplate.isCustomizationVariablePrivate(i) ? cms_privateVariableNamePrefix : cms_sharedVariableNamePrefix);
		fullVariableName += templateVariableName;

		//-- Check if variable name already exists in the customizationData
		const CustomizationVariable *const existingCustomizationVariable = customizationData.findConstVariable(fullVariableName);
		if (existingCustomizationVariable)
			continue;

		//-- Create a CustomizationVariable of the appropriate type.
		customizationData.addVariableTakeOwnership(fullVariableName, trTemplate.createCustomizationVariable(i));
	}
}

// ----------------------------------------------------------------------
/**
 * @see TextureRenderer::fetchTexture()
 */

const Texture *BlueprintTextureRenderer::fetchTexture() const
{
	//-- Check for disabled texture rendering.
	if (ConfigClientTextureRenderer::getDisableTextureBaking())
	{
		// Return the default texture.
		return TextureList::fetchDefaultTexture();
	}

	const Texture *returnTexture = 0;

	if (!m_isModified)
	{
		NOT_NULL(m_sharedTextureRenderer);

		if (m_sharedTextureRenderer->isBakingComplete() || !m_previousSharedTextureRenderer)
		{
			//-- use the current shared texture renderer's texture
			returnTexture = m_sharedTextureRenderer->fetchTexture();

			//-- if we were holding a reference to the previous texture renderer, release it
			if (m_previousSharedTextureRenderer)
			{
				m_previousSharedTextureRenderer->release();
				m_previousSharedTextureRenderer = 0;
			}
		}
		else
		{
			//-- use the previous texture renderer's texture
			NOT_NULL(m_previousSharedTextureRenderer);
			returnTexture = m_previousSharedTextureRenderer->fetchTexture();
		}
	}
	else
	{
		// Texture has been modified via customization or first time build.

		//-- clear modified flag
		m_isModified = false;

		//-- texture is modified, fetch new shared texture renderer
		const BlueprintTextureRendererTemplate &trTemplate = getBlueprintTextureRendererTemplate();
		const BlueprintSharedTextureRenderer *const newSharedTr = trTemplate.fetchSharedTextureRenderer(*m_intValues);
		NOT_NULL(newSharedTr);

		//-- Determine from which texture renderer we should grab the texture.  We'll try to use 
		//   the old shared texture renderer if the new one is not yet prepared.

		if (newSharedTr->isBakingComplete())
		{
			// the newly retrieved texture renderer is ready to go.
			returnTexture = newSharedTr->fetchTexture();

			//-- ditch the previous shared texture renderer
			if (m_previousSharedTextureRenderer)
			{
				m_previousSharedTextureRenderer->release();
				m_previousSharedTextureRenderer = 0;
			}
			
			//-- ditch the most recent texture renderer
			if (m_sharedTextureRenderer)
			{
				m_sharedTextureRenderer->release();
				m_sharedTextureRenderer = 0;
			}
		}
		else
		{
			// the newly retrieved texture renderer is not ready yet.
			// determine whether to hang on to the most recent or the previous texture
			// renderer to use while we're waiting for the new Tr to bake.

			if (!m_sharedTextureRenderer)
			{
				// there is no current shared texture renderer.  must use the new "unbaked" texture for now.
				// @todo handle fetching best approximation texture from btrTemplate here.
				returnTexture = newSharedTr->fetchTexture();
			}
			else if (m_sharedTextureRenderer->isBakingComplete() || !m_previousSharedTextureRenderer)
			{
				//-- ditch previous shared texture renderer
				if (m_previousSharedTextureRenderer)
				{
					m_previousSharedTextureRenderer->release();
					m_previousSharedTextureRenderer = 0;
				}

				//-- move current shared texture renderer to previous
				m_previousSharedTextureRenderer = m_sharedTextureRenderer;
				m_sharedTextureRenderer         = 0;

				//-- get texture
				returnTexture = m_previousSharedTextureRenderer->fetchTexture();
			}
			else
			{
				// use the old previous shared texture renderer, keep as old
				NOT_NULL(m_previousSharedTextureRenderer);
				returnTexture = m_previousSharedTextureRenderer->fetchTexture();

				// release the current
				NOT_NULL(m_sharedTextureRenderer);
				m_sharedTextureRenderer->release();
				m_sharedTextureRenderer = 0;
			}
		}

		//-- keep track of the new texture renderer
		m_sharedTextureRenderer = newSharedTr;
	}

	return returnTexture;
}

// ----------------------------------------------------------------------

bool BlueprintTextureRenderer::isTextureReady() const
{
	if (ConfigClientTextureRenderer::getDisableTextureBaking())
	{
		//-- If we ignore texture baking, texture renderers are always ready.
		return true;
	}

	//-- Update the texture as necessary for any changes to customization variables.
	Texture const *const texture = fetchTexture();

	bool const readyStatus = (m_sharedTextureRenderer && m_sharedTextureRenderer->isTextureReady());
	if (texture)
		texture->release();

	return readyStatus;
}

// ======================================================================
/**
 * Handle reception of CustomizationData modifications and dispatch
 * to the associated BlendTextureRenderer instance.
 *
 * @param customizationData  the CustomizationData instance modified.
 * @param context            a casted version of the BlueprintTextureRenderer
 *                           instance pointer.
 */

void BlueprintTextureRenderer::handleCustomizationModificationCallback(const CustomizationData &customizationData, const void *context)
{
	NOT_NULL(context);

	//-- convert context to SkeletalMeshGenerator instance
	BlueprintTextureRenderer *const bpTextureRenderer = const_cast<BlueprintTextureRenderer*>(static_cast<const BlueprintTextureRenderer*>(context));

	//-- call the instance handler
	bpTextureRenderer->handleCustomizationModification(customizationData);
}

// ======================================================================

BlueprintTextureRenderer::BlueprintTextureRenderer(const BlueprintTextureRendererTemplate &blueprintTextureRendererTemplate) :
	TextureRenderer(blueprintTextureRendererTemplate),
	m_customizationData(0),
	m_intValues(new IntVector(static_cast<size_t>(blueprintTextureRendererTemplate.getCustomizationVariableCount()))),
	m_isModified(true),
	m_sharedTextureRenderer(0),
	m_previousSharedTextureRenderer(0)
{
}

// ----------------------------------------------------------------------

BlueprintTextureRenderer::~BlueprintTextureRenderer()
{
	//-- release shared texture renderers
	if (m_previousSharedTextureRenderer)
	{
		m_previousSharedTextureRenderer->release();
		m_previousSharedTextureRenderer = 0;
	}

	if (m_sharedTextureRenderer)
	{
		m_sharedTextureRenderer->release();
		m_sharedTextureRenderer = 0;
	}

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

void BlueprintTextureRenderer::handleCustomizationModification(const CustomizationData &customizationData)
{
	//-- sanity check: the given customizationData should be the same as our internal m_customizationData.
	DEBUG_FATAL(&customizationData != m_customizationData, ("BlueprintTextureRenderer instance is notified of a CustomizationData change not associated with this BlueprintTextureRenderer instance"));
	UNREF(customizationData);
	NOT_NULL(m_intValues);
	NOT_NULL(m_customizationData);

	//-- find values for each of the MeshGeneratorTemplate's variable names
	const BlueprintTextureRendererTemplate &trTemplate = getBlueprintTextureRendererTemplate();
	bool  modified = false;

	const int variableCount = trTemplate.getCustomizationVariableCount();
	DEBUG_FATAL(variableCount != static_cast<int>(m_intValues->size()), ("out of sync variable data storage"));

	for (int i = 0; i < variableCount; ++i)
	{
		//-- get variable name
		const std::string &templateVariableName = trTemplate.getCustomizationVariableName(i);

		// for now, all TextureRenderer variable names are assumed to be in directory "/private/"
		const std::string  fullVariableName = (trTemplate.isCustomizationVariablePrivate(i) ? cms_privateVariableNamePrefix : cms_sharedVariableNamePrefix) + templateVariableName;

		//-- get RangedIntCustomizationVariable for the given variable name
		const RangedIntCustomizationVariable *const variable = safe_cast<const RangedIntCustomizationVariable*>(m_customizationData->findConstVariable(fullVariableName));
		if (!variable)
		{
			WARNING(ConfigClientGraphics::getLogBadCustomizationData(), ("BlueprintTextureRenderer [%s]: no customization data variable for variable [%s].", trTemplate.getCrcName().getString(), fullVariableName.c_str()));
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

	//-- update flag that keeps track of when we need to rebuild before
	//   the next fetchTexture().  We cannot just set it because we may
	//   have multiple CustomizationData changes, resulting in multiple
	//   calls to this function, prior to needing to bake.  We don't bak
	//   here because we want to bake only after we have all changes for
	//   the frame.
	if (modified)
	{
		m_isModified = true;

		//-- Request the baked texture now so that it will be baked by the time the
		//   next main loop render section gets invoked.
		//   NOTE: this undermines the ability for this instance to efficiently batch up
		//         multiple changes occuring during a single frame.  I will end up baking
		//         multiple textures with each incremental change if more than one change
		//         occurs in a single frame.  The reason for trying this is to get the
		//         baked texture changes to occur on the same frame as the frame where
		//         the changes are made.  This is because the CustomizableShader hue'ing
		//         works the same way: changes are applied on the same frame.  Without this
		//         fix, the character faces will get hued a frame after the body gets hued,
		//         causing a visual flicker.
		//
		//         The texture baking code should be checked to see if, upon startup, a
		//         baked texture is getting baked multiple times due to initial data setup.
		//         If so, something should be done about it.

		const Texture *const texture = fetchTexture();
		if (texture)
			texture->release();
	}
}

// ======================================================================
