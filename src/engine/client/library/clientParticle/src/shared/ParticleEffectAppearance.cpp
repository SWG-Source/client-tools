// ============================================================================
//
// ParticleEffectAppearance.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientParticle/FirstClientParticle.h"
#include "clientParticle/ParticleEffectAppearance.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientParticle/ConfigClientParticle.h"
#include "clientParticle/ParticleEffectAppearanceTemplate.h"
#include "clientParticle/ParticleEmitterGroup.h"
#include "clientParticle/ParticleManager.h"
#include "dpvsModel.hpp"
#include "dpvsObject.hpp"
#include "sharedCollision/Distance3d.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/Transform.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedUtility/LocalMachineOptionManager.h"

#include <vector>

// ============================================================================
//
// ParticleEffectAppearanceNamespace
//
// ============================================================================

//--------------------------------------------------------------------------
namespace ParticleEffectAppearanceNamespace
{
	Vector     s_globalWind(0.0f, 0.0f, 0.0f);
	float      s_globalLodBias = 1.0f;
	bool       s_globalWindEnabled = true;
	VectorArgb s_globalColorModifier(1.0f, 1.0f, 1.0f, 1.0f);
	int        s_globalUserLimit = 2048;

	float      s_originIconSize = 0.25f;
	//Sphere     s_originIconSphere( Vector::zero, s_originIconSize );
	BoxExtent  s_selectionExtent( AxialBox( Vector(-s_originIconSize, -s_originIconSize, -s_originIconSize), Vector(s_originIconSize, s_originIconSize, s_originIconSize) ));

	typedef stdvector<ParticleEmitterGroup *>::fwd ParticleEmitterGroupList;
	typedef stdvector<ParticleEmitterGroupList *>::fwd ParticleEmitterGroupListList;

	ParticleEmitterGroupListList ms_particleEmitterGroupListList;
	ParticleEmitterGroupList * newParticleEmitterGroupList();
	void deleteParticleEmitterGroupList(ParticleEmitterGroupList * particleEmitterGroupList);
}

using namespace ParticleEffectAppearanceNamespace;

//-----------------------------------------------------------------------------
ParticleEmitterGroupList * ParticleEffectAppearanceNamespace::newParticleEmitterGroupList()
{
	if (ms_particleEmitterGroupListList.empty())
	{
		for (int i = 0; i < 256; ++i)
			ms_particleEmitterGroupListList.push_back(new ParticleEmitterGroupList);
	}

	ParticleEmitterGroupList * const result = ms_particleEmitterGroupListList.back();
	ms_particleEmitterGroupListList.pop_back();

	return result;
}

//-----------------------------------------------------------------------------
void ParticleEffectAppearanceNamespace::deleteParticleEmitterGroupList(ParticleEmitterGroupList * particleEmitterGroupList)
{
	if (ms_particleEmitterGroupListList.size() < 256)
	{
		DEBUG_FATAL(!particleEmitterGroupList->empty(), ("particleEmitterGroupList is not empty"));
		particleEmitterGroupList->clear();
		ms_particleEmitterGroupListList.push_back(particleEmitterGroupList);
	}
	else
		delete particleEmitterGroupList;
}

// ============================================================================
//
// ParticleEffectAppearance
//
// ============================================================================

//----------------------------------------------------------------------
MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ParticleEffectAppearance, true, 0, 0, 0);

//----------------------------------------------------------------------
void ParticleEffectAppearance::install()
{
	installMemoryBlockManager();

	LocalMachineOptionManager::registerOption (s_globalLodBias, "ParticleEffectAppearance", "globalLodBias");
	ParticleEffectAppearance::setGlobalLodBias(s_globalLodBias);

	LocalMachineOptionManager::registerOption (s_globalUserLimit, "ParticleEffectAppearance", "globalUserLimit");
	ParticleEffectAppearance::setGlobalUserLimit(s_globalUserLimit);

	ExitChain::add(remove, "ParticleEffectAppearance::remove");
}

