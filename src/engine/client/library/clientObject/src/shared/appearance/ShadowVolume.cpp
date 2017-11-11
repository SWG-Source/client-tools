//===================================================================
//
// ShadowVolume.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ShadowVolume.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/DynamicIndexBuffer.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/SystemIndexBuffer.h"
#include "clientGraphics/SystemVertexBuffer.h"
#include "clientObject/ConfigClientObject.h"
#include "clientObject/ShadowManager.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/Appearance.h"
#include "sharedMath/Sphere.h"

#include <vector>
#include <algorithm>

//===================================================================

#define SHADOW_EXTRUDE_TO_POINT 1

//===================================================================
// anonymous
//===================================================================

namespace ShadowVolumeNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct ShadowEdgePrimitive
	{
	public:

		SystemVertexBuffer* shadowEdgeVertexBuffer;
		int                 shadowEdgeVertexCount;

		SystemIndexBuffer*  shadowEdgeIndexBuffer;
		int                 shadowEdgeIndexCount;

	public:

		ShadowEdgePrimitive () :
			shadowEdgeVertexBuffer (0),
			shadowEdgeVertexCount (0),
			shadowEdgeIndexBuffer (0),
			shadowEdgeIndexCount (0)
		{
		}

		int getMemorySize () const
		{
			int memorySize = 0;

			memorySize += isizeof (ShadowEdgePrimitive);
			memorySize += isizeof (SystemVertexBuffer);
			memorySize += shadowEdgeVertexBuffer ? (shadowEdgeVertexBuffer->getNumberOfVertices () * shadowEdgeVertexBuffer->getVertexSize ()) : 0;
			memorySize += isizeof (SystemIndexBuffer);
			memorySize += shadowEdgeIndexBuffer ? (isizeof (Index) * shadowEdgeIndexBuffer->getNumberOfIndices ()) : 0;

			return memorySize;
		}
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool        ms_allowShadowSubmissions;
	bool        ms_permanentlyDisableShadowVolumes;
	bool        ms_viewer;
	const float ms_shadowVolumeExtrudeDistance = 256.f;
	VectorArgb  ms_interiorShadowColor (0.1f, 0.f, 0.f, 0.f);
	int         ms_maximumVertexBufferSize = 0;

	char        ms_crashReportInfo[MAX_PATH * 2];

	typedef std::vector<ShadowVolume*> ShadowVolumeList;
	ShadowVolumeList ms_shadowVolumeList;

#ifdef _DEBUG
	ShadowVolumeList ms_renderedShadowVolumeList;
#endif

	typedef std::vector<ShadowEdgePrimitive> ShadowEdgePrimitiveList;
	ShadowEdgePrimitiveList ms_shadowEdgePrimitiveList;

	uint ms_numberOfShadowEdgePrimitives;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Index findOrAdd (Vector* vList, int& numberOfVertices, const Vector& vector)
	{
		int i;
		for (i = 0; i < numberOfVertices; ++i)
			if (vList [i] == vector)
				return static_cast<Index> (i);

		vList [i] = vector;

		++numberOfVertices;

		return static_cast<Index> (i);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void setVertexBuffer (const SystemVertexBuffer* const systemVertexBuffer, const int numberOfVertices)
	{
		DynamicVertexBuffer vertexBuffer (systemVertexBuffer->getFormat ());
		vertexBuffer.lock (numberOfVertices);
		VertexBufferWriteIterator v = vertexBuffer.begin ();
		v.copy (systemVertexBuffer->beginReadOnly (), numberOfVertices);
		vertexBuffer.unlock ();
		Graphics::setVertexBuffer (vertexBuffer);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void setIndexBuffer (const SystemIndexBuffer* const systemIndexBuffer, const int numberOfIndices)
	{
		DynamicIndexBuffer indexBuffer;
		indexBuffer.lock (numberOfIndices);
		indexBuffer.copyIndices (0, systemIndexBuffer->beginReadOnly (), 0, numberOfIndices);
		indexBuffer.unlock ();
		Graphics::setIndexBuffer (indexBuffer);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace ShadowVolumeNamespace;

//===================================================================
// ShadowVolume::ShadowPrimitive
//===================================================================

struct ShadowVolume::ShadowPrimitive
{
public:

	//-- extremely compact vertex, index, face, and edge data
	Vector*             compactVertexArray;
	int                 compactVertexCount;
	Index*              compactIndexArray;
	Index*              compactIndexArrayToIndexArrayMap;
	int                 compactIndexCount;
	Vector*             compactFaceNormalArray;
	bool*               compactFaceDotTestArray;
	int                 compactFaceCount;
	Edge*               compactEdgeArray;
	int                 compactEdgeCount;
	bool                computedEdgeConnectivity;

	SystemVertexBuffer* shadowVertexBuffer;
	SystemIndexBuffer*  shadowFrontIndexBuffer;
	int                 shadowFrontIndexCount;
	SystemIndexBuffer*  shadowBackIndexBuffer;
	int                 shadowBackIndexCount;

public:

	ShadowPrimitive () :
		compactVertexArray (0),
		compactVertexCount (0),
		compactIndexArray (0),
		compactIndexArrayToIndexArrayMap (0),
		compactIndexCount (0),
		compactFaceNormalArray (0),
		compactFaceDotTestArray (0),
		compactFaceCount (0),
		compactEdgeArray (0),
		compactEdgeCount (0),
		computedEdgeConnectivity (false),
		shadowVertexBuffer (0),
		shadowFrontIndexBuffer (0),
		shadowFrontIndexCount (0),
		shadowBackIndexBuffer (0),
		shadowBackIndexCount (0)
	{
	}

	bool isEmpty () const
	{
		return compactIndexCount == 0;
	}

	int getMemorySize () const
	{
		if (isEmpty ())
			return 0;

		int memorySize = 0;

		memorySize += isizeof (ShadowPrimitive);
		memorySize += isizeof (Vector) * compactVertexCount;
		memorySize += 2 * isizeof (Index) * compactIndexCount;
		memorySize += isizeof (Vector) * compactFaceCount;
		memorySize += isizeof (bool) * compactFaceCount;
		memorySize += isizeof (Edge) * compactEdgeCount;
		memorySize += isizeof (SystemVertexBuffer);
		memorySize += shadowVertexBuffer ? (shadowVertexBuffer->getNumberOfVertices () * shadowVertexBuffer->getVertexSize ()) : 0;
		memorySize += 2 * isizeof (SystemIndexBuffer);
		memorySize += shadowFrontIndexBuffer ? (isizeof (Index) * shadowFrontIndexBuffer->getNumberOfIndices ()) : 0;
		memorySize += shadowBackIndexBuffer ? (isizeof (Index) * shadowBackIndexBuffer->getNumberOfIndices ()) : 0;

		return memorySize;
	}
};

//===================================================================
// ProxyLocalShaderPrimitive
//===================================================================

class ProxyLocalShaderPrimitive : public ShaderPrimitive
{
public:

	enum Mode
	{
		M_extrude,
		M_prepareFarCap
	};

public:

	static void  install (void);
	static void  remove (void);

	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

public:

	ProxyLocalShaderPrimitive(
		const Shader& shader, 
		const ShadowVolume& shadowVolume, 
		const Object& object, 
		const Appearance &appearance,
		bool isInWorldCell, Mode mode
		);
	virtual ~ProxyLocalShaderPrimitive ();

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

private:

	ProxyLocalShaderPrimitive ();
	ProxyLocalShaderPrimitive (const ProxyLocalShaderPrimitive& rhs);             //lint -esym(754, ProxyLocalShaderPrimitive::ProxyLocalShaderPrimitive)
	ProxyLocalShaderPrimitive& operator= (const ProxyLocalShaderPrimitive& rhs);  //lint -esym(754, ProxyLocalShaderPrimitive::operator=)

private:

	static MemoryBlockManager* ms_memoryBlockManager;

private:

	const Shader&                        m_shader;
	const ShadowVolume&                  m_shadowVolume;
	const Object&                        m_object;
	const Appearance&                    m_appearance;
	const bool                           m_isInWorldCell;
	const Mode                           m_mode;
};

//-------------------------------------------------------------------

MemoryBlockManager* ProxyLocalShaderPrimitive::ms_memoryBlockManager;

//-------------------------------------------------------------------

void ProxyLocalShaderPrimitive::install (void)
{
	DEBUG_FATAL (ms_memoryBlockManager, ("ProxyLocalShaderPrimitive already installed"));

	ms_memoryBlockManager = new MemoryBlockManager ("ProxyLocalShaderPrimitive::ms_memoryBlockManager", false, sizeof(ProxyLocalShaderPrimitive), 256, 1, 0);
}

//-------------------------------------------------------------------

void ProxyLocalShaderPrimitive::remove (void)
{
	DEBUG_FATAL (!ms_memoryBlockManager, ("ProxyLocalShaderPrimitive is not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//-------------------------------------------------------------------

void* ProxyLocalShaderPrimitive::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_memoryBlockManager);
	DEBUG_FATAL (size != sizeof (ProxyLocalShaderPrimitive), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate ();
}

//-------------------------------------------------------------------

void ProxyLocalShaderPrimitive::operator delete (void* pointer)
{
	NOT_NULL (ms_memoryBlockManager);

	if (pointer)
		ms_memoryBlockManager->free (pointer);
}

//-------------------------------------------------------------------

ProxyLocalShaderPrimitive::ProxyLocalShaderPrimitive (const Shader& shader, 
																		const ShadowVolume& shadowVolume, 
																		const Object& object, 
																		const Appearance& appearance, 
																		const bool isInWorldCell, 
																		const Mode mode
																		) :
	ShaderPrimitive (),
	m_shader (shader),
	m_shadowVolume (shadowVolume),
	m_object (object),
	m_appearance(appearance),
	m_isInWorldCell (isInWorldCell),
	m_mode (mode)
{
}

//-------------------------------------------------------------------

ProxyLocalShaderPrimitive::~ProxyLocalShaderPrimitive ()
{
}

//-------------------------------------------------------------------

const Vector ProxyLocalShaderPrimitive::getPosition_w() const
{
	return Vector::zero;
}

//-------------------------------------------------------------------

float ProxyLocalShaderPrimitive::getDepthSquaredSortKey () const
{
	return 0.f;
}

//-------------------------------------------------------------------

int ProxyLocalShaderPrimitive::getVertexBufferSortKey () const
{
	return 0;
}

//-------------------------------------------------------------------

const StaticShader& ProxyLocalShaderPrimitive::prepareToView () const
{
	return m_shader.prepareToView ();
}

//-------------------------------------------------------------------

void ProxyLocalShaderPrimitive::prepareToDraw () const
{
	switch (m_mode)
	{
	case M_extrude:
		{
			Graphics::setObjectToWorldTransformAndScale (m_appearance.getTransform_w (), m_shadowVolume.m_primitiveType == ShadowVolume::PT_static ? m_appearance.getScale () : Vector::xyz111);

			//-- is the object in an interior?
			static Vector skewedUnitY (0.05f, 0.95f, 0.05f);

			const Vector directionToLight = (ms_viewer || m_isInWorldCell) ? ShadowVolume::getDirectionToLight () : skewedUnitY;
			m_shadowVolume.extrudeShadowVolume(m_object.getTransform_o2c().rotate_p2l(directionToLight));			
		}
		break;

	case M_prepareFarCap:
		{
			Transform t = m_appearance.getTransform_w ();
			t.move_p (-ShadowVolume::getDirectionToLight () * ms_shadowVolumeExtrudeDistance);

			Graphics::setObjectToWorldTransformAndScale (t, m_shadowVolume.m_primitiveType == ShadowVolume::PT_static ? m_appearance.getScale () : Vector::xyz111);
		}
		break;
	}
}

//-------------------------------------------------------------------

void ProxyLocalShaderPrimitive::draw () const
{
}

//===================================================================
// ShadowVolume::LocalShaderPrimitiveRenderEdgesOneSided
//===================================================================

class ShadowVolume::LocalShaderPrimitiveRenderEdgesOneSided : public ShaderPrimitive
{
public:

	LocalShaderPrimitiveRenderEdgesOneSided (const Shader& shader, const ShadowVolume& shadowVolume, GlCullMode cullMode);
	virtual ~LocalShaderPrimitiveRenderEdgesOneSided ();

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

private:

	LocalShaderPrimitiveRenderEdgesOneSided ();
	LocalShaderPrimitiveRenderEdgesOneSided (const LocalShaderPrimitiveRenderEdgesOneSided& rhs);             //lint -esym(754, LocalShaderPrimitiveRenderEdgesOneSided::LocalShaderPrimitiveRenderEdgesOneSided)
	LocalShaderPrimitiveRenderEdgesOneSided& operator= (const LocalShaderPrimitiveRenderEdgesOneSided& rhs);  //lint -esym(754, LocalShaderPrimitiveRenderEdgesOneSided::operator=)

private:

	const Shader&               m_shader;
	const ShadowVolume&         m_shadowVolume;
	const GlCullMode            m_cullMode;
};

//-------------------------------------------------------------------
// PUBLIC ShadowVolume::LocalShaderPrimitiveRenderEdgesOneSided
//-------------------------------------------------------------------

ShadowVolume::LocalShaderPrimitiveRenderEdgesOneSided::LocalShaderPrimitiveRenderEdgesOneSided (const Shader& shader, const ShadowVolume& shadowVolume, GlCullMode cullMode) :
	ShaderPrimitive (),
	m_shader (shader),
	m_shadowVolume (shadowVolume),
	m_cullMode (cullMode)
{
}

//-------------------------------------------------------------------

ShadowVolume::LocalShaderPrimitiveRenderEdgesOneSided::~LocalShaderPrimitiveRenderEdgesOneSided ()
{
}

//-------------------------------------------------------------------

const Vector ShadowVolume::LocalShaderPrimitiveRenderEdgesOneSided::getPosition_w() const
{
	return Vector::zero;
}

//-------------------------------------------------------------------

float ShadowVolume::LocalShaderPrimitiveRenderEdgesOneSided::getDepthSquaredSortKey () const
{
	return 0.f;
}

// ----------------------------------------------------------------------

int ShadowVolume::LocalShaderPrimitiveRenderEdgesOneSided::getVertexBufferSortKey () const
{
	return 0;
}

//-------------------------------------------------------------------

const StaticShader& ShadowVolume::LocalShaderPrimitiveRenderEdgesOneSided::prepareToView () const
{
	return m_shader.prepareToView ();
}

//-------------------------------------------------------------------

void ShadowVolume::LocalShaderPrimitiveRenderEdgesOneSided::prepareToDraw () const
{
}

//-------------------------------------------------------------------

void ShadowVolume::LocalShaderPrimitiveRenderEdgesOneSided::draw () const
{
	Graphics::setCullMode (m_cullMode);
	m_shadowVolume.renderShadowVolumeEdges ();
}

//===================================================================
// ShadowVolume::LocalShaderPrimitiveRenderCapsOneSided
//===================================================================

class ShadowVolume::LocalShaderPrimitiveRenderCapsOneSided : public ShaderPrimitive
{
public:

	LocalShaderPrimitiveRenderCapsOneSided (const Shader& shader, ShadowVolume& shadowVolume, const GlCullMode cullMode, const ShadowVolume::CapMode capMode);
	virtual ~LocalShaderPrimitiveRenderCapsOneSided ();

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

private:

	LocalShaderPrimitiveRenderCapsOneSided ();
	LocalShaderPrimitiveRenderCapsOneSided (const LocalShaderPrimitiveRenderCapsOneSided& rhs);             //lint -esym(754, LocalShaderPrimitiveRenderCapsOneSided::LocalShaderPrimitiveRenderCapsOneSided)
	LocalShaderPrimitiveRenderCapsOneSided& operator= (const LocalShaderPrimitiveRenderCapsOneSided& rhs);  //lint -esym(754, LocalShaderPrimitiveRenderCapsOneSided::operator=)

private:

	const Shader&               m_shader;
	ShadowVolume&               m_shadowVolume;
	const GlCullMode            m_cullMode;
	const ShadowVolume::CapMode m_capMode;
};

//-------------------------------------------------------------------
// PUBLIC ShadowVolume::LocalShaderPrimitiveRenderCapsOneSided
//-------------------------------------------------------------------

ShadowVolume::LocalShaderPrimitiveRenderCapsOneSided::LocalShaderPrimitiveRenderCapsOneSided (const Shader& shader, ShadowVolume& shadowVolume, const GlCullMode cullMode, const ShadowVolume::CapMode capMode) :
	ShaderPrimitive (),
	m_shader (shader),
	m_shadowVolume (shadowVolume),
	m_cullMode (cullMode),
	m_capMode (capMode)
{
}

//-------------------------------------------------------------------

ShadowVolume::LocalShaderPrimitiveRenderCapsOneSided::~LocalShaderPrimitiveRenderCapsOneSided ()
{
}

//-------------------------------------------------------------------

const Vector ShadowVolume::LocalShaderPrimitiveRenderCapsOneSided::getPosition_w() const
{
	return Vector::zero;
}

//-------------------------------------------------------------------

float ShadowVolume::LocalShaderPrimitiveRenderCapsOneSided::getDepthSquaredSortKey() const
{
	return 0.f;
}

//-------------------------------------------------------------------

int ShadowVolume::LocalShaderPrimitiveRenderCapsOneSided::getVertexBufferSortKey() const
{
	return 0;
}

//-------------------------------------------------------------------

const StaticShader& ShadowVolume::LocalShaderPrimitiveRenderCapsOneSided::prepareToView () const
{
	return m_shader.prepareToView ();
}

//-------------------------------------------------------------------

void ShadowVolume::LocalShaderPrimitiveRenderCapsOneSided::prepareToDraw () const
{
}

//-------------------------------------------------------------------

void ShadowVolume::LocalShaderPrimitiveRenderCapsOneSided::draw () const
{
	Graphics::setCullMode (m_cullMode);
	m_shadowVolume.renderShadowVolumeCaps (m_capMode);
}

//===================================================================
// ShadowVolume::LocalShaderPrimitiveRenderEdgesTwoSided
//===================================================================

class ShadowVolume::LocalShaderPrimitiveRenderEdgesTwoSided : public ShaderPrimitive
{
public:

	LocalShaderPrimitiveRenderEdgesTwoSided (const Shader& shader, const ShadowVolume& shadowVolume);
	virtual ~LocalShaderPrimitiveRenderEdgesTwoSided ();

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

private:

	LocalShaderPrimitiveRenderEdgesTwoSided ();
	LocalShaderPrimitiveRenderEdgesTwoSided (const LocalShaderPrimitiveRenderEdgesTwoSided& rhs);             //lint -esym(754, LocalShaderPrimitiveRenderEdgesTwoSided::LocalShaderPrimitiveRenderEdgesTwoSided)
	LocalShaderPrimitiveRenderEdgesTwoSided& operator= (const LocalShaderPrimitiveRenderEdgesTwoSided& rhs);  //lint -esym(754, LocalShaderPrimitiveRenderEdgesTwoSided::operator=)

private:

	const Shader&               m_shader;
	const ShadowVolume&         m_shadowVolume;
};

//-------------------------------------------------------------------
// PUBLIC ShadowVolume::LocalShaderPrimitiveRenderEdgesTwoSided
//-------------------------------------------------------------------

ShadowVolume::LocalShaderPrimitiveRenderEdgesTwoSided::LocalShaderPrimitiveRenderEdgesTwoSided (const Shader& shader, const ShadowVolume& shadowVolume) :
	ShaderPrimitive (),
	m_shader (shader),
	m_shadowVolume (shadowVolume)
{
}

//-------------------------------------------------------------------

ShadowVolume::LocalShaderPrimitiveRenderEdgesTwoSided::~LocalShaderPrimitiveRenderEdgesTwoSided ()
{
}

//-------------------------------------------------------------------

const Vector ShadowVolume::LocalShaderPrimitiveRenderEdgesTwoSided::getPosition_w() const
{
	return Vector::zero;
}

//-------------------------------------------------------------------

float ShadowVolume::LocalShaderPrimitiveRenderEdgesTwoSided::getDepthSquaredSortKey () const
{
	return 0.f;
}

// ----------------------------------------------------------------------

int ShadowVolume::LocalShaderPrimitiveRenderEdgesTwoSided::getVertexBufferSortKey () const
{
	return 0;
}

//-------------------------------------------------------------------

const StaticShader& ShadowVolume::LocalShaderPrimitiveRenderEdgesTwoSided::prepareToView () const
{
	return m_shader.prepareToView ();
}

//-------------------------------------------------------------------

void ShadowVolume::LocalShaderPrimitiveRenderEdgesTwoSided::prepareToDraw () const
{
}

//-------------------------------------------------------------------

void ShadowVolume::LocalShaderPrimitiveRenderEdgesTwoSided::draw () const
{
	GlCullMode const cullMode = Graphics::getCullMode ();
	Graphics::setCullMode (GCM_none);
	m_shadowVolume.renderShadowVolumeEdges ();
	Graphics::setCullMode (cullMode);
}

//===================================================================
// ShadowVolume::LocalShaderPrimitiveRenderCapsTwoSided
//===================================================================

class ShadowVolume::LocalShaderPrimitiveRenderCapsTwoSided : public ShaderPrimitive
{
public:

	LocalShaderPrimitiveRenderCapsTwoSided (const Shader& shader, ShadowVolume& shadowVolume, const ShadowVolume::CapMode capMode);
	virtual ~LocalShaderPrimitiveRenderCapsTwoSided ();

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

private:

	LocalShaderPrimitiveRenderCapsTwoSided ();
	LocalShaderPrimitiveRenderCapsTwoSided (const LocalShaderPrimitiveRenderCapsTwoSided& rhs);             //lint -esym(754, LocalShaderPrimitiveRenderCapsTwoSided::LocalShaderPrimitiveRenderCapsTwoSided)
	LocalShaderPrimitiveRenderCapsTwoSided& operator= (const LocalShaderPrimitiveRenderCapsTwoSided& rhs);  //lint -esym(754, LocalShaderPrimitiveRenderCapsTwoSided::operator=)

private:

	const Shader&               m_shader;
	ShadowVolume&               m_shadowVolume;
	const ShadowVolume::CapMode m_capMode;
};

//-------------------------------------------------------------------
// PUBLIC ShadowVolume::LocalShaderPrimitiveRenderCapsTwoSided
//-------------------------------------------------------------------

ShadowVolume::LocalShaderPrimitiveRenderCapsTwoSided::LocalShaderPrimitiveRenderCapsTwoSided (const Shader& shader, ShadowVolume& shadowVolume, const ShadowVolume::CapMode capMode) :
	ShaderPrimitive (),
	m_shader (shader),
	m_shadowVolume (shadowVolume),
	m_capMode (capMode)
{
}

//-------------------------------------------------------------------

ShadowVolume::LocalShaderPrimitiveRenderCapsTwoSided::~LocalShaderPrimitiveRenderCapsTwoSided ()
{
}

//-------------------------------------------------------------------

const Vector ShadowVolume::LocalShaderPrimitiveRenderCapsTwoSided::getPosition_w() const
{
	return Vector::zero;
}

//-------------------------------------------------------------------

float ShadowVolume::LocalShaderPrimitiveRenderCapsTwoSided::getDepthSquaredSortKey() const
{
	return 0.f;
}

//-------------------------------------------------------------------

int ShadowVolume::LocalShaderPrimitiveRenderCapsTwoSided::getVertexBufferSortKey() const
{
	return 0;
}

//-------------------------------------------------------------------

const StaticShader& ShadowVolume::LocalShaderPrimitiveRenderCapsTwoSided::prepareToView () const
{
	return m_shader.prepareToView ();
}

//-------------------------------------------------------------------

void ShadowVolume::LocalShaderPrimitiveRenderCapsTwoSided::prepareToDraw () const
{
}

//-------------------------------------------------------------------

void ShadowVolume::LocalShaderPrimitiveRenderCapsTwoSided::draw () const
{
	GlCullMode const cullMode = Graphics::getCullMode ();
	Graphics::setCullMode (GCM_none);
	m_shadowVolume.renderShadowVolumeCaps (m_capMode);
	Graphics::setCullMode (cullMode);
}

//===================================================================
// STATIC PUBLIC ShadowVolume
//===================================================================

bool          ShadowVolume::ms_debugReport;
bool          ShadowVolume::ms_enabled;
bool          ShadowVolume::ms_showExtrudedEdges;
bool          ShadowVolume::ms_showNonManifoldEdges;
Vector        ShadowVolume::ms_directionToLight;
VectorArgb    ShadowVolume::ms_shadowColor;
bool          ShadowVolume::ms_supportsTwoSidedStencil;
const Shader* ShadowVolume::ms_shadowVolumeOneSidedIncrementShader;
const Shader* ShadowVolume::ms_shadowVolumeOneSidedDecrementShader;
const Shader* ShadowVolume::ms_shadowVolumeTwoSidedShader;
const Shader* ShadowVolume::ms_shadowVolumeScreenAlphaShader;
bool          ShadowVolume::ms_renderShadowsThisFrame  = false;
ShadowVolume::ProxyLocalShaderPrimitiveList* ShadowVolume::ms_proxyLocalShaderPrimitiveList;

//-------------------------------------------------------------------

void ShadowVolume::install ()
{
	ProxyLocalShaderPrimitive::install ();

	ms_permanentlyDisableShadowVolumes = MemoryManager::getLimit () < 375;

	DebugFlags::registerFlag (ms_permanentlyDisableShadowVolumes, "ClientObject", "permanentlyDisableShadowVolumes");
	REPORT_LOG (ms_permanentlyDisableShadowVolumes, ("ShadowVolume: permanently disabled\n"));

	//-- we're going to give shadow blobs a chance
	DebugFlags::registerFlag (ms_debugReport, "ClientObject", "reportShadowVolume", debugDump);

	ms_supportsTwoSidedStencil = Graphics::supportsTwoSidedStencil ();
	//DEBUG_REPORT_LOG (true, ("ShadowVolume: using %i-sided stencil\n", ms_supportsTwoSidedStencil ? 2 : 1));
	if (ms_supportsTwoSidedStencil)
	{
		ms_shadowVolumeTwoSidedShader = ShaderTemplateList::fetchShader ("shader/shadowvolume_twosided.sht");
	}
	else
	{
		ms_shadowVolumeOneSidedIncrementShader = ShaderTemplateList::fetchShader ("shader/shadowvolume_onesided_increment.sht");
		ms_shadowVolumeOneSidedDecrementShader = ShaderTemplateList::fetchShader ("shader/shadowvolume_onesided_decrement.sht");
	}

	ms_shadowVolumeScreenAlphaShader = ShaderTemplateList::fetchShader ("shader/shadowvolume_screenalpha.sht");

	ms_proxyLocalShaderPrimitiveList = new ProxyLocalShaderPrimitiveList;

	ms_interiorShadowColor.set (ConfigClientObject::getInteriorShadowAlpha (), 0.f, 0.f, 0.f);

	VertexBufferFormat format;
	format.setPosition ();

	{
		DynamicVertexBuffer vertexBuffer (format);

		const int numberOfLockableDynamicVertices = vertexBuffer.getNumberOfLockableDynamicVertices (true);
		ms_maximumVertexBufferSize = numberOfLockableDynamicVertices - (numberOfLockableDynamicVertices % 4);

		//DEBUG_REPORT_LOG (true, ("ShadowVolume: numberOfLockableDynamicVertices = %i\n", numberOfLockableDynamicVertices));
		//DEBUG_REPORT_LOG (true, ("ShadowVolume: maximumShadowVertexBufferSize = %i\n", ms_maximumVertexBufferSize));
	}

	{
		ShadowEdgePrimitive shadowEdgePrimitive;
		shadowEdgePrimitive.shadowEdgeVertexBuffer = new SystemVertexBuffer (format, ms_maximumVertexBufferSize);
		shadowEdgePrimitive.shadowEdgeIndexBuffer = new SystemIndexBuffer ((ms_maximumVertexBufferSize / 4) * 6);
		ms_shadowEdgePrimitiveList.push_back (shadowEdgePrimitive);
	}

	ms_crashReportInfo[0] = '\0';
	CrashReportInformation::addDynamicText (ms_crashReportInfo);

	ExitChain::add (remove, "ShadowVolume");
}

//-------------------------------------------------------------------

void ShadowVolume::remove ()
{
	CrashReportInformation::removeDynamicText (ms_crashReportInfo);

	for (uint i = 0; i < ms_shadowEdgePrimitiveList.size (); ++i)
	{
		ShadowEdgePrimitive& shadowEdgePrimitive = ms_shadowEdgePrimitiveList [i];

		delete shadowEdgePrimitive.shadowEdgeVertexBuffer;
		shadowEdgePrimitive.shadowEdgeVertexBuffer = 0;

		delete shadowEdgePrimitive.shadowEdgeIndexBuffer;
		shadowEdgePrimitive.shadowEdgeIndexBuffer = 0;
	}

	ms_shadowEdgePrimitiveList.clear ();

	if (ms_shadowVolumeOneSidedIncrementShader)
	{
		ms_shadowVolumeOneSidedIncrementShader->release();
		ms_shadowVolumeOneSidedIncrementShader = 0;
	}

	if (ms_shadowVolumeOneSidedDecrementShader)
	{
		ms_shadowVolumeOneSidedDecrementShader->release();
		ms_shadowVolumeOneSidedDecrementShader = 0;
	}

	if (ms_shadowVolumeTwoSidedShader)
	{
		ms_shadowVolumeTwoSidedShader->release();
		ms_shadowVolumeTwoSidedShader = 0;
	}

	if (ms_shadowVolumeScreenAlphaShader)
	{
		ms_shadowVolumeScreenAlphaShader->release();
		ms_shadowVolumeScreenAlphaShader = 0;
	}

	clearProxyLocalShaderPrimitiveList ();

	delete ms_proxyLocalShaderPrimitiveList;
	ms_proxyLocalShaderPrimitiveList = 0;

	ProxyLocalShaderPrimitive::remove ();

	DebugFlags::unregisterFlag (ms_debugReport);
}

//-------------------------------------------------------------------

bool ShadowVolume::getEnabled ()
{
	return ms_enabled && (ms_viewer || !ms_permanentlyDisableShadowVolumes);
}

//-------------------------------------------------------------------

void ShadowVolume::setEnabled (bool const enabled)
{
	ms_enabled = enabled;
}

//-------------------------------------------------------------------

void ShadowVolume::setShowExtrudedEdges (bool showExtrudedEdges)
{
	ms_showExtrudedEdges = showExtrudedEdges;
}

//-------------------------------------------------------------------

bool ShadowVolume::getShowExtrudedEdges ()
{
	return ms_showExtrudedEdges;
}

//-------------------------------------------------------------------

void ShadowVolume::setShowNonManifoldEdges (bool showNonManifoldEdges)
{
	ms_showNonManifoldEdges = showNonManifoldEdges;
}

//-------------------------------------------------------------------

bool ShadowVolume::getShowNonManifoldEdges ()
{
	return ms_showNonManifoldEdges;
}

//-------------------------------------------------------------------

void ShadowVolume::setDirectionToLight (const Vector& directionToLight, bool viewer)
{
	ms_viewer = viewer;
	ms_directionToLight = directionToLight;
}

//-------------------------------------------------------------------

const Vector& ShadowVolume::getDirectionToLight ()
{
	return ms_directionToLight;
}

//-------------------------------------------------------------------

void ShadowVolume::setShadowColor (const VectorArgb& shadowColor)
{
	ms_shadowColor = shadowColor;
	ms_renderShadowsThisFrame = shadowColor.a > 0.f;
}

//-------------------------------------------------------------------

void ShadowVolume::renderShadowAlpha (const Camera* camera)
{
	clearProxyLocalShaderPrimitiveList ();

	if (ms_enabled && ms_renderShadowsThisFrame && ms_shadowVolumeScreenAlphaShader)
	{
		//-- draw screen poly
		VertexBufferFormat format;
		format.setPosition();
		format.setTransformed();
		format.setColor0();
		DynamicVertexBuffer vertexBuffer (format);



		vertexBuffer.lock (4);

			VertexBufferWriteIterator v = vertexBuffer.begin();

			VectorArgb color = ms_shadowColor;

			if(ms_viewer || camera->getParentCell () == CellProperty::getWorldCellProperty ())
			{
				float timeOfDay = ShadowManager::getTimeOfDay();

				timeOfDay += 0.5f;

				if(timeOfDay > 1.0f)
					timeOfDay = 1.0f;

				float shadowValue = 0.70f;
				color = VectorArgb(shadowValue * timeOfDay, 0.0f, 0.0f, 0.0f);
			}
			else
			{
				color = ms_interiorShadowColor;
			}

			v.setPosition  (Vector (static_cast<float> (camera->getViewportX0 ()), static_cast<float> (camera->getViewportY0 ()), 1.f));
			v.setOoz  (1.f);
			v.setColor0 (color);
			++v;

			v.setPosition  (Vector (static_cast<real> (camera->getViewportX0 () + camera->getViewportWidth ()), static_cast<real> (camera->getViewportY0 ()), 1.f));
			v.setOoz  (1.f);
			v.setColor0 (color);
			++v;

			v.setPosition  (Vector (static_cast<real> (camera->getViewportX0 () + camera->getViewportWidth ()), static_cast<real> (camera->getViewportY0 () + camera->getViewportHeight ()), 1.f));
			v.setOoz  (1.f);
			v.setColor0 (color);
			++v;

			v.setPosition  (Vector (static_cast<real> (camera->getViewportX0 ()), static_cast<real> (camera->getViewportY0 () + camera->getViewportHeight ()), 1.f));
			v.setOoz  (1.f);
			v.setColor0 (color);

		vertexBuffer.unlock ();

		Graphics::setStaticShader (ms_shadowVolumeScreenAlphaShader->prepareToView());
		Graphics::setVertexBuffer (vertexBuffer);
		Graphics::drawTriangleFan ();
	}
}

//-------------------------------------------------------------------

void ShadowVolume::setAllowShadowSubmissions (bool const allowShadowSubmissions)
{
	ms_allowShadowSubmissions = allowShadowSubmissions;
}

//===================================================================
// STATIC PRIVATE ShadowVolume
//===================================================================

void ShadowVolume::clearProxyLocalShaderPrimitiveList ()
{
	NOT_NULL (ms_proxyLocalShaderPrimitiveList);
	std::for_each (ms_proxyLocalShaderPrimitiveList->begin (), ms_proxyLocalShaderPrimitiveList->end (), PointerDeleter ());
	ms_proxyLocalShaderPrimitiveList->clear ();

#ifdef _DEBUG
	ms_renderedShadowVolumeList.clear ();
#endif
}

//-------------------------------------------------------------------

void ShadowVolume::addEdge (ShadowVolume::Edge* const edgeList, int& numberOfEdges, const int v0, const int v1, const int face)
{
	NOT_NULL (edgeList);

	int i;
	for (i = 0; i < numberOfEdges; ++i)
	{
		if (edgeList [i].numberOfFaces >= 2)
			continue;

		if (edgeList [i].v0 == v0 && edgeList [i].v1 == v1)
			break;

		if (edgeList [i].v0 == v1 && edgeList [i].v1 == v0)
			break;
	}

	if (i == numberOfEdges)
	{
		edgeList [i].v0 = v0;
		edgeList [i].v1 = v1;
		edgeList [i].numberOfFaces = 0;
#ifdef _DEBUG
		edgeList [i].isNonManifold = false;
#endif

		++numberOfEdges;
	}

	if (edgeList [i].numberOfFaces < 2)
		edgeList [i].face [edgeList [i].numberOfFaces++] = face;
	else
	{
#ifdef _DEBUG
		//DEBUG_REPORT_LOG_PRINT (true, ("ShadowVolume::addEdge - more than 2 faces share this edge!\n"));
		edgeList [i].isNonManifold = true;
#endif
	}
}

//-------------------------------------------------------------------

int ShadowVolume::getMemorySize () const
{
	int memorySize = 0;

	memorySize += isizeof (ShadowVolume);
	memorySize += 2 * isizeof (LocalShaderPrimitiveRenderEdgesOneSided);
	memorySize += 4 * isizeof (LocalShaderPrimitiveRenderCapsOneSided);
	memorySize += 1 * isizeof (LocalShaderPrimitiveRenderEdgesTwoSided);
	memorySize += 2 * isizeof (LocalShaderPrimitiveRenderCapsTwoSided);

	uint i;
	for (i = 0; i < m_shadowPrimitiveList->size (); ++i)
		memorySize += (*m_shadowPrimitiveList) [i]->getMemorySize ();

	memorySize += isizeof (Metrics);

	return memorySize;
}

//-------------------------------------------------------------------

void ShadowVolume::debugDump ()
{
#ifdef _DEBUG
	int memorySize = 0;

	uint i;
	for (i = 0; i < ms_shadowVolumeList.size (); ++i)
		memorySize += ms_shadowVolumeList [i]->getMemorySize ();

	for (i = 0; i < ms_shadowEdgePrimitiveList.size (); ++i)
		memorySize += ms_shadowEdgePrimitiveList [i].getMemorySize ();

	DEBUG_REPORT_PRINT (true, ("-- ShadowVolume\n"));
	DEBUG_REPORT_PRINT (true, ("    permanently disabled = %s\n", ms_permanentlyDisableShadowVolumes ? "yes" : "no"));
	DEBUG_REPORT_PRINT (true, ("                 enabled = %s\n", getEnabled () ? "yes" : "no"));
	DEBUG_REPORT_PRINT (true, ("                   count = %i\n", ms_shadowVolumeList.size ()));
	DEBUG_REPORT_PRINT (true, ("              total size = %iM (%iK)\n", memorySize / (1024 * 1024), memorySize / 1024));
	DEBUG_REPORT_PRINT (true, ("edge primitive list size = %i\n", ms_shadowEdgePrimitiveList.size ()));
#endif
}

//===================================================================
// PUBLIC ShadowVolume
//===================================================================

ShadowVolume::ShadowVolume (const ShaderType shaderType, const PrimitiveType primitiveType, const char* debugName) :
	m_shaderType (shaderType),
	m_primitiveType (primitiveType),
	m_debugName (debugName),
	m_localShaderPrimitiveRenderEdgesOneSidedCullClockwise (0),
	m_localShaderPrimitiveRenderEdgesOneSidedCullCounterClockwise (0),
	m_localShaderPrimitiveRenderFrontCapsOneSidedCullClockwise (0),
	m_localShaderPrimitiveRenderFrontCapsOneSidedCullCounterClockwise (0),
	m_localShaderPrimitiveRenderBackCapsOneSidedCullClockwise (0),
	m_localShaderPrimitiveRenderBackCapsOneSidedCullCounterClockwise (0),
	m_localShaderPrimitiveRenderEdgesTwoSided (0),
	m_localShaderPrimitiveRenderFrontCapsTwoSided (0),
	m_localShaderPrimitiveRenderBackCapsTwoSided (0),
	m_shadowPrimitiveList (0),
	m_metrics ()
{
#ifdef _DEBUG
	{
		ms_shadowVolumeList.push_back (this);
	}
#endif

	m_shadowPrimitiveList = new ShadowPrimitiveList;

	if (ms_supportsTwoSidedStencil)
	{
		m_localShaderPrimitiveRenderEdgesTwoSided = new LocalShaderPrimitiveRenderEdgesTwoSided (*ms_shadowVolumeTwoSidedShader, *this);
		m_localShaderPrimitiveRenderFrontCapsTwoSided = new LocalShaderPrimitiveRenderCapsTwoSided (*ms_shadowVolumeTwoSidedShader, *this, ShadowVolume::CM_front);
#if SHADOW_EXTRUDE_TO_POINT == 0
		m_localShaderPrimitiveRenderBackCapsTwoSided = new LocalShaderPrimitiveRenderCapsTwoSided (*ms_shadowVolumeTwoSidedShader, *this, ShadowVolume::CM_back);
#endif
	}
	else
	{
		m_localShaderPrimitiveRenderEdgesOneSidedCullClockwise = new LocalShaderPrimitiveRenderEdgesOneSided (*ms_shadowVolumeOneSidedIncrementShader, *this, GCM_clockwise);
		m_localShaderPrimitiveRenderEdgesOneSidedCullCounterClockwise = new LocalShaderPrimitiveRenderEdgesOneSided (*ms_shadowVolumeOneSidedDecrementShader, *this, GCM_counterClockwise);
		m_localShaderPrimitiveRenderFrontCapsOneSidedCullClockwise = new LocalShaderPrimitiveRenderCapsOneSided (*ms_shadowVolumeOneSidedIncrementShader, *this, GCM_clockwise, ShadowVolume::CM_front);
		m_localShaderPrimitiveRenderFrontCapsOneSidedCullCounterClockwise = new LocalShaderPrimitiveRenderCapsOneSided (*ms_shadowVolumeOneSidedDecrementShader, *this, GCM_counterClockwise, ShadowVolume::CM_front);
#if SHADOW_EXTRUDE_TO_POINT == 0
		m_localShaderPrimitiveRenderBackCapsOneSidedCullClockwise = new LocalShaderPrimitiveRenderCapsOneSided (*ms_shadowVolumeOneSidedIncrementShader, *this, GCM_clockwise, ShadowVolume::CM_back);
		m_localShaderPrimitiveRenderBackCapsOneSidedCullCounterClockwise = new LocalShaderPrimitiveRenderCapsOneSided (*ms_shadowVolumeOneSidedDecrementShader, *this, GCM_counterClockwise, ShadowVolume::CM_back);
#endif
	}
}

//-------------------------------------------------------------------

ShadowVolume::~ShadowVolume ()
{
#ifdef _DEBUG

#if 0
	if (std::find (ms_renderedShadowVolumeList.begin (), ms_renderedShadowVolumeList.end (), this) != ms_renderedShadowVolumeList.end ())
		DEBUG_WARNING (true, ("deleting submitted shadow volume before render %i", ms_renderedShadowVolumeList.size ()));
#endif

	{
		ShadowVolumeList::iterator iter = std::find (ms_shadowVolumeList.begin (), ms_shadowVolumeList.end (), this);
		if (iter != ms_shadowVolumeList.end ())
			IGNORE_RETURN (ms_shadowVolumeList.erase (iter));
	}
#endif

	uint i;
	for (i = 0; i < m_shadowPrimitiveList->size (); ++i)
	{
		ShadowPrimitive& shadowPrimitive = *(*m_shadowPrimitiveList) [i];

		delete [] shadowPrimitive.compactVertexArray;
		shadowPrimitive.compactVertexArray = 0;

		shadowPrimitive.compactVertexCount = 0;

		delete [] shadowPrimitive.compactIndexArray;
		shadowPrimitive.compactIndexArray = 0;

		delete [] shadowPrimitive.compactIndexArrayToIndexArrayMap;
		shadowPrimitive.compactIndexArrayToIndexArrayMap = 0;

		shadowPrimitive.compactIndexCount = 0;

		delete [] shadowPrimitive.compactFaceNormalArray;
		shadowPrimitive.compactFaceNormalArray = 0;

		delete [] shadowPrimitive.compactFaceDotTestArray;
		shadowPrimitive.compactFaceDotTestArray = 0;

		shadowPrimitive.compactFaceCount = 0;

		delete [] shadowPrimitive.compactEdgeArray;
		shadowPrimitive.compactEdgeArray = 0;

		shadowPrimitive.compactEdgeCount = 0;

		delete shadowPrimitive.shadowVertexBuffer;
		shadowPrimitive.shadowVertexBuffer = 0;

		delete shadowPrimitive.shadowFrontIndexBuffer;
		shadowPrimitive.shadowFrontIndexBuffer = 0;

		delete shadowPrimitive.shadowBackIndexBuffer;
		shadowPrimitive.shadowBackIndexBuffer = 0;

		delete (*m_shadowPrimitiveList) [i];
		(*m_shadowPrimitiveList) [i] = 0;
	}

	m_shadowPrimitiveList->clear ();
	delete m_shadowPrimitiveList;
	m_shadowPrimitiveList = 0;

	delete m_localShaderPrimitiveRenderEdgesOneSidedCullClockwise;
	m_localShaderPrimitiveRenderEdgesOneSidedCullClockwise = 0;

	delete m_localShaderPrimitiveRenderEdgesOneSidedCullCounterClockwise;
	m_localShaderPrimitiveRenderEdgesOneSidedCullCounterClockwise = 0;

	delete m_localShaderPrimitiveRenderFrontCapsOneSidedCullClockwise;
	m_localShaderPrimitiveRenderFrontCapsOneSidedCullClockwise = 0;

	delete m_localShaderPrimitiveRenderFrontCapsOneSidedCullCounterClockwise;
	m_localShaderPrimitiveRenderFrontCapsOneSidedCullCounterClockwise = 0;

	delete m_localShaderPrimitiveRenderBackCapsOneSidedCullClockwise;
	m_localShaderPrimitiveRenderBackCapsOneSidedCullClockwise = 0;

	delete m_localShaderPrimitiveRenderBackCapsOneSidedCullCounterClockwise;
	m_localShaderPrimitiveRenderBackCapsOneSidedCullCounterClockwise = 0;

	delete m_localShaderPrimitiveRenderEdgesTwoSided;
	m_localShaderPrimitiveRenderEdgesTwoSided = 0;

	delete m_localShaderPrimitiveRenderFrontCapsTwoSided;
	m_localShaderPrimitiveRenderFrontCapsTwoSided = 0;

	delete m_localShaderPrimitiveRenderBackCapsTwoSided;
	m_localShaderPrimitiveRenderBackCapsTwoSided = 0;
}  //lint !e1740  //-- m_debugName not zeroed or freed

//-------------------------------------------------------------------

void ShadowVolume::addPrimitive (const VertexBufferReadIterator& vi, const int numberOfVertices, const Index* const indexArray, const int numberOfIndices)
{
	//-- if the ib or vb is empty, do nothing
	if (numberOfVertices == 0 && numberOfIndices == 0)
		return;

	NOT_NULL (indexArray);

	//-- for static shadow volumes, we can have more than one primitive. for animating shadow volumes, we can only have one primitive
	if (m_primitiveType == PT_static || (m_primitiveType == PT_animating && m_shadowPrimitiveList->empty ()))
		m_shadowPrimitiveList->push_back (new ShadowPrimitive ());

	NOT_NULL (m_shadowPrimitiveList);
	NOT_NULL (m_shadowPrimitiveList->back ());
	ShadowPrimitive& shadowPrimitive = *m_shadowPrimitiveList->back ();

	if (!shadowPrimitive.compactVertexArray)
	{
		shadowPrimitive.compactVertexArray = new Vector [static_cast<size_t> (numberOfVertices)];
		shadowPrimitive.compactVertexCount = 0;
	}

	if (!shadowPrimitive.compactIndexArray)
	{
		shadowPrimitive.compactIndexArray = new Index [static_cast<size_t> (numberOfIndices)];
		shadowPrimitive.compactIndexCount = 0;
	}

	if (!shadowPrimitive.compactEdgeArray)
	{
		shadowPrimitive.compactEdgeArray = new Edge [static_cast<size_t> (numberOfIndices)];
		shadowPrimitive.compactEdgeCount = 0;
	}

	if (m_primitiveType == PT_static || (m_primitiveType == PT_animating && !shadowPrimitive.compactIndexArrayToIndexArrayMap))
	{
		if (m_primitiveType == PT_static)
		{
			//-- produce compact vb
			const Index *ii = indexArray;

#if 1
			//-- create a compact vb/ib using as much sharing as possible
			int i;
			for (i = 0; i < numberOfIndices; ++i, ++ii)
				shadowPrimitive.compactIndexArray [shadowPrimitive.compactIndexCount++] = findOrAdd (shadowPrimitive.compactVertexArray, shadowPrimitive.compactVertexCount, (vi + *ii).getPosition ());
#else
			int i;

			//-- just copy the vb and ib
			shadowPrimitive.compactVertexCount = numberOfVertices;
			for (i = 0; i < shadowPrimitive.compactVertexCount; ++i)
				shadowPrimitive.compactVertexArray [i] = (vi + i).getPosition ();

			shadowPrimitive.compactIndexCount = numberOfIndices;
			for (i = 0; i < shadowPrimitive.compactIndexCount; ++i, ++ii)
				shadowPrimitive.compactIndexArray [i] = *ii;
#endif
		}
		else
		{
			//-- produce compact vb and mapping array
			shadowPrimitive.compactIndexArrayToIndexArrayMap = new Index [static_cast<size_t> (numberOfIndices)];

			const Index *ii = indexArray;

			int i;
			for (i = 0; i < numberOfIndices; ++i, ++ii)
			{
				const Index destinationIndex = findOrAdd (shadowPrimitive.compactVertexArray, shadowPrimitive.compactVertexCount, (vi + *ii).getPosition ());
				shadowPrimitive.compactIndexArray [shadowPrimitive.compactIndexCount++] = destinationIndex;
				shadowPrimitive.compactIndexArrayToIndexArrayMap [destinationIndex] = *ii;
			}
		}
	}

	if (shadowPrimitive.compactIndexCount * 6 > 65536)
	{
		DEBUG_WARNING (true, ("ShadowVolume::precompute (%s) - too many indices!", m_debugName ? m_debugName : "NULL"));

		//-- hack to not allow shadow volumes > 64K indices
		delete [] shadowPrimitive.compactVertexArray;
		shadowPrimitive.compactVertexArray = 0;
		shadowPrimitive.compactVertexCount = 0;

		delete [] shadowPrimitive.compactIndexArray;
		shadowPrimitive.compactIndexArray = 0;
		shadowPrimitive.compactIndexCount = 0;

		delete [] shadowPrimitive.compactEdgeArray;
		shadowPrimitive.compactEdgeArray = 0;
		shadowPrimitive.compactEdgeCount = 0;

		delete [] shadowPrimitive.compactIndexArrayToIndexArrayMap;
		shadowPrimitive.compactIndexArrayToIndexArrayMap = 0;
	}
	else
	{
		//-- pre-compute object space face normals and construct edge list
		DEBUG_FATAL (shadowPrimitive.compactIndexCount % 3 != 0, ("compactIndexCount (%i) % 3 != 0", shadowPrimitive.compactIndexCount));

		shadowPrimitive.compactFaceCount = shadowPrimitive.compactIndexCount / 3;

		if (!shadowPrimitive.compactFaceNormalArray)
			shadowPrimitive.compactFaceNormalArray  = new Vector [static_cast<size_t> (shadowPrimitive.compactFaceCount)];

		if (!shadowPrimitive.compactFaceDotTestArray)
			shadowPrimitive.compactFaceDotTestArray = new bool [static_cast<size_t> (shadowPrimitive.compactFaceCount)];

		//-- update the vertexarray if animating
		if (m_primitiveType == PT_animating)
		{
			int i;
			for (i = 0; i < shadowPrimitive.compactVertexCount; ++i)
				shadowPrimitive.compactVertexArray [i] = (vi + shadowPrimitive.compactIndexArrayToIndexArrayMap [i]).getPosition ();
		}

		//-- update the shadow vertex buffer (once if static, always if animating)
		if ((m_primitiveType == PT_static && !shadowPrimitive.shadowVertexBuffer) || (m_primitiveType == PT_animating))
		{
			//-- copy vertex buffer into shadow vertex buffer
			if (!shadowPrimitive.shadowVertexBuffer)
			{
				VertexBufferFormat format;
				format.setPosition ();

				shadowPrimitive.shadowVertexBuffer = new SystemVertexBuffer (format, shadowPrimitive.compactVertexCount);
			}

			//-- copy compact vertex array
			VertexBufferWriteIterator sv = shadowPrimitive.shadowVertexBuffer->begin();
			int i;
			for (i = 0; i < shadowPrimitive.compactVertexCount; ++i)
			{
				sv.setPosition (shadowPrimitive.compactVertexArray [i]);
				++sv;
			}
		}

		if (!shadowPrimitive.shadowFrontIndexBuffer)
			shadowPrimitive.shadowFrontIndexBuffer = new SystemIndexBuffer (shadowPrimitive.compactIndexCount);

		if (!shadowPrimitive.shadowBackIndexBuffer)
			shadowPrimitive.shadowBackIndexBuffer = new SystemIndexBuffer (shadowPrimitive.compactIndexCount);

		//-- compute edge connectivity
		if (!shadowPrimitive.computedEdgeConnectivity)
		{
			int i;
			for (i = 0; i < shadowPrimitive.compactFaceCount; ++i)
			{
				//-- inverting the winding order will show streamers; may be useful to artists!
				const int i0 = shadowPrimitive.compactIndexArray [3 * i + 0];
				const int i1 = shadowPrimitive.compactIndexArray [3 * i + 1];
				const int i2 = shadowPrimitive.compactIndexArray [3 * i + 2];

#if 0
				DEBUG_WARNING (i0 == i1 || i1 == i2 || i2 == i0, ("zero-area triangle detected"));

				//-- do we have duplicate geometry?
				int j;
				for (j = 0; j < i; ++j)
				{
					const int j0 = shadowPrimitive.compactIndexArray [3 * j + 0];
					const int j1 = shadowPrimitive.compactIndexArray [3 * j + 1];
					const int j2 = shadowPrimitive.compactIndexArray [3 * j + 2];

					if ((j0 == i0 && j1 == i1 && j2 == i2) ||
						(j0 == i1 && j1 == i2 && j2 == i0) ||
						(j0 == i2 && j1 == i0 && j2 == i1))
						DEBUG_WARNING (true, ("duplicate geometry detected"));
				}
#endif

				//-- compute edge array (only needs to be done once)
				addEdge (shadowPrimitive.compactEdgeArray, shadowPrimitive.compactEdgeCount, i0, i1, i);
				addEdge (shadowPrimitive.compactEdgeArray, shadowPrimitive.compactEdgeCount, i1, i2, i);
				addEdge (shadowPrimitive.compactEdgeArray, shadowPrimitive.compactEdgeCount, i2, i0, i);
			}

			shadowPrimitive.computedEdgeConnectivity = true;
		}

		//-- compute face normals
		{
			int i;
			for (i = 0; i < shadowPrimitive.compactFaceCount; ++i)
			{
				//-- inverting the winding order will show streamers; may be useful to artists!
				const int i0 = shadowPrimitive.compactIndexArray [3 * i + 0];
				const int i1 = shadowPrimitive.compactIndexArray [3 * i + 1];
				const int i2 = shadowPrimitive.compactIndexArray [3 * i + 2];

				const Vector& v0 = shadowPrimitive.compactVertexArray [i0];
				const Vector& v1 = shadowPrimitive.compactVertexArray [i1];
				const Vector& v2 = shadowPrimitive.compactVertexArray [i2];

				//-- compute normal (no need to normalize because we're only using it for backface culling)
				shadowPrimitive.compactFaceNormalArray [i] = (v0 - v2).cross (v1 - v0);
			}
		}
	}
}

//-------------------------------------------------------------------

void ShadowVolume::addPrimitive (const SystemVertexBuffer* const vertexBuffer, const StaticIndexBuffer* const indexBuffer)
{
	indexBuffer->lockReadOnly ();

		const VertexBufferReadIterator vertexBufferReadIterator = vertexBuffer->beginReadOnly ();
		const int numberOfVertices = vertexBuffer->getNumberOfVertices ();
		const Index* const indexArray = indexBuffer->beginReadOnly ();
		const int numberOfIndices = indexBuffer->getNumberOfIndices ();
		addPrimitive (vertexBufferReadIterator, numberOfVertices, indexArray, numberOfIndices);

	indexBuffer->unlock ();
}

//-------------------------------------------------------------------

void ShadowVolume::addPrimitive (const StaticVertexBuffer* const vertexBuffer, const StaticIndexBuffer* const indexBuffer)
{
	vertexBuffer->lockReadOnly ();
	indexBuffer->lockReadOnly ();

		const VertexBufferReadIterator vertexBufferReadIterator = vertexBuffer->beginReadOnly ();
		const int numberOfVertices = vertexBuffer->getNumberOfVertices ();
		const Index* const indexArray = indexBuffer->beginReadOnly ();
		const int numberOfIndices = indexBuffer->getNumberOfIndices ();
		addPrimitive (vertexBufferReadIterator, numberOfVertices, indexArray, numberOfIndices);

	indexBuffer->unlock ();
	vertexBuffer->unlock ();
}

//-------------------------------------------------------------------

void ShadowVolume::render(Object const * const object, const Appearance *const appearance) const
{
	NOT_NULL (object);
	NOT_NULL (appearance);

	//-- are shadows on?
	if (  !ms_enabled 
		|| !ms_renderShadowsThisFrame 
		||  m_shadowPrimitiveList->empty () 
		|| (  !ms_viewer 
		   && !ms_allowShadowSubmissions
			)
		)
		return;

	//-- see if anything related to us is in the world
	bool isInWorldCell = object->isInWorldCell ();
	if (object->isInWorld ())
		// object is in the world
		;
	else
	{
		//-- object is not in the world
		if (object->getRootParent ())
		{
			//-- object has a parent
			if (object->getRootParent ()->isInWorld ())
			{
				//-- object has a parent in the world
				isInWorldCell = object->getRootParent ()->isInWorldCell ();
			}
			else
			{
				//-- object does not have a parent (but may be contained)
				const ContainedByProperty* containedByProperty = object->getContainedByProperty ();

				if (containedByProperty)
				{
					const Object* const containedByObject = containedByProperty->getContainedBy ();

					if (containedByObject && containedByObject->isInWorld ())
					{
						isInWorldCell = containedByObject->isInWorldCell ();
					}
				}
				else
				{
					//-- object's parent is not in the world and is not contained by anything in the world

					//-- this changed because of component appearances
					isInWorldCell = true;
//					return;
				}
			}
		}
		else
		{
			//-- object does not have a parent
			return;
		}
	}
	
	//-- is the object too small?
	Camera const & camera = ShaderPrimitiveSorter::getCurrentCamera ();
	if (!ShadowManager::volumetricShouldRender (camera, appearance->getTransform_w().getPosition_p(), appearance->getSphere().getRadius()))
		return;

	//-- add the shadow
	NOT_NULL (ms_proxyLocalShaderPrimitiveList);

#ifdef _DEBUG
	ms_renderedShadowVolumeList.push_back (const_cast<ShadowVolume *> (this));
#endif

	if (ms_supportsTwoSidedStencil)
	{
		ProxyLocalShaderPrimitive* proxyLocalShaderPrimitive = NON_NULL (new ProxyLocalShaderPrimitive (*ms_shadowVolumeTwoSidedShader, *this, *object, *appearance, isInWorldCell, ProxyLocalShaderPrimitive::M_extrude));
		ms_proxyLocalShaderPrimitiveList->push_back (proxyLocalShaderPrimitive);
		ShaderPrimitiveSorter::add (*proxyLocalShaderPrimitive);

		NOT_NULL (m_localShaderPrimitiveRenderEdgesTwoSided);
		ShaderPrimitiveSorter::add (*m_localShaderPrimitiveRenderEdgesTwoSided);

		NOT_NULL (m_localShaderPrimitiveRenderFrontCapsTwoSided);
		ShaderPrimitiveSorter::add (*m_localShaderPrimitiveRenderFrontCapsTwoSided);

#if SHADOW_EXTRUDE_TO_POINT == 0
		if (m_localShaderPrimitiveRenderBackCapsTwoSided)
		{
			proxyLocalShaderPrimitive = NON_NULL (new ProxyLocalShaderPrimitive (*ms_shadowVolumeTwoSidedShader, *this, *object, *appearance, isInWorldCell, ProxyLocalShaderPrimitive::M_prepareFarCap));
			ms_proxyLocalShaderPrimitiveList->push_back (proxyLocalShaderPrimitive);
			ShaderPrimitiveSorter::add (*proxyLocalShaderPrimitive);

			NOT_NULL (m_localShaderPrimitiveRenderBackCapsTwoSided);
			ShaderPrimitiveSorter::add (*m_localShaderPrimitiveRenderBackCapsTwoSided);
		}
#endif
	}
	else
	{
		ProxyLocalShaderPrimitive* proxyLocalShaderPrimitive = NON_NULL (new ProxyLocalShaderPrimitive (*ms_shadowVolumeOneSidedIncrementShader, *this, *object, *appearance, isInWorldCell, ProxyLocalShaderPrimitive::M_extrude));
		ms_proxyLocalShaderPrimitiveList->push_back (proxyLocalShaderPrimitive);
		ShaderPrimitiveSorter::add (*proxyLocalShaderPrimitive);

		NOT_NULL (m_localShaderPrimitiveRenderEdgesOneSidedCullClockwise);
		ShaderPrimitiveSorter::add (*m_localShaderPrimitiveRenderEdgesOneSidedCullClockwise);

		NOT_NULL (m_localShaderPrimitiveRenderEdgesOneSidedCullCounterClockwise);
		ShaderPrimitiveSorter::add (*m_localShaderPrimitiveRenderEdgesOneSidedCullCounterClockwise);

		NOT_NULL (m_localShaderPrimitiveRenderFrontCapsOneSidedCullClockwise);
		ShaderPrimitiveSorter::add (*m_localShaderPrimitiveRenderFrontCapsOneSidedCullClockwise);

		NOT_NULL (m_localShaderPrimitiveRenderFrontCapsOneSidedCullCounterClockwise);
		ShaderPrimitiveSorter::add (*m_localShaderPrimitiveRenderFrontCapsOneSidedCullCounterClockwise);

#if SHADOW_EXTRUDE_TO_POINT == 0
		if (m_localShaderPrimitiveRenderBackCapsOneSidedCullClockwise)
		{
			proxyLocalShaderPrimitive = NON_NULL (new ProxyLocalShaderPrimitive (*ms_shadowVolumeOneSidedIncrementShader, *this, *object, *appearance, isInWorldCell, ProxyLocalShaderPrimitive::M_prepareFarCap));
			ms_proxyLocalShaderPrimitiveList->push_back (proxyLocalShaderPrimitive);
			ShaderPrimitiveSorter::add (*proxyLocalShaderPrimitive);

			NOT_NULL (m_localShaderPrimitiveRenderBackCapsOneSidedCullClockwise);
			ShaderPrimitiveSorter::add (*m_localShaderPrimitiveRenderBackCapsOneSidedCullClockwise);

			NOT_NULL (m_localShaderPrimitiveRenderBackCapsOneSidedCullCounterClockwise);
			ShaderPrimitiveSorter::add (*m_localShaderPrimitiveRenderBackCapsOneSidedCullCounterClockwise);
		}
#endif
	}
}

//===================================================================
// PRIVATE ShadowVolume
//===================================================================

void ShadowVolume::extrudeShadowVolume (const Vector& directionToLight_o) const
{
	PROFILER_AUTO_BLOCK_DEFINE("ShadowVolume::extrudeShadowVolume");

	//-- setup CrashReportInformation string.
	IGNORE_RETURN (snprintf (ms_crashReportInfo, sizeof (ms_crashReportInfo) - 1, "ShadowVolume: %s\n", m_debugName));
	ms_crashReportInfo[sizeof (ms_crashReportInfo) - 1] = '\0';

	ms_numberOfShadowEdgePrimitives = 0;

	uint i;
	for (i = 0; i < m_shadowPrimitiveList->size (); ++i)
	{
		ShadowPrimitive& shadowPrimitive = *(*m_shadowPrimitiveList) [i];
		if (shadowPrimitive.isEmpty ())
			continue;

		//-- update the face normal array
		{
			int j;
			for (j = 0; j < shadowPrimitive.compactFaceCount; ++j)
				shadowPrimitive.compactFaceDotTestArray [j] = shadowPrimitive.compactFaceNormalArray [j].dot (directionToLight_o) >= 0.f;
		}

		//-- update the caps
		{
			Index* sfi = shadowPrimitive.shadowFrontIndexBuffer->begin ();
			Index* sbi = shadowPrimitive.shadowBackIndexBuffer->begin ();
			shadowPrimitive.shadowFrontIndexCount = 0;
			shadowPrimitive.shadowBackIndexCount = 0;

			int j;
			for (j = 0; j < shadowPrimitive.compactFaceCount; ++j)
			{
				if (shadowPrimitive.compactFaceDotTestArray [j])
				{
					*sfi++ = shadowPrimitive.compactIndexArray [j * 3 + 0];
					*sfi++ = shadowPrimitive.compactIndexArray [j * 3 + 1];
					*sfi++ = shadowPrimitive.compactIndexArray [j * 3 + 2];

					shadowPrimitive.shadowFrontIndexCount += 3;
				}
				else
				{
					*sbi++ = shadowPrimitive.compactIndexArray [j * 3 + 0];
					*sbi++ = shadowPrimitive.compactIndexArray [j * 3 + 1];
					*sbi++ = shadowPrimitive.compactIndexArray [j * 3 + 2];

					shadowPrimitive.shadowBackIndexCount += 3;
				}
			}
		}

		{
			if (ms_numberOfShadowEdgePrimitives >= ms_shadowEdgePrimitiveList.size ())
			{
				VertexBufferFormat format;
				format.setPosition ();

				ShadowEdgePrimitive newShadowEdgePrimitive;
				newShadowEdgePrimitive.shadowEdgeVertexBuffer = new SystemVertexBuffer (format, ms_maximumVertexBufferSize);
				newShadowEdgePrimitive.shadowEdgeIndexBuffer = new SystemIndexBuffer ((ms_maximumVertexBufferSize / 4) * 6);
				ms_shadowEdgePrimitiveList.push_back (newShadowEdgePrimitive);
			}

			ShadowEdgePrimitive & shadowEdgePrimitive = ms_shadowEdgePrimitiveList [ms_numberOfShadowEdgePrimitives++];

			VertexBufferWriteIterator sv = shadowEdgePrimitive.shadowEdgeVertexBuffer->begin ();
			shadowEdgePrimitive.shadowEdgeVertexCount = 0;

			Index* si = shadowEdgePrimitive.shadowEdgeIndexBuffer->begin ();
			shadowEdgePrimitive.shadowEdgeIndexCount = 0;

			Vector v0;
			Vector v1;
			Vector v2;
			Vector v3;

			const Vector infinity = directionToLight_o * ms_shadowVolumeExtrudeDistance;

			int numberOfShadowVolumeQuads = 0;

			int j;
			for (j = 0; j < shadowPrimitive.compactEdgeCount; ++j)
			{
				const Edge& edge = shadowPrimitive.compactEdgeArray [j];

#ifdef _DEBUG
				if (!ms_showExtrudedEdges && ms_showNonManifoldEdges && edge.isNonManifold)
				{
					v0 = shadowPrimitive.compactVertexArray [edge.v0];
					v1 = shadowPrimitive.compactVertexArray [edge.v1];
					ShaderPrimitiveSorter::getCurrentCamera ().addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, v0, v1, PackedArgb::solidWhite));
				}

//				if (edge.isNonManifold)
//					continue;
#endif

				if ((edge.numberOfFaces == 1 && shadowPrimitive.compactFaceDotTestArray [edge.face [0]]) ||
					(edge.numberOfFaces == 2 && shadowPrimitive.compactFaceDotTestArray [edge.face [0]] != shadowPrimitive.compactFaceDotTestArray [edge.face [1]]))
				{
					//-- check to see if the vertices are in the same order
					bool invert = false;

					if (edge.numberOfFaces == 2)
					{
						//-- which face was culled?
						const int unculledFace = shadowPrimitive.compactFaceDotTestArray [edge.face [0]] ? edge.face [0] : edge.face [1];

						int k;
						for (k = 0; k < 3; ++k)
						{
							int start    = k;
							int backward = (k + 2) % 3;

							//-- find v0
							if (shadowPrimitive.compactIndexArray [unculledFace * 3 + start] == edge.v0)
							{
								//-- check order to see if we need to invert the bindings
								if (shadowPrimitive.compactIndexArray [unculledFace * 3 + backward] == edge.v1)
									invert = true;

								break;
							}
						}
					}

					//-- start with the edge
					v0 = shadowPrimitive.compactVertexArray [edge.v0];
					v1 = shadowPrimitive.compactVertexArray [edge.v1];


					//-- instead of extending to infinity, should we clip against frustum?
#if SHADOW_EXTRUDE_TO_POINT
					v2 = -infinity;
					v3 = -infinity;
#else
					v2 = v0 - infinity;
					v3 = v1 - infinity;
#endif

#ifdef _DEBUG
					if ((!ms_showNonManifoldEdges && ms_showExtrudedEdges) || (ms_showExtrudedEdges && ms_showNonManifoldEdges && edge.isNonManifold))
					{
						ShaderPrimitiveSorter::getCurrentCamera ().addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, v0, v1, PackedArgb::solidWhite));
//						ShaderPrimitiveSorter::getCurrentCamera ().addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, v1, v2, PackedArgb::solidWhite));
//						ShaderPrimitiveSorter::getCurrentCamera ().addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, v2, v3, PackedArgb::solidWhite));
//						ShaderPrimitiveSorter::getCurrentCamera ().addDebugPrimitive (new Line3dDebugPrimitive (Line3dDebugPrimitive::S_none, Transform::identity, v3, v0, PackedArgb::solidWhite));
					}
#endif

					//-- figure out which ShadowEdgePrimitive we're on
					if (shadowEdgePrimitive.shadowEdgeVertexCount >= ms_maximumVertexBufferSize)
					{
						if (ms_numberOfShadowEdgePrimitives >= ms_shadowEdgePrimitiveList.size ())
						{
							VertexBufferFormat format;
							format.setPosition ();

							ShadowEdgePrimitive newShadowEdgePrimitive;
							newShadowEdgePrimitive.shadowEdgeVertexBuffer = new SystemVertexBuffer (format, ms_maximumVertexBufferSize);
							newShadowEdgePrimitive.shadowEdgeIndexBuffer = new SystemIndexBuffer ((ms_maximumVertexBufferSize / 4) * 6);
							ms_shadowEdgePrimitiveList.push_back (newShadowEdgePrimitive);
						}

						shadowEdgePrimitive = ms_shadowEdgePrimitiveList [ms_numberOfShadowEdgePrimitives++];

						sv = shadowEdgePrimitive.shadowEdgeVertexBuffer->begin ();
						shadowEdgePrimitive.shadowEdgeVertexCount = 0;

						si = shadowEdgePrimitive.shadowEdgeIndexBuffer->begin ();
						shadowEdgePrimitive.shadowEdgeIndexCount = 0;
					}

					//-- add two triangles to the shadow vertex list
					sv.setPosition (v0);
					++sv;
					sv.setPosition (v1);
					++sv;
					sv.setPosition (v2);
					++sv;

#if SHADOW_EXTRUDE_TO_POINT == 0
					sv.setPosition (v3);
					++sv;
#endif

					if (invert)
					{
#if SHADOW_EXTRUDE_TO_POINT
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 3 + 0);
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 3 + 1);
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 3 + 2);
#else
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 4 + 0);
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 4 + 1);
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 4 + 2);
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 4 + 1);
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 4 + 3);
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 4 + 2);
#endif
					}
					else
					{
#if SHADOW_EXTRUDE_TO_POINT
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 3 + 0);
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 3 + 2);
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 3 + 1);
#else
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 4 + 0);
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 4 + 2);
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 4 + 1);
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 4 + 1);
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 4 + 2);
						(*si++) = static_cast<Index> (numberOfShadowVolumeQuads * 4 + 3);
