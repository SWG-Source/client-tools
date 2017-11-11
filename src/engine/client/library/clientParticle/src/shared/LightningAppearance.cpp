// ============================================================================
//
// LightningAppearance.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/LightningAppearance.h"

#include "clientAudio/Audio.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientParticle/LightningAppearanceTemplate.h"
#include "clientParticle/LightningManager.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/ParticleManager.h"
#include "clientParticle/ParticleTexture.h"
#include "clientParticle/SetupClientParticle.h"
#include "dpvsModel.hpp"
#include "dpvsObject.hpp"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Misc.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/TerrainObject.h"

// ============================================================================
//
// LightningAppearanceNamespace
//
// ============================================================================

namespace LightningAppearanceNamespace
{
	VectorArgb s_globalColorModifier(1.0f, 1.0f, 1.0f, 1.0f);
};

using namespace LightningAppearanceNamespace;

// ============================================================================
//
// LightningAppearance::LocalShaderPrimitive
//
// ============================================================================

//-----------------------------------------------------------------------------
class LightningAppearance::LocalShaderPrimitive : public ShaderPrimitive
{
friend class LightningAppearance;

public:

	static VertexBufferFormat LocalShaderPrimitive::getVertexBufferFormat();

public:

	explicit LocalShaderPrimitive(LightningAppearance const &LightningAppearance);
	virtual ~LocalShaderPrimitive();

	virtual float               alter(float time);
	virtual const Vector        getPosition_w() const;
	virtual const StaticShader &prepareToView() const;
	virtual float               getDepthSquaredSortKey() const;
	virtual int                 getVertexBufferSortKey() const;
	virtual void                prepareToDraw() const;
	virtual void                draw() const;

private:

	LightningAppearance const &m_lightningAppearance;
	DynamicVertexBuffer        m_vertexBuffer;
	Camera const *             m_camera;
	Object const *             m_object;
	Shader const *             m_shader;
	float                      m_depthSquaredSortKey;

private:

	// Disabled

	LocalShaderPrimitive();
	LocalShaderPrimitive(LocalShaderPrimitive const &);
	LocalShaderPrimitive &operator=(LocalShaderPrimitive const &);
};

//-----------------------------------------------------------------------------
VertexBufferFormat LightningAppearance::LocalShaderPrimitive::getVertexBufferFormat()
{
	VertexBufferFormat format;

	format.setPosition();
	format.setColor0();
	format.setNumberOfTextureCoordinateSets(1);
	format.setTextureCoordinateSetDimension(0, 2);

	return format;
}

//-----------------------------------------------------------------------------
LightningAppearance::LocalShaderPrimitive::LocalShaderPrimitive(LightningAppearance const &lightningAppearance)
 : ShaderPrimitive()
 , m_lightningAppearance(lightningAppearance)
 , m_vertexBuffer(getVertexBufferFormat())
 , m_camera(NULL)
 , m_object(NULL)
 , m_shader(NULL)
 , m_depthSquaredSortKey(0.0f)
{
	ParticleTexture const &particleTexture = m_lightningAppearance.m_lightningAppearanceTemplate->getParticleTexture();

	if (   particleTexture.isTextureVisible()
	    && !particleTexture.getShaderPath().isEmpty())
	{
		m_shader = ShaderTemplateList::fetchShader(particleTexture.getShaderPath().getString());
	}
}

//-----------------------------------------------------------------------------
LightningAppearance::LocalShaderPrimitive::~LocalShaderPrimitive()
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
float LightningAppearance::LocalShaderPrimitive::alter(real time)
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
StaticShader const &LightningAppearance::LocalShaderPrimitive::prepareToView() const
{
	StaticShader const &result = ((m_shader != NULL) && ParticleManager::isTexturingEnabled()) ? m_shader->prepareToView() : ShaderTemplateList::get3dVertexColorACStaticShader();

	return result;
}

//-----------------------------------------------------------------------------
const Vector LightningAppearance::LocalShaderPrimitive::getPosition_w() const
{
	return (m_object != NULL) ? m_object->getPosition_w() : Vector::zero;
}
	
//-------------------------------------------------------------------

float LightningAppearance::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return m_depthSquaredSortKey;
}

//-----------------------------------------------------------------------------
int LightningAppearance::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return m_vertexBuffer.getSortKey();
}

//-----------------------------------------------------------------------------
void LightningAppearance::LocalShaderPrimitive::prepareToDraw() const
{
	if (m_lightningAppearance.getActiveBoltCount() > 0)
		m_lightningAppearance.renderGeometry();
}

//-----------------------------------------------------------------------------
void LightningAppearance::LocalShaderPrimitive::draw() const
{
	if (m_lightningAppearance.getActiveBoltCount() > 0)
		Graphics::drawQuadList();
}