//----------------------------------------------------------------------
void ParticleEffectAppearance::remove()
{
	while (!ms_particleEmitterGroupListList.empty())
	{
		delete ms_particleEmitterGroupListList.back();
		ms_particleEmitterGroupListList.pop_back();
	}

	removeMemoryBlockManager();
}

//--------------------------------------------------------------------------
int ParticleEffectAppearance::getGlobalCount()
{
	return ms_memoryBlockManager->getElementCount();
}

//--------------------------------------------------------------------------
ParticleEffectAppearance::ParticleEffectAppearance(ParticleEffectAppearanceTemplate const *particleEffectAppearanceTemplate)
 : Appearance(particleEffectAppearanceTemplate)
 , m_particleEffectAppearanceTemplate(particleEffectAppearanceTemplate)
 , m_particleEmitterGroups(NULL)
 , m_parentParticleEffectAppearance(NULL)
 , m_age(0.0f)
 , m_playBackRate(1.0f)
 , m_paused(false)
 , m_enabled(true)
 , m_extent()
 , m_accumulatedSmallerRadiusChanges(0)
 , m_autoDelete(true)
 , m_unBounded(false)
 , m_dpvsObject(NULL)
 , m_currentCameraPosition_w()
 , m_infiniteLooping(m_particleEffectAppearanceTemplate->isInfiniteLooping())
 , m_colorModifier(VectorArgb::solidWhite)
 , m_lodBias(1.0f)
 , m_restartOnRemoveFromWorld(true)
 , m_scale_w(1.0f)
#ifdef _DEBUG
 , m_originIcon(NULL)
#endif

{
#ifdef _DEBUG
	ParticleManager::debugRegister(*this);
#endif // _DEBUG

	setScale(Vector::xyz111);

	// Set the extent initial value, maybe on the first frame of ths effect, set the extent to the object position

	m_extent.setMin(Vector::zero);
	m_extent.setMax(Vector::zero);

	m_particleEmitterGroups = newParticleEmitterGroupList();
	m_particleEmitterGroups->reserve(m_particleEffectAppearanceTemplate->m_particleEffectDescription->getParticleEmitterGroupDescriptions().size());

	// Create the ParticleEmitterGroup objects from the descriptions

	ParticleEffectDescription::ParticleEmitterGroupDescriptions::const_iterator current = m_particleEffectAppearanceTemplate->m_particleEffectDescription->getParticleEmitterGroupDescriptions().begin();
	
	for (; current != m_particleEffectAppearanceTemplate->m_particleEffectDescription->getParticleEmitterGroupDescriptions().end(); ++current)
	{
		ParticleEmitterGroup *particleEmitterGroup = new ParticleEmitterGroup(*current, this);
		particleEmitterGroup->restart();

		m_particleEmitterGroups->push_back(particleEmitterGroup);
	}

	m_dpvsObject = RenderWorld::createObject(this, m_extent.getBox());

#ifdef _DEBUG
	m_originIcon = AppearanceTemplateList::createAppearance("appearance/godclient_particle_handle.apt");
#endif
	
	useRenderEffectsFlag(true);
}

//--------------------------------------------------------------------------
ParticleEffectAppearance::~ParticleEffectAppearance()
{
	// Delete all the effects

	ParticleEmitterGroups::iterator iterParticleEmitterGroups = m_particleEmitterGroups->begin();

	for (; iterParticleEmitterGroups != m_particleEmitterGroups->end(); ++iterParticleEmitterGroups)
	{
		ParticleEmitterGroup * const particleEmitterGroup = NON_NULL(*iterParticleEmitterGroups);
		delete particleEmitterGroup;
	}
	m_particleEmitterGroups->clear();

	deleteParticleEmitterGroupList(m_particleEmitterGroups);
	m_particleEmitterGroups = NULL;
	m_parentParticleEffectAppearance = NULL;
	m_particleEffectAppearanceTemplate = NULL;

	IGNORE_RETURN(m_dpvsObject->release());
	m_dpvsObject = NULL;

#ifdef _DEBUG
	delete m_originIcon;
	m_originIcon = NULL;
#endif

#ifdef _DEBUG
	ParticleManager::debugUnRegister(*this);
#endif // _DEBUG
}

