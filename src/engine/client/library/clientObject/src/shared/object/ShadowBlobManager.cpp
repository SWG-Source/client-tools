// ======================================================================
//
// ShadowBlobManager.cpp
// asommers
// 
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ShadowBlobManager.h"

#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/VertexBufferFormat.h"
#include "clientGraphics/VertexBufferIterator.h"
#include "clientObject/ShadowBlobObject.h"
#include "clientObject/ShadowManager.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/ObjectList.h"

#include <algorithm>
#include <vector>

// ======================================================================

namespace ShadowBlobManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Vertex
	{
		Vector position;
		float  u;
		float  v;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool ms_installed;
	bool ms_enabled;
	bool ms_renderShadowBlobGeometry;
	ObjectList* ms_objectList;

	typedef std::vector<Vertex> VertexList;
	std::vector<VertexList*> ms_vertexListList;

	class ShadowBlobShaderPrimitive;
	ShadowBlobShaderPrimitive* ms_currentShadowBlobShaderPrimitive;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove ();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	VertexList* createVertexList ()
	{
		if (!ms_vertexListList.empty ())
		{
			VertexList* const vertexList = ms_vertexListList.back ();
			vertexList->clear ();

			ms_vertexListList.pop_back ();

			return vertexList;
		}

		return new VertexList ();
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void destroyVertexList (VertexList* const vertexList)
	{
		ms_vertexListList.push_back (vertexList);
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace ShadowBlobManagerNamespace;

// ======================================================================

class ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive : public ShaderPrimitive
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install ();

	static void update (float elapsedTime);

public:

	explicit ShadowBlobShaderPrimitive (const Transform& transform);
	virtual ~ShadowBlobShaderPrimitive ();

	int getNumberOfVertices () const;
	void addVertex (const Vector& position, float u, float v);

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey () const;
	virtual int                 getVertexBufferSortKey () const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

private:

	static void remove ();

private:

	ShadowBlobShaderPrimitive ();
	ShadowBlobShaderPrimitive (const ShadowBlobShaderPrimitive& rhs);             //lint -esym (754, ShadowBlobShaderPrimitive::ShadowBlobShaderPrimitive)
	ShadowBlobShaderPrimitive& operator= (const ShadowBlobShaderPrimitive& rhs);  //lint -esym (754, ShadowBlobShaderPrimitive::operator=)

private:

	static const Shader* ms_shadowBlobShader;

private:

	const Transform m_transform;
	VertexList* const m_vertexList;
};

//-------------------------------------------------------------------

const Shader* ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive::ms_shadowBlobShader;

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL (ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive, true, 0, 0, 0);

//-------------------------------------------------------------------

void ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive::install ()
{
	installMemoryBlockManager ();

	ms_shadowBlobShader = ShaderTemplateList::fetchShader ("shader/shadowblob.sht");

	ExitChain::add (ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive::remove, "ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive::remove");
}

//-------------------------------------------------------------------

void ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive::update (float elapsedTime)
{
	ms_shadowBlobShader->alter (elapsedTime);
}

//-------------------------------------------------------------------

void ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive::remove ()
{
	removeMemoryBlockManager ();

	ms_shadowBlobShader->release ();
	ms_shadowBlobShader = 0;
}

//-------------------------------------------------------------------

ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive::ShadowBlobShaderPrimitive (const Transform& transform) :
	ShaderPrimitive (),
	m_transform (transform),
	m_vertexList (createVertexList ())
{
}

//-------------------------------------------------------------------

ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive::~ShadowBlobShaderPrimitive ()
{
	destroyVertexList (m_vertexList);
}

//-------------------------------------------------------------------

int ShadowBlobShaderPrimitive::getNumberOfVertices () const
{
	return static_cast<int> (m_vertexList->size ());
}

//-------------------------------------------------------------------

void ShadowBlobShaderPrimitive::addVertex (const Vector& position, const float u, const float v)
{
	Vertex vertex;
	vertex.position = position;
	vertex.u = u;
	vertex.v = v;

	m_vertexList->push_back (vertex);
}

//-------------------------------------------------------------------

const Vector ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive::getPosition_w () const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive::getDepthSquaredSortKey () const
{
	return 0.f;
}

//-------------------------------------------------------------------

int ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive::getVertexBufferSortKey () const
{
	return 0;
}

//-------------------------------------------------------------------

const StaticShader& ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive::prepareToView () const
{
	if (ms_renderShadowBlobGeometry)
		return ShaderTemplateList::get3dVertexColorStaticShader ();

	return ms_shadowBlobShader->prepareToView ();
}

//-------------------------------------------------------------------

void ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive::prepareToDraw () const
{
	Graphics::setObjectToWorldTransformAndScale (m_transform, Vector::xyz111);

	{
		VertexBufferFormat format;
		format.setPosition ();
		format.setColor0 ();
		format.setNumberOfTextureCoordinateSets (1);
		format.setTextureCoordinateSetDimension (0, 2);

		DynamicVertexBuffer vertexBuffer (format);

		vertexBuffer.lock (m_vertexList->size ());

			VertexBufferWriteIterator v = vertexBuffer.begin ();
			const uint n = m_vertexList->size ();
			uint i;
			for (i = 0; i < n; ++i, ++v)
			{
				const Vertex& vertex = (*m_vertexList) [i];

				v.setPosition (vertex.position);
				v.setColor0 (VectorArgb::solidWhite);
				v.setTextureCoordinates (0, vertex.u, vertex.v);
			}

		vertexBuffer.unlock ();

		Graphics::setVertexBuffer (vertexBuffer);
	}
}

//-------------------------------------------------------------------

void ShadowBlobManagerNamespace::ShadowBlobShaderPrimitive::draw () const
{
	const GlFillMode fillMode = Graphics::getFillMode ();

	if (ms_renderShadowBlobGeometry)
		Graphics::setFillMode (GFM_wire);

	Graphics::drawTriangleList ();

	if (ms_renderShadowBlobGeometry)
		Graphics::setFillMode (fillMode);

	//-- free this primitive for reuse
	delete const_cast<ShadowBlobShaderPrimitive*> (this);
}

// ======================================================================
// STATIC PUBLIC ShadowBlobManager
// ======================================================================

void ShadowBlobManager::install ()
{
	DEBUG_FATAL (ms_installed, ("ShadowBlobManager::install: installed"));
	ms_installed = true;

	ShadowBlobShaderPrimitive::install ();

	ms_objectList = new ObjectList;

	DebugFlags::registerFlag (ms_renderShadowBlobGeometry, "ClientObject", "renderShadowBlobGeometry");
	ExitChain::add (ShadowBlobManagerNamespace::remove, "ShadowBlobManagerNamespace::remove");
}

// ----------------------------------------------------------------------

bool ShadowBlobManager::getEnabled ()
{
	return ms_enabled;
}

// ----------------------------------------------------------------------

void ShadowBlobManager::setEnabled (bool const enabled)
{
	ms_enabled = enabled;
}

// ----------------------------------------------------------------------

void ShadowBlobManager::enableShadowBlobObject (const ShadowBlobObject& shadowBlobObject)
{
	DEBUG_FATAL (!ms_installed, ("ShadowBlobManager::install: not installed"));

	if (ms_enabled)
		ms_objectList->addObject (const_cast<ShadowBlobObject*> (&shadowBlobObject));
}

// ----------------------------------------------------------------------

void ShadowBlobManager::disableShadowBlobObject (const ShadowBlobObject& shadowBlobObject)
{
	if (ms_enabled)
		ms_objectList->removeObject (const_cast<ShadowBlobObject*> (&shadowBlobObject));
}

// ----------------------------------------------------------------------

void ShadowBlobManager::renderShadowBlobs (const Appearance& appearance)
{
	const int numberOfShadowBlobs = ms_objectList->getNumberOfObjects ();
	if (numberOfShadowBlobs > 0)
	{
		Camera const & camera = ShaderPrimitiveSorter::getCurrentCamera ();

		ms_currentShadowBlobShaderPrimitive = new ShadowBlobShaderPrimitive (appearance.getTransform_w ());

		int i;
		for (i = 0; i < numberOfShadowBlobs; ++i)
		{
			const ShadowBlobObject* const shadowBlobObject = safe_cast<const ShadowBlobObject*> (ms_objectList->getObject (i));
			if (shadowBlobObject)
			{
				const Vector position_w = shadowBlobObject->getAppearanceSphereCenter_w ();
				if (ShadowManager::simpleShouldRender (camera, position_w, shadowBlobObject->getRadius ()))
					appearance.renderShadowBlob (appearance.getOwner ()->rotateTranslate_w2o (position_w), shadowBlobObject->getRadius ());
			}
		}

		if (ms_currentShadowBlobShaderPrimitive->getNumberOfVertices () > 0)
			ShaderPrimitiveSorter::add (*ms_currentShadowBlobShaderPrimitive, 3);
		else
		{
			delete ms_currentShadowBlobShaderPrimitive;
			ms_currentShadowBlobShaderPrimitive = 0;
		}
	}
}

// ----------------------------------------------------------------------

void ShadowBlobManager::addVertex (const Vector& position, const float u, const float v)
{
	if (ms_currentShadowBlobShaderPrimitive)
		ms_currentShadowBlobShaderPrimitive->addVertex (position, u, v);
}

// ----------------------------------------------------------------------

void ShadowBlobManager::update (float const elapsedTime)
{
	ShadowBlobShaderPrimitive::update (elapsedTime);
}

// ======================================================================
// STATIC PRIVATE ShadowBlobManager
// ======================================================================

void ShadowBlobManagerNamespace::remove ()
{
	DEBUG_FATAL (!ms_installed, ("ShadowBlobManager::remove: not installed"));
	ms_installed = false;

	delete ms_objectList;
	ms_objectList = 0;

	std::for_each (ms_vertexListList.begin (), ms_vertexListList.end (), PointerDeleter ());
	ms_vertexListList.clear ();

	DebugFlags::unregisterFlag (ms_renderShadowBlobGeometry);
}

// ======================================================================

