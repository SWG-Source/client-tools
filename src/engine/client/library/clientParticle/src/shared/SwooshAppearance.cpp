// ============================================================================
//
// SwooshAppearance.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/SwooshAppearance.h"

#include "clientAudio/Audio.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientParticle/ConfigClientParticle.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/ParticleManager.h"
#include "clientParticle/ParticleTexture.h"
#include "clientParticle/SetupClientParticle.h"
#include "clientParticle/SwooshAppearanceTemplate.h"
#include "dpvsObject.hpp"
#include "dpvsModel.hpp"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Misc.h"
#include "sharedFoundation/Os.h"
#include "sharedMath/CatmullRomSpline.h"
#include "sharedMath/PackedArgb.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/TerrainObject.h"

// ============================================================================
//
// SwooshAppearanceNamespace
//
// ============================================================================

namespace SwooshAppearanceNamespace
{
	VectorArgb s_globalColorModifier(1.0f, 1.0f, 1.0f, 1.0f);

#ifdef _DEBUG
	int s_globalCount = 0;
	int s_cullCountThisFrame = 0;
	int s_currentUpdateFrame = 0;
#endif // _DEBUG
}

using namespace SwooshAppearanceNamespace;

// ============================================================================
//
// SwooshAppearance::LocalShaderPrimitive
//
// ============================================================================

//lint -esym(754, LocalShaderPrimitive::LocalShaderPrimitive) //copy ctor not referenced

//-----------------------------------------------------------------------------
class SwooshAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
friend class SwooshAppearance;

public:

	static VertexBufferFormat getVertexBufferFormat();

public:

	explicit LocalShaderPrimitive(SwooshAppearance const &SwooshAppearance);
	virtual ~LocalShaderPrimitive();

	virtual float               alter(float time);
	virtual const Vector        getPosition_w() const;
	virtual const StaticShader &prepareToView() const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual void                prepareToDraw() const;
	virtual void                draw() const;
	virtual float               getRadius() const;

private:

	SwooshAppearance const &m_swooshAppearance;
	DynamicVertexBuffer     m_vertexBuffer;
	Camera const *          m_camera;
	Object const *          m_object;
	Shader const *          m_shader;
	float                   m_depthSquaredSortKey;

private:

	// Disabled

	LocalShaderPrimitive();
	LocalShaderPrimitive(LocalShaderPrimitive const &);
	LocalShaderPrimitive &operator=(LocalShaderPrimitive const &);
};

//-----------------------------------------------------------------------------
VertexBufferFormat SwooshAppearance::LocalShaderPrimitive::getVertexBufferFormat()
{
	VertexBufferFormat format;

	format.setPosition();
	format.setColor0();
	format.setNumberOfTextureCoordinateSets(1);
	format.setTextureCoordinateSetDimension(0, 2);

	return format;
}

//-----------------------------------------------------------------------------
SwooshAppearance::LocalShaderPrimitive::LocalShaderPrimitive(SwooshAppearance const &swooshAppearance)
 : ShaderPrimitive()
 , m_swooshAppearance(swooshAppearance)
 , m_vertexBuffer(getVertexBufferFormat())
 , m_camera(NULL)
 , m_object(NULL)
 , m_shader(NULL)
 , m_depthSquaredSortKey(0.0f)
{
	ParticleTexture const &particleTexture = m_swooshAppearance.m_swooshAppearanceTemplate->getParticleTexture();

	if (   particleTexture.isTextureVisible()
	    && !particleTexture.getShaderPath().isEmpty())
	{
		m_shader = ShaderTemplateList::fetchShader(particleTexture.getShaderPath().getString());
	}
}

//-----------------------------------------------------------------------------
SwooshAppearance::LocalShaderPrimitive::~LocalShaderPrimitive()
{
	if (m_shader != NULL)
	{
		m_shader->release();
	}

	m_camera = NULL;
	m_object = NULL;
	m_shader = NULL;
}

//-----------------------------------------------------------------------------
float SwooshAppearance::LocalShaderPrimitive::alter(real time)
{
	if (m_shader != NULL)
	{
		return m_shader->alter(time);
	}
	else
	{
		return AlterResult::cms_kill;
	}
}

//-----------------------------------------------------------------------------
StaticShader const &SwooshAppearance::LocalShaderPrimitive::prepareToView() const
{
	StaticShader const &result = ((m_shader != NULL) && ParticleManager::isTexturingEnabled()) ? m_shader->prepareToView() : ShaderTemplateList::get3dVertexColorACStaticShader();

	return result;
}

//-----------------------------------------------------------------------------
const Vector SwooshAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return (m_object != NULL) ? m_object->getPosition_w() : Vector::zero;
}

//-------------------------------------------------------------------

float SwooshAppearance::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return m_depthSquaredSortKey;
}

//-----------------------------------------------------------------------------
int SwooshAppearance::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return m_vertexBuffer.getSortKey();
}

//-----------------------------------------------------------------------------
void SwooshAppearance::LocalShaderPrimitive::prepareToDraw() const
{
	m_swooshAppearance.renderGeometry();
}

//-----------------------------------------------------------------------------
void SwooshAppearance::LocalShaderPrimitive::draw() const
{
	GlCullMode const cullMode = Graphics::getCullMode();

	Graphics::setCullMode(GCM_none);
		Graphics::drawQuadList();
	Graphics::setCullMode(cullMode);
}

//----------------------------------------------------------------------

float SwooshAppearance::LocalShaderPrimitive::getRadius() const
{
	return m_swooshAppearance.getSphere().getRadius();
}

// ============================================================================
//
// SwooshAppearance
//
// ============================================================================

//--------------------------------------------------------------------------
SwooshAppearance::SwooshAppearance(SwooshAppearanceTemplate const *swooshAppearanceTemplate)
 : Appearance(swooshAppearanceTemplate)
 , m_swooshAppearanceTemplate(swooshAppearanceTemplate)
 , m_localShaderPrimitive(NULL)
 , m_currentTime(0.0f)
 , m_extent_w()
 , m_accumulatedSmallerRadiusChanges(0)
 , m_unBounded(false)
 , m_dpvsObject(NULL)
 , m_positionList(new PositionData[swooshAppearanceTemplate->getSamplePositionCount()])
 , m_positionIndex(0)
 , m_startObject(NULL)
 , m_endObject(NULL)
 , m_hasBeenAltered(false)
 , m_soundId()
 , m_colorModifier(VectorArgb::solidWhite)
 , m_shaderScrollPosition(0.0f)
 , m_sampledPositionCount(0)
 , m_textureFrame(0)
 , m_width(swooshAppearanceTemplate->getWidth())
 , m_killIndex(swooshAppearanceTemplate->getSamplePositionCount())
 , m_hardPoint1()
 , m_hardPoint2()
 , m_referenceObject(NULL)
 , m_hardPointAppearance(NULL)
 , m_allowAutoDelete(true)
 , m_movementSpeed(0.0f)
 , m_renderThisFrame(false)
 , m_visibilityPercent(0.0f)
 , m_lastDeltaTime(0.0f)
 , m_secondsBetweenSamples(1.0f / swooshAppearanceTemplate->getSamplesPerSecond())
 , m_secondsSinceLastSample(m_secondsBetweenSamples)
 , m_enabled(true)
 , m_templateSamplePositionCount(swooshAppearanceTemplate->getSamplePositionCount())
 , m_currentValidSamplePositionCount(0)
 , m_positionReference(PR_none)
{
#ifdef _DEBUG
	++s_globalCount;
#endif // _DEBUG

	m_localShaderPrimitive = new LocalShaderPrimitive(*this);

	// Set the extent initial value, maybe on the first frame of ths effect, set the extent to the object position

	m_extent_w.setMin(Vector::zero);
	m_extent_w.setMax(Vector::zero);

	{
		DPVS::Model *const model = RenderWorld::fetchDefaultModel();
		m_dpvsObject = RenderWorld::createObject(this, model);
		IGNORE_RETURN(model->release());
	}
	m_dpvsObject->set(DPVS::Object::ENABLED, false);

	useRenderEffectsFlag(true);
}