//--------------------------------------------------------------------------

DPVS::Object * ParticleEffectAppearance::getDpvsObject() const
{
	return m_dpvsObject;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::render() const
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ParticleEffectAppearance::render");

	// Make sure particles are enabled

	if (ParticleManager::isParticlesEnabled())
	{
		m_currentCameraPosition_w = ShaderPrimitiveSorter::getCurrentCameraPosition();

		// Render all the effects

		ParticleEmitterGroups::const_iterator iterParticleEmitterGroups = m_particleEmitterGroups->begin();

		for (; iterParticleEmitterGroups != m_particleEmitterGroups->end(); ++iterParticleEmitterGroups)
		{
			ParticleEmitterGroup const *particleEmitterGroup = (*iterParticleEmitterGroups);

			particleEmitterGroup->addToCameraScene(&ShaderPrimitiveSorter::getCurrentCamera(), getOwner());
		}

#ifdef _DEBUG
		if (ParticleManager::isDebugOriginIconEnabled())
		{
			drawOriginIcon();
		}

		// Draw extents

		if (ParticleManager::isDebugExtentsEnabled())
		{
			drawDebugExtents();
		}
#endif // _DEBUG
	}
}

//--------------------------------------------------------------------------
float ParticleEffectAppearance::alter(float deltaTime)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("ParticleEffectAppearance::alter");

	// Make sure particles are enabled

	if (!ParticleManager::isParticlesEnabled())
	{
		// Since particles are disabled, just kill them immediatelly

		return AlterResult::cms_kill;
	}
	else if (   (m_parentParticleEffectAppearance == NULL)
	         && (deltaTime > 2.0f))
	{
		// If we for some insane reason get a huge delta time, clamp it, otherwise it will lock potentially up the game while the particles alter internally

		deltaTime = clamp(0.0f, deltaTime, 2.0f);
	}

	// Don't alter the particle system if it is infinite looping and 
	// it was not rendered this frame

	if (isInfiniteLooping() &&
		!getRenderedThisFrame())
	{
		if (m_age > 20.0f)
		{
			return AlterResult::cms_alterNextFrame;
		}
		else
		{
			bool hasEverythingAltered = true;

			ParticleEmitterGroups::const_iterator iterParticleEmitterGroups = m_particleEmitterGroups->begin();

			for (; iterParticleEmitterGroups != m_particleEmitterGroups->end(); ++iterParticleEmitterGroups)
			{
				ParticleEmitterGroup const *particleEmitterGroup = (*iterParticleEmitterGroups);

				hasEverythingAltered = particleEmitterGroup->hasChildrenLoopedOnce();

				if (!hasEverythingAltered)
				{
					break;
				}
			}

			if (hasEverythingAltered)
			{
				return AlterResult::cms_alterNextFrame;
			}
		}
	}

	float const playBackRate_w = getPlayBackRate_w();
	bool const pausedByPlayBackRate = ((deltaTime * playBackRate_w) <= 0.0f);

	if (!m_paused && !pausedByPlayBackRate)
	{
		// Update all the effects

		float deltaTimeLeft = deltaTime;
		float const maxDeltaTime = 0.25f;
		
		while (deltaTimeLeft > 0.0f)
		{
			float const delta = (deltaTimeLeft < maxDeltaTime) ? deltaTimeLeft : maxDeltaTime;

			// Update the effects as normal after the ramp up time and
			// start up time is over

			ParticleEmitterGroups::iterator current = m_particleEmitterGroups->begin();

			for (; current != m_particleEmitterGroups->end(); ++current)
			{
				ParticleEmitterGroup *particleEmitterGroup = (*current);

				particleEmitterGroup->alter(delta * playBackRate_w);
			}

			deltaTimeLeft -= delta;
		}

		// Build the extent around the effect
		
		ParticleEmitterGroups::iterator iterParticleEmitterGroups = m_particleEmitterGroups->begin();

		AxialBox newBox_w;
		bool addedSomeParticles = false;

		const Object *owner = getOwner();
		const Transform &o2w = (owner) ? owner->getTransform_o2w() : getTransform_w();

		for (; iterParticleEmitterGroups != m_particleEmitterGroups->end(); ++iterParticleEmitterGroups)
		{
			ParticleEmitterGroup const *particleEmitterGroup = (*iterParticleEmitterGroups);

			if (particleEmitterGroup->hasAliveParticles())
			{
				newBox_w.add(particleEmitterGroup->getExtent().getBox());
				addedSomeParticles = true;
			}
		}

		if (!addedSomeParticles)
		{
			Vector ownerPosition(o2w.getPosition_p());

			newBox_w.add(ownerPosition);
		}

		// newBox is in world space, but we need object space
		AxialBox newBox;
		newBox.add( o2w.rotateTranslate_p2l(newBox_w.getMin()) );
		newBox.add( o2w.rotateTranslate_p2l(newBox_w.getMax()) );
			
		bool updatedExtent = false;

		if (!m_extent.getBox().contains(newBox))
		{
			m_accumulatedSmallerRadiusChanges = 0;
			m_extent.setBox(newBox);
			updatedExtent = true;
		}
		else
		{
			++m_accumulatedSmallerRadiusChanges;
		
			if (m_accumulatedSmallerRadiusChanges > 16)
			{
				m_accumulatedSmallerRadiusChanges = 0;
				m_extent.setBox(newBox);
				updatedExtent = true;
			}
		}

		if (updatedExtent)
		{
			DEBUG_FATAL(m_extent.getBox().isEmpty(), ("Invalid extent"));
			//DEBUG_REPORT_LOG(true, ("ParticleEffectAppearance: m_extent.getRadius(%f) min(%f, %f, %f) max(%f, %f, %f)\n", m_extent.getRadius(), m_extent.getMin().x, m_extent.getMin().y, m_extent.getMin().z, m_extent.getMax().x, m_extent.getMax().y, m_extent.getMax().z));

			Sphere const & sphere = m_extent.getSphere();

			if (sphere.getRadius() > 500.0f)
			{
				//DEBUG_REPORT_LOG(true, ("ParticleEffectAppearance: name(%s) m_scale_w(%f) m_extent.getRadius(%f) min(%f, %f, %f) max(%f, %f, %f)  sphere(%f, %f, %f, %f)\n", getAppearanceTemplate()->getName(), m_scale_w, m_extent.getRadius(), m_extent.getMin().x, m_extent.getMin().y, m_extent.getMin().z, m_extent.getMax().x, m_extent.getMax().y, m_extent.getMax().z, sphere.getCenter().x, sphere.getCenter().y, sphere.getCenter().z, sphere.getRadius()));
				m_dpvsObject->set(DPVS::Object::UNBOUNDED, true);
				{
					DPVS::Model *const model = RenderWorld::fetchDefaultModel();
					m_dpvsObject->setTestModel(model);
					IGNORE_RETURN(model->release());
				}
			}
			else
			{
				DPVS::SphereModel *const model = RenderWorld::fetchSphereModel(sphere);
				m_dpvsObject->setTestModel(model);
				IGNORE_RETURN(model->release());
			}
		}

		// Update the age last because some internal values are calculated based on the age

		m_age += deltaTime;
	}

	float result = AlterResult::cms_alterNextFrame;

	if (!isAlive())
	{
		result = AlterResult::cms_kill;
	}