// ============================================================================
//
// LightningAppearance::LightningBoltData
//
// ============================================================================

//-----------------------------------------------------------------------------
LightningAppearance::LightningBoltData::LightningBoltData()
 : m_enabled(false)
 , m_largeBolt(false)
 , m_morphPositionOffsetList_o()
 , m_positionList_w()
 , m_lightningBoltIndex(0)
 , m_lightningBoltIndexTimer(0.0f)
 , m_startPosition_w()
 , m_endPosition_w()
 , m_textureFrame(0)
 , m_textureFrameTimer(0.0f)
 , m_startObject(NULL)
 , m_endObject(NULL)
{
}

//-----------------------------------------------------------------------------
LightningAppearance::LightningBoltData::~LightningBoltData()
{
	delete m_startObject;
	m_startObject = NULL;

	delete m_endObject;
	m_endObject = NULL;
}

// ============================================================================
//
// LightningAppearance
//
// ============================================================================

#ifdef _DEBUG
int LightningAppearance::m_globalCount = 0;
#endif // _DEBUG

//--------------------------------------------------------------------------
LightningAppearance::LightningAppearance(LightningAppearanceTemplate const *lightningAppearanceTemplate)
 : Appearance(lightningAppearanceTemplate)
 , m_lightningAppearanceTemplate(lightningAppearanceTemplate)
 , m_localShaderPrimitive(NULL)
 , m_currentTime(0.0f)
 , m_timeScale(1.0f)
 , m_paused(false)
 , m_enabled(true)
 , m_extent_w()
 , m_accumulatedSmallerRadiusChanges(0)
 , m_unBounded(false)
 , m_dpvsObject(NULL)
 , m_currentCameraPosition_w()
 , m_hasBeenAltered(false)
 , m_activeBoltCount(0)
 , m_soundId()
 , m_colorModifier(VectorArgb::solidWhite)
 , m_largeBoltShaderScrollPosition(0.0f)
 , m_smallBoltShaderScrollPosition(0.0f)
{
#ifdef _DEBUG
	++m_globalCount;
#endif // _DEBUG

	m_localShaderPrimitive = new LocalShaderPrimitive(*this);

	// Set the extent initial value, maybe on the first frame of ths effect, set the extent to the object position

	m_extent_w.setMin(Vector::zero);
	m_extent_w.setMax(Vector::zero);

	m_dpvsObject = RenderWorld::createObject(this, m_extent_w.getBox());
}

//--------------------------------------------------------------------------
LightningAppearance::~LightningAppearance()
{
	delete m_localShaderPrimitive;
	m_localShaderPrimitive = NULL;

	m_lightningAppearanceTemplate = NULL;

#ifdef _DEBUG
	--m_globalCount;
#endif // _DEBUG

	IGNORE_RETURN(m_dpvsObject->release());
	m_dpvsObject = NULL;
}

//--------------------------------------------------------------------------
DPVS::Object * LightningAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

//--------------------------------------------------------------------------
void LightningAppearance::render() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("LightningAppearance::render");

	// Make sure particles are enabled

	m_currentCameraPosition_w = ShaderPrimitiveSorter::getCurrentCameraPosition();

	m_localShaderPrimitive->m_camera = &ShaderPrimitiveSorter::getCurrentCamera();
	m_localShaderPrimitive->m_object = getOwner();
	m_localShaderPrimitive->m_depthSquaredSortKey = m_extent_w.getSphere().getCenter().magnitudeBetweenSquared(m_currentCameraPosition_w);

	// Add the primitive to be rendered at the appropriate time

	ShaderPrimitiveSorter::add(*m_localShaderPrimitive);

#ifdef _DEBUG
	// Draw extents

	if (ParticleManager::isDebugExtentsEnabled())
	{
		drawDebugExtents();
	}
#endif // _DEBUG
}