//--------------------------------------------------------------------------
SwooshAppearance::SwooshAppearance(SwooshAppearanceTemplate const *swooshAppearanceTemplate, Object const * const object)
 : Appearance(swooshAppearanceTemplate)
 , m_swooshAppearanceTemplate(swooshAppearanceTemplate)
 , m_localShaderPrimitive(NULL)
 , m_currentTime(0.0f)
 , m_extent_w()
 , m_accumulatedSmallerRadiusChanges(0)
 , m_unBounded(false)
 , m_dpvsObject(NULL)
 , m_positionList(new PositionData[swooshAppearanceTemplate->getSamplePositionCount()])
 , m_positionIndex(0)
 , m_startObject(NULL)
 , m_endObject(NULL)
 , m_hasBeenAltered(false)
 , m_soundId()
 , m_colorModifier(VectorArgb::solidWhite)
 , m_shaderScrollPosition(0.0f)
 , m_sampledPositionCount(0)
 , m_textureFrame(0)
 , m_width(swooshAppearanceTemplate->getWidth())
 , m_killIndex(swooshAppearanceTemplate->getSamplePositionCount())
 , m_hardPoint1()
 , m_hardPoint2()
 , m_referenceObject(object)
 , m_hardPointAppearance(NULL)
 , m_allowAutoDelete(true)
 , m_movementSpeed(0.0f)
 , m_renderThisFrame(false)
 , m_visibilityPercent(0.0f)
 , m_lastDeltaTime(0.0f)
 , m_secondsBetweenSamples(1.0f / swooshAppearanceTemplate->getSamplesPerSecond())
 , m_secondsSinceLastSample(m_secondsBetweenSamples)
 , m_enabled(true)
 , m_templateSamplePositionCount(swooshAppearanceTemplate->getSamplePositionCount())
 , m_currentValidSamplePositionCount(0)
 , m_positionReference(PR_object)
{
#ifdef _DEBUG
	++s_globalCount;
#endif // _DEBUG

	m_localShaderPrimitive = new LocalShaderPrimitive(*this);

	// Set the extent initial value, maybe on the first frame of ths effect, set the extent to the object position

	m_extent_w.setMin(Vector::zero);
	m_extent_w.setMax(Vector::zero);

	{
		DPVS::Model *const model = RenderWorld::fetchDefaultModel();
		m_dpvsObject = RenderWorld::createObject(this, model);
		IGNORE_RETURN(model->release());
	}
	m_dpvsObject->set(DPVS::Object::ENABLED, false);

	useRenderEffectsFlag(true);
}

//--------------------------------------------------------------------------
SwooshAppearance::SwooshAppearance(SwooshAppearanceTemplate const *swooshAppearanceTemplate, Appearance const * const hardPointAppearance, CrcString const &hardPoint)
 : Appearance(swooshAppearanceTemplate)
 , m_swooshAppearanceTemplate(swooshAppearanceTemplate)
 , m_localShaderPrimitive(NULL)
 , m_currentTime(0.0f)
 , m_extent_w()
 , m_accumulatedSmallerRadiusChanges(0)
 , m_unBounded(false)
 , m_dpvsObject(NULL)
 , m_positionList(new PositionData[swooshAppearanceTemplate->getSamplePositionCount()])
 , m_positionIndex(0)
 , m_startObject(NULL)
 , m_endObject(NULL)
 , m_hasBeenAltered(false)
 , m_soundId()
 , m_colorModifier(VectorArgb::solidWhite)
 , m_shaderScrollPosition(0.0f)
 , m_sampledPositionCount(0)
 , m_textureFrame(0)
 , m_width(swooshAppearanceTemplate->getWidth())
 , m_killIndex(swooshAppearanceTemplate->getSamplePositionCount())
 , m_hardPoint1(hardPoint)
 , m_hardPoint2()
 , m_referenceObject(NULL)
 , m_hardPointAppearance(hardPointAppearance)
 , m_allowAutoDelete(true)
 , m_movementSpeed(0.0f)
 , m_renderThisFrame(false)
 , m_visibilityPercent(0.0f)
 , m_lastDeltaTime(0.0f)
 , m_secondsBetweenSamples(1.0f / swooshAppearanceTemplate->getSamplesPerSecond())
 , m_secondsSinceLastSample(m_secondsBetweenSamples)
 , m_enabled(true)
 , m_templateSamplePositionCount(swooshAppearanceTemplate->getSamplePositionCount())
 , m_currentValidSamplePositionCount(0)
 , m_positionReference(PR_oneHardPoint)
{
#ifdef _DEBUG
	++s_globalCount;
#endif // _DEBUG

	DEBUG_WARNING((m_hardPointAppearance == NULL), ("The hardpoint appearance should not be NULL."));

	m_localShaderPrimitive = new LocalShaderPrimitive(*this);

	// Set the extent initial value, maybe on the first frame of ths effect, set the extent to the object position

	m_extent_w.setMin(Vector::zero);
	m_extent_w.setMax(Vector::zero);

	{
		DPVS::Model *const model = RenderWorld::fetchDefaultModel();
		m_dpvsObject = RenderWorld::createObject(this, model);
		IGNORE_RETURN(model->release());
	}
	m_dpvsObject->set(DPVS::Object::ENABLED, false);

	useRenderEffectsFlag(true);
}

