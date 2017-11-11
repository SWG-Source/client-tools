// ======================================================================
//
// ReticleManager.cpp
// asommers
// 
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/ReticleManager.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "clientGraphics/VertexBufferFormat.h"
#include "clientGraphics/VertexBufferIterator.h"
#include "clientObject/ReticleObject.h"
#include "clientObject/ShadowManager.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Timer.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/ObjectList.h"

#include <algorithm>
#include <vector>

// ======================================================================

namespace ReticleManagerNamespace
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
	bool ms_enabled = true;
	bool ms_renderReticleGeometry;
	ObjectList* ms_objectList;
	
	const float cms_maximumReticleDistance          = 500.f;
	const float cms_ignoreRatio                     = 10.f / 8000.f;

	typedef std::vector<Vertex> VertexList;
	std::vector<VertexList*> ms_vertexListList;

	class ReticleShaderPrimitive;
	ReticleShaderPrimitive* ms_currentReticleShaderPrimitive;

	std::vector<ReticleObject*> ms_reticleObjectList;

	Vector ms_lastGroundReticlePoint;
	NetworkId ms_lastGroundReticleCell;
	bool ms_reticleCurrentlyValid = false;
	bool ms_renderReticles = false;

	ReticleManager::ReticleManagerType ms_reticleType = ReticleManager::RMT_HeavyWeapons;
	const Texture *ms_currentTexture = NULL;
	const Texture *ms_reticleTextures[ReticleManager::RMT_TotalTypes];
	const Tag  TAG_MAIN = TAG(M,A,I,N);

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
	bool shouldRender (Camera const & camera, Vector const & position_w, float const radius, float const maximumDistance)
	{	
		const int ignoreSize = static_cast<int> (cms_ignoreRatio * Graphics::getCurrentRenderTargetWidth ());

		//-- check minimum screen size
		float screenRadius = 0.f;
		if (camera.computeRadiusInScreenSpace (camera.rotateTranslate_w2o (position_w), radius, screenRadius) && screenRadius < ignoreSize)
			return false;
		
		//-- check maximum distance
		if (camera.getPosition_w ().magnitudeBetweenSquared (position_w) > sqr (maximumDistance))
			return false;
		
		return true;
	}
}

using namespace ReticleManagerNamespace;

// ======================================================================

class ReticleManagerNamespace::ReticleShaderPrimitive : public ShaderPrimitive
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install ();

	static void update (float elapsedTime);

public:

	explicit ReticleShaderPrimitive (const Transform& transform);
	virtual ~ReticleShaderPrimitive ();

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

	ReticleShaderPrimitive ();
	ReticleShaderPrimitive (const ReticleShaderPrimitive& rhs);             //lint -esym (754, ReticleShaderPrimitive::ReticleShaderPrimitive)
	ReticleShaderPrimitive& operator= (const ReticleShaderPrimitive& rhs);  //lint -esym (754, ReticleShaderPrimitive::operator=)

private:

	static Shader* ms_ReticleShader;

private:

	const Transform m_transform;
	VertexList* const m_vertexList;
};

//-------------------------------------------------------------------

Shader* ReticleManagerNamespace::ReticleShaderPrimitive::ms_ReticleShader;

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL (ReticleManagerNamespace::ReticleShaderPrimitive, true, 0, 0, 0);

//-------------------------------------------------------------------

void ReticleManagerNamespace::ReticleShaderPrimitive::install ()
{
	installMemoryBlockManager ();

	ms_ReticleShader = ShaderTemplateList::fetchModifiableShader ("shader/reticle.sht");

	ExitChain::add (ReticleManagerNamespace::ReticleShaderPrimitive::remove, "ReticleManagerNamespace::ReticleShaderPrimitive::remove");
}

//-------------------------------------------------------------------

