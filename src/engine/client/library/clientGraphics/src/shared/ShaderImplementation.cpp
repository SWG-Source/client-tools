// ======================================================================
//
// ShaderImplementation.cpp
//
// Copyright 2001 - 2003 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/ShaderImplementation.h"

#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsDebugFlags.h"
#include "clientGraphics/GraphicsOptionTags.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderImplementationList.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/StaticShaderTemplate.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/Misc.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedSynchronization/RecursiveMutex.h"
#include <algorithm>
#include <map>
#include <string>
#include <vector>

// ======================================================================

const Tag TAG_A128 = TAG(A,1,2,8);
const Tag TAG_ALPH = TAG(A,L,P,H);
const Tag TAG_IMPL = TAG(I,M,P,L);
const Tag TAG_MAIN = TAG(M,A,I,N);
const Tag TAG_OPTN = TAG(O,P,T,N);
const Tag TAG_PASS = TAG(P,A,S,S);
const Tag TAG_PEXE = TAG(P,E,X,E);
const Tag TAG_PFFP = TAG(P,F,F,P);
const Tag TAG_PIXL = TAG(P,I,X,L);
const Tag TAG_PPSH = TAG(P,P,S,H);
const Tag TAG_PSHP = TAG(P,S,H,P);
const Tag TAG_PSRC = TAG(P,S,R,C);
const Tag TAG_PTCH = TAG(P,T,C,H);
const Tag TAG_PTXM = TAG(P,T,X,M);
const Tag TAG_PVSH = TAG(P,V,S,H);
const Tag TAG_SCAP = TAG(S,C,A,P);
const Tag TAG_STAG = TAG(S,T,A,G);
const Tag TAG_VBUF = TAG(V,B,U,F);
const Tag TAG_VEXE = TAG(V,E,X,E);
const Tag TAG_VSHP = TAG(V,S,H,P);
const Tag TAG_VSRC = TAG(V,S,R,C);
const Tag TAG_VTAG = TAG(V,T,A,G);

namespace ShaderImplementationNamespace
{
#if PRODUCTION == 0
	bool ms_convertAlphaBlendingToAlphaTesting;
#endif

	int remapOldInconsistentShaderCapabilityLevels(int oldLevel)
	{
		if (oldLevel == ShaderCapability(1, 0, true)) return ShaderCapability(0, 2);
		if (oldLevel == ShaderCapability(1, 5, true)) return ShaderCapability(0, 3);
		if (oldLevel == ShaderCapability(2, 0))       return ShaderCapability(1, 1);
		if (oldLevel == ShaderCapability(2, 5, true)) return ShaderCapability(1, 4);
		if (oldLevel == ShaderCapability(3, 0, true)) return ShaderCapability(2, 0);
		
		WARNING(true, ("Could not remap old shader capability %d.%d", GetShaderCapabilityMajor(oldLevel), GetShaderCapabilityMinor(oldLevel)));
		return ShaderCapability(0,0);
	}

	int remapRecentInconsistentShaderCapabilityLevels(int oldLevel)
	{
		if (oldLevel == ShaderCapability(0, 2))       return ShaderCapability(0, 2);
		if (oldLevel == ShaderCapability(0, 3))       return ShaderCapability(0, 3);
		if (oldLevel == ShaderCapability(1, 0, true)) return ShaderCapability(1, 1);
		if (oldLevel == ShaderCapability(1, 4))       return ShaderCapability(1, 4);
		if (oldLevel == ShaderCapability(2, 0))       return ShaderCapability(2, 0);

		WARNING(true, ("Could not remap recent shader capability %d.%d", GetShaderCapabilityMajor(oldLevel), GetShaderCapabilityMinor(oldLevel)));
		return ShaderCapability(0,0);
	}
}
using namespace ShaderImplementationNamespace;

// ======================================================================

ShaderImplementationGraphicsData::~ShaderImplementationGraphicsData()
{
}

// ======================================================================

void ShaderImplementation::install()
{
#ifdef _DEBUG
	DebugFlags::registerFlag(ms_convertAlphaBlendingToAlphaTesting, "ClientGraphics", "convertAlphaBlendingToAlphaTesting");
#endif
	ExitChain::add(ShaderImplementation::remove, "ShaderImplementation");
}

//----------------------------------------------------------------------

void ShaderImplementation::remove()
{
	ShaderImplementationPassVertexShader::reportDangling();
}

// ======================================================================

ShaderImplementation::ShaderImplementation(const char *name, Iff &iff)
:
	m_users(0),
	m_graphicsData(NULL),
	m_name(DuplicateString(name)),
	m_pass(new Passes),
	m_phase(-1),
	m_castsShadows(true),
	m_isCollidable(true)
{
	load(iff);

#if PRODUCTION == 0
	if (ms_convertAlphaBlendingToAlphaTesting && m_phase == 4)
		m_phase = 2;
#endif

}

// ----------------------------------------------------------------------

ShaderImplementation::~ShaderImplementation()
{
	DEBUG_FATAL(m_users < 0, ("Negative user count"));

	delete [] m_name;
	delete m_graphicsData;

	Passes::iterator end = m_pass->end();
	for (Passes::iterator i = m_pass->begin(); i != end; ++i)
		delete (*i);
	delete m_pass;
}

// ----------------------------------------------------------------------

void ShaderImplementation::fetch() const
{
	ShaderImplementationList::ms_criticalSection.enter();
		++m_users;
	ShaderImplementationList::ms_criticalSection.leave();
}

// ----------------------------------------------------------------------

void ShaderImplementation::release() const
{
	ShaderImplementationList::ms_criticalSection.enter();

		if (--m_users <= 0)
		{
			ShaderImplementationList::remove(this);
			delete this; //lint !e605 // Increase in pointer capability
		}

	ShaderImplementationList::ms_criticalSection.leave();
}

// ----------------------------------------------------------------------

void ShaderImplementation::load(Iff &iff)
{
	iff.enterForm(TAG_IMPL);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff);
				break;

			case TAG_0001:
				load_0001(iff);
				break;

			case TAG_0002:
				load_0002(iff);
				break;

			case TAG_0003:
				load_0003(iff);
				break;

			case TAG_0004:
				load_0004(iff);
				break;

			case TAG_0005:
				load_0005(iff);
				break;

			case TAG_0006:
				load_0006(iff);
				break;

			case TAG_0007:
				load_0007(iff);
				break;

			case TAG_0008:
				load_0008(iff);
				break;

			case TAG_0009:
				load_0009(iff);
				break;

			default:
				{
					char buffer[256];
					iff.formatLocation(buffer, sizeof(buffer));
					DEBUG_FATAL(true, ("Unknown implementation version in %s", buffer));
				}
				break;
		}

	iff.exitForm(TAG_IMPL);
}

// ----------------------------------------------------------------------

void ShaderImplementation::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
		iff.exitChunk(TAG_DATA);

		m_pass->reserve(static_cast<uint>(numberOfPasses));
		for (int i = 0; i < numberOfPasses; ++i)
			m_pass->push_back(new Pass(iff));

	iff.exitForm(TAG_0000);	

	// infer the phase from the pass' first alpha blending state
	{
		if (numberOfPasses && (*m_pass)[0]->hasAlphaBlending())
			m_phase = ShaderPrimitiveSorter::getPhase(TAG_ALPH);
		else
			m_phase = ShaderPrimitiveSorter::getPhase(TAG_MAIN);
	}

	m_castsShadows = m_isCollidable = isOpaqueSolid();

	checkOldVersionForSupport();
}

// ----------------------------------------------------------------------

void ShaderImplementation::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		iff.enterChunk(TAG_DATA);
				const int numberOfPasses = iff.read_int8();
				IGNORE_RETURN(iff.read_bool8());
		iff.exitChunk(TAG_DATA);

		m_pass->reserve(static_cast<uint>(numberOfPasses));
		for (int i = 0; i < numberOfPasses; ++i)
			m_pass->push_back(new Pass(iff));

	iff.exitForm(TAG_0001);	

	// infer the phase from the pass' first alpha blending state
	// if (ShaderPrimitiveSorter::isInstalled())
	{
		if (numberOfPasses && (*m_pass)[0]->hasAlphaBlending())
			m_phase = ShaderPrimitiveSorter::getPhase(TAG_ALPH);
		else
			m_phase = ShaderPrimitiveSorter::getPhase(TAG_MAIN);
	}

	m_castsShadows = m_isCollidable = isOpaqueSolid();

	checkOldVersionForSupport();
}

// ----------------------------------------------------------------------

void ShaderImplementation::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);

		iff.enterChunk(TAG_DATA);
				const int numberOfPasses = iff.read_int8();
				m_phase = ShaderPrimitiveSorter::getPhase(iff.read_uint32());
		iff.exitChunk(TAG_DATA);

		m_pass->reserve(static_cast<uint>(numberOfPasses));
		for (int i = 0; i < numberOfPasses; ++i)
			m_pass->push_back(new Pass(iff));

	iff.exitForm(TAG_0002);

	m_castsShadows = m_isCollidable = isOpaqueSolid();

	checkOldVersionForSupport();
}

// ----------------------------------------------------------------------

void ShaderImplementation::load_0003(Iff &iff)
{
	iff.enterForm(TAG_0003);

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			m_phase = ShaderPrimitiveSorter::getPhase(iff.read_uint32());
		iff.exitChunk(TAG_DATA);

		if (iff.enterChunk(TAG_OPTN, true))
		{
			const int numberOfOptionTags = iff.getChunkLengthTotal(sizeof(Tag));

			for (int i = 0; i < numberOfOptionTags; ++i)
				if (!GraphicsOptionTags::get(iff.read_uint32()))
				{
					iff.exitChunk(TAG_OPTN, true);
					iff.exitForm(TAG_0003, true);
					return;
				}
				
			iff.exitChunk(TAG_OPTN);
		}

		m_pass->reserve(static_cast<uint>(numberOfPasses));
		bool vertexShader = false;
		for (int i = 0; i < numberOfPasses; ++i)
		{
			m_pass->push_back(new Pass(iff));
			if (i == 0)
				vertexShader = m_pass->back()->usesVertexShader();
			else
				DEBUG_WARNING(vertexShader != m_pass->back()->usesVertexShader(), ("Some passes of an implementation use vertex shaders, some do not"));
		}

	iff.exitForm(TAG_0003);	

	m_castsShadows = m_isCollidable = isOpaqueSolid();

	checkOldVersionForSupport();
}

// ----------------------------------------------------------------------

void ShaderImplementation::load_0004(Iff &iff)
{
	iff.enterForm(TAG_0004);

		int const graphicsShaderCapability = Graphics::getShaderCapability();
		bool supported = false;
		iff.enterChunk(TAG_SCAP);
			while (!supported && iff.getChunkLengthLeft(sizeof(int32)))
				if (remapOldInconsistentShaderCapabilityLevels(iff.read_int32()) == graphicsShaderCapability)
					supported = true;
		iff.exitChunk(TAG_SCAP, true);

		// bail out early if the shader isn't supported
		if (!supported && !ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability())
		{
			iff.exitForm(TAG_0004, true);
			return;
		}

		if (iff.enterChunk(TAG_OPTN, true))
		{
			const int numberOfOptionTags = iff.getChunkLengthTotal(sizeof(Tag));

			for (int i = 0; i < numberOfOptionTags; ++i)
				if (!GraphicsOptionTags::get(iff.read_uint32()))
				{
					iff.exitChunk(TAG_OPTN, true);
					iff.exitForm(TAG_0004, true);
					return;
				}
				
			iff.exitChunk(TAG_OPTN);
		}

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			m_phase = ShaderPrimitiveSorter::getPhase(iff.read_uint32());
		iff.exitChunk(TAG_DATA);

		m_pass->reserve(static_cast<uint>(numberOfPasses));
		bool vertexShader = false;
		for (int i = 0; i < numberOfPasses; ++i)
		{
			m_pass->push_back(new Pass(iff));
			if (i == 0)
				vertexShader = m_pass->back()->usesVertexShader();
			else
				DEBUG_WARNING(vertexShader != m_pass->back()->usesVertexShader(), ("Some passes of an implementation use vertex shaders, some do not"));
		}

	iff.exitForm(TAG_0004);	

	m_castsShadows = m_isCollidable = isOpaqueSolid();

	if (supported)
		m_graphicsData = Graphics::createShaderImplementationGraphicsData(*this);
}

// ----------------------------------------------------------------------