//--------------------------------------------------------------------------
SwooshAppearance::SwooshAppearance(SwooshAppearanceTemplate const *swooshAppearanceTemplate, Appearance const * const hardPointAppearance, CrcString const &hardPoint1, CrcString const &hardPoint2)
 : Appearance(swooshAppearanceTemplate)
 , m_swooshAppearanceTemplate(swooshAppearanceTemplate)
 , m_localShaderPrimitive(NULL)
 , m_currentTime(0.0f)
 , m_extent_w()
 , m_accumulatedSmallerRadiusChanges(0)
 , m_unBounded(false)
 , m_dpvsObject(NULL)
 , m_positionList(new PositionData[swooshAppearanceTemplate->getSamplePositionCount()])
 , m_positionIndex(0)
 , m_startObject(NULL)
 , m_endObject(NULL)
 , m_hasBeenAltered(false)
 , m_soundId()
 , m_colorModifier(VectorArgb::solidWhite)
 , m_shaderScrollPosition(0.0f)
 , m_sampledPositionCount(0)
 , m_textureFrame(0)
 , m_width(swooshAppearanceTemplate->getWidth())
 , m_killIndex(swooshAppearanceTemplate->getSamplePositionCount())
 , m_hardPoint1(hardPoint1)
 , m_hardPoint2(hardPoint2)
 , m_referenceObject(NULL)
 , m_hardPointAppearance(hardPointAppearance)
 , m_allowAutoDelete(true)
 , m_movementSpeed(0.0f)
 , m_renderThisFrame(false)
 , m_visibilityPercent(0.0f)
 , m_lastDeltaTime(0.0f)
 , m_secondsBetweenSamples(1.0f / swooshAppearanceTemplate->getSamplesPerSecond())
 , m_secondsSinceLastSample(m_secondsBetweenSamples)
 , m_enabled(true)
 , m_templateSamplePositionCount(swooshAppearanceTemplate->getSamplePositionCount())
 , m_currentValidSamplePositionCount(0)
 , m_positionReference(PR_twoHardPoints)
{
#ifdef _DEBUG
	++s_globalCount;
#endif // _DEBUG

	DEBUG_WARNING((m_hardPointAppearance == NULL), ("The hardpoint appearance should not be NULL."));

	m_localShaderPrimitive = new LocalShaderPrimitive(*this);

	// Set the extent initial value, maybe on the first frame of ths effect, set the extent to the object position

	m_extent_w.setMin(Vector::zero);
	m_extent_w.setMax(Vector::zero);

	{
		DPVS::Model *const model = RenderWorld::fetchDefaultModel();
		m_dpvsObject = RenderWorld::createObject(this, model);
		IGNORE_RETURN(model->release());
	}
	m_dpvsObject->set(DPVS::Object::ENABLED, false);

	useRenderEffectsFlag(true);
}

//--------------------------------------------------------------------------
SwooshAppearance::~SwooshAppearance()
{
	delete m_localShaderPrimitive;
	m_localShaderPrimitive = NULL;

	m_swooshAppearanceTemplate = NULL;

#ifdef _DEBUG
	--s_globalCount;
#endif // _DEBUG

	IGNORE_RETURN(m_dpvsObject->release());
	m_dpvsObject = NULL;

	delete m_startObject.getPointer();
	m_startObject = NULL;

	delete m_endObject.getPointer();
	m_endObject = NULL;

	delete [] m_positionList;
}

//--------------------------------------------------------------------------
DPVS::Object * SwooshAppearance::getDpvsObject() const
{
	return m_dpvsObject;
} //lint !e1763 //const method indirectly modifies object

//--------------------------------------------------------------------------
void SwooshAppearance::render() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("SwooshAppearance::render");

	// Make sure particles are enabled

	Vector const & currentCameraPosition = ShaderPrimitiveSorter::getCurrentCameraPosition();

	if (m_renderThisFrame)
	{
		// See how far the center point is from the last known camera position to determine whether to cull the swoosh or not

		Vector const extentCenter_w(m_extent_w.getMin() + (m_extent_w.getMin() - m_extent_w.getMax()) / 2.0f);

		if (currentCameraPosition.magnitudeBetweenSquared(extentCenter_w) > sqr(ConfigClientParticle::getSwooshCullDistance()))
		{
			m_renderThisFrame = false;

	#ifdef _DEBUG
			++s_cullCountThisFrame;
	#endif // _DEBUG
		}
	}

	if (m_renderThisFrame)
	{
		m_localShaderPrimitive->m_camera = &ShaderPrimitiveSorter::getCurrentCamera();
		m_localShaderPrimitive->m_object = getOwner();
		m_localShaderPrimitive->m_depthSquaredSortKey = m_extent_w.getSphere().getCenter().magnitudeBetweenSquared(currentCameraPosition);

		// Add the primitive to be rendered at the appropriate time

		ShaderPrimitiveSorter::add(*m_localShaderPrimitive);
	}

#ifdef _DEBUG
	// Draw extents

	if (ParticleManager::isDebugExtentsEnabled())
	{
		drawDebugExtents();
	}
#endif // _DEBUG
}