void ReticleManagerNamespace::ReticleShaderPrimitive::update (float elapsedTime)
{	
	
	if(ms_currentTexture != ms_reticleTextures[static_cast<int>(ms_reticleType)])
	{
		ms_ReticleShader->getStaticShader()->setTexture(TAG_MAIN, *ms_reticleTextures[static_cast<int>(ms_reticleType)]);
		ms_currentTexture = ms_reticleTextures[static_cast<int>(ms_reticleType)];
	}

	ms_ReticleShader->alter (elapsedTime);
}

//-------------------------------------------------------------------

void ReticleManagerNamespace::ReticleShaderPrimitive::remove ()
{
	removeMemoryBlockManager ();

	ms_ReticleShader->release ();
	ms_ReticleShader = 0;
}

//-------------------------------------------------------------------

ReticleManagerNamespace::ReticleShaderPrimitive::ReticleShaderPrimitive (const Transform& transform) :
	ShaderPrimitive (),
	m_transform (transform),
	m_vertexList (createVertexList ())
{
}

//-------------------------------------------------------------------

ReticleManagerNamespace::ReticleShaderPrimitive::~ReticleShaderPrimitive ()
{
	destroyVertexList (m_vertexList);
}

//-------------------------------------------------------------------

int ReticleShaderPrimitive::getNumberOfVertices () const
{
	return static_cast<int> (m_vertexList->size ());
}

//-------------------------------------------------------------------

void ReticleShaderPrimitive::addVertex (const Vector& position, const float u, const float v)
{	
	DEBUG_FATAL(m_vertexList->size() > 75000, ("vertex list is too big"));
	Vertex vertex;
	vertex.position = position;
	vertex.u = u;
	vertex.v = v;

	m_vertexList->push_back (vertex);
}

//-------------------------------------------------------------------

const Vector ReticleManagerNamespace::ReticleShaderPrimitive::getPosition_w () const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float ReticleManagerNamespace::ReticleShaderPrimitive::getDepthSquaredSortKey () const
{
	return 0.f;
}

//-------------------------------------------------------------------

int ReticleManagerNamespace::ReticleShaderPrimitive::getVertexBufferSortKey () const
{
	return 0;
}

//-------------------------------------------------------------------

const StaticShader& ReticleManagerNamespace::ReticleShaderPrimitive::prepareToView () const
{
	if (ms_renderReticleGeometry)
		return ShaderTemplateList::get3dVertexColorStaticShader ();

	return ms_ReticleShader->prepareToView ();
}

//-------------------------------------------------------------------

void ReticleManagerNamespace::ReticleShaderPrimitive::prepareToDraw () const
{
	Graphics::setObjectToWorldTransformAndScale (m_transform, Vector::xyz111);

	{
		VertexBufferFormat format;
		format.setPosition ();
		format.setColor0 ();
		format.setNumberOfTextureCoordinateSets (1);
		format.setTextureCoordinateSetDimension (0, 2);

		DynamicVertexBuffer vertexBuffer (format);


#if 1
		// JU_TODO: pick one of these: bail out for now
		// bail out if the vert list is larger that what the dynamic vertex buffer can take
		const uint dynamicVertexBufferSize = vertexBuffer.getNumberOfLockableDynamicVertices(true);
		if(m_vertexList->size() > dynamicVertexBufferSize)
		{
			return;
		}

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
#else
		// JU_TODO: pick one of these: only draw what I can 
		// bail out if the vert list is larger that what the dynamic vertex buffer can take
		const uint dynamicVertexBufferSize = vertexBuffer.getNumberOfLockableDynamicVertices(true);
		uint vertexListSize = m_vertexList->size(); 
		if(m_vertexList->size() > dynamicVertexBufferSize)
		{
			vertexListSize = dynamicVertexBufferSize;
			vertexBuffer.lock (vertexListSize);
		}
		else
		{
			vertexBuffer.lock (m_vertexList->size ());
		}

			VertexBufferWriteIterator v = vertexBuffer.begin ();
			const uint n = vertexListSize;
			uint i;
			for (i = 0; i < n; ++i, ++v)
			{
				const Vertex& vertex = (*m_vertexList) [i];

				v.setPosition (vertex.position);
				v.setColor0 (VectorArgb::solidWhite);
				v.setTextureCoordinates (0, vertex.u, vertex.v);
			}

		vertexBuffer.unlock ();
#endif

		Graphics::setVertexBuffer (vertexBuffer);
	}
}