void ShaderImplementation::load_0005(Iff &iff)
{
	iff.enterForm(TAG_0005);

		int const graphicsShaderCapability = Graphics::getShaderCapability();
		bool supported = false;
		iff.enterChunk(TAG_SCAP);
			while (!supported && iff.getChunkLengthLeft(sizeof(int32)))
				if (remapOldInconsistentShaderCapabilityLevels(iff.read_int32()) == graphicsShaderCapability)
					supported = true;
		iff.exitChunk(TAG_SCAP, true);

		// bail out early if the shader isn't supported
		if (!supported && !ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability())
		{
			iff.exitForm(TAG_0005, true);
			return;
		}

		if (iff.enterChunk(TAG_OPTN, true))
		{
			const int numberOfOptionTags = iff.getChunkLengthTotal(sizeof(Tag));

			for (int i = 0; i < numberOfOptionTags; ++i)
				if (!GraphicsOptionTags::get(iff.read_uint32()))
				{
					iff.exitChunk(TAG_OPTN, true);
					iff.exitForm(TAG_0005, true);
					return;
				}
				
			iff.exitChunk(TAG_OPTN);
		}

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			m_phase = ShaderPrimitiveSorter::getPhase(iff.read_uint32());
			m_castsShadows = iff.read_bool8();
			m_isCollidable = iff.read_bool8();
		iff.exitChunk(TAG_DATA);

		m_pass->reserve(static_cast<uint>(numberOfPasses));
		bool vertexShader = false;
		for (int i = 0; i < numberOfPasses; ++i)
		{
			m_pass->push_back(new Pass(iff));
			if (i == 0)
				vertexShader = m_pass->back()->usesVertexShader();
			else
				DEBUG_WARNING(vertexShader != m_pass->back()->usesVertexShader(), ("Some passes of an implementation use vertex shaders, some do not"));
		}

	iff.exitForm(TAG_0005);

	if (supported)
		m_graphicsData = Graphics::createShaderImplementationGraphicsData(*this);
}

// ----------------------------------------------------------------------

void ShaderImplementation::load_0006(Iff &iff)
{
	iff.enterForm(TAG_0006);

		int const graphicsShaderCapability = Graphics::getShaderCapability();
		bool supported = false;
		iff.enterChunk(TAG_SCAP);
			while (!supported && iff.getChunkLengthLeft(sizeof(int32)))
				if (remapRecentInconsistentShaderCapabilityLevels(iff.read_int32()) == graphicsShaderCapability)
					supported = true;
		iff.exitChunk(TAG_SCAP, true);

		// bail out early if the shader isn't supported
		if (!supported && !ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability())
		{
			iff.exitForm(TAG_0006, true);
			return;
		}

		if (iff.enterChunk(TAG_OPTN, true))
		{
			const int numberOfOptionTags = iff.getChunkLengthTotal(sizeof(Tag));

			for (int i = 0; i < numberOfOptionTags; ++i)
				if (!GraphicsOptionTags::get(iff.read_uint32()))
				{
					iff.exitChunk(TAG_OPTN, true);
					iff.exitForm(TAG_0006, true);
					return;
				}
				
			iff.exitChunk(TAG_OPTN);
		}

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			m_phase = ShaderPrimitiveSorter::getPhase(iff.read_uint32());
			m_castsShadows = iff.read_bool8();
			m_isCollidable = iff.read_bool8();
		iff.exitChunk(TAG_DATA);

		m_pass->reserve(static_cast<uint>(numberOfPasses));
		bool vertexShader = false;
		for (int i = 0; i < numberOfPasses; ++i)
		{
			m_pass->push_back(new Pass(iff));
			if (i == 0)
				vertexShader = m_pass->back()->usesVertexShader();
			else
				DEBUG_WARNING(vertexShader != m_pass->back()->usesVertexShader(), ("Some passes of an implementation use vertex shaders, some do not"));
		}

	iff.exitForm(TAG_0006);

	if (supported)
		m_graphicsData = Graphics::createShaderImplementationGraphicsData(*this);
}

// ----------------------------------------------------------------------

void ShaderImplementation::load_0007(Iff &iff)
{
	iff.enterForm(TAG_0007);

		int const graphicsShaderCapability = Graphics::getShaderCapability();
		bool supported = false;
		iff.enterChunk(TAG_SCAP);
			while (!supported && iff.getChunkLengthLeft(sizeof(int32)))
				if (iff.read_int32() == graphicsShaderCapability)
					supported = true;
		iff.exitChunk(TAG_SCAP, true);

		// bail out early if the shader isn't supported
		if (!supported && !ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability())
		{
			iff.exitForm(TAG_0007, true);
			return;
		}

		if (iff.enterChunk(TAG_OPTN, true))
		{
			const int numberOfOptionTags = iff.getChunkLengthTotal(sizeof(Tag));

			for (int i = 0; i < numberOfOptionTags; ++i)
				if (!GraphicsOptionTags::get(iff.read_uint32()))
				{
					iff.exitChunk(TAG_OPTN, true);
					iff.exitForm(TAG_0007, true);
					return;
				}
				
			iff.exitChunk(TAG_OPTN);
		}

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			m_phase = ShaderPrimitiveSorter::getPhase(iff.read_uint32());
			m_castsShadows = iff.read_bool8();
			m_isCollidable = iff.read_bool8();
		iff.exitChunk(TAG_DATA);

		m_pass->reserve(static_cast<uint>(numberOfPasses));
		bool vertexShader = false;
		for (int i = 0; i < numberOfPasses; ++i)
		{
			m_pass->push_back(new Pass(iff));
			if (i == 0)
				vertexShader = m_pass->back()->usesVertexShader();
			else
				DEBUG_WARNING(vertexShader != m_pass->back()->usesVertexShader(), ("Some passes of an implementation use vertex shaders, some do not"));
		}

	iff.exitForm(TAG_0007);

	if (supported)
		m_graphicsData = Graphics::createShaderImplementationGraphicsData(*this);
}

// ----------------------------------------------------------------------

void ShaderImplementation::load_0008(Iff &iff)
{
	iff.enterForm(TAG_0008);

		int const graphicsShaderCapability = Graphics::getShaderCapability();
		bool supported = false;
		iff.enterChunk(TAG_SCAP);
			while (!supported && iff.getChunkLengthLeft(sizeof(int32)))
				if (iff.read_int32() == graphicsShaderCapability)
					supported = true;
		iff.exitChunk(TAG_SCAP, true);

		// bail out early if the shader isn't supported
		if (!supported && !ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability())
		{
			iff.exitForm(TAG_0008, true);
			return;
		}

		if (iff.enterChunk(TAG_OPTN, true))
		{
			const int numberOfOptionTags = iff.getChunkLengthTotal(sizeof(Tag));

			m_optionTags.reserve(numberOfOptionTags);
			for (int i = 0; i < numberOfOptionTags; ++i)
			{
				uint32 const tag = iff.read_uint32();
				if (!GraphicsOptionTags::get(tag))
				{
					iff.exitChunk(TAG_OPTN, true);
					iff.exitForm(TAG_0008, true);
					return;
				}
				else
				{
					m_optionTags.push_back(tag);
				}
			}

			std::sort(m_optionTags.begin(), m_optionTags.end());
				
			iff.exitChunk(TAG_OPTN);
		}

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			m_phase = ShaderPrimitiveSorter::getPhase(iff.read_uint32());
			m_castsShadows = iff.read_bool8();
			m_isCollidable = iff.read_bool8();
			//-- discard heat
			iff.read_bool8();
		iff.exitChunk(TAG_DATA);

		m_pass->reserve(static_cast<uint>(numberOfPasses));
		bool vertexShader = false;
		for (int i = 0; i < numberOfPasses; ++i)
		{
			m_pass->push_back(new Pass(iff));
			if (i == 0)
				vertexShader = m_pass->back()->usesVertexShader();
			else
				DEBUG_WARNING(vertexShader != m_pass->back()->usesVertexShader(), ("Some passes of an implementation use vertex shaders, some do not"));
		}

	iff.exitForm(TAG_0008);

	if (supported)
		m_graphicsData = Graphics::createShaderImplementationGraphicsData(*this);
}


// ----------------------------------------------------------------------

void ShaderImplementation::load_0009(Iff &iff)
{
	iff.enterForm(TAG_0009);

		int const graphicsShaderCapability = Graphics::getShaderCapability();
		bool supported = false;
		iff.enterChunk(TAG_SCAP);
			while (!supported && iff.getChunkLengthLeft(sizeof(int32)))
				if (iff.read_int32() == graphicsShaderCapability)
					supported = true;
		iff.exitChunk(TAG_SCAP, true);

		// bail out early if the shader isn't supported
		if (!supported && !ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability())
		{
			iff.exitForm(TAG_0009, true);
			return;
		}

		if (iff.enterChunk(TAG_OPTN, true))
		{
			const int numberOfOptionTags = iff.getChunkLengthTotal(sizeof(Tag));

			m_optionTags.reserve(numberOfOptionTags);
			for (int i = 0; i < numberOfOptionTags; ++i)
			{
				uint32 const tag = iff.read_uint32();
				if (!GraphicsOptionTags::get(tag))
				{
					iff.exitChunk(TAG_OPTN, true);
					iff.exitForm(TAG_0009, true);
					return;
				}
				else
				{
					m_optionTags.push_back(tag);
				}
			}

			std::sort(m_optionTags.begin(), m_optionTags.end());
				
			iff.exitChunk(TAG_OPTN);
		}

		iff.enterChunk(TAG_DATA);
			const int numberOfPasses = iff.read_int8();
			m_phase = ShaderPrimitiveSorter::getPhase(iff.read_uint32());
			m_castsShadows = iff.read_bool8();
			m_isCollidable = iff.read_bool8();
		iff.exitChunk(TAG_DATA);

		m_pass->reserve(static_cast<uint>(numberOfPasses));
		bool vertexShader = false;
		for (int i = 0; i < numberOfPasses; ++i)
		{
			m_pass->push_back(new Pass(iff));
			if (i == 0)
				vertexShader = m_pass->back()->usesVertexShader();
			else
				DEBUG_WARNING(vertexShader != m_pass->back()->usesVertexShader(), ("Some passes of an implementation use vertex shaders, some do not"));
		}

	iff.exitForm(TAG_0009);

	if (supported)
		m_graphicsData = Graphics::createShaderImplementationGraphicsData(*this);
}

// ----------------------------------------------------------------------

void ShaderImplementation::checkOldVersionForSupport()
{
	if (!m_pass->empty())
	{
		if (usesVertexShader())
		{
			if (Graphics::getShaderCapability() < ShaderCapability(1,1))
				return;
		}
		else
		{
			if (Graphics::getShaderCapability() >= ShaderCapability(1,1))
				return;
		}
	}

	m_graphicsData = Graphics::createShaderImplementationGraphicsData(*this);
}

// ----------------------------------------------------------------------

bool ShaderImplementation::isSupported() const
{
	return m_graphicsData != NULL;
}

// ----------------------------------------------------------------------

int ShaderImplementation::getNumberOfPasses() const
{
	return static_cast<int>(m_pass->size());
}

// ----------------------------------------------------------------------

bool ShaderImplementation::isOpaqueSolid() const
{
	if (m_pass->empty())
		return false;

	const Pass *pass = (*m_pass)[0];
	return !pass->hasAlphaBlending() && !pass->hasAlphaTesting();
}

// ----------------------------------------------------------------------

bool ShaderImplementation::usesVertexShader() const
{
	if (m_pass->empty())
		return false;

	return (*m_pass)[0]->usesVertexShader();
}

// ----------------------------------------------------------------------

bool ShaderImplementation::usesMaterial(Tag materialTag) const
{
	Passes::iterator const iEnd = m_pass->end();
	for (Passes::iterator i = m_pass->begin(); i != iEnd; ++i)
		if ((*i)->usesMaterial(materialTag))
			return true;

	return false;
}

// ----------------------------------------------------------------------

bool ShaderImplementation::usesTexture(Tag textureTag) const
{
	Passes::iterator const iEnd = m_pass->end();
	for (Passes::iterator i = m_pass->begin(); i != iEnd; ++i)
		if ((*i)->usesTexture(textureTag))
			return true;

	return false;
}

// ----------------------------------------------------------------------

bool ShaderImplementation::usesTextureCoordinateSet(Tag textureCoordinateSetTag) const
{
	Passes::iterator const iEnd = m_pass->end();
	for (Passes::iterator i = m_pass->begin(); i != iEnd; ++i)
		if ((*i)->usesTextureCoordinateSet(textureCoordinateSetTag))
			return true;

	return false;
}

// ----------------------------------------------------------------------

bool ShaderImplementation::usesTextureFactor(Tag textureFactorTag) const
{
	Passes::iterator const iEnd = m_pass->end();
	for (Passes::iterator i = m_pass->begin(); i != iEnd; ++i)
		if ((*i)->usesTextureFactor(textureFactorTag))
			return true;

	return false;
}

// ----------------------------------------------------------------------

bool ShaderImplementation::usesTextureScroll(Tag textureScrollTag) const
{
	Passes::iterator const iEnd = m_pass->end();
	for (Passes::iterator i = m_pass->begin(); i != iEnd; ++i)
		if ((*i)->usesTextureScroll(textureScrollTag))
			return true;

	return false;
}

// ----------------------------------------------------------------------