//--------------------------------------------------------------------------
void LightningAppearance::renderGeometry() const
{
	// Get the number of enabled bolts

	int boltCount = 0;

	{
		for (unsigned int index = 0; index < m_maxLightningBoltCount; ++index)
		{
			if (m_lightningBoltDataList[index].m_enabled)
			{
				++boltCount;
			}
		}
	}

	// If there are no bolts active, then don't render anything

	if (boltCount == 0)
	{
		return;
	}

	// Render all the active bolts
	Vector const & scale = getScale();
	float const largeBoltStretchDistanceSquared = sqr(m_lightningAppearanceTemplate->getLargeBoltShaderStretchDistance() * scale.x);
	float const smallBoltStretchDistanceSquared = sqr(m_lightningAppearanceTemplate->getSmallBoltShaderStretchDistance() * scale.x);

	m_localShaderPrimitive->m_vertexBuffer.lock((LightningBolt::getPositionCount() - 1) * 4 * boltCount);
	{
		VertexBufferWriteIterator vbwIter = m_localShaderPrimitive->m_vertexBuffer.begin();

		for (unsigned int index = 0; index < m_maxLightningBoltCount; ++index)
		{
			LightningBoltData const &lightningBoltData = m_lightningBoltDataList[index];

			if (!lightningBoltData.m_enabled)
			{
				continue;
			}

			float thickness = 1.0f;
			float scrollPosition = 0.0f;
			float stretchDistance = 1.0f;
			
			if (lightningBoltData.m_largeBolt)
			{
				thickness = (m_lightningAppearanceTemplate->getLargeBoltThickness() * scale.x) / 2.0f;
				scrollPosition = m_largeBoltShaderScrollPosition;
				stretchDistance = largeBoltStretchDistanceSquared;
			}
			else
			{
				thickness = (m_lightningAppearanceTemplate->getSmallBoltThickness() * scale.x) / 2.0f;
				scrollPosition = m_smallBoltShaderScrollPosition;
				stretchDistance = smallBoltStretchDistanceSquared;
			}

			if (thickness <= 0.01f)
			{
				thickness = 0.01f;
			}

			// Get the RGB

			float alpha = lightningBoltData.m_largeBolt ? m_lightningAppearanceTemplate->getLargeBoltAlpha() : m_lightningAppearanceTemplate->getSmallBoltAlpha();
			float red = alpha * (lightningBoltData.m_largeBolt ? m_lightningAppearanceTemplate->getLargeBoltColor().r : m_lightningAppearanceTemplate->getSmallBoltColor().r);
			float green = alpha * (lightningBoltData.m_largeBolt ? m_lightningAppearanceTemplate->getLargeBoltColor().g : m_lightningAppearanceTemplate->getSmallBoltColor().g);
			float blue = alpha * (lightningBoltData.m_largeBolt ? m_lightningAppearanceTemplate->getLargeBoltColor().b : m_lightningAppearanceTemplate->getSmallBoltColor().b);

			alpha *= m_colorModifier.a * s_globalColorModifier.a;
			red *= m_colorModifier.r * s_globalColorModifier.r;
			green *= m_colorModifier.g * s_globalColorModifier.g;
			blue *= m_colorModifier.b * s_globalColorModifier.b;

			Vector vPrevious;
			Vector vCurrent(lightningBoltData.m_positionList_w[0] + lightningBoltData.m_morphPositionOffsetList_o[0]);
			Vector vNext(lightningBoltData.m_positionList_w[1] + lightningBoltData.m_morphPositionOffsetList_o[1]);
			Vector currentBankVector;
			Vector a;
			Vector d;
			float drawDistanceTotalSquared = 0.0f;
			int currentTextureFrame = lightningBoltData.m_textureFrame;
			int previousTextureFrame = -1;
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

			for (unsigned int positionIndex = 1; positionIndex < LightningBolt::getPositionCount(); ++positionIndex)
			{
				vPrevious = vCurrent;
				vCurrent = vNext;
				
				if (positionIndex < (LightningBolt::getPositionCount() - 1))
				{
					vNext = lightningBoltData.m_positionList_w[positionIndex + 1] + lightningBoltData.m_morphPositionOffsetList_o[positionIndex + 1];
				}
				else
				{
					vNext = vCurrent;
				}

				PackedArgb color(VectorArgb(alpha, red, green, blue));

				// Get the vector for the current quad

				Vector nextBankVector;

				if (positionIndex < (LightningBolt::getPositionCount() - 1))
				{
					Vector nextDirectionToCamera(m_localShaderPrimitive->m_camera->getPosition_w() - vNext);
					nextBankVector = (vNext - vCurrent).cross(nextDirectionToCamera);
					nextBankVector.approximateNormalize();
				}
				else
				{
					nextBankVector = currentBankVector;
				}

				Vector const e(vCurrent + (nextBankVector * thickness));
				Vector const f(vCurrent - (nextBankVector * thickness));

				// Set the geometry positions

				if (positionIndex == 1)
				{
					Vector currentDirectionToCamera(m_localShaderPrimitive->m_camera->getPosition_w() - vCurrent);
					currentBankVector = (vCurrent - vPrevious).cross(currentDirectionToCamera);
					currentBankVector.approximateNormalize();

					a = vPrevious + (currentBankVector * thickness);
					d = vPrevious - (currentBankVector * thickness);
				}

				Vector b(vCurrent + (currentBankVector * thickness));
				Vector c(vCurrent - (currentBankVector * thickness));

				b = (b + (e - b) / 2.0f);
				c = (c + (f - c) / 2.0f);

				// Only get the new texture coordinates when the current texture index is used up

				if (floatingAv == bv)
				{
					m_lightningAppearanceTemplate->getParticleTexture().getFrameUVs(currentTextureFrame, au, av, bu, bv, cu, cv, du, dv);
					previousTextureFrame = currentTextureFrame;
					floatingAv = av;
				}

				float const currentDistanceSquared = (vPrevious - vCurrent).magnitudeSquared();

				drawDistanceTotalSquared += currentDistanceSquared;

				if (   (drawDistanceTotalSquared > stretchDistance)
				    || ((stretchDistance - drawDistanceTotalSquared) < (currentDistanceSquared * 0.5f)))
				{
					drawDistanceTotalSquared = 0.0f;
					floatingBv = bv;

					if (++currentTextureFrame > m_lightningAppearanceTemplate->getParticleTexture().getFrameEnd())
					{
						currentTextureFrame = m_lightningAppearanceTemplate->getParticleTexture().getFrameStart();
					}
				}
				else
				{
					floatingBv = clamp(0.0f, av - ((av - bv) * (drawDistanceTotalSquared / stretchDistance)), 1.0f);
				}

				// clockwise order
				//
				// b - c
				// |   |
				// a - d
				
				// Add the vertices to the vertex buffer

				vbwIter.setPosition(a);
				vbwIter.setColor0(color);
				vbwIter.setTextureCoordinates(0, du, floatingAv + scrollPosition);
				++vbwIter;

				vbwIter.setPosition(b);
				vbwIter.setColor0(color);
				vbwIter.setTextureCoordinates(0, cu, floatingBv + scrollPosition);
				++vbwIter;

				vbwIter.setPosition(c);
				vbwIter.setColor0(color);
				vbwIter.setTextureCoordinates(0, bu, floatingBv + scrollPosition);
				++vbwIter;

				vbwIter.setPosition(d);
				vbwIter.setColor0(color);
				vbwIter.setTextureCoordinates(0, au, floatingAv + scrollPosition);
				++vbwIter;

				a = b;
				d = c;
				currentBankVector = nextBankVector;
				floatingAv = floatingBv;
			}
		}
	}
	m_localShaderPrimitive->m_vertexBuffer.unlock();
	
	Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
	Graphics::setVertexBuffer(m_localShaderPrimitive->m_vertexBuffer);
}

