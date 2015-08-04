// ======================================================================
//
// ShaderPrimitiveSorter.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"

#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsOptionTags.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/PostProcessingEffectsManager.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedMath/VectorRgba.h"
#include "sharedObject/CellProperty.h"
#include "sharedUtility/LocalMachineOptionManager.h"
#include <algorithm>
#include <bitset>
#include <map>
#include <typeinfo>
#include <vector>

// ======================================================================

const Tag TAG_ENVM = TAG(E,N,V,M);

#ifdef _DEBUG
struct ShaderPrimitiveSorter::DebugPhase
{
	char *name;
	bool  enabled;
}; 
#endif

class ShaderPrimitiveSorter::Phase
{
public:

	struct Entry
	{
		const StaticShader     *staticShader;
		const ShaderPrimitive  *shaderPrimitive;
		bool                    alphaFadeOpacityEnabled;
		float                   alphaFadeOpacity;

		int                     shaderImplementationSortKey;
		int                     shaderTemplateSortKey;
		int                     vertexBufferSortKey;
		int                     textureSortKey;
		real                    depthSortKey;
		LightBitSet             lightBitSet;
	};

public:

	static void install();

public:

	Phase();
	~Phase();

#ifdef _DEBUG
	void  setDrawEnable(bool enabled);
	bool  getDrawEnable() const;
	void  clearFrameDrawTime();
	float getFrameDrawTime() const;
#endif

	void setSort(Sort sort);
	void setDrawTime(DrawTime drawTime);
	void setPhaseDefaultAlphaFadeOpacity(float defaultAlphaFadeOpacity);

	void add(const ShaderPrimitive &shaderPrimitive, const StaticShader &staticShader, const LightBitSet &lightBitVector);
	void addWithAlphaFadeOpacity(const ShaderPrimitive &shaderPrimitive, const StaticShader &staticShader, float alphaFadeOpacity, const LightBitSet &lightBitVector);

	void sort();
	void draw();
	void clear();

	void pushCell();
	void popCell();

public:

	typedef void (ShaderPrimitiveSorter::Phase::*SortFunction)();
	typedef void (*GetSortKeysFunction)(Entry &);

private:

	typedef std::vector<Entry> ShaderPrimitives;

private:

	void sort_unknown();
	void sort_fifo();
	void sort_shaderImplementation_shaderTemplate_vertexBuffer_texture();
	void sort_texture_vertexBuffer();
	void sort_vertexBuffer_texture();
	void sort_z();

	void add(const ShaderPrimitive &shaderPrimitive, const StaticShader &staticShader, bool alphaFadeEnabled, float alphaFadeOpacity, const LightBitSet &lightBitVector);

	static void getSortKeys_unknown(Entry &);
	static void getSortKeys_none(Entry &);
	static void getSortKeys_depth(Entry &);
	static void getSortKeys_shaderImplementation_shaderTemplate_texture_vertexBuffer(Entry &);

private:

	static SortFunction  ms_sortPerformance;

#if PRODUCTION == 0
	static bool          ms_profileByType;
	static bool          ms_profilePrepareToDraw;
	static bool          ms_profileDraw;
	static ProfilerBlock ms_profilerBlockPrepareToDraw;
	static ProfilerBlock ms_profilerBlockDraw;
	static ProfilerBlock ms_profilerBlockCompositeStart;
	static ProfilerBlock ms_profilerBlockCompositeFinish;
#endif

private:

#ifdef _DEBUG
	bool                             m_drawEnable;
	float                            m_frameDrawTime;
#endif

	DrawTime                         m_drawTime;
	SortFunction                     m_sort;
	GetSortKeysFunction              m_getSortKeys;
	ShaderPrimitives                 m_shaderPrimitives;
	std::vector<int>                 m_stackOffsets;
};

// ======================================================================

namespace ShaderPrimitiveSorterNamespace
{
	struct Sort_ShaderImplementation_ShaderTemplate_Texture_VertexBuffer
	{
		bool operator()(const ShaderPrimitiveSorter::Phase::Entry &lhs, const ShaderPrimitiveSorter::Phase::Entry &rhs) const;
	};

	struct Sort_ShaderImplementation_ShaderTemplate_VertexBuffer_Texture
	{
		bool operator()(const ShaderPrimitiveSorter::Phase::Entry &lhs, const ShaderPrimitiveSorter::Phase::Entry &rhs) const;
	};

	struct Sort_Texture_VertexBuffer
	{
		bool operator()(const ShaderPrimitiveSorter::Phase::Entry &lhs, const ShaderPrimitiveSorter::Phase::Entry &rhs) const;
	};

	struct Sort_VertexBuffer_Texture
	{
		bool operator()(const ShaderPrimitiveSorter::Phase::Entry &lhs, const ShaderPrimitiveSorter::Phase::Entry &rhs) const;
	};

	struct Sort_Depth
	{
		bool operator()(const ShaderPrimitiveSorter::Phase::Entry &lhs, const ShaderPrimitiveSorter::Phase::Entry &rhs) const;
	};

	bool ms_showDebugHeatShaders = false;
	bool ms_debugDisableHeatShaders = false;
	bool ms_showDebugHeatShaderRects = false;

	int ms_pixelsCompositedThisFrame = 0;
	int ms_compositePasses = 0;
	bool ms_reportPixelsComposited = false;
	
	//----------------------------------------------------------------------

}
using namespace ShaderPrimitiveSorterNamespace;

inline bool Sort_ShaderImplementation_ShaderTemplate_Texture_VertexBuffer::operator()(const ShaderPrimitiveSorter::Phase::Entry &lhs, const ShaderPrimitiveSorter::Phase::Entry &rhs) const
{
	if (lhs.shaderImplementationSortKey < rhs.shaderImplementationSortKey)
		return true;
	if (lhs.shaderImplementationSortKey > rhs.shaderImplementationSortKey)
		return false;

	if (lhs.shaderTemplateSortKey < rhs.shaderTemplateSortKey)
		return true;
	if (lhs.shaderTemplateSortKey > rhs.shaderTemplateSortKey)
		return false;

	if (lhs.textureSortKey < rhs.textureSortKey)
		return true;
	if (lhs.textureSortKey > rhs.textureSortKey)
		return false;

	return (lhs.vertexBufferSortKey < rhs.vertexBufferSortKey);
}

//----------------------------------------------------------------------

inline bool Sort_ShaderImplementation_ShaderTemplate_VertexBuffer_Texture::operator()(const ShaderPrimitiveSorter::Phase::Entry &lhs, const ShaderPrimitiveSorter::Phase::Entry &rhs) const
{
	if (lhs.shaderImplementationSortKey < rhs.shaderImplementationSortKey)
		return true;
	if (lhs.shaderImplementationSortKey > rhs.shaderImplementationSortKey)
		return false;

	if (lhs.shaderTemplateSortKey < rhs.shaderTemplateSortKey)
		return true;
	if (lhs.shaderTemplateSortKey > rhs.shaderTemplateSortKey)
		return false;

	if (lhs.vertexBufferSortKey < rhs.vertexBufferSortKey)
		return true;
	if (lhs.vertexBufferSortKey > rhs.vertexBufferSortKey)
		return false;

	return (lhs.textureSortKey < rhs.textureSortKey);
}

//----------------------------------------------------------------------