#ifdef _DEBUG
	m_originIcon->alter( deltaTime );
#endif

	return result;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setKeepAlive(bool const keepAlive)
{
	Appearance::setKeepAlive(keepAlive);

	if (   keepAlive
	    && !isInfiniteLooping())
	{
		DEBUG_REPORT_LOG(true, ("The particle effect is set to keep alive but the particle effect is not infinite looping: %s\n", m_particleEffectAppearanceTemplate->getName()));
	}
}

//--------------------------------------------------------------------------
Extent const *ParticleEffectAppearance::getSelectionExtent() const
{
	return &s_selectionExtent;
}

//--------------------------------------------------------------------------
Extent const *ParticleEffectAppearance::getExtent() const
{
	return &m_extent;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::restart()
{
	// Reset the time

	m_age = 0.0f;

	// Send the restart message to all children

	ParticleEmitterGroups::iterator current = m_particleEmitterGroups->begin();

	for (; current != m_particleEmitterGroups->end(); ++current)
	{
		ParticleEmitterGroup *particleEmitterGroup = (*current);

		particleEmitterGroup->restart();
	}
}

//--------------------------------------------------------------------------
ParticleEmitterGroup const * const ParticleEffectAppearance::getEmitterGroup(int const index) const
{
	DEBUG_FATAL(index >= static_cast<int>(m_particleEmitterGroups->size()), ("The requested emitter group index is out of range (%d).", index));

	ParticleEmitterGroups::const_iterator current = m_particleEmitterGroups->begin();
	int currentIndex = 0;

	for (; current != m_particleEmitterGroups->end(); ++current, ++currentIndex)
	{
		if (currentIndex == index)
		{
			break;
		}
	}

	return *current;
}

//--------------------------------------------------------------------------
bool ParticleEffectAppearance::isDeletable() const
{
	bool result = !(isInfiniteLooping() && isEnabled());

	if (result)
	{
		ParticleEmitterGroups::const_iterator current = m_particleEmitterGroups->begin();

		for (; current != m_particleEmitterGroups->end(); ++current)
		{
			ParticleEmitterGroup *particleEmitterGroup = (*current);

			bool const notDeletable = !particleEmitterGroup->isDeletable();

			if (notDeletable)
			{
				result = false;
				break;
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setEnabled(bool const enabled)
{
	m_enabled = enabled;
}

//--------------------------------------------------------------------------
int ParticleEffectAppearance::getParticleCount() const
{
	int result = 0;

	ParticleEmitterGroups::const_iterator current = m_particleEmitterGroups->begin();

	for (; current != m_particleEmitterGroups->end(); ++current)
	{
		ParticleEmitterGroup *particleEmitterGroup = (*current);

		result += particleEmitterGroup->getParticleCount();
	}

	return result;
}

//--------------------------------------------------------------------------
bool ParticleEffectAppearance::hasAliveParticles() const
{
	bool result = false;
	ParticleEmitterGroups::const_iterator iterParticleEmitterGroups = m_particleEmitterGroups->begin();

	for (; iterParticleEmitterGroups != m_particleEmitterGroups->end(); ++iterParticleEmitterGroups)
	{
		ParticleEmitterGroup const & particleEmitterGroup = *NON_NULL(*iterParticleEmitterGroups);

		if (particleEmitterGroup.hasAliveParticles())
		{
			result = true;
			break;
		}
	}

	return result;
}

//--------------------------------------------------------------------------
Sphere const &ParticleEffectAppearance::getSphere() const
{
	return m_extent.getSphere();
}

//--------------------------------------------------------------------------
bool ParticleEffectAppearance::isEnabled() const
{
	return m_enabled;
}

//--------------------------------------------------------------------------
bool ParticleEffectAppearance::isPaused() const
{
	return m_paused;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setPaused(bool const paused)
{
	m_paused = paused;
}

// If autoDelete is enabled, then the appearance can tell objects when they
// need to kill themselves
//--------------------------------------------------------------------------
bool ParticleEffectAppearance::realIsAlive() const
{
	bool result = true;

	if (m_autoDelete)
	{
		result = !isDeletable();
	}
	
	return result;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setAutoDelete(bool const autoDelete)
{
	m_autoDelete = autoDelete;
}

// This is set to false in the particle editor so the user can move the camera
// below the ground and Umbra won't cull it out.
//--------------------------------------------------------------------------
void ParticleEffectAppearance::setUnBounded(bool const unBounded)
{
	m_unBounded = unBounded;
	m_dpvsObject->set(DPVS::Object::UNBOUNDED, m_unBounded);
}

//--------------------------------------------------------------------------
float ParticleEffectAppearance::getScale_w() const
{
	return m_scale_w;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setPlayBackRate(float const playBackRate)
{
	DEBUG_FATAL((playBackRate < 0.0f), ("The particle effect appearance playback rate must be >= 0."));

	m_playBackRate = playBackRate;
}

//--------------------------------------------------------------------------
float ParticleEffectAppearance::getPlayBackRate() const
{
	return m_playBackRate;
}

//--------------------------------------------------------------------------
float ParticleEffectAppearance::getPlayBackRate_w() const
{
	float result = m_playBackRate;
	float const initialPlayBackRate = m_particleEffectAppearanceTemplate->m_particleEffectDescription->getInitialPlayBackRate();
	float const initialPlayBackRateTime = m_particleEffectAppearanceTemplate->m_particleEffectDescription->getInitialPlayBackRateTime();
	float const playBackRate = m_particleEffectAppearanceTemplate->m_particleEffectDescription->getPlayBackRate();

	if (m_age < initialPlayBackRateTime)
	{
		result *= initialPlayBackRate;
	}
	else
	{
		result *= playBackRate;
	}

	if (m_parentParticleEffectAppearance != NULL)
	{
		// Chain up adding the playback rates

		result *= m_parentParticleEffectAppearance->getPlayBackRate_w();
	}

	return result;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setScale(Vector const & scale)
{
	DEBUG_FATAL(!WithinEpsilonInclusive(scale.x, scale.y, FLT_MIN) || !WithinEpsilonInclusive(scale.x, scale.z, FLT_MIN), ("ParticleEffectAppearance::setScale called with a non-uniform scale."));
	DEBUG_FATAL((scale.x <= 0.0f), ("The scale(%f) must be > 0", scale.x));

	Appearance::setScale(Vector(scale.x, scale.x, scale.x));

	m_scale_w = scale.x * m_particleEffectAppearanceTemplate->m_particleEffectDescription->getScale();

	if (m_parentParticleEffectAppearance != NULL)
	{
		m_scale_w *= m_parentParticleEffectAppearance->getScale_w();
	}
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setParentParticleEffectAppearance(ParticleEffectAppearance const & parentParticleEffectAppearance)
{
	m_parentParticleEffectAppearance = &parentParticleEffectAppearance;

	// Update the world space scale

	setScale(getScale());
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setGlobalWind(Vector const &wind)
{
	s_globalWind = wind;
}

//--------------------------------------------------------------------------
Vector const &ParticleEffectAppearance::getGlobalWind()
{
	return s_globalWind;
}

//--------------------------------------------------------------------------
ParticleEffectAppearance const *ParticleEffectAppearance::getParticleEffectAppearance() const
{
	return m_parentParticleEffectAppearance;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setOwner(Object *newOwner)
{
	Appearance::setOwner(newOwner);

	ParticleEmitterGroups::const_iterator iterParticleEmitterGroups = m_particleEmitterGroups->begin();

	for (; iterParticleEmitterGroups != m_particleEmitterGroups->end(); ++iterParticleEmitterGroups)
	{
		ParticleEmitterGroup *particleEmitterGroup = (*iterParticleEmitterGroups);

		particleEmitterGroup->setOwner(newOwner);
	}

#ifdef _DEBUG
	m_originIcon->setOwner(newOwner);
#endif
}

//--------------------------------------------------------------------------
#ifdef _DEBUG
void ParticleEffectAppearance::drawDebugExtents() const
{
	if (getOwner() != NULL)
	{
		Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorZStaticShader());
		Graphics::setObjectToWorldTransformAndScale(getOwner()->getTransform_o2w(), Vector::xyz111);
		Graphics::drawExtent(&m_extent, VectorArgb::solidWhite);
	}
}
#endif

//--------------------------------------------------------------------------
#ifdef _DEBUG
void ParticleEffectAppearance::drawOriginIcon() const
{
	if (getOwner() != NULL && m_parentParticleEffectAppearance == 0)
	{
		m_originIcon->render();
	}
}
#endif


// ----------------------------------------------------------------------

#ifdef _DEBUG
bool ParticleEffectAppearance::collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const
{
	if (collideParameters.getToolPickTest() && ParticleManager::isDebugOriginIconEnabled())
	{
		return m_originIcon->collide(start_o, end_o, collideParameters, result);
	}

	return false;
}

// ----------------------------------------------------------------------

bool ParticleEffectAppearance::implementsCollide() const
{
	return true;
}
#endif

//--------------------------------------------------------------------------
void ParticleEffectAppearance::addToWorld()
{
	Appearance::addToWorld();

	ParticleEmitterGroups::const_iterator iterParticleEmitterGroups = m_particleEmitterGroups->begin();

	for (; iterParticleEmitterGroups != m_particleEmitterGroups->end(); ++iterParticleEmitterGroups)
	{
		ParticleEmitterGroup *particleEmitterGroup = (*iterParticleEmitterGroups);

		particleEmitterGroup->addToWorld();
	}
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::removeFromWorld()
{
	Appearance::removeFromWorld();

	if (m_restartOnRemoveFromWorld && hasAliveParticles())
	{
		restart();
	}

	ParticleEmitterGroups::const_iterator iterParticleEmitterGroups = m_particleEmitterGroups->begin();

	for (; iterParticleEmitterGroups != m_particleEmitterGroups->end(); ++iterParticleEmitterGroups)
	{
		ParticleEmitterGroup *particleEmitterGroup = (*iterParticleEmitterGroups);

		particleEmitterGroup->removeFromWorld();
	}
}

//--------------------------------------------------------------------------
bool ParticleEffectAppearance::isInfiniteLooping() const
{
	return m_infiniteLooping;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setGlobalLodBias(float const bias)
{
	s_globalLodBias = clamp(0.0f, bias, 1.0f);
}

//--------------------------------------------------------------------------
float ParticleEffectAppearance::getGlobalLodBias()
{
	return s_globalLodBias;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setLodBias(float const bias)
{
	m_lodBias = clamp(0.0f, bias, 1.0f);
}

//--------------------------------------------------------------------------
float ParticleEffectAppearance::getLodBias() const
{
	return m_lodBias * s_globalLodBias;
}

//--------------------------------------------------------------------------
VectorArgb const &ParticleEffectAppearance::getColorModifier() const
{
	return m_colorModifier;
}

//--------------------------------------------------------------------------
VectorArgb const &ParticleEffectAppearance::getGlobalColorModifier()
{
	return s_globalColorModifier;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setColorModifier(VectorArgb const &color)
{
	m_colorModifier = color;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setGlobalColorModifier(VectorArgb const &color)
{
	s_globalColorModifier = color;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setGlobalWindEnabled(bool const enabled)
{
	s_globalWindEnabled = enabled;
}

//--------------------------------------------------------------------------
bool ParticleEffectAppearance::isGlobalWindEnabled()
{
	return s_globalWindEnabled;
}

//--------------------------------------------------------------------------
void ParticleEffectAppearance::setRestartOnRemoveFromWorld(bool const restartOnRemoveFromWorld)
{
	m_restartOnRemoveFromWorld = restartOnRemoveFromWorld;
}

//--------------------------------------------------------------------------
ParticleEffectAppearance * ParticleEffectAppearance::asParticleEffectAppearance()
{
	return this;
}

//--------------------------------------------------------------------------
ParticleEffectAppearance const * ParticleEffectAppearance::asParticleEffectAppearance() const
{
	return this;
}

//--------------------------------------------------------------------------
ParticleEffectAppearance * ParticleEffectAppearance::asParticleEffectAppearance(Appearance * appearance)
{
	return (appearance != NULL) ? appearance->asParticleEffectAppearance() : NULL;
}

//--------------------------------------------------------------------------
ParticleEffectAppearance const * ParticleEffectAppearance::asParticleEffectAppearance(Appearance const * appearance)
{
	return (appearance != NULL) ? appearance->asParticleEffectAppearance() : NULL;
}

void ParticleEffectAppearance::setGlobalUserLimit(int const limit)
{
	s_globalUserLimit = limit;
}

int const ParticleEffectAppearance::getGlobalUserLimit()
{
	return s_globalUserLimit;
}

// ============================================================================