bool ShaderImplementation::usesAlphaReference(Tag alphaReferenceTag) const
{
	Passes::iterator const iEnd = m_pass->end();
	for (Passes::iterator i = m_pass->begin(); i != iEnd; ++i)
		if ((*i)->usesAlphaReference(alphaReferenceTag))
			return true;

	return false;
}

// ----------------------------------------------------------------------

bool ShaderImplementation::usesStencilReference(Tag stencilReferenceTag) const
{
	Passes::iterator const iEnd = m_pass->end();
	for (Passes::iterator i = m_pass->begin(); i != iEnd; ++i)
		if ((*i)->usesStencilReference(stencilReferenceTag))
			return true;

	return false;
}

// ----------------------------------------------------------------------

void ShaderImplementation::verifyCompatibility(VertexBufferFormat const & vertexBufferFormat, StaticShaderTemplate const & staticShaderTemplate) const
{
#ifdef _DEBUG
	Passes::iterator const iEnd = m_pass->end();
	for (Passes::iterator i = m_pass->begin(); i != iEnd; ++i)
		(*i)->verifyCompatibility(vertexBufferFormat, staticShaderTemplate);
#else
	UNREF(vertexBufferFormat);
	UNREF(staticShaderTemplate);
#endif
}

// ======================================================================

ShaderImplementationPass::ShaderImplementationPass(Iff &iff)
:
	m_fixedFunctionPipeline(NULL),
	m_vertexShader(NULL),
	m_stage(NULL),
	m_pixelShader(NULL),

	m_shadeMode(SM_Gouraud),
	m_fogMode(FM_Normal),

	m_ditherEnable(false),

	m_zEnable(true),
	m_zWrite(true),
	m_zCompare(C_LessOrEqual),

	m_alphaBlendEnable(true),
	m_alphaBlendOperation(BO_Add),
	m_alphaBlendSource(B_SourceAlpha),
	m_alphaBlendDestination(B_InverseSourceAlpha),

	m_alphaTestEnable(false),
	m_alphaTestReferenceValueTag(0),
	m_alphaTestFunction(C_Greater),

	m_writeEnable(BINARY1(1111)),

	m_textureFactorTag(0),
	m_textureFactorTag2(0),
	m_textureScrollTag(0),

	m_stencilEnable(false),
	m_stencilTwoSidedMode(false),
	m_stencilReferenceValueTag(0),
	m_stencilCompareFunction(C_Greater),
	m_stencilPassOperation(SO_Keep),
	m_stencilZFailOperation(SO_Keep),
	m_stencilFailOperation(SO_Keep),
	m_stencilCounterClockwiseCompareFunction(C_Greater),
	m_stencilCounterClockwisePassOperation(SO_Keep),
	m_stencilCounterClockwiseZFailOperation(SO_Keep),
	m_stencilCounterClockwiseFailOperation(SO_Keep),
	m_stencilWriteMask(0),
	m_stencilMask(0),

	m_materialTag(0),
	m_heat(false)

{
	load(iff);

#if PRODUCTION == 0
	if (ms_convertAlphaBlendingToAlphaTesting && m_alphaBlendEnable)
	{
		m_alphaBlendEnable = false;
		m_alphaTestEnable = true;
		m_alphaTestReferenceValueTag = TAG_A128;
	}
#endif

#ifdef _DEBUG
	if (m_fixedFunctionPipeline && m_pixelShader)
	{
		DEBUG_WARNING(true, ("shader uses FFP and PS"));
	}
	if (m_vertexShader && m_stage)
	{
		DEBUG_WARNING(true, ("shader uses VS and STAGES"));
	}
#endif
}

// ----------------------------------------------------------------------

ShaderImplementationPass::~ShaderImplementationPass()
{
	if (m_stage)
	{
		Stages::iterator end = m_stage->end();
		for (Stages::iterator i = m_stage->begin(); i != end; ++i)
			delete (*i);
		delete m_stage;
	}

	if (m_vertexShader)
		m_vertexShader->release();

	delete m_pixelShader;
	delete m_fixedFunctionPipeline;
}

// ----------------------------------------------------------------------

void ShaderImplementationPass::load(Iff &iff)
{
	iff.enterForm(TAG_PASS);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff);
				break;

			case TAG_0001:
				load_0001(iff);
				break;

			case TAG_0002:
				load_0002(iff);
				break;

			case TAG_0003:
				load_0003(iff);
				break;

			case TAG_0004:
				load_0004(iff);
				break;

			case TAG_0005:
				load_0005(iff);
				break;

			case TAG_0006:
				load_0006(iff);
				break;

			case TAG_0007:
				load_0007(iff);
				break;

			case TAG_0008:
				load_0008(iff);
				break;

			case TAG_0009:
				load_0009(iff);
				break;

			case TAG_0010:
				load_0010(iff);
				break;

			default:
				{
					char buffer[256];
					iff.formatLocation(buffer, sizeof(buffer));
					DEBUG_FATAL(true, ("Bad pass version %s", buffer));
				}
				break;
		}

	iff.exitForm(TAG_PASS);
}

// ----------------------------------------------------------------------

void ShaderImplementationPass::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		
		iff.enterChunk(TAG_DATA);

			const int numberOfStages = iff.read_int8();

			m_shadeMode                       = static_cast<ShadeMode>(iff.read_int8());

			m_ditherEnable                    = iff.read_bool8();

			m_zEnable                         = iff.read_bool8();
			m_zWrite                          = iff.read_bool8();
			m_zCompare                        = static_cast<Compare>(iff.read_int8());

			m_alphaBlendEnable                = iff.read_bool8();
			m_alphaBlendOperation             = static_cast<BlendOperation>(iff.read_int8());
			m_alphaBlendSource                = static_cast<Blend>(iff.read_int8());
			m_alphaBlendDestination           = static_cast<Blend>(iff.read_int8());

			m_alphaTestEnable                 = iff.read_bool8();
			m_alphaTestReferenceValueTag      = static_cast<Tag>(iff.read_uint32());
			m_alphaTestFunction               = static_cast<Compare>(iff.read_int8());

			m_writeEnable                     = iff.read_uint8();

			m_textureFactorTag                = static_cast<Tag>(iff.read_uint32());

			m_fixedFunctionPipeline           = new FixedFunctionPipeline(iff, true);		
			m_materialTag                     = m_fixedFunctionPipeline->m_deprecated_lightingMaterialTag;

			m_stencilEnable                   = iff.read_bool8();
			m_stencilReferenceValueTag        = static_cast<Tag>(iff.read_uint32());
			m_stencilCompareFunction          = static_cast<Compare>(iff.read_int8());
			m_stencilPassOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilZFailOperation           = static_cast<StencilOperation>(iff.read_int8());
			m_stencilFailOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilWriteMask                = iff.read_uint32();
			m_stencilMask                     = iff.read_uint32();

		iff.exitChunk(TAG_DATA);

		m_stage = new Stages;
		m_stage->reserve(static_cast<uint>(numberOfStages));
		for (int i = 0; i < numberOfStages; ++i)
			m_stage->push_back(new Stage(iff));

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void ShaderImplementationPass::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);
		
		iff.enterChunk(TAG_DATA);
			const int numberOfStages = iff.read_int8();

			const Tag pixelShaderTag = static_cast<Tag>(iff.read_uint32());
			UNREF(pixelShaderTag);
			DEBUG_FATAL(pixelShaderTag != 0, ("Old data with pixel shader tag"));

			m_shadeMode                       = static_cast<ShadeMode>(iff.read_int8());

			m_ditherEnable                    = iff.read_bool8();

			m_zEnable                         = iff.read_bool8();
			m_zWrite                          = iff.read_bool8();
			m_zCompare                        = static_cast<Compare>(iff.read_int8());

			m_alphaBlendEnable                = iff.read_bool8();
			m_alphaBlendOperation             = static_cast<BlendOperation>(iff.read_int8());
			m_alphaBlendSource                = static_cast<Blend>(iff.read_int8());
			m_alphaBlendDestination           = static_cast<Blend>(iff.read_int8());

			m_alphaTestEnable                 = iff.read_bool8();
			m_alphaTestReferenceValueTag      = static_cast<Tag>(iff.read_uint32());
			m_alphaTestFunction               = static_cast<Compare>(iff.read_int8());

			m_writeEnable                     = iff.read_uint8();

			m_textureFactorTag                = static_cast<Tag>(iff.read_uint32());

			m_fixedFunctionPipeline           = new FixedFunctionPipeline(iff, true);		
			m_materialTag                     = m_fixedFunctionPipeline->m_deprecated_lightingMaterialTag;

			m_stencilEnable                   = iff.read_bool8();
			m_stencilReferenceValueTag        = static_cast<Tag>(iff.read_uint32());
			m_stencilCompareFunction          = static_cast<Compare>(iff.read_int8());
			m_stencilPassOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilZFailOperation           = static_cast<StencilOperation>(iff.read_int8());
			m_stencilFailOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilWriteMask                = iff.read_uint32();
			m_stencilMask                     = iff.read_uint32();

		iff.exitChunk(TAG_DATA);

		m_stage = new Stages;
		m_stage->reserve(static_cast<uint>(numberOfStages));
		for (int i = 0; i < numberOfStages; ++i)
			m_stage->push_back(new Stage(iff));

	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void ShaderImplementationPass::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);
		
		iff.enterChunk(TAG_DATA);

			const bool pixelShader    = iff.read_bool8();
			const int  numberOfStages = iff.read_int8();

			m_shadeMode                       = static_cast<ShadeMode>(iff.read_int8());
			m_fogMode                         = static_cast<FogMode>(iff.read_int8());

			m_ditherEnable                    = iff.read_bool8();

			m_zEnable                         = iff.read_bool8();
			m_zWrite                          = iff.read_bool8();
			m_zCompare                        = static_cast<Compare>(iff.read_int8());

			m_alphaBlendEnable                = iff.read_bool8();
			m_alphaBlendOperation             = static_cast<BlendOperation>(iff.read_int8());
			m_alphaBlendSource                = static_cast<Blend>(iff.read_int8());
			m_alphaBlendDestination           = static_cast<Blend>(iff.read_int8());

			m_alphaTestEnable                 = iff.read_bool8();
			m_alphaTestReferenceValueTag      = static_cast<Tag>(iff.read_uint32());
			m_alphaTestFunction               = static_cast<Compare>(iff.read_int8());

			m_writeEnable                     = iff.read_uint8();

			m_textureFactorTag                = static_cast<Tag>(iff.read_uint32());

			m_fixedFunctionPipeline           = new FixedFunctionPipeline(iff, true);		
			m_materialTag                     = m_fixedFunctionPipeline->m_deprecated_lightingMaterialTag;

			m_stencilEnable                   = iff.read_bool8();
			m_stencilReferenceValueTag        = static_cast<Tag>(iff.read_uint32());
			m_stencilCompareFunction          = static_cast<Compare>(iff.read_int8());
			m_stencilPassOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilZFailOperation           = static_cast<StencilOperation>(iff.read_int8());
			m_stencilFailOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilWriteMask                = iff.read_uint32();
			m_stencilMask                     = iff.read_uint32();

		iff.exitChunk(TAG_DATA);

		if (pixelShader)
		{
			DEBUG_FATAL(true, ("Old data with pixel shader"));
		}
		else
		{
			m_stage = new Stages;
			m_stage->reserve(static_cast<uint>(numberOfStages));
			for (int i = 0; i < numberOfStages; ++i)
				m_stage->push_back(new Stage(iff));
		}

	iff.exitForm(TAG_0002);
}

// ----------------------------------------------------------------------

void ShaderImplementationPass::load_0003(Iff &iff)
{
	iff.enterForm(TAG_0003);
		
		iff.enterChunk(TAG_DATA);

			const bool pixelShader    = iff.read_bool8();
			const int  numberOfStages = iff.read_int8();

			m_shadeMode                       = static_cast<ShadeMode>(iff.read_int8());
			m_fogMode                         = static_cast<FogMode>(iff.read_int8());

			m_ditherEnable                    = iff.read_bool8();

			m_zEnable                         = iff.read_bool8();
			m_zWrite                          = iff.read_bool8();
			m_zCompare                        = static_cast<Compare>(iff.read_int8());

			m_alphaBlendEnable                = iff.read_bool8();
			m_alphaBlendOperation             = static_cast<BlendOperation>(iff.read_int8());
			m_alphaBlendSource                = static_cast<Blend>(iff.read_int8());
			m_alphaBlendDestination           = static_cast<Blend>(iff.read_int8());

			m_alphaTestEnable                 = iff.read_bool8();
			m_alphaTestReferenceValueTag      = static_cast<Tag>(iff.read_uint32());
			m_alphaTestFunction               = static_cast<Compare>(iff.read_int8());

			m_writeEnable                     = iff.read_uint8();

			m_textureFactorTag                = static_cast<Tag>(iff.read_uint32());

			m_fixedFunctionPipeline           = new FixedFunctionPipeline(iff, true);		
			m_materialTag                     = m_fixedFunctionPipeline->m_deprecated_lightingMaterialTag;

			m_stencilEnable                   = iff.read_bool8();
			m_stencilReferenceValueTag        = static_cast<Tag>(iff.read_uint32());
			m_stencilCompareFunction          = static_cast<Compare>(iff.read_int8());
			m_stencilPassOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilZFailOperation           = static_cast<StencilOperation>(iff.read_int8());
			m_stencilFailOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilWriteMask                = iff.read_uint32();
			m_stencilMask                     = iff.read_uint32();

			iff.read_uint32();  // unused vertex shader tag

		iff.exitChunk(TAG_DATA);

		if (pixelShader)
		{
			DEBUG_FATAL(true, ("Old data with pixel shader"));
		}
		else
		{
			m_stage = new Stages;
			m_stage->reserve(static_cast<uint>(numberOfStages));
			for (int i = 0; i < numberOfStages; ++i)
				m_stage->push_back(new Stage(iff));
		}

	iff.exitForm(TAG_0003);
}