inline bool Sort_Texture_VertexBuffer::operator()(const ShaderPrimitiveSorter::Phase::Entry &lhs, const ShaderPrimitiveSorter::Phase::Entry &rhs) const
{
	if (lhs.textureSortKey < rhs.textureSortKey)
		return true;
	if (lhs.textureSortKey > rhs.textureSortKey)
		return false;
		
	return (lhs.vertexBufferSortKey < rhs.vertexBufferSortKey);
}

//----------------------------------------------------------------------

inline bool Sort_VertexBuffer_Texture::operator()(const ShaderPrimitiveSorter::Phase::Entry &lhs, const ShaderPrimitiveSorter::Phase::Entry &rhs) const
{
	if (lhs.vertexBufferSortKey < rhs.vertexBufferSortKey)
		return true;
	if (lhs.vertexBufferSortKey > rhs.vertexBufferSortKey)
		return false;
		
	return (lhs.textureSortKey < rhs.textureSortKey);
}

//----------------------------------------------------------------------

inline bool Sort_Depth::operator()(const ShaderPrimitiveSorter::Phase::Entry &lhs, const ShaderPrimitiveSorter::Phase::Entry &rhs) const
{
	return (lhs.depthSortKey > rhs.depthSortKey);
}

// ======================================================================

ShaderPrimitiveSorter::Phase::SortFunction ShaderPrimitiveSorter::Phase::ms_sortPerformance = NULL;
#if PRODUCTION == 0
bool                                       ShaderPrimitiveSorter::Phase::ms_profileByType;
bool                                       ShaderPrimitiveSorter::Phase::ms_profilePrepareToDraw;
bool                                       ShaderPrimitiveSorter::Phase::ms_profileDraw;
ProfilerBlock                              ShaderPrimitiveSorter::Phase::ms_profilerBlockPrepareToDraw("prepareToDraw");
ProfilerBlock                              ShaderPrimitiveSorter::Phase::ms_profilerBlockDraw("draw");
ProfilerBlock ShaderPrimitiveSorter::Phase::ms_profilerBlockCompositeStart("compositeStart");
ProfilerBlock ShaderPrimitiveSorter::Phase::ms_profilerBlockCompositeFinish("compositeFinish");

#endif

// ======================================================================

void ShaderPrimitiveSorter::Phase::install()
{
	if (ms_sortPerformance == NULL)
		ms_sortPerformance = &sort_shaderImplementation_shaderTemplate_vertexBuffer_texture;

#if PRODUCTION == 0
	
	DebugFlags::registerFlag(ms_debugDisableHeatShaders, "ClientGraphics/ShaderPrimitiveSorter", "debugDisableHeatShaders");
	DebugFlags::registerFlag(ms_showDebugHeatShaders, "ClientGraphics/ShaderPrimitiveSorter", "showDebugHeatShaders");
	DebugFlags::registerFlag(ms_showDebugHeatShaderRects, "ClientGraphics/ShaderPrimitiveSorter", "showDebugHeatShaderRects");
	DebugFlags::registerFlag(ms_profileByType, "ClientGraphics/ShaderPrimitiveSorter", "profileByType");
	DebugFlags::registerFlag(ms_profilePrepareToDraw, "ClientGraphics/ShaderPrimitiveSorter", "profilePrepareToDraw");
	DebugFlags::registerFlag(ms_profileDraw, "ClientGraphics/ShaderPrimitiveSorter", "profileDraw");
#endif
}

// ----------------------------------------------------------------------

ShaderPrimitiveSorter::Phase::Phase()
:
#ifdef _DEBUG
	m_drawEnable(true),
	m_frameDrawTime(0.f),
#endif
	m_drawTime(D_unknown),
	m_sort(&Phase::sort_unknown),
	m_getSortKeys(getSortKeys_unknown),
	m_shaderPrimitives(),
	m_stackOffsets()
{
	m_stackOffsets.push_back(0);
}

// ----------------------------------------------------------------------