//--------------------------------------------------------------------------
float LightningAppearance::alter(float deltaTime)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("LightningAppearance::alter");

	// Don't alter the particle system if it is infinite looping and 
	// it was not rendered this frame

	if (!getRenderedThisFrame() && m_hasBeenAltered)
	{
		if (m_currentTime > 20.0f)
		{
			return AlterResult::cms_alterNextFrame;
		}
		else
		{
			return AlterResult::cms_alterNextFrame;
		}
	}

	bool const pausedByTimeScale = ((deltaTime * getTimeScale()) <= 0.0f);

	if (!m_paused && !pausedByTimeScale)
	{
		Vector const & scale = getScale();

		m_currentTime += deltaTime * getTimeScale();

		LightningManager::alter(deltaTime);

		AxialBox newBox;

		m_largeBoltShaderScrollPosition += deltaTime * m_lightningAppearanceTemplate->getLargeBoltShaderScrollSpeed() * scale.x;

		// This is to prevent insanely high texture coordinates

		if (m_largeBoltShaderScrollPosition > 1.0f)
		{
			m_largeBoltShaderScrollPosition -= 1.0f;
		}

		if (m_largeBoltShaderScrollPosition > 4096.0f)
		{
			m_largeBoltShaderScrollPosition = 0.0f;
		}

		m_smallBoltShaderScrollPosition += deltaTime * m_lightningAppearanceTemplate->getSmallBoltShaderScrollSpeed() * scale.x;

		// This is to prevent insanely high texture coordinates

		if (m_smallBoltShaderScrollPosition > 1.0f)
		{
			m_smallBoltShaderScrollPosition -= 1.0f;
		}

		if (m_smallBoltShaderScrollPosition > 4096.0f)
		{
			m_smallBoltShaderScrollPosition = 0.0f;
		}

		// Update update each enabled lightning bolt

		float const shaderFramesPerSecond = m_lightningAppearanceTemplate->getParticleTexture().getFramesPerSecond();
		float const textureFrameTime = (shaderFramesPerSecond > 1.0f) ? (1.0f / shaderFramesPerSecond) : 1.0f;

		unsigned int boltIndex = 0;
		for (boltIndex = 0; boltIndex < m_maxLightningBoltCount; ++boltIndex)
		{
			LightningBoltData &lightningBoltData = m_lightningBoltDataList[boltIndex];

			if (!lightningBoltData.m_enabled)
			{
				continue;
			}

			// As the lightning bolt gets shorter in length, the amplitude should be scaled

			float const boltLength = (lightningBoltData.m_startPosition_w - lightningBoltData.m_endPosition_w).approximateMagnitude();
			float amplitudeScale = 1.0f;
			float const amplitudeScaleDistance = 15.0f;

			if (boltLength < amplitudeScaleDistance)
			{
				amplitudeScale = boltLength / amplitudeScaleDistance;

				if (amplitudeScale < 0.01f)
				{
					amplitudeScale = 0.01f;
				}
			}

			// Throw out any large delta times
			
			if (deltaTime < 1.0f)
			{
				BoxExtent extent_w;
			
				// Invalidate the extents to be populated shortly with new extents

				extent_w.setMin(Vector::maxXYZ);
				extent_w.setMax(Vector::negativeMaxXYZ);
			
				LightningBolt const &lightningBolt = LightningManager::getLightningBolt(lightningBoltData.m_lightningBoltIndex);
			
				// Update the morph bolt target offsets

				lightningBoltData.m_lightningBoltIndexTimer += deltaTime;
			
				float const lightningBoltIndexTime = (1.0f / 12.0f);
			
				if (lightningBoltData.m_lightningBoltIndexTimer > lightningBoltIndexTime)
				{
					lightningBoltData.m_lightningBoltIndexTimer -= lightningBoltIndexTime;
			
					lightningBoltData.m_lightningBoltIndex = rand() % LightningManager::getLightningBoltCount();
				}
			
				// Update the texture frame

				lightningBoltData.m_textureFrameTimer += deltaTime;
				
				if (lightningBoltData.m_textureFrameTimer > textureFrameTime)
				{
					lightningBoltData.m_textureFrameTimer -= textureFrameTime;
				
					if (++lightningBoltData.m_textureFrame > m_lightningAppearanceTemplate->getParticleTexture().getFrameEnd())
					{
						lightningBoltData.m_textureFrame = m_lightningAppearanceTemplate->getParticleTexture().getFrameStart();
					}
				}

				// Save all the world positions for this bolt this frame

				{
					float const arcUp = (lightningBoltData.m_largeBolt ? m_lightningAppearanceTemplate->getLargeBoltArc() : m_lightningAppearanceTemplate->getSmallBoltArc()) * scale.x;

					for (unsigned int positionIndex = 0; positionIndex < LightningBolt::getPositionCount(); ++positionIndex)
					{
						float const t = static_cast<float>(positionIndex) / static_cast<float>(LightningBolt::getPositionCount() - 1);
						lightningBoltData.m_positionList_w[positionIndex] = lightningBolt.getPosition_w(lightningBoltData.m_startPosition_w, lightningBoltData.m_endPosition_w, t, arcUp);
					}
				}

				// Get the chaos

				float const chaos = (lightningBoltData.m_largeBolt ? m_lightningAppearanceTemplate->getLargeBoltChaos() : m_lightningAppearanceTemplate->getSmallBoltChaos());

				// I have to ensure small delta times because the visual breaks
				// when large delta time are fed into the system
			
				float deltaTimeLeft = deltaTime;
				float const maxDeltaTime = 0.03f;
				
				while (deltaTimeLeft > 0.0f)
				{
					float const delta = (deltaTimeLeft < maxDeltaTime) ? deltaTimeLeft : maxDeltaTime;
			
					WaveForm const &amplitudeWaveForm = (lightningBoltData.m_largeBolt) ? m_lightningAppearanceTemplate->getLargeBoltAmplitude() : m_lightningAppearanceTemplate->getSmallBoltAmplitude();
					
					WaveFormControlPointIter iterAmplitude;
					iterAmplitude.reset(amplitudeWaveForm.getIteratorBegin());
					
					float const floatPositionCount = static_cast<float>(LightningBolt::getPositionCount());
					
					for (unsigned int positionIndex = 0; positionIndex < LightningBolt::getPositionCount(); ++positionIndex)
					{
						float const percent = static_cast<float>(positionIndex) / floatPositionCount;
						float const amplitude = amplitudeWaveForm.getValue(iterAmplitude, percent) * amplitudeScale * scale.x;

						Vector const &previousPosition = lightningBoltData.m_morphPositionOffsetList_o[positionIndex];
						Vector destinationPosition = lightningBolt.getPositionOffset(positionIndex) * amplitude;
						Vector morphPosition(previousPosition + (destinationPosition - previousPosition) * delta * 5.0f * chaos);
						
						lightningBoltData.m_morphPositionOffsetList_o[positionIndex] = morphPosition;
						
						extent_w.updateMinAndMax(morphPosition + lightningBoltData.m_positionList_w[positionIndex]);
					}
			
					deltaTimeLeft -= delta;
				}
			
				// Add the extent of this bolt
			
				newBox.add(extent_w.getBox());
			}
			else
			{
				newBox.add(lightningBoltData.m_startPosition_w);
				newBox.add(lightningBoltData.m_endPosition_w);
			}
		}
		
		if (m_activeBoltCount > 0)
		{
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

		// Position the start/end appearances to the bolts

		for (boltIndex = 0; boltIndex < m_maxLightningBoltCount; ++boltIndex)
		{
			LightningBoltData &lightningBoltData = m_lightningBoltDataList[boltIndex];

			if (!lightningBoltData.m_enabled)
			{
				continue;
			}

			if (m_lightningAppearanceTemplate->getStartAppearanceCount() == LightningAppearanceTemplate::AC_onePerBolt)
			{
				if (lightningBoltData.m_startObject != NULL)
				{
					Vector const &offset_o = lightningBoltData.m_morphPositionOffsetList_o[0];

					lightningBoltData.m_startObject->setPosition_w(lightningBoltData.m_startPosition_w + offset_o);
				}
			}
			else if (boltIndex == 0)
			{
				if (lightningBoltData.m_startObject != NULL)
				{
					lightningBoltData.m_startObject->setPosition_w(getAverageStartPosition());
				}
			}

			if (m_lightningAppearanceTemplate->getEndAppearanceCount() == LightningAppearanceTemplate::AC_onePerBolt)
			{
				if (lightningBoltData.m_endObject != NULL)
				{
					Vector const &offset_o = lightningBoltData.m_morphPositionOffsetList_o[LightningBolt::getPositionCount() - 1];

					lightningBoltData.m_endObject->setPosition_w(lightningBoltData.m_endPosition_w + offset_o);
				}
			}
			else if (boltIndex == 0)
			{
				if (lightningBoltData.m_endObject != NULL)
				{
					lightningBoltData.m_endObject->setPosition_w(getAverageEndPosition());
				}
			}
			
			//-- scale the start and end appearances
			
			if (lightningBoltData.m_startObject != NULL)
			{
				Appearance * const appearance = lightningBoltData.m_startObject->getAppearance();
				if (appearance != NULL)
				{
					appearance->setUniformScale(getScale().x);
					ParticleEffectAppearance * const particleAppearance = ParticleEffectAppearance::asParticleEffectAppearance(appearance);
					if (particleAppearance != NULL)
						particleAppearance->setColorModifier(m_colorModifier);
				}
			}
			
			if (lightningBoltData.m_endObject != NULL)
			{
				Appearance * const appearance = lightningBoltData.m_endObject->getAppearance();
				if (appearance != NULL)
				{
					appearance->setUniformScale(getScale().x);
					ParticleEffectAppearance * const particleAppearance = ParticleEffectAppearance::asParticleEffectAppearance(appearance);
					if (particleAppearance != NULL)
						particleAppearance->setColorModifier(m_colorModifier);
				}
			}
		}
	}
	
	// Set the appearance extents

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
Extent const *LightningAppearance::getExtent() const
{
	return &m_extent_w;
}

//--------------------------------------------------------------------------
void LightningAppearance::restart()
{
	// Reset the time

	m_currentTime = 0.0f;
}

//--------------------------------------------------------------------------
bool LightningAppearance::isDeletable() const
{
	bool result = true;

	return result;
}

//--------------------------------------------------------------------------
void LightningAppearance::setEnabled(bool const enabled)
{
	m_enabled = enabled;
}

//--------------------------------------------------------------------------
void LightningAppearance::setTimeScale(float const timeScale)
{
	DEBUG_FATAL((timeScale < 0.0f), ("The lightning appearance timeScale rate must be >= 0."));

	m_timeScale = timeScale;
}

//--------------------------------------------------------------------------
Sphere const &LightningAppearance::getSphere() const
{
	return m_extent_w.getSphere();
}

#ifdef _DEBUG
//--------------------------------------------------------------------------
int LightningAppearance::getGlobalCount()
{
	return m_globalCount;
}
#endif // _DEBUG

//--------------------------------------------------------------------------
bool LightningAppearance::isEnabled() const
{
	return m_enabled;
}

//--------------------------------------------------------------------------
bool LightningAppearance::isPaused() const
{
	return m_paused;
}

//--------------------------------------------------------------------------
void LightningAppearance::setPaused(bool const paused)
{
	m_paused = paused;
}

// This is set to false in the lightning editor so the user can move the camera
// below the ground and Umbra won't cull it out.
//--------------------------------------------------------------------------
void LightningAppearance::setUnBounded(bool const unBounded)
{
	m_unBounded = unBounded;
	m_dpvsObject->set(DPVS::Object::UNBOUNDED, m_unBounded);
}

//--------------------------------------------------------------------------
float LightningAppearance::getTimeScale() const
{
	return m_timeScale;
}

//--------------------------------------------------------------------------
void LightningAppearance::drawDebugExtents() const
{
	if (getOwner() != NULL)
	{
		Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorZStaticShader());
		Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
		Graphics::drawExtent(&m_extent_w, VectorArgb::solidWhite);
	}
}

