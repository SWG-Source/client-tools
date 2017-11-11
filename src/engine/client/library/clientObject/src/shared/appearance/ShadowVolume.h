//===================================================================
//
// ShadowVolume.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ShadowVolume_H
#define INCLUDED_ShadowVolume_H

//===================================================================

#include "clientGraphics/Graphics.def"

class Camera;
class Object;
class Appearance;
class Shader;
class StaticIndexBuffer;
class StaticVertexBuffer;
class SystemIndexBuffer;
class SystemVertexBuffer;
class Vector;
class VectorArgb;
class VertexBufferReadIterator;

//===================================================================

class ShadowVolume
{
public:

	enum PrimitiveType
	{
		PT_static,
		PT_animating
	};

	enum ShaderType
	{
		ST_fixedFunction,
		ST_vertexShader
	};

public:

	ShadowVolume (ShaderType shaderType, PrimitiveType primitiveType, const char* debugName);
	~ShadowVolume ();

	//-- for static primitives, this is done only once per primitive. for animating primitives, this is done every time the primitive changes
	void addPrimitive (const SystemVertexBuffer* vertexBuffer, const StaticIndexBuffer* indexBuffer);
	void addPrimitive (const StaticVertexBuffer* vertexBuffer, const StaticIndexBuffer* indexBuffer);
	void render(Object const * object, const Appearance *appearance) const;

public:

	static void          install ();
	static void          remove ();

	static bool          getEnabled ();
	static void          setEnabled (bool enabled);

	static void          setShowExtrudedEdges (bool newShowExtrudedEdges);
	static bool          getShowExtrudedEdges ();

	static void          setShowNonManifoldEdges (bool newShowNonManifoldEdges);
	static bool          getShowNonManifoldEdges ();

	static const Vector& getDirectionToLight (void);
	static void          setDirectionToLight (const Vector& newDirectionToLight, bool viewer=false);

	static void          setShadowColor (const VectorArgb& shadowColor);

	static void          renderShadowAlpha (const Camera* camera);

	static void setAllowShadowSubmissions (bool allowShadowSubmissions);

private:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum CapMode
	{
		CM_front,
		CM_back
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Edge
	{
	public:

		int                 v0;
		int                 v1;
		int                 numberOfFaces;
		int                 face [2];

#ifdef _DEBUG
		bool                isNonManifold;
#endif
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Metrics
	{
	public:

		int                 minimumShadowVolumeVertices;
		int                 maximumShadowVolumeVertices;
		int                 minimumShadowVolumeIndices;
		int                 maximumShadowVolumeIndices;

	public:

		Metrics::Metrics () :
			minimumShadowVolumeVertices (1 << 17),
			maximumShadowVolumeVertices (0),
			minimumShadowVolumeIndices (1 << 17),
			maximumShadowVolumeIndices (0)
		{
		}
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct ShadowPrimitive;

	friend class ProxyLocalShaderPrimitive;

	class LocalShaderPrimitiveRenderEdgesOneSided;
	friend class LocalShaderPrimitiveRenderEdgesOneSided;

	class LocalShaderPrimitiveRenderCapsOneSided;
	friend class LocalShaderPrimitiveRenderCapsOneSided;

	class LocalShaderPrimitiveRenderEdgesTwoSided;
	friend class LocalShaderPrimitiveRenderEdgesTwoSided;

	class LocalShaderPrimitiveRenderCapsTwoSided;
	friend class LocalShaderPrimitiveRenderCapsTwoSided;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	static bool          ms_debugReport;
	static bool          ms_enabled;
	static bool          ms_showExtrudedEdges;
	static bool          ms_showNonManifoldEdges;
	static Vector        ms_directionToLight;
	static VectorArgb    ms_shadowColor;
	static bool          ms_supportsTwoSidedStencil;
	static const Shader* ms_shadowVolumeOneSidedIncrementShader;
	static const Shader* ms_shadowVolumeOneSidedDecrementShader;
	static const Shader* ms_shadowVolumeTwoSidedShader;
	static const Shader* ms_shadowVolumeScreenAlphaShader;
	static bool          ms_renderShadowsThisFrame;

	typedef stdvector<ProxyLocalShaderPrimitive*>::fwd ProxyLocalShaderPrimitiveList;
	static ProxyLocalShaderPrimitiveList* ms_proxyLocalShaderPrimitiveList;

private:

	static void  clearProxyLocalShaderPrimitiveList ();
	static void  addEdge (ShadowVolume::Edge* const edgeList, int& numberOfEdges, const int v0, const int v1, const int face);

	static void  debugDump ();

private:

	//-- info about the source geometry
	const ShaderType                         m_shaderType;
	const PrimitiveType                      m_primitiveType;
	const char* const                        m_debugName;

	//-- one-sided stencil
	LocalShaderPrimitiveRenderEdgesOneSided* m_localShaderPrimitiveRenderEdgesOneSidedCullClockwise;
	LocalShaderPrimitiveRenderEdgesOneSided* m_localShaderPrimitiveRenderEdgesOneSidedCullCounterClockwise;
	LocalShaderPrimitiveRenderCapsOneSided*  m_localShaderPrimitiveRenderFrontCapsOneSidedCullClockwise;
	LocalShaderPrimitiveRenderCapsOneSided*  m_localShaderPrimitiveRenderFrontCapsOneSidedCullCounterClockwise;
	LocalShaderPrimitiveRenderCapsOneSided*  m_localShaderPrimitiveRenderBackCapsOneSidedCullClockwise;
	LocalShaderPrimitiveRenderCapsOneSided*  m_localShaderPrimitiveRenderBackCapsOneSidedCullCounterClockwise;

	//-- two-sided stencil
	LocalShaderPrimitiveRenderEdgesTwoSided* m_localShaderPrimitiveRenderEdgesTwoSided;
	LocalShaderPrimitiveRenderCapsTwoSided*  m_localShaderPrimitiveRenderFrontCapsTwoSided;
	LocalShaderPrimitiveRenderCapsTwoSided*  m_localShaderPrimitiveRenderBackCapsTwoSided;

	//-- geometry that contains the shadow
	typedef stdvector<ShadowPrimitive*>::fwd ShadowPrimitiveList;
	mutable ShadowPrimitiveList*             m_shadowPrimitiveList;

	//-- used to keep track of how we're doing
	mutable Metrics                          m_metrics;

private:

	void addPrimitive (const VertexBufferReadIterator& vertexBufferReadIterator, int numberOfVertices, const Index* indexArray, int numberOfIndices);

	void extrudeShadowVolume (const Vector& lightDirection_o) const;
	void renderShadowVolumeEdges () const;
	void renderShadowVolumeCaps (const CapMode capMode) const;

	int getMemorySize () const;

private:

	ShadowVolume (const ShadowVolume& rhs);
	ShadowVolume& operator= (const ShadowVolume& rhs);
};

//===================================================================

#endif