//--------------------------------------------------------------------------
void SwooshAppearance::renderGeometry() const
{
	// Make sure we have sampled some points

	int const splineSubQuads = m_swooshAppearanceTemplate->getSplineSubQuads();
	int const lockedVertexCount = (static_cast<int>(m_currentValidSamplePositionCount) - 1) * 4 * splineSubQuads;

	if (   (m_currentValidSamplePositionCount > 1)
	    && (lockedVertexCount > 0))
	{
		int usedVertexCount = 0;
		m_localShaderPrimitive->m_vertexBuffer.lock(lockedVertexCount);

		VertexBufferWriteIterator vbwIter = m_localShaderPrimitive->m_vertexBuffer.begin();

		// Get the color

		float const initialAlpha = m_visibilityPercent * clamp(0.0f, m_swooshAppearanceTemplate->getAlpha() * m_colorModifier.a * s_globalColorModifier.a, 1.0f);

		VectorArgb const &shaderColor = m_swooshAppearanceTemplate->getColor();
		float red = clamp(0.0f, shaderColor.r * m_colorModifier.r * s_globalColorModifier.r, 1.0f);
		float green = clamp(0.0f, shaderColor.g * m_colorModifier.g * s_globalColorModifier.g, 1.0f);
		float blue = clamp(0.0f, shaderColor.b * m_colorModifier.b * s_globalColorModifier.b, 1.0f);

		float drawDistanceTotalSquared = 0.0f;
		int currentTextureFrame = m_textureFrame;
		float au = 0.0f;
		float av = 0.0f;
		float bu = 0.0f;
		float bv = 0.0f;
		float cu = 0.0f;
		float cv = 0.0f;
		float du = 0.0f;
		float dv = 0.0f;
		float floatingAv = 0.0f;
		float floatingBv = 0.0f;
		Vector controlPointMiddleA;
		Vector controlPointMiddleB;
		Vector controlPointMiddleC;
		Vector controlPointMiddleD;
		Vector controlPointTopA;
		Vector controlPointTopB;
		Vector controlPointTopC;
		Vector controlPointTopD;
		Vector controlPointBottomA;
		Vector controlPointBottomB;
		Vector controlPointBottomC;
		Vector controlPointBottomD;
		float const stretchDistance = m_swooshAppearanceTemplate->getShaderStretchDistance() * m_swooshAppearanceTemplate->getShaderStretchDistance();
		SwooshAppearanceTemplate::FadeAlpha const fadeAlpha = m_swooshAppearanceTemplate->getFadeAlpha();
		SwooshAppearanceTemplate::TaperGeometry const taperGeometry = m_swooshAppearanceTemplate->getTaperGeometry();
		bool const cameraFacing = isCameraFacing();
		float const halfWidth = m_width / 2.0f;

		// Render from newest position to oldest position

		int visualIndexCount = -1;

		for (unsigned int logicalIndex = 0; logicalIndex <= m_sampledPositionCount; ++logicalIndex)
		{
			unsigned int const visualIndex = static_cast<unsigned int>(getVisualIndex(logicalIndex));

			if (!m_positionList[visualIndex].m_valid)
			{
				continue;
			}

			++visualIndexCount;

			controlPointTopA = controlPointTopB;
			controlPointTopB = controlPointTopC;
			controlPointTopC = controlPointTopD;
			controlPointBottomA = controlPointBottomB;
			controlPointBottomB = controlPointBottomC;
			controlPointBottomC = controlPointBottomD;

			if (cameraFacing)
			{
				controlPointMiddleA = controlPointMiddleB;
				controlPointMiddleB = controlPointMiddleC;
				controlPointMiddleC = controlPointMiddleD;

				unsigned int const nextIndex = m_positionList[visualIndex].m_nextValidIndex;
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), nextIndex, m_templateSamplePositionCount);

				controlPointMiddleD = getMiddlePosition_w(visualIndex);

				Vector const directionToCamera(m_localShaderPrimitive->m_camera->getPosition_w() - controlPointMiddleD);
				Vector bankVector((controlPointMiddleD - getMiddlePosition_w(nextIndex)).cross(directionToCamera));
				IGNORE_RETURN(bankVector.approximateNormalize());

				controlPointTopD = getMiddlePosition_w(visualIndex) + bankVector * +halfWidth;
				controlPointBottomD = getMiddlePosition_w(visualIndex) + bankVector * -halfWidth;

				m_positionList[visualIndex].m_topPosition_w = controlPointTopD;
				m_positionList[visualIndex].m_bottomPosition_w = controlPointBottomD;
			}
			else
			{
				controlPointTopD = getTopPosition_w(visualIndex);
				controlPointBottomD = getBottomPosition_w(visualIndex);
			}

			if (visualIndexCount < 2)
			{
				continue;
			}
			else if (visualIndexCount == 2)
			{
				controlPointMiddleA = controlPointMiddleB;
				controlPointTopA = controlPointTopB;
				controlPointBottomA = controlPointBottomB;
			}
			else if(static_cast<unsigned int>(visualIndexCount) >= m_currentValidSamplePositionCount)
			{
				controlPointMiddleD = controlPointMiddleC;
				controlPointTopD = controlPointTopC;
				controlPointBottomD = controlPointBottomC;
			}

			float const vertexPercent = 1.0f - (static_cast<float>(visualIndexCount - 1) / static_cast<float>(m_currentValidSamplePositionCount - 1));
			DEBUG_FATAL((vertexPercent < 0.0f) || (vertexPercent > 1.0f), ("Invalid percent: %f", vertexPercent));

			Vector a;
			Vector b;
			Vector c;
			Vector d;
			PackedArgb colorCD;
			PackedArgb colorAB;

			{
				float const alpha = getAlpha(initialAlpha, vertexPercent, fadeAlpha);

				if (m_swooshAppearanceTemplate->isMultiplyColorByAlpha())
				{
					colorAB.setArgb(VectorArgb(alpha, red * alpha, green * alpha, blue * alpha));
				}
				else
				{
					colorAB.setArgb(VectorArgb(alpha, red, green, blue));
				}
			}

			getTaperGeometry(vertexPercent, taperGeometry, controlPointBottomB, controlPointTopB, b);
			a = controlPointBottomB;

			for (int j = 1; j <= splineSubQuads; ++j)
			{
				c = b;
				d = a;
				colorCD = colorAB;

				float const t = static_cast<float>(j) / static_cast<float>(splineSubQuads);
				float const vertexPercentDelta = 1.0f / static_cast<float>(m_currentValidSamplePositionCount - 1);
				float const subPercent = vertexPercent - (vertexPercentDelta * t);

				DEBUG_FATAL((subPercent > vertexPercent), ("Invalid sub percent 2: %f", subPercent));

				// Calculate the color

				{
					float alpha = getAlpha(initialAlpha, subPercent, fadeAlpha);

#ifdef _DEBUG
					if (ParticleManager::isDebugShowSwooshQuadsEnabled())
					{
						red = Random::randomReal();
						green = Random::randomReal();
						blue = Random::randomReal();
						alpha = 1.0f;
					}
#endif // _DEBUG

					if (m_swooshAppearanceTemplate->isMultiplyColorByAlpha())
					{
						colorAB.setArgb(VectorArgb(alpha, red * alpha, green * alpha, blue * alpha));
					}
					else
					{
						colorAB.setArgb(VectorArgb(alpha, red, green, blue));
					}
				}

				// Calculate the spline sub position

				{
					// The positions retrieved from these calls are between control point B and C

					CatmullRomSpline::getCatmullRomSplinePoint3d(controlPointTopA, controlPointTopB, controlPointTopC, controlPointTopD, t, b);
					CatmullRomSpline::getCatmullRomSplinePoint3d(controlPointBottomA, controlPointBottomB, controlPointBottomC, controlPointBottomD, t, a);

					getTaperGeometry(subPercent, taperGeometry, a, b, b);
				}

				// Calculate the texture coordinates

				{
					// Only get the new texture coordinates when the current texture index is used up

					if (WithinEpsilonInclusive(floatingAv, av, 0.0001f))
					{
						// Texture UVs

						// b - c
						// |   |
						// a - d

						// [0,0] - [1,0]
						// |           |
						// [0,1] - [1,1]

						m_swooshAppearanceTemplate->getParticleTexture().getFrameUVs(currentTextureFrame, au, av, bu, bv, cu, cv, du, dv);
						floatingAv = bv;
					}

					float const currentDistanceSquared = (b - c).approximateMagnitude();

					drawDistanceTotalSquared += currentDistanceSquared;

					if (   (drawDistanceTotalSquared > stretchDistance)
						|| ((stretchDistance - drawDistanceTotalSquared) < (currentDistanceSquared * 0.5f)))
					{
						drawDistanceTotalSquared = 0.0f;
						floatingBv = av;

						if (++currentTextureFrame > m_swooshAppearanceTemplate->getParticleTexture().getFrameEnd())
						{
							currentTextureFrame = m_swooshAppearanceTemplate->getParticleTexture().getFrameStart();
						}
					}
					else
					{
						floatingBv = clamp(0.0f, ((av - bv) * (drawDistanceTotalSquared / stretchDistance)), 1.0f);
					}
				}

				vbwIter.setPosition(a);
				vbwIter.setColor0(colorAB);
				vbwIter.setTextureCoordinates(0, cu, floatingBv + m_shaderScrollPosition);
				++vbwIter;
				++usedVertexCount;

				vbwIter.setPosition(b);
				vbwIter.setColor0(colorAB);
				vbwIter.setTextureCoordinates(0, bu, floatingBv + m_shaderScrollPosition);
				++vbwIter;
				++usedVertexCount;

				vbwIter.setPosition(c);
				vbwIter.setColor0(colorCD);
				vbwIter.setTextureCoordinates(0, au, floatingAv + m_shaderScrollPosition);
				++vbwIter;
				++usedVertexCount;

				vbwIter.setPosition(d);
				vbwIter.setColor0(colorCD);
				vbwIter.setTextureCoordinates(0, du, floatingAv + m_shaderScrollPosition);
				++vbwIter;
				++usedVertexCount;

				// Move the texture along

				floatingAv = floatingBv;
			}
		}

		m_localShaderPrimitive->m_vertexBuffer.unlock();

		Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
		Graphics::setVertexBuffer(m_localShaderPrimitive->m_vertexBuffer);

		DEBUG_FATAL(usedVertexCount != lockedVertexCount, ("Did not use all the locked vertices. locked(%d) used(%d)", lockedVertexCount, usedVertexCount));
	}
	else
	{
		DEBUG_WARNING(true, ("The swoosh is not ready for rendering."));
	}
}

