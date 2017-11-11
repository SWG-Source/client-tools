//===================================================================
//
// ClientDynamicRadialFloraManager.cpp
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/ClientDynamicRadialFloraManager.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/StaticIndexBuffer.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientTerrain/ConfigClientTerrain.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/Transform.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedUtility/FileName.h"

#include <algorithm>
#include <map>
#include <string>

//===================================================================

namespace
{
	const Tag TAG_WABV = TAG (W,A,B,V);
	const Tag TAG_WBLW = TAG (W,B,L,W);

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static inline float scurve (float t)
	{
		return ((3.0f - (2.0f * t)) * t * t);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static inline float computeAlpha (const float minimum, const float value, const float maximum, const float featherIn)
	{
		if (value < minimum)
			return 0.f;

		if (value > maximum)
			return 0.f;

		const float feather = featherIn * (maximum - minimum) * 0.5f;

		if (value < minimum + feather)
		{
			return (minimum == 0.f) ? 1.f : (scurve ((value - minimum) / feather));
		}
		else
			if (value > (maximum - feather))
			{
				return scurve ((maximum - value) / feather);
			}
			
		return 1.f;
	}
}

//===================================================================

class ClientDynamicRadialFloraManager::FloraRenderBucket
{
public:

	class LocalShaderPrimitive;

public:

	enum {
		sliceSize = 500
	};

	FloraRenderBucket (
		const ClientDynamicRadialFloraManager &i_owner,
		float renderDepth, 
		bool applyColor, 
		bool belowWater, 
		const char *shaderTemplateName
		);

	~FloraRenderBucket ();

	double      getFloraSwayAngle() const { return m_owner.m_floraSwayAngle; }

	void        draw () const;

	const char* getShaderTemplateName () const;
	void        clearBuckets () const;
	void        addToBucket (float depth, const ClientDynamicRadialFloraManager::DynamicRadialNode* dynamicRadialNode) const;

private:

	struct SortableRadialNode
	{
		typedef ClientDynamicRadialFloraManager::DynamicRadialNode RadialNode;

		SortableRadialNode(float i_depth, const RadialNode &i_radialNode)
			: depth(i_depth), radialNode(&i_radialNode) {}

		SortableRadialNode(const SortableRadialNode &o) { depth=o.depth; radialNode=o.radialNode; }

		bool operator<(const SortableRadialNode &x) const { return depth>x.depth; } // inverted sorter for back to front rendering

		float depth;
		const ClientDynamicRadialFloraManager::DynamicRadialNode *radialNode;
	};

	typedef std::vector<SortableRadialNode>    RadialNodeList;
	typedef std::vector<LocalShaderPrimitive*> LocalShaderPrimitiveList;	

private:

	FloraRenderBucket ();
	FloraRenderBucket (const FloraRenderBucket&);             //lint -esym(754, FloraRenderBucket::FloraRenderBucket)
	FloraRenderBucket& operator= (const FloraRenderBucket&);  //lint -esym(754, FloraRenderBucket::operator=)

	void _sortRadialNodes() const;
	void _clearLocalShaderPrimitiveList() const;

private:

	const ClientDynamicRadialFloraManager                    &m_owner;
	const ClientProceduralTerrainAppearance                  &m_terrainAppearance;
	const float                                               m_renderDepth;
	const bool                                                m_applyColor;
	const int                                                 m_phase;
	const char* const                                         m_shaderTemplateName;
	const Shader*                                             m_shader;
	mutable RadialNodeList                                    m_radialNodes;
	mutable LocalShaderPrimitiveList                          m_localShaderPrimitiveList;

private:

	friend class LocalShaderPrimitive;
};

//===================================================================

class ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive : public ShaderPrimitive
{
public:

	typedef ClientDynamicRadialFloraManager::FloraRenderBucket   FloraRenderBucket;
	typedef FloraRenderBucket::SortableRadialNode                SortableRadialNode;
	typedef ClientDynamicRadialFloraManager::DynamicRadialNode   DynamicRadialNode;
	typedef ClientProceduralTerrainAppearance::DynamicFloraData  DynamicFloraData;