// ----------------------------------------------------------------------

void ShaderImplementationPass::load_0004(Iff &iff)
{
	iff.enterForm(TAG_0004);
		
		iff.enterChunk(TAG_DATA);

			const bool pixelShader    = iff.read_bool8();
			const int  numberOfStages = iff.read_int8();

			m_shadeMode                       = static_cast<ShadeMode>(iff.read_int8());
			m_fogMode                         = static_cast<FogMode>(iff.read_int8());

			m_ditherEnable                    = iff.read_bool8();

			m_zEnable                         = iff.read_bool8();
			m_zWrite                          = iff.read_bool8();
			m_zCompare                        = static_cast<Compare>(iff.read_int8());

			m_alphaBlendEnable                = iff.read_bool8();
			m_alphaBlendOperation             = static_cast<BlendOperation>(iff.read_int8());
			m_alphaBlendSource                = static_cast<Blend>(iff.read_int8());
			m_alphaBlendDestination           = static_cast<Blend>(iff.read_int8());

			m_alphaTestEnable                 = iff.read_bool8();
			m_alphaTestReferenceValueTag      = static_cast<Tag>(iff.read_uint32());
			m_alphaTestFunction               = static_cast<Compare>(iff.read_int8());

			m_writeEnable                     = iff.read_uint8();

			m_textureFactorTag                = static_cast<Tag>(iff.read_uint32());

			m_stencilEnable                   = iff.read_bool8();
			m_stencilReferenceValueTag        = static_cast<Tag>(iff.read_uint32());
			m_stencilCompareFunction          = static_cast<Compare>(iff.read_int8());
			m_stencilPassOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilZFailOperation           = static_cast<StencilOperation>(iff.read_int8());
			m_stencilFailOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilWriteMask                = iff.read_uint32();
			m_stencilMask                     = iff.read_uint32();

		iff.exitChunk(TAG_DATA);

		if (iff.getCurrentName() == TAG_PFFP)
		{
			m_fixedFunctionPipeline = new FixedFunctionPipeline(iff, false);
			m_materialTag = m_fixedFunctionPipeline->m_deprecated_lightingMaterialTag;
		}
		else
			if (iff.getCurrentName() == TAG_PVSH)
			{
				char buffer[256];

				iff.enterForm(TAG_PVSH);
					iff.enterChunk(TAG_0000);
						iff.read_string(buffer, sizeof(buffer));
					iff.exitChunk(TAG_0000);
				iff.exitForm(TAG_PVSH);

				m_vertexShader = VertexShader::fetch(buffer);
			}
			else
				DEBUG_FATAL(true, ("unknown transform unit"));

		if (pixelShader)
		{
			DEBUG_FATAL(true, ("Old data with pixel shader"));
		}
		else
		{
			m_stage = new Stages;
			m_stage->reserve(static_cast<uint>(numberOfStages));
			for (int i = 0; i < numberOfStages; ++i)
				m_stage->push_back(new Stage(iff));
		}

	iff.exitForm(TAG_0004);
}

// ----------------------------------------------------------------------

void ShaderImplementationPass::load_0005(Iff &iff)
{
	iff.enterForm(TAG_0005);
		
		iff.enterChunk(TAG_DATA);

			const int  numberOfStages = iff.read_int8();

			m_shadeMode                       = static_cast<ShadeMode>(iff.read_int8());
			m_fogMode                         = static_cast<FogMode>(iff.read_int8());

			m_ditherEnable                    = iff.read_bool8();

			m_zEnable                         = iff.read_bool8();
			m_zWrite                          = iff.read_bool8();
			m_zCompare                        = static_cast<Compare>(iff.read_int8());

			m_alphaBlendEnable                = iff.read_bool8();
			m_alphaBlendOperation             = static_cast<BlendOperation>(iff.read_int8());
			m_alphaBlendSource                = static_cast<Blend>(iff.read_int8());
			m_alphaBlendDestination           = static_cast<Blend>(iff.read_int8());

			m_alphaTestEnable                 = iff.read_bool8();
			m_alphaTestReferenceValueTag      = static_cast<Tag>(iff.read_uint32());
			m_alphaTestFunction               = static_cast<Compare>(iff.read_int8());

			m_writeEnable                     = iff.read_uint8();

			m_textureFactorTag                = static_cast<Tag>(iff.read_uint32());

			m_stencilEnable                   = iff.read_bool8();
			m_stencilReferenceValueTag        = static_cast<Tag>(iff.read_uint32());
			m_stencilCompareFunction          = static_cast<Compare>(iff.read_int8());
			m_stencilPassOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilZFailOperation           = static_cast<StencilOperation>(iff.read_int8());
			m_stencilFailOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilWriteMask                = iff.read_uint32();
			m_stencilMask                     = iff.read_uint32();

		iff.exitChunk(TAG_DATA);

		if (iff.getCurrentName() == TAG_PFFP)
		{
			m_fixedFunctionPipeline = new FixedFunctionPipeline(iff, false);
			m_materialTag = m_fixedFunctionPipeline->m_deprecated_lightingMaterialTag;
		}
		else
			if (iff.getCurrentName() == TAG_PVSH)
			{
				char buffer[256];

				iff.enterForm(TAG_PVSH);
					iff.enterChunk(TAG_0000);
						iff.read_string(buffer, sizeof(buffer));
					iff.exitChunk(TAG_0000);
				iff.exitForm(TAG_PVSH);

				m_vertexShader = VertexShader::fetch(buffer);
			}
			else
				DEBUG_FATAL(true, ("unknown transform unit"));

		if (numberOfStages == 0 && !iff.atEndOfForm() && iff.getCurrentName() == TAG_PPSH)
		{
			m_pixelShader = new PixelShader(iff);
		}
		else
		{
			m_stage = new Stages;
			m_stage->reserve(static_cast<uint>(numberOfStages));
			for (int i = 0; i < numberOfStages; ++i)
				m_stage->push_back(new Stage(iff));
		}

	iff.exitForm(TAG_0005);
}

// ----------------------------------------------------------------------

void ShaderImplementationPass::load_0006(Iff &iff)
{
	iff.enterForm(TAG_0006);
		
		iff.enterChunk(TAG_DATA);

			const int  numberOfStages = iff.read_int8();

			m_shadeMode                       = static_cast<ShadeMode>(iff.read_int8());
			m_fogMode                         = static_cast<FogMode>(iff.read_int8());

			m_ditherEnable                    = iff.read_bool8();

			m_zEnable                         = iff.read_bool8();
			m_zWrite                          = iff.read_bool8();
			m_zCompare                        = static_cast<Compare>(iff.read_int8());

			m_alphaBlendEnable                = iff.read_bool8();
			m_alphaBlendOperation             = static_cast<BlendOperation>(iff.read_int8());
			m_alphaBlendSource                = static_cast<Blend>(iff.read_int8());
			m_alphaBlendDestination           = static_cast<Blend>(iff.read_int8());

			m_alphaTestEnable                 = iff.read_bool8();
			m_alphaTestReferenceValueTag      = static_cast<Tag>(iff.read_uint32());
			m_alphaTestFunction               = static_cast<Compare>(iff.read_int8());

			m_writeEnable                     = iff.read_uint8();

			m_textureFactorTag                = static_cast<Tag>(iff.read_uint32());

			m_stencilEnable                   = iff.read_bool8();
			m_stencilReferenceValueTag        = static_cast<Tag>(iff.read_uint32());
			m_stencilCompareFunction          = static_cast<Compare>(iff.read_int8());
			m_stencilPassOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilZFailOperation           = static_cast<StencilOperation>(iff.read_int8());
			m_stencilFailOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilWriteMask                = iff.read_uint32();
			m_stencilMask                     = iff.read_uint32();

			m_materialTag                     = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_DATA);

		if (iff.getCurrentName() == TAG_PFFP)
			m_fixedFunctionPipeline = new FixedFunctionPipeline(iff, false);
		else
			if (iff.getCurrentName() == TAG_PVSH)
			{
				char buffer[256];

				iff.enterForm(TAG_PVSH);
					iff.enterChunk(TAG_0000);
						iff.read_string(buffer, sizeof(buffer));
					iff.exitChunk(TAG_0000);
				iff.exitForm(TAG_PVSH);

				m_vertexShader = VertexShader::fetch(buffer);
			}
			else
				DEBUG_FATAL(true, ("unknown transform unit"));

		if (numberOfStages == 0 && !iff.atEndOfForm() && iff.getCurrentName() == TAG_PPSH)
		{
			m_pixelShader = new PixelShader(iff);
		}
		else
		{
			m_stage = new Stages;
			m_stage->reserve(static_cast<uint>(numberOfStages));
			for (int i = 0; i < numberOfStages; ++i)
				m_stage->push_back(new Stage(iff));
		}

	iff.exitForm(TAG_0006);
}

// ----------------------------------------------------------------------

void ShaderImplementationPass::load_0007(Iff &iff)
{
	iff.enterForm(TAG_0007);
		
		iff.enterChunk(TAG_DATA);

			const int  numberOfStages = iff.read_int8();

			m_shadeMode                       = static_cast<ShadeMode>(iff.read_int8());
			m_fogMode                         = static_cast<FogMode>(iff.read_int8());

			m_ditherEnable                    = iff.read_bool8();

			m_zEnable                         = iff.read_bool8();
			m_zWrite                          = iff.read_bool8();
			m_zCompare                        = static_cast<Compare>(iff.read_int8());

			m_alphaBlendEnable                = iff.read_bool8();
			m_alphaBlendOperation             = static_cast<BlendOperation>(iff.read_int8());
			m_alphaBlendSource                = static_cast<Blend>(iff.read_int8());
			m_alphaBlendDestination           = static_cast<Blend>(iff.read_int8());

			m_alphaTestEnable                 = iff.read_bool8();
			m_alphaTestReferenceValueTag      = static_cast<Tag>(iff.read_uint32());
			m_alphaTestFunction               = static_cast<Compare>(iff.read_int8());

			m_writeEnable                     = iff.read_uint8();

			m_textureFactorTag                = static_cast<Tag>(iff.read_uint32());
			m_textureFactorTag2               = static_cast<Tag>(iff.read_uint32());

			m_stencilEnable                   = iff.read_bool8();
			m_stencilReferenceValueTag        = static_cast<Tag>(iff.read_uint32());
			m_stencilCompareFunction          = static_cast<Compare>(iff.read_int8());
			m_stencilPassOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilZFailOperation           = static_cast<StencilOperation>(iff.read_int8());
			m_stencilFailOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilWriteMask                = iff.read_uint32();
			m_stencilMask                     = iff.read_uint32();

			m_materialTag                     = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_DATA);

		if (iff.getCurrentName() == TAG_PFFP)
			m_fixedFunctionPipeline = new FixedFunctionPipeline(iff, false);
		else
			if (iff.getCurrentName() == TAG_PVSH)
			{
				char buffer[256];

				iff.enterForm(TAG_PVSH);
					iff.enterChunk(TAG_0000);
						iff.read_string(buffer, sizeof(buffer));
					iff.exitChunk(TAG_0000);
				iff.exitForm(TAG_PVSH);

				m_vertexShader = VertexShader::fetch(buffer);
			}
			else
				DEBUG_FATAL(true, ("unknown transform unit"));

		if (numberOfStages == 0 && !iff.atEndOfForm() && iff.getCurrentName() == TAG_PPSH)
		{
			m_pixelShader = new PixelShader(iff);
		}
		else
		{
			m_stage = new Stages;
			m_stage->reserve(static_cast<uint>(numberOfStages));
			for (int i = 0; i < numberOfStages; ++i)
				m_stage->push_back(new Stage(iff));
		}

	iff.exitForm(TAG_0007);
}

// ----------------------------------------------------------------------