//--------------------------------------------------------------------------
float SwooshAppearance::alter(float deltaTime)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("SwooshAppearance::alter");

#ifdef _DEBUG
	if (s_currentUpdateFrame != Os::getNumberOfUpdates())
	{
		s_currentUpdateFrame = Os::getNumberOfUpdates();
		s_cullCountThisFrame = 0;
	}
#endif // _DEBUG

	m_secondsSinceLastSample += deltaTime;

	bool const pausedByTimeScale = (deltaTime <= 0.0f);
	m_lastDeltaTime = deltaTime;

	// Create start appearance

	if (   (m_startObject == NULL)
		&& !m_swooshAppearanceTemplate->getStartAppearancePath().empty())
	{
		m_startObject = new MemoryBlockManagedObject();
		m_startObject->setAppearance(AppearanceTemplateList::createAppearance(m_swooshAppearanceTemplate->getStartAppearancePath().c_str()));

		RenderWorld::addObjectNotifications(*m_startObject);
		SetupClientParticle::addNotifications(*m_startObject);
		getOwner()->addChildObject_o(m_startObject);
	}

	// Create end appearance

	if (   (m_endObject == NULL)
		&& !m_swooshAppearanceTemplate->getEndAppearancePath().empty())
	{
		m_endObject = new MemoryBlockManagedObject();
		m_endObject->setAppearance(AppearanceTemplateList::createAppearance(m_swooshAppearanceTemplate->getEndAppearancePath().c_str()));

		RenderWorld::addObjectNotifications(*m_endObject);
		SetupClientParticle::addNotifications(*m_endObject);
		getOwner()->addChildObject_o(m_endObject);
	}

	// Update the start appearance position

	if (   (m_startObject != NULL)
	    && (m_sampledPositionCount > 0))
	{
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), m_positionIndex, m_templateSamplePositionCount);
		Vector topPosition_p = getOwner()->getTransform_o2w().rotateTranslate_p2l(m_positionList[m_positionIndex].m_topPosition_w);

		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), m_positionIndex, m_templateSamplePositionCount);
		Vector bottomPosition_p = getOwner()->getTransform_o2w().rotateTranslate_p2l(m_positionList[m_positionIndex].m_bottomPosition_w);

		switch (m_swooshAppearanceTemplate->getStartAppearancePosition())
		{
			case SwooshAppearanceTemplate::AP_top:    { m_startObject->setPosition_p(topPosition_p); } break;
			default:                                  { DEBUG_FATAL(true, ("Invalid appearance position.")); }
			case SwooshAppearanceTemplate::AP_middle: { m_startObject->setPosition_p(topPosition_p - (topPosition_p - bottomPosition_p) / 2.0f); } break;
			case SwooshAppearanceTemplate::AP_bottom: { m_startObject->setPosition_p(bottomPosition_p); } break;
			case SwooshAppearanceTemplate::AP_random: { m_startObject->setPosition_p(topPosition_p - (topPosition_p - bottomPosition_p) * Random::randomReal()); } break;
		}
	}

	// Update the end appearance position

	if (   (m_endObject != NULL)
	    && (m_sampledPositionCount > 0))
	{
		unsigned int const endIndex = (m_sampledPositionCount < m_templateSamplePositionCount) ? 0u : (m_positionIndex + 1) % m_templateSamplePositionCount;

		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), endIndex, m_templateSamplePositionCount);
		Vector topPosition_p = getOwner()->getTransform_o2w().rotateTranslate_p2l(m_positionList[endIndex].m_topPosition_w);
		Vector bottomPosition_p = getOwner()->getTransform_o2w().rotateTranslate_p2l(m_positionList[endIndex].m_bottomPosition_w);

		switch (m_swooshAppearanceTemplate->getEndAppearancePosition())
		{
			case SwooshAppearanceTemplate::AP_top:    { m_endObject->setPosition_w(topPosition_p); } break;
			default:                                  { DEBUG_FATAL(true, ("Invalid appearance position.")); }
			case SwooshAppearanceTemplate::AP_middle: { m_endObject->setPosition_p(topPosition_p - (topPosition_p - bottomPosition_p) / 2.0f); } break;
			case SwooshAppearanceTemplate::AP_bottom: { m_endObject->setPosition_p(bottomPosition_p); } break;
			case SwooshAppearanceTemplate::AP_random: { m_endObject->setPosition_p(topPosition_p - (topPosition_p - bottomPosition_p) * Random::randomReal()); } break;
		}
	}

	// Update the swoosh position

	if (!pausedByTimeScale)
	{
		m_shaderScrollPosition += deltaTime * m_swooshAppearanceTemplate->getShaderScrollSpeed();

		Vector previousPosition;

		if (!m_enabled)
		{
			// No more swoosh simulation

			m_movementSpeed *= 0.75f;
		}
		else if (m_killIndex != m_templateSamplePositionCount)
		{
			// This cleans up the effect nicely

			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), m_killIndex, m_templateSamplePositionCount);

			switch (m_positionReference)
			{
				case PR_none:
				default:               { DEBUG_FATAL(true, ("Invalid position reference")); } break;
				case PR_object:
				case PR_oneHardPoint: { setPosition_w(m_positionList[m_killIndex].m_middlePosition_w); } break;
				case PR_twoHardPoints: { setPosition_w(m_positionList[m_killIndex].m_topPosition_w, m_positionList[m_killIndex].m_bottomPosition_w); } break;
			}

			if (m_killIndex == m_positionIndex)
			{
				m_enabled = false;
			}

			m_movementSpeed *= 0.75f;
		}
		else if ((m_referenceObject == NULL) && (m_positionReference == PR_object))
		{
			// The reference object has been destroyed

			kill();

			m_movementSpeed *= 0.75f;
		}
		else if ((m_sampledPositionCount > 0)
			|| (m_positionReference == PR_oneHardPoint)
			|| (m_positionReference == PR_twoHardPoints)
			|| (m_positionReference == PR_object))
		{
			m_currentTime += deltaTime;
			
			if (m_referenceObject != NULL)
			{
				setPosition_w(m_referenceObject->getPosition_w());
			}
			else if (m_hardPointAppearance != NULL)
			{
				// Get the position information from the hardpoints on the appearance

				if (!m_hardPoint1.isEmpty())
				{
					Transform hardPointTransform1;
					IGNORE_RETURN(m_hardPointAppearance->findHardpoint(m_hardPoint1, hardPointTransform1));

					Vector a(m_hardPointAppearance->getOwner()->rotateTranslate_o2w(hardPointTransform1.getPosition_p()));

					if (!m_hardPoint2.isEmpty())
					{
						Transform hardPointTransform2;
						IGNORE_RETURN(m_hardPointAppearance->findHardpoint(m_hardPoint2, hardPointTransform2));

						Vector b(m_hardPointAppearance->getOwner()->rotateTranslate_o2w(hardPointTransform2.getPosition_p()));

						setPosition_w(a, b);
					}
					else
					{
						setPosition_w(a);
					}
				}
			}
		}

		// Build the extents box

		if (m_sampledPositionCount > 1)
		{
			m_dpvsObject->set(DPVS::Object::ENABLED, true);

			AxialBox newBox;

			if (isCameraFacing())
			{
				newBox.setMin(Vector::maxXYZ);
				newBox.setMax(Vector::negativeMaxXYZ);

				// Camera facing geometry

				for (unsigned int index = 0; index < m_sampledPositionCount; ++index)
				{
					VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0u, index, m_templateSamplePositionCount);
					newBox.add(m_positionList[index].m_middlePosition_w);
				}
			}
			else if (m_positionReference == PR_twoHardPoints)
			{
				newBox.setMin(Vector::maxXYZ);
				newBox.setMax(Vector::negativeMaxXYZ);

				for (unsigned int index = 0; index < m_sampledPositionCount; ++index)
				{
					VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0u, index, m_templateSamplePositionCount);
					newBox.add(m_positionList[index].m_topPosition_w);
					newBox.add(m_positionList[index].m_bottomPosition_w);
				}
			}
			else
			{
				DEBUG_FATAL(m_positionReference != PR_none, ("Invalid position reference."));

				newBox.setMin(Vector::zero);
				newBox.setMax(Vector::zero);
			}

			bool updatedExtent = false;

			if (!m_extent_w.getBox().contains(newBox))
			{
				m_accumulatedSmallerRadiusChanges = 0;
				m_extent_w.setBox(newBox);
				updatedExtent = true;
			}
			else
			{
				++m_accumulatedSmallerRadiusChanges;

				if (m_accumulatedSmallerRadiusChanges > 16)
				{
					m_accumulatedSmallerRadiusChanges = 0;
					m_extent_w.setBox(newBox);
					updatedExtent = true;
				}
			}

			if (updatedExtent)
			{
				Sphere sphere(getOwner()->rotateTranslate_w2o(Sphere(newBox.getCenter(), newBox.getRadius())));

				{
					DPVS::SphereModel *const model = RenderWorld::fetchSphereModel(sphere);
					m_dpvsObject->setTestModel(model);
					IGNORE_RETURN(model->release());
				}
			}
		}
		else
		{
			m_dpvsObject->set(DPVS::Object::ENABLED, false);
		}
	}

	// Determine if the swoosh appearance is ready for rendering

	{
		// Determine how many valid sample points we have

		m_currentValidSamplePositionCount = 0;
		int previousValidIndex = -1;

		for (unsigned int index = 0; index < m_sampledPositionCount; ++index)
		{
			unsigned int const visualIndex = static_cast<unsigned int>(getVisualIndex(index));

			if (m_positionList[visualIndex].m_valid)
			{
				++m_currentValidSamplePositionCount;

				// For camera-facing swooshes, determine the next valid visual index

				if (isCameraFacing())
				{
					if (previousValidIndex != -1)
					{
						m_positionList[previousValidIndex].m_nextValidIndex = visualIndex;
					}

					previousValidIndex = static_cast<int>(visualIndex);
				}
			}
		}
	}

	{
		// Make sure we have sampled enough points

		if (   ((m_currentValidSamplePositionCount < 2) || (m_width > 0.0f))
			&& (m_currentValidSamplePositionCount < 3))
		{
			m_renderThisFrame = false;
		}
		else
		{
			// Get the visibility based on movement speed

			if (m_movementSpeed < SwooshAppearanceTemplate::getNoVisibilitySpeed())
			{
				m_visibilityPercent = 0.0f;
			}
			else if (m_movementSpeed > SwooshAppearanceTemplate::getFullVisibilitySpeed())
			{
				m_visibilityPercent = 1.0f;
			}
			else
			{
				float const visibilitySpeedDifference = (SwooshAppearanceTemplate::getFullVisibilitySpeed() - SwooshAppearanceTemplate::getNoVisibilitySpeed());
				m_visibilityPercent = (visibilitySpeedDifference > 0.0f) ? clamp(0.0f, (m_movementSpeed - SwooshAppearanceTemplate::getNoVisibilitySpeed()) / visibilitySpeedDifference, 1.0f) : 1.0f;
			}

			if (m_visibilityPercent <= 0.0f)
			{
				m_renderThisFrame = false;

#ifdef _DEBUG
				++s_cullCountThisFrame;
#endif // _DEBUG
			}
			else
			{
				m_renderThisFrame = true;
			}
		}
	}

	// Determine whether to alter next frame

	float result = AlterResult::cms_alterNextFrame;

	if (!isAlive())
	{
		result = AlterResult::cms_kill;
	}

	// Remember that we've been altered at least once

	m_hasBeenAltered = true;

	return result;
}