//--------------------------------------------------------------------------
void LightningAppearance::addToWorld()
{
	Appearance::addToWorld();

	setEnabled(true);

	// Sound

	Object *owner = getOwner();

	if (   (owner != NULL)
	    && !Audio::isSoundValid(m_soundId)
	    && !m_lightningAppearanceTemplate->getSoundPath().empty())
	{
		DEBUG_FATAL((m_soundId.isValid()), ("The sound id (%d) for the lightning effect should be 0 (invalid).", m_soundId.getId()));
		m_soundId = Audio::attachSound(m_lightningAppearanceTemplate->getSoundPath().c_str(), owner);
	}
}

//--------------------------------------------------------------------------
void LightningAppearance::removeFromWorld()
{
	Appearance::removeFromWorld();

	setEnabled(false);

	// Sound

	Audio::stopSound(m_soundId, 1.0f);
	m_soundId.invalidate();
}

//--------------------------------------------------------------------------
void LightningAppearance::setPosition_w(int const index, Vector const &startPosition_w, Vector const &endPosition_w)
{
	DEBUG_FATAL((index < 0) || (index >= m_maxLightningBoltCount), ("Out of range"));
	DEBUG_FATAL(!getOwner(), ("LightingAppearance must have an owner object."));

	unsigned int min = 0;
	unsigned int max = static_cast<unsigned int>(m_maxLightningBoltCount) - 1;
	unsigned int clampedIndex = clamp(min, static_cast<unsigned int>(index), max);

	LightningBoltData &lightningBoltData = m_lightningBoltDataList[clampedIndex];

	if (!lightningBoltData.m_enabled)
	{
		lightningBoltData.m_enabled = true;
		++m_activeBoltCount;

		DEBUG_FATAL((m_activeBoltCount > m_maxLightningBoltCount), ("Out of range"));
		m_activeBoltCount = clamp(0, m_activeBoltCount, static_cast<int>(m_maxLightningBoltCount));

		if (m_lightningAppearanceTemplate != NULL)
		{
			float const largeBoltPercent = m_lightningAppearanceTemplate->getLargeBoltPercent();
			float currentPercent = 0.0f;
			
			for (unsigned int index = 0; index < static_cast<unsigned int>(m_activeBoltCount); ++index)
			{
				if (currentPercent < largeBoltPercent)
				{
					m_lightningBoltDataList[index].m_largeBolt = true;
				}
				else
				{
					m_lightningBoltDataList[index].m_largeBolt = false;
				}
				
				currentPercent += 1.0f / static_cast<float>(m_activeBoltCount);
			}
			
			// Start appearance
			
			std::string const &startAppearancePath = m_lightningAppearanceTemplate->getStartAppearancePath();
			
			if (   !startAppearancePath.empty()
				&& (lightningBoltData.m_startObject == NULL))
			{
				lightningBoltData.m_startObject = new MemoryBlockManagedObject();
				Appearance * const appearance = AppearanceTemplateList::createAppearance(startAppearancePath.c_str());
				appearance->setUniformScale(getScale().x);
				lightningBoltData.m_startObject->setAppearance(appearance);
				
				RenderWorld::addObjectNotifications(*lightningBoltData.m_startObject);
				SetupClientParticle::addNotifications(*lightningBoltData.m_startObject);
				getOwner()->addChildObject_o(lightningBoltData.m_startObject);
			}
			
			// End appearance
			
			std::string const &endAppearancePath = m_lightningAppearanceTemplate->getEndAppearancePath();
			
			if (   !endAppearancePath.empty()
				&& (lightningBoltData.m_endObject == NULL))
			{
				lightningBoltData.m_endObject = new MemoryBlockManagedObject();
				Appearance * const appearance = AppearanceTemplateList::createAppearance(endAppearancePath.c_str());
				appearance->setUniformScale(getScale().x);
				lightningBoltData.m_endObject->setAppearance(appearance);
				
				RenderWorld::addObjectNotifications(*lightningBoltData.m_endObject);
				SetupClientParticle::addNotifications(*lightningBoltData.m_endObject);
				getOwner()->addChildObject_o(lightningBoltData.m_endObject);
			}
		}
	}

	lightningBoltData.m_startPosition_w = startPosition_w;
	lightningBoltData.m_endPosition_w = endPosition_w;
}