	static void install();
	static void remove();

	LocalShaderPrimitive (
		const FloraRenderBucket  &owner,
		const SortableRadialNode *radialNodes,
		int radialNodeCount
		);
	virtual ~LocalShaderPrimitive ();

	virtual const Vector        getPosition_w () const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual const StaticShader& prepareToView () const;
	virtual void                prepareToDraw () const;
	virtual void                draw () const;

private:

	LocalShaderPrimitive ();
	LocalShaderPrimitive (const LocalShaderPrimitive& rhs);             //lint -esym(754,LocalShaderPrimitive::LocalShaderPrimitive)
	LocalShaderPrimitive& operator= (const LocalShaderPrimitive& rhs);  //lint -esym(754,LocalShaderPrimitive::operator=)

private:

	static VertexBufferFormat _makeVertexFormat()
	{
		VertexBufferFormat format;
		format.setPosition ();
		format.setNormal ();
		format.setColor0 ();
		format.setNumberOfTextureCoordinateSets (1);
		format.setTextureCoordinateSetDimension (0, 2);

		return format;
	}

	const FloraRenderBucket                                 &m_owner;
	const SortableRadialNode                                *m_radialNodes;
	int                                                      m_radialNodeCount;

	mutable DynamicVertexBuffer  m_vertexBuffer;

	static StaticIndexBuffer    *ms_indexBuffer;
	static MemoryBlockManager   *ms_memoryBlockManager;
};

//===================================================================

class ClientDynamicRadialFloraManager::DynamicRadialNode : public ClientRadialFloraManager::RadialNode
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

private:

	Transform m_transform;       //lint !e1925  //-- public data member
	float     m_deltaDirection;  //lint !e1925  //-- public data member
	float     m_randomWidthScale;
	float     m_randomHeightScale;
	PackedRgb m_color;           //lint !e1925  //-- public data member
	ClientProceduralTerrainAppearance::DynamicFloraData m_dynamicFloraData;  //lint !e1925  //-- public data member

public:

	DynamicRadialNode ();
	virtual ~DynamicRadialNode ();

	////////////////////////////////////////////////////////////////////////////

	virtual const Vector     getPosition () const;
	virtual void             setPosition (const Vector& position);
	virtual bool             shouldFloat () const;
	virtual bool             shouldAlignToTerrain () const;
	virtual const Transform& getTransform () const;
	virtual void             setTransform (const Transform& transform);

	////////////////////////////////////////////////////////////////////////////

	void                     draw () const;

	const Transform &        transform()                                   const { return m_transform;        }
	      Transform &        transform()                                         { return m_transform;        }

	float                    getDeltaDirection()                           const { return m_deltaDirection;   }
	void                     setDeltaDirection(float i_dir)                      { m_deltaDirection=i_dir;    }

	float                    getRandomWidthScale()                         const { return m_randomWidthScale; }
	void                     setRandomWidthScale(float val)                      { m_randomWidthScale=val;      }

	float                    getRandomHeightScale()                        const { return m_randomHeightScale;  }
	void                     setRandomHeightScale(float val)                     { m_randomHeightScale=val;     }

	const PackedRgb &        getColor()                                    const { return m_color;            }
	void                     setColor(const PackedRgb &i_color)                  { m_color=i_color;           }

	typedef ClientProceduralTerrainAppearance::DynamicFloraData data;

	const data &getFloraData()              const { return m_dynamicFloraData; }
	void        setFloraData(const data &i)       { m_dynamicFloraData=i; }

private:

	DynamicRadialNode (const DynamicRadialNode&);             //lint -esym(754, DynamicRadialNode::DynamicRadialNode)
	DynamicRadialNode& operator= (const DynamicRadialNode&);  //lint -esym(754, DynamicRadialNode::operator=)
};

//===================================================================

ClientDynamicRadialFloraManager::FloraRenderBucket::FloraRenderBucket (const ClientDynamicRadialFloraManager &i_owner, 
																							  float renderDepth, 
																							  const bool applyColor, 
																							  const bool belowWater, 
																							  const char* shaderTemplateName
																							  ) 