void ShaderImplementationPass::load_0008(Iff &iff)
{
	iff.enterForm(TAG_0008);
		
		iff.enterChunk(TAG_DATA);

			const int  numberOfStages = iff.read_int8();

			m_shadeMode                       = static_cast<ShadeMode>(iff.read_int8());
			m_fogMode                         = static_cast<FogMode>(iff.read_int8());

			m_ditherEnable                    = iff.read_bool8();

			m_zEnable                         = iff.read_bool8();
			m_zWrite                          = iff.read_bool8();
			m_zCompare                        = static_cast<Compare>(iff.read_int8());

			m_alphaBlendEnable                = iff.read_bool8();
			m_alphaBlendOperation             = static_cast<BlendOperation>(iff.read_int8());
			m_alphaBlendSource                = static_cast<Blend>(iff.read_int8());
			m_alphaBlendDestination           = static_cast<Blend>(iff.read_int8());

			m_alphaTestEnable                 = iff.read_bool8();
			m_alphaTestReferenceValueTag      = static_cast<Tag>(iff.read_uint32());
			m_alphaTestFunction               = static_cast<Compare>(iff.read_int8());

			m_writeEnable                     = iff.read_uint8();

			m_textureFactorTag                = static_cast<Tag>(iff.read_uint32());
			m_textureFactorTag2               = static_cast<Tag>(iff.read_uint32());
			m_textureScrollTag                = static_cast<Tag>(iff.read_uint32());

			m_stencilEnable                   = iff.read_bool8();
			m_stencilReferenceValueTag        = static_cast<Tag>(iff.read_uint32());
			m_stencilCompareFunction          = static_cast<Compare>(iff.read_int8());
			m_stencilPassOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilZFailOperation           = static_cast<StencilOperation>(iff.read_int8());
			m_stencilFailOperation            = static_cast<StencilOperation>(iff.read_int8());
			m_stencilWriteMask                = iff.read_uint32();
			m_stencilMask                     = iff.read_uint32();

			m_materialTag                     = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_DATA);

		if (iff.getCurrentName() == TAG_PFFP)
			m_fixedFunctionPipeline = new FixedFunctionPipeline(iff, false);
		else
			if (iff.getCurrentName() == TAG_PVSH)
			{
				char buffer[256];

				iff.enterForm(TAG_PVSH);
					iff.enterChunk(TAG_0000);
						iff.read_string(buffer, sizeof(buffer));
					iff.exitChunk(TAG_0000);
				iff.exitForm(TAG_PVSH);

				m_vertexShader = VertexShader::fetch(buffer);
			}
			else
				DEBUG_FATAL(true, ("unknown transform unit"));

		if (numberOfStages == 0 && !iff.atEndOfForm() && iff.getCurrentName() == TAG_PPSH)
		{
			m_pixelShader = new PixelShader(iff);
		}
		else
		{
			m_stage = new Stages;
			m_stage->reserve(static_cast<uint>(numberOfStages));
			for (int i = 0; i < numberOfStages; ++i)
				m_stage->push_back(new Stage(iff));
		}

	iff.exitForm(TAG_0008);
}

// ----------------------------------------------------------------------

void ShaderImplementationPass::load_0009(Iff &iff)
{
	iff.enterForm(TAG_0009);
		
		iff.enterChunk(TAG_DATA);

			const int  numberOfStages = iff.read_int8();

			m_shadeMode                       = static_cast<ShadeMode>(iff.read_int8());
			m_fogMode                         = static_cast<FogMode>(iff.read_int8());

			m_ditherEnable                    = iff.read_bool8();

			m_zEnable                         = iff.read_bool8();
			m_zWrite                          = iff.read_bool8();
			m_zCompare                        = static_cast<Compare>(iff.read_int8());

			m_alphaBlendEnable                = iff.read_bool8();
			m_alphaBlendOperation             = static_cast<BlendOperation>(iff.read_int8());
			m_alphaBlendSource                = static_cast<Blend>(iff.read_int8());
			m_alphaBlendDestination           = static_cast<Blend>(iff.read_int8());

			m_alphaTestEnable                 = iff.read_bool8();
			m_alphaTestReferenceValueTag      = static_cast<Tag>(iff.read_uint32());
			m_alphaTestFunction               = static_cast<Compare>(iff.read_int8());

			m_writeEnable                     = iff.read_uint8();

			m_textureFactorTag                = static_cast<Tag>(iff.read_uint32());
			m_textureFactorTag2               = static_cast<Tag>(iff.read_uint32());
			m_textureScrollTag                = static_cast<Tag>(iff.read_uint32());

			m_stencilEnable                          = iff.read_bool8();
			m_stencilTwoSidedMode                    = iff.read_bool8();
			m_stencilReferenceValueTag               = static_cast<Tag>(iff.read_uint32());
			m_stencilCompareFunction                 = static_cast<Compare>(iff.read_int8());
			m_stencilPassOperation                   = static_cast<StencilOperation>(iff.read_int8());
			m_stencilZFailOperation                  = static_cast<StencilOperation>(iff.read_int8());
			m_stencilFailOperation                   = static_cast<StencilOperation>(iff.read_int8());
			m_stencilCounterClockwiseCompareFunction = static_cast<Compare>(iff.read_int8());
			m_stencilCounterClockwisePassOperation   = static_cast<StencilOperation>(iff.read_int8());
			m_stencilCounterClockwiseZFailOperation  = static_cast<StencilOperation>(iff.read_int8());
			m_stencilCounterClockwiseFailOperation   = static_cast<StencilOperation>(iff.read_int8());
			m_stencilWriteMask                       = iff.read_uint32();
			m_stencilMask                            = iff.read_uint32();

			m_materialTag                            = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_DATA);

		if (iff.getCurrentName() == TAG_PFFP)
			m_fixedFunctionPipeline = new FixedFunctionPipeline(iff, false);
		else
			if (iff.getCurrentName() == TAG_PVSH)
			{
				char buffer[256];

				iff.enterForm(TAG_PVSH);
					iff.enterChunk(TAG_0000);
						iff.read_string(buffer, sizeof(buffer));
					iff.exitChunk(TAG_0000);
				iff.exitForm(TAG_PVSH);

				m_vertexShader = VertexShader::fetch(buffer);
			}
			else
				DEBUG_FATAL(true, ("unknown transform unit"));

		if (numberOfStages == 0 && !iff.atEndOfForm() && iff.getCurrentName() == TAG_PPSH)
		{
			m_pixelShader = new PixelShader(iff);
		}
		else
		{
			m_stage = new Stages;
			m_stage->reserve(static_cast<uint>(numberOfStages));
			for (int i = 0; i < numberOfStages; ++i)
				m_stage->push_back(new Stage(iff));
		}

	iff.exitForm(TAG_0009);
}

// ----------------------------------------------------------------------

void ShaderImplementationPass::load_0010(Iff &iff)
{
	iff.enterForm(TAG_0010);
		
		iff.enterChunk(TAG_DATA);

			const int  numberOfStages = iff.read_int8();

			m_shadeMode                       = static_cast<ShadeMode>(iff.read_int8());
			m_fogMode                         = static_cast<FogMode>(iff.read_int8());

			m_ditherEnable                    = iff.read_bool8();
			m_heat                            = iff.read_bool8();

			m_zEnable                         = iff.read_bool8();
			m_zWrite                          = iff.read_bool8();
			m_zCompare                        = static_cast<Compare>(iff.read_int8());

			m_alphaBlendEnable                = iff.read_bool8();
			m_alphaBlendOperation             = static_cast<BlendOperation>(iff.read_int8());
			m_alphaBlendSource                = static_cast<Blend>(iff.read_int8());
			m_alphaBlendDestination           = static_cast<Blend>(iff.read_int8());

			m_alphaTestEnable                 = iff.read_bool8();
			m_alphaTestReferenceValueTag      = static_cast<Tag>(iff.read_uint32());
			m_alphaTestFunction               = static_cast<Compare>(iff.read_int8());

			m_writeEnable                     = iff.read_uint8();

			m_textureFactorTag                = static_cast<Tag>(iff.read_uint32());
			m_textureFactorTag2               = static_cast<Tag>(iff.read_uint32());
			m_textureScrollTag                = static_cast<Tag>(iff.read_uint32());

			m_stencilEnable                          = iff.read_bool8();
			m_stencilTwoSidedMode                    = iff.read_bool8();
			m_stencilReferenceValueTag               = static_cast<Tag>(iff.read_uint32());
			m_stencilCompareFunction                 = static_cast<Compare>(iff.read_int8());
			m_stencilPassOperation                   = static_cast<StencilOperation>(iff.read_int8());
			m_stencilZFailOperation                  = static_cast<StencilOperation>(iff.read_int8());
			m_stencilFailOperation                   = static_cast<StencilOperation>(iff.read_int8());
			m_stencilCounterClockwiseCompareFunction = static_cast<Compare>(iff.read_int8());
			m_stencilCounterClockwisePassOperation   = static_cast<StencilOperation>(iff.read_int8());
			m_stencilCounterClockwiseZFailOperation  = static_cast<StencilOperation>(iff.read_int8());
			m_stencilCounterClockwiseFailOperation   = static_cast<StencilOperation>(iff.read_int8());
			m_stencilWriteMask                       = iff.read_uint32();
			m_stencilMask                            = iff.read_uint32();

			m_materialTag                            = static_cast<Tag>(iff.read_uint32());

		iff.exitChunk(TAG_DATA);

		if (iff.getCurrentName() == TAG_PFFP)
			m_fixedFunctionPipeline = new FixedFunctionPipeline(iff, false);
		else
			if (iff.getCurrentName() == TAG_PVSH)
			{
				char buffer[256];

				iff.enterForm(TAG_PVSH);
					iff.enterChunk(TAG_0000);
						iff.read_string(buffer, sizeof(buffer));
					iff.exitChunk(TAG_0000);
				iff.exitForm(TAG_PVSH);

				m_vertexShader = VertexShader::fetch(buffer);
			}
			else
				DEBUG_FATAL(true, ("unknown transform unit"));

		if (numberOfStages == 0 && !iff.atEndOfForm() && iff.getCurrentName() == TAG_PPSH)
		{
			m_pixelShader = new PixelShader(iff);
		}
		else
		{
			m_stage = new Stages;
			m_stage->reserve(static_cast<uint>(numberOfStages));
			for (int i = 0; i < numberOfStages; ++i)
				m_stage->push_back(new Stage(iff));
		}

	iff.exitForm(TAG_0010);
}

// ----------------------------------------------------------------------

bool ShaderImplementationPass::usesMaterial(Tag materialTag) const
{
	return (materialTag == m_materialTag);
}

// ----------------------------------------------------------------------

bool ShaderImplementationPass::usesTexture(Tag textureTag) const
{
	if (m_pixelShader)
		return m_pixelShader->usesTexture(textureTag);

	Stages::iterator end = m_stage->end();
	for (Stages::iterator i = m_stage->begin(); i != end; ++i)
		if ((*i)->usesTexture(textureTag))
			return true;

	return false;
}

// ----------------------------------------------------------------------

bool ShaderImplementationPass::usesTextureCoordinateSet(Tag textureCoordinateSetTag) const
{
	if (m_vertexShader && m_vertexShader->usesTextureCoordinateSet(textureCoordinateSetTag))
		return true;
		
	if (m_stage)
	{
		Stages::iterator end = m_stage->end();
		for (Stages::iterator i = m_stage->begin(); i != end; ++i)
			if ((*i)->usesTextureCoordinateSet(textureCoordinateSetTag))
				return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool ShaderImplementationPass::usesTextureFactor(Tag textureFactorTag) const
{
	return (textureFactorTag == m_textureFactorTag) || (textureFactorTag == m_textureFactorTag2);
}

// ----------------------------------------------------------------------

bool ShaderImplementationPass::usesTextureScroll(Tag textureScrollTag) const
{
	return (textureScrollTag == m_textureScrollTag);
}

// ----------------------------------------------------------------------

bool ShaderImplementationPass::usesAlphaReference(Tag alphaReferenceTag) const
{
	return (alphaReferenceTag == m_alphaTestReferenceValueTag);
}

// ----------------------------------------------------------------------

bool ShaderImplementationPass::usesStencilReference(Tag stencilReferenceValueTag) const
{
	return (stencilReferenceValueTag == m_stencilReferenceValueTag);
}

// ----------------------------------------------------------------------

void ShaderImplementationPass::verifyCompatibility(VertexBufferFormat const & vertexBufferFormat, StaticShaderTemplate const & staticShaderTemplate) const
{
#ifdef _DEBUG
	if (m_vertexShader)
		m_vertexShader->verifyCompatibility(vertexBufferFormat, staticShaderTemplate);

	if (m_pixelShader)
		m_pixelShader->verifyCompatibility(vertexBufferFormat, staticShaderTemplate);

	if (m_fixedFunctionPipeline)
		m_fixedFunctionPipeline->verifyCompatibility(vertexBufferFormat, staticShaderTemplate);

	if (m_stage)
	{
		Stages::iterator const iEnd = m_stage->end();
		for (Stages::iterator i = m_stage->begin(); i != iEnd; ++i)
			(*i)->verifyCompatibility(vertexBufferFormat, staticShaderTemplate);
	}
#else
	UNREF(vertexBufferFormat);
	UNREF(staticShaderTemplate);
#endif
}

// ======================================================================

ShaderImplementationPassFixedFunctionPipeline::ShaderImplementationPassFixedFunctionPipeline(Iff &iff, bool old)
:
	m_lighting(false),
	m_lightingSpecularEnable(false),
	m_lightingColorVertex(false),
	m_deprecated_lightingMaterialTag(0),
	m_lightingAmbientColorSource(MS_Material),
	m_lightingDiffuseColorSource(MS_Material),
	m_lightingSpecularColorSource(MS_Material),
	m_lightingEmissiveColorSource(MS_Material)
{
	if (old)
		load_old(iff);
	else
		load(iff);

#ifdef _DEBUG
	if (GraphicsDebugFlags::disablePrecalculatedLighting)
		m_lightingColorVertex = false;
#endif
}

// ----------------------------------------------------------------------

ShaderImplementationPassFixedFunctionPipeline::~ShaderImplementationPassFixedFunctionPipeline()
{
}

// ----------------------------------------------------------------------

void ShaderImplementationPassFixedFunctionPipeline::load(Iff &iff)
{
	iff.enterForm(TAG_PFFP);

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
					DEBUG_FATAL(true, ("Unknown implementation version in %s", buffer));
				}
				break;
		}

	iff.exitForm(TAG_PFFP);
}

