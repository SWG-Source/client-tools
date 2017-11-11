// ======================================================================
//
// ShaderEffect.cpp
//
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/ShaderEffect.h"

#include "sharedFile/Iff.h"
#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/GraphicsDebugFlags.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderEffectList.h"
#include "clientGraphics/ShaderImplementation.h"
#include "clientGraphics/ShaderImplementationList.h"
#include "sharedSynchronization/RecursiveMutex.h"

#include <vector>

#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/Graphics.h"

// ======================================================================

const Tag TAG_EFCT = TAG(E,F,C,T);

// ======================================================================

ShaderEffect::ShaderEffect(CrcString const & fileName, Iff & iff)
:
	m_users(0),
	m_name(fileName),
	m_implementation(NULL),
	m_containsPrecalculatedVertexLighting(false)
{
	load(iff);

#ifdef _DEBUG
	if (GraphicsDebugFlags::disablePrecalculatedLighting)
		m_containsPrecalculatedVertexLighting = false;
#endif
}

// ----------------------------------------------------------------------

ShaderEffect::~ShaderEffect()
{
	DEBUG_FATAL(m_users < 0, ("Negative user count"));

	if (m_implementation)
	{
		m_implementation->release();
		m_implementation = 0;
	}
}

// ----------------------------------------------------------------------

void ShaderEffect::fetch() const
{
	ShaderEffectList::enterCriticalSection();
		++m_users;
	ShaderEffectList::leaveCriticalSection();
}

// ----------------------------------------------------------------------

void ShaderEffect::release() const
{
	ShaderEffectList::enterCriticalSection();

		if (--m_users <= 0)
		{
			ShaderEffectList::remove(this);
			delete this;  //lint !e605 // Increase in pointer capability
		}

	ShaderEffectList::leaveCriticalSection();
}

// ----------------------------------------------------------------------

void ShaderEffect::load(Iff &iff)
{
	iff.enterForm(TAG_EFCT);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff);
				break;

			case TAG_0001:
				load_0001(iff);
				break;

			default:
				{
					char buffer[256];
					iff.formatLocation(buffer, sizeof(buffer));
					DEBUG_FATAL(true, ("Unknown effect version in %s", buffer));
				}
				break;
		}

	iff.exitForm(TAG_EFCT);
}


// ----------------------------------------------------------------------

void ShaderEffect::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_DATA);
				const int numberOfImplementations = iff.read_int8();
		iff.exitChunk(TAG_DATA);

		int i = 0;
		for (; !m_implementation && i < numberOfImplementations; ++i)
			m_implementation = ShaderImplementationList::fetch(iff);

		if (ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability())
		{
			for ( ; i < numberOfImplementations; ++i)
			{
				ShaderImplementation const * imp = ShaderImplementationList::fetch(iff);
				if (imp)
					imp->release();
			}
		}

		DEBUG_WARNING(!m_implementation, ("[%s] no implementation passed validation", iff.getFileName()));

	iff.exitForm(TAG_0000, true);	
}

// ----------------------------------------------------------------------

void ShaderEffect::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		iff.enterChunk(TAG_DATA);
				const int numberOfImplementations = iff.read_int8();
				m_containsPrecalculatedVertexLighting = iff.read_bool8();
		iff.exitChunk(TAG_DATA);

		int i = 0;
		for ( ; !m_implementation && i < numberOfImplementations; ++i)
			m_implementation = ShaderImplementationList::fetch(iff);

		if (ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability())
		{
			for ( ; i < numberOfImplementations; ++i)
			{
				ShaderImplementation const * imp = ShaderImplementationList::fetch(iff);
				if (imp)
					imp->release();
			}
		}

		DEBUG_WARNING(!m_implementation, ("[%s] no implementation passed validation", iff.getFileName()));

	iff.exitForm(TAG_0001, true);
}

// ----------------------------------------------------------------------

const ShaderImplementation * ShaderEffect::getActiveShaderImplementation() const
{
	return m_implementation;
}

// ======================================================================