//--------------------------------------------------------------------------
Extent const *SwooshAppearance::getExtent() const
{
	return &m_extent_w;
}

//--------------------------------------------------------------------------
void SwooshAppearance::restart()
{
	m_currentTime = 0.0f;
	m_accumulatedSmallerRadiusChanges = 0;	
	m_positionIndex = 0;
	m_shaderScrollPosition = 0.0f;
	m_sampledPositionCount = 0;
	m_killIndex = NON_NULL(m_swooshAppearanceTemplate)->getSamplePositionCount();
	m_secondsSinceLastSample = 0.0f;
	m_currentValidSamplePositionCount = 0;
}

//--------------------------------------------------------------------------
Sphere const &SwooshAppearance::getSphere() const
{
	return m_extent_w.getSphere();
}

// This is set to false in the Swoosh editor so the user can move the camera
// below the ground and Umbra won't cull it out.
//--------------------------------------------------------------------------
void SwooshAppearance::setUnBounded(bool const unBounded)
{
	m_unBounded = unBounded;
	m_dpvsObject->set(DPVS::Object::UNBOUNDED, m_unBounded);
}

//--------------------------------------------------------------------------
void SwooshAppearance::drawDebugExtents() const
{
	if (getOwner() != NULL)
	{
		Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorZStaticShader());
		Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
		Graphics::drawExtent(&m_extent_w, VectorArgb::solidWhite);
	}
}

//--------------------------------------------------------------------------
void SwooshAppearance::addToWorld()
{
	Appearance::addToWorld();

	// Sound

	Object *owner = getOwner();

	if (   (owner != NULL)
	    && !Audio::isSoundValid(m_soundId)
	    && !m_swooshAppearanceTemplate->getSoundPath().empty())
	{
		DEBUG_FATAL((m_soundId.isValid()), ("The sound id (%d) for the Swoosh effect should be 0 (invalid).", m_soundId.getId()));
		m_soundId = Audio::attachSound(m_swooshAppearanceTemplate->getSoundPath().c_str(), owner);
	}
}

//--------------------------------------------------------------------------
void SwooshAppearance::removeFromWorld()
{
	Appearance::removeFromWorld();

	// Sound

	Audio::stopSound(m_soundId, 1.0f);
	m_soundId.invalidate();
}