//-------------------------------------------------------------------

void ReticleManagerNamespace::ReticleShaderPrimitive::draw () const
{
	const GlFillMode fillMode = Graphics::getFillMode ();

	if (ms_renderReticleGeometry)
		Graphics::setFillMode (GFM_wire);

	Graphics::drawTriangleList ();

	if (ms_renderReticleGeometry)
		Graphics::setFillMode (fillMode);

	//-- free this primitive for reuse
	delete const_cast<ReticleShaderPrimitive*> (this);
}

// ======================================================================
// STATIC PUBLIC ReticleManager
// ======================================================================

void ReticleManager::install ()
{
	DEBUG_FATAL (ms_installed, ("ReticleManager::install: installed"));
	ms_installed = true;

	ReticleShaderPrimitive::install ();

	ms_objectList = new ObjectList;

	// Load 1 texture per Reticle type. If you add a new type, you need to make sure
	// you load the texture here.
	char tmp[512];		
	sprintf(tmp, "texture/heavyweapons_reticule.dds");
	ms_reticleTextures[static_cast<int>(ReticleManager::RMT_HeavyWeapons)] = TextureList::fetch(tmp);

	sprintf(tmp, "texture/storyteller_reticule.dds");
	ms_reticleTextures[static_cast<int>(ReticleManager::RMT_StoryTeller)] = TextureList::fetch(tmp);

	sprintf(tmp, "texture/grenade_reticule.dds");
	ms_reticleTextures[static_cast<int>(ReticleManager::RMT_GrenadeTarget)] = TextureList::fetch(tmp);	


	DebugFlags::registerFlag (ms_renderReticleGeometry, "ClientObject", "renderReticleGeometry");
	ExitChain::add (ReticleManagerNamespace::remove, "ReticleManagerNamespace::remove");
}

// ----------------------------------------------------------------------

bool ReticleManager::getEnabled ()
{
	return ms_enabled;
}

// ----------------------------------------------------------------------

void ReticleManager::setEnabled (bool const enabled)
{
	ms_enabled = enabled;
}

// ----------------------------------------------------------------------

void ReticleManager::enableReticleObject (const ReticleObject& reticleObject)
{
	DEBUG_FATAL (!ms_installed, ("ReticleManager::install: not installed"));

	if (ms_enabled)
	{
		if (!ms_objectList->find(&reticleObject, NULL))
		{
			ms_objectList->addObject (const_cast<ReticleObject*> (&reticleObject));
		}
	}
}

// ----------------------------------------------------------------------

void ReticleManager::disableReticleObject (const ReticleObject& reticleObject)
{
	ms_objectList->removeObject (const_cast<ReticleObject*> (&reticleObject));
}


void ReticleManager::renderReticles (const Appearance& appearance)
{
	if(!ms_enabled || !ms_reticleCurrentlyValid || !ms_renderReticles)
		return;
	const int numberOfReticles = ms_objectList->getNumberOfObjects ();
	if (numberOfReticles > 0)
	{
		Camera const & camera = ShaderPrimitiveSorter::getCurrentCamera ();

		ms_currentReticleShaderPrimitive = new ReticleShaderPrimitive (appearance.getTransform_w ());

		int i;
		for (i = 0; i < numberOfReticles; ++i)
		{
			const ReticleObject* const reticleObject = safe_cast<const ReticleObject*> (ms_objectList->getObject (i));
			if (reticleObject)
			{
				const Vector position_w = reticleObject->getAppearanceSphereCenter_w ();
				if (ReticleManagerNamespace::shouldRender (camera, position_w, reticleObject->getRadius (), cms_maximumReticleDistance))
					appearance.renderReticle (appearance.getOwner ()->rotateTranslate_w2o (position_w), reticleObject->getRadius ());
			}
		}

		if (ms_currentReticleShaderPrimitive->getNumberOfVertices () > 0)
			ShaderPrimitiveSorter::add (*ms_currentReticleShaderPrimitive, 3);
		else
		{
			delete ms_currentReticleShaderPrimitive;
			ms_currentReticleShaderPrimitive = 0;
		}
	}
}