// ----------------------------------------------------------------------

void ShaderImplementationPassFixedFunctionPipeline::load_old(Iff &iff)
{
	m_lighting                        = iff.read_bool8();
	m_lightingSpecularEnable          = iff.read_bool8();
	m_lightingColorVertex             = iff.read_bool8();
	m_deprecated_lightingMaterialTag  = static_cast<Tag>(iff.read_uint32());
	m_lightingAmbientColorSource      = static_cast<MaterialSource>(iff.read_uint8());
	m_lightingDiffuseColorSource      = static_cast<MaterialSource>(iff.read_uint8());
	m_lightingSpecularColorSource     = static_cast<MaterialSource>(iff.read_uint8());
	m_lightingEmissiveColorSource     = static_cast<MaterialSource>(iff.read_uint8());
}

// ----------------------------------------------------------------------

void ShaderImplementationPassFixedFunctionPipeline::load_0000(Iff &iff)
{
	iff.enterChunk(TAG_0000);
		m_lighting                        = iff.read_bool8();
		m_lightingSpecularEnable          = iff.read_bool8();
		m_lightingColorVertex             = iff.read_bool8();
		m_deprecated_lightingMaterialTag  = static_cast<Tag>(iff.read_uint32());
		m_lightingAmbientColorSource      = static_cast<MaterialSource>(iff.read_uint8());
		m_lightingDiffuseColorSource      = static_cast<MaterialSource>(iff.read_uint8());
		m_lightingSpecularColorSource     = static_cast<MaterialSource>(iff.read_uint8());
		m_lightingEmissiveColorSource     = static_cast<MaterialSource>(iff.read_uint8());
	iff.exitChunk(TAG_0000);
}

// ----------------------------------------------------------------------

void ShaderImplementationPassFixedFunctionPipeline::load_0001(Iff &iff)
{
	iff.enterChunk(TAG_0001);
		m_lighting                        = iff.read_bool8();
		m_lightingSpecularEnable          = iff.read_bool8();
		m_lightingColorVertex             = iff.read_bool8();
		m_lightingAmbientColorSource      = static_cast<MaterialSource>(iff.read_uint8());
		m_lightingDiffuseColorSource      = static_cast<MaterialSource>(iff.read_uint8());
		m_lightingSpecularColorSource     = static_cast<MaterialSource>(iff.read_uint8());
		m_lightingEmissiveColorSource     = static_cast<MaterialSource>(iff.read_uint8());
	iff.exitChunk(TAG_0001);
}

// ----------------------------------------------------------------------

void ShaderImplementationPassFixedFunctionPipeline::verifyCompatibility(VertexBufferFormat const & vertexBufferFormat, StaticShaderTemplate const & ) const
{
#ifdef _DEBUG
	DEBUG_WARNING(m_lighting && !vertexBufferFormat.hasNormal(), ("Fixed function pipeline has lighting but vertex buffer missing normal"));
	DEBUG_WARNING(m_lighting && m_lightingColorVertex && (m_lightingAmbientColorSource == MS_VertexColor0 || m_lightingDiffuseColorSource == MS_VertexColor0 || m_lightingSpecularColorSource == MS_VertexColor0 || m_lightingEmissiveColorSource == MS_VertexColor0) && !vertexBufferFormat.hasColor0(), ("Fixed function pipeline requires missing vertex color0"));
	DEBUG_WARNING(m_lighting && m_lightingColorVertex && (m_lightingAmbientColorSource == MS_VertexColor1 || m_lightingDiffuseColorSource == MS_VertexColor1 || m_lightingSpecularColorSource == MS_VertexColor1 || m_lightingEmissiveColorSource == MS_VertexColor1) && !vertexBufferFormat.hasColor1(), ("Fixed function pipeline requires missing vertex color1"));
#else
	UNREF(vertexBufferFormat);
#endif
}

// ======================================================================

namespace ShaderImplementationPassVertexShaderNamespace
{
	typedef std::map<CrcString const *, ShaderImplementationPassVertexShader *, LessPointerComparator> VertexShaderMap;

	VertexShaderMap  ms_vertexShaderMap;
	RecursiveMutex   ms_criticalSection;

#if PRODUCTION == 0
	typedef std::map<std::string, ShaderImplementationPassVertexShader *> VertexShaderNameMap;
	
	VertexShaderNameMap s_vertexNameToOwnersMap;
#endif

}

namespace ShaderImplementationPassPixelShaderNamespace
{
#if PRODUCTION == 0
	typedef std::map<std::string, ShaderImplementationPassPixelShader *> PixelShaderNameMap;

	PixelShaderNameMap s_nameToOwnersMap;
#endif
}

#if PRODUCTION == 0
void ShaderImplementation::reloadPixelShader(const char *name)
{
	using namespace ShaderImplementationPassPixelShaderNamespace;
	std::string nameStr = std::string(name);
	PixelShaderNameMap::iterator it = s_nameToOwnersMap.find(nameStr);
	if(it == s_nameToOwnersMap.end())
		return;
	ShaderImplementationPassPixelShader *psh = it->second;
	psh->reloadShaderProgram();
	
}
#endif

#if PRODUCTION == 0
void ShaderImplementation::reloadVertexShader(const char *name)
{
	using namespace ShaderImplementationPassVertexShaderNamespace;
	std::string nameStr = std::string(name);
	VertexShaderNameMap::iterator it = s_vertexNameToOwnersMap.find(nameStr);
	if(it == s_vertexNameToOwnersMap.end())
		return;
	ShaderImplementationPassVertexShader *vsh = it->second;
	vsh->reloadShaderProgram();
	
}
#endif

// ======================================================================

void ShaderImplementationPassPixelShader::reloadShaderProgram()
{
	const_cast<ShaderImplementationPassPixelShaderProgram *>(m_program)->reload();
}

// ======================================================================

void ShaderImplementationPassVertexShader::reloadShaderProgram()
{
	using namespace ShaderImplementationPassVertexShaderNamespace;

	//Unload	
//	ShaderImplementationPassVertexShader::~ShaderImplementationPassVertexShader();

	{
		using namespace ShaderImplementationPassVertexShaderNamespace;

		VertexShaderMap::iterator i = ms_vertexShaderMap.find(&m_fileName);
		DEBUG_FATAL(i == ms_vertexShaderMap.end(), ("vertex shader %s not found", m_fileName));
		ms_vertexShaderMap.erase(i);

		delete [] m_text;
		delete m_graphicsData;

		m_text = 0;
		m_graphicsData = 0;
	}

	//Load
	if (ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability() || Graphics::getShaderCapability() >= ShaderCapability(1,1))
	{
		AbstractFile *file = TreeFile::open(m_fileName.getString(), AbstractFile::PriorityData, false);
		m_textLength = file->length();
		m_text = new char[m_textLength+1];
		file->read(m_text, m_textLength);
		m_text[m_textLength] = '\0';
		delete file;
		m_graphicsData = Graphics::createVertexShaderData(*this);
	}

	const bool result = ms_vertexShaderMap.insert(VertexShaderMap::value_type(&m_fileName, this)).second;
	UNREF(result);
	DEBUG_FATAL(!result, ("already existed!"));
}

// ======================================================================

ShaderImplementationPassVertexShaderGraphicsData::~ShaderImplementationPassVertexShaderGraphicsData()
{
}

// ======================================================================

void ShaderImplementationPassVertexShader::assignAsynchronousLoaderFunctions()
{
	if (AsynchronousLoader::isInstalled())
		AsynchronousLoader::bindFetchReleaseFunctions("vsh", &asynchronousLoaderFetchNoCreate, &asynchronousLoaderRelease);
}

// ----------------------------------------------------------------------

const void *ShaderImplementationPassVertexShader::asynchronousLoaderFetchNoCreate(const char *fileName)
{
	return fetch(fileName, false);
}

// ----------------------------------------------------------------------

void ShaderImplementationPassVertexShader::asynchronousLoaderRelease(const void *vertexShader)
{
	static_cast<const VertexShader *>(vertexShader)->release();
}

// ----------------------------------------------------------------------

const ShaderImplementationPassVertexShader *ShaderImplementationPassVertexShader::fetch(const char *fileName)
{
	return fetch(fileName, true);
}

// ----------------------------------------------------------------------

const ShaderImplementationPassVertexShader *ShaderImplementationPassVertexShader::fetch(const char *fileName, bool create)
{
	using namespace ShaderImplementationPassVertexShaderNamespace;

	VertexShader *result = NULL;

	TemporaryCrcString cfn(fileName, true);

	ms_criticalSection.enter();

		VertexShaderMap::iterator i = ms_vertexShaderMap.find(&cfn);
		if (i != ms_vertexShaderMap.end())
		{
			result = i->second;
			result->fetch();
		}
		else if (create)
			result = new VertexShader(cfn);

	ms_criticalSection.leave();

	// @todo use crc lower string
	return result;
}

//----------------------------------------------------------------------

void ShaderImplementationPassVertexShader::reportDangling()
{
	using namespace ShaderImplementationPassVertexShaderNamespace;
	for (VertexShaderMap::iterator it = ms_vertexShaderMap.begin(); it != ms_vertexShaderMap.end(); ++it)
	{
		CrcString const * const name = (*it).first;
		WARNING(true, ("ShaderImplementation [%s] still allocated", name->getString()));
	}
}

// ----------------------------------------------------------------------
// @todo only allocate if the items are necessary

ShaderImplementationPassVertexShader::ShaderImplementationPassVertexShader(CrcString const & fileName)
:
	m_referenceCount(1),
	m_fileName(fileName),
	m_text(NULL),
	m_textLength(0),
	m_graphicsData(NULL)
{
	using namespace ShaderImplementationPassVertexShaderNamespace;

	if (ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability() || Graphics::getShaderCapability() >= ShaderCapability(1,1))
	{
		load(fileName);
		m_graphicsData = Graphics::createVertexShaderData(*this);
	}

	const bool result = ms_vertexShaderMap.insert(VertexShaderMap::value_type(&m_fileName, this)).second;
	UNREF(result);
	DEBUG_FATAL(!result, ("already existed!"));
}

// ----------------------------------------------------------------------

ShaderImplementationPassVertexShader::~ShaderImplementationPassVertexShader()
{
	using namespace ShaderImplementationPassVertexShaderNamespace;

	VertexShaderMap::iterator i = ms_vertexShaderMap.find(&m_fileName);
	DEBUG_FATAL(i == ms_vertexShaderMap.end(), ("vertex shader %s not found", m_fileName));
	ms_vertexShaderMap.erase(i);

	delete [] m_text;
	delete m_graphicsData;
}

// ----------------------------------------------------------------------

void ShaderImplementationPassVertexShader::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------

void ShaderImplementationPassVertexShader::release() const
{
	using namespace ShaderImplementationPassVertexShaderNamespace;

	ms_criticalSection.enter();
		if (--m_referenceCount <= 0)
		{
			DEBUG_FATAL(m_referenceCount < 0, ("reference count %d", m_referenceCount));
				delete this;
		}
	ms_criticalSection.leave();
}

// ----------------------------------------------------------------------