//--------------------------------------------------------------------------
void SwooshAppearance::setColorModifier(VectorArgb const &color)
{
	m_colorModifier = color;
}

//--------------------------------------------------------------------------
void SwooshAppearance::setGlobalColorModifier(VectorArgb const &color)
{
	s_globalColorModifier = color;
}

//--------------------------------------------------------------------------
VectorArgb const &SwooshAppearance::getColorModifier() const
{
	return m_colorModifier;
}

//--------------------------------------------------------------------------
VectorArgb const &SwooshAppearance::getGlobalColorModifier() const
{
	return s_globalColorModifier;
}

//--------------------------------------------------------------------------
void SwooshAppearance::setWidth(float const width)
{
	DEBUG_FATAL((m_width <= 0.0f), ("Width (%f) must be > 0", m_width));

	m_width = width;
}

//--------------------------------------------------------------------------
void SwooshAppearance::setPosition_w(Vector const & topPosition_w, Vector const & bottomPosition_w)
{
	// Make sure enough time has elapsed till we sample again

	if (m_secondsSinceLastSample >= m_secondsBetweenSamples)
	{
		float const usableSecondsSinceLastSample = static_cast<float>(m_templateSamplePositionCount) * m_secondsBetweenSamples;

		// If too much time elapsed, clamp the time to the most we could benefit from sampling

		if (m_secondsSinceLastSample > usableSecondsSinceLastSample)
		{
			m_secondsSinceLastSample = usableSecondsSinceLastSample;
		}
		
		// Sample all the positions we missed since the last sample

		int duplicateCount = 0;

		while (m_secondsSinceLastSample >= m_secondsBetweenSamples)
		{
			// Calculate the movement speed

			if (m_sampledPositionCount > 0)
			{
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), m_positionIndex, m_templateSamplePositionCount);
				float const topMovementSpeed = (topPosition_w - m_positionList[m_positionIndex].m_topPosition_w).approximateMagnitude() * m_lastDeltaTime;
				float const bottomMovementSpeed = (bottomPosition_w - m_positionList[m_positionIndex].m_bottomPosition_w).approximateMagnitude() * m_lastDeltaTime;

				m_movementSpeed = (topMovementSpeed > bottomMovementSpeed) ? topMovementSpeed : bottomMovementSpeed;
			}
			else
			{
				m_movementSpeed = 0.0f;
			}

			if (m_positionReference == PR_none)
			{
				m_positionReference = PR_twoHardPoints;
			}

			// Mark invalid position index due to duplicates

			m_positionList[m_positionIndex].m_valid = (duplicateCount <= 0);

			// Set the current position

			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), m_positionIndex, m_templateSamplePositionCount);
			m_positionList[m_positionIndex].m_topPosition_w = topPosition_w;
			m_positionList[m_positionIndex].m_middlePosition_w = Vector::zero;
			m_positionList[m_positionIndex].m_bottomPosition_w = bottomPosition_w;

			// Move the current index along

			if (m_sampledPositionCount > 0)
			{
				if (++m_positionIndex >= m_templateSamplePositionCount)
				{
					m_positionIndex = 0;
				}
			}
			else
			{
				DEBUG_FATAL((m_positionIndex != 0), ("Position index should be 0"));
			}

			// Keep track of how many positions have been sampled so far

			if (m_sampledPositionCount < m_templateSamplePositionCount)
			{
				++m_sampledPositionCount;
			}

			m_secondsSinceLastSample -= m_secondsBetweenSamples;
			++duplicateCount;
		}
	}

	m_positionList[m_positionIndex].m_topPosition_w = topPosition_w;
	m_positionList[m_positionIndex].m_middlePosition_w = Vector::zero;
	m_positionList[m_positionIndex].m_bottomPosition_w = bottomPosition_w;
	m_positionList[m_positionIndex].m_valid = true;
}

//--------------------------------------------------------------------------
void SwooshAppearance::setPosition_w(Vector const &position_w)
{
	// Make sure enough time has elapsed till we sample again

	if (m_secondsSinceLastSample >= m_secondsBetweenSamples)
	{
		float const usableSecondsSinceLastSample = static_cast<float>(m_templateSamplePositionCount) * m_secondsBetweenSamples;

		// If too much time elapsed, clamp the time to the most we could benefit from sampling

		if (m_secondsSinceLastSample > usableSecondsSinceLastSample)
		{
			m_secondsSinceLastSample = usableSecondsSinceLastSample;
		}
		
		// Sample all the positions we missed since the last sample

		int duplicateCount = 0;

		while (m_secondsSinceLastSample >= m_secondsBetweenSamples)
		{
			// Calculate the movement speed

			if (m_sampledPositionCount > 0)
			{
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), m_positionIndex, m_templateSamplePositionCount);
				m_movementSpeed = (position_w - m_positionList[m_positionIndex].m_middlePosition_w).approximateMagnitude() * m_lastDeltaTime;
			}
			else
			{
				m_movementSpeed = 0.0f;
			}

			// Mark invalid position index due to duplicates

			m_positionList[m_positionIndex].m_valid = (duplicateCount <= 0);

			// Set the current position

			VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), m_positionIndex, m_templateSamplePositionCount);
			m_positionList[m_positionIndex].m_middlePosition_w = position_w;

			// Move the current index along

			if (m_sampledPositionCount > 0)
			{
				if (++m_positionIndex >= m_templateSamplePositionCount)
				{
					m_positionIndex = 0;
				}
			}
			else
			{
				DEBUG_FATAL((m_positionIndex != 0), ("Position index should be 0"));
			}

			// Keep track of how many positions have been samples so far

			if (m_sampledPositionCount < m_templateSamplePositionCount)
			{
				++m_sampledPositionCount;
			}

			m_secondsSinceLastSample -= m_secondsBetweenSamples;
			++duplicateCount;
		}
	}

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), m_positionIndex, m_templateSamplePositionCount);
	m_positionList[m_positionIndex].m_middlePosition_w = position_w;
	m_positionList[m_positionIndex].m_valid = true;
}

//--------------------------------------------------------------------------
void SwooshAppearance::kill()
{
	//DEBUG_REPORT_LOG(true, ("Kill: m_killIndex: %d m_positionIndex: %d\n", m_killIndex, m_positionIndex));

	if (   m_allowAutoDelete
	    && (m_killIndex == m_templateSamplePositionCount))
	{
		m_killIndex = m_positionIndex;

		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), m_killIndex, m_templateSamplePositionCount);

		// Kill any appearance attachments

		if (m_startObject != NULL)
		{
			ParticleEffectAppearance *particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(m_startObject->getAppearance());

			if (particleEffectAppearance != NULL)
			{
				particleEffectAppearance->setEnabled(false);
			}
		}

		if (m_endObject != NULL)
		{
			ParticleEffectAppearance *particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(m_endObject->getAppearance());

			if (particleEffectAppearance != NULL)
			{
				particleEffectAppearance->setEnabled(false);
			}
		}
	}
}

//--------------------------------------------------------------------------
void SwooshAppearance::setAllowAutoDelete(bool const allowAutoDelete)
{
	m_allowAutoDelete = allowAutoDelete;
}