#endif
					}

					++numberOfShadowVolumeQuads;

#if SHADOW_EXTRUDE_TO_POINT
					shadowEdgePrimitive.shadowEdgeVertexCount += 3;
					shadowEdgePrimitive.shadowEdgeIndexCount += 3;
#else
					shadowEdgePrimitive.shadowEdgeVertexCount += 4;
					shadowEdgePrimitive.shadowEdgeIndexCount += 6;
#endif
				}
			}
		}
	}
}

//-------------------------------------------------------------------

void ShadowVolume::renderShadowVolumeEdges () const
{
	PROFILER_AUTO_BLOCK_DEFINE("ShadowVolume::renderShadowVolumeEdges");

	for (uint i = 0; i < ms_numberOfShadowEdgePrimitives; ++i)
	{
		const ShadowEdgePrimitive&      shadowEdgePrimitive    = ms_shadowEdgePrimitiveList [i];
		const SystemVertexBuffer* const shadowEdgeVertexBuffer = shadowEdgePrimitive.shadowEdgeVertexBuffer;
		int                             shadowEdgeVertexCount  = shadowEdgePrimitive.shadowEdgeVertexCount;
		const SystemIndexBuffer* const  shadowEdgeIndexBuffer  = shadowEdgePrimitive.shadowEdgeIndexBuffer;
		int                             shadowEdgeIndexCount   = shadowEdgePrimitive.shadowEdgeIndexCount;

		if (shadowEdgeVertexCount)
		{
			DEBUG_FATAL (shadowEdgeIndexCount == 0, ("has shadow vertices but no shadow indices"));

			setVertexBuffer (shadowEdgeVertexBuffer, shadowEdgeVertexCount);
			setIndexBuffer (shadowEdgeIndexBuffer, shadowEdgeIndexCount);
			Graphics::drawIndexedTriangleList (0, 0, shadowEdgeVertexCount, 0, shadowEdgeIndexCount / 3);
		}
	}
}