ShaderPrimitiveSorter::Phase::~Phase()
{
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void ShaderPrimitiveSorter::Phase::setDrawEnable(bool enable)
{
	m_drawEnable = enable;
}

// ----------------------------------------------------------------------

bool ShaderPrimitiveSorter::Phase::getDrawEnable() const
{
	return m_drawEnable;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::clearFrameDrawTime()
{
	m_frameDrawTime = 0.f;
}

// ----------------------------------------------------------------------

float ShaderPrimitiveSorter::Phase::getFrameDrawTime() const
{
	return m_frameDrawTime;
}

#endif

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::setSort(Sort sort)
{
	// store a pointer to the sort routine to avoid switching within sort()
	switch (sort)
	{
		case S_unknown:
			m_sort = &Phase::sort_unknown;
			m_getSortKeys = getSortKeys_unknown;
			break;

		case S_fifo:
			m_sort = &Phase::sort_fifo;
			m_getSortKeys = getSortKeys_none;
			break;

		case S_performance:
			m_sort = ms_sortPerformance;
			m_getSortKeys = getSortKeys_shaderImplementation_shaderTemplate_texture_vertexBuffer;
			break;

		case S_painters:
			m_sort = &Phase::sort_z;
			m_getSortKeys = getSortKeys_depth;
			break;

		default:
			DEBUG_FATAL(true, ("Invalid sort"));
			break;
	}
}

// ----------------------------------------------------------------------

inline void ShaderPrimitiveSorter::Phase::setDrawTime(DrawTime drawTime)
{
	m_drawTime = drawTime;
}

// ----------------------------------------------------------------------

inline void ShaderPrimitiveSorter::Phase::add(const ShaderPrimitive &shaderPrimitive, const StaticShader &staticShader, bool alphaFadeOpacityEnabled, float alphaFadeOpacity, const LightBitSet &lightBitSet)
{
	// @todo this should initialize using a constructor
	Entry entry;
	entry.shaderPrimitive = &shaderPrimitive;		
	entry.staticShader = &staticShader;
	entry.alphaFadeOpacityEnabled = alphaFadeOpacityEnabled;
	entry.alphaFadeOpacity = alphaFadeOpacity;
	entry.lightBitSet = lightBitSet;
	(*m_getSortKeys)(entry);
	m_shaderPrimitives.push_back(entry);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::add(const ShaderPrimitive &shaderPrimitive, const StaticShader &staticShader, const LightBitSet &lightBitSet)
{
	add(shaderPrimitive, staticShader, false, staticShader.getShaderTemplate().isOpaqueSolid() ? 0.0f : 1.0f, lightBitSet);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::addWithAlphaFadeOpacity(const ShaderPrimitive &shaderPrimitive, const StaticShader &staticShader, float alphaFadeOpacity, const LightBitSet &lightBitSet)
{
	add(shaderPrimitive, staticShader, true, alphaFadeOpacity, lightBitSet);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::sort_unknown()
{
	DEBUG_FATAL(true, ("unknown sort order"));
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::sort_fifo()
{
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::sort_shaderImplementation_shaderTemplate_vertexBuffer_texture()
{
	std::sort(m_shaderPrimitives.begin()  + m_stackOffsets.back(), m_shaderPrimitives.end(), Sort_ShaderImplementation_ShaderTemplate_VertexBuffer_Texture());
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::sort_texture_vertexBuffer()
{
	std::sort(m_shaderPrimitives.begin() + m_stackOffsets.back(), m_shaderPrimitives.end(), Sort_Texture_VertexBuffer());
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::sort_vertexBuffer_texture()
{
	std::sort(m_shaderPrimitives.begin()  + m_stackOffsets.back(), m_shaderPrimitives.end(), Sort_VertexBuffer_Texture());
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::sort_z()
{
	std::stable_sort(m_shaderPrimitives.begin() + m_stackOffsets.back(), m_shaderPrimitives.end(), Sort_Depth());
}

// ----------------------------------------------------------------------

inline void ShaderPrimitiveSorter::Phase::sort()
{
	(this->*m_sort)();
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::getSortKeys_unknown(Entry &)
{
	DEBUG_FATAL(true, ("unknown sort order"));
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::getSortKeys_none(Entry &)
{
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::getSortKeys_depth(Entry &entry)
{
	entry.depthSortKey = entry.shaderPrimitive->getDepthSquaredSortKey();
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::getSortKeys_shaderImplementation_shaderTemplate_texture_vertexBuffer(Entry &entry)
{
	entry.shaderImplementationSortKey  = entry.staticShader->getShaderImplementationSortKey();
	entry.shaderTemplateSortKey        = entry.staticShader->getShaderTemplateSortKey();
	entry.vertexBufferSortKey          = entry.shaderPrimitive->getVertexBufferSortKey();
	entry.textureSortKey               = entry.staticShader->getTextureSortKey();
}

//----------------------------------------------------------------------

void ShaderPrimitiveSorter::finishCompositing(int sectionRectX0, int sectionRectY0, int sectionRectX1, int sectionRectY1, StaticShader & compositingShader)
{
	Graphics::setScissorRect(false, sectionRectX0, sectionRectY0, sectionRectX1 - sectionRectX0, sectionRectY1 - sectionRectY0);

	Texture * const primaryBuffer = PostProcessingEffectsManager::getPrimaryBuffer();
	Texture * const secondaryBuffer = PostProcessingEffectsManager::getSecondaryBuffer();
	Texture * const heatBuffer = PostProcessingEffectsManager::getTertiaryBuffer();

	if (ms_showDebugHeatShaders || ms_debugDisableHeatShaders || !ms_heatShadersEnabled || !heatBuffer || !primaryBuffer)
		return;

	int const sectionRectWidth = sectionRectX1 - sectionRectX0;
	int const sectionRectHeight = sectionRectY1 - sectionRectY0;

	ms_pixelsCompositedThisFrame += sectionRectWidth * sectionRectHeight;
	++ms_compositePasses;

	//-- create vertex buffer used for compositing
	VertexBufferFormat format;
	format.setPosition ();
	format.setTransformed ();
	format.setNumberOfTextureCoordinateSets(1);
	format.setTextureCoordinateSetDimension(0, 2);
	DynamicVertexBuffer vertexBuffer (format);
	
	int const destinationWidth = primaryBuffer->getWidth();
	int const destinationHeight = primaryBuffer->getHeight();
	float const fWidth = static_cast<float>(destinationWidth);
	float const fHeight = static_cast<float>(destinationHeight);
	
	Rectangle2d const rect(static_cast<float>(sectionRectX0), static_cast<float>(sectionRectY0), static_cast<float>(sectionRectX1), static_cast<float>(sectionRectY1));
	Rectangle2d const uvs(rect.x0 / fWidth, rect.y0 / fHeight, rect.x1 / fWidth, rect.y1 / fHeight);
	
	vertexBuffer.lock (4);
	{
		VertexBufferWriteIterator v = vertexBuffer.begin ();
		
		v.setPosition (Vector (rect.x0 - 0.5f, rect.y0 - 0.5f, 1.f));
		v.setOoz (1.f);
		v.setTextureCoordinates(0, uvs.x0, uvs.y0);
		++v;
		
		v.setPosition (Vector (rect.x1 - 0.5f, rect.y0 - 0.5f, 1.f));
		v.setOoz (1.f);
		v.setTextureCoordinates(0, uvs.x1, uvs.y0);
		++v;
		
		v.setPosition (Vector (rect.x1 - 0.5f, rect.y1 - 0.5f, 1.f));
		v.setOoz (1.f);
		v.setTextureCoordinates(0, uvs.x1, uvs.y1);
		++v;
		
		v.setPosition (Vector (rect.x0 - 0.5f, rect.y1 - 0.5f, 1.f));
		v.setOoz (1.f);
		v.setTextureCoordinates(0, uvs.x0, uvs.y1);
	}
	vertexBuffer.unlock ();
	
	//-- temporarily disable FOG
	bool fogEnabled = false;
	float fogDensity = 0.0f;
	PackedArgb fogColor;
	Graphics::getFog(fogEnabled, fogDensity, fogColor);
	Graphics::setFog(false, fogDensity, fogColor);

	//-- copy the original region
	secondaryBuffer->copyFrom(0, *primaryBuffer, 
		sectionRectX0, sectionRectY0, sectionRectWidth, sectionRectHeight, 
		sectionRectX0, sectionRectY0, sectionRectWidth, sectionRectHeight);

	Graphics::setRenderTarget(primaryBuffer, CF_none, 0);
	compositingShader.setTexture(TAG(M,A,I,N), *secondaryBuffer);
	compositingShader.setTexture(TAG(H,E,A,T), *heatBuffer);
	Graphics::setStaticShader(compositingShader);
	VectorRgba const pixelShaderUserConstants(ms_elapsedTime, ms_showDebugHeatShaderRects ? 1.0f : 0.0f, 0.0f, 0.0f);
	Graphics::setPixelShaderUserConstants(&pixelShaderUserConstants, 1);
	Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
	Graphics::setVertexBuffer(vertexBuffer);

	Graphics::drawTriangleFan();
			
	//-- re-enable the fog
	Graphics::setFog(fogEnabled, fogDensity, fogColor);
	
}

//----------------------------------------------------------------------

bool ShaderPrimitiveSorter::startCompositing(ShaderPrimitive const & shaderPrimitive, int & sectionRectX0, int & sectionRectY0, int & sectionRectX1, int & sectionRectY1)
{
	static int const s_minimumCompositeArea = 200;
	static int const s_maximumCompositeDistance = 100;

	Texture * const primaryBuffer = PostProcessingEffectsManager::getPrimaryBuffer();
	Texture * const heatBuffer = PostProcessingEffectsManager::getTertiaryBuffer();
	
	if (!heatBuffer || !primaryBuffer)
		return false;
	
	//-- if we are showing debug heat shaders, just render them normally
	if (ms_showDebugHeatShaders)
		return true;
	
	//-- don't do any special heat rendering
	if (ms_debugDisableHeatShaders)
		return false;				
	
	int sectionRectWidth = sectionRectX1 - sectionRectX0;
	int sectionRectHeight = sectionRectY1 - sectionRectY0;

	//Sphere const cameraCompositeDistanceSphere_c(Vector::zero, s_maximumCompositeDistance);

	//-- calculate the screen size of the primitive, so we can render as little as possible
	Vector screenPos;				
	Vector const shaderPrimitivePos_c = ms_currentCamera->rotateTranslate_p2o(shaderPrimitive.getPosition_w());
	
	//-- The various shader primitives need to implement radius for this to work.
	//-- If you are having problems with heat shimmer not appearing, make sure your shader primitive has a radius
	float shaderPrimitiveRadius = shaderPrimitive.getRadius();

	if ((shaderPrimitivePos_c.magnitude() - shaderPrimitiveRadius) > s_maximumCompositeDistance)
		return false;
	
	float screenRadiusInPixels = 0.0f;
	ms_currentCamera->computeRadiusInScreenSpace(shaderPrimitivePos_c, shaderPrimitiveRadius, screenRadiusInPixels);

	int const destinationWidth = primaryBuffer->getWidth();
	int const destinationHeight = primaryBuffer->getHeight();
	
	//-- inside the sphere
	if (ms_currentCamera->getPosition_w().magnitudeBetweenSquared(shaderPrimitive.getPosition_w()) < sqr(shaderPrimitiveRadius))
	{
		sectionRectX0 = 0;
		sectionRectY0 = 0;
		sectionRectX1 = destinationWidth;
		sectionRectY1 = destinationHeight;
	}	
	
	else if (ms_currentCamera->projectInWorldSpace(shaderPrimitive.getPosition_w(), &screenPos.x, &screenPos.y, NULL, false))
	{
		screenPos.x = clamp(0.0f, screenPos.x, static_cast<float>(destinationWidth));
		screenPos.y = clamp(0.0f, screenPos.y, static_cast<float>(destinationHeight));
		sectionRectX0 = clamp(0, static_cast<int>(screenPos.x - screenRadiusInPixels), destinationWidth);
		sectionRectY0 = clamp(0, static_cast<int>(screenPos.y - screenRadiusInPixels), destinationHeight);
		sectionRectX1 = clamp(0, static_cast<int>(screenPos.x + screenRadiusInPixels), destinationWidth);
		sectionRectY1 = clamp(0, static_cast<int>(screenPos.y + screenRadiusInPixels), destinationHeight);
	}

	sectionRectWidth = sectionRectX1 - sectionRectX0;
	sectionRectHeight = sectionRectY1 - sectionRectY0;

	/*

	//-- squash rects for visualization
	sectionRectX0 += sectionRectWidth / 4;
	sectionRectX1 -= sectionRectWidth / 4;
	sectionRectY0 += sectionRectHeight / 4;
	sectionRectY1 -= sectionRectHeight / 4;

	sectionRectWidth = sectionRectX1 - sectionRectX0;
	sectionRectHeight = sectionRectY1 - sectionRectY0;
*/

	if (sectionRectWidth == 0 || sectionRectHeight == 0)
		return false;		

	if (sectionRectWidth * sectionRectHeight < s_minimumCompositeArea)
		return false;
	
	//-- setup the heat buffer render
	Graphics::setRenderTarget(heatBuffer, CF_none, 0);
	Graphics::setViewport(0, 0, destinationWidth, destinationHeight, 0.0f, 1.0f);
	
	Graphics::setScissorRect(true, sectionRectX0, sectionRectY0, sectionRectWidth, sectionRectHeight);
	Graphics::setViewport(sectionRectX0, sectionRectY0, sectionRectWidth, sectionRectHeight, 0.0f, 1.0f);
	Graphics::clearViewport(true, 0, false, 1.0f, false, 0);
	Graphics::setViewport(0, 0, destinationWidth, destinationHeight, 0.0f, 1.0f);
	
	//-- don't use alpha fade when rendering heat objects
	Graphics::setAlphaFadeOpacity(false, 1.0f);

	return true;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::draw()
{
#ifdef _DEBUG
	if (!m_drawEnable)
		return;

	PerformanceTimer performanceTimer;
	performanceTimer.start();
#endif

	NP_PROFILER_AUTO_BLOCK_DEFINE("ShaderPrimitiveSorter::Phase::draw");

	ShaderPrimitives::iterator end = m_shaderPrimitives.end();
	for (ShaderPrimitives::iterator i = m_shaderPrimitives.begin() + m_stackOffsets.back(); i != end; ++i)
	{
		Entry const           &entry           = *i;
		StaticShader const    &staticShader    = *entry.staticShader;
		ShaderPrimitive const &shaderPrimitive = *entry.shaderPrimitive;
		bool const alphaFadeOpacityEnabled     = entry.alphaFadeOpacityEnabled;
		float const alphaFadeOpacity           = entry.alphaFadeOpacity;
		int const              numberOfPasses  = staticShader.getNumberOfPasses();

#if PRODUCTION == 0
		char const * const typeName = ms_profileByType ? typeid(shaderPrimitive).name() : NULL;
		NP_PROFILER_BLOCK_DEFINE(profilerBlockByTime, typeName);
		if (typeName)
			NP_PROFILER_BLOCK_ENTER(profilerBlockByTime);
#endif

		ShaderPrimitiveSorter::setLights(i->lightBitSet);

#if PRODUCTION == 0
		if (ms_profilePrepareToDraw)
			NP_PROFILER_BLOCK_ENTER(ms_profilerBlockPrepareToDraw);
#endif

		shaderPrimitive.prepareToDraw();

#if PRODUCTION == 0
		if (ms_profilePrepareToDraw)
			NP_PROFILER_BLOCK_LEAVE(ms_profilerBlockPrepareToDraw);
#endif

		Texture * const primaryBuffer = PostProcessingEffectsManager::getPrimaryBuffer();

		int const destinationWidth = primaryBuffer ? primaryBuffer->getWidth() : 0;
		int const destinationHeight = primaryBuffer ? primaryBuffer->getHeight() : 0;

		int sectionRectX0 = 0;
		int sectionRectY0 = 0;
		int sectionRectX1 = destinationWidth;
		int sectionRectY1 = destinationHeight;
		
		Graphics::setAlphaFadeOpacity(alphaFadeOpacityEnabled, alphaFadeOpacity);
		
		for (int i = 0; i < numberOfPasses; ++i)
		{
			Graphics::setStaticShader(staticShader, i);
						
			bool const isHeat = entry.staticShader->isHeatPass(i);
			
			if (isHeat)
			{
				if (!ms_heatShadersEnabled)
				{
					continue;
				}
				
#if PRODUCTION == 0
				if (ms_profileDraw)
					NP_PROFILER_BLOCK_ENTER(ms_profilerBlockCompositeStart);
#endif
				
				bool const result = startCompositing(shaderPrimitive, sectionRectX0, sectionRectY0, sectionRectX1, sectionRectY1);
				
#if PRODUCTION == 0
				if (ms_profileDraw)
					NP_PROFILER_BLOCK_LEAVE(ms_profilerBlockCompositeStart);
#endif
				
				if (!result)
					continue;

				Graphics::setAlphaFadeOpacity(false, alphaFadeOpacity);
			}
			
			
#if PRODUCTION == 0
			if (ms_profileDraw)
				NP_PROFILER_BLOCK_ENTER(ms_profilerBlockDraw);
#endif

			shaderPrimitive.draw();
			
#if PRODUCTION == 0
			if (ms_profileDraw)
				NP_PROFILER_BLOCK_LEAVE(ms_profilerBlockDraw);
#endif

			if (isHeat)
			{
				//-- @todo: this should probably be specified by the shader primitive's shader
				StaticShader * const compositingShader = PostProcessingEffectsManager::getHeatCompositingShader();
				
				if (compositingShader)
				{

#if PRODUCTION == 0
				if (ms_profileDraw)
					NP_PROFILER_BLOCK_ENTER(ms_profilerBlockCompositeFinish);
#endif
					
					finishCompositing(sectionRectX0, sectionRectY0, sectionRectX1, sectionRectY1, *compositingShader);

#if PRODUCTION == 0
				if (ms_profileDraw)
					NP_PROFILER_BLOCK_LEAVE(ms_profilerBlockCompositeFinish);
#endif

				}
				
			}
		}
		
#if PRODUCTION == 0
		if (typeName)
			NP_PROFILER_BLOCK_LEAVE(profilerBlockByTime);
#endif
	}

	Graphics::setAlphaFadeOpacity(false, 1.0f);

#ifdef _DEBUG
	performanceTimer.stop();
	m_frameDrawTime += performanceTimer.getElapsedTime ();
#endif
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::clear()
{
	ShaderPrimitives::iterator start = m_shaderPrimitives.begin() + m_stackOffsets.back();
	ShaderPrimitives::iterator end = m_shaderPrimitives.end();

	if (start != end)	
		m_shaderPrimitives.erase(start, end);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::pushCell()
{
	DEBUG_FATAL(m_drawTime == D_unknown, ("Phase draw time not set"));

	if (m_drawTime == D_beforeNextPush)
	{
		sort();
		draw();
		clear();
	}

	m_stackOffsets.push_back(m_shaderPrimitives.size());
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::Phase::popCell()
{
	sort();
	draw();
	clear();

	m_stackOffsets.pop_back();
}

// ======================================================================

namespace ShaderPrimitiveSorterNamespace
{
	struct FogData
	{
		bool       enabled;
		float      density;
		PackedArgb color;

		FogData(bool enabled, float density, PackedArgb const & color);
	};

	stdvector<FogData>::fwd                                               ms_fogStack;
	stdvector<CellProperty const *>::fwd                                  ms_cellPropertyStack;
	bool                                                                  ms_pushed;
};

using namespace ShaderPrimitiveSorterNamespace;

ShaderPrimitiveSorter::Phases                    ShaderPrimitiveSorter::ms_phase;
ShaderPrimitiveSorter::PhaseMap                  ShaderPrimitiveSorter::ms_phaseMap;
const Camera                                    *ShaderPrimitiveSorter::ms_currentCamera;
Vector                                           ShaderPrimitiveSorter::ms_currentCameraPosition;
ShaderPrimitiveSorter::LightList                 ShaderPrimitiveSorter::ms_lightList;
Texture const                                   *ShaderPrimitiveSorter::ms_defaultEnvironmentTexture;
ShaderPrimitiveSorter::EnvironmentTextures       ShaderPrimitiveSorter::ms_environmentTextures;

ShaderPrimitiveSorter::LightList                 ShaderPrimitiveSorter::ms_activeLightList;
ShaderPrimitiveSorter::LightBitSet               ShaderPrimitiveSorter::ms_lightsAffectingShadersWithoutPrecalculatedVertexLighting;
ShaderPrimitiveSorter::LightBitSet               ShaderPrimitiveSorter::ms_lightsAffectingShadersWithPrecalculatedVertexLighting;
stdvector<int>::fwd                              ShaderPrimitiveSorter::ms_lightListStackOffset;

ShaderPrimitiveSorter::PrepareToViewOverrideFunction ShaderPrimitiveSorter::ms_prepareToViewFunction;

ShaderPrimitiveSorter::BelowTransparentWaterFunction ShaderPrimitiveSorter::ms_belowTransparentWaterFunction;
int                                              ShaderPrimitiveSorter::ms_belowTransparentWaterTestPhase;
int                                              ShaderPrimitiveSorter::ms_belowTransparentWaterPhase;
int                                              ShaderPrimitiveSorter::ms_aboveWaterPhase;

bool                                             ShaderPrimitiveSorter::ms_popped = true;
float ShaderPrimitiveSorter::ms_elapsedTime = 0.0f;
bool ShaderPrimitiveSorter::ms_heatShadersEnabled = true;

Rectangle2d ShaderPrimitiveSorter::ms_clipRectangle;
bool ShaderPrimitiveSorter::ms_useClipRectangle = false;
int ShaderPrimitiveSorter::ms_clipEnvironmentFlags = 0xffff;
bool ShaderPrimitiveSorter::ms_useWaterTests = true;

#ifdef _DEBUG
bool                                             ShaderPrimitiveSorter::ms_lightListOverflow;
bool                                             ShaderPrimitiveSorter::ms_debugReport;
ShaderPrimitiveSorter::DebugPhaseList            ShaderPrimitiveSorter::ms_debugPhase;
#endif

// ======================================================================

ShaderPrimitiveSorterNamespace::FogData::FogData(bool iEnabled, float iDensity, PackedArgb const & iColor)
:
	enabled(iEnabled),
	density(iDensity),
	color(iColor)
{
}

// ======================================================================

void ShaderPrimitiveSorter::install(int numberOfPhases)
{
	DEBUG_FATAL(numberOfPhases<= 0, ("numberOfPhases must be positive"));
	ms_phase.resize(numberOfPhases);
	Phase::install();
	ms_lightListStackOffset.push_back(0);

	ms_defaultEnvironmentTexture = TextureList::fetchDefaultEnvironmentTexture();
	ms_environmentTextures.push_back(ms_defaultEnvironmentTexture);
	ms_fogStack.push_back(FogData(false, 0.0f, PackedArgb::solidWhite));

	ms_prepareToViewFunction = defaultPrepareToView;

	LocalMachineOptionManager::registerOption(ms_heatShadersEnabled, "ClientGraphics/ShaderPrimitiveSorter", "heatShadersEnabled");

#ifdef _DEBUG
	DebugFlags::registerFlag(ms_debugReport, "ClientGraphics/ShaderPrimitiveSorter", "debugReport", debugDump);
	DebugFlags::registerFlag(ms_reportPixelsComposited, "ClientGraphics/ShaderPrimitiveSorter", "reportPixelsComposited");

	ms_debugPhase.resize(numberOfPhases);
	for (int i = 0; i < numberOfPhases; ++i)
	{
		char buffer[256];
		sprintf(buffer, "Phase%d Enabled", i);
		ms_debugPhase[i].name = DuplicateString(buffer);
		ms_debugPhase[i].enabled = true;
	}
#endif

	ms_heatShadersEnabled = ms_heatShadersEnabled && getHeatShadersCapable();

	ExitChain::add(ShaderPrimitiveSorter::remove, "ShaderPrimitiveSorter::remove");
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::remove()
{
#ifdef _DEBUG
	for (uint i = 0; i < ms_debugPhase.size(); ++i)
		delete [] ms_debugPhase[i].name;
#endif

	DEBUG_FATAL(ms_environmentTextures.size() != 1, ("environment textures still on stack"));
	ms_environmentTextures.clear();

	ms_defaultEnvironmentTexture->release();
	ms_defaultEnvironmentTexture = NULL;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::setPhase(Tag tag, int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(ms_phase.size()));

	PhaseMap::iterator i = ms_phaseMap.find(tag);
	if (i != ms_phaseMap.end())
	{
		// the the new phase index
		i->second = index;
	}
	else
	{
		// insert the new descriptor
		PhaseMap::value_type entry(tag, index);
		std::pair<PhaseMap::iterator, bool> result = ms_phaseMap.insert(entry);
		DEBUG_FATAL(!result.second, ("insert() said entry was already there, but find() didn't locate it"));
	}
}

// ----------------------------------------------------------------------

int ShaderPrimitiveSorter::getPhase(Tag tag)
{
	DEBUG_FATAL(!isInstalled(), ("not installed"));
	PhaseMap::iterator i = ms_phaseMap.find(tag);
	DEBUG_FATAL(i == ms_phaseMap.end(), ("Tag not found in phase map"));
	return i->second;
}

// ----------------------------------------------------------------------
/**
 * Set the sort for a particular phase.
 *
 * This routine allows the user to control the sort on a phase-by-phase basis.
 * Some graphics can be reordered in any manner for best rendering performance,
 * while other graphics must be draw back-to-front for correct visuals.  Other
 * art should be draw in the order requested (for example, a user interface).
 * 
 * @param phase The phase for which to set the sort.
 * @param sort  The new sort for the phase.
 */

void ShaderPrimitiveSorter::setPhaseSort(int phase, Sort sort)
{
	DEBUG_FATAL(phase < 0 || phase >= static_cast<int>(ms_phase.size()), ("Invalid phase %d/%d", ms_phase.size()));
	ms_phase[phase].setSort(sort);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::setPhaseDrawTime(int phase, DrawTime drawTime)
{
	DEBUG_FATAL(phase < 0 || phase >= static_cast<int>(ms_phase.size()), ("Invalid phase %d/%d", ms_phase.size()));
	ms_phase[phase].setDrawTime(drawTime);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void ShaderPrimitiveSorter::setPhaseDebugName(int phase, const char *debugName)
{
	DEBUG_FATAL(phase < 0 || phase >= static_cast<int>(ms_phase.size()), ("Invalid phase %d/%d", ms_phase.size()));
	if (ms_debugPhase[phase].name)
		delete [] ms_debugPhase[phase].name;

	ms_debugPhase[phase].name = DuplicateString(debugName);
	DebugFlags::registerFlag(ms_debugPhase[phase].enabled, "ClientGraphics/ShaderPrimitiveSorter", ms_debugPhase[phase].name);
}

#endif

// ----------------------------------------------------------------------

bool ShaderPrimitiveSorter::isInstalled()
{
	return !ms_phase.empty();
}

// ----------------------------------------------------------------------

void  ShaderPrimitiveSorter::setCurrentCamera(const Camera &camera)
{
	ms_currentCamera = &camera;
	ms_currentCameraPosition = camera.getPosition_w();
}

// ----------------------------------------------------------------------

void  ShaderPrimitiveSorter::clearCurrentCamera()
{
	ms_currentCamera = NULL;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::enableLight(const Light &light)
{
	DEBUG_FATAL(ms_popped, ("Enabling a light after cell popped"));

	int index = light.getShaderPrimitiveSorterLightIndex();
	if (index < 0)
	{
		index = static_cast<int>(ms_lightList.size() - ms_lightListStackOffset.back());
		if (index >= MAX_NUMBER_OF_LIGHTS)
		{
#ifdef _DEBUG
			WARNING(!ms_lightListOverflow, ("too many lights"));
			ms_lightListOverflow = true;
#endif
			return;
		}
		
		ms_lightList.push_back(&light);
		light.setShaderPrimitiveSorterLightIndex(index);
	}

	DEBUG_FATAL(index >= static_cast<int>(ms_lightList.size() - ms_lightListStackOffset.back()), ("Light index out of range"));

	if (light.affectsShadersWithoutPrecalculatedVertexLighting())
		ms_lightsAffectingShadersWithoutPrecalculatedVertexLighting.set(static_cast<size_t>(index));
	if (light.affectsShadersWithPrecalculatedVertexLighting())
		ms_lightsAffectingShadersWithPrecalculatedVertexLighting.set(static_cast<size_t>(index));
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::disableLight(const Light &light)
{
	DEBUG_FATAL(ms_popped, ("Disabling a light after cell popped"));

	const int index = light.getShaderPrimitiveSorterLightIndex();
	if (index >= 0)
	{
		DEBUG_FATAL(index >= static_cast<int>(ms_lightList.size() - ms_lightListStackOffset.back()), ("Light index out of range"));
		ms_lightsAffectingShadersWithoutPrecalculatedVertexLighting.reset(static_cast<size_t>(index));
		ms_lightsAffectingShadersWithPrecalculatedVertexLighting.reset(static_cast<size_t>(index));
	}
	else
	{
		DEBUG_FATAL(!ms_lightListOverflow, ("Disabling a light that was never enabled"));
	}
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::setLights(const LightBitSet &lightBitSet)
{
	ms_activeLightList.clear();	

	// check all the lights to see which ones are currently active
	const uint offset = ms_lightListStackOffset.back();
	const uint max = ms_lightList.size() - offset;
	for (uint i = 0; i < max; ++i)
		if (lightBitSet[i])
		{
			const Light *light = ms_lightList[offset + i];
			ms_activeLightList.push_back(light);
		}

	Graphics::setLights(ms_activeLightList);
}

// ----------------------------------------------------------------------
/**
 * Add a primitive to its appropriate phase.
 *
 * @param shaderPrimitive The primitive to be added.
 */

void ShaderPrimitiveSorter::add(const ShaderPrimitive &shaderPrimitive)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ShaderPrimitiveSorter::add simple");

	DEBUG_FATAL(ms_popped, ("Adding primitives after cell popped"));
	const StaticShader &staticShader = ms_prepareToViewFunction(shaderPrimitive);

	// this may be true when the shader has failed validation because of a lost device
	if (!staticShader.isValid())
		return;

	if (ms_useClipRectangle)
	{
		NP_PROFILER_AUTO_BLOCK_DEFINE("ShaderPrimitiveSorter::add simple test clipRectangle");

		Vector const & pos_w = shaderPrimitive.getPosition_w();
		float const radius = shaderPrimitive.getRadius();

		if (radius > 0.0f)
		{
			Rectangle2d const shaderPrimitiveRectangle(pos_w.x - radius, pos_w.z - radius, pos_w.x + radius, pos_w.z + radius);
			if (!ms_clipRectangle.intersects(shaderPrimitiveRectangle))
			{
				return;
			}
		}
	}

	int phase = staticShader.getPhase();
	DEBUG_FATAL(phase < 0 || phase >= static_cast<int>(ms_phase.size()), ("Invalid phase %d/%d", phase, ms_phase.size()));

	//-- see if the primitive is supposed to be affected by water
	if (ms_useWaterTests && phase == ms_belowTransparentWaterTestPhase && ms_belowTransparentWaterFunction)
	{
		NP_PROFILER_AUTO_BLOCK_DEFINE("ShaderPrimitiveSorter::add simple test below water");
		if (ms_belowTransparentWaterFunction (shaderPrimitive.getPosition_w()))
			phase = ms_belowTransparentWaterPhase;
		else
			phase = ms_aboveWaterPhase;
	}

	{
		NP_PROFILER_AUTO_BLOCK_DEFINE("ShaderPrimitiveSorter::add simple insertion");

		if (staticShader.containsPrecalculatedVertexLighting())
			ms_phase[phase].add(shaderPrimitive, staticShader, ms_lightsAffectingShadersWithPrecalculatedVertexLighting);
		else
			ms_phase[phase].add(shaderPrimitive, staticShader, ms_lightsAffectingShadersWithoutPrecalculatedVertexLighting);
	}
}

// ----------------------------------------------------------------------
/**
 * Add a primitive to a specific phase.
 *
 * @param shaderPrimitive The primitive to be added.
 * @param phase The phase index
 */

void ShaderPrimitiveSorter::add(const ShaderPrimitive &shaderPrimitive, const int phase)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ShaderPrimitiveSorter::add");

	if (ms_useClipRectangle)
	{
		Vector const & pos_w = shaderPrimitive.getPosition_w();
		float const radius = shaderPrimitive.getRadius();

		if (radius > 0.0f)
		{
			Rectangle2d const shaderPrimitiveRectangle(pos_w.x - radius, pos_w.z - radius, pos_w.x + radius, pos_w.z + radius);
			if (!ms_clipRectangle.intersects(shaderPrimitiveRectangle))
			{
				return;
			}
		}
	}

	DEBUG_FATAL(ms_popped, ("Adding primitives after cell popped"));
	StaticShader const & staticShader = ms_prepareToViewFunction(shaderPrimitive);
	DEBUG_FATAL(phase < 0 || phase >= static_cast<int>(ms_phase.size()), ("Invalid phase %d/%d", phase, ms_phase.size()));

	if (staticShader.containsPrecalculatedVertexLighting())
		ms_phase[phase].add(shaderPrimitive, staticShader, ms_lightsAffectingShadersWithPrecalculatedVertexLighting);
	else
		ms_phase[phase].add(shaderPrimitive, staticShader, ms_lightsAffectingShadersWithoutPrecalculatedVertexLighting);
}

// ----------------------------------------------------------------------
/**
 * Add a primitive to its appropriate phase.
 *
 * @param shaderPrimitive The primitive to be added.
 */

void ShaderPrimitiveSorter::addWithAlphaFadeOpacity(const ShaderPrimitive &shaderPrimitive, bool opaqueEnable, float opaqueAlpha, bool alphaEnable, float alphaAlpha)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ShaderPrimitiveSorter::add alphaflags");

	UNREF(alphaAlpha);

	DEBUG_FATAL(ms_popped, ("Adding primitives after cell popped"));
	StaticShader const & staticShader = ms_prepareToViewFunction(shaderPrimitive);

	// this may be true when the shader has failed validation because of a lost device
	if (!staticShader.isValid())
		return;

	int phase = staticShader.getPhase();
	DEBUG_FATAL(phase < 0 || phase >= static_cast<int>(ms_phase.size()), ("Invalid phase %d/%d", phase, ms_phase.size()));

	float alphaFadeOpacity = 1.0f;
	if (phase == 3) // Main phase
	{
		if (!opaqueEnable)
		{
			if (staticShader.containsPrecalculatedVertexLighting())
				ms_phase[phase].add(shaderPrimitive, staticShader, ms_lightsAffectingShadersWithPrecalculatedVertexLighting);
			else
				ms_phase[phase].add(shaderPrimitive, staticShader, ms_lightsAffectingShadersWithoutPrecalculatedVertexLighting);
			return;
		}

		alphaFadeOpacity = opaqueAlpha;
		phase = 6;
	}
	else
		if (phase == 5 || phase == 6) // Alpha/Blend/PreAlpha phases.
		{
			if (!alphaEnable)
			{
				if (staticShader.containsPrecalculatedVertexLighting())
					ms_phase[phase].add(shaderPrimitive, staticShader, ms_lightsAffectingShadersWithPrecalculatedVertexLighting);
				else
					ms_phase[phase].add(shaderPrimitive, staticShader, ms_lightsAffectingShadersWithoutPrecalculatedVertexLighting);
				return;
			}

			alphaFadeOpacity = alphaAlpha;
		}

	//-- see if the primitive is supposed to be affected by water
	if (ms_useWaterTests && ms_belowTransparentWaterFunction && phase == ms_belowTransparentWaterTestPhase)
	{
		NP_PROFILER_AUTO_BLOCK_DEFINE("ShaderPrimitiveSorter::add alphaflags test below water");
		
		if (ms_belowTransparentWaterFunction(shaderPrimitive.getPosition_w()))
			phase = ms_belowTransparentWaterPhase;
		else
			phase = ms_aboveWaterPhase;
	}

	if (staticShader.containsPrecalculatedVertexLighting())
		ms_phase[phase].addWithAlphaFadeOpacity(shaderPrimitive, staticShader, alphaFadeOpacity, ms_lightsAffectingShadersWithPrecalculatedVertexLighting);
	else
		ms_phase[phase].addWithAlphaFadeOpacity(shaderPrimitive, staticShader, alphaFadeOpacity, ms_lightsAffectingShadersWithoutPrecalculatedVertexLighting);
}

// ----------------------------------------------------------------------
/**
 * Add a primitive with a specific set of active lights to its appropriate phase.
 *
 * @param shaderPrimitive The primitive to be added.
 * @param lightBitSet     The set of active lights for this primitive.
 */

void ShaderPrimitiveSorter::add(const ShaderPrimitive &shaderPrimitive, const LightBitSet &lightBitSet)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ShaderPrimitiveSorter::add lightbitset");
	
	DEBUG_FATAL(ms_popped, ("Adding primitives after cell popped"));
	StaticShader const & staticShader = ms_prepareToViewFunction(shaderPrimitive);
	const int phase = staticShader.getPhase();
	DEBUG_FATAL(phase < 0 || phase >= static_cast<int>(ms_phase.size()), ("Invalid phase %d/%d", phase, ms_phase.size()));

	ms_phase[phase].add(shaderPrimitive, staticShader, lightBitSet);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::pushCell(CellProperty const * cellProperty, Texture const * environmentTexture, bool fogEnabled, float fogDensity, PackedArgb const & fogColor)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ShaderPrimitiveSorter::pushCell");

#ifdef _DEBUG
	{
		const int count = static_cast<int>(ms_phase.size());
		for (int i = 0; i < count; ++i)
			if (ms_phase[i].getDrawEnable() != ms_debugPhase[i].enabled)
				ms_phase[i].setDrawEnable(ms_debugPhase[i].enabled);
	}
#endif

	ms_pixelsCompositedThisFrame = 0;
	ms_compositePasses = 0;

	if (!ms_popped && !ms_cellPropertyStack.empty() && ms_cellPropertyStack.back())
		ms_cellPropertyStack.back()->callPreDrawRenderHookFunctions();

	// tell all the phases we're pushing
	{
		const int count = static_cast<int>(ms_phase.size());
		for (int i = 0; i < count; ++i)
			ms_phase[i].pushCell();
	}

	ms_environmentTextures.push_back(environmentTexture ? environmentTexture : ms_defaultEnvironmentTexture);
	ms_fogStack.push_back(FogData(fogEnabled, fogDensity, fogColor));

	Graphics::setGlobalTexture(TAG_ENVM, *ms_environmentTextures.back());
	Graphics::setFog(fogEnabled, fogDensity, fogColor);

	// preserve all the lights in the current cell
	ms_lightListStackOffset.push_back(ms_lightList.size());

	// empty out the light bits for the new cell
	ms_lightsAffectingShadersWithoutPrecalculatedVertexLighting.reset();
	ms_lightsAffectingShadersWithPrecalculatedVertexLighting.reset();

	ms_cellPropertyStack.push_back(cellProperty);
	if (cellProperty)
		cellProperty->callEnterRenderHookFunctions();

	ms_popped = false;

#ifdef _DEBUG
	ms_lightListOverflow = false;
#endif
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::pushCell(Texture const * environmentTexture, bool fogEnabled, float fogDensity, PackedArgb const & fogColor)
{
	pushCell(NULL, environmentTexture, fogEnabled, fogDensity, fogColor);
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::pushCell(CellProperty const & cellProperty)
{
	pushCell(&cellProperty, cellProperty.getEnvironmentTexture(), cellProperty.isFogEnabled(), cellProperty.getFogDensity(), cellProperty.getFogColor());
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::popCell()
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ShaderPrimitiveSorter::popCell");

	if (!ms_popped && ms_cellPropertyStack.back())
		ms_cellPropertyStack.back()->callPreDrawRenderHookFunctions();

	DEBUG_FATAL(ms_lightListStackOffset.size() == 1, ("Stack underflow"));

	// tell all the phases we're popping
	{
		const int count = static_cast<int>(ms_phase.size());
		for (int i = 0; i < count; ++i)
			ms_phase[i].popCell();
	}

	ms_environmentTextures.pop_back();
	Graphics::setGlobalTexture(TAG_ENVM, *ms_environmentTextures.back());

	ms_fogStack.pop_back();
	FogData fogData = ms_fogStack.back();
	Graphics::setFog(fogData.enabled, fogData.density, fogData.color);

	// make sure the lights are cleared out
	ms_activeLightList.clear();	
	Graphics::setLights(ms_activeLightList);

	// remove all the lights added in the popped cell
	LightList::const_iterator iEnd = ms_lightList.end();
	const int offset = ms_lightListStackOffset.back();
	for (LightList::const_iterator i = ms_lightList.begin() + offset; i != iEnd; ++i)
		(*i)->setShaderPrimitiveSorterLightIndex(-1);
	ms_lightList.erase(ms_lightList.begin() + offset, ms_lightList.end());
	ms_lightListStackOffset.pop_back();
	ms_lightsAffectingShadersWithoutPrecalculatedVertexLighting.reset();
	ms_lightsAffectingShadersWithPrecalculatedVertexLighting.reset();

	if (ms_cellPropertyStack.back())
	{
		ms_cellPropertyStack.back()->callExitRenderHookFunctions();
#ifdef _DEBUG
		ms_cellPropertyStack.back() = NULL;
#endif
	}
	ms_cellPropertyStack.pop_back();

	ms_popped = true;

#ifdef _DEBUG
	ms_lightListOverflow = false;
#endif

	DEBUG_REPORT_PRINT(ms_reportPixelsComposited && ms_pixelsCompositedThisFrame > 0, 
		("Composited [%8d] KiloPixels in [%4d] passes\n", ms_pixelsCompositedThisFrame / 1000, ms_compositePasses));
}

// ----------------------------------------------------------------------

StaticShader const & ShaderPrimitiveSorter::defaultPrepareToView(const ShaderPrimitive &shaderPrimitive)
{
  return shaderPrimitive.prepareToView();
}
 
// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::setPrepareToViewOverrideFunction(PrepareToViewOverrideFunction prepareToViewOverrideFunction)
{
	if (prepareToViewOverrideFunction)
		ms_prepareToViewFunction = prepareToViewOverrideFunction;
	else
		ms_prepareToViewFunction = defaultPrepareToView;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::setBelowTransparentWaterFunctionAndPhases(BelowTransparentWaterFunction belowTransparentWaterFunction, int belowTransparentWaterTestPhase, int belowTransparentWaterPhase, int aboveWaterPhase)
{
	DEBUG_FATAL(belowTransparentWaterTestPhase < 0 || belowTransparentWaterTestPhase >= static_cast<int>(ms_phase.size()), ("Invalid belowTransparentWaterTestPhase %d/%d", belowTransparentWaterTestPhase, ms_phase.size()));
	DEBUG_FATAL(belowTransparentWaterPhase < 0 || belowTransparentWaterPhase >= static_cast<int>(ms_phase.size()), ("Invalid belowTransparentWaterPhase %d/%d", belowTransparentWaterPhase, ms_phase.size()));
	DEBUG_FATAL(aboveWaterPhase < 0 || aboveWaterPhase >= static_cast<int>(ms_phase.size()), ("Invalid aboveWaterPhase %d/%d", aboveWaterPhase, ms_phase.size()));
	ms_belowTransparentWaterFunction  = belowTransparentWaterFunction;
	ms_belowTransparentWaterTestPhase = belowTransparentWaterTestPhase;
	ms_belowTransparentWaterPhase     = belowTransparentWaterPhase;
	ms_aboveWaterPhase     = aboveWaterPhase;
}

//----------------------------------------------------------------------

void ShaderPrimitiveSorter::alter(float elapsedTimeSecs)
{
	ms_elapsedTime += elapsedTimeSecs;
}

//----------------------------------------------------------------------

void ShaderPrimitiveSorter::setHeatShadersEnabled(bool enabled)
{
	ms_heatShadersEnabled = enabled;
}

//----------------------------------------------------------------------

bool ShaderPrimitiveSorter::getHeatShadersEnabled()
{
	return ms_heatShadersEnabled;
}

// ----------------------------------------------------------------------

void ShaderPrimitiveSorter::setUseClipRectangle(bool b)
{
	ms_useClipRectangle = b;
}

//----------------------------------------------------------------------

void ShaderPrimitiveSorter::setClipRectangle(float left, float bottom, float right, float top)
{
	ms_clipRectangle.set(left, bottom, right, top);
}

//----------------------------------------------------------------------

void ShaderPrimitiveSorter::setClipEnvironmentFlags(int flags)
{
	ms_clipEnvironmentFlags = flags;
}

//----------------------------------------------------------------------

/**
* We commonly want to disable these tests during the environment render
*/

void ShaderPrimitiveSorter::setUseWaterTests(bool b)
{
	ms_useWaterTests = b;
}

//----------------------------------------------------------------------

bool ShaderPrimitiveSorter::getHeatShadersCapable()
{
	return GraphicsOptionTags::get(TAG(H,E,A,T));
}

//----------------------------------------------------------------------


#ifdef _DEBUG

void ShaderPrimitiveSorter::debugDump ()
{
	float totalTime = 0.f;

	const int count = static_cast<int>(ms_phase.size());
	for (int i = 0; i < count; ++i)
		totalTime += ms_phase[i].getFrameDrawTime();

	if (totalTime <= 0.f)
		totalTime = 1.f;

	for (int j = 0; j < count; ++j)
	{
		DEBUG_REPORT_PRINT(true, ("%20s : %1.5f %3i%%\n", ms_debugPhase[j].name, ms_phase[j].getFrameDrawTime(), static_cast<int> (100.f * ms_phase[j].getFrameDrawTime() / totalTime)));
		ms_phase[j].clearFrameDrawTime();
	}
}


#endif

// ======================================================================