//--------------------------------------------------------------------------
bool SwooshAppearance::realIsAlive() const
{
	bool result = true;

	if (m_allowAutoDelete)
	{
		result = m_enabled;// || ((m_startObject != NULL) && m_startObject->getAppearance()->isAlive()) || ((m_endObject != NULL) && m_endObject->getAppearance()->isAlive());
	}

	return result;
}

//--------------------------------------------------------------------------
float SwooshAppearance::getAlpha(float const initialAlpha, float const percent, SwooshAppearanceTemplate::FadeAlpha const fadeAlpha) const
{
	float result = 1.0f;

	switch (fadeAlpha)
	{
		case SwooshAppearanceTemplate::FA_count:
		default:                                { DEBUG_FATAL(true, ("Unexpected value")); }
		case SwooshAppearanceTemplate::FA_none: { result = initialAlpha; } break;
		case SwooshAppearanceTemplate::FA_0:    { result = initialAlpha - initialAlpha * (1.0f - (percent * 4.0f)); } break;
		case SwooshAppearanceTemplate::FA_1:    { result = initialAlpha - initialAlpha * (1.0f - (percent * 3.0f)); } break;
		case SwooshAppearanceTemplate::FA_2:    { result = initialAlpha - initialAlpha * (1.0f - (percent * 2.0f)); } break;
		case SwooshAppearanceTemplate::FA_3:    { result = initialAlpha - initialAlpha * (1.0f - (percent * 1.0f)); } break;
		case SwooshAppearanceTemplate::FA_4:    { result = initialAlpha - initialAlpha * (1.0f - (percent * percent)); } break;
		case SwooshAppearanceTemplate::FA_5:    { result = initialAlpha - initialAlpha * (1.0f - (percent * percent * percent)); } break;
		case SwooshAppearanceTemplate::FA_6:    { result = initialAlpha - initialAlpha * (1.0f - (percent * percent * percent * percent)); } break;
		case SwooshAppearanceTemplate::FA_7:    { result = initialAlpha - initialAlpha * (1.0f - (percent * percent * percent * percent * percent)); } break;
		case SwooshAppearanceTemplate::FA_8:    { result = initialAlpha - initialAlpha * (1.0f - (percent * percent * percent * percent * percent * percent)); } break;
		case SwooshAppearanceTemplate::FA_9:    { result = initialAlpha - initialAlpha * (1.0f - (percent * percent * percent * percent * percent * percent * percent)); } break;
	}

	return clamp(0.0f, result, initialAlpha);
}

//--------------------------------------------------------------------------
void SwooshAppearance::getTaperGeometry(float const percent, SwooshAppearanceTemplate::TaperGeometry const taperGeometry, Vector const &topPosition_w, Vector const &bottomPosition_w, Vector &result) const
{
	Vector const &top = topPosition_w;
	Vector const &bottom = bottomPosition_w;

	switch (taperGeometry)
	{
		case SwooshAppearanceTemplate::TG_count:
		default:                                { DEBUG_FATAL(true, ("Unexpected value")); }
		case SwooshAppearanceTemplate::TG_none: { result = bottom; } break;
		case SwooshAppearanceTemplate::TG_0:    { result = top + (bottom - top) * clamp(0.0f, percent * 4.0f, 1.0f); } break;
		case SwooshAppearanceTemplate::TG_1:    { result = top + (bottom - top) * clamp(0.0f, percent * 3.0f, 1.0f); } break;
		case SwooshAppearanceTemplate::TG_2:    { result = top + (bottom - top) * clamp(0.0f, percent * 2.0f, 1.0f); } break;
		case SwooshAppearanceTemplate::TG_3:    { result = top + (bottom - top) * percent * 1.0f; } break;
		case SwooshAppearanceTemplate::TG_4:    { result = top + (bottom - top) * percent * percent; } break;
		case SwooshAppearanceTemplate::TG_5:    { result = top + (bottom - top) * percent * percent * percent * percent; } break;
		case SwooshAppearanceTemplate::TG_6:    { result = top + (bottom - top) * percent * percent * percent * percent * percent * percent; } break;
		case SwooshAppearanceTemplate::TG_7:    { result = top + (bottom - top) * percent * percent * percent * percent * percent * percent * percent * percent; } break;
		case SwooshAppearanceTemplate::TG_8:    { result = top + (bottom - top) * percent * percent * percent * percent * percent * percent * percent * percent * percent * percent; } break;
		case SwooshAppearanceTemplate::TG_9:    { result = top + (bottom - top) * percent * percent * percent * percent * percent * percent * percent * percent * percent * percent * percent * percent; } break;
	}
}

//--------------------------------------------------------------------------
void SwooshAppearance::setReferenceObject(Object const *referenceObject)
{
	m_positionReference = PR_object;
	m_referenceObject = referenceObject;
}

//--------------------------------------------------------------------------
unsigned int SwooshAppearance::getPositionSampleCount() const
{
	return m_sampledPositionCount;
}

//--------------------------------------------------------------------------
Vector const & SwooshAppearance::getTopPosition_w(unsigned int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), index, m_templateSamplePositionCount);

	return m_positionList[index].m_topPosition_w;
}

//--------------------------------------------------------------------------
Vector const & SwooshAppearance::getMiddlePosition_w(unsigned int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), index, m_templateSamplePositionCount);

	return m_positionList[index].m_middlePosition_w;
}

//--------------------------------------------------------------------------
Vector const & SwooshAppearance::getBottomPosition_w(unsigned int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(static_cast<unsigned int>(0), index, m_templateSamplePositionCount);

	return m_positionList[index].m_bottomPosition_w;
}

//--------------------------------------------------------------------------
bool SwooshAppearance::isCameraFacing() const
{
	return (m_positionReference == PR_object) || (m_positionReference == PR_oneHardPoint);
}

//--------------------------------------------------------------------------
int SwooshAppearance::getVisualIndex(unsigned int const index) const
{
	if (m_sampledPositionCount != 0)
		return static_cast<int>(((m_sampledPositionCount + m_positionIndex) - index) % m_sampledPositionCount);
	else
		return 0;
}

#ifdef _DEBUG
//--------------------------------------------------------------------------
int SwooshAppearance::getGlobalCount()
{
	return s_globalCount;
}

//--------------------------------------------------------------------------
int SwooshAppearance::getCullCountThisFrame()
{
	return s_cullCountThisFrame;
}
#endif // _DEBUG

//--------------------------------------------------------------------------
SwooshAppearance * SwooshAppearance::asSwooshAppearance()
{
	return this;
}

//--------------------------------------------------------------------------
SwooshAppearance const * SwooshAppearance::asSwooshAppearance() const
{
	return this;
}

//--------------------------------------------------------------------------
SwooshAppearance * SwooshAppearance::asSwooshAppearance(Appearance * appearance)
{
	return (appearance != NULL) ? appearance->asSwooshAppearance() : NULL;
}

//--------------------------------------------------------------------------
SwooshAppearance const * SwooshAppearance::asSwooshAppearance(Appearance const * appearance)
{
	return (appearance != NULL) ? appearance->asSwooshAppearance() : NULL;
}

// ============================================================================
