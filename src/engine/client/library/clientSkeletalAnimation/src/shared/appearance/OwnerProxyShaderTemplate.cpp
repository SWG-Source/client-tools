// ======================================================================
//
// OwnerProxyShaderTemplate.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/OwnerProxyShaderTemplate.h"

#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientSkeletalAnimation/OwnerProxyShader.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

#include <string>

// ======================================================================

const Tag TAG_OPST = TAG(O,P,S,T);

// ======================================================================

const ConstCharCrcString          OwnerProxyShaderTemplate::cms_noRenderShaderTemplateName("shader/owner_proxy_norender.sht");

bool                              OwnerProxyShaderTemplate::ms_installed;
MemoryBlockManager *OwnerProxyShaderTemplate::ms_memoryBlockManager;
Shader                           *OwnerProxyShaderTemplate::ms_noRenderShader;

// ======================================================================

void OwnerProxyShaderTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("OwnerProxyShaderTemplate already installed."));

	//-- Create the MemoryBlockManager
	ms_memoryBlockManager = new MemoryBlockManager("OwnerProxyShaderTemplate", true, sizeof(OwnerProxyShaderTemplate), 0, 0, 0);

	//-- Register the ShaderTemplate loader.
	ShaderTemplateList::registerShaderTemplateType(TAG_OPST, create);

	//-- Get do-nothing shader.
	// @todo replace with loading of real shader.
	ms_noRenderShader = ShaderTemplateList::fetchModifiableShader(cms_noRenderShaderTemplateName);

	ms_installed = true;
	ExitChain::add(remove, "OwnerProxyShaderTemplate");
}

// ----------------------------------------------------------------------

void *OwnerProxyShaderTemplate::operator new(size_t size)
{
	DEBUG_FATAL(!ms_installed, ("OwnerProxyShaderTemplate not installed."));
	DEBUG_FATAL(size != sizeof(OwnerProxyShaderTemplate), ("OwnerProxyShaderTemplate::operator new() does not support derived classes."));
	UNREF(size);

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void OwnerProxyShaderTemplate::operator delete(void *data)
{
	DEBUG_FATAL(!ms_installed, ("OwnerProxyShaderTemplate not installed."));
	if (data)
		ms_memoryBlockManager->free(data);
}

// ----------------------------------------------------------------------

Shader &OwnerProxyShaderTemplate::fetchNoRenderShader()
{
	NOT_NULL(ms_noRenderShader);

	ms_noRenderShader->fetch();
	return *ms_noRenderShader;
}

// ======================================================================

bool OwnerProxyShaderTemplate::isOpaqueSolid() const
{
	// WARNING: I cannot know the status of this since the actual ShaderTemplate
	//          used is simply unknown at this time.  Assume it is opaque.
	return true;
}

// ----------------------------------------------------------------------

bool OwnerProxyShaderTemplate::isCollidable() const
{
	// WARNING: I cannot know the status of this since the actual ShaderTemplate
	//          used is simply unknown at this time.  Assume it is collidable.
	return true;
}

// ----------------------------------------------------------------------

bool OwnerProxyShaderTemplate::castsShadows() const
{
	// WARNING: I cannot know the status of this since the actual ShaderTemplate
	//          used is simply unknown at this time.  Assume it casts shadows.
	return true;
}

// ----------------------------------------------------------------------

const Shader *OwnerProxyShaderTemplate::fetchShader() const
{
	Shader *const shader = new OwnerProxyShader(*this);
	shader->fetch();

	return shader;
}

// ----------------------------------------------------------------------

Shader *OwnerProxyShaderTemplate::fetchModifiableShader() const
{
	Shader *const shader = new OwnerProxyShader(*this);
	shader->fetch();

	return shader;
}

// ======================================================================

void OwnerProxyShaderTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("OwnerProxyShaderTemplate not installed."));

	ms_installed = false;

	//-- Register the ShaderTemplate loader.
	ShaderTemplateList::deregisterShaderTemplateType(TAG_OPST);
	
	// @todo clean this up properly when handling a real loaded shader.
	ms_noRenderShader->release();
	ms_noRenderShader = 0;

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

ShaderTemplate *OwnerProxyShaderTemplate::create(const CrcString &name, Iff &iff)
{
	return new OwnerProxyShaderTemplate(name, iff);
}

// ======================================================================
// class OwnerProxyShaderTemplate: private member functions
// ======================================================================

OwnerProxyShaderTemplate::OwnerProxyShaderTemplate(const CrcString &name, Iff &iff) :
	ShaderTemplate(name),
	m_baseShaderTemplateName(0)
{
	iff.enterForm(TAG_OPST);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char buffer[5];

					ConvertTagToString(version, buffer);
					FATAL(true, ("unsupported OwnerProxyShaderTemplate version [%s].", buffer));
				}
		}

	iff.exitForm(TAG_OPST);
}

// ----------------------------------------------------------------------

OwnerProxyShaderTemplate::~OwnerProxyShaderTemplate()
{
	delete m_baseShaderTemplateName;
}

// ----------------------------------------------------------------------

void OwnerProxyShaderTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_INFO);

			//-- Load the base ShaderTemplate name.
			char buffer[1024];
			iff.read_string(buffer, sizeof(buffer));

			if (!m_baseShaderTemplateName)
				m_baseShaderTemplateName = new std::string(buffer);
			else
				*m_baseShaderTemplateName = buffer;

		iff.exitChunk(TAG_INFO);

	iff.exitForm(TAG_0000);
}

// ======================================================================
