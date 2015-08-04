// ============================================================================
//
// ParticleEmitter.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleEmitter.h"

#include "clientAudio/Audio.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientParticle/ConfigClientParticle.h"
#include "clientParticle/ParticleAttachmentDescription.h"
#include "clientParticle/ParticleDescriptionMesh.h"
#include "clientParticle/ParticleDescriptionQuad.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientParticle/ParticleEmitterDescription.h"
#include "clientParticle/ParticleEmitterShape.h"
#include "clientParticle/ParticleManager.h"
#include "clientParticle/ParticleMesh.h"
#include "clientParticle/ParticleQuad.h"
#include "clientParticle/SetupClientParticle.h"
#include "clientParticle/SwooshAppearance.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Distance3d.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorLocator.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FloatMath.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Object.h"
#include "sharedObject/TextAppearance.h"
#include "sharedRandom/Random.h"
#include "sharedTerrain/TerrainObject.h"

#include <algorithm>
#include <vector>
#include <limits>

// ============================================================================
//
// ParticleEmitterNamespace
//
// ============================================================================

namespace ParticleEmitterNamespace
{
	ParticleEmitter::CreateTextAppearanceObjectFunction s_createTextAppearanceObjectFunction = NULL;
	ParticleEmitter::DrawBoxFunction s_drawBoxFunction = NULL;
	ParticleEmitter::DrawCylinderFunction s_drawCylinderFunction = NULL;

	static int s_terrainHeightCallCount = 0;

	typedef std::vector<Particle *> ParticleList;
	typedef std::vector<ParticleList *> ParticleListList;
	ParticleListList ms_particleListList;

	ParticleList * newParticleList();
	void deleteParticleList(ParticleList * particleList);
	void deleteParticleListList();
	VectorArgb const & getDebugTextColor();
}

using namespace ParticleEmitterNamespace;

// ----------------------------------------------------------------------------
ParticleList * ParticleEmitterNamespace::newParticleList()
{
	if (ms_particleListList.empty())
	{
		for (int i = 0; i < 256; ++i)
			ms_particleListList.push_back(new ParticleList);
	}

	ParticleList * const result = ms_particleListList.back();
	ms_particleListList.pop_back();

	return result;
}

// ----------------------------------------------------------------------------
void ParticleEmitterNamespace::deleteParticleList(ParticleList * const particleList)
{
	if (ms_particleListList.size() < 256)
	{
		DEBUG_FATAL(!particleList->empty(), ("particleList is not empty"));
		particleList->clear();
		ms_particleListList.push_back(particleList);
	}
	else
		delete particleList;
}

// ----------------------------------------------------------------------------
void ParticleEmitterNamespace::deleteParticleListList()
{
	while (!ms_particleListList.empty())
	{
		delete ms_particleListList.back();
		ms_particleListList.pop_back();
	}
}

// ----------------------------------------------------------------------------
VectorArgb const & ParticleEmitterNamespace::getDebugTextColor()
{
	switch (rand() % 7)
	{
		case 0: { return VectorArgb::solidBlue; }
		case 1: { return VectorArgb::solidCyan; }
		case 2: { return VectorArgb::solidGray; }
		case 3: { return VectorArgb::solidGreen; }
		case 4: { return VectorArgb::solidRed; }
		case 5: { return VectorArgb::solidMagenta; }
		case 6: { return VectorArgb::solidYellow; }
		default: {}
	}

	return VectorArgb::solidWhite;
}

// ============================================================================
//
// ParticleEmitter::LocalShaderPrimitive
//
// ============================================================================

//-----------------------------------------------------------------------------
class ParticleEmitter::LocalShaderPrimitive : public ShaderPrimitive
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

friend class ParticleEmitter;

public:

	static VertexBufferFormat LocalShaderPrimitive::getVertexBufferFormat();

public:

	explicit LocalShaderPrimitive(ParticleEmitter const &particleEmitter);
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

	ParticleEmitter const &m_particleEmitter;
	DynamicVertexBuffer    m_vertexBuffer;
	Camera const *         m_camera;
	Object const *         m_object;
	Shader const *         m_shader;
	float                  m_depthSquaredSortKey;

private:

	// Disabled

	LocalShaderPrimitive();
	LocalShaderPrimitive(LocalShaderPrimitive const &);
	LocalShaderPrimitive &operator=(LocalShaderPrimitive const &);
};

//-----------------------------------------------------------------------------
MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ParticleEmitter::LocalShaderPrimitive, true, 0, 0, 0);

//-----------------------------------------------------------------------------
VertexBufferFormat ParticleEmitter::LocalShaderPrimitive::getVertexBufferFormat()
{
	VertexBufferFormat format;

	format.setPosition();
	format.setColor0();
	format.setNumberOfTextureCoordinateSets(1);
	format.setTextureCoordinateSetDimension(0, 2);

	return format;
}

//-----------------------------------------------------------------------------
ParticleEmitter::LocalShaderPrimitive::LocalShaderPrimitive(ParticleEmitter const &particleEmitter)
 : ShaderPrimitive()
 , m_particleEmitter(particleEmitter)
 , m_vertexBuffer(getVertexBufferFormat())
 , m_camera(NULL)
 , m_object(NULL)
 , m_shader(NULL)
 , m_depthSquaredSortKey(0.0f)
{
	if (particleEmitter.m_particleEmitterDescription.m_particleDescription->getParticleType() == ParticleDescription::PT_quad)
	{
		ParticleDescriptionQuad *particleDescriptionQuad = safe_cast<ParticleDescriptionQuad *>(particleEmitter.m_particleEmitterDescription.m_particleDescription);

		if (particleDescriptionQuad->getParticleTexture().isTextureVisible() &&
			!particleDescriptionQuad->getParticleTexture().getShaderPath().isEmpty())
		{
			m_shader = ShaderTemplateList::fetchShader(particleDescriptionQuad->getParticleTexture().getShaderPath().getString());
		}
	}
}

//-----------------------------------------------------------------------------
ParticleEmitter::LocalShaderPrimitive::~LocalShaderPrimitive()
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
float ParticleEmitter::LocalShaderPrimitive::alter(real time)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ParticleEmitter::LSP::alter");

	if (m_shader != NULL)
		return m_shader->alter(time);
	else
		return AlterResult::cms_kill;
}

//-----------------------------------------------------------------------------
StaticShader const &ParticleEmitter::LocalShaderPrimitive::prepareToView() const
{
	StaticShader const &result = ((m_shader != NULL) && ParticleManager::isTexturingEnabled()) ? m_shader->prepareToView() : ShaderTemplateList::get3dVertexColorACStaticShader();

	return result;
}

//-----------------------------------------------------------------------------
const Vector ParticleEmitter::LocalShaderPrimitive::getPosition_w() const
{
	return (m_object != NULL) ? m_object->getPosition_w() : Vector::zero;
}
	
//-------------------------------------------------------------------

float ParticleEmitter::LocalShaderPrimitive::getDepthSquaredSortKey() const
{
	return m_depthSquaredSortKey;
}

//-----------------------------------------------------------------------------
int ParticleEmitter::LocalShaderPrimitive::getVertexBufferSortKey() const
{
	return m_vertexBuffer.getSortKey();
}

//-----------------------------------------------------------------------------
void ParticleEmitter::LocalShaderPrimitive::prepareToDraw() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ParticleEmitter::LocalShaderPrimitive::prepareToDraw");
	m_particleEmitter.draw();
}

//-----------------------------------------------------------------------------
void ParticleEmitter::LocalShaderPrimitive::draw() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ParticleEmitter::LocalShaderPrimitive::draw");

	if (m_particleEmitter.hasAliveParticles())
	{
		Graphics::setCullMode(GCM_none);
			Graphics::drawTriangleList();
		Graphics::setCullMode(GCM_counterClockwise);
	}
}

//----------------------------------------------------------------------

float ParticleEmitter::LocalShaderPrimitive::getRadius() const
{
	return m_particleEmitter.getExtent().getSphere().getRadius();
}

// ============================================================================
//
// ParticleEmitter
//
// ============================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ParticleEmitter, true, 0, 0, 0);

//-----------------------------------------------------------------------------
void ParticleEmitter::install()
{
	installMemoryBlockManager();
	LocalShaderPrimitive::install();

	ExitChain::add(ParticleEmitter::remove, "ParticleEmitter::remove");
}

//-----------------------------------------------------------------------------
void ParticleEmitter::remove()
{
	removeMemoryBlockManager();
	deleteParticleListList();
}

//-----------------------------------------------------------------------------
int ParticleEmitter::getGlobalCount()
{
	return ms_memoryBlockManager->getElementCount();
}

//-----------------------------------------------------------------------------
void ParticleEmitter::setCreateTextAppearanceObjectFunction(ParticleEmitter::CreateTextAppearanceObjectFunction createTextAppearanceObjectFunction)
{
	s_createTextAppearanceObjectFunction = createTextAppearanceObjectFunction;
}

//-----------------------------------------------------------------------------
void ParticleEmitter::setDrawBoxFunction(ParticleEmitter::DrawBoxFunction drawBoxFunction)
{
	s_drawBoxFunction = drawBoxFunction;
}

//-----------------------------------------------------------------------------
void ParticleEmitter::setDrawCylinderFunction(ParticleEmitter::DrawCylinderFunction drawCylinderFunction)
{
	s_drawCylinderFunction = drawCylinderFunction;
}

//-----------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter(ParticleEmitterDescription const &particleEmitterDescription, ParticleEffectAppearance const * const particleEffectAppearance)
 : ParticleGenerator(particleEffectAppearance)
 , m_particleEmitterDescription(particleEmitterDescription)
 , m_particles(newParticleList())
 , m_particleAttachments((m_particleEmitterDescription.m_particleDescription->getParticleAttachmentDescriptions().size() > 0) ? new ParticleAttachments : NULL)
 , m_lifeTime(Random::randomReal(particleEmitterDescription.getEmitterLifeTimeMin(), particleEmitterDescription.getEmitterLifeTimeMax()))
 , m_age(0.0f)
 , m_timeElapsed(0.0f)
 , m_enabled(true)
 , m_frameFirst(true)
 , m_frameLast(false)
 , m_newParticles(0.0f)
 , m_localShaderPrimitive(NULL)
 , m_object(new MemoryBlockManagedObject())
 , m_previousTransform_o2w(Transform::identity)
 , m_startDelay(0.0f)
 , m_loopCount(0)
 , m_currentLoop(0)
 , m_iterEmitterTranslationX()
 , m_iterEmitterTranslationY()
 , m_iterEmitterTranslationZ()
 , m_iterEmitterRotationX()
 , m_iterEmitterRotationY()
 , m_iterEmitterRotationZ()
 , m_iterEmitterDistance()
 , m_iterEmitterShapeSize()
 , m_iterEmitterSpread()
 , m_iterParticleGenerationRate()
 , m_iterParticleEmitSpeed()
 , m_iterParticleInheritVelocityPercent()
 , m_iterParticleClusterCount()
 , m_iterParticleClusterRadius()
 , m_iterParticleLifeTime()
 , m_iterParticleWeight()
 , m_particleForwardVelocityMaintained(ParticleEmitterDescription::m_defaultParticleVelocityMaintained)
 , m_particleUpVelocityMaintained(ParticleEmitterDescription::m_defaultParticleVelocityMaintained)
 , m_soundId()
 , m_lodPercent(0.0f)
 , m_accumulatedDistance(0.0f)
 , m_currentCameraPosition_w(Vector::zero)
 , m_averageParticlePosition(Vector::zero)
 , m_averageParticleVelocity(Vector::zero)
#ifdef _DEBUG
 , m_debugTextObject(NULL)
 , m_debugTextObjectAppearance(NULL)
 , m_debugTextColor()