void ShaderImplementationPassVertexShader::load(CrcString const &fileName)
{
	using namespace ShaderImplementationPassVertexShaderNamespace;

	AbstractFile *file = TreeFile::open(fileName.getString(), AbstractFile::PriorityData, false);
	m_textLength = file->length();
	m_text = new char[m_textLength+1];
	file->read(m_text, m_textLength);
	m_text[m_textLength] = '\0';
	delete file;

	#if PRODUCTION == 0
		s_vertexNameToOwnersMap.insert(std::make_pair(std::string(fileName.getString()), this));
	#endif
}

// ----------------------------------------------------------------------

bool ShaderImplementationPassVertexShader::usesTextureCoordinateSet(Tag textureCoordinateSetTag) const
{
#if 1
	UNREF(textureCoordinateSetTag);
	return true;
#else
	if (m_textureCoordinateSetTags)
	{
		TextureCoordinateSetTags::iterator iEnd = m_textureCoordinateSetTags->end();
		for (TextureCoordinateSetTags::iterator i = m_textureCoordinateSetTags->begin(); i != iEnd; ++i)
			if (textureCoordinateSetTag == *i)
				return true;
	}

	return false;
#endif
}

// ----------------------------------------------------------------------

void ShaderImplementationPassVertexShader::verifyCompatibility(VertexBufferFormat const & vertexBufferFormat, StaticShaderTemplate const & staticShaderTemplate) const
{
#if 0 // def _DEBUG
	// check texture coordinate set tags
	DEBUG_WARNING(m_vertexBufferFormat.hasPosition() && !vertexBufferFormat.hasPosition(), ("vertex shader requires missing position"));
	DEBUG_WARNING(m_vertexBufferFormat.hasNormal()   && !vertexBufferFormat.hasNormal(),   ("vertex shader requires missing normal"));
	DEBUG_WARNING(m_vertexBufferFormat.hasColor0()   && !vertexBufferFormat.hasColor0(),   ("vertex shader requires missing color0"));
	DEBUG_WARNING(m_vertexBufferFormat.hasColor1()   && !vertexBufferFormat.hasColor1(),   ("vertex shader requires missing color1"));

	if (m_textureCoordinateSetTags->empty())
	{
		// check texture coordinate sets literally
		DEBUG_WARNING(m_vertexBufferFormat.getNumberOfTextureCoordinateSets() > vertexBufferFormat.getNumberOfTextureCoordinateSets(), ("vertex shader requires more texture coordinate sets %d/%d", m_vertexBufferFormat.getNumberOfTextureCoordinateSets(), vertexBufferFormat.getNumberOfTextureCoordinateSets()));
		for (int i = 0; i < m_vertexBufferFormat.getNumberOfTextureCoordinateSets(); ++i)
		{
			DEBUG_WARNING(m_vertexBufferFormat.getTextureCoordinateSetDimension(i) > vertexBufferFormat.getTextureCoordinateSetDimension(i), ("vertex shader requires more texture coordinate sets dimensions @ %d %d/%d", i, m_vertexBufferFormat.getTextureCoordinateSetDimension(i), vertexBufferFormat.getTextureCoordinateSetDimension(i)));
		}
	}
	else
	{
		// check patch tags
		int numberOfTextureCoordinateSets = vertexBufferFormat.getNumberOfTextureCoordinateSets();
		TextureCoordinateSetTags::iterator iEnd = m_textureCoordinateSetTags->end();
		for (TextureCoordinateSetTags::iterator i = m_textureCoordinateSetTags->begin(); i != iEnd; ++i)
		{
			char buffer[5];
			ConvertTagToString(*i, buffer);

			uint8 index = 0;
			if (!staticShaderTemplate.getTextureCoordinateSetTag(*i, index))
				DEBUG_WARNING(true, ("vertex shader requires missing texture coordinate set tag %s", buffer));
			else
				DEBUG_WARNING(index >= numberOfTextureCoordinateSets, ("vertex shader references texture coordinate set %s, shader says index %d, but the vertex buffer only has %d sets", buffer, static_cast<int>(index), numberOfTextureCoordinateSets));
		}
	}
#else
	UNREF(vertexBufferFormat);
	UNREF(staticShaderTemplate);
#endif
}

// ======================================================================

ShaderImplementationPassStage::ShaderImplementationPassStage(Iff &iff)
:
	m_colorOperation(TO_modulate),
	m_colorArgument0(TA_temp),
	m_colorArgument1(TA_current),
	m_colorArgument2(TA_texture),

	m_alphaOperation(TO_modulate),
	m_alphaArgument0(TA_temp),
	m_alphaArgument1(TA_current),
	m_alphaArgument2(TA_texture),

	m_resultArgument(TA_current),

	m_textureTag(0),
	m_textureCoordinateSetTag(0),
	m_textureAddressU(TA_invalid),
	m_textureAddressV(TA_invalid),
	m_textureAddressW(TA_invalid),
	m_textureMipFilter(TF_invalid),
	m_textureMinificationFilter(TF_invalid),
	m_textureMagnificationFilter(TF_invalid),
	m_textureCoordinateGeneration(CG_passThru)
{
	load(iff);
}

// ----------------------------------------------------------------------

ShaderImplementationPassStage::~ShaderImplementationPassStage()
{
}

// ----------------------------------------------------------------------

void ShaderImplementationPassStage::load(Iff &iff)
{
	iff.enterForm(TAG_STAG);

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
					DEBUG_FATAL(true, ("Bad stage version %s", buffer));
				}
				break;
		}

	iff.exitForm(TAG_STAG);
}

// ----------------------------------------------------------------------

void ShaderImplementationPassStage::load_0000(Iff &iff)
{
	iff.enterChunk(TAG_0000);
		m_colorOperation               = static_cast<TextureOperation>(iff.read_uint8()); 

		m_colorArgument0               = static_cast<TextureArgument>(iff.read_uint8()); 
		m_colorArgument0Complement     = iff.read_bool8(); 
		m_colorArgument0AlphaReplicate = iff.read_bool8(); 

		m_colorArgument1               = static_cast<TextureArgument>(iff.read_uint8()); 
		m_colorArgument1Complement     = iff.read_bool8(); 
		m_colorArgument1AlphaReplicate = iff.read_bool8(); 

		m_colorArgument2               = static_cast<TextureArgument>(iff.read_uint8()); 
		m_colorArgument2Complement     = iff.read_bool8(); 
		m_colorArgument2AlphaReplicate = iff.read_bool8(); 

		m_alphaOperation               = static_cast<TextureOperation>(iff.read_uint8()); 

		m_alphaArgument0               = static_cast<TextureArgument>(iff.read_uint8()); 
		m_alphaArgument0Complement     = iff.read_bool8(); 

		m_alphaArgument1               = static_cast<TextureArgument>(iff.read_uint8()); 
		m_alphaArgument1Complement     = iff.read_bool8(); 

		m_alphaArgument2               = static_cast<TextureArgument>(iff.read_uint8()); 
		m_alphaArgument2Complement     = iff.read_bool8(); 

		m_resultArgument               = static_cast<TextureArgument>(iff.read_uint8()); 

		m_textureTag                   = iff.read_uint32(); 
		m_textureCoordinateSetTag      = iff.read_uint32(); 
		m_textureAddressU              = static_cast<TextureAddress>(iff.read_uint8()); 
		m_textureAddressV              = static_cast<TextureAddress>(iff.read_uint8()); 
		m_textureAddressW              = static_cast<TextureAddress>(iff.read_uint8()); 
		m_textureMipFilter             = static_cast<TextureFilter>(iff.read_uint8()); 
		m_textureMinificationFilter    = static_cast<TextureFilter>(iff.read_uint8()); 
		m_textureMagnificationFilter   = static_cast<TextureFilter>(iff.read_uint8()); 
		m_textureCoordinateGeneration  = static_cast<CoordinateGeneration>(iff.read_uint8()); 
	iff.exitChunk(TAG_0000);
}

// ----------------------------------------------------------------------

void ShaderImplementationPassStage::load_0001(Iff &iff)
{
	iff.enterChunk(TAG_0001);
		m_colorOperation               = static_cast<TextureOperation>(iff.read_uint8()); 

		m_colorArgument0               = static_cast<TextureArgument>(iff.read_uint8()); 
		m_colorArgument0Complement     = iff.read_bool8(); 
		m_colorArgument0AlphaReplicate = iff.read_bool8(); 

		m_colorArgument1               = static_cast<TextureArgument>(iff.read_uint8()); 
		m_colorArgument1Complement     = iff.read_bool8(); 
		m_colorArgument1AlphaReplicate = iff.read_bool8(); 

		m_colorArgument2               = static_cast<TextureArgument>(iff.read_uint8()); 
		m_colorArgument2Complement     = iff.read_bool8(); 
		m_colorArgument2AlphaReplicate = iff.read_bool8(); 

		m_alphaOperation               = static_cast<TextureOperation>(iff.read_uint8()); 

		m_alphaArgument0               = static_cast<TextureArgument>(iff.read_uint8()); 
		m_alphaArgument0Complement     = iff.read_bool8(); 

		m_alphaArgument1               = static_cast<TextureArgument>(iff.read_uint8()); 
		m_alphaArgument1Complement     = iff.read_bool8(); 

		m_alphaArgument2               = static_cast<TextureArgument>(iff.read_uint8()); 
		m_alphaArgument2Complement     = iff.read_bool8(); 

		m_resultArgument               = static_cast<TextureArgument>(iff.read_uint8()); 

		m_textureTag                   = iff.read_uint32();
		m_textureCoordinateSetTag      = iff.read_uint32();
		m_textureCoordinateGeneration  = static_cast<CoordinateGeneration>(iff.read_uint8()); 
	iff.exitChunk(TAG_0001);
}

// ----------------------------------------------------------------------

bool ShaderImplementationPassStage::usesTexture(Tag textureTag) const
{
	return (textureTag == m_textureTag);
}

// ----------------------------------------------------------------------

bool ShaderImplementationPassStage::usesTextureCoordinateSet(Tag textureCoordinateSetTag) const
{
	return (textureCoordinateSetTag == m_textureCoordinateSetTag);
}

// ----------------------------------------------------------------------

void ShaderImplementationPassStage::verifyCompatibility(VertexBufferFormat const & vertexBufferFormat , StaticShaderTemplate const & staticShaderTemplate) const
{
#ifdef _DEBUG
	if (m_textureTag)
	{
		char buffer[5];
		ConvertTagToString(m_textureTag, buffer);
		DEBUG_WARNING(!staticShaderTemplate.hasTextureData(m_textureTag), ("stage requires missing texture tag %s", buffer));
	}

	if (m_textureCoordinateSetTag)
	{
		char buffer[5];
		ConvertTagToString(m_textureCoordinateSetTag, buffer);

		uint8 index = 0;
		if (!staticShaderTemplate.getTextureCoordinateSetTag(m_textureCoordinateSetTag, index))
			DEBUG_WARNING(true, ("stage requires missing texture coordinate set tag %s", buffer));
		else
			DEBUG_WARNING(index >= vertexBufferFormat.getNumberOfTextureCoordinateSets(), ("stage references texture coordinate set %s, shader says index %d, but the vertex buffer only has %d sets", buffer, static_cast<int>(index), vertexBufferFormat.getNumberOfTextureCoordinateSets()));
	}
#else
	UNREF(vertexBufferFormat);	
	UNREF(staticShaderTemplate);	
#endif
}

// ======================================================================

ShaderImplementationPassPixelShader::ShaderImplementationPassPixelShader(Iff &iff)
:
	m_program(NULL),
	m_textureSamplers(new TextureSamplers),
	m_maxTextureSampler(-1)
{
	load(iff);
}

// ----------------------------------------------------------------------

ShaderImplementationPassPixelShader::~ShaderImplementationPassPixelShader()
{
	TextureSamplers::iterator end = m_textureSamplers->end();
	for (TextureSamplers::iterator i = m_textureSamplers->begin(); i != end; ++i)
		delete (*i);

	m_program->release();
	delete m_textureSamplers;
}

// ----------------------------------------------------------------------

void ShaderImplementationPassPixelShader::load(Iff &iff)
{
	iff.enterForm(TAG_PPSH);

		switch (iff.getCurrentName())
		{
			case TAG_0001:
				load_0001(iff);
				break;

			default:
				{
					char buffer[256];
					iff.formatLocation(buffer, sizeof(buffer));
					DEBUG_FATAL(true, ("Bad pixel shader version %s", buffer));
				}
				break;
		}

	iff.exitForm(TAG_PPSH);
}

// ----------------------------------------------------------------------