:	m_owner(i_owner),
	m_terrainAppearance(i_owner.getTerrainAppearance()),
	m_renderDepth (renderDepth),
	m_applyColor (applyColor),
	m_phase (belowWater ? ShaderPrimitiveSorter::getPhase (TAG_WBLW) : ShaderPrimitiveSorter::getPhase (TAG_WABV)),
	m_shaderTemplateName (shaderTemplateName),
	m_shader (0)
{
	m_shader = ShaderTemplateList::fetchShader (FileName (FileName::P_shader, shaderTemplateName));
}

//-------------------------------------------------------------------
	
ClientDynamicRadialFloraManager::FloraRenderBucket::~FloraRenderBucket ()
{
	_clearLocalShaderPrimitiveList();

	m_shader->release ();
	m_shader = 0;
}

//-------------------------------------------------------------------

const char* ClientDynamicRadialFloraManager::FloraRenderBucket::getShaderTemplateName () const
{
	return m_shaderTemplateName;
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::FloraRenderBucket::clearBuckets () const
{
	//-- clear the radial node map
	m_radialNodes.clear();

	//-- clear out the local shader primitive list
	_clearLocalShaderPrimitiveList();
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::FloraRenderBucket::_clearLocalShaderPrimitiveList() const
{
	LocalShaderPrimitiveList::iterator i = m_localShaderPrimitiveList.begin();
	LocalShaderPrimitiveList::iterator iend = m_localShaderPrimitiveList.end();
	for (;i!=iend;++i)
	{
		delete *i;
	}
	m_localShaderPrimitiveList.clear();
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::FloraRenderBucket::_sortRadialNodes() const
{
	std::sort(m_radialNodes.begin(), m_radialNodes.end());

	if (ConfigClientTerrain::getRadialFloraSortFrontToBack())
	{
		RadialNodeList::iterator i=m_radialNodes.begin();
		RadialNodeList::iterator j=m_radialNodes.end()-1;
		while (i<j)
		{
			SortableRadialNode temp(*i);
			*i=*j;
			*j=temp;

			++i;
			--j;
		}
	}
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::FloraRenderBucket::addToBucket(float depth, const ClientDynamicRadialFloraManager::DynamicRadialNode *dynamicRadialNode) const
{
	m_radialNodes.push_back(SortableRadialNode(depth, *dynamicRadialNode));
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::alter(float elapsedTime)
{
	m_floraSwayAngle += PI_TIMES_2*elapsedTime;
	ClientRadialFloraManager::alter(elapsedTime);
}

//-------------------------------------------------------------------
		
void ClientDynamicRadialFloraManager::FloraRenderBucket::draw () const
{
	if (m_radialNodes.empty())
	{
		return;
	}

	_sortRadialNodes();

	const int numRadials = m_radialNodes.size();
	int i;

	for (i=0;i<numRadials;i+=sliceSize)
	{
		LocalShaderPrimitive *new_prim;
		int count;

		count = numRadials - i;
		if (count>sliceSize)
		{
			count=sliceSize;
		}

		new_prim=new LocalShaderPrimitive(*this, &m_radialNodes[i], count);
		m_localShaderPrimitiveList.push_back(new_prim);
	}

	//-- render each local shader primitive
	{
		for (LocalShaderPrimitiveList::iterator iter = m_localShaderPrimitiveList.begin (); iter != m_localShaderPrimitiveList.end (); ++iter)
			ShaderPrimitiveSorter::add (*(*iter), m_phase);
	}

	DEBUG_REPORT_PRINT (ms_reportDynamicFlora, ("phase=%i, distance=%1.0f, flora=%i, primitives=%i\n", m_phase, m_renderDepth, m_radialNodes.size (), m_localShaderPrimitiveList.size ()));
}

//===================================================================
StaticIndexBuffer    *ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive::ms_indexBuffer;
MemoryBlockManager   *ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive::ms_memoryBlockManager;

void ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("Already installed"));
	ms_memoryBlockManager = new MemoryBlockManager("ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive", true, sizeof(ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive), 0, 0, 0);
	ExitChain::add(remove, "ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive::remove()");

	ms_indexBuffer = new StaticIndexBuffer(FloraRenderBucket::sliceSize * 6);

	ms_indexBuffer->lock();

	const Index positiveIndices [6] = { 0, 1, 2, 0, 2, 3 };
	//const Index negativeIndices [6] = { 0, 2, 1, 0, 3, 2 };

	Index *ii=ms_indexBuffer->begin();
	for (int i=0;i<FloraRenderBucket::sliceSize;i++)
	{
		const Index indexOffset=Index(i*4);
		ii[0]=Index(positiveIndices[0] + indexOffset);
		ii[1]=Index(positiveIndices[1] + indexOffset);
		ii[2]=Index(positiveIndices[2] + indexOffset);
		ii[3]=Index(positiveIndices[3] + indexOffset);
		ii[4]=Index(positiveIndices[4] + indexOffset);
		ii[5]=Index(positiveIndices[5] + indexOffset);
		ii+=6;
	}

	ms_indexBuffer->unlock();
}

// ----------------------------------------------------------------------

void ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive::remove()
{
	if (ms_indexBuffer)
	{
		delete ms_indexBuffer;
	}

	NOT_NULL(ms_memoryBlockManager);
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

//===================================================================

ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive::LocalShaderPrimitive (
	const ClientDynamicRadialFloraManager::FloraRenderBucket &owner,
	const ClientDynamicRadialFloraManager::FloraRenderBucket::SortableRadialNode *radialNodes,
	int radialNodeCount
	)
	: ShaderPrimitive(),
	m_owner(owner),
	m_radialNodes(radialNodes),
	m_radialNodeCount(radialNodeCount),
	m_vertexBuffer(_makeVertexFormat())
{
}

//-------------------------------------------------------------------

ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive::~LocalShaderPrimitive ()
{
}

//-------------------------------------------------------------------

const Vector ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive::getPosition_w() const
{
	return Vector::zero;
}
	
//-------------------------------------------------------------------

float ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return sqr(m_owner.m_renderDepth);
}

// ----------------------------------------------------------------------

int ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	// @todo get the id of the dynamic vb
	return 0;
}

//-------------------------------------------------------------------

const StaticShader& ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive::prepareToView () const
{
	return m_owner.m_shader->prepareToView ();
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive::prepareToDraw () const
{
	const Vector cameraPosition = ShaderPrimitiveSorter::getCurrentCamera ().getPosition_w ();

	const double swayAngle = m_owner.getFloraSwayAngle();

	const float uvs [4][2] =
	{
		{ 0.0f, 1.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f }
	};

	bool applyColor = m_owner.m_applyColor;
	VectorArgb color = VectorArgb::solidWhite;

	////////////////////////////////////////////////////////////////////////////////

	struct _vertex
	{
		Vector       pos;
		Vector       norm;
		PackedArgb   col0;
		float        tc0[2];
	};

	struct _quad_vertex
	{
		_vertex v0;
		_vertex v1;
		_vertex v2;
		_vertex v3;
	} q;

	q.v0.tc0[0]=uvs[0][0];
	q.v0.tc0[1]=uvs[0][1];
	q.v1.tc0[0]=uvs[1][0];
	q.v1.tc0[1]=uvs[1][1];
	q.v2.tc0[0]=uvs[2][0];
	q.v2.tc0[1]=uvs[2][1];
	q.v3.tc0[0]=uvs[3][0];
	q.v3.tc0[1]=uvs[3][1];

	////////////////////////////////////////////////////////////////////////////////

	m_vertexBuffer.lock (m_radialNodeCount * 4);

#ifdef _DEBUG

	const VertexBufferDescriptor &vbd = m_vertexBuffer.getVertexDescriptor();

	DEBUG_FATAL(vbd.vertexSize                   != sizeof(_vertex), (""));
	DEBUG_FATAL(vbd.offsetPosition               != 0, (""));
	DEBUG_FATAL(vbd.offsetNormal                 != 12, (""));
	DEBUG_FATAL(vbd.offsetColor0                 != 24, (""));
	DEBUG_FATAL(vbd.offsetTextureCoordinateSet[0]!= 28, (""));

#endif

	VertexBufferWriteIterator v = m_vertexBuffer.begin();
	_quad_vertex *qiter = (_quad_vertex *)v.getDataPointer();

	Transform t;

	for (int j = 0; j < m_radialNodeCount; ++j)
	{
		const DynamicRadialNode *radialNode = m_radialNodes[j].radialNode;

		//-----------------------------------------------------------------

		const Vector normal = radialNode->getNormal();
		q.v0.norm=normal;
		q.v1.norm=normal;
		q.v2.norm=normal;
		q.v3.norm=normal;

		//-----------------------------------------------------------------

		if (applyColor)
		{
			color = radialNode->getColor().convert();
		}
		color.a = radialNode->getAlpha();

		q.v0.col0=color;
		q.v1.col0=color;
		q.v2.col0=color;
		q.v3.col0=color;

		//-----------------------------------------------------------------

		//-- @ todo remove "t" and use appropriate flag
		if (!applyColor)
		{
			t = radialNode->transform();
			const Vector toCam = t.rotateTranslate_p2l (cameraPosition);
			t.yaw_l (toCam.theta ());
		}
		const Transform& transform = applyColor ? radialNode->transform() : t;

		const Transform::matrix_t &matrix = transform.getMatrix();

		//-----------------------------------------------------------------

		const Vector offset(matrix[0][3], matrix[1][3], matrix[2][3]);
		const DynamicFloraData &floraData = radialNode->getFloraData();

		// random scaling
		const float widthDelta = floraData.familyChildData->maxWidth - floraData.familyChildData->minWidth;
		const float heightDelta = floraData.familyChildData->maxHeight - floraData.familyChildData->minHeight;
		
		float widthPad = 0.0f;
		float heightPad = 0.0f;

		if(widthDelta != 0.0f)
		{
			widthPad = radialNode->getRandomWidthScale() * widthDelta;
		}
		
		if(floraData.familyChildData->maintainAspectRatio) // check for locked aspect ratio
		{
			if(widthDelta != 0.0f)
			{
				heightPad = widthPad/widthDelta * heightDelta;
			}
		}
		else if(heightDelta != 0.0f)
		{
			heightPad = radialNode->getRandomHeightScale() * heightDelta;
		}

		const float  width     = floraData.familyChildData->minWidth + widthPad;
		const float  height    = floraData.familyChildData->minHeight + heightPad;

		Vector sway;
		if (floraData.familyChildData->shouldSway)
		{
			double angle = double(floraData.familyChildData->period) * (swayAngle + double(radialNode->getDeltaDirection()));
			sway = floraData.familyChildData->displacement * float(sin(angle)) * Vector::unitX;
		}
		else
		{
			sway = Vector::zero;
		}

		//-----------------------------------------------------------------

		const Vector wvec(matrix[0][0]*width, matrix[1][0]*width, matrix[2][0]*width);
		const Vector hvec(matrix[0][1]*height, matrix[1][1]*height, matrix[2][1]*height);
	

		q.v0.pos=offset-wvec;
		q.v1.pos=offset+hvec-wvec+sway;
		q.v2.pos=offset+hvec+wvec+sway;
		q.v3.pos=offset+wvec;

		//-----------------------------------------------------------------

		*qiter++ = q;
	}

	m_vertexBuffer.unlock ();

	//--
	Graphics::setObjectToWorldTransformAndScale (Transform::identity, Vector::xyz111);
	Graphics::setVertexBuffer(m_vertexBuffer);
	Graphics::setIndexBuffer(*ms_indexBuffer);
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::FloraRenderBucket::LocalShaderPrimitive::draw () const
{
	const GlCullMode cullMode = Graphics::getCullMode ();
	Graphics::setCullMode (GCM_none);
	Graphics::drawIndexedTriangleList(0, 0, m_radialNodeCount*4, 0, m_radialNodeCount*2);
	Graphics::setCullMode (cullMode);
}

//===================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL (ClientDynamicRadialFloraManager::DynamicRadialNode, false, 512, 8, 0);

//===================================================================

ClientDynamicRadialFloraManager::DynamicRadialNode::DynamicRadialNode () : 
	ClientRadialFloraManager::RadialNode (),
	m_transform (),
	m_deltaDirection (0.f),
	m_randomWidthScale (0.0f),
	m_randomHeightScale (0.0f),
	m_color (),
	m_dynamicFloraData ()
{
}

//-------------------------------------------------------------------
	
ClientDynamicRadialFloraManager::DynamicRadialNode::~DynamicRadialNode ()
{
}

//-------------------------------------------------------------------

const Vector ClientDynamicRadialFloraManager::DynamicRadialNode::getPosition () const
{
	return m_transform.getPosition_p ();
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::DynamicRadialNode::setPosition (const Vector& position)
{
	m_transform.setPosition_p (position);
}

//-------------------------------------------------------------------

bool ClientDynamicRadialFloraManager::DynamicRadialNode::shouldFloat () const
{
	return false;
}

//-------------------------------------------------------------------

bool ClientDynamicRadialFloraManager::DynamicRadialNode::shouldAlignToTerrain () const
{
	return m_dynamicFloraData.familyChildData->alignToTerrain;
}

//-------------------------------------------------------------------

const Transform& ClientDynamicRadialFloraManager::DynamicRadialNode::getTransform () const
{
	return m_transform;
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::DynamicRadialNode::setTransform (const Transform& newTransform) 
{
	m_transform = newTransform;
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::DynamicRadialNode::draw () const
{
	const Camera& camera      = ShaderPrimitiveSorter::getCurrentCamera ();
	const float   width       = m_dynamicFloraData.familyChildData->maxWidth;
	const float   height      = m_dynamicFloraData.familyChildData->maxHeight;
	const float   floraRadius = std::max (width, height);
	const Vector  position    = m_transform.getPosition_p ();
	const Vector  center (position.x, position.y + height * 0.5f, position.z);

	if (camera.testVisibility_w (Sphere (center, floraRadius)))
	{
		float depth = camera.getPosition_w().magnitudeBetween(position);
		safe_cast<ClientDynamicRadialFloraManager*> (m_manager)->addToBucket (position, depth, this);
	}
}

//===================================================================

#ifdef _DEBUG
bool ClientDynamicRadialFloraManager::ms_noDraw;
bool ClientDynamicRadialFloraManager::ms_reportDynamicFlora;
#endif

//===================================================================

void ClientDynamicRadialFloraManager::install ()
{
	DEBUG_REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportInstall (), ("ClientDynamicRadialFloraManager::install\n"));

#ifdef _DEBUG
	DebugFlags::registerFlag (ms_noDraw,             "ClientTerrain", "noRenderDynamicFlora");
	DebugFlags::registerFlag (ms_reportDynamicFlora, "ClientTerrain", "reportDynamicFlora");
#endif

	FloraRenderBucket::LocalShaderPrimitive::install ();
	DynamicRadialNode::install ();

	ExitChain::add (remove, "ClientDynamicRadialFloraManager::remove");
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::remove ()
{
#ifdef _DEBUG
	DebugFlags::unregisterFlag (ms_noDraw);
	DebugFlags::unregisterFlag (ms_reportDynamicFlora);
#endif
}

//===================================================================

ClientDynamicRadialFloraManager::ClientDynamicRadialFloraManager (const ClientProceduralTerrainAppearance& terrainAppearance, const bool& enabled, float const minimumDistance, float const & maximumDistance, FindFloraFunction findFloraFunction, const bool applyColor) :
	ClientRadialFloraManager (terrainAppearance, enabled, minimumDistance, maximumDistance),
	m_floraSwayAngle(0),
	m_findFloraFunction (findFloraFunction),
	m_applyColor (applyColor)
{
	internalInitialize ();
}

//-------------------------------------------------------------------

ClientDynamicRadialFloraManager::~ClientDynamicRadialFloraManager ()
{
	freeBuckets();
	m_findFloraFunction = 0;
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::internalInitialize ()
{
	const float ringWidth = (m_maximumDistance - m_minimumDistance) / 10.f;
	float start = ringWidth;
	while (start <= m_maximumDistance)
	{
		m_floraBucketList.push_back(FloraBucket(start));
		start += ringWidth;
	}

	m_floraBucketList.push_back(FloraBucket(1024.f));
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::maximumDistanceChanged ()
{
	ClientRadialFloraManager::maximumDistanceChanged ();

	freeBuckets();

	internalInitialize ();
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::freeBuckets()
{
	FloraBucketList::const_iterator i, iend;
	i=m_floraBucketList.begin();
	iend=m_floraBucketList.end();
	for (;i!=iend;++i)
	{
		const FloraBucket &fb = *i;
		freeFloraRenderBuckets(fb.m_belowWaterFloraRenderBucketList);
		freeFloraRenderBuckets(fb.m_aboveWaterFloraRenderBucketList);
	}
	m_floraBucketList.clear();
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::freeFloraRenderBuckets(FloraRenderBucketList &i_list)
{
	FloraRenderBucketList::iterator i, iend;
	i=i_list.begin();
	iend=i_list.end();
	for (;i!=iend;++i)
	{
		FloraRenderBucket *n = *i;
		delete n;
	}
	i_list.clear();
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::clearBuckets() const
{
	FloraBucketList::const_iterator i, iend;
	i=m_floraBucketList.begin();
	iend=m_floraBucketList.end();
	for (;i!=iend;++i)
	{
		const FloraBucket &fb = *i;
		clearFloraRenderBuckets(fb.m_belowWaterFloraRenderBucketList);
		clearFloraRenderBuckets(fb.m_aboveWaterFloraRenderBucketList);
	}
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::clearFloraRenderBuckets(FloraRenderBucketList &i_list)
{
	FloraRenderBucketList::iterator i, iend;
	i=i_list.begin();
	iend=i_list.end();
	for (;i!=iend;++i)
	{
		FloraRenderBucket *n = *i;
		n->clearBuckets();
	}
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::drawBuckets() const
{
	FloraBucketList::const_iterator i, iend;
	i=m_floraBucketList.begin();
	iend=m_floraBucketList.end();
	for (;i!=iend;++i)
	{
		const FloraBucket &fb = *i;
		drawFloraRenderBuckets(fb.m_belowWaterFloraRenderBucketList);
		drawFloraRenderBuckets(fb.m_aboveWaterFloraRenderBucketList);
	}
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::drawFloraRenderBuckets(FloraRenderBucketList &i_list)
{
	FloraRenderBucketList::iterator i, iend;
	i=i_list.begin();
	iend=i_list.end();
	for (;i!=iend;++i)
	{
		FloraRenderBucket *n = *i;
		n->draw();
	}
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::draw () const
{
#ifdef _DEBUG
	if (ms_noDraw)
	{
		clearRegionList ();
		return;
	}
#endif

	if (!isEnabled ())
		return;

	PROFILER_AUTO_BLOCK_DEFINE ("ClientDynamicRadialFloraManager::draw");

	const Camera& camera = ShaderPrimitiveSorter::getCurrentCamera ();

	//-- update the flora
	update (getReferenceObject () ? getReferenceObject ()->getPosition_w () : camera.getPosition_w ());

	//-- reset the extent list for the next frame
	clearRegionList ();

	////////////////////////////////////////////////////////////
	//-- clear buckets
	clearBuckets();
	////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////
	//-- queue up radial nodes into flora node buckets
	RadialNodeList::const_iterator rni;
	for (rni=m_radialNodeList.begin();rni!=m_radialNodeList.end();++rni)
	{
	#ifdef _DEBUG
		_verifyRadialNodeReference(*rni);
	#endif
		if (rni->hasFlora)
		{
			DynamicRadialNode *rn = safe_cast<DynamicRadialNode *>(rni->nodePointer);
			rn->draw();
		}
	}
	////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////
	//-- render flora node buckets
	drawBuckets();
	////////////////////////////////////////////////////////////
}

//-------------------------------------------------------------------

bool ClientDynamicRadialFloraManager::createFlora (float positionX, float positionZ, RadialNode* radialNode, bool& floraAllowed) const
{
	ClientProceduralTerrainAppearance::DynamicFloraData dynamicFloraData;
	NOT_NULL (m_findFloraFunction);

	if ((m_terrainAppearance.*m_findFloraFunction) (positionX, positionZ, dynamicFloraData, floraAllowed))
	{
		DynamicRadialNode *const dynamicRadialNode = safe_cast<DynamicRadialNode*> (radialNode);
		dynamicRadialNode->setFloraData(dynamicFloraData);
		dynamicRadialNode->setColor(dynamicFloraData.color);

		return true;
	}

	return false;
}

//-------------------------------------------------------------------

ClientDynamicRadialFloraManager::RadialNode* ClientDynamicRadialFloraManager::createRadialNode (const Vector& position) const
{
	const float offset = position.x + position.z;

	DynamicRadialNode *const node = new DynamicRadialNode;
	node->transform().setPosition_p (position);
	node->transform().yaw_l (offset);
	node->setDeltaDirection(Random::randomReal(PI_TIMES_2));
	node->setRandomWidthScale(Random::randomReal(1.0f));
	node->setRandomHeightScale(Random::randomReal(1.0f));

	return node;
}

//-------------------------------------------------------------------

void ClientDynamicRadialFloraManager::addToBucket (const Vector& position, const float i_depth, const DynamicRadialNode* const dynamicRadialNode)
{
	DEBUG_FATAL (!dynamicRadialNode->getHasFlora (), ("ClientDynamicRadialFloraManager::addToBucket - adding empty flora to bucket"));

	float alpha = computeAlpha(m_minimumDistance, i_depth, m_maximumDistance, 0.75f);
	const_cast<DynamicRadialNode*>(dynamicRadialNode)->setAlpha(alpha);

	float depth=sqr(i_depth);

	//-- choose the right ring
	uint ring;
	uint num_rings=m_floraBucketList.size();
	for (ring = 0; ring < num_rings; ++ring)
	{
		if (depth < m_floraBucketList[ring].m_ringDistance)
			break;
	}
	if (ring == num_rings)
		--ring;
	FloraBucket *floraBucket = &m_floraBucketList[ring];

	// take render depth from next lower ring.
	const float renderDepth = ring == 0 ? 0.f : m_floraBucketList[ring-1].m_ringDistance;

	//-- are we below water?
	float waterHeight;
	const bool belowWater = m_terrainAppearance.getWaterHeight (position, waterHeight) && position.y < waterHeight;

	//-- find the bucket
	FloraRenderBucketList &floraBucketList = belowWater ? floraBucket->m_belowWaterFloraRenderBucketList : floraBucket->m_aboveWaterFloraRenderBucketList;

	const char* const shaderTemplateName = dynamicRadialNode->getFloraData().familyChildData->shaderTemplateName;

	// attempt to find a flora node using this shader.
	uint i;
	for (i = 0; i < floraBucketList.size (); ++i)
		if (floraBucketList[i]->getShaderTemplateName () == shaderTemplateName)
			break;

	//-- we can't use an existing bucket, so create a new one
	if (i == floraBucketList.size())
	{
		FloraRenderBucket *new_node;
		
		new_node = new FloraRenderBucket (*this, renderDepth, m_applyColor, belowWater, shaderTemplateName);
		floraBucketList.push_back(new_node);
	}

	FloraRenderBucket *const dynamicFloraRenderBucket = floraBucketList[i];
	dynamicFloraRenderBucket->addToBucket(depth, dynamicRadialNode);
}

//===================================================================