//--------------------------------------------------------------------------
void LightningAppearance::setColorModifier(VectorArgb const &color)
{
	m_colorModifier = color;
}

//--------------------------------------------------------------------------
void LightningAppearance::setGlobalColorModifier(VectorArgb const &color)
{
	s_globalColorModifier = color;
}

//--------------------------------------------------------------------------
VectorArgb const &LightningAppearance::getColorModifier() const
{
	return m_colorModifier;
}

//--------------------------------------------------------------------------
VectorArgb const &LightningAppearance::getGlobalColorModifier() const
{
	return s_globalColorModifier;
}

//--------------------------------------------------------------------------
Vector LightningAppearance::getAverageStartPosition() const
{
	Vector result;
	int boltCount = 0;

	for (unsigned int index = 0; index < static_cast<unsigned int>(m_activeBoltCount); ++index)
	{
		LightningBoltData const &lightningBoltData = m_lightningBoltDataList[index];

		if (lightningBoltData.m_enabled)
		{
			Vector const &offset = lightningBoltData.m_morphPositionOffsetList_o[0];

			result += lightningBoltData.m_startPosition_w + offset;
			++boltCount;
		}
	}

	if (boltCount > 0)
	{
		result /= static_cast<float>(boltCount);
	}

	return result;
}

//--------------------------------------------------------------------------
Vector LightningAppearance::getAverageEndPosition() const
{
	Vector result;
	int boltCount = 0;

	for (unsigned int index = 0; index < static_cast<unsigned int>(m_activeBoltCount); ++index)
	{
		LightningBoltData const &lightningBoltData = m_lightningBoltDataList[index];

		if (lightningBoltData.m_enabled)
		{
			Vector const &offset = lightningBoltData.m_morphPositionOffsetList_o[LightningBolt::getPositionCount() - 1];

			result += lightningBoltData.m_endPosition_w + offset;
			++boltCount;
		}
	}

	if (boltCount > 0)
	{
		result /= static_cast<float>(boltCount);
	}

	return result;
}