void ShaderImplementationPassPixelShader::load_0001(Iff &iff)
{
	using namespace ShaderImplementationPassPixelShaderNamespace;

	iff.enterForm(TAG_0001);

		// load the texture samplers
		iff.enterChunk(TAG_DATA);
			const int numberOfTextureSamplers = iff.read_int8();
			char buffer[256];
			iff.read_string(buffer, sizeof(buffer));
#if PRODUCTION == 0
			s_nameToOwnersMap.insert(std::make_pair(std::string(buffer), this));
#endif
			m_program = Program::fetch(buffer);
		iff.exitChunk(TAG_DATA);

		m_textureSamplers->reserve(numberOfTextureSamplers);
		for (int i = 0; i < numberOfTextureSamplers; ++i)
		{
			TextureSampler *textureSampler = new TextureSampler(iff);
			m_textureSamplers->push_back(textureSampler);
			if (textureSampler->m_textureIndex > m_maxTextureSampler)
				m_maxTextureSampler = textureSampler->m_textureIndex;
		}

	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

bool ShaderImplementationPassPixelShader::usesTexture(Tag textureTag) const
{
	TextureSamplers::iterator const iEnd = m_textureSamplers->end();
	for (TextureSamplers::iterator i = m_textureSamplers->begin(); i != iEnd; ++i)
		if ((*i)->usesTexture(textureTag))
			return true;

	return false;
}

// ----------------------------------------------------------------------

void ShaderImplementationPassPixelShader::verifyCompatibility(VertexBufferFormat const & vertexBufferFormat, StaticShaderTemplate const & staticShaderTemplate) const
{
#ifdef _DEBUG
	TextureSamplers::iterator const iEnd = m_textureSamplers->end();
	for (TextureSamplers::iterator i = m_textureSamplers->begin(); i != iEnd; ++i)
		(*i)->verifyCompatibility(vertexBufferFormat, staticShaderTemplate);
#else
	UNREF(vertexBufferFormat);	
	UNREF(staticShaderTemplate);	
#endif
}

// ======================================================================

ShaderImplementationPassPixelShaderProgramGraphicsData::~ShaderImplementationPassPixelShaderProgramGraphicsData()
{
}

// ======================================================================

namespace ShaderImplementationPassPixelShaderProgramNamespace
{
	typedef std::map<CrcString const *, ShaderImplementationPassPixelShaderProgram *, LessPointerComparator> ProgramMap;

	static ProgramMap      ms_programMap;
	static RecursiveMutex  ms_criticalSection;
}

// ======================================================================

void ShaderImplementationPassPixelShaderProgram::assignAsynchronousLoaderFunctions()
{
	if (AsynchronousLoader::isInstalled())
		AsynchronousLoader::bindFetchReleaseFunctions("psh", &asynchronousLoaderFetchNoCreate, &asynchronousLoaderRelease);
}

// ----------------------------------------------------------------------

const void *ShaderImplementationPassPixelShaderProgram::asynchronousLoaderFetchNoCreate(const char *fileName)
{
	return fetch(fileName, false);
}

// ----------------------------------------------------------------------

void ShaderImplementationPassPixelShaderProgram::asynchronousLoaderRelease(const void *program)
{
	static_cast<const Program *>(program)->release();
}

// ----------------------------------------------------------------------

const ShaderImplementationPassPixelShaderProgram *ShaderImplementationPassPixelShaderProgram::fetch(const char *fileName)
{
	return fetch(fileName, true);
}

// ----------------------------------------------------------------------

const ShaderImplementationPassPixelShaderProgram *ShaderImplementationPassPixelShaderProgram::fetch(const char *fileName, bool create)
{
	using namespace ShaderImplementationPassPixelShaderProgramNamespace;

	Program *result = NULL;

	TemporaryCrcString cfn(fileName, true);

	ms_criticalSection.enter();

		ProgramMap::iterator i = ms_programMap.find(&cfn);
		if (i != ms_programMap.end())
		{
			result = i->second;
			result->fetch();
		}
		else
			if (create)
				result = new Program(cfn);

	ms_criticalSection.leave();

	// @todo use crc lower string
	return result;
}

// ----------------------------------------------------------------------

ShaderImplementationPassPixelShaderProgram::ShaderImplementationPassPixelShaderProgram(CrcString const &fileName)
:
	m_referenceCount(1),
	m_fileName(fileName),
	m_exe(NULL),
	m_graphicsData(NULL)
{
	using namespace ShaderImplementationPassPixelShaderProgramNamespace;

	if (ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability() || Graphics::getShaderCapability() >= ShaderCapability(1,1))
	{
		Iff iff(fileName.getString());
		load(iff);

		m_graphicsData = Graphics::createPixelShaderProgramData(*this);
	}

	const bool result = ms_programMap.insert(ProgramMap::value_type(&m_fileName, this)).second;
	UNREF(result);
	DEBUG_FATAL(!result, ("already existed!"));
}

// ----------------------------------------------------------------------

ShaderImplementationPassPixelShaderProgram::~ShaderImplementationPassPixelShaderProgram()
{
	using namespace ShaderImplementationPassPixelShaderProgramNamespace;

	ProgramMap::iterator i = ms_programMap.find(&m_fileName);
	DEBUG_FATAL(i == ms_programMap.end(), ("pixel program %s not found", m_fileName));
	ms_programMap.erase(i);

	delete [] m_exe;
	delete m_graphicsData;
}

// ----------------------------------------------------------------------

char const * ShaderImplementationPassPixelShaderProgram::getFileName() const
{
	return m_fileName.getString();
}

// ----------------------------------------------------------------------

void ShaderImplementationPassPixelShaderProgram::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------

void ShaderImplementationPassPixelShaderProgram::release() const
{
	using namespace ShaderImplementationPassPixelShaderProgramNamespace;

	ms_criticalSection.enter();
		if (--m_referenceCount <= 0)
		{
			DEBUG_FATAL(m_referenceCount < 0, ("reference count %d", m_referenceCount));
			delete this;
		}
	ms_criticalSection.leave();
}

// ----------------------------------------------------------------------

void ShaderImplementationPassPixelShaderProgram::reload()
{
	//Unload
	using namespace ShaderImplementationPassPixelShaderProgramNamespace;
	
//	ShaderImplementationPassPixelShaderProgram::~ShaderImplementationPassPixelShaderProgram();

	{
		using namespace ShaderImplementationPassPixelShaderProgramNamespace;

		ProgramMap::iterator i = ms_programMap.find(&m_fileName);
		DEBUG_FATAL(i == ms_programMap.end(), ("pixel program %s not found", m_fileName));
		ms_programMap.erase(i);

		delete [] m_exe;
		delete m_graphicsData;

		m_exe = 0;
		m_graphicsData = 0;
	}

	//Load
	if (ConfigClientGraphics::getLoadAllAssetsRegardlessOfShaderCapability() || Graphics::getShaderCapability() >= ShaderCapability(1,1))
	{
		Iff iff(m_fileName.getString());
		load(iff);

		m_graphicsData = Graphics::createPixelShaderProgramData(*this);
	}

	const bool result = ms_programMap.insert(ProgramMap::value_type(&m_fileName, this)).second;
	UNREF(result);
	DEBUG_FATAL(!result, ("already existed!"));

	ms_criticalSection.leave();
}

// ----------------------------------------------------------------------

int ShaderImplementationPassPixelShaderProgram::getVersionMajor() const
{
	return (m_exe[0] >> 8) & 0xff;
}

// ----------------------------------------------------------------------

int ShaderImplementationPassPixelShaderProgram::getVersionMinor() const
{
	return (m_exe[0] >> 0) & 0xff;
}

// ----------------------------------------------------------------------

void ShaderImplementationPassPixelShaderProgram::load(Iff &iff)
{
	iff.enterForm(TAG_PSHP);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char buffer[256];
					iff.formatLocation(buffer, sizeof(buffer));
					DEBUG_FATAL(true, ("Bad pixel program version %s", buffer));
				}
				break;
		}

	iff.exitForm(TAG_PSHP);
}

// ----------------------------------------------------------------------

void ShaderImplementationPassPixelShaderProgram::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		// load the source code
		iff.enterChunk(TAG_PSRC);
		iff.exitChunk(TAG_PSRC, true);

		// load the d3d8 executable data
		iff.enterChunk(TAG_PEXE);
			const int exeLength = iff.getChunkLengthLeft(sizeof(DWORD));
			m_exe = new DWORD[exeLength];
			iff.read_uint32(exeLength, m_exe);
		iff.exitChunk(TAG_PEXE);

	iff.exitForm(TAG_0000);
}

// ======================================================================

ShaderImplementationPassPixelShaderTextureSampler::ShaderImplementationPassPixelShaderTextureSampler(Iff &iff)
:
	m_textureIndex(0),
	m_textureTag(0),
	m_textureAddressU(ShaderImplementationPassStage::TA_invalid),
	m_textureAddressV(ShaderImplementationPassStage::TA_invalid),
	m_textureAddressW(ShaderImplementationPassStage::TA_invalid),
	m_textureMipFilter(ShaderImplementationPassStage::TF_invalid),
	m_textureMinificationFilter(ShaderImplementationPassStage::TF_invalid),
	m_textureMagnificationFilter(ShaderImplementationPassStage::TF_invalid)
{
	load(iff);
}

// ----------------------------------------------------------------------

ShaderImplementationPassPixelShaderTextureSampler::~ShaderImplementationPassPixelShaderTextureSampler()
{
}

// ----------------------------------------------------------------------

void ShaderImplementationPassPixelShaderTextureSampler::load(Iff &iff)
{
	iff.enterForm(TAG_PTXM);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff);
				break;

			case TAG_0001:
				load_0001(iff);
				break;

			case TAG_0002:
				load_0002(iff);
				break;

			default:
				{
					char buffer[256];
					iff.formatLocation(buffer, sizeof(buffer));
					DEBUG_FATAL(true, ("Bad texture sampler version %s", buffer));
				}
				break;
		}

	iff.exitForm(TAG_PTXM);
}

// ----------------------------------------------------------------------

void ShaderImplementationPassPixelShaderTextureSampler::load_0000(Iff &iff)
{
	iff.enterChunk(TAG_0000);
		m_textureIndex                 = iff.read_int8(); 
		m_textureTag                   = iff.read_uint32(); 
		IGNORE_RETURN(iff.read_uint32()); // m_textureCoordinateSetTag
		m_textureAddressU              = static_cast<ShaderImplementationPassStage::TextureAddress>(iff.read_uint8()); 
		m_textureAddressV              = static_cast<ShaderImplementationPassStage::TextureAddress>(iff.read_uint8()); 
		m_textureAddressW              = static_cast<ShaderImplementationPassStage::TextureAddress>(iff.read_uint8()); 
		m_textureMipFilter             = static_cast<ShaderImplementationPassStage::TextureFilter>(iff.read_uint8()); 
		m_textureMinificationFilter    = static_cast<ShaderImplementationPassStage::TextureFilter>(iff.read_uint8()); 
		m_textureMagnificationFilter   = static_cast<ShaderImplementationPassStage::TextureFilter>(iff.read_uint8()); 
		IGNORE_RETURN(iff.read_uint8()); // m_textureCoordinateGeneration
	iff.exitChunk(TAG_0000);
}

// ----------------------------------------------------------------------

void ShaderImplementationPassPixelShaderTextureSampler::load_0001(Iff &iff)
{
	iff.enterChunk(TAG_0001);
		m_textureIndex                 = iff.read_int8(); 
		m_textureTag                   = iff.read_uint32(); 
		IGNORE_RETURN(iff.read_uint32()); // m_textureCoordinateSetTag
		IGNORE_RETURN(iff.read_uint8());  // m_textureCoordinateGeneration
	iff.exitChunk(TAG_0001, true);
}

// ----------------------------------------------------------------------

void ShaderImplementationPassPixelShaderTextureSampler::load_0002(Iff &iff)
{
	iff.enterChunk(TAG_0002);
		m_textureIndex                 = iff.read_int8(); 
		m_textureTag                   = iff.read_uint32(); 
	iff.exitChunk(TAG_0002);
}

// ----------------------------------------------------------------------

bool ShaderImplementationPassPixelShaderTextureSampler::usesTexture(Tag textureTag) const
{
	return (textureTag == m_textureTag);
}

// ----------------------------------------------------------------------

void ShaderImplementationPassPixelShaderTextureSampler::verifyCompatibility(VertexBufferFormat const &, StaticShaderTemplate const & staticShaderTemplate) const
{
#ifdef _DEBUG
	char buffer[5];
	ConvertTagToString(m_textureTag, buffer);
	DEBUG_WARNING(!staticShaderTemplate.hasTextureData(m_textureTag), ("stage requires missing texture tag %s", buffer));
#else
	UNREF(staticShaderTemplate);	
#endif
}

//----------------------------------------------------------------------

bool ShaderImplementation::hasOptionTag(Tag tag) const
{
	return std::binary_search(m_optionTags.begin(), m_optionTags.end(), tag);
}

//----------------------------------------------------------------------

std::vector<Tag> const & ShaderImplementation::getOptionTags() const
{
	return m_optionTags;
}

//----------------------------------------------------------------------

bool ShaderImplementation::isHeatPass(int pass) const
{
	WithinRangeInclusiveInclusive(0, pass, static_cast<int>(m_pass->size()) - 1);
	return (*m_pass)[pass]->isHeat();
}

// ======================================================================
