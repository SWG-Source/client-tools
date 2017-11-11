// ======================================================================
//
// OwnerProxyShader.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/OwnerProxyShader.h"

#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientSkeletalAnimation/OwnerProxyShaderTemplate.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/Object.h"

#include <string>

// ======================================================================

namespace OwnerProxyShaderNamespace
{
	bool    s_installed;
	Shader *s_noRenderShader;

	bool    s_enabled;
}

using namespace OwnerProxyShaderNamespace;

// ======================================================================

const std::string OwnerProxyShader::cms_sharedOwnerDirectoryName("/shared_owner/");
const std::string OwnerProxyShader::cms_shaderTemplateDirectory("shader/");
const std::string OwnerProxyShader::cms_shaderTemplateExtension(".sht");
const char *const OwnerProxyShader::cms_directorySeparatorCharacters = "/\\";
const char        OwnerProxyShader::cms_extensionCharacter = '.';
      bool        OwnerProxyShader::cms_stitchedSkinInheritsFromSelf = false;

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(OwnerProxyShader, true, 0, 0, 0);

// ======================================================================

inline const OwnerProxyShaderTemplate &OwnerProxyShader::getProxyShaderTemplate() const
{
	return *NON_NULL(safe_cast<const OwnerProxyShaderTemplate*>(&getShaderTemplate()));
}

// ======================================================================

void OwnerProxyShader::install(bool stitchedSkinInheritsFromSelf)
{
	DEBUG_FATAL(s_installed, ("OwnerProxyShader already installed."));

	installMemoryBlockManager();
	s_noRenderShader = &(OwnerProxyShaderTemplate::fetchNoRenderShader());

	s_enabled = true;

	s_installed = true;
	cms_stitchedSkinInheritsFromSelf = stitchedSkinInheritsFromSelf;
	ExitChain::add(remove, "OwnerProxyShader");
}

// ----------------------------------------------------------------------

void OwnerProxyShader::setEnabled(bool enabled)
{
	s_enabled = enabled;
}

// ======================================================================

OwnerProxyShader::OwnerProxyShader(const OwnerProxyShaderTemplate &shaderTemplate) :
	Shader(shaderTemplate),
	m_shader(fetchNoRenderShader())
{
}

// ----------------------------------------------------------------------

Shader *OwnerProxyShader::convertToModifiableShader() const
{
	return const_cast<OwnerProxyShader*>(this);
}

// ----------------------------------------------------------------------

bool OwnerProxyShader::obeysCustomizationData() const
{
	return true;
}

// ----------------------------------------------------------------------

void OwnerProxyShader::setCustomizationData(CustomizationData *customizationData)
{
	assignShader(customizationData);
} //lint !e818 // (Info -- Pointer parameter 'customizationData' (line 32) could be declared as pointing to const) // it is part of the immutable virtual contract.

// ----------------------------------------------------------------------

void OwnerProxyShader::addCustomizationVariables(CustomizationData &customizationData) const
{
	NOT_NULL(m_shader);
	m_shader->addCustomizationVariables(customizationData);
}

// ----------------------------------------------------------------------

const StaticShader &OwnerProxyShader::prepareToView() const
{
	if (s_enabled)
	{
		NOT_NULL(m_shader);
		return m_shader->prepareToView();
	}
	else
	{
		NOT_NULL(s_noRenderShader);
		return s_noRenderShader->prepareToView();
	}
}

// ----------------------------------------------------------------------

bool OwnerProxyShader::usesVertexShader() const
{
	NOT_NULL(m_shader);
	return m_shader->usesVertexShader();
}

// ----------------------------------------------------------------------

const StaticShader *OwnerProxyShader::getStaticShader() const
{
	NOT_NULL(m_shader);
	return m_shader->getStaticShader();
}

// ----------------------------------------------------------------------

StaticShader *OwnerProxyShader::getStaticShader()
{
	NOT_NULL(m_shader);
	return m_shader->getStaticShader();
}

// ----------------------------------------------------------------------

Shader const *OwnerProxyShader::getRealShader() const
{
	return m_shader;
}

// ======================================================================

void OwnerProxyShader::remove()
{
	DEBUG_FATAL(!s_installed, ("OwnerProxyShader not installed."));
	s_installed = false;

	s_noRenderShader->release();
	s_noRenderShader = 0;

	removeMemoryBlockManager();
}

// ======================================================================

OwnerProxyShader::~OwnerProxyShader()
{
	if (m_shader)
	{
		m_shader->release();
		m_shader = 0;
	}
}