//--------------------------------------------------------------------------

int LightningAppearance::getMaxLightningBoltCount()
{
	return m_maxLightningBoltCount;
}

//--------------------------------------------------------------------------

void LightningAppearance::setScale(Vector const & scale)
{
	DEBUG_WARNING(!WithinEpsilonInclusive(scale.x, scale.y, FLT_MIN) || !WithinEpsilonInclusive(scale.x, scale.z, FLT_MIN), ("LightningAppearance::setScale called with a non-uniform scale."));
	Appearance::setScale(scale);
}


//--------------------------------------------------------------------------

LightningAppearance * LightningAppearance::asLightningAppearance()
{
	return this;
}

//--------------------------------------------------------------------------

LightningAppearance const * LightningAppearance::asLightningAppearance() const
{
	return this;
}

//--------------------------------------------------------------------------

LightningAppearance * LightningAppearance::asLightningAppearance(Appearance * appearance)
{
	return (appearance != NULL) ? appearance->asLightningAppearance() : NULL;
}

//--------------------------------------------------------------------------

LightningAppearance const * LightningAppearance::asLightningAppearance(Appearance const * appearance)
{
	return (appearance != NULL) ? appearance->asLightningAppearance() : NULL;
}

//----------------------------------------------------------------------

void LightningAppearance::setStartAppearancesEnabled(bool enabled)
{
	for (int boltIndex = 0; boltIndex < m_maxLightningBoltCount; ++boltIndex)
	{
		LightningBoltData &lightningBoltData = m_lightningBoltDataList[boltIndex];
		
		if (!lightningBoltData.m_enabled)
		{
			continue;
		}
		
		if (lightningBoltData.m_startObject != NULL)
			lightningBoltData.m_startObject->setActive(enabled);
	}
}

//----------------------------------------------------------------------

void LightningAppearance::setEndAppearancesEnabled(bool enabled)
{
	for (int boltIndex = 0; boltIndex < m_maxLightningBoltCount; ++boltIndex)
	{
		LightningBoltData &lightningBoltData = m_lightningBoltDataList[boltIndex];
		
		if (!lightningBoltData.m_enabled)
		{
			continue;
		}
		
		if (lightningBoltData.m_endObject != NULL)
			lightningBoltData.m_endObject->setActive(enabled);
	}
}

// ============================================================================