// ----------------------------------------------------------------------

void ReticleManager::addVertex (const Vector& position, const float u, const float v)
{
	if (ms_currentReticleShaderPrimitive)
		ms_currentReticleShaderPrimitive->addVertex (position, u, v);
}

// ----------------------------------------------------------------------

void ReticleManager::update (float const elapsedTime)
{	
	ReticleShaderPrimitive::update (elapsedTime);
}

// ----------------------------------------------------------------------

ReticleObject *ReticleManager::getReticleObject()
{
	while(!ms_reticleObjectList.empty())
	{
		ReticleObject *result = ms_reticleObjectList.back();
		ms_reticleObjectList.pop_back();
		if(result)
			return result;
	}
	
	ReticleObject *result = new ReticleObject(5.0f);
	return result;
}

// ----------------------------------------------------------------------

void ReticleManager::giveBackReticleObject(ReticleObject *reticle)
{
	if(reticle)
		ms_reticleObjectList.push_back(reticle);
}

// ======================================================================
// STATIC PRIVATE ReticleManager
// ======================================================================

void ReticleManagerNamespace::remove ()
{
	DEBUG_FATAL (!ms_installed, ("ReticleManager::remove: not installed"));
	ms_installed = false;

	delete ms_objectList;
	ms_objectList = 0;

	std::for_each (ms_vertexListList.begin (), ms_vertexListList.end (), PointerDeleter ());
	ms_vertexListList.clear ();

	for(std::vector<ReticleObject*>::iterator i = ms_reticleObjectList.begin(); i != ms_reticleObjectList.end(); ++i)
	{		
		if((*i)->isInWorld())
			(*i)->removeFromWorld();
		delete (*i);
	}
	ms_reticleObjectList.clear();

	DebugFlags::unregisterFlag (ms_renderReticleGeometry);

	for(int j = 0; j < ReticleManager::RMT_TotalTypes; ++j)
	{	
		ms_reticleTextures[j]->release();
		ms_reticleTextures[j] = 0;
	}
}

// ----------------------------------------------------------------------

void ReticleManager::setLastGroundReticlePoint(const Vector &lastGroundReticlePoint)
{
	ms_lastGroundReticlePoint = lastGroundReticlePoint;
}

// ----------------------------------------------------------------------

const Vector &ReticleManager::getLastGroundReticlePoint()
{
	return ms_lastGroundReticlePoint;
}

// ----------------------------------------------------------------------

void ReticleManager::setLastGroundReticleCell(const NetworkId &lastGroundReticleCell)
{
	ms_lastGroundReticleCell = lastGroundReticleCell;
}

// ----------------------------------------------------------------------

const NetworkId &ReticleManager::getLastGroundReticleCell()
{
	return ms_lastGroundReticleCell;
}

// ----------------------------------------------------------------------

void ReticleManager::setReticleCurrentlyValid(bool reticleValid)
{
	ms_reticleCurrentlyValid = reticleValid;
}

// ----------------------------------------------------------------------

bool ReticleManager::getReticleCurrentlyValid(void)
{
	return ms_reticleCurrentlyValid;
}

// ----------------------------------------------------------------------

void ReticleManager::setRenderReticles(bool renderReticles)
{
	ms_renderReticles = renderReticles;
}

// ----------------------------------------------------------------------

bool ReticleManager::getRenderReticles(void)
{
	return ms_renderReticles;
}

// ----------------------------------------------------------------------

void ReticleManager::setReticleType(ReticleManagerType const & type)
{
	ms_reticleType = type;
}

// ======================================================================