//-------------------------------------------------------------------

void ShadowVolume::renderShadowVolumeCaps (const ShadowVolume::CapMode capMode) const
{
	PROFILER_AUTO_BLOCK_DEFINE("ShadowVolume::renderShadowVolumeCaps");

	switch (capMode)
	{
	case CM_front:
		{
			uint i;
			for (i = 0; i < m_shadowPrimitiveList->size (); ++i)
			{
				const ShadowPrimitive& shadowPrimitive = *(*m_shadowPrimitiveList) [i];

				if (shadowPrimitive.shadowFrontIndexCount)
				{
					setVertexBuffer (shadowPrimitive.shadowVertexBuffer, shadowPrimitive.shadowVertexBuffer->getNumberOfVertices ());
					setIndexBuffer (shadowPrimitive.shadowFrontIndexBuffer, shadowPrimitive.shadowFrontIndexBuffer->getNumberOfIndices ());
					Graphics::drawIndexedTriangleList (0, 0, shadowPrimitive.shadowVertexBuffer->getNumberOfVertices (), 0, shadowPrimitive.shadowFrontIndexCount / 3);
				}
			}
		}
		break;

	case CM_back:
		{
			uint i;
			for (i = 0; i < m_shadowPrimitiveList->size (); ++i)
			{
				const ShadowPrimitive& shadowPrimitive = *(*m_shadowPrimitiveList) [i];

				if (shadowPrimitive.shadowBackIndexCount)
				{
					setVertexBuffer (shadowPrimitive.shadowVertexBuffer, shadowPrimitive.shadowVertexBuffer->getNumberOfVertices ());
					setIndexBuffer (shadowPrimitive.shadowBackIndexBuffer, shadowPrimitive.shadowBackIndexBuffer->getNumberOfIndices ());
					Graphics::drawIndexedTriangleList (0, 0, shadowPrimitive.shadowVertexBuffer->getNumberOfVertices (), 0, shadowPrimitive.shadowBackIndexCount / 3);
				}
			}
		}
		break;
	}
}

//===================================================================