#endif // _DEBUG
{
	m_particles->reserve(static_cast<unsigned int>(particleEmitterDescription.m_emitterMaxParticles));
	m_localShaderPrimitive = new LocalShaderPrimitive(*this);

	DEBUG_FATAL(m_particleEmitterDescription.m_emitterTranslationX.getControlPointCount() < 2, ("m_particleEmitterDescription.m_emitterTranslationX.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_emitterTranslationY.getControlPointCount() < 2, ("m_particleEmitterDescription.m_emitterTranslationY.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_emitterTranslationZ.getControlPointCount() < 2, ("m_particleEmitterDescription.m_emitterTranslationZ.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_emitterRotationX.getControlPointCount() < 2, ("m_particleEmitterDescription.m_emitterRotationX.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_emitterRotationY.getControlPointCount() < 2, ("m_particleEmitterDescription.m_emitterRotationY.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_emitterRotationZ.getControlPointCount() < 2, ("m_particleEmitterDescription.m_emitterRotationZ.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_emitterDistance.getControlPointCount() < 2, ("m_particleEmitterDescription.m_emitterDistance.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_emitterShapeSize.getControlPointCount() < 2, ("m_particleEmitterDescription.m_emitterShapeSize.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_emitterSpread.getControlPointCount() < 2, ("m_particleEmitterDescription.m_emitterSpread.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_particleGenerationRate.getControlPointCount() < 2, ("m_particleEmitterDescription.m_particleGenerationRate.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_particleEmitSpeed.getControlPointCount() < 2, ("m_particleEmitterDescription.m_particleEmitSpeed.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_particleInheritVelocityPercent.getControlPointCount() < 2, ("m_particleEmitterDescription.m_particleInheritVelocityPercent.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_particleClusterCount.getControlPointCount() < 2, ("m_particleEmitterDescription.m_particleClusterCount.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_particleClusterRadius.getControlPointCount() < 2, ("m_particleEmitterDescription.m_particleClusterRadius.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_particleLifeTime.getControlPointCount() < 2, ("m_particleEmitterDescription.m_particleLifeTime.getControlPointCount() < 2"));
	DEBUG_FATAL(m_particleEmitterDescription.m_particleWeight.getControlPointCount() < 2, ("m_particleEmitterDescription.m_particleWeight.getControlPointCount() < 2"));
}

//-----------------------------------------------------------------------------
ParticleEmitter::~ParticleEmitter()
{
	removeAllParticles();
	deleteParticleList(m_particles);

	removeAllAttachments();
	delete m_particleAttachments;
	m_particleAttachments = NULL;

	delete m_localShaderPrimitive;
	m_localShaderPrimitive = NULL;

#ifdef _DEBUG
	delete m_debugTextObject;
	m_debugTextObject = NULL;
#endif // _DEBUG

	delete m_object.getPointer();

	Audio::stopSound(m_soundId, 1.0f);
	m_soundId.invalidate();
}

//-----------------------------------------------------------------------------
void ParticleEmitter::removeAllParticles()
{
	Particles::iterator iterParticles = m_particles->begin();

	for (; iterParticles != m_particles->end(); ++iterParticles)
	{
		Particle * const particle = NON_NULL(*iterParticles);
		delete particle;
	}
	m_particles->clear();
}

//-----------------------------------------------------------------------------
void ParticleEmitter::removeAllAttachments()
{
	if (m_particleAttachments != NULL)
	{
		ParticleAttachments::iterator iterParticleAttachments = m_particleAttachments->begin();

		for (; iterParticleAttachments != m_particleAttachments->end(); ++iterParticleAttachments)
		{
			ParticleAttachment * const particleAttachment = (*iterParticleAttachments).getPointer();
			delete particleAttachment;
		}
		m_particleAttachments->clear();
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::loop()
{
	m_lifeTime = Random::randomReal(m_particleEmitterDescription.getEmitterLifeTimeMin(), m_particleEmitterDescription.getEmitterLifeTimeMax());

	m_iterEmitterTranslationX.reset(m_particleEmitterDescription.m_emitterTranslationX.getIteratorBegin());
	m_iterEmitterTranslationY.reset(m_particleEmitterDescription.m_emitterTranslationY.getIteratorBegin());
	m_iterEmitterTranslationZ.reset(m_particleEmitterDescription.m_emitterTranslationZ.getIteratorBegin());
	m_iterEmitterRotationX.reset(m_particleEmitterDescription.m_emitterRotationX.getIteratorBegin());
	m_iterEmitterRotationY.reset(m_particleEmitterDescription.m_emitterRotationY.getIteratorBegin());
	m_iterEmitterRotationZ.reset(m_particleEmitterDescription.m_emitterRotationZ.getIteratorBegin());
	m_iterEmitterDistance.reset(m_particleEmitterDescription.m_emitterDistance.getIteratorBegin());
	m_iterEmitterShapeSize.reset(m_particleEmitterDescription.m_emitterShapeSize.getIteratorBegin());
	m_iterEmitterSpread.reset(m_particleEmitterDescription.m_emitterSpread.getIteratorBegin());
	m_iterParticleGenerationRate.reset(m_particleEmitterDescription.m_particleGenerationRate.getIteratorBegin());
	m_iterParticleEmitSpeed.reset(m_particleEmitterDescription.m_particleEmitSpeed.getIteratorBegin());
	m_iterParticleInheritVelocityPercent.reset(m_particleEmitterDescription.m_particleInheritVelocityPercent.getIteratorBegin());
	m_iterParticleClusterCount.reset(m_particleEmitterDescription.m_particleClusterCount.getIteratorBegin());
	m_iterParticleClusterRadius.reset(m_particleEmitterDescription.m_particleClusterRadius.getIteratorBegin());
	m_iterParticleLifeTime.reset(m_particleEmitterDescription.m_particleLifeTime.getIteratorBegin());
	m_iterParticleWeight.reset(m_particleEmitterDescription.m_particleWeight.getIteratorBegin());

	m_particleForwardVelocityMaintained = Random::randomReal(m_particleEmitterDescription.m_particleForwardVelocityMaintainedMin, m_particleEmitterDescription.m_particleForwardVelocityMaintainedMax);
	m_particleUpVelocityMaintained = Random::randomReal(m_particleEmitterDescription.m_particleUpVelocityMaintainedMin, m_particleEmitterDescription.m_particleUpVelocityMaintainedMax);

	m_age = 0.0f;
	m_timeElapsed = 0.0f;
	m_newParticles = 0.0f;
	m_averageParticlePosition = m_object->getPosition_w();
	m_averageParticleVelocity = Vector::zero;

	if (m_currentLoop <= 0)
	{
		m_startDelay = m_particleEmitterDescription.m_timing.getRandomStartDelay();
	}
	else
	{
		m_startDelay = m_particleEmitterDescription.m_timing.getRandomLoopDelay();
	}

	if (m_currentLoop <= 0)
	{
		m_frameFirst = true;
		m_frameLast = false;
	}
	else
	{
		if (m_particleEmitterDescription.m_emitterOneShot ||
		    (m_startDelay > 0.5f))
		{
			m_frameFirst = true;
			frameLast();
			m_frameLast = false;
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::restart()
{
	removeAllParticles();
	removeAllAttachments();

	m_currentLoop = 0;
	m_loopCount = m_particleEmitterDescription.m_timing.getRandomLoopCount();
	m_frameFirst = true;
	m_frameLast = false;

	loop();
}

//-----------------------------------------------------------------------------

void ParticleEmitter::addToCameraScene(Camera const *camera, Object const *object) const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ParticleEmitter::addToCameraScene");

	m_currentCameraPosition_w = (camera != NULL) ? camera->getPosition_w() : Vector::zero;

	//if (camera)	
	//{
	//	const Vector position = m_previousTransform_o2w.getPosition_p();
	//	const Vector top (position.x, 1000.f, position.z);
	//	const Vector bottom (position.x, -1000.f, position.z);
	//	camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_z, Transform::identity, top, bottom, VectorArgb::solidWhite));
	//}

#ifdef _DEBUG
	// Debug emitter axis

	if (ParticleManager::isDebugAxisEnabled())
	{
		drawDebugAxis();
	}

	// Debug particle velocity

	if (ParticleManager::isDebugVelocityEnabled())
	{
		drawDebugVelocity();
	}

	// Debug particle orientation

	if (ParticleManager::isDebugParticleOrientationEnabled())
	{
		drawDebugParticleOrientation();
	}
#endif // _DEBUG

	// Make sure there is something to render

	if (!m_particles->empty())
	{
		// Make sure this is a quad emitter

		if (m_particleEmitterDescription.m_particleDescription->getParticleType() == ParticleDescription::PT_quad)
		{
			// Set the shader depth sort key

			m_localShaderPrimitive->m_depthSquaredSortKey = m_extent_w.getSphere().getCenter().magnitudeBetweenSquared(camera->getPosition_w());

			// Make sure this emitter has some particles that are visible

			m_localShaderPrimitive->m_camera = camera;
			m_localShaderPrimitive->m_object = object;

			// Add the primitive to be rendered at the appropriate time

			ShaderPrimitiveSorter::add(*m_localShaderPrimitive);

#ifdef _DEBUG
			if (ParticleManager::isDebugExtentsEnabled())
			{
				drawDebugExtents();
			}
#endif // _DEBUG
		}
		if(m_particleEmitterDescription.m_particleDescription->getParticleType() == ParticleDescription::PT_mesh)
		{
			m_localShaderPrimitive->m_camera = camera;
			m_localShaderPrimitive->m_object = object;
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::draw() const
{
	if (m_particleEmitterDescription.m_particleDescription->getParticleType() == ParticleDescription::PT_quad)
	{
		ParticleDescriptionQuad *particleDescriptionQuad = safe_cast<ParticleDescriptionQuad *>(m_particleEmitterDescription.m_particleDescription);

		drawParticlesQuad(particleDescriptionQuad);
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::sortParticles() const
{
	// Calculate the camera Z distance for each particle

	Particles::const_iterator iterParticles = m_particles->begin();

	if (m_particleEmitterDescription.m_localSpaceParticles)
	{
		// Local space

		for (; iterParticles != m_particles->end(); ++iterParticles)
		{
			Particle *particle = (*iterParticles);

			particle->m_distanceZ = m_localShaderPrimitive->m_camera->rotateTranslate_w2o(m_object->getTransform_o2w().rotateTranslate_l2p(particle->m_position)).z;
		}
	}
	else
	{
		// World space

		for (; iterParticles != m_particles->end(); ++iterParticles)
		{
			Particle *particle = (*iterParticles);

			particle->m_distanceZ = m_localShaderPrimitive->m_camera->rotateTranslate_w2o(particle->m_position).z;
		}
	}

	// Figure out the sort order

	for (unsigned int i = 0; i < m_particles->size(); ++i)
	{
		for (unsigned int j = i + 1; j < m_particles->size(); ++j)
		{
			if ((*m_particles)[i]->m_distanceZ < (*m_particles)[j]->m_distanceZ)
			{
				Particle *temp = (*m_particles)[i];

				(*m_particles)[i] = (*m_particles)[j];

				(*m_particles)[j] = temp;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::drawParticlesQuad(ParticleDescriptionQuad const *particleDescriptionQuad) const
{
	DEBUG_WARNING((m_object == NULL), ("Rendering with a NULL m_object: %s", getParentParticleEffectAppearance().getAppearanceTemplateName()));

	if (m_object == NULL)
	{
		return;
	}

	sortParticles();

	// Get the terrain fog color

	TerrainObject const * const terrainObject = TerrainObject::getConstInstance();
	float fogColorRed = 1.0f;
	float fogColorGreen = 1.0f;
	float fogColorBlue = 1.0f;
	
	if (terrainObject != NULL)
	{
		fogColorRed = static_cast<float>(terrainObject->getFogColor().r) / 255.0f;
		fogColorGreen = static_cast<float>(terrainObject->getFogColor().g) / 255.0f;;
		fogColorBlue = static_cast<float>(terrainObject->getFogColor().b) / 255.0f;;
	}

	// Get the effect scale

	float const effectScale = getParentParticleEffectAppearance().getScale_w();

	m_localShaderPrimitive->m_vertexBuffer.lock(getParticleCount() * 6);
	{
		VertexBufferWriteIterator vbwIter = m_localShaderPrimitive->m_vertexBuffer.begin();

		VectorArgb const &colorModifier = getParentParticleEffectAppearance().getColorModifier();
		VectorArgb const &globalColorModifier = ParticleEffectAppearance::getGlobalColorModifier();

		// Make sure that all the particles that reach this call are alive, also make sure that
		// their alpha is not zero, and any dimension is not zero.

		Particles::const_iterator iterParticles = m_particles->begin();

		for (; iterParticles != m_particles->end(); ++iterParticles)
		{
			ParticleQuad *particle = safe_cast<ParticleQuad *>(*iterParticles);
			float const particleAgePercent = particle->getAgePercent();

			DEBUG_FATAL((particle == NULL), ("The particle is not a quad but it should be."));

			// Get the alpha

			float alpha = particleDescriptionQuad->getAlpha().getValue(particle->m_iterAlpha, particleAgePercent);

			DEBUG_FATAL((alpha < 0.0f) || (alpha > 1.0f), ("alpha(%f) out of range", alpha));

			// Get the RGB

			float red = 1.0f;
			float green = 1.0f;
			float blue = 1.0f;

			switch (particleDescriptionQuad->getColor().getSampleType())
			{
				case ColorRamp::ST_single:
					{
						particleDescriptionQuad->getColor().getColorAtPercent(particle->m_iterColor.m_randomSamplePercent, red, green, blue);
					}
					break;
				case ColorRamp::ST_all:
					{
						particleDescriptionQuad->getColor().getColorAtPercent(particleAgePercent, red, green, blue);
					}
					break;
				default:
					{
						DEBUG_FATAL(true, ("Invalid color ramp sample mode specified."));
					}
					break;
			}

			// Add in the code driven color modifiers

			red *= colorModifier.r * globalColorModifier.r;
			green *= colorModifier.g * globalColorModifier.g;
			blue *= colorModifier.b * globalColorModifier.b;
			alpha *= colorModifier.a * globalColorModifier.a;

			if (m_particleEmitterDescription.m_particleTimeOfDayColorPercent > 0.0f)
			{
				red += (fogColorRed - red) * m_particleEmitterDescription.m_particleTimeOfDayColorPercent;
				green += (fogColorGreen - green) * m_particleEmitterDescription.m_particleTimeOfDayColorPercent;
				blue += (fogColorBlue - blue) * m_particleEmitterDescription.m_particleTimeOfDayColorPercent;
			}

			PackedArgb color(VectorArgb(alpha, red, green, blue));

			// Get the length

			float const particleLength = particleDescriptionQuad->getLength().getValue(particle->m_iterLength, particleAgePercent) * effectScale;

			// Get the width, possibly linked to the length

			float const particleWidth = particleDescriptionQuad->isLengthAndWidthLinked() ? particleLength : (particleDescriptionQuad->getWidth().getValue(particle->m_iterWidth, particleAgePercent) * effectScale);

			// Get the render orientation

			Vector upVector;
			Vector sideVector;

			calculateRenderOrientationVectors(particleDescriptionQuad, *particle, upVector, sideVector, m_object->getTransform_o2w());

			float au = 0.0f;
			float av = 0.0f;
			float bu = 0.0f;
			float bv = 0.0f;
			float cu = 0.0f;
			float cv = 0.0f;
			float du = 0.0f;
			float dv = 0.0f;

			particleDescriptionQuad->getParticleTexture().getUVs(particleAgePercent, particle->m_age, au, av, bu, bv, cu, cv, du, dv);

			Vector particlePosition;

			// Local or world space

			if (m_particleEmitterDescription.m_localSpaceParticles)
			{
				Transform const &tranfsorm_o2w = m_object->getTransform_o2w();

				particlePosition = tranfsorm_o2w.rotateTranslate_l2p(particle->m_position);
			}
			else
			{
				particlePosition = particle->m_position;
			}

			upVector *= particleLength;
			sideVector *= particleWidth;

			Vector const a(particlePosition - upVector + sideVector);
			Vector const b(particlePosition + upVector + sideVector);
			Vector const c(particlePosition + upVector - sideVector);
			Vector const d(particlePosition - upVector - sideVector);

			// Add the vertices to the vertex buffer

			vbwIter.setPosition(a);
			vbwIter.setColor0(color);
			vbwIter.setTextureCoordinates(0, au, av);
			++vbwIter;

			vbwIter.setPosition(c);
			vbwIter.setColor0(color);
			vbwIter.setTextureCoordinates(0, cu, cv);
			++vbwIter;

			vbwIter.setPosition(b);
			vbwIter.setColor0(color);
			vbwIter.setTextureCoordinates(0, bu, bv);
			++vbwIter;

			vbwIter.setPosition(a);
			vbwIter.setColor0(color);
			vbwIter.setTextureCoordinates(0, au, av);
			++vbwIter;

			vbwIter.setPosition(d);
			vbwIter.setColor0(color);
			vbwIter.setTextureCoordinates(0, du, dv);
			++vbwIter;

			vbwIter.setPosition(c);
			vbwIter.setColor0(color);
			vbwIter.setTextureCoordinates(0, cu, cv);
			++vbwIter;
		}
	}
	m_localShaderPrimitive->m_vertexBuffer.unlock();
	
	Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
	Graphics::setVertexBuffer(m_localShaderPrimitive->m_vertexBuffer);
}

//-----------------------------------------------------------------------------
void ParticleEmitter::alter(float const deltaTime)
{
	DEBUG_WARNING((m_object == NULL), ("Altering with a NULL m_object: %s", getParentParticleEffectAppearance().getAppearanceTemplateName()));

	if (m_object == NULL)
	{
		return;
	}

	bool createParticlesThisFrame = false;
	bool doLoop = false;

	// Reset the extent

	m_extent_w.setMin(Vector::maxXYZ);
	m_extent_w.setMax(Vector::negativeMaxXYZ);

	// If the parent particle effect appearance is disabled, then this emitter
	// needs to be disable so that everything dies out

	m_enabled = getParentParticleEffectAppearance().isEnabled();

	// Check for a start delay

	m_timeElapsed += deltaTime;

	if (m_timeElapsed >= m_startDelay)
	{
		m_age += deltaTime;

		// Get the age percent of the emitter

		float const emitterAgePercent = (m_frameFirst && m_particleEmitterDescription.m_emitterOneShot) ? 0.0f : getAgePercent();
		float const effectScale = getParentParticleEffectAppearance().getScale_w();

		// Add in the emitter rotation

		Transform transform;

		float const emitterRotationY = m_particleEmitterDescription.m_emitterRotationY.getValue(m_iterEmitterRotationY, emitterAgePercent);
		transform.yaw_l(emitterRotationY * PI_TIMES_2);

		float const emitterRotationX = m_particleEmitterDescription.m_emitterRotationX.getValue(m_iterEmitterRotationX, emitterAgePercent);
		transform.pitch_l(emitterRotationX * PI_TIMES_2);

		float const emitterRotationZ = m_particleEmitterDescription.m_emitterRotationZ.getValue(m_iterEmitterRotationZ, emitterAgePercent);
		transform.roll_l(emitterRotationZ * PI_TIMES_2);

		// Add in the emitter translation

		float const emitterTranslationX = m_particleEmitterDescription.m_emitterTranslationX.getValue(m_iterEmitterTranslationX, emitterAgePercent) * effectScale;
		float const emitterTranslationY = m_particleEmitterDescription.m_emitterTranslationY.getValue(m_iterEmitterTranslationY, emitterAgePercent) * effectScale;
		float const emitterTranslationZ = m_particleEmitterDescription.m_emitterTranslationZ.getValue(m_iterEmitterTranslationZ, emitterAgePercent) * effectScale;
		Vector const emitterTranslation(emitterTranslationX, emitterTranslationY, emitterTranslationZ);
		transform.move_p(emitterTranslation);

		m_object->setTransform_o2p(transform);

		// If this is the first frame of the emitter, set the previous transform to the
		// current transform

		if (m_frameFirst)
		{
			m_previousTransform_o2w = m_object->getTransform_o2w();
		}

		if (m_enabled)
		{
			if ((emitterAgePercent < 1.0f) &&
			    (!m_particleEmitterDescription.m_emitterOneShot ||
			     m_frameFirst))
			{
				// Start the sound on the first frame of the emitter being alive

				if (m_frameFirst)
				{
					frameFirst();
				}

				// Get the accumulated movement distance for distance generated particles

				if (m_particleEmitterDescription.m_generationType == ParticleEmitterDescription::G_distance)
				{
					Vector a(m_object->getPosition_w());
					Vector b(m_previousTransform_o2w.getPosition_p());
					
					m_accumulatedDistance += a.magnitudeBetweenSquared(b);
				}

				createParticlesThisFrame = true;

				// Unmark the first frame

				m_frameFirst = false;
			}
			else if (emitterAgePercent >= 1.0f)
			{
				// Handle any events on the last frame the emitter is alive

				if ((m_currentLoop > m_loopCount) && !m_frameLast)
				{
					m_frameLast = true;

					frameLast();
				}

				if (m_particleEmitterDescription.m_emitterLoopImmediately)
				{
					// Loop immediately regardless of the number of particles alive

					doLoop = true;
				}
				else if (m_particles->empty())
				{
					// Wait until all the particles are dead before we allow a loop

					doLoop = true;
				}
			}
		}

		// Calculate some values needed if flocking is used

		if (m_particleEmitterDescription.m_flockingType != ParticleEmitterDescription::FT_none)
		{
			// Average particle position

			m_averageParticlePosition = Vector::zero;

			if (m_particleEmitterDescription.m_flockingCohesionGain > 0.0f)
			{
				Particles::iterator iterParticles = m_particles->begin();

				for (; iterParticles != m_particles->end(); ++iterParticles)
				{
					Particle const &particle = *(*iterParticles);

					m_averageParticlePosition += particle.m_position;
				}

				if (m_particles->size() > 1)
				{
					m_averageParticlePosition /= static_cast<float>(m_particles->size());
				}
			}

			// Average particle velocity

			m_averageParticleVelocity = Vector::zero;

			if (m_particleEmitterDescription.m_flockingAlignmentGain > 0.0f)
			{
				Particles::iterator iterParticles = m_particles->begin();

				for (; iterParticles != m_particles->end(); ++iterParticles)
				{
					Particle const &particle = *(*iterParticles);

					m_averageParticleVelocity += particle.m_velocity;
				}

				if (!m_particles->empty())
				{
					m_averageParticleVelocity /= static_cast<float>(m_particles->size());
				}
			}
		}

		// Save the previous transform

		m_previousTransform_o2w = m_object->getTransform_o2w();
	}

	// Update existing particles

	updateExistingParticles(deltaTime);

	// Create new particles, new particles get their own update on their first frame

	if (createParticlesThisFrame)
	{
		createNewParticles(deltaTime);
	}

	removeOldParticles();
	removeOldAttachments();

	// Update the extents
	
	if (   (m_particles->empty())
		|| (m_object == NULL))
	{
		if (m_object != NULL)
		{
			Vector objectPosition_w(m_object->getPosition_w());
			m_extent_w.setMin(objectPosition_w);
			m_extent_w.setMax(objectPosition_w);
		}
		else
		{
			m_extent_w.setMin(Vector::zero);
			m_extent_w.setMax(Vector::zero);
		}
	}
	else if (m_particleEmitterDescription.m_localSpaceParticles)
	{
		// If the emitter is using local reference frame, we need to rotate the extents
		// into world space
	
		Vector v1(m_extent_w.getLeft(), m_extent_w.getTop(), m_extent_w.getBack());
		Vector v2(m_extent_w.getLeft(), m_extent_w.getTop(), m_extent_w.getFront());
		Vector v3(m_extent_w.getRight(), m_extent_w.getTop(), m_extent_w.getFront());
		Vector v4(m_extent_w.getRight(), m_extent_w.getTop(), m_extent_w.getBack());
		Vector v5(m_extent_w.getLeft(), m_extent_w.getBottom(), m_extent_w.getBack());
		Vector v6(m_extent_w.getLeft(), m_extent_w.getBottom(), m_extent_w.getFront());
		Vector v7(m_extent_w.getRight(), m_extent_w.getBottom(), m_extent_w.getFront());
		Vector v8(m_extent_w.getRight(), m_extent_w.getBottom(), m_extent_w.getBack());
	
		m_extent_w.setMin(Vector::maxXYZ);
		m_extent_w.setMax(Vector::negativeMaxXYZ);
	
		m_extent_w.updateMinAndMax(m_object->getTransform_o2w().rotateTranslate_l2p(v1));
		m_extent_w.updateMinAndMax(m_object->getTransform_o2w().rotateTranslate_l2p(v2));
		m_extent_w.updateMinAndMax(m_object->getTransform_o2w().rotateTranslate_l2p(v3));
		m_extent_w.updateMinAndMax(m_object->getTransform_o2w().rotateTranslate_l2p(v4));
		m_extent_w.updateMinAndMax(m_object->getTransform_o2w().rotateTranslate_l2p(v5));
		m_extent_w.updateMinAndMax(m_object->getTransform_o2w().rotateTranslate_l2p(v6));
		m_extent_w.updateMinAndMax(m_object->getTransform_o2w().rotateTranslate_l2p(v7));
		m_extent_w.updateMinAndMax(m_object->getTransform_o2w().rotateTranslate_l2p(v8));
	}

	m_extent_w.calculateCenterAndRadius();

	DEBUG_FATAL(m_extent_w.getBox().isEmpty(), ("Invalid extent"));
	//DEBUG_REPORT_LOG(true, ("ParticleEmitter: m_extent_w.getRadius(%f) min(%f, %f, %f) max(%f, %f, %f)\n", m_extent_w.getRadius(), m_extent_w.getMin().x, m_extent_w.getMin().y, m_extent_w.getMin().z, m_extent_w.getMax().x, m_extent_w.getMax().y, m_extent_w.getMax().z));

	// Calculate the extent of all the particles

	calculateLod();

	// See if we need to loop

	if (doLoop)
	{
		// Increment the number of times we have looped

		++m_currentLoop;

		if (isInfiniteLooping() ||
			(m_currentLoop <= m_loopCount))
		{
			// Loop the system since we are infinite looping or the current loop
			// is less than the number of times to loop

			loop();
		}
	}

#ifdef _DEBUG
	if (ParticleManager::isDebugWorldTextEnabled())
	{
		if ((m_object != NULL) &&
			(s_createTextAppearanceObjectFunction != NULL) &&
			(m_debugTextObject == NULL))
		{
			VectorArgb const color(VectorArgb::solidGreen);
			float const sizeModifier = 0.6f;
			m_debugTextObject = s_createTextAppearanceObjectFunction("", color, sizeModifier);

			m_object->addChildObject_o(m_debugTextObject);
			m_debugTextObjectAppearance = safe_cast<TextAppearance *>(m_debugTextObject->getAppearance());
			m_debugTextObjectAppearance->setColor(getDebugTextColor());

			m_debugTextObject->move_o(Vector::unitY * 0.15f);
		}
	}
	else
	{
		delete m_debugTextObject;
		m_debugTextObject = NULL;
		m_debugTextObjectAppearance = NULL;
	}

	if (m_debugTextObjectAppearance != NULL)
	{
		// Update the debug world text

		std::string debugText;
		FormattedString<256> fs;

		// Filename
		{
			char const * const templatePath = getParentParticleEffectAppearance().getAppearanceTemplate()->getName();
			std::string const fileName(FileNameUtils::get(templatePath, FileNameUtils::fileName));
			debugText += fs.sprintf("%s\n", fileName.c_str());
		}

		// Particle count
		{
			int const particleCount = getParticleCount();
			debugText += fs.sprintf("count(%d)\n", particleCount);
		}

		// Z distance from camera
		{
			float const zDistance = sqrt(m_localShaderPrimitive->m_depthSquaredSortKey);
			debugText += fs.sprintf("distance (%.1fm)\n", zDistance);
		}

		// LOD
		{
			float minLodDistance = 0.0f;
			float maxLodDistance = 0.0f;

			if (m_particleEmitterDescription.isEmitterUsingNoLod())
			{
				debugText += fs.sprintf("no lod 100%\n");
			}
			else if (m_particleEmitterDescription.isEmitterUsingGlobalLod())
			{
				minLodDistance = ConfigClientParticle::getMinGlobalLodDistance();
				maxLodDistance = ConfigClientParticle::getMaxGlobalLodDistance();

				debugText += fs.sprintf("global lod [%.0f...%.0f] %.0f%%\n", minLodDistance, maxLodDistance, m_lodPercent * 100.0f);
			}
			else
			{
				minLodDistance = m_particleEmitterDescription.getEmitterLodDistanceMin();
				maxLodDistance = m_particleEmitterDescription.getEmitterLodDistanceMax();

				debugText += fs.sprintf("custom lod [%.0f...%.0f] %.0f%%\n", minLodDistance, maxLodDistance, m_lodPercent * 100.0f);
			}
		}

		// Scale
		{
			debugText += fs.sprintf("scale(%.1f)\n", getParentParticleEffectAppearance().getScale_w());
		}

		m_debugTextObjectAppearance->setText(debugText.c_str());
	}

	if (ParticleManager::isDebugFlockingCageEnabled() &&
	    (s_drawBoxFunction != NULL) &&
	    (m_particleEmitterDescription.m_flockingType != ParticleEmitterDescription::FT_none))
	{
		float const cageWidth = m_particleEmitterDescription.m_flockingCageWidth;
		float const cageHeight = m_particleEmitterDescription.m_flockingCageHeight;
		VectorArgb cageColor(VectorArgb::solidYellow);

		if (m_particleEmitterDescription.m_flockingCageShape == ParticleEmitterDescription::FCS_cylinder)
		{
			int const tessTheta = 0;
			int const tessRho = 0;
			int const tessZ = 0;
			int const nSpokes = 32;

			if ((cageWidth > 0.0f) &&
			    (cageHeight > 0.0f))
			{
				s_drawCylinderFunction(m_object->getPosition_w(), cageWidth, cageHeight, tessTheta, tessRho, tessZ, nSpokes, cageColor);
			}
		}
		else if (m_particleEmitterDescription.m_flockingCageShape == ParticleEmitterDescription::FCS_rectangle)
		{
			AxialBox box;

			box.add(Vector(m_object->getPosition_w().x - cageWidth, m_object->getPosition_w().y + cageHeight, m_object->getPosition_w().z - cageWidth));
			box.add(Vector(m_object->getPosition_w().x - cageWidth, m_object->getPosition_w().y + cageHeight, m_object->getPosition_w().z + cageWidth));
			box.add(Vector(m_object->getPosition_w().x + cageWidth, m_object->getPosition_w().y + cageHeight, m_object->getPosition_w().z + cageWidth));
			box.add(Vector(m_object->getPosition_w().x + cageWidth, m_object->getPosition_w().y + cageHeight, m_object->getPosition_w().z - cageWidth));

			box.add(Vector(m_object->getPosition_w().x - cageWidth, m_object->getPosition_w().y, m_object->getPosition_w().z - cageWidth));
			box.add(Vector(m_object->getPosition_w().x - cageWidth, m_object->getPosition_w().y, m_object->getPosition_w().z + cageWidth));
			box.add(Vector(m_object->getPosition_w().x + cageWidth, m_object->getPosition_w().y, m_object->getPosition_w().z + cageWidth));
			box.add(Vector(m_object->getPosition_w().x + cageWidth, m_object->getPosition_w().y, m_object->getPosition_w().z - cageWidth));

			if (box.getRadius() > 0)
			{
				s_drawBoxFunction(box, cageColor);
			}
		}
		else
		{
			DEBUG_FATAL(true, ("Unknown flocking cage shape"));
		}
	}
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
bool ParticleEmitter::isDeletable() const
{
	bool result = false;
	bool doneLooping = (!isInfiniteLooping() && (m_currentLoop > m_loopCount)) || !m_enabled;

	if (doneLooping && !hasAliveParticles())
	{
		if (   (m_particleAttachments == NULL)
		    || m_particleAttachments->empty())
		{
			result = true;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
int ParticleEmitter::getParticleCount() const
{
	return static_cast<int>(m_particles->size());
}

//-----------------------------------------------------------------------------
bool ParticleEmitter::hasAliveParticles() const
{
	bool result = false;

	if (getParticleCount() > 0)
	{
		result = true;
	}
	else if (m_particleAttachments != NULL)
	{
		// Go deep looking for some particles

		ParticleAttachments::const_iterator iterParticleAttachments = m_particleAttachments->begin();

		for (; iterParticleAttachments != m_particleAttachments->end(); ++iterParticleAttachments)
		{
			ParticleAttachment const * const particleAttachment = *iterParticleAttachments;

			if (particleAttachment != NULL)
			{
				ParticleEffectAppearance const * const particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(particleAttachment->getAppearance());

				if (   (particleEffectAppearance != NULL)
					&& particleEffectAppearance->hasAliveParticles())
				{
					result = true;
					break;
				}
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
int ParticleEmitter::getParticleCountIncludingAttachments() const
{
	int result = getParticleCount();

	if (m_particleAttachments != NULL)
	{
		ParticleAttachments::const_iterator iterParticleAttachments = m_particleAttachments->begin();

		for (; iterParticleAttachments != m_particleAttachments->end(); ++iterParticleAttachments)
		{
			ParticleAttachment const * const particleAttachment = (*iterParticleAttachments);

			if (particleAttachment != NULL)
			{
				ParticleEffectAppearance const * const particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(particleAttachment->getAppearance());

				if (particleEffectAppearance != NULL)
				{
					result += particleEffectAppearance->getParticleCount();
				}
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void ParticleEmitter::initializeSingleParticle(Particle &particle)
{
	float const emitterAgePercent = m_particleEmitterDescription.m_emitterOneShot ? 0.0f : getAgePercent();

	// Get the effect scale

	float const effectScale = getParentParticleEffectAppearance().getScale_w();

	// Get the shape size and distance

	float const shapeSize = m_particleEmitterDescription.m_emitterShapeSize.getValue(m_iterEmitterShapeSize, emitterAgePercent) * effectScale;
	float const distance = m_particleEmitterDescription.m_emitterDistance.getValue(m_iterEmitterDistance, emitterAgePercent) * effectScale;

	if (!m_particleEmitterDescription.m_localSpaceParticles)
	{
		// Start the particle position at some point between the current and last position of
		// the emitter, this helps to remove the spurting look of moving emitters.

		Vector currentEmitterPosition(m_object->getTransform_o2w().getPosition_p());
		Vector previousEmitterPosition(m_previousTransform_o2w.getPosition_p());
		Vector emitterMovementVector(currentEmitterPosition - previousEmitterPosition);

		particle.m_position = previousEmitterPosition + emitterMovementVector * Random::randomReal(0.0f, 1.0f);
	}

	// Set the position and direction vector based on the emitter type

	switch (m_particleEmitterDescription.m_emitterEmitDirection)
	{
		case ParticleEmitterDescription::ED_directional:
			{
				// Set the direction vector

				float const spread = m_particleEmitterDescription.m_emitterSpread.getValue(m_iterEmitterSpread, emitterAgePercent) * PI_OVER_180;
				float const rotation1 = (rand() % 2) ? spread : -spread;
				float const rotation2 = Random::randomReal(-PI, PI);
				//float const x1 = 0.0f;
				float const y1 = 1.0f;
				//float const z1 = 0.0f;

				// Rotate around the z axis

				float const x2 = /*x1 * cosf(rotation1)*/ - y1 * sinf(rotation1);
				float const y2 = /*x1 * sinf(rotation1)*/ + y1 * cosf(rotation1);
				//float const z2 = z1;

				// Rotation around the y axis

				float const x3 = /*z2 * sinf(rotation2)*/ + x2 * cosf(rotation2);
				float const y3 = y2;
				float const z3 = /*z2 * cos(rotation2)*/ - x2 * sinf(rotation2);

				Vector localDirection(Vector(x3, y3, z3));

				// Set the initial position

				Vector localPosition(m_particleEmitterDescription.m_emitterShape->getValue() * shapeSize + localDirection * distance);

				// Local or world space

				if (m_particleEmitterDescription.m_localSpaceParticles)
				{
					particle.m_position = localPosition;
					particle.m_velocity = localDirection;
				}
				else
				{
					// Move the position based on the emit shape

					particle.m_position += m_object->getTransform_o2w().rotate_l2p(localPosition);
					particle.m_velocity = m_object->getTransform_o2w().rotate_l2p(localDirection);
				}
			}
			break;
		case ParticleEmitterDescription::ED_omni:
			{
				// Set the direction vector

				Vector localDirection(m_particleEmitterDescription.m_emitterShape->getValue());

				// Set the initial position

				Vector localPosition(localDirection * shapeSize + localDirection * distance);

				// Local or world space

				if (m_particleEmitterDescription.m_localSpaceParticles)
				{
					particle.m_position = localPosition;
					particle.m_velocity = localDirection;
				}
				else
				{
					// Move the position based on the emit shape

					particle.m_position += m_object->getTransform_o2w().rotate_l2p(localPosition);
					particle.m_velocity = m_object->getTransform_o2w().rotate_l2p(localDirection);
				}
			}
			break;
		case ParticleEmitterDescription::ED_invalid:
		default:
			{
				DEBUG_FATAL(1, ("Invalid emitter type specified."));
			}
	}

	float const particleSpeed = m_particleEmitterDescription.m_particleEmitSpeed.getValue(m_iterParticleEmitSpeed, emitterAgePercent) * effectScale;

	particle.m_velocity *= particleSpeed;
	particle.m_lifeTime = m_particleEmitterDescription.m_particleLifeTime.getValue(m_iterParticleLifeTime, emitterAgePercent);
	DEBUG_FATAL((particle.m_lifeTime < 0.0f), ("particle.m_lifeTime(%f) < 0", particle.m_lifeTime));
	particle.m_age = 0.0f;
	particle.m_weight = m_particleEmitterDescription.m_particleWeight.getValue(m_iterParticleWeight, emitterAgePercent) * effectScale;

	particle.m_iterSpeedScale.reset(m_particleEmitterDescription.m_particleDescription->getSpeedScale().getIteratorBegin());
	particle.m_iterParticleRelativeRotationX.reset(m_particleEmitterDescription.m_particleDescription->getParticleRelativeRotationX().getIteratorBegin());
	particle.m_iterParticleRelativeRotationY.reset(m_particleEmitterDescription.m_particleDescription->getParticleRelativeRotationY().getIteratorBegin());
	particle.m_iterParticleRelativeRotationZ.reset(m_particleEmitterDescription.m_particleDescription->getParticleRelativeRotationZ().getIteratorBegin());	

	if (m_particleEmitterDescription.m_particleSnapToTerrainOnCreation &&
	    !m_particleEmitterDescription.m_localSpaceParticles)
	{
		float terrainHeight;
		Vector terrainNormal;
		const bool heightValid = TerrainObject::getConstInstance()->getHeight(particle.m_position, terrainHeight, terrainNormal);

		if (heightValid)
		{
			// See if there's a floor to collide with above the terrain...
			FloorLocator floorLocator;
			Vector positionTest = particle.m_position;
			bool floorFound = false;
			if (m_object->getCellProperty() == CellProperty::getWorldCellProperty())
				floorFound = CollisionWorld::makeLocator(CellProperty::getWorldCellProperty(), positionTest, floorLocator);
			else
			{
				Transform const & cellToWorld = m_object->getParentCell()->getOwner().getTransform_o2w();
				positionTest = cellToWorld.rotateTranslate_p2l(positionTest);
				floorFound = CollisionWorld::makeLocator(m_object->getParentCell(), positionTest, floorLocator);
			}

			if (floorFound)
			{
				terrainHeight = floorLocator.getPosition_w().y;
				terrainNormal = floorLocator.getSurfaceNormal_w();
			}
		}

#ifdef _DEBUG
		++s_terrainHeightCallCount;
#endif // _DEBUG

		if (heightValid)
		{
			particle.m_position.y = terrainHeight + m_particleEmitterDescription.m_particleSnapToTerrainOnCreationHeight;

			if (m_particleEmitterDescription.m_particleAlignToTerrainNormalOnCreation)
			{
				particle.m_velocity = terrainNormal * particleSpeed;
			}
		}
	}

	particle.m_positionPrevious = particle.m_position;
}

//-----------------------------------------------------------------------------
void ParticleEmitter::createSingleParticle(float const deltaTime)
{
	if (m_particleEmitterDescription.m_particleDescription->getParticleType() == ParticleDescription::PT_quad)
	{
		ParticleDescriptionQuad const *particleDescriptionQuad = safe_cast<ParticleDescriptionQuad const *>(m_particleEmitterDescription.m_particleDescription);

		createSingleParticleQuad(particleDescriptionQuad, deltaTime);
	}
	else
	{
		if (m_particleEmitterDescription.m_particleDescription->getParticleType() == ParticleDescription::PT_mesh)
		{
			ParticleDescriptionMesh *particleDescrptionMesh = safe_cast<ParticleDescriptionMesh *>(m_particleEmitterDescription.m_particleDescription);

			createSingleParticleMesh(particleDescrptionMesh, deltaTime);
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::createSingleParticleMesh(ParticleDescriptionMesh const *particleDescriptionMesh, float const deltaTime)
{
	NOT_NULL(m_particles);

	if (!ParticleMesh::isParticlePoolFull())
	{
		ParticleMesh *particleMesh = new ParticleMesh();
		m_particles->push_back(particleMesh);
		particleMesh->m_object = new MemoryBlockManagedObject();
		particleMesh->m_object->setAppearance(AppearanceTemplateList::createAppearance(particleDescriptionMesh->getMeshPath().getString()));
		RenderWorld::addObjectNotifications(*particleMesh->m_object);

		if (m_particleEmitterDescription.m_localSpaceParticles)
		{
			m_object->addChildObject_o(particleMesh->m_object);
		}
		else
		{
			SetupClientParticle::addNotifications(*particleMesh->m_object);

			if (TerrainObject::getInstance() != NULL)
			{
				particleMesh->m_object->setParentCell(m_object->getParentCell());
				particleMesh->m_object->addToWorld();
			}
		}

		// Initialize the mesh particle values

		particleMesh->m_iterScale.reset(particleDescriptionMesh->m_scale.getIteratorBegin());
		particleMesh->m_iterRotationX.reset(particleDescriptionMesh->m_rotationX.getIteratorBegin());
		particleMesh->m_iterRotationY.reset(particleDescriptionMesh->m_rotationY.getIteratorBegin());
		particleMesh->m_iterRotationZ.reset(particleDescriptionMesh->m_rotationZ.getIteratorBegin());
		particleMesh->m_rotationInitial.x = (m_particleEmitterDescription.m_particleRandomInitialRotation) ? Random::randomReal(0.0f, 1.0f) : 1.0f;
		particleMesh->m_rotationInitial.x *= (particleDescriptionMesh->isRandomRotationDirection()) ? ((rand() % 2) ? 1.0f : -1.0f) : 1.0f;
		particleMesh->m_rotationInitial.y = (m_particleEmitterDescription.m_particleRandomInitialRotation) ? Random::randomReal(0.0f, 1.0f) : 1.0f;
		particleMesh->m_rotationInitial.y *= (particleDescriptionMesh->isRandomRotationDirection()) ? ((rand() % 2) ? 1.0f : -1.0f) : 1.0f;
		particleMesh->m_rotationInitial.z = (m_particleEmitterDescription.m_particleRandomInitialRotation) ? Random::randomReal(0.0f, 1.0f) : 1.0f;
		particleMesh->m_rotationInitial.z *= (particleDescriptionMesh->isRandomRotationDirection()) ? ((rand() % 2) ? 1.0f : -1.0f) : 1.0f;

		// Initialize the shared values

		initializeSingleParticle(*particleMesh);

		// Simulate the initial delta time

		updateSingleParticle(particleMesh, deltaTime);
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::createSingleParticleQuad(ParticleDescriptionQuad const *particleDescriptionQuad, float const deltaTime)
{
	NOT_NULL(m_particles);

	// Make sure the global list has room for another particle

	if (!ParticleQuad::isParticlePoolFull(m_particleEmitterDescription.m_usePriorityParticles))
	{
		// Create a new particle

		ParticleQuad *particleQuad = new ParticleQuad();

		// Get the particle index

		m_particles->push_back(particleQuad);

		// Initialize the quad particle values

		particleQuad->m_initialRotation = (m_particleEmitterDescription.m_particleRandomInitialRotation) ? Random::randomReal(0.0f, 1.0f) : 1.0f;
		particleQuad->m_initialRotation *= (!m_particleEmitterDescription.m_particleDescription->isRandomRotationDirection()) ? 1.0f : ((rand() % 2) ? 1.0f : -1.0f);
		particleQuad->m_iterLength.reset(particleDescriptionQuad->getLength().getIteratorBegin());
		particleQuad->m_iterWidth.reset(particleDescriptionQuad->getWidth().getIteratorBegin());
		particleQuad->m_iterRotation.reset(particleDescriptionQuad->getRotation().getIteratorBegin());
		particleQuad->m_iterColor.reset(m_particleEmitterDescription.m_particleDescription->getColor().getIteratorBegin());
		particleQuad->m_iterAlpha.reset(m_particleEmitterDescription.m_particleDescription->getAlpha().getIteratorBegin());

		// Initialize the shared values

		initializeSingleParticle(*particleQuad);

		// Set the direction the particle is facing

		if (particleQuad->m_velocity == Vector::zero)
		{
			// The particle is not moving, position it facing the direction the
			// emitter is moving

			Vector a(m_object->getPosition_w());
			Vector b(m_previousTransform_o2w.getPosition_p());
			Vector deltaPosition(a - b);

			if (deltaPosition == Vector::zero)
			{
				if (m_particleEmitterDescription.m_localSpaceParticles)
				{
					particleQuad->m_upVector = Vector::unitY;
				}
				else
				{
					particleQuad->m_upVector = m_object->getTransform_o2w().getLocalFrameJ_p();
				}
			}
			else
			{
				particleQuad->m_upVector = deltaPosition;
			}
		}
		else
		{
			particleQuad->m_upVector = particleQuad->m_velocity;
		}

		IGNORE_RETURN(particleQuad->m_upVector.normalize());

		if (m_particleEmitterDescription.m_particleOrientation == ParticleEmitterDescription::PO_orientWithVelocity)
		{
			if (m_particleEmitterDescription.m_localSpaceParticles)
			{
				Vector localFrameJ_o(m_object->getTransform_o2p().getLocalFrameJ_p());

				if (particleQuad->m_upVector == localFrameJ_o)
				{
					particleQuad->m_sideVector = particleQuad->m_upVector.cross(m_object->getTransform_o2p().getLocalFrameK_p());
				}
				else
				{
					particleQuad->m_sideVector = particleQuad->m_upVector.cross(localFrameJ_o);
				}
			}
			else
			{
				Vector localFrameJ_w(m_object->getTransform_o2w().getLocalFrameJ_p());

				if (particleQuad->m_upVector.withinEpsilon(localFrameJ_w,0.01f)) 
				{
					particleQuad->m_sideVector = particleQuad->m_upVector.cross(m_object->getTransform_o2w().getLocalFrameK_p());
				}
				else
				{
					particleQuad->m_sideVector = particleQuad->m_upVector.cross(localFrameJ_w);
				}

			}

			IGNORE_RETURN(particleQuad->m_sideVector.normalize());
		}

		// Simulate the initial delta time

		updateSingleParticle(particleQuad, deltaTime);
	}
}

//-----------------------------------------------------------------------------
float ParticleEmitter::getAgePercent() const
{
	DEBUG_FATAL((m_lifeTime < 0.0f), ("lifetime(%f) < 0", m_lifeTime));

	float result = 0.0f;

	if (m_age >= m_lifeTime)
	{
		result = 1.0f;
	}
	else
	{
		result = m_age / m_lifeTime;
	}

	return result;
}

//-----------------------------------------------------------------------------
void ParticleEmitter::updateExistingParticles(float const deltaTime)
{
	NOT_NULL(m_particles);

	if (deltaTime > 0.0f)
	{
		Particles::iterator particleListIter = m_particles->begin();

#ifdef _DEBUG
		unsigned int particleCount = m_particles->size();
		UNREF(particleCount);
#endif // _DEBUG

		for (; particleListIter != m_particles->end(); ++particleListIter)
		{
			Particle *particle = &(*(*particleListIter));

			updateSingleParticle(particle, deltaTime);
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::updateSingleParticle(Particle *particle, float const deltaTime)
{
	// Make sure the particle is still alive
	if (!particle->isAlive())
	{
		// This happens when the current delta pushes the particle age beyond its lifetime, no need to continue updating it since it will die this frame
		return;
	}

	// Particle is a little older
	particle->m_age += deltaTime;
	if(particle->m_age > particle->m_lifeTime)
	{
		particle->m_age = particle->m_lifeTime;
	}

	// Cache the age percent
	particle->calculateAgePercent();

	// Update the particle position
	float const particleSpeedScale = m_particleEmitterDescription.m_particleDescription->getSpeedScale().getValue(particle->m_iterSpeedScale, particle->getAgePercent());

	particle->m_positionPrevious = particle->m_position;
	float const percentOfWind = (1.0f - m_particleEmitterDescription.m_windResistenceGlobalPercent);
	Vector wind;
	
	if (   !m_particleEmitterDescription.m_localSpaceParticles
	    && ParticleEffectAppearance::isGlobalWindEnabled())
	{
		wind = percentOfWind * ParticleEffectAppearance::getGlobalWind();
	}
	
	particle->m_position = particle->m_position + deltaTime * particle->m_velocity * particleSpeedScale + wind * deltaTime;

	// Adjust the position for gravity

	float const weight = particle->m_weight;
	float const fall = (-9.8f * weight * deltaTime);

	if (m_particleEmitterDescription.m_particleGroundCollision)
	{
		// See if we have collided with the terrain
		float terrainHeight;
		Vector terrainNormal;
		bool floorCollide = false;

		bool heightValid = TerrainObject::getConstInstance()->getHeight(particle->m_position, terrainHeight, terrainNormal);
		FloorLocator floorLocator;

		if (heightValid)
		{
			// See if there's a floor to collide with above the terrain...
			if (m_object)
			{
				Vector positionTest = particle->m_position;
				positionTest.y = particle->m_positionPrevious.y;
				bool floorFound = false;
				if (m_object->getCellProperty() == CellProperty::getWorldCellProperty())
					floorFound = CollisionWorld::makeLocator(CellProperty::getWorldCellProperty(), positionTest, floorLocator);
				else
				{
					Transform const & cellToWorld = m_object->getParentCell()->getOwner().getTransform_o2w();
					positionTest = cellToWorld.rotateTranslate_p2l(positionTest);
					floorFound = CollisionWorld::makeLocator(m_object->getParentCell(), positionTest, floorLocator);
				}

				if (floorFound && floorLocator.getPosition_w().y > terrainHeight)
				{
					terrainHeight = floorLocator.getPosition_w().y;
					terrainNormal = floorLocator.getSurfaceNormal_w();
					floorCollide = true;
				}
			}
		}

#ifdef _DEBUG
		++s_terrainHeightCallCount;
#endif // _DEBUG

		if (heightValid)
		{
			float const heightAboveGround = m_particleEmitterDescription.m_particleCollisionHeightAboveGround;

			// If the particle is above the ground, add the gravity pull to it

			if (particle->m_position.y >= (terrainHeight + heightAboveGround))
			{
				particle->m_velocity.y += fall;
			}

			if (particle->m_position.y <= (terrainHeight + heightAboveGround))
			{
				bool collisionInfoOkay = false;;
				Vector collidePoint;
				Vector collideNormal; 

				if (!floorCollide)
				{
					Vector a(particle->m_positionPrevious);
					a.y -= heightAboveGround;
					Vector b(particle->m_position);
					b.y -= heightAboveGround;
					CollisionInfo collisionInfo;
					if (TerrainObject::getConstInstance()->collide(a, b, collisionInfo))
					{
						collisionInfoOkay = true;
						collidePoint = collisionInfo.getPoint();
						collideNormal = collisionInfo.getNormal();
					}
				}
				else
				{
					if (particle->m_positionPrevious.y >= (terrainHeight + heightAboveGround))
					{
						collisionInfoOkay = true;
						collidePoint = floorLocator.getPosition_w();
						collideNormal = floorLocator.getSurfaceNormal_w();
					}
				}

				if (collisionInfoOkay)
				{
					// Set the particle at the point of collision
					
					particle->m_position = collidePoint;
					particle->m_position.y += heightAboveGround;
					
					// Redirect the particle in the right direction
					
					Vector const &incomingVector = particle->m_velocity;
					Vector reflectedVector(collideNormal.reflectIncoming(incomingVector));
					particle->m_velocity = reflectedVector;

					particle->m_velocity.x *= m_particleForwardVelocityMaintained;
					particle->m_velocity.y *= m_particleUpVelocityMaintained;
					particle->m_velocity.z *= m_particleForwardVelocityMaintained;

					// See what we need to do with particle attachments

					bool const attachmentDescriptionExists = (m_particleEmitterDescription.m_particleDescription->getParticleAttachmentDescriptions().size() > 0);

					if ((particle->m_particleAttachment == NULL) && attachmentDescriptionExists && !ParticleAttachment::isPoolFull())
					{
						ParticleDescription::ParticleAttachmentDescriptions::const_iterator begin = m_particleEmitterDescription.m_particleDescription->getParticleAttachmentDescriptions().begin();
						ParticleDescription::ParticleAttachmentDescriptions::const_iterator end = m_particleEmitterDescription.m_particleDescription->getParticleAttachmentDescriptions().end();
						ParticleDescription::ParticleAttachmentDescriptions::const_iterator current = begin;

						for (; current != end; ++current)
						{
							if (current->getSpawn() == ParticleAttachmentDescription::S_onCollision)
							{
								std::string const &attachmentPath = current->getAttachmentPath();

								if (!attachmentPath.empty())
								{
									// Create the new attachment for this particle

									createParticleAttachment(*particle, attachmentPath, deltaTime);

									// Set the death mode

									bool const killAttachmentWhenParticleDies = m_particleEmitterDescription.m_particleDescription->getParticleAttachmentDescriptions().begin()->isKillAttachmentWhenParticleDies();

									particle->m_particleAttachment->setKillWhenParticleDies(killAttachmentWhenParticleDies);

									// Save off the attachment

									m_particleAttachments->push_back(Watcher<ParticleAttachment>(particle->m_particleAttachment));
								}
							}
						}
					}

					// See if we need to kill the particle at the time of collision

					if (m_particleEmitterDescription.m_particleKillParticlesOnCollision)
					{
						particle->kill();
					}
				}
				else
				{
					// I know this is crazy, but my implementation is waiting on some fixes from other people

					int i = 1;
					UNREF(i);
				}
			}
		}
		else
		{
			// I know this is crazy, but my implementation is waiting on some fixes from other people

			int i = 1;
			UNREF(i);
		}
	}
	else
	{
		particle->m_velocity.y += fall;
	}

	ParticleMesh *particleMesh = NULL;
	ParticleQuad *particleQuad = NULL;

	switch (m_particleEmitterDescription.m_particleDescription->getParticleType())
	{
		case ParticleDescription::PT_mesh:
			{
				ParticleDescriptionMesh *particleDescriptionMesh = safe_cast<ParticleDescriptionMesh *>(m_particleEmitterDescription.m_particleDescription);
				particleMesh = safe_cast<ParticleMesh *>(particle);

				if (particleMesh->m_object != NULL)
				{
					particleMesh->m_object->setPosition_w(particle->m_position);
				}
				if(m_particleEmitterDescription.m_particleOrientation == ParticleEmitterDescription::PO_faceCameraMesh)
				{
					if(particleMesh->m_object && m_localShaderPrimitive && m_localShaderPrimitive->m_camera)
					{
						Vector cameraWorld = m_localShaderPrimitive->m_camera->getPosition_p();
						particleMesh->m_object->lookAt_p(cameraWorld, Vector::unitY);
					}
				}
				else if (((m_particleEmitterDescription.m_particleOrientation == ParticleEmitterDescription::PO_orientWithVelocity) ||
					 (m_particleEmitterDescription.m_particleOrientation == ParticleEmitterDescription::PO_orientWithVelocityBankToCamera)) &&
					(m_particleEmitterDescription.m_flockingType != ParticleEmitterDescription::FT_ground) &&
				    (particleMesh->m_object != NULL))
				{
					Vector upVector(particle->m_velocity.cross(Vector::unitY).cross(particle->m_velocity));
					upVector.normalize();

					particleMesh->m_object->lookAt_p(particle->m_position + particle->m_velocity * 10.0f, upVector);
				}
				else
				{
					Transform objectToParentTransform;

					// Position

					objectToParentTransform.setPosition_p(particleMesh->m_position);

					// Pitch

					float rotationX = particleMesh->m_rotationInitial.x + particleDescriptionMesh->m_rotationX.getValue(particleMesh->m_iterRotationX, particleMesh->getAgePercent());

					if (particleMesh->m_rotationInitial.x < 0.0f)
					{
						rotationX *= -1.0f;
					}

					objectToParentTransform.pitch_l(rotationX * PI_TIMES_2);

					// Yaw

					float rotationY = particleMesh->m_rotationInitial.y + particleDescriptionMesh->m_rotationY.getValue(particleMesh->m_iterRotationY, particleMesh->getAgePercent());

					if (particleMesh->m_rotationInitial.y < 0.0f)
					{
						rotationY *= -1.0f;
					}

					objectToParentTransform.yaw_l(rotationY * PI_TIMES_2);

					// Roll

					float rotationZ = particleMesh->m_rotationInitial.z + particleDescriptionMesh->m_rotationZ.getValue(particleMesh->m_iterRotationZ, particleMesh->getAgePercent());

					if (particleMesh->m_rotationInitial.z < 0.0f)
					{
						rotationZ *= -1.0f;
					}

					objectToParentTransform.roll_l(rotationZ * PI_TIMES_2);

					// Set the transform

					particleMesh->m_object->setTransform_o2p(objectToParentTransform);
				}

				// Get the effect scale

				float const effectScale = getParentParticleEffectAppearance().getScale_w();

				// Scale the mesh

				if (particleMesh->m_object != NULL)
				{
					float const scale = particleDescriptionMesh->m_scale.getValue(particleMesh->m_iterScale, particleMesh->getAgePercent()) * effectScale;

					if (scale > 0.01)
					{
						Vector scaleVector(scale, scale, scale);
						particleMesh->m_object->setScale(scaleVector);
					}

					// Add this particle extents to the bounding emitter extents

					Extent const *appearanceExtent = particleMesh->m_object->getAppearance()->getExtent();
					
					float const radius = (appearanceExtent == NULL) ? 0.0f : appearanceExtent->getSphere().getRadius();

					m_extent_w.updateMinAndMax(Vector(particleMesh->m_position.x + radius, particleMesh->m_position.y + radius, particleMesh->m_position.z + radius));
					m_extent_w.updateMinAndMax(Vector(particleMesh->m_position.x - radius, particleMesh->m_position.y - radius, particleMesh->m_position.z - radius));
				}
			}
			break;
		case ParticleDescription::PT_quad:
			{
				ParticleDescriptionQuad *particleDescriptionQuad = safe_cast<ParticleDescriptionQuad *>(m_particleEmitterDescription.m_particleDescription);
				particleQuad = safe_cast<ParticleQuad *>(particle);

				// Get the length

				float const particleLength = particleDescriptionQuad->getLength().getValue(particleQuad->m_iterLength, particleQuad->getAgePercent());

				// Get the width

				float const particleWidth = particleDescriptionQuad->isLengthAndWidthLinked() ? particleLength : particleDescriptionQuad->getWidth().getValue(particleQuad->m_iterWidth, particleQuad->getAgePercent());

				// Re-orient the particle to match its velocity if the particle is set to "orient with velocity" mode and the
				// particle is moving. If the particle is not moving then the particle has an initial orientation and
				// does not need to be re-oriented

				if (((m_particleEmitterDescription.m_particleOrientation == ParticleEmitterDescription::PO_orientWithVelocity) ||
				     (m_particleEmitterDescription.m_particleOrientation == ParticleEmitterDescription::PO_orientWithVelocityBankToCamera)) &&
				    (particleQuad->m_velocity != Vector::zero))
				{
					if (particleQuad->m_velocity != Vector::zero)
					{
						particleQuad->m_upVector = particleQuad->m_velocity;
						particleQuad->m_upVector.normalize();
					}
				}

				// Add this particle extents to the bounding emitter extents

				float const radius = std::max(particleLength, particleWidth) * getParentParticleEffectAppearance().getScale_w();
				m_extent_w.updateMinAndMax(Vector(particleQuad->m_position.x + radius, particleQuad->m_position.y + radius, particleQuad->m_position.z + radius));
				m_extent_w.updateMinAndMax(Vector(particleQuad->m_position.x - radius, particleQuad->m_position.y - radius, particleQuad->m_position.z - radius));
			}
			break;
	}

	// Update any properties the particle needs to

	particle->alter(deltaTime);

	// See what we need to do with particle attachments

	bool const attachmentDescriptionExists = (m_particleEmitterDescription.m_particleDescription->getParticleAttachmentDescriptions().size() > 0);

	if ((particle->m_particleAttachment == NULL) && attachmentDescriptionExists && !ParticleAttachment::isPoolFull())
	{
		ParticleDescription::ParticleAttachmentDescriptions::const_iterator begin = m_particleEmitterDescription.m_particleDescription->getParticleAttachmentDescriptions().begin();
		ParticleDescription::ParticleAttachmentDescriptions::const_iterator end = m_particleEmitterDescription.m_particleDescription->getParticleAttachmentDescriptions().end();
		ParticleDescription::ParticleAttachmentDescriptions::const_iterator current = begin;

		for (; current != end; ++current)
		{
			float const startPercentMin = current->getStartPercentMin();
			float const startPercentMax = current->getStartPercentMax();
			float const startPercent = Random::randomReal(startPercentMin, startPercentMax);

			if (particle->getAgePercent() >= startPercent)
			{
				std::string const & attachmentPath = current->getAttachmentPath();

				if (!attachmentPath.empty())
				{
					// Create the new attachment for this particle

					createParticleAttachment(*particle, attachmentPath, deltaTime);

					// Set the death mode

					bool const killAttachmentWhenParticleDies = m_particleEmitterDescription.m_particleDescription->getParticleAttachmentDescriptions().begin()->isKillAttachmentWhenParticleDies();

					particle->m_particleAttachment->setKillWhenParticleDies(killAttachmentWhenParticleDies);

					// Save off the attachment

					m_particleAttachments->push_back(Watcher<ParticleAttachment>(particle->m_particleAttachment));
				}
			}
		}
	}

	// Flocking

	{
		if (m_particleEmitterDescription.m_flockingType != ParticleEmitterDescription::FT_none)
		{
			float flockingSeperationDistance = m_particleEmitterDescription.m_flockingSeperationDistance;
			float flockingAlignmentGain = m_particleEmitterDescription.m_flockingAlignmentGain;
			float flockingCohesionGain = m_particleEmitterDescription.m_flockingCohesionGain;
			float flockingSeperationGain = m_particleEmitterDescription.m_flockingSeperationGain;

			// Group position

			if (flockingCohesionGain > 0.0f)
			{
				Vector errorPosition(m_averageParticlePosition - particle->m_position);

				errorPosition *= flockingCohesionGain;
				errorPosition *= deltaTime;
				particle->m_velocity += errorPosition;
			}

			// Group velocity

			if (flockingAlignmentGain > 0.0f)
			{
				Vector errorVelocity(m_averageParticleVelocity - particle->m_velocity);

				errorVelocity *= flockingAlignmentGain;
				errorVelocity *= deltaTime;
				particle->m_velocity += errorVelocity;
			}

			// Group seperation

			if (flockingSeperationGain > 0.0f)
			{
				Particles::iterator iterParticles = m_particles->begin();

				Vector seperationVector(Vector::zero);
				float closestParticle = std::numeric_limits<float>::max();
				float const comfortDistance = (flockingSeperationDistance * flockingSeperationDistance);

				for (; iterParticles != m_particles->end(); ++iterParticles)
				{
					if (particle != (*iterParticles))
					{
						float const distanceFromParticle = Vector(particle->m_position - (*iterParticles)->m_position).magnitudeSquared();

						if ((distanceFromParticle <= comfortDistance) &&
							(distanceFromParticle < closestParticle))
						{
							closestParticle = distanceFromParticle;

							Vector const &neighborPosition = (*iterParticles)->m_position;

							seperationVector = (particle->m_position - neighborPosition);
						}
					}
				}

				seperationVector *= flockingSeperationGain;
				seperationVector *= deltaTime;

				particle->m_velocity += seperationVector;
			}

			// XZ cage

			float const cageWidth = m_particleEmitterDescription.m_flockingCageWidth;
			float const cageHeight = m_particleEmitterDescription.m_flockingCageHeight;
			Vector cageAvoidanceVector(particle->m_position - m_object->getPosition_w());

			if (m_particleEmitterDescription.m_flockingCageEdgeType == ParticleEmitterDescription::FCET_soft)
			{
				if (cageAvoidanceVector.x > cageWidth)
				{
					particle->m_velocity.x -= particle->m_velocity.magnitudeSquared() * deltaTime;
				}
				else if (cageAvoidanceVector.x < -cageWidth)
				{
					particle->m_velocity.x += particle->m_velocity.magnitudeSquared() * deltaTime;
				}

				if (cageAvoidanceVector.z > cageWidth)
				{
					particle->m_velocity.z -= particle->m_velocity.magnitudeSquared() * deltaTime;
				}
				else if (cageAvoidanceVector.z < -cageWidth)
				{
					particle->m_velocity.z += particle->m_velocity.magnitudeSquared() * deltaTime;
				}
			}
			else if (m_particleEmitterDescription.m_flockingCageEdgeType == ParticleEmitterDescription::FCET_hard)
			{
				// Soft internal cage

				if (cageAvoidanceVector.x > (cageWidth * 0.75f))
				{
					particle->m_velocity.x -= particle->m_velocity.magnitudeSquared() * deltaTime * 6.0f;
				}
				else if (cageAvoidanceVector.x < (-cageWidth * 0.75f))
				{
					particle->m_velocity.x += particle->m_velocity.magnitudeSquared() * deltaTime * 6.0f;
				}

				if (cageAvoidanceVector.z > (cageWidth * 0.75f))
				{
					particle->m_velocity.z -= particle->m_velocity.magnitudeSquared() * deltaTime * 6.0f;
				}
				else if (cageAvoidanceVector.z < (-cageWidth * 0.75f))
				{
					particle->m_velocity.z += particle->m_velocity.magnitudeSquared() * deltaTime * 6.0f;
				}

				// Hard cage

				if ((cageAvoidanceVector.x > cageWidth) &&
					(particle->m_velocity.x > 0.0f))
				{
					particle->m_velocity.x = -particle->m_velocity.x;
				}
				else if ((cageAvoidanceVector.x < -cageWidth) &&
						 (particle->m_velocity.x < 0.0f))
				{
					particle->m_velocity.x = -particle->m_velocity.x;
				}

				if ((cageAvoidanceVector.z > cageWidth) &&
					(particle->m_velocity.z > 0.0f))
				{
					particle->m_velocity.z = -particle->m_velocity.z;
				}
				else if ((cageAvoidanceVector.z < -cageWidth) &&
						 (particle->m_velocity.z < 0.0f))
				{
					particle->m_velocity.z = -particle->m_velocity.z;
				}
			}

			if (m_particleEmitterDescription.m_flockingType == ParticleEmitterDescription::FT_air ||
			    m_particleEmitterDescription.m_flockingType == ParticleEmitterDescription::FT_water)
			{
				// Air flocking does not do ground height checks, it just bounds the particles inside the cage

				if (m_particleEmitterDescription.m_flockingCageShape == ParticleEmitterDescription::FCS_cylinder)
				{
					if (m_particleEmitterDescription.m_flockingCageEdgeType == ParticleEmitterDescription::FCET_soft)
					{
					}
					else if (m_particleEmitterDescription.m_flockingCageEdgeType == ParticleEmitterDescription::FCET_hard)
					{
					}
					else
					{
						DEBUG_FATAL(true, ("Unknown flocking cage edge type"));
					}
				}
				else if (m_particleEmitterDescription.m_flockingCageShape == ParticleEmitterDescription::FCS_rectangle)
				{
					if (m_particleEmitterDescription.m_flockingCageEdgeType == ParticleEmitterDescription::FCET_soft)
					{
						if (cageAvoidanceVector.y > cageHeight)
						{
							particle->m_velocity.y -= particle->m_velocity.magnitudeSquared() * deltaTime;
						}
						else if (cageAvoidanceVector.y < 0.0f)
						{
							particle->m_velocity.y += particle->m_velocity.magnitudeSquared() * deltaTime;
						}
					}
					else if (m_particleEmitterDescription.m_flockingCageEdgeType == ParticleEmitterDescription::FCET_hard)
					{
						// Soft internal cage

						if (cageAvoidanceVector.y > (cageHeight * 0.75f))
						{
							particle->m_velocity.y -= particle->m_velocity.magnitudeSquared() * deltaTime * 6.0f;
						}
						else if (cageAvoidanceVector.y < (cageHeight * 0.25f))
						{
							particle->m_velocity.y += particle->m_velocity.magnitudeSquared() * deltaTime * 6.0f;
						}

						// Hard cage

						if ((cageAvoidanceVector.y > cageHeight) &&
						    (particle->m_velocity.y > 0.0f))
						{
							particle->m_velocity.y = -particle->m_velocity.y;
						}
						else if ((cageAvoidanceVector.y < 0.0f) &&
						         (particle->m_velocity.y < 0.0f))
						{
							particle->m_velocity.y = -particle->m_velocity.y;
						}
					}
					else
					{
						DEBUG_FATAL(true, ("Unknown flocking cage edge type"));
					}
				}
				else
				{
					DEBUG_FATAL(true, ("Unknown flocking cage shape"));
				}
			}
			else if (m_particleEmitterDescription.m_flockingType == ParticleEmitterDescription::FT_ground)
			{
				float terrainHeight;
				Vector terrainNormal;
				bool const heightValid = TerrainObject::getConstInstance()->getHeight(particle->m_position, terrainHeight, terrainNormal);

				if (heightValid)
				{
					// See if there's a floor to collide with above the terrain...
					if (m_object)
					{
						FloorLocator floorLocator;
						Vector positionTest = particle->m_position;
						bool floorFound = false;
						if (m_object->getCellProperty() == CellProperty::getWorldCellProperty())
							floorFound = CollisionWorld::makeLocator(CellProperty::getWorldCellProperty(), positionTest, floorLocator);
						else
						{
							Transform const & cellToWorld = m_object->getParentCell()->getOwner().getTransform_o2w();
							positionTest = cellToWorld.rotateTranslate_p2l(positionTest);
							floorFound = CollisionWorld::makeLocator(m_object->getParentCell(), positionTest, floorLocator);
						}

						if (floorFound && floorLocator.getPosition_w().y > terrainHeight)
						{
							terrainHeight = floorLocator.getPosition_w().y;
							terrainNormal = floorLocator.getSurfaceNormal_w();
						}
					}
				}


#ifdef _DEBUG
				++s_terrainHeightCallCount;
#endif // _DEBUG

				if (heightValid)
				{
					particle->m_velocity.y = 0.0f;
					particle->m_position.y = terrainHeight + cageHeight;
				}

				if (m_particleEmitterDescription.m_particleOrientation == ParticleEmitterDescription::PO_orientWithVelocity)
				{
					if ((particleMesh != NULL) &&
				        (particleMesh->m_object != NULL))
					{
						// Orient the mesh using the terrain normal

						particleMesh->m_object->lookAt_p(particle->m_position + particle->m_velocity * 10.0f, terrainNormal);
					}
					else if (particleQuad != NULL)
					{
						// Orient the quad using the terrain normal

						particleQuad->m_sideVector = particle->m_velocity.cross(terrainNormal);
						particleQuad->m_sideVector.normalize();
						particle->m_velocity = terrainNormal.cross(particleQuad->m_sideVector);
					}
				}
			}
			else
			{
				DEBUG_FATAL(true, ("Unknown flock type"));
			}

			// Make sure the particles do not lose speed

			float const effectScale = getParentParticleEffectAppearance().getScale_w();
			float const particleSpeed = m_particleEmitterDescription.m_particleEmitSpeed.getValue(m_iterParticleEmitSpeed, getAgePercent()) * effectScale * 2.0f;
			float const velocityMagnitude = particle->m_velocity.magnitude();

			if ((velocityMagnitude < particleSpeed) ||
			    (velocityMagnitude > particleSpeed))
			{
				particle->m_velocity.normalize();
				particle->m_velocity *= particleSpeed;
			}
		}
	}

	// Random particle direction change

	if (m_particleEmitterDescription.m_particleChangeDirectionRadian > 0.0f)
	{
		unsigned int randTime = static_cast<unsigned int>(Clock::framesPerSecond() * m_particleEmitterDescription.m_particleChangeDirectionTime);

		if ((randTime == 0) ||
		    ((randTime > 0) && ((rand() % randTime) == 0)))
		{
			float const magnitude = particle->m_velocity.magnitude();

			Vector upVector(0.0f, 1.0f, 0.0f);
			float const dotProduct = particle->m_velocity.dot(upVector);
			if (dotProduct < -0.1f &&
			    dotProduct > +0.1f)
			{
				upVector = Vector(1.0f, 0.0f, 0.0f);
			}

			Vector sideVector(particle->m_velocity.cross(upVector));
			upVector = particle->m_velocity.cross(sideVector);

			Transform transform;
			transform.setLocalFrameIJK_p(sideVector, upVector, particle->m_velocity);

			transform.pitch_l(Random::randomReal(-m_particleEmitterDescription.m_particleChangeDirectionRadian, m_particleEmitterDescription.m_particleChangeDirectionRadian));

			particle->m_velocity = transform.rotate_l2p(Vector(0.0f, 0.0f, magnitude));
			particle->m_velocity.normalize();
			particle->m_velocity *= magnitude;
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::calculateRenderOrientationVectors(ParticleDescriptionQuad const *particleDescriptionQuad, ParticleQuad &particle, Vector &upVector, Vector &sideVector, Transform const &transform) const
{
	// Get the current rotation amount

	float currentRotation = (particle.m_initialRotation + particleDescriptionQuad->getRotation().getValue(particle.m_iterRotation, particle.getAgePercent())) * PI_TIMES_2;

	if (particle.m_initialRotation < 0.0f)
	{
		currentRotation *= -1.0f;
	}
	
	// Get the up and side vector

	switch (m_particleEmitterDescription.m_particleOrientation)
	{
		case ParticleEmitterDescription::PO_faceCamera:
			{
				Transform cameraTransform_o2w(m_localShaderPrimitive->m_camera->getTransform_o2w());

				cameraTransform_o2w.roll_l(currentRotation);

				// Up vector

				upVector = cameraTransform_o2w.rotate_l2p(Vector::unitY);

				// Side vector

				sideVector = cameraTransform_o2w.rotate_l2p(Vector::unitX);
			}
			break;
		case ParticleEmitterDescription::PO_orientWithVelocity:
			{
				if (m_particleEmitterDescription.m_localSpaceParticles)
				{
					// Up vector

					upVector = transform.rotate_l2p(particle.m_position) - transform.rotate_l2p(particle.m_positionPrevious);

					if (upVector == Vector::zero)
					{
						if (particle.m_upVector == Vector::zero)
						{
							upVector = transform.rotate_l2p(Vector::unitX);
						}
						else
						{
							upVector = transform.rotate_l2p(particle.m_upVector);
						}
					}

					// Side vector

					sideVector = transform.rotate_l2p(particle.m_sideVector);
				}
				else
				{
					// Up vector

					upVector = particle.m_position - particle.m_positionPrevious;

					if (upVector == Vector::zero)
					{
						if (particle.m_upVector == Vector::zero)
						{
							upVector = Vector::unitX;
						}
						else
						{
							upVector = particle.m_upVector;
						}
					}

					// Side vector

					sideVector = particle.m_sideVector;
				}

				upVector.normalize();
			}
			break;
		case ParticleEmitterDescription::PO_orientWithVelocityBankToCamera:
			{
				if (m_particleEmitterDescription.m_localSpaceParticles)
				{
					// Up vector

					upVector = transform.rotate_l2p(particle.m_position) - transform.rotate_l2p(particle.m_positionPrevious);

					if (upVector == Vector::zero)
					{
						upVector = transform.rotate_l2p(particle.m_upVector);
					}

					upVector.normalize();

					// Side vector

					Vector directionToCamera(m_localShaderPrimitive->m_camera->getPosition_w() - transform.rotateTranslate_l2p(particle.m_position));

					sideVector = upVector.cross(directionToCamera);
				}
				else
				{
					// Up vector

					upVector = particle.m_upVector;

					// Side vector

					Vector directionToCamera(m_localShaderPrimitive->m_camera->getPosition_w() - particle.m_position);

					sideVector = upVector.cross(directionToCamera);
				}

				sideVector.normalize();
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("Invalid particle orientation specified"));
			}
			break;
	}

	if(m_particleEmitterDescription.m_particleDescription->getUsesParticleRelativeRotation())
	{
	
		// Add in the particle relative rotation

		Vector unitZ = sideVector.cross(upVector);

		Transform localTransform;
		localTransform.setLocalFrameIJK_p(sideVector, unitZ, upVector);

		float const particleRelativeRotationY = m_particleEmitterDescription.m_particleDescription->getParticleRelativeRotationY().getValue(particle.m_iterParticleRelativeRotationY, particle.getAgePercent());
		localTransform.yaw_l(particleRelativeRotationY * PI_TIMES_2);

		float const particleRelativeRotationX = m_particleEmitterDescription.m_particleDescription->getParticleRelativeRotationX().getValue(particle.m_iterParticleRelativeRotationX, particle.getAgePercent());
		localTransform.pitch_l(particleRelativeRotationX * PI_TIMES_2);

		float const particleRelativeRotationZ = m_particleEmitterDescription.m_particleDescription->getParticleRelativeRotationZ().getValue(particle.m_iterParticleRelativeRotationZ, particle.getAgePercent());
		localTransform.roll_l(particleRelativeRotationZ * PI_TIMES_2);
		
		upVector = localTransform.rotate_l2p(Vector::unitZ);
		sideVector = localTransform.rotate_l2p(Vector::unitX);		
	}
}

//-----------------------------------------------------------------------------
class ParticleDead
{
public:

	ParticleDead() {}

	bool operator() (Particle *particle) const
	{
		bool result = false;

		if (particle == NULL)
		{
			result = true;
		}
		else
		{
			bool particleDead = !particle->isAlive();

			if (particleDead)
			{
				delete particle;
				particle = NULL;
				result = true;
			}
		}

		return result;
	}
};

//-----------------------------------------------------------------------------
void ParticleEmitter::removeOldParticles()
{
	if (!m_particles->empty())
	{
		Particles::iterator iter = std::remove_if(m_particles->begin(), m_particles->end(), ParticleDead());
		IGNORE_RETURN(m_particles->erase(iter, m_particles->end()));
	}
}

//-----------------------------------------------------------------------------
class ParticleAttachmentDead
{
public:

	ParticleAttachmentDead() {}

	bool operator() (Watcher<ParticleAttachment>(particleAttachment)) const
	{
		bool result = false;

		if (particleAttachment == NULL)
		{
			// This means a particle deleted the attachment forcefully

			result = true;
		}

		return result;
	}
};

//-----------------------------------------------------------------------------
void ParticleEmitter::removeOldAttachments()
{
	if ((m_particleAttachments != NULL) && !m_particleAttachments->empty())
	{
#ifdef _DEBUG
		int const particleAttachmentsSize = m_particleAttachments->size();
		UNREF(particleAttachmentsSize);
#endif // _DEBUG

		ParticleAttachments::iterator iter = std::remove_if(m_particleAttachments->begin(), m_particleAttachments->end(), ParticleAttachmentDead());
		IGNORE_RETURN(m_particleAttachments->erase(iter, m_particleAttachments->end()));
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::createNewParticles(float const deltaTime)
{	
	float const emitterAgePercent = getAgePercent();

	if (m_particleEmitterDescription.m_emitterOneShot)
	{
		// This handles one shots that shoot one time and it handles one shots that emit groups of particles at
		// a time.

		if ((m_particles->empty()) || m_particleEmitterDescription.m_emitterLoopImmediately)
		{
			int const generationRate = Random::random(m_particleEmitterDescription.m_emitterOneShotMin, m_particleEmitterDescription.m_emitterOneShotMax);

			// Make sure there is room for all the requested one shot particles, if not it waits until there
			// is room for the whole bunch.

			if ((m_particles->size() + generationRate) <= m_particleEmitterDescription.m_emitterMaxParticles)
			{
				m_newParticles = static_cast<float>(generationRate);
			}
		}
	}
	else
	{
		float const generationRate = m_particleEmitterDescription.m_particleGenerationRate.getValue(m_iterParticleGenerationRate, emitterAgePercent);

		switch (m_particleEmitterDescription.m_generationType)
		{
			case ParticleEmitterDescription::G_rate:
				{
					m_newParticles += deltaTime * m_lodPercent * generationRate;
				}
				break;
			case ParticleEmitterDescription::G_distance:
				{
					// Get the effect scale

					float const effectScale = getParentParticleEffectAppearance().getScale_w();
					float const newParticles = sqrt(m_accumulatedDistance / (generationRate * generationRate * effectScale));
					m_newParticles += newParticles * m_lodPercent;
					m_accumulatedDistance = 0.0f;
				}
				break;
			default:
				{
					DEBUG_FATAL(true, ("Unexpected particle generation method specified."));
				}
				break;
		}
	}

	if(m_particleEmitterDescription.m_firstParticleImmediately && m_frameFirst && (m_newParticles < 1.0f))
	{
		//Make sure a particle shoots out first time
		m_newParticles += 1.0f;
	}


	while (m_newParticles >= 1.0f)
	{
		// Throw out new particles if there is no room for them in this emitter

		if (m_particles->size() < m_particleEmitterDescription.m_emitterMaxParticles)
		{
			if (m_particleEmitterDescription.m_emitterOneShot || (m_particleEmitterDescription.m_firstParticleImmediately && m_frameFirst))
			{
				// One shot particles should all be synchronized with the delta time

				createSingleParticle(deltaTime);
			}
			else
			{
				createSingleParticle(deltaTime * Random::randomReal(0.0f, 1.0f));
			}
		}

		m_newParticles -= 1.0f;
	}
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
int ParticleEmitter::getTerrainHeightCallCount()
{
	return s_terrainHeightCallCount;
}
#endif // _DEBUG

//-----------------------------------------------------------------------------
BoxExtent const &ParticleEmitter::getExtent() const
{
	return m_extent_w;
}

//-----------------------------------------------------------------------------
void ParticleEmitter::frameFirst()
{
	if (!m_soundId.isValid() &&
	    !m_particleEmitterDescription.getSoundPath().empty() &&
	    m_object->isInWorld())
	{
		DEBUG_FATAL((m_soundId.isValid()), ("The sound id (%d) for the particle emitter should be 0 (invalid).", m_soundId.getId()));
		m_soundId = Audio::attachSound(m_particleEmitterDescription.getSoundPath().c_str(), m_object);
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::frameLast()
{
	if (!isInfiniteLooping())
	{
		Audio::stopSound(m_soundId, 1.0f);
		m_soundId.invalidate();
	}
}

//-----------------------------------------------------------------------------
Particle const &ParticleEmitter::getParticle(int const particleIndex) const
{
	NOT_NULL(m_particles);

	return *(*m_particles)[particleIndex];
}

//-----------------------------------------------------------------------------
void ParticleEmitter::calculateLod()
{
	float percent = 1.0f;

	if ((m_object != NULL) &&
	    !m_particleEmitterDescription.isEmitterUsingNoLod())
	{
		float const distanceToEmitter = (m_object->getTransform_o2w().getPosition_p() - m_currentCameraPosition_w).approximateMagnitude();
		float const extentRadius = m_extent_w.getSphere().getRadius();
		float distance = distanceToEmitter;

		if ((!m_particles->empty()) &&
		    (extentRadius > 0.0f))
		{
			// Verify the extent box not inverted

			if (m_extent_w.getLeft() > m_extent_w.getRight() ||
			    m_extent_w.getBottom() > m_extent_w.getTop() ||
				m_extent_w.getBack() > m_extent_w.getFront())
			{
				// Invalid box extent

				DEBUG_WARNING(true, ("Particle extent box is inverted: left: %.2f right: %.2f bottom: %.2f top %.2f back: %.2f front: %.2f", m_extent_w.getLeft(), m_extent_w.getRight(), m_extent_w.getBottom(), m_extent_w.getTop(), m_extent_w.getBack(), m_extent_w.getFront()));
			}
			else
			{
				// Box extent is valid, calculate the camera percent between the min and max lod distance

				Vector const closestPoint(Distance3d::ClosestPointABox(m_currentCameraPosition_w, m_extent_w.getBox()));
				float const distanceToExtent = (closestPoint - m_currentCameraPosition_w).approximateMagnitude();

				// Set the lod distance to the smaller of the 2 distances

				distance = (distanceToEmitter < distanceToExtent) ? distanceToEmitter : distanceToExtent;
			}
		}

		float minLodDistance = 0.0f;
		float maxLodDistance = 0.0f;

		if (m_particleEmitterDescription.isEmitterUsingGlobalLod())
		{
			minLodDistance = ConfigClientParticle::getMinGlobalLodDistance();
			maxLodDistance = ConfigClientParticle::getMaxGlobalLodDistance();
		}
		else
		{
			minLodDistance = m_particleEmitterDescription.getEmitterLodDistanceMin();
			maxLodDistance = m_particleEmitterDescription.getEmitterLodDistanceMax();
		}

		minLodDistance *= getParentParticleEffectAppearance().getScale_w();
		maxLodDistance *= getParentParticleEffectAppearance().getScale_w();

		float const lodDifference = (maxLodDistance - minLodDistance);

		if (lodDifference > 0.0f)
		{
			percent = 1.0f - (distance - minLodDistance) / lodDifference;
		}

		percent = clamp(0.0f, percent * getParentParticleEffectAppearance().getLodBias(), 1.0f);
	}

	m_lodPercent = percent;
}

//-----------------------------------------------------------------------------
void ParticleEmitter::setOwner(Object *newOwner)
{
	DEBUG_WARNING((newOwner == NULL), ("Setting a NULL owner."));

	if ((m_object != NULL) &&
	    (newOwner != NULL))
	{
		Object * const parent = m_object->getParent();
		if (parent != NULL)
		{
			parent->removeChildObject(m_object, Object::DF_none);
		}

		newOwner->addChildObject_o(m_object);
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::drawDebugAxis() const
{
	// Draw an axis of the emitter

	float const size = 3.0f;

	Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorZStaticShader());
	Graphics::setObjectToWorldTransformAndScale(m_object->getTransform_o2w(), Vector(size, size, size));
	Graphics::drawFrame();
}

//-----------------------------------------------------------------------------
void ParticleEmitter::drawDebugVelocity() const
{
	Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorZStaticShader());

	if (m_particleEmitterDescription.m_localSpaceParticles)
	{
		Graphics::setObjectToWorldTransformAndScale(m_object->getTransform_o2w(), Vector::xyz111);
	}
	else
	{
		Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
	}

	Particles::const_iterator iterParticles = m_particles->begin();

	for (; iterParticles != m_particles->end(); ++iterParticles)
	{
		Particle *particle = (*iterParticles);

		Vector velocity(particle->m_velocity);
		velocity.approximateNormalize();

		Vector a(particle->m_position);
		Vector b(particle->m_position + velocity);

		Graphics::drawLine(a, b, VectorArgb::solidWhite);
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::drawDebugParticleOrientation() const
{
	if (m_particleEmitterDescription.m_particleDescription->getParticleType() == ParticleDescription::PT_quad)
	{
		Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorZStaticShader());
		
		if (m_particleEmitterDescription.m_localSpaceParticles)
		{
			Graphics::setObjectToWorldTransformAndScale(m_object->getTransform_o2w(), Vector::xyz111);
		}
		else
		{
			Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);
		}

		Particles::const_iterator iterParticles = m_particles->begin();
	
		for (; iterParticles != m_particles->end(); ++iterParticles)
		{
			ParticleQuad const *particle = safe_cast<ParticleQuad const *>(*iterParticles);
		
			// Up vector

			Vector a(particle->m_position);
			Vector b(particle->m_position + particle->m_upVector);
			
			Graphics::drawLine(a, b, VectorArgb::solidGreen);

			// Side vector

			Vector c(particle->m_position);
			Vector d(particle->m_position + particle->m_sideVector);
			
			Graphics::drawLine(c, d, VectorArgb::solidRed);
		}
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::drawDebugExtents() const
{
	Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorZStaticShader());
	Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);

	Graphics::drawExtent(&m_extent_w, VectorArgb::solidCyan);

	//BoxExtent renderExtent;
	//
	//renderExtent.setMin(m_extent_w.getMin() + m_object->getPosition_w());
	//renderExtent.setMax(m_extent_w.getMax() + m_object->getPosition_w());
	//
	//Graphics::drawExtent(&renderExtent, VectorArgb::solidCyan);
}

//-----------------------------------------------------------------------------
void ParticleEmitter::addToWorld()
{
	if (Audio::isSoundValid(m_soundId) &&
	    !m_particleEmitterDescription.getSoundPath().empty() &&
	    m_object->isInWorld())
	{
		DEBUG_FATAL((m_soundId.isValid()), ("The sound id (%d) for the particle emitter should be 0 (invalid).", m_soundId.getId()));
		m_soundId = Audio::attachSound(m_particleEmitterDescription.getSoundPath().c_str(), m_object);
	}
}

//-----------------------------------------------------------------------------
void ParticleEmitter::removeFromWorld()
{
	Audio::stopSound(m_soundId, 1.0f);
	m_soundId.invalidate();
}

//-----------------------------------------------------------------------------
int ParticleEmitter::getCurrentLoopCount()
{
	return m_loopCount;
}

//-----------------------------------------------------------------------------
bool ParticleEmitter::isInfiniteLooping() const
{
	return (m_loopCount == -1);
}

//-----------------------------------------------------------------------------
void ParticleEmitter::createParticleAttachment(Particle & particle, std::string const & attachmentPath, float const deltaTime)
{
	DEBUG_WARNING(!TreeFile::exists(attachmentPath.c_str()), ("Trying to attach an asset to a particle that does not exist in the treefile: %s", attachmentPath.c_str()));

	Object * const parentObject = m_particleEmitterDescription.m_localSpaceParticles ? m_object.getPointer() : NULL;

	particle.m_particleAttachment = new ParticleAttachment();

	RenderWorld::addObjectNotifications(*particle.m_particleAttachment);

	if (parentObject != NULL)
	{
		// Attach this particle system as a child object

		parentObject->addChildObject_o(particle.m_particleAttachment);
		particle.m_particleAttachment->setPosition_p(particle.m_position);
	}
	else
	{
		// This particle system has no owner and is rendered in the world

		SetupClientParticle::addNotifications(*particle.m_particleAttachment);
		
		if (TerrainObject::getInstance() != NULL)
		{
			particle.m_particleAttachment->setParentCell(getParentParticleEffectAppearance().getOwner()->getParentCell());
			particle.m_particleAttachment->addToWorld();
			particle.m_particleAttachment->setPosition_w(particle.m_position);
		}
	}

	Appearance * const appearance = AppearanceTemplateList::createAppearance(attachmentPath.c_str());
	particle.m_particleAttachment->setAppearance(appearance);

	ParticleEffectAppearance * const particleEffectAppearance = ParticleEffectAppearance::asParticleEffectAppearance(appearance);

	if (particleEffectAppearance != NULL)
	{
		particleEffectAppearance->setParentParticleEffectAppearance(getParentParticleEffectAppearance());
		
		// Needs an update when first created

		particleEffectAppearance->alter(deltaTime);
	}
	else
	{
		SwooshAppearance * const swooshAppearance = SwooshAppearance::asSwooshAppearance(appearance);

		if (swooshAppearance != NULL)
		{
			swooshAppearance->setReferenceObject(particle.m_particleAttachment);
		}
	}
}

// ============================================================================