// ----------------------------------------------------------------------

void OwnerProxyShader::assignShader(CustomizationData *customizationData)
{
	//-- Hang on to the old shader.
	Shader *const oldShader = m_shader;

	if (!customizationData)
	{
		//-- When there's no way to find the Owner, (e.g. a wearable not being worn),
		//   don't render these polys via a Shader that does not render.
		m_shader = fetchNoRenderShader();
	}
	else
	{
		//-- Retrieve the CustomizationData instance that owns the content of the 'shared_owner' directory.
		const CustomizationData *const ownerCd = customizationData->getDirectoryOwner(cms_sharedOwnerDirectoryName);

		// Self-selection check is valid only if we're not in the viewer
		if (!ownerCd || ( (!cms_stitchedSkinInheritsFromSelf) && (ownerCd == customizationData) ) )
		{
			// Either the shared_owner directory doesn't exist, or the current object
			// owns the directory.  In either case, it is indicative of a non-existing
			// owner other than the wearable, so don't render it.
			m_shader = fetchNoRenderShader();
		}
		else
		{
			//-- Retrieve the Appearance for the Object instance.
			const Object                   &ownerObject        = ownerCd->getOwnerObject();
			const Appearance         *const appearance         = ownerObject.getAppearance();
			const AppearanceTemplate *const appearanceTemplate = (appearance ? appearance->getAppearanceTemplate() : 0);
			const char               *const atName             = (appearanceTemplate ? appearanceTemplate->getName() : 0);

			if (!atName || !strlen(atName))
			{
				// No AppearanceTemplate name exists, use the no render shader.
				m_shader = fetchNoRenderShader();
			}
			else
			{
				//-- Build the real Shader name from the appearance's AppearanceTemplate name.
				const std::string ownerShaderTemplateName = buildOwnerShaderTemplateName(atName);

				//-- Fetch the Shader.
				DEBUG_WARNING(!TreeFile::exists(ownerShaderTemplateName.c_str()), ("Appearance [%s] is missing shader [%s] required for skinning: did shader names accidentally change?", NON_NULL(appearanceTemplate)->getName(), ownerShaderTemplateName.c_str()));
				m_shader = ShaderTemplateList::fetchModifiableShader(TemporaryCrcString(ownerShaderTemplateName.c_str(), true));

				if (m_shader)
				{
					if (dynamic_cast<OwnerProxyShader const *>(m_shader))
					{
						DEBUG_WARNING(true, ("Object %s has a customizable shader [%s] which points to the same customizable base shader", ownerObject.getDebugInformation().c_str(), ownerShaderTemplateName.c_str()));

						// New shader points to an owner proxy shader which results in a cyclical dependency
						m_shader->release();
						m_shader = fetchNoRenderShader();
					}
					else
					{
						//-- Set the CustomizationData for the new shader.
						m_shader->setCustomizationData(customizationData);
					}
				}
			}
		}
	}

	//-- Release the old shader.
	if (oldShader)
		oldShader->release();
}

// ----------------------------------------------------------------------

Shader *OwnerProxyShader::fetchNoRenderShader() const
{
	return &(OwnerProxyShaderTemplate::fetchNoRenderShader());
}

// ----------------------------------------------------------------------

const std::string OwnerProxyShader::buildOwnerShaderTemplateName(const std::string &appearanceTemplateName) const
{
	std::string  prefixName;

	//-- Strip off any path from the appearance template name.
	std::string::size_type endOfDirectoryPos = appearanceTemplateName.find_last_of(cms_directorySeparatorCharacters);
	if (static_cast<int>(endOfDirectoryPos) != static_cast<int>(std::string::npos))
	{
		if ((endOfDirectoryPos + 1) < appearanceTemplateName.length())
			IGNORE_RETURN(prefixName.assign(appearanceTemplateName.begin() + (endOfDirectoryPos + 1), appearanceTemplateName.end()));
	}

	//-- Truncate off the extension.
	std::string::size_type extensionPos = prefixName.rfind(cms_extensionCharacter);
	if (static_cast<int>(extensionPos) != static_cast<int>(std::string::npos))
		prefixName.resize(extensionPos);

	//-- Add the underscore separating the prefix from the base.
	prefixName += '_';

	//-- Build the owner's ShaderTemplate name.
	return cms_shaderTemplateDirectory + prefixName + getProxyShaderTemplate().getBaseShaderTemplateName() + cms_shaderTemplateExtension;
}

// ======================================================================
