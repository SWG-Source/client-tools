// ==================================================================
//
// SkeletalAppearance2.cpp
// copyright 2001, 2002 Sony Online Entertainment
// 
// ==================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"

#include "clientAnimation/PlaybackScriptManager.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/LodDistanceTable.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientSkeletalAnimation/AnimationEnvironment.h"
#include "clientSkeletalAnimation/AnimationEnvironmentNames.h"
#include "clientSkeletalAnimation/BasicMeshGeneratorTemplate.h"
#include "clientSkeletalAnimation/BasicSkeletonTemplate.h"
#include "clientSkeletalAnimation/CharacterLodManager.h"
#include "clientSkeletalAnimation/CompositeMesh.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/FullGeometrySkeletalAppearanceBatchRenderer.h"
#include "clientSkeletalAnimation/MeshGenerator.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientSkeletalAnimation/SkeletonTemplate.h"
#include "clientSkeletalAnimation/SkeletonTemplateList.h"
#include "clientSkeletalAnimation/SoftwareBlendSkeletalShaderPrimitive.h"
#include "clientSkeletalAnimation/TargetPitchTransformModifier.h"
#include "clientSkeletalAnimation/TransformAnimationController.h"
#include "clientSkeletalAnimation/TransformAnimationResolver.h"
#include "clientTextureRenderer/TextureRenderer.h"
#include "clientTextureRenderer/TextureRendererShaderPrimitive.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedDebug/ProfilerTimer.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/VoidBindSecond.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Os.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/Object.h"
#include "sharedRandom/Random.h"
#include "sharedUtility/LocalMachineOptionManager.h"

#include <algorithm>
#include <float.h>
#include <limits>
#include <map>
#include <set>
#include <string>

#include "dpvsModel.hpp"
#include "dpvsObject.hpp"

// ======================================================================

#define TRACK_APPEARANCE_BUILDING  0

// ======================================================================

namespace SkeletalAppearance2Namespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct LodInfo
	{
		float maxScreenFraction;
		float minScreenFraction;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	int const cs_maxLodCount = 4;

#if PRODUCTION == 0
	int const cs_skinningModeCount = 3;

	char const *const cs_skinningModeName[] =
		{
			"no skinning",
			"hard skinning",
			"soft skinning"
		};
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void reportAllocations();

	void fixBoxExtentMinMax(BoxExtent &boxExtent);

#if PRODUCTION == 0
	void printRenderStatistics();
	void reportRebuildMeshStatistics();
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	LodInfo  s_lodInfoArray[cs_maxLodCount];
	float    s_batchRendererStartNearFraction;
	float    s_batchRendererStartFarFraction;

	bool s_reportAllocations;
	bool s_disableBatcher = true;
	bool s_logReadyIssues;

#if PRODUCTION == 0

	bool s_disableSkeletalAppearanceRendering;
	bool s_disableCollisionChecking;
	bool s_disableSphereTreeUpdates;
	bool s_reportRenderStatistics;
	bool s_showSkeleton;

	// These store per-lod batched/unbatched counts.
	int  s_perLodRenderedShaderPrimitiveCount[2 * cs_maxLodCount];
	int  s_perLodRenderedCharacterCount[2 * cs_maxLodCount];

	// This stores per-character skinning mode counts.
	int  s_perSkinningModeCharacterCount[2 * cs_skinningModeCount];
	int  s_addToCameraSceneCount;

	bool                 s_reportRebuildMeshStatistics;
	int                  s_rebuildMeshCallCount;
	ProfilerTimer::Type  s_rebuildMeshCallTime;

#endif

	int  s_alterAllocationAmount;
	int  s_rebuildMeshAllocationAmount;
	int  s_shaderPrimitiveAllocationAmount;

	float  s_twoOverScreenLength;

	SkeletalAppearance2::ContainsDestroyedAttachmentWearableCallback  s_destroyedAttachmentWearableCallback;

	bool  s_maximumDesiredDetailLevelEnabled;
	int   s_maximumDesiredDetailLevelIndex;

	bool  s_uiContextEnabled;

	int const totalShaderColors = 3;
	int const qualityPerShader = 4;
	std::string const cs_holoShaderLibrary[SkeletalAppearance2::m_maxShaderSize] = {"shader/membrane_hologram_bluegreen01.sht",
															  "shader/membrane_hologram_bluegreen02.sht",
															  "shader/membrane_hologram_black_invis.sht",
															  "shader/membrane_hologram_purple01.sht",
															  "shader/membrane_hologram_purple02.sht",
															  "shader/membrane_hologram_black_invis.sht",
	                                                          "shader/membrane_hologram_orange01.sht",
	                                                          "shader/membrane_hologram_orange02.sht",
	                                                          "shader/membrane_hologram_black_invis.sht"};


	std::string const cs_blueGlowieMembraneShader("shader/membrane_glowie.sht");
	std::string const cs_blueGlowieMembraneBumpShader("shader/membrane_glowie.sht");
	std::string const cs_holonetShader("shader/holonet.sht");
	std::string const cs_holonetBumpShader("shader/holonet.sht");


	ShaderTemplate const *ms_currentOverrideShaderTemplate;	
	ShaderTemplate const *ms_currentOverrideBumpShaderTemplate;	
	typedef std::map<std::pair<ShaderTemplate const *, Texture const *>, StaticShader *> OverrideShaderMap;
	OverrideShaderMap ms_overrideShaderMap;

	StaticShader const & prepareToViewOverrideFunction(ShaderPrimitive const & shaderPrimitive)
	{
		Tag const TAG_NHOL = TAG(N,H,O,L);
		StaticShader const & shaderPrimitiveShader = shaderPrimitive.prepareToView();

		// //depot/swg/test/data/sku.0/sys.client/built/engine/effect/e_h_lightsaberblade.eft
		// //depot/swg/test/data/sku.0/sys.client/built/engine/effect/e_particle_alpha.eft
		// //depot/swg/test/data/sku.0/sys.client/built/engine/effect/e_particle_emisadd.eft
		// //depot/swg/test/data/sku.0/sys.client/built/engine/effect/e_particle_emisadd_2x.eft
		// //depot/swg/test/data/sku.0/sys.client/built/engine/effect/e_emisadd_masked_vcolor_bloom.eft
		// //  have these option set - nhol == no blue glowie/hologram - will cause artifacting if a swoosh goes through this
		if (ms_currentOverrideShaderTemplate && !shaderPrimitiveShader.hasOptionTag(TAG_NHOL))
		{
			//DEBUG_WARNING(true, ("shader template %s", shaderPrimitiveShader.getStaticShaderTemplate().getName().getString()));
			//-- Extract MAIN from the existing shader primitive and set it in the override shader
			Tag const TAG_MAIN = TAG(M,A,I,N);
			Tag const TAG_NRML = TAG(N,R,M,L);
			Texture const * mainTexture = 0;
			Texture const * normalTexture = 0;
			bool hasNormal = shaderPrimitiveShader.getTexture(TAG_NRML, normalTexture);
			ShaderTemplate const * useThisShader = hasNormal ? ms_currentOverrideBumpShaderTemplate : ms_currentOverrideShaderTemplate;
			if (shaderPrimitiveShader.getTexture(TAG_MAIN, mainTexture))
			{
				OverrideShaderMap::iterator iter = ms_overrideShaderMap.find(std::make_pair(ms_currentOverrideShaderTemplate, mainTexture));
				if (iter != ms_overrideShaderMap.end())
					return *iter->second;

				Shader * const shader = useThisShader->fetchModifiableShader();
				StaticShader * const staticShader = shader->getStaticShader();
				if (staticShader)
				{
					mainTexture->fetch();
					staticShader->setTexture(TAG_MAIN, *mainTexture);
					if(hasNormal)
					{
						normalTexture->fetch();
						staticShader->setTexture(TAG_NRML, *normalTexture);
					}
					IGNORE_RETURN(ms_overrideShaderMap.insert(std::make_pair(std::make_pair(ms_currentOverrideShaderTemplate, mainTexture), staticShader)));

					return staticShader->prepareToView();
				}
				else
					shader->release();
			}
		}

		return shaderPrimitiveShader;
	}
}

using namespace SkeletalAppearance2Namespace;

// ======================================================================

bool                              SkeletalAppearance2::ms_installed;
float                             SkeletalAppearance2::ms_lodBias;
CompositeMesh                    *SkeletalAppearance2::ms_compositeMesh;

#ifdef _DEBUG
bool                              SkeletalAppearance2::ms_renderTargetDirection;
#endif

// ======================================================================
// namespace SkeletalAppearance2Namespace
// ======================================================================

void SkeletalAppearance2Namespace::reportAllocations()
{
	DEBUG_REPORT_PRINT(true, ("SkelAppear:alter/rebuildMesh/shaderPrim mem alloc(KB):%d/%d/%d\n", s_alterAllocationAmount/1024, s_rebuildMeshAllocationAmount/1024, s_shaderPrimitiveAllocationAmount/1024));
}

// ----------------------------------------------------------------------

#if PRODUCTION == 0

void SkeletalAppearance2Namespace::reportRebuildMeshStatistics()
{
	//-- Get profiler frequency.
	ProfilerTimer::Type  timeUnused;
	ProfilerTimer::Type  ticksPerSecond;

	ProfilerTimer::getCalibratedTime(timeUnused, ticksPerSecond);
	float const callTimeInSeconds = (ticksPerSecond > 0) ? static_cast<float>(s_rebuildMeshCallTime)/static_cast<float>(ticksPerSecond) : 0.0f;

	REPORT_PRINT(true, ("rebuildMesh: [%d] rebuilds in [%g] seconds, os update frame [%d]\n", s_rebuildMeshCallCount, callTimeInSeconds, Os::getNumberOfUpdates()));

	if (s_rebuildMeshCallCount > 0)
		REPORT_LOG(true, ("rebuildMesh: [%d] rebuilds in [%g] seconds, os update frame [%d]\n", s_rebuildMeshCallCount, callTimeInSeconds, Os::getNumberOfUpdates()));

	//-- Reset statistics data for next frame.
	s_rebuildMeshCallCount = 0;
	s_rebuildMeshCallTime  = 0;
}

#endif

// ----------------------------------------------------------------------

void SkeletalAppearance2Namespace::fixBoxExtentMinMax(BoxExtent &boxExtent)
{
	Vector const originalMinVector = boxExtent.getMin();
	Vector const originalMaxVector = boxExtent.getMax();

	Vector const newMinVector(
		std::min(originalMinVector.x, originalMaxVector.x),
		std::min(originalMinVector.y, originalMaxVector.y),
		std::min(originalMinVector.z, originalMaxVector.z));

	Vector const newMaxVector(
		std::max(originalMinVector.x, originalMaxVector.x),
		std::max(originalMinVector.y, originalMaxVector.y),
		std::max(originalMinVector.z, originalMaxVector.z));

	boxExtent.setMin(newMinVector);
	boxExtent.setMax(newMaxVector);
}

// ----------------------------------------------------------------------

#if PRODUCTION == 0

void SkeletalAppearance2Namespace::printRenderStatistics()
{
	REPORT_PRINT(true, ("SA2:render() calls: %d\n", s_addToCameraSceneCount));

	REPORT_PRINT(true, ("SA2:Per-Character Counts (standard/batched/total)\n"));
	{
		for (int i = 0; i < cs_maxLodCount; ++i)
			REPORT_PRINT(true, ("l%d: %d/%d/%d\n", i, s_perLodRenderedCharacterCount[i], s_perLodRenderedCharacterCount[cs_maxLodCount + i], s_perLodRenderedCharacterCount[i] + s_perLodRenderedCharacterCount[cs_maxLodCount + i]));
	}

	REPORT_PRINT(true, ("\nSA2:Per-ShaderPrimitive Counts (standard/batched/total)\n"));
	{
		for (int i = 0; i < cs_maxLodCount; ++i)
			REPORT_PRINT(true, ("l%d: %d/%d/%d\n", i, s_perLodRenderedShaderPrimitiveCount[i], s_perLodRenderedShaderPrimitiveCount[cs_maxLodCount + i], s_perLodRenderedShaderPrimitiveCount[i] + s_perLodRenderedShaderPrimitiveCount[cs_maxLodCount + i]));
	}

	REPORT_PRINT(true, ("SA2:Per-Skinning Mode Character Counts (standard/batched/total)\n"));
	{
		for (int i = 0; i < cs_skinningModeCount; ++i)
			REPORT_PRINT(true, ("%s: %d/%d/%d\n", cs_skinningModeName[i], s_perSkinningModeCharacterCount[i], s_perSkinningModeCharacterCount[cs_skinningModeCount + i], s_perSkinningModeCharacterCount[i] + s_perSkinningModeCharacterCount[cs_skinningModeCount + i]));
	}

	//-- Clear counters.
	s_addToCameraSceneCount = 0;

	Zero(s_perLodRenderedShaderPrimitiveCount);
	Zero(s_perLodRenderedCharacterCount);
	Zero(s_perSkinningModeCharacterCount);
}

#endif

// ======================================================================
// class SkeletalAppearance2::AttachedAppearance
// ======================================================================

class SkeletalAppearance2::AttachedAppearance
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class FindByObject
	{
	public:

		explicit FindByObject(const Object *targetObject);

		bool operator()(const AttachedAppearance *appearance) const;

	private:

		const Object *m_targetObject;

	private:
		// disabled
		FindByObject();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	static void install();
	static void remove();

	static void *operator new(size_t size);
	static void  operator delete(void *data, size_t size);

public:

	AttachedAppearance(Object *object, const CrcString &transformName, int detailCount);

	bool                  lookupTransformIndex(int detailIndex, const Skeleton &skeleton);

	Object               *getObject();
	const Object         *getObject() const;
	Appearance           *getAppearance();
	const Appearance     *getAppearance() const;
	const CrcString      &getTransformName() const;
	int                   getTransformIndex(int detailIndex) const;

	bool                  isHidden() const;
	void                  setHidden(bool hidden);

private:

	typedef stdvector<int>::fwd  IntVector;

private:

	static MemoryBlockManager *ms_memoryBlockManager;

private:

	Watcher<Object>      m_object;
	PersistentCrcString  m_transformName;
	IntVector            m_perLodTransformIndex;
	bool                 m_hidden;

private:
	// disabled
	AttachedAppearance();
	AttachedAppearance(AttachedAppearance const & rhs);
	AttachedAppearance & operator=(AttachedAppearance const & rhs);
};

// ======================================================================
// class SkeletalAppearance2::AttachedTransformModifier
// ======================================================================

class SkeletalAppearance2::AttachedTransformModifier
{
public:

	AttachedTransformModifier(CrcString const &transformName, TransformModifier *transformModifier);
	~AttachedTransformModifier();

	CrcString const         &getTransformName() const;

	TransformModifier       *getTransformModifier();
	TransformModifier const *getTransformModifier() const;

private:

	// Disabled.
	AttachedTransformModifier();
	AttachedTransformModifier(AttachedTransformModifier const & rhs);
	AttachedTransformModifier & operator=(AttachedTransformModifier const & rhs);

private:

	PersistentCrcString  m_transformName;
	TransformModifier   *m_transformModifier;

};

// ======================================================================

class SkeletalAppearance2::SkeletonSegmentDescriptor
{
public:

	static void validate(SkeletonSegmentDescriptorVector const &descriptorVector, SkeletalAppearance2 const &appearance);

public:

	SkeletonSegmentDescriptor(const CrcLowerString &skeletonTemplateName, const CrcLowerString &attachmentTransformName);
	SkeletonSegmentDescriptor();

	const CrcLowerString &getSkeletonTemplateName() const;
	const CrcLowerString &getAttachmentTransformName() const;

	bool operator <(const SkeletonSegmentDescriptor &rhs) const;

private:

	const CrcLowerString *m_skeletonTemplateName;
	const CrcLowerString *m_attachmentTransformName;
};

// ======================================================================

MemoryBlockManager *SkeletalAppearance2::AttachedAppearance::ms_memoryBlockManager;

// ======================================================================
// class SkeletalAppearance2::AttachedAppearance::FindByObject
// ======================================================================

inline SkeletalAppearance2::AttachedAppearance::FindByObject::FindByObject(const Object *targetObject) :
	m_targetObject(targetObject)
{
}

// ----------------------------------------------------------------------
//lint -esym(1714, FindByObject::operator*) // not referenced // false

inline bool SkeletalAppearance2::AttachedAppearance::FindByObject::operator()(const AttachedAppearance *attachedObject) const
{
	return (attachedObject->getObject() == m_targetObject);
}

// ======================================================================
// class SkeletalAppearance2::AttachedAppearance
// ======================================================================

void SkeletalAppearance2::AttachedAppearance::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("already installed"));

	ms_memoryBlockManager = new MemoryBlockManager("SkeletalAppearance2::AttachedAppearance", true, sizeof(AttachedAppearance), 0, 0, 0);
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::AttachedAppearance::remove()
{
	DEBUG_FATAL(!ms_memoryBlockManager, ("not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *SkeletalAppearance2::AttachedAppearance::operator new(size_t size)
{
	DEBUG_FATAL(size != sizeof(AttachedAppearance), ("derivation from AttachedAppearance not supported"));
	UNREF(size);

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::AttachedAppearance::operator delete(void *data, size_t size)
{
	DEBUG_FATAL(size != sizeof(AttachedAppearance), ("derivation from AttachedAppearance not supported"));
	UNREF(size);

	ms_memoryBlockManager->free(data);	
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::setContainsDestroyedAttachmentWearableCallback(ContainsDestroyedAttachmentWearableCallback callback)
{
	s_destroyedAttachmentWearableCallback = callback;
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::getMaximumDesiredDetailLevel(bool &enabled, int &lodIndex)
{
	enabled  = s_maximumDesiredDetailLevelEnabled;
	lodIndex = s_maximumDesiredDetailLevelIndex;
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::setMaximumDesiredDetailLevel(bool enabled, int lodIndex)
{
	s_maximumDesiredDetailLevelEnabled = enabled;
	s_maximumDesiredDetailLevelIndex   = lodIndex;
}

// ======================================================================

SkeletalAppearance2::AttachedAppearance::AttachedAppearance(Object *object, const CrcString &transformName, int detailCount) :
	m_object(object),
	m_transformName(transformName),
	m_perLodTransformIndex(static_cast<size_t>(detailCount), -1),
	m_hidden(false)
{
}

// ----------------------------------------------------------------------

bool SkeletalAppearance2::AttachedAppearance::lookupTransformIndex(int detailIndex, const Skeleton &skeleton)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, detailIndex, static_cast<int>(m_perLodTransformIndex.size()));
	int &transformIndex = m_perLodTransformIndex[static_cast<size_t>(detailIndex)];

	bool foundIt   = 0;
	transformIndex = 0;

	skeleton.findTransformIndex(m_transformName, &transformIndex, &foundIt);
	if (!foundIt)
	{
		// we will place at root transform if it is not found for this detail level.
		transformIndex = 0;
	}

	return foundIt;
}

// ----------------------------------------------------------------------

inline Object *SkeletalAppearance2::AttachedAppearance::getObject()
{
	return m_object;
} //lint !e1762 // function could be made const // I consider this a logically non-const operation.

// ----------------------------------------------------------------------

inline const Object *SkeletalAppearance2::AttachedAppearance::getObject() const
{
	return m_object;
}

// ----------------------------------------------------------------------

Appearance *SkeletalAppearance2::AttachedAppearance::getAppearance()
{
	Object *const object = getObject();

	if (object)
		return object->getAppearance();
	else
		return 0;
}

// ----------------------------------------------------------------------

const Appearance *SkeletalAppearance2::AttachedAppearance::getAppearance() const
{
	const Object *const object = getObject();

	if (object)
		return object->getAppearance();
	else
		return 0;
}

// ----------------------------------------------------------------------

inline const CrcString &SkeletalAppearance2::AttachedAppearance::getTransformName() const
{
	return m_transformName;
}

// ----------------------------------------------------------------------

inline int SkeletalAppearance2::AttachedAppearance::getTransformIndex(int detailIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, detailIndex, static_cast<int>(m_perLodTransformIndex.size()));
	return m_perLodTransformIndex[static_cast<size_t>(detailIndex)];
}

// ----------------------------------------------------------------------

inline bool SkeletalAppearance2::AttachedAppearance::isHidden() const
{
	return m_hidden;
}

// ----------------------------------------------------------------------

inline void SkeletalAppearance2::AttachedAppearance::setHidden(bool hidden)
{
	m_hidden = hidden;
}

// ======================================================================
// class SkeletalAppearance2::AttachedTransformModifier
// ======================================================================

SkeletalAppearance2::AttachedTransformModifier::AttachedTransformModifier(CrcString const &transformName, TransformModifier *transformModifier) :
	m_transformName(transformName),
	m_transformModifier(transformModifier)
{
	NOT_NULL(m_transformModifier);
}

// ----------------------------------------------------------------------

SkeletalAppearance2::AttachedTransformModifier::~AttachedTransformModifier()
{
	delete m_transformModifier;
}

// ----------------------------------------------------------------------

inline CrcString const &SkeletalAppearance2::AttachedTransformModifier::getTransformName() const
{
	return m_transformName;
}

// ----------------------------------------------------------------------

inline TransformModifier *SkeletalAppearance2::AttachedTransformModifier::getTransformModifier()
{
	return m_transformModifier;
}

// ----------------------------------------------------------------------

inline TransformModifier const *SkeletalAppearance2::AttachedTransformModifier::getTransformModifier() const
{
	return m_transformModifier;
}

// ======================================================================
// class SkeletalAppearance2::SkeletonSegmentDescriptor
// ======================================================================
/**
 * Ensure the segment descriptor vector is valid, FATAL if not.
 *
 * This function checks for multiple instances of the same skeleton segment
 * appearing in the vector list.  If the same skeleton segment appears multiple
 * times, that indicates that the logic that collects the set of required skeleton 
 * segments is broken.
 *
 * It is conceivable that we would want the same skeleton segment to attach
 * to a character in multiple attachment points.  We don't do that now, but
 * if/when we do, we'll need to update this function to consider the skeleton
 * template name and the attachment point as a unit instead of only looking
 * at the skeleton template.
 */

void SkeletalAppearance2::SkeletonSegmentDescriptor::validate(SkeletonSegmentDescriptorVector const &descriptorVector, SkeletalAppearance2 const &appearance)
{
	typedef std::set<TemporaryCrcString>  NameSet;
	NameSet  nameSet;

	SkeletonSegmentDescriptorVector::const_iterator const endIt = descriptorVector.end();
	for (SkeletonSegmentDescriptorVector::const_iterator it = descriptorVector.begin(); it != endIt; ++it)
	{
		TemporaryCrcString const skeletonTemplateName(it->getSkeletonTemplateName().getString(), true);
		if (nameSet.find(skeletonTemplateName) == nameSet.end())
			IGNORE_RETURN(nameSet.insert(skeletonTemplateName));
		else
		{
			//-- This skeleton template name appears multiple times in the descriptor vector.
			Object const *const object = appearance.getOwner();

			FATAL(true,
				("SkeletonTemplate [%s] was listed to be added multiple times to appearance from template [%s] (object id=[%s], object template=[%s]).",
				skeletonTemplateName.getString(),
				appearance.getAppearanceTemplateName(),
				object ? object->getNetworkId().getValueString().c_str() : "<null object>",
				object ? object->getObjectTemplateName() : "<null object>"
				));
		}
	} 
}

// ======================================================================

inline SkeletalAppearance2::SkeletonSegmentDescriptor::SkeletonSegmentDescriptor(const CrcLowerString &skeletonTemplateName, const CrcLowerString &attachmentTransformName)
:	m_skeletonTemplateName(&skeletonTemplateName),
	m_attachmentTransformName(&attachmentTransformName)
{
}

// ----------------------------------------------------------------------

inline SkeletalAppearance2::SkeletonSegmentDescriptor::SkeletonSegmentDescriptor()
:	m_skeletonTemplateName(0),
	m_attachmentTransformName(0)
{
}

// ----------------------------------------------------------------------

inline const CrcLowerString &SkeletalAppearance2::SkeletonSegmentDescriptor::getSkeletonTemplateName() const
{
	NOT_NULL(m_skeletonTemplateName);
	return *m_skeletonTemplateName;
}

// ----------------------------------------------------------------------

inline const CrcLowerString &SkeletalAppearance2::SkeletonSegmentDescriptor::getAttachmentTransformName() const
{
	NOT_NULL(m_attachmentTransformName);
	return *m_attachmentTransformName;
}

// ----------------------------------------------------------------------

//lint -esym(1714, SkeletonSegmentDescriptor::operator<) // not referenced // not true
inline bool SkeletalAppearance2::SkeletonSegmentDescriptor::operator <(const SkeletonSegmentDescriptor &rhs) const
{
	NOT_NULL(m_skeletonTemplateName);
	NOT_NULL(m_attachmentTransformName);

	if (*m_skeletonTemplateName < *rhs.m_skeletonTemplateName)
		return true;
	else if (*m_skeletonTemplateName > *rhs.m_skeletonTemplateName)
		return false;
	else
		return *m_attachmentTransformName < *rhs.m_attachmentTransformName;
}

// ======================================================================
// SkeletalAppearance2: inlines
// ======================================================================

/**
 * Retrieve the const vector of ShaderPrimitive instances associated with the
 * current display detail level.
 *
 * @return  the const vector of ShaderPrimitive instances associated with the
 *          current display detail level.
 */

const SkeletalAppearance2::ShaderPrimitiveVector &SkeletalAppearance2::getDisplayLodShaderPrimitives() const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_displayLodIndex, static_cast<int>(m_perLodShaderPrimitives->size()));
	return (*m_perLodShaderPrimitives)[static_cast<size_t>(m_displayLodIndex)];
}

// ----------------------------------------------------------------------
/**
 * Retrieve the vector of ShaderPrimitive instances associated with the
 * current display detail level.
 *
 * @return  the vector of ShaderPrimitive instances associated with the
 *          current display detail level.
 */

SkeletalAppearance2::ShaderPrimitiveVector &SkeletalAppearance2::getDisplayLodShaderPrimitives()
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_displayLodIndex, static_cast<int>(m_perLodShaderPrimitives->size()));
	return (*m_perLodShaderPrimitives)[static_cast<size_t>(m_displayLodIndex)];
}

// ======================================================================
// SkeletalAppearance2: inlines
// ======================================================================

inline const SkeletalAppearanceTemplate &SkeletalAppearance2::getSkeletalAppearanceTemplate() const
{
	return *NON_NULL(safe_cast<const SkeletalAppearanceTemplate*>(getAppearanceTemplate()));
}

// ----------------------------------------------------------------------

inline CompositeMesh &SkeletalAppearance2::getCompositeMesh()
{
	// If mesh building ever becomes multithreaded, this should return a per-thread version of this variable.

	NOT_NULL(ms_compositeMesh);
	return *ms_compositeMesh;
}

// ======================================================================
// SkeletalAppearance2: public static member functions
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(SkeletalAppearance2, true, 0, 0, 0);

void SkeletalAppearance2::install()
{
	DEBUG_FATAL(ms_installed, ("already installed"));

	ms_lodBias       = 1.0f;

	installMemoryBlockManager();
	AttachedAppearance::install();

#if PRODUCTION == 0
	//-- Register debug flags.
	DebugFlags::registerFlag(s_disableSkeletalAppearanceRendering, "ClientSkeletalAnimation/Character", "disableRendering");
	DebugFlags::registerFlag(s_reportRenderStatistics, "ClientSkeletalAnimation/Character", "reportRenderStatistics", printRenderStatistics);
#ifdef _DEBUG
	DebugFlags::registerFlag(ms_renderTargetDirection, "ClientSkeletalAnimation/Character", "renderTargetDirection");
#endif
	DebugFlags::registerFlag(s_logReadyIssues, "ClientSkeletalAnimation/Character", "logReadyIssues");
	DebugFlags::registerFlag(s_showSkeleton, "ClientSkeletalAnimation/Character", "showSkeleton");
	DebugFlags::registerFlag(s_disableBatcher, "ClientSkeletalAnimation/Character", "disableBatcher");
	DebugFlags::registerFlag(s_disableCollisionChecking, "ClientSkeletalAnimation/Character", "disableCollisionChecking");
	DebugFlags::registerFlag(s_disableSphereTreeUpdates, "ClientSkeletalAnimation/Character", "disableSphereTreeUpdates");
	DebugFlags::registerFlag(s_reportAllocations, "ClientSkeletalAnimation/Character", "reportSa2Allocations", reportAllocations);
	DebugFlags::registerFlag(s_reportRebuildMeshStatistics, "ClientSkeletalAnimation/Character", "reportRebuildMeshStatistics", reportRebuildMeshStatistics);
#endif

	LocalMachineOptionManager::registerOption (ms_lodBias, "ClientSkeletalAnimation", "lodBias");
	LocalMachineOptionManager::registerOption (s_disableBatcher, "ClientSkeletalAnimation", "disableBatcher");

	//-- Figure out pixel-to-screen-fraction multiplier.
	// @todo get this logic hooked up to a callback for window size changes.
	s_twoOverScreenLength = 2.0f / std::max(1.0f, static_cast<float>(std::min(Graphics::getCurrentRenderTargetWidth(), Graphics::getCurrentRenderTargetHeight())));

	ms_compositeMesh = new CompositeMesh();

	//-- Setup LOD switching screen fraction array.

	// Initialize.
	s_lodInfoArray[0].maxScreenFraction = std::numeric_limits<float>::max();
	s_lodInfoArray[0].minScreenFraction = ConfigClientSkeletalAnimation::getLod1ScreenFraction();

	s_lodInfoArray[1].maxScreenFraction = s_lodInfoArray[0].minScreenFraction;
	s_lodInfoArray[1].minScreenFraction = ConfigClientSkeletalAnimation::getLod2ScreenFraction();
	
	s_lodInfoArray[2].maxScreenFraction = s_lodInfoArray[1].minScreenFraction;
	s_lodInfoArray[2].minScreenFraction = ConfigClientSkeletalAnimation::getLod3ScreenFraction();

	s_lodInfoArray[3].maxScreenFraction = s_lodInfoArray[2].minScreenFraction;
	s_lodInfoArray[3].minScreenFraction = -std::numeric_limits<float>::max();

	float lodWidth[cs_maxLodCount];

	{
		for (int lodIndex = 1; lodIndex < cs_maxLodCount; ++lodIndex)
			lodWidth[lodIndex] = s_lodInfoArray[lodIndex].maxScreenFraction - s_lodInfoArray[lodIndex].minScreenFraction;

		// Adjust first lod's width so we add the right thing to extend lod 1's max.  Failure to do this means lod 1 ends up being good enough for large stuff so that we rarely see lod 0.
		lodWidth[0] = 1.0f - ConfigClientSkeletalAnimation::getLod1ScreenFraction();
	}

	// Adjust min and max.
	{
		float const overlapFraction = 0.15f;

		for (int lodIndex = 3; lodIndex >= 0; --lodIndex)
		{
			// Increase max screen fraction for this lod.
			if (lodIndex > 0)
			{
				float const increaseAmount = overlapFraction * lodWidth[lodIndex - 1];
				s_lodInfoArray[lodIndex].maxScreenFraction += increaseAmount;
			}

			// Decrease min screen fraction for this lod.
			if (lodIndex < cs_maxLodCount - 1)
			{
				float const decreaseAmount = overlapFraction * lodWidth[lodIndex + 1];
				s_lodInfoArray[lodIndex].minScreenFraction -= decreaseAmount;
			}
		}
	}

	//-- Setup batch renderer's staring distance (near and far for temporal coherence).
	setBatchRenderScreenFraction(!s_disableBatcher, ConfigClientSkeletalAnimation::getBatchRenderScreenFraction());

	ms_installed = true;
	ExitChain::add(remove, "SkeletalAppearance2");
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::setDetailLevelBias(float lodBias)
{
	ms_lodBias = lodBias;
}

// ----------------------------------------------------------------------

float SkeletalAppearance2::getDetailLevelBias()
{
	return ms_lodBias;
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::setShowSkeleton(bool showSkeleton)
{
#if PRODUCTION == 0
	s_showSkeleton = showSkeleton;
#else
	UNREF(showSkeleton);
#endif
}

// ----------------------------------------------------------------------

bool SkeletalAppearance2::getShowSkeleton()
{
#if PRODUCTION == 0
	return s_showSkeleton;
#else
	return false;
#endif
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::getBatchRenderScreenFraction(bool &enabled, float &screenFraction)
{
	enabled        = !s_disableBatcher;
	screenFraction =  s_batchRendererStartFarFraction;
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::setBatchRenderScreenFraction(bool enabled,  float screenFraction)
{
	s_disableBatcher                 = !enabled;
	s_batchRendererStartNearFraction = screenFraction;
	s_batchRendererStartFarFraction  = s_batchRendererStartNearFraction;

	if (s_batchRendererStartNearFraction > 0.0f)
	{
		float const bandWidth = 0.2f * s_batchRendererStartNearFraction;
		s_batchRendererStartNearFraction += bandWidth;
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::setUiContextEnabled(bool enabled)
{
	s_uiContextEnabled = enabled;
}

// ----------------------------------------------------------------------

bool SkeletalAppearance2::getUiContextEnabled()
{
	return s_uiContextEnabled;
}

// ======================================================================
// SkeletalAppearance2: public member functions
// ======================================================================

SkeletalAppearance2::SkeletalAppearance2(const SkeletalAppearanceTemplate *newAppearanceTemplate)
:	Appearance(newAppearanceTemplate),
	m_animationEnvironment(new AnimationEnvironment()),
	m_skeletons(new SkeletonVector()),
	m_ownedMeshGenerators(new MeshGeneratorVectorVector()),
	m_customizationData(0),
	m_showMesh(true),
	m_showAttachments(true),
	m_animationResolver(0),
	m_overridingAnimationResolver(0),
	m_perLodShaderPrimitives(0),
	m_perLodMruFrameVector(),
	m_attachedAppearances(0),
	m_wornAppearanceObjects(0),
	m_wearablesLockedFrame(0),
	m_appearanceDirty(),
	m_perLodMeshExtentMinDelta(),
	m_perLodMeshExtentMaxDelta(),
	m_meshExtent(0),
	m_showMeshExtent(false),
	m_displayLodIndex(0),
	m_lodIsReady(false),
	m_userControlledDetailLevel(false),
	m_maxAvailableDetailLevelIndex(0),
	m_animationLocomotionVelocity(m_animationEnvironment->getVector(AnimationEnvironmentNames::cms_locomotionVelocity)),
	m_directionToTarget(m_animationEnvironment->getVector(AnimationEnvironmentNames::cms_directionToTarget)),
	m_targetObject(),
	m_targetPositionValid (false),
	m_targetPosition_w (),
	m_targetPosition_o (),
	m_extent(),
	m_scale(m_animationEnvironment->getFloat(AnimationEnvironmentNames::cms_appearanceScale)),
	m_yawDirection(m_animationEnvironment->getInt(AnimationEnvironmentNames::cms_yawDirection)),
	m_dpvsObject(0),
	m_attachedTransformModifiers(0),
	m_fadeState(FS_uninitialized),
	m_fadeFraction(0.0f),
	m_fadeHold(0.0f),
	m_mostRecentRenderUsedBatch(false),
	m_plannedLodIndex(0),
	m_plannedLodSetFrameNumber(-100),
	m_everyOtherFrameSkinningEnabled(false),
	m_forceHardSkinningEnabled(false),
	m_extentDelegateObject(0),
	m_extentDelegateTransformed(),
	m_unloadUnusedResourcesTimer(Random::randomReal(1.f, 2.f)),
	m_targetHardpoint(),
	m_isBlueGlowie(false),
	m_isHolonet(false),
    m_hologramType(SkeletalAppearance2::HT_none),
	m_blackHologramFrame(0),
	m_blueGlowieShaderTemplate(0),
	m_blueGlowieBumpShaderTemplate(0),
	m_holonetShaderTemplate(0),
	m_holonetBumpShaderTemplate(0)
	
{

	DEBUG_FATAL(!ms_installed, ("SkeletalAppearance2 not installed"));

	MeshGeneratorTemplate const *meshGeneratorTemplateLodHack = 0;

	//-- Initialize scale.
	m_scale = 1.0f;

	//-- Initialize yaw direction.
	m_yawDirection = 0;

	m_animationEnvironment->setSkeletalAppearance(*this);
	m_animationResolver = new TransformAnimationResolver(*NON_NULL(newAppearanceTemplate), *this);

	//-- Create all Skeleton LODs from template.
	newAppearanceTemplate->createSkeletonLods(*m_skeletons, *m_animationResolver);
	// DEBUG_FATAL(m_skeletons->empty() || !m_skeletons->front(), ("No Skeleton LOD instances were generated for [%s].", newAppearanceTemplate->getName()));

	const SkeletonVector::size_type  skeletonLodCount = m_skeletons->size();

	//-- Create owned MeshGenerator instances from template.
	NOT_NULL(newAppearanceTemplate);
	int meshGeneratorDetailLevelCount = 0;
	
	if (newAppearanceTemplate->getMeshGeneratorCount() > 0)
	{
		if (newAppearanceTemplate->getSkeletonTemplateCount() > 0)
		{
			// Common case.
			meshGeneratorDetailLevelCount = static_cast<int>(skeletonLodCount);
		}
		else
		{
			// Needed when making these on the fly with no skeletons, used for client-baked wearables.  
			if (newAppearanceTemplate->getMeshGeneratorCount() > 0)
			{
				// Grab the LOD count from the first mesh generator.  Technically this could be wrong --- we really want the
				// max LOD count for all mesh generators associated with this .sat file.
				// @todo fix this.  @todo  better: fix the hack.
				meshGeneratorTemplateLodHack = MeshGeneratorTemplateList::fetch(newAppearanceTemplate->getMeshGeneratorName(0));
				if (meshGeneratorTemplateLodHack)
					meshGeneratorDetailLevelCount = meshGeneratorTemplateLodHack->getDetailCount();

				// I release the reference at the end of the constructor so I don't open, close then reopen the same MGN again later
				// on in the constructor.  Not a reference leak.
			}
		}
	}

	m_ownedMeshGenerators->resize(static_cast<MeshGeneratorVectorVector::size_type>(meshGeneratorDetailLevelCount));

	//-- Determine the max LOD index I'll use.  The current strategy is to use
	//   as many LODs as there are skeleton LODs.
	// @todo -TRF- REMOVE THIS VARIABLE --- it is the source of a number of client-baked wearable bugs.  Use a separate value for skeleton and mesh generators.
	m_maxAvailableDetailLevelIndex = static_cast<int>(skeletonLodCount) - 1;

	//-- Create the per-lod ShaderPrimitive vectors.
	m_perLodShaderPrimitives = new ShaderPrimitiveVectorVector(static_cast<ShaderPrimitiveVectorVector::size_type>(m_maxAvailableDetailLevelIndex + 1));

	//-- Initialize per-lod MRU frame number with current frame number.
	m_perLodMruFrameVector.resize(static_cast<IntVector::size_type>(m_maxAvailableDetailLevelIndex + 1));
	{
		int const currentFrameNumber    = Os::getNumberOfUpdates();
		IntVector::iterator const endIt = m_perLodMruFrameVector.end();
		for (IntVector::iterator it = m_perLodMruFrameVector.begin(); it != endIt; ++it)
			*it = currentFrameNumber;
	}

	//-- Set initial LOD index to the most distance LOD supported.  This should prevent
	//   the game from initially needing all the highest-resolution data.
	m_displayLodIndex = m_maxAvailableDetailLevelIndex;
	m_plannedLodIndex = m_maxAvailableDetailLevelIndex;

	//-- Create the m_appearanceDirty vector, initially set each LOD's flag to true.
	m_appearanceDirty.resize(static_cast<BoolVector::size_type>(m_maxAvailableDetailLevelIndex + 1));
	{
		BoolVector::iterator const endIt = m_appearanceDirty.end();
		for (BoolVector::iterator it = m_appearanceDirty.begin(); it != endIt; ++it)
			*it = true; //lint !e1058 // initializing a non-const reference with non-lvalue // this is the right way to do this.
	}

	m_perLodMeshExtentMinDelta.resize(static_cast<VectorVector::size_type>(m_maxAvailableDetailLevelIndex + 1));
	m_perLodMeshExtentMaxDelta.resize(static_cast<VectorVector::size_type>(m_maxAvailableDetailLevelIndex + 1));

	//-- Initialize extent to something large so that we try to render if camera is anywhere close to it.
	//   Otherwise we may not catch that a large creature is visible --- extents only update when baked or rendered,
	//   which only happens when extents report that it is visible.
	m_extent.setMin(Vector(-5.0f, 0.0f, -5.0f));
	m_extent.setMax(Vector(5.0f, 10.0f, 5.0f));
	m_extent.calculateCenterAndRadius();

	//-- Initialize DPVS data.
	{
		DPVS::Model *const testModel = RenderWorld::fetchDefaultModel();
		m_dpvsObject = RenderWorld::createObject(this, testModel);
		IGNORE_RETURN(testModel->release());
	}

	updateDpvsTestObjectWithExtents();

	//-- Cleanup mgn LOD hack reference.
	if (meshGeneratorTemplateLodHack)
	{
		meshGeneratorTemplateLodHack->release();
		meshGeneratorTemplateLodHack = 0;			
	}

	for(int i = 0; i < m_maxShaderSize; ++i)
	{
		m_holoShaderTemplate[i] = 0;
	}
}

// ------------------------------------------------------------------

SkeletalAppearance2::~SkeletalAppearance2(void)
{
	if(m_blueGlowieShaderTemplate)
	{
		m_blueGlowieShaderTemplate->release();
		m_blueGlowieShaderTemplate = 0;
	}
	if(m_blueGlowieBumpShaderTemplate)
	{
		m_blueGlowieBumpShaderTemplate->release();
		m_blueGlowieBumpShaderTemplate = 0;
	}
	if(m_holonetShaderTemplate)
	{
		m_holonetShaderTemplate->release();
		m_holonetShaderTemplate = 0;
	}
	if(m_holonetBumpShaderTemplate)
	{
		m_holonetBumpShaderTemplate->release();
		m_holonetBumpShaderTemplate = 0;
	}


	for(int i = 0; i < m_maxShaderSize; ++i)
		if(m_holoShaderTemplate[i])
		{
			m_holoShaderTemplate[i]->release();
			m_holoShaderTemplate[i] = 0;
		}

	

	if (m_attachedTransformModifiers)
	{
		std::for_each(m_attachedTransformModifiers->begin(), m_attachedTransformModifiers->end(), PointerDeleter());
		delete m_attachedTransformModifiers;
		m_attachedTransformModifiers=0;
	}

	IGNORE_RETURN(m_dpvsObject->release());
	m_dpvsObject = NULL;

	if (m_customizationData)
	{
		m_customizationData->release();
		m_customizationData = 0;
	}

	delete m_meshExtent;
	m_meshExtent=0;

	if (m_wornAppearanceObjects)
	{
		delete m_wornAppearanceObjects;
		m_wornAppearanceObjects = NULL;
	}

	if (m_attachedAppearances)
	{
		std::for_each(m_attachedAppearances->begin(), m_attachedAppearances->end(), PointerDeleter());
		delete m_attachedAppearances;
		m_attachedAppearances=0;
	}

	{
		const ShaderPrimitiveVectorVector::iterator endIt = m_perLodShaderPrimitives->end();
		for (ShaderPrimitiveVectorVector::iterator it = m_perLodShaderPrimitives->begin(); it != endIt; ++it)
			std::for_each(it->begin(), it->end(), PointerDeleter());

		delete m_perLodShaderPrimitives;
		m_perLodShaderPrimitives=0;
	}

	m_overridingAnimationResolver = 0;
	delete m_animationResolver;
	m_animationResolver=0;

	//-- release skeleton instances
	std::for_each(m_skeletons->begin(), m_skeletons->end(), PointerDeleter());
	delete m_skeletons;

	//-- Release owned MeshGenerator instances.
	const MeshGeneratorVectorVector::iterator endVectorIt = m_ownedMeshGenerators->end();
	for (MeshGeneratorVectorVector::iterator vectorIt = m_ownedMeshGenerators->begin(); vectorIt != endVectorIt; ++vectorIt)
	{
		//-- Get the LOD's MeshGenerator instances.
		MeshGeneratorVector &meshGenerators = *vectorIt;

		//-- Set each MeshGenerator instance's CustomizationData.
		std::for_each(meshGenerators.begin(), meshGenerators.end(), VoidMemberFunction(&MeshGenerator::release));
	}
	delete m_ownedMeshGenerators;

	delete m_animationEnvironment;
	m_animationEnvironment=0;
}

//-----------------------------------------------------------------

SkeletalAppearance2 * SkeletalAppearance2::asSkeletalAppearance2()
{
	return this;
}

//-----------------------------------------------------------------

SkeletalAppearance2 const * SkeletalAppearance2::asSkeletalAppearance2() const
{
	return this;
}

//-----------------------------------------------------------------

bool SkeletalAppearance2::collide(const Vector& start_o, const Vector& end_o, CollideParameters const & /*collideParameters*/, CollisionInfo& result) const
{
#if PRODUCTION == 0
	if (s_disableCollisionChecking)
		return false;
#endif

	bool collided = false;

	result.setPoint(end_o);

	if (m_extent.intersect(start_o, end_o))
	{
		//-- Only test against character if display LOD is available (async loaded).  Ensure it's built.
		if ((m_maxAvailableDetailLevelIndex >= 0) && const_cast<SkeletalAppearance2*>(this)->rebuildIfDirtyAndAvailable())
		{
			const ShaderPrimitiveVector &shaderPrimitives = getDisplayLodShaderPrimitives();
			const ShaderPrimitiveVector::const_iterator itEnd = shaderPrimitives.end();
			for (ShaderPrimitiveVector::const_iterator it = shaderPrimitives.begin(); it != itEnd; ++it)
			{
				ShaderPrimitive *const primitive = *it;
				NOT_NULL(primitive);

				if (primitive->collide(start_o, result.getPoint(), result))
					collided = true;
			}
		}
	}

	return collided;
}

// ----------------------------------------------------------------------

bool SkeletalAppearance2::implementsCollide() const
{
	return true;
}

// ----------------------------------------------------------------------

bool SkeletalAppearance2::isLoaded() const
{
	if ((m_displayLodIndex >= 0) && (m_displayLodIndex <= m_maxAvailableDetailLevelIndex))
	{
		// Current LOD index is valid.
		//-- Indicate we're loaded if current LOD is loaded for use.
		return isDetailLevelAvailable(m_displayLodIndex);
	}
	else
	{
		//-- There's nothing more to do if the current lod index is out of range, might as well 
		//   indicate we're loaded since nothing more will come about.
		return true;
	}
}

//----------------------------------------------------------------------

const BoxExtent & SkeletalAppearance2::getDelegateBoxExtent (bool exactMeshExtents) const
{
	const Object * const owner = getOwner ();
	if (m_extentDelegateObject.getPointer () && owner)
	{
		const Appearance * const app = m_extentDelegateObject->getAppearance ();
		if (app)
		{
			const Extent * extent = 0;
			
			if (exactMeshExtents)
			{
				const SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
				if (skelApp)
					extent = &const_cast<SkeletalAppearance2 *>(skelApp)->getExactMeshExtent ();
			}

			if (!extent)
				extent = app->getExtent ();

			if (extent)
			{
				const Transform & owner_transform_o2w = owner->getTransform_o2w ();
				const Transform & delegate_transform_o2w = m_extentDelegateObject->getTransform_o2w ();				
				const Transform & transform_owner2delegate = owner_transform_o2w.rotateTranslate_p2l (delegate_transform_o2w);
				m_extentDelegateTransformed.transform (extent, transform_owner2delegate, 1.0f);

				return m_extentDelegateTransformed;
			}
		}
	}

	return m_extent;
}

//-----------------------------------------------------------------

const Extent *SkeletalAppearance2::getExtent(void) const
{
	if (m_extentDelegateObject.getPointer ())
		return &getDelegateBoxExtent (false);

	//-- Never try to calculate extents based on a call to getExtent().  This will
	//   cause massive problems to the extent change logic.
	return &m_extent;
}

// ----------------------------------------------------------------------

AxialBox const SkeletalAppearance2::getTangibleExtent() const
{
	if (getAppearanceTemplateName() && *getAppearanceTemplateName())
	{
		Extent const * const extent = getExtent();
		if (extent)
			return extent->getBoundingBox();
	}

	return AxialBox();
}

// ----------------------------------------------------------------------

const Sphere &SkeletalAppearance2::getSphere(void) const
{
	if (m_extentDelegateObject.getPointer ())
	{
		const Extent * const extent = getExtent ();
		if (extent)
			return extent->getSphere ();
	}

	return m_extent.getSphere();
}

// ----------------------------------------------------------------------
/**
 * This function works like collide() but forces skeletal geometry to be
 * skinned prior to the collision check.
 *
 * This call is expensive and should be used only when it is known that
 * the appearance was not drawn prior to the collision check.  If the
 * object is being drawn, call collision() instead.  This function will
 * do the complete skinning operation.  UI needed this to do collision
 * testing against invisible objects.
 */

bool SkeletalAppearance2::collideForceGeometryUpdate(const Vector& start_o, const Vector& end_o, CollisionInfo& result) const
{
	//-- Try calling the normal collide now.
	return collide(start_o, end_o, CollideParameters::cms_default, result);
}

// ------------------------------------------------------------------

float SkeletalAppearance2::alter(float deltaTime)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("SkeletalAppearance2::alter");

	//-- Start memory usage tracking.
#ifdef _DEBUG
	unsigned long const bytesAllocatedBefore = MemoryManager::getCurrentNumberOfBytesAllocated();
#endif

	// alter the animation resolver.  this will move the animation forward
	// for the skeleton.  still needs evaluate called to evaluate the
	// animation.
	m_animationResolver->alter(deltaTime);

	//-- Stop memory usage tracking.
#ifdef _DEBUG
	unsigned long const bytesAllocatedAfter = MemoryManager::getCurrentNumberOfBytesAllocated();
	s_alterAllocationAmount += std::max(0, static_cast<int>(bytesAllocatedAfter - bytesAllocatedBefore));
#endif

	//-- Handle fade
	handleFade(deltaTime);

	//-- Unload any per-LOD data we haven't used in a while.
	if (m_unloadUnusedResourcesTimer.updateZero(deltaTime))
	{
		m_unloadUnusedResourcesTimer.setExpireTime(Random::randomReal(1.f, 2.f));
		unloadUnusedResources();
	}

	// update target info
	updateTargetPosition();

	// @todo figure out what this should really return.
	return AlterResult::cms_alterNextFrame;
}

// ------------------------------------------------------------------

DPVS::Object *SkeletalAppearance2::getDpvsObject() const
{
	return m_dpvsObject;
} //lint !e1763 // (Info -- Member function 'SkeletalAppearance2::getDpvsObject(void) const' marked as const indirectly modifies class) // Intentional.

// ------------------------------------------------------------------

void SkeletalAppearance2::render() const
{
	//-- Don't render if disabled.
#if PRODUCTION == 0
	if (s_disableSkeletalAppearanceRendering)
		return;

	// Increment call count.
	++s_addToCameraSceneCount;
#endif

	NP_PROFILER_AUTO_BLOCK_DEFINE("SkeletalAppearance2::render");

	//-- Get the camera and the owner object.
	const Camera *camera = &ShaderPrimitiveSorter::getCurrentCamera();
	const Object *object = getOwner();
	NOT_NULL(object);

	//-- Override animation resolver's transforms if requested.
	if (m_overridingAnimationResolver)
	{
		// Copy transforms from overriding resolver into our resolver.
		m_animationResolver->copyTransformsFrom(*m_overridingAnimationResolver);

		// Remove the overriding resolver.
		m_overridingAnimationResolver = 0;
	}

	//It isn't safe to calculate the screen radius based on a detail level that is above the maximum
	// because it may cause us to not render, thus never allowing us to (later, in calculateDisplayLodIndex)
	// set the level to where we want it.  This can cause objects which have invalid LOD levels below
	// the maximum to never render even though the maximum LOD is set.
	int savedDisplayLodIndex = -1;
	if(s_maximumDesiredDetailLevelEnabled && (m_displayLodIndex > s_maximumDesiredDetailLevelIndex))
	{
		savedDisplayLodIndex = m_displayLodIndex;
		m_displayLodIndex = s_maximumDesiredDetailLevelIndex;		
	}

	//-- Find the projected screen fraction occupied by the extents of this appearance.
	float screenRadiusInPixels = FLT_MAX;
	
	bool projectionSuccess       = camera->computeRadiusInScreenSpace(camera->rotateTranslate_w2o(object->getAppearanceSphereCenter_w()), getSphere().getRadius(), screenRadiusInPixels);
	float screenDiameterFraction = (projectionSuccess ? (s_twoOverScreenLength * screenRadiusInPixels) : FLT_MIN);
	
	bool skipRender = (screenDiameterFraction <= ConfigClientSkeletalAnimation::getNoRenderScreenFraction());

	/*if(s_maximumDesiredDetailLevelEnabled)
	{
		skipRender = false;
		screenDiameterFraction = 0.5;
		projectionSuccess = true;
	}*/

	if(savedDisplayLodIndex > 0)
	{
		m_displayLodIndex = savedDisplayLodIndex;
	}
	//-- Render the mesh.
	if (!skipRender)
	{

		//- Set up blue glowie if desired
		if(m_isBlueGlowie)
		{
			//- Load shader template if needed
			if(m_blueGlowieShaderTemplate == NULL)
			{
				m_blueGlowieBumpShaderTemplate = ShaderTemplateList::fetch(cs_blueGlowieMembraneShader.c_str());
				m_blueGlowieShaderTemplate = ShaderTemplateList::fetch(cs_blueGlowieMembraneShader.c_str());
			}

			ms_currentOverrideShaderTemplate = m_blueGlowieShaderTemplate;
			ms_currentOverrideBumpShaderTemplate = m_blueGlowieBumpShaderTemplate;

			//- Set up as override
			ShaderPrimitiveSorter::setPrepareToViewOverrideFunction(prepareToViewOverrideFunction);
		}
		else if(m_hologramType != HT_none)
		{
			int shaderIndexOffset = static_cast<int>(m_hologramType) / qualityPerShader; 

			for(int i = 0; i < m_maxShaderSize; ++i)
			{
				if(m_holoShaderTemplate[i] == 0)
					m_holoShaderTemplate[i] = ShaderTemplateList::fetch(cs_holoShaderLibrary[i].c_str());
			}

			int holoShaderIndex = Graphics::getFrameNumber() % 2;
			if(m_blackHologramFrame && !Random::random(m_blackHologramFrame))
			{
				holoShaderIndex = 2; // invis this frame
			}

			// Offset to the correct color version of our shader.
			holoShaderIndex += shaderIndexOffset * totalShaderColors;

			ms_currentOverrideShaderTemplate = m_holoShaderTemplate[holoShaderIndex];
			ms_currentOverrideBumpShaderTemplate = m_holoShaderTemplate[holoShaderIndex];

			//- Set up as override
			ShaderPrimitiveSorter::setPrepareToViewOverrideFunction(prepareToViewOverrideFunction);
		}
        else if(m_isHolonet)
		{
			//- Load shader template if needed
			if(m_holonetShaderTemplate == NULL)
			{
				m_holonetBumpShaderTemplate = ShaderTemplateList::fetch(cs_holonetBumpShader.c_str());
				m_holonetShaderTemplate = ShaderTemplateList::fetch(cs_holonetShader.c_str());
			}

			ms_currentOverrideShaderTemplate = m_holonetShaderTemplate;
			ms_currentOverrideBumpShaderTemplate = m_holonetBumpShaderTemplate;

            //- Set up as override
			ShaderPrimitiveSorter::setPrepareToViewOverrideFunction(prepareToViewOverrideFunction);
		}

		//-- Save previous display lod index so we can restore it if we're in UI context.
		int const previousDisplayLodIndex = m_displayLodIndex;

		//-- Determine LOD index.
		m_displayLodIndex = calculateDisplayLodIndex(screenDiameterFraction, m_displayLodIndex);
		if (m_displayLodIndex < 0)
		{
			//-- For the case of client-baked wearables.  Nothing to do: exit.
			if (s_uiContextEnabled)
			{
				//-- Restore previous display lod index.
				m_displayLodIndex = previousDisplayLodIndex;
			}
			return;
		}

		//-- Keep track of most-recently-used frame where we used this detail level's shader prims.
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_displayLodIndex, static_cast<int>(m_perLodMruFrameVector.size()));
		m_perLodMruFrameVector[static_cast<IntVector::size_type>(m_displayLodIndex)] = Os::getNumberOfUpdates();

		//-- Update extents.
		// @todo note the case of updating extents with attached appearances could be implemented more efficiently
		//       within this function body here.  The primary savings would come through iteration of appearances
		//       and application of transforms.
		updateExtentWithLod(m_extent, m_displayLodIndex);

		if (m_showMesh)
		{
			//-- If any wearables were deleted, catch them here to force a rebuild.
			// @todo see if we can remove this.
			const_cast<SkeletalAppearance2*>(this)->removeDeletedWearables();

			//-- Handle rebuild or adjust display LOD if dirty and not all mesh generators are loaded.
			bool const readyForRender = const_cast<SkeletalAppearance2*>(this)->rebuildOrAdjustDisplayLodIndex();
			if (!readyForRender)
			{
				// This appearance simply is not ready to render right now.  At least one of its owned or worn
				// mesh generators is not finished asynchronously loading.

				//-- Handle fade-in state trigger.
				if (m_fadeState == FS_uninitialized)
				{
					// This is the first time this appearance is trying to render itself.
					// Move fade in to "waiting for async loading to complete" state.
					m_fadeState = FS_waitingForLoadCompletion;
					m_fadeFraction = 0.0f;
				}
			}
			else
			{
				//-- Handle fade-in state triggers.
				switch (m_fadeState)
				{
					case FS_uninitialized:
						{
							//-- This is the first time this appearance is trying to render itself.
							//   Apparently there is no async loading to wait for, so move directly
							//   to the fading-in state.
							m_fadeState    = FS_fadingIn;
							m_fadeFraction = 0.0f;
						}
						break;

					case FS_waitingForLoadCompletion:
					case FS_fadingIn:
					case FS_notFading:
					default:
						{
							// Nothing special to do.
						}
						break;
				}

				// Handle mesh rendering.

				//-- Update the animation environment's last rendered frame number.
				m_animationEnvironment->setMostRecentVisibleGraphicsFrameNumber(Graphics::getFrameNumber());

				//-- Determine if we're batch rendering.
				bool useBatcher = false;
				if (!s_disableBatcher 
					&& (m_fadeState == FS_notFading) 
					&& (m_hologramType == HT_none)
					&& !m_isBlueGlowie
				)
				{
					if (m_mostRecentRenderUsedBatch || s_uiContextEnabled)
					{
						// Rendered with batcher last frame.  Keep it turned on if screen fraction is smaller than near screen fraction.
						useBatcher = (screenDiameterFraction < s_batchRendererStartNearFraction);
					}
					else
					{
						// Did not render with batcher last frame.  Only turn on if screen fraction is smaller than far screen fraction.
						useBatcher = (screenDiameterFraction <= s_batchRendererStartFarFraction);
					}
				}

				if (!s_uiContextEnabled)
				{
					// Only update this value if we're not rendering in the UI context.
					m_mostRecentRenderUsedBatch = useBatcher;
				}

				//-- Figure out skinning mode for shader primitives.
				SkeletalAppearanceTemplate const &appearanceTemplate = getSkeletalAppearanceTemplate();
				ShaderPrimitive::SkinningMode  skinningMode;

				bool const mustUseSoftSkinning = appearanceTemplate.mustUseSoftSkinning();

				if (screenDiameterFraction <= ConfigClientSkeletalAnimation::getNoSkinningScreenFraction())
					skinningMode = ShaderPrimitive::SM_noSkinning;
				else if (!mustUseSoftSkinning && (m_forceHardSkinningEnabled || (screenDiameterFraction <= ConfigClientSkeletalAnimation::getHardSkinningScreenFraction())))
					skinningMode = ShaderPrimitive::SM_hardSkinning;
				else
					skinningMode = ShaderPrimitive::SM_softSkinning;

#if PRODUCTION == 0
				//-- Track # characters rendered in each skinning mode (separate batched from standard rendering).
				++s_perSkinningModeCharacterCount[(useBatcher ? cs_skinningModeCount : 0) + static_cast<int>(skinningMode)];

				//-- Track # characters rendered at each LOD (separate batched from standard rendering).
				++s_perLodRenderedCharacterCount[(useBatcher ? cs_maxLodCount : 0) + m_displayLodIndex];
#endif

				//-- Tell each shader primitive about the skinning mode.  Render if not batching.
				const ShaderPrimitiveVector &shaderPrimitives = getDisplayLodShaderPrimitives();

				const ShaderPrimitiveVector::const_iterator itEnd = shaderPrimitives.end();
				for (ShaderPrimitiveVector::const_iterator it = shaderPrimitives.begin(); it != itEnd; ++it)
				{
					//-- Get the shader primitive.
					ShaderPrimitive *const shaderPrimitive = (*it);
					NOT_NULL(shaderPrimitive);

					//-- NOTE: do not sned the sbsShaderPrimitive directly to the ShaderPrimitiveSorter.  That
					//   will screw up and bypass the TextureRendererShaderPrimitives that sometimes piggyback
					//   on top of a SBSSP.
					SoftwareBlendSkeletalShaderPrimitive *const sbsShaderPrimitive = shaderPrimitive->asSoftwareBlendSkeletalShaderPrimitive();
					NOT_NULL(sbsShaderPrimitive);

					//-- Set the skinning parameters.
					sbsShaderPrimitive->setSkinningMode(skinningMode);
					sbsShaderPrimitive->setEveryOtherFrameSkinningEnabled(m_everyOtherFrameSkinningEnabled);

					if (!useBatcher)
					{
						if (m_fadeState == FS_notFading)
							ShaderPrimitiveSorter::add(*shaderPrimitive);
						else
							ShaderPrimitiveSorter::addWithAlphaFadeOpacity(*shaderPrimitive, true, m_fadeFraction, true, m_fadeFraction);

#if PRODUCTION == 0
						//-- Track # shader primitives rendered at each LOD (separate batched from standard rendering).
						++s_perLodRenderedShaderPrimitiveCount[m_displayLodIndex];
#endif
					}
				}

				//-- Submit appearance for batch rendering.
				if (useBatcher)
				{
					FullGeometrySkeletalAppearanceBatchRenderer::getInstance()->submit(*this);

#if PRODUCTION == 0
						//-- Track # shader primitives rendered at each LOD (separate batched from standard rendering).
					s_perLodRenderedShaderPrimitiveCount[cs_maxLodCount + m_displayLodIndex] += static_cast<int>(shaderPrimitives.size());
#endif
				}

				//-- Render attachments.
				if (m_attachedAppearances && !m_attachedAppearances->empty() && m_showAttachments)
				{
					const Transform *const transformToRoot = NON_NULL(getDisplayLodSkeleton())->getJointToRootTransformArray();
					Transform              transformToWorld(Transform::IF_none);

					//-- Be prepared to delete entries where the attached Object has been deleted.
					bool deletedAnyAttachments = false;

					for (AttachedAppearanceVector::iterator aaIt = m_attachedAppearances->begin(); aaIt != m_attachedAppearances->end();)
					{
						// get attached appearance
						NOT_NULL(*aaIt);
						AttachedAppearance *aaData = *aaIt;

						// Skip hidden attached items --- hidden items are not rendered.
						if (aaData->isHidden())
						{
							++aaIt;
							continue;
						}

						Object           *const attachedObject     = aaData->getObject();
						const Appearance *const attachedAppearance = (attachedObject ? attachedObject->getAppearance() : 0);

						if (attachedAppearance)
						{
							NOT_NULL(attachedObject);

							// Construct hardpointToWorld.
							transformToWorld.multiply(object->getTransform_o2w(), transformToRoot[aaData->getTransformIndex(m_displayLodIndex)]);
							attachedAppearance->setTransform_w(transformToWorld);

							// The following line assumes the Object for the attachment is not in the world (reasonable for an attachment).  
							// If the attached object is in the world and in a cell, the logic here is wrong.
							attachedObject->setTransform_o2p(transformToWorld);

							// Submit for rendering.
							attachedAppearance->objectListCameraRender();

							//-- Increment loop.
							++aaIt;
						}
						else
						{
							//-- Delete the entry.
							DEBUG_REPORT_LOG(true, ("Detatching currently attached but deleted object from [%s].\n", getOwner()->getDebugName()));

							delete aaData;
							aaIt = m_attachedAppearances->erase(aaIt);

							deletedAnyAttachments = true;
						}
					}

					if (deletedAnyAttachments && s_destroyedAttachmentWearableCallback)
						(*s_destroyedAttachmentWearableCallback)(*const_cast<Object*>(object));
				}
			}
		}

		//-- Restore previous display lod index if we're in the ui context so we don't disturb world temporal-based logic.
		if (s_uiContextEnabled)
			m_displayLodIndex = previousDisplayLodIndex;

		//- Turn off blue glowie shader override
		if (m_isBlueGlowie || m_hologramType != HT_none)
		{
			ms_currentOverrideShaderTemplate = 0;
			ms_currentOverrideBumpShaderTemplate = 0;
			ShaderPrimitiveSorter::setPrepareToViewOverrideFunction(0);
		}
		//- Turn off holonet shader override
		if (m_isHolonet)
		{
			ms_currentOverrideShaderTemplate = 0;
			ms_currentOverrideBumpShaderTemplate = 0;
			ShaderPrimitiveSorter::setPrepareToViewOverrideFunction(0);
		}
	}
	else if (m_displayLodIndex >= 0)
	{
		bool displayLodIndexChanged = false;
		int displayLodIndex = calculateDisplayLodIndex(screenDiameterFraction, m_displayLodIndex);
		if (displayLodIndex >= 0 && m_displayLodIndex != displayLodIndex)
		{
			m_displayLodIndex = displayLodIndex;
			displayLodIndexChanged = true;
		}
		if (!m_lodIsReady || displayLodIndexChanged)
			updateExtentWithLod(m_extent, m_displayLodIndex);
	}

	//-- Handle mesh extent rendering.
	if (m_showMeshExtent)
	{
		Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorStaticShader());
		Graphics::setObjectToWorldTransformAndScale(object->getTransform_a2w(), Vector::xyz111);
		Graphics::drawExtent(&const_cast<SkeletalAppearance2*>(this)->getExactMeshExtent(), VectorArgb::solidYellow);
	}

#if PRODUCTION == 0
	//-- Handle skeleton rendering.
	if (s_showSkeleton)
	{
		//-- Determine LOD index.
		m_displayLodIndex = calculateDisplayLodIndex(screenDiameterFraction, m_displayLodIndex);

		//-- Rebuild mesh as necessary.
		IGNORE_RETURN(const_cast<SkeletalAppearance2*>(this)->rebuildIfDirtyAndAvailable());

		//-- Render the skeleton.
		Skeleton const *const skeleton = getDisplayLodSkeleton();
		if (skeleton)
			skeleton->addShaderPrimitives(*this);
	}
#endif

	//-- Handle rendering the target direction.
#ifdef _DEBUG
	if (ms_renderTargetDirection)
	{
		camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_none, object->getTransform_o2w(), Vector(0.0f, 1.5f, 0.0f), Vector(0.0f, 1.5f, 0.0f) + m_directionToTarget * 2.0f, PackedArgb::solidRed));
		camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_none, object->getTransform_o2w(), Vector(0.0f, 1.5f, 0.0f), Vector(0.0f, 1.5f, 2.0f), PackedArgb::solidBlue));
	}

	Appearance::render();
#endif

	//-- Handle rendering a sphere or frame of reference if this creature is an attacker or defender.
#ifdef _DEBUG
	if (ConfigClientSkeletalAnimation::getRenderPlaybackScriptFeedback() && camera)
	{
		const bool isAttacker = (PlaybackScriptManager::getPlaybackScriptForActorAtIndex(0, object) != NULL);
		const bool isDefender = (PlaybackScriptManager::getPlaybackScriptForActorAtIndex(1, object) != NULL);

		if (isAttacker || isDefender)
		{
			const Transform &transform_o2w = object->getTransform_o2w();
			const Sphere    &sphere        = getSphere();

			if (isAttacker)
				camera->addDebugPrimitive(new FrameDebugPrimitive(UtilityDebugPrimitive::S_none, transform_o2w, sphere.getRadius()));

			if (isDefender)
				camera->addDebugPrimitive(new SphereDebugPrimitive(UtilityDebugPrimitive::S_none, transform_o2w, sphere.getCenter(), sphere.getRadius(), 6, 6));
		}
	}
#endif
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::setCustomizationData(CustomizationData *customizationData)
{
	if (customizationData == m_customizationData)
		return;

	//-- release old reference
	if (m_customizationData)
		m_customizationData->release();

	//-- keep track of new customization data
	m_customizationData = customizationData;

	//-- fetch new reference
	if (m_customizationData)
		m_customizationData->fetch();

	//-- Set customization data for all owned MeshGenerator objects.
	//   note: we do not set the CustomizationData for wearables, as each
	//         has its own Object instance and associated CustomizationData.
	const MeshGeneratorVectorVector::iterator endVectorIt = m_ownedMeshGenerators->end();
	for (MeshGeneratorVectorVector::iterator vectorIt = m_ownedMeshGenerators->begin(); vectorIt != endVectorIt; ++vectorIt)
	{
		//-- Get the LOD's MeshGenerator instances.
		MeshGeneratorVector &meshGenerators = *vectorIt;

		//-- Set each MeshGenerator instance's CustomizationData.
		const MeshGeneratorVector::iterator endIt = meshGenerators.end();
		for (MeshGeneratorVector::iterator it = meshGenerators.begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			(*it)->setCustomizationData(customizationData);
		}
	}

	//-- force a rebuild because this is the only way right now to feed
	//   CustomizationData into just _our_ TextureRenderer instances.
	//   -TRF- this probably guarantees that we'll build every mesh
	//   at least twice, since Object instances probably will 
	//   initialize and set the CustomizationData after the appearance 
	//   is created.
	markAsDirty();
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::addCustomizationVariables(CustomizationData &customizationData) const
{
	//-- Force use of highest detail level.  I do this because only the highest detail level is
	//   guaranteed to have all the customization variables.
	m_displayLodIndex = 0;

	//-- Force generation of lazily-constructed appearance data.
	bool const isReady = const_cast<SkeletalAppearance2*>(this)->rebuildIfDirtyAndAvailable();
	if (!isReady)
	{
		// Code counting on this should disable asynchronous loading before creating this appearance.
		DEBUG_WARNING(true, ("addCustomizationVariables() failed for appearance [%s] due to asynchronous loading.", getAppearanceTemplateName() ? getAppearanceTemplateName() : "<NULL appearance template name>"));
		return;
	}

	//-- Add CustomizationData for each owned MeshGenerator.  Use only the highest detail LOD's meshes
	//   to do this.
	if (!m_ownedMeshGenerators->empty())
	{
		MeshGeneratorVector const &meshGenerators = getOwnedMeshGeneratorsForDetailLevel(0);

		if (!meshGenerators.empty())
		{
			const MeshGeneratorVector::const_iterator endIt = meshGenerators.end();
			for (MeshGeneratorVector::const_iterator it = meshGenerators.begin(); it != endIt; ++it)
			{
				NOT_NULL(*it);
				(*it)->addCustomizationVariables(customizationData);
			}
		}
	}

	//-- add CustomizationData for each ShaderPrimitive
	// @todo -TRF- this is wrong in the case of an owner object wearing another object.
	//       This will report all of the wearable's customizations as being part of the
	//       owner when this is false.  Fix this by separating the ShaderPrimitive instances
	//       generated from owned MeshGenerator instances and only calling this on the owned
	//       ShaderPrimitive instances.

	// Note: Only scan through the highest LOD level's primitives.
	if (!m_perLodShaderPrimitives->empty())
	{
		const ShaderPrimitiveVector &shaderPrimitives = m_perLodShaderPrimitives->front();

		const ShaderPrimitiveVector::const_iterator endIt = shaderPrimitives.end();
		for (ShaderPrimitiveVector::const_iterator it = shaderPrimitives.begin(); it != endIt; ++it)
		{
			NOT_NULL(*it);
			(*it)->addCustomizationVariables(customizationData);
		}
	}
}

// ----------------------------------------------------------------------

bool SkeletalAppearance2::findHardpoint(CrcString const &hardpointName, Transform &hardpointTransform) const
{
	bool foundTransform = false;
	int  transformIndex;

	//-- First check the skeleton for the hardpoint/joint.
	const Skeleton *const skeleton = getDisplayLodSkeleton();
	if (!skeleton)
		DEBUG_WARNING(s_logReadyIssues, ("findHardpoint() skipped skeleton for appearance [%s] because async loading detail level [%d].", getAppearanceTemplateName(), m_displayLodIndex));
	else
	{
		skeleton->findTransformIndex(hardpointName, &transformIndex, &foundTransform);
		if (foundTransform)
		{
			hardpointTransform = skeleton->getJointToRootTransformArray()[transformIndex];
			return true;
		}
	}

	//-- Check all attached appearances for the hardpoint.
	if (m_attachedAppearances && !m_attachedAppearances->empty())
	{
		Transform  attachedTransform(Transform::IF_none);

		const AttachedAppearanceVector::const_iterator endIt = m_attachedAppearances->end();
		for (AttachedAppearanceVector::const_iterator it = m_attachedAppearances->begin(); it != endIt; ++it)
		{
			//-- Get the attached appearance.
			const Appearance *attachedAppearance = (*it)->getAppearance();
			if (!attachedAppearance)
			{
				// @todo consider removing this attachment here.
				continue;
			}

			//-- Return the first attached appearance's hardpoint that matches the name.
			if (attachedAppearance->findHardpoint(hardpointName, attachedTransform))
			{
				//-- Fixup transform to incorporate it's attachment to this appearance at one of this appearance's hardpoints.
				transformIndex = (*it)->getTransformIndex(m_displayLodIndex);

				if (skeleton)
					hardpointTransform.multiply(skeleton->getJointToRootTransformArray()[transformIndex], attachedTransform);
				else
					hardpointTransform = attachedTransform;

				return true;
			}
		}
	}

	//-- Failed to find the transform.
	return false;
}

// ----------------------------------------------------------------------

int SkeletalAppearance2::getDetailLevelCount() const
{
	return m_maxAvailableDetailLevelIndex + 1;
}

// ----------------------------------------------------------------------
/**
 * Indicates whether a valid detail level is ready for use.
 *
 * Currently the only reason a detail level would not be ready for use is
 * if it is in the process of getting asynchronously loaded.
 *
 * @param lodIndex  must be in range 0 .. (getDetailLevelCount() - 1), inclusive.
 *
 * @return  true if the detail level is ready for use; false if the detail
 *          level is not ready for use.
 */

bool SkeletalAppearance2::isDetailLevelAvailable(int lodIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, getDetailLevelCount());
	return areAllMeshGeneratorsReadyForDetailLevel(lodIndex);
}

// ----------------------------------------------------------------------
/**
 * Rebuild the appearance data if the current display LOD index's
 * display data is dirty.
 *
 * The caller generally shouldn't need to call this function; it is handled
 * under the hood.
 *
 * Note this function assumes the current LOD display level is set
 * properly.  The addToCameraScene, each frame, determines the
 * display lod level.  It is possible this function, if called before
 * addToCameraScene, will rebuild a different LOD display level than
 * that which is used to render.
 *
 * @return  true if the detail level is available and built for use; false otherwise.
 */

bool SkeletalAppearance2::rebuildIfDirtyAndAvailable()
{
	return rebuildIfDirtyAndAvailable(m_displayLodIndex);
}

// ------------------------------------------------------------------

void SkeletalAppearance2::rebuildMesh(int lodIndex)
{
	NP_PROFILER_AUTO_BLOCK_DEFINE("SkeletalAppearance2::rebuildMesh");

#if PRODUCTION == 0
	++s_rebuildMeshCallCount;

	ProfilerTimer::Type  startTime;
	ProfilerTimer::getTime(startTime);
#endif

#if TRACK_APPEARANCE_BUILDING
	//-- Test code to find how many times per frame this is getting called.
	{
		static int countFrameNumber = -1;
		static int callCountThisFrame = 0;

		const int currentFrameNumber = Os::getNumberOfUpdates();
		if (countFrameNumber != currentFrameNumber)
		{
			// Clear the frame count.
			callCountThisFrame = 0;
			countFrameNumber   = currentFrameNumber;
		}
		
		// Bump up count for this frame.
		++callCountThisFrame;

		DEBUG_REPORT_LOG(true, ("rebuildMesh: frame [%d] call #[%d].\n", currentFrameNumber, callCountThisFrame));
	}
#endif

	//-- Start memory usage tracking.
#ifdef _DEBUG
	unsigned long bytesAllocatedBefore = MemoryManager::getCurrentNumberOfBytesAllocated();
#endif

	//-- Remove any wearables associated with deleted Object instances.
	//   This allows the rest of the functions to assume there are no stale wearables
	//   present.
	removeDeletedWearables();

	//-- Can't do anything if there are no Skeleton instances.
	if (m_skeletons->empty())
	{
		return;
	}

	//-- build the composite mesh from this base appearance and all wearables
	//   Note: this must happen before we apply each MeshGenerator's skeleton
	//   modifications below.

	//-- Remove all mesh generators from CompositeMesh.
	// @todo first clear out a composite mesh.
	CompositeMesh &compositeMesh = getCompositeMesh();

	//-- Note: I do not clear out the existing MeshGenerators in the CompositeMesh because I do it
	//         at the tail end of processing.  If I didn't do it at the tail end of processing, there
	//         would be MeshGenerator leaks caused at exit time.

	buildCompositeMesh(compositeMesh, lodIndex);

	//-- Get this LOD's skeleton.
	const int       usedSkeletonIndex = std::min(lodIndex, static_cast<int>(m_skeletons->size()) - 1);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, usedSkeletonIndex, static_cast<int>(m_skeletons->size()));
	Skeleton *const skeleton          = (*m_skeletons)[static_cast<SkeletonVector::size_type>(usedSkeletonIndex)];
	NOT_NULL(skeleton);

	//-- Modify the LOD skeleton as necessary for each MeshGenerator.
	skeleton->beginSkeletonModification();

	// Build the LOD skeleton from this base appearance and all wearables.
	addSkeletonSegments(*skeleton, usedSkeletonIndex);

	// Apply the skeleton modifications.
	compositeMesh.applySkeletonModifications(*skeleton);

	// We're done modifying the LOD skeleton
	skeleton->endSkeletonModification();

	// Find the transform indices for all attached appearances at this detail level.  The indices
	// may have changed when we rebuilt the skeleton if any hardpoints or segments were added or removed.
	lookupAttachmentTransformIndices(lodIndex);

	//-- Handle building the mesh data for shader primitives so long as there's any mesh detail levels.
	if (!m_perLodShaderPrimitives->empty())
	{
		//-- Build the new shader primitives.  we do this before releasing the old ones so
		//   that we avoid releasing textures and other expensive resources that we may need
		//   to load immediately afterward with the new build.
		ShaderPrimitiveVector  workingShaderPrimitives;

		// Start tracking memory usage.
#ifdef _DEBUG
		unsigned long const spBytesAllocatedBefore = MemoryManager::getCurrentNumberOfBytesAllocated();
#endif

		compositeMesh.addShaderPrimitives(*this, lodIndex, skeleton->getTransformNameMap(), workingShaderPrimitives);

		// Stop tracking memory usage.
#ifdef _DEBUG
		unsigned long const spBytesAllocatedAfter = MemoryManager::getCurrentNumberOfBytesAllocated();
		s_shaderPrimitiveAllocationAmount += std::max(0, static_cast<int>(spBytesAllocatedAfter - spBytesAllocatedBefore));

		// Compensate rebuildMesh alloc amount for shader prim alloc amount so we don't double count.
		bytesAllocatedBefore += std::max(0, static_cast<int>(spBytesAllocatedAfter - spBytesAllocatedBefore));
#endif

		//-- swap the new and old --- now m_shaderPrimitives holds the valid shader primitives
		NOT_NULL(m_perLodShaderPrimitives);
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, static_cast<int>(m_perLodShaderPrimitives->size()));

		ShaderPrimitiveVector &shaderPrimitives = (*m_perLodShaderPrimitives)[static_cast<ShaderPrimitiveVectorVector::size_type>(lodIndex)];
		shaderPrimitives.swap(workingShaderPrimitives);

		//-- release the old shader primitives
		std::for_each(workingShaderPrimitives.begin(), workingShaderPrimitives.end(), PointerDeleter());
	}

	//-- Mark as clean.
	m_appearanceDirty[static_cast<BoolVector::size_type>(lodIndex)] = false; //lint !e1058 // error 1058: (Error -- Initializing a non-const reference '_STL::_Bit_reference &' with a non-lvalue) // This is the class interface.

	//-- Reset most recently used frame so we don't pitch it as soon as we create it.  Chances
	//   are we created it because we're going to need it in the next few frames.
	m_perLodMruFrameVector[static_cast<BoolVector::size_type>(lodIndex)] = Os::getNumberOfUpdates();

	//-- Clear out all MeshGenerator instances so we don't leak these MeshGenerator instances.
	compositeMesh.removeAllMeshGenerators();

	//-- Attach transform modifiers.
	if (m_attachedTransformModifiers)
	{
		AttachedTransformModifierVector::iterator const endIt = m_attachedTransformModifiers->end();
		for (AttachedTransformModifierVector::iterator it = m_attachedTransformModifiers->begin(); it != endIt; ++it)
		{
			AttachedTransformModifier *const atm = *it;
			if (atm)
			{
				//-- Check if this LOD skeleton has the specified transform name.
				bool  foundIt        = false;
				int   transformIndex = -1;
				skeleton->findTransformIndex(atm->getTransformName(), &transformIndex, &foundIt);

				if (foundIt)
				{
					//-- Attach the modifier.
					skeleton->attachTransformModifier(transformIndex, atm->getTransformModifier());
				}
			}
		}
	}

	//-- Build mesh extent/skeleton extent deltas.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, static_cast<int>(m_perLodMeshExtentMinDelta.size()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, static_cast<int>(m_perLodMeshExtentMaxDelta.size()));

	//-- Update extent deltas.
	calculateExtentDeltas(lodIndex, m_perLodMeshExtentMinDelta[static_cast<VectorVector::size_type>(lodIndex)], m_perLodMeshExtentMaxDelta[static_cast<VectorVector::size_type>(lodIndex)]);

	//-- Update extent.
	updateExtentWithLod(m_extent, lodIndex);

#if PRODUCTION == 0
	//-- Stop recording time spent in function.
	ProfilerTimer::Type  stopTime;
	ProfilerTimer::getTime(stopTime);

	s_rebuildMeshCallTime += (stopTime - startTime);
#endif

	//-- Stop memory usage tracking.
#ifdef _DEBUG
	unsigned long const bytesAllocatedAfter = MemoryManager::getCurrentNumberOfBytesAllocated();
	s_rebuildMeshAllocationAmount += std::max(0, static_cast<int>(bytesAllocatedAfter - bytesAllocatedBefore));
#endif
}

// ----------------------------------------------------------------------

TransformAnimationController *SkeletalAppearance2::getAnimationController()
{
	// Return the primary animation controller.
	return m_animationResolver->getAnimationController(0);
}

// ----------------------------------------------------------------------

const TransformAnimationController *SkeletalAppearance2::getAnimationController() const
{
	// Return the primary animation controller.
	return m_animationResolver->getAnimationController(0);
}

// ----------------------------------------------------------------------
/**
 * Retrieve the number of Skeleton detail levels present in this
 * SkeletalAppearance2 instance.
 *
 * At the time of this writing, I anticipate a 1-1 correspondence between
 * skeleton and mesh LODs.
 *
 * @return  the number of Skeleton detail levels present in this
 *          SkeletalAppearance2 instance.
 */

int SkeletalAppearance2::getSkeletonLodCount() const
{
	return static_cast<int>(m_skeletons->size());
}

// ----------------------------------------------------------------------
/**
 * Retrieve a non-modifiable reference to the Skeleton instance used
 * for the specified LOD.
 *
 * @param lodIndex  index of the detail level for which the Skeleton
 *                  should be retrieved.  Must fall in the range
 *                  0 through getSkeletonLodCount() - 1, inclusive.
 *
 * @return  a non-modifiable reference to the Skeleton instance used
 *          for the specified LOD.
 */

const Skeleton &SkeletalAppearance2::getSkeleton(int lodIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, getSkeletonLodCount());

	//-- Ensure skeleton is properly initialized.
	if (m_appearanceDirty[static_cast<BoolVector::size_type>(lodIndex)])
		const_cast<SkeletalAppearance2*>(this)->rebuildMesh(m_displayLodIndex);

	const Skeleton *const skeleton = (*m_skeletons)[static_cast<size_t>(lodIndex)];
	NOT_NULL(skeleton);

	return *skeleton;
}

// ----------------------------------------------------------------------
/**
 * Retrieve a modifiable reference to the Skeleton instance used
 * for the specified LOD.
 *
 * @param lodIndex  index of the detail level for which the Skeleton
 *                  should be retrieved.  Must fall in the range
 *                  0 through getSkeletonLodCount() - 1, inclusive.
 *
 * @return  a modifiable reference to the Skeleton instance used
 *          for the specified LOD.
 */

Skeleton &SkeletalAppearance2::getSkeleton(int lodIndex)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, getSkeletonLodCount());

	Skeleton *const skeleton = (*m_skeletons)[static_cast<size_t>(lodIndex)];
	NOT_NULL(skeleton);

	return *skeleton;
}

// ----------------------------------------------------------------------
/**
 * Attach the specified object's appearance to the given transform.
 *
 * The appearance for the attached object will be rendered at the specified
 * hardpoint/joint.  It will not be skinned along with the character.  If
 * the caller wants to attach a skinned object like a piece of clothing,
 * the wear() call should be used instead.
 *
 * The code will handle the case of the Object instance being destroyed
 * prior to a detatch() call.
 *
 * @param object         the Appearance instance for object will be attached to this appearance.
 * @param transformName  the name of the joint/hardpoint that will become the local origin for
 *                       the attached Object.
 */

void SkeletalAppearance2::attach(Object *object, const CrcString &transformName)
{
	//-- Ensure object is not NULL.
	if (!object)
	{
		DEBUG_WARNING(true, ("attempting to attach NULL object to an instance of [%s], ignoring.", getAppearanceTemplateName()));
		return;
	}

	//-- Ensure object appearance is not NULL.
	if (!(object->getAppearance()))
	{
		WARNING_STRICT_FATAL(true, ("appearance [%s]: tried to attach object [%s] with NULL appearance.", getAppearanceTemplateName(), object->getObjectTemplateName()));
		return;
	}

	//-- Create the AttachedAppearanceVector as necessary.
	if (!m_attachedAppearances)
		m_attachedAppearances = new AttachedAppearanceVector();

#ifdef _DEBUG
	//-- Ensure the given Object instance is not already attached.
	const AttachedAppearanceVector::const_iterator findIt = std::find_if(m_attachedAppearances->begin(), m_attachedAppearances->end(), AttachedAppearance::FindByObject(object));
	DEBUG_FATAL(findIt != m_attachedAppearances->end(), ("appearance [%s]: attachment failed: object [%s] already attached (to transform [%s])", getAppearanceTemplateName(), object->getObjectTemplateName(), (*findIt)->getTransformName().getString()));
#endif

	//-- Create the AttachedAppearance info.
	const int detailCount = static_cast<int>(m_skeletons->size());

	m_attachedAppearances->push_back(new AttachedAppearance(object, transformName, detailCount));
	AttachedAppearance *const attachedAppearance = m_attachedAppearances->back();

	//DEBUG_REPORT_LOG(true, ("appearance [0x%08x][%s]: attached object [0x%08x][%s] (to transform [%s])\n", this, getAppearanceTemplateName(), object, object->getObjectTemplateName(), transformName.getString()));
	
	//-- Look up transform index for each detail level
	int detailIndex = 0;

	const SkeletonVector::iterator endIt = m_skeletons->end();
	for (SkeletonVector::iterator it = m_skeletons->begin(); it != endIt; ++it, ++detailIndex)
	{
		Skeleton *const skeleton = *it;
		NOT_NULL(skeleton);

		const bool attachIssues = !attachedAppearance->lookupTransformIndex(detailIndex, *skeleton);
		DEBUG_WARNING(attachIssues, ("appearance [%s] object [%s]: attachment transform [%s] does not exist for detail level [%d]", getAppearanceTemplateName(), object->getObjectTemplateName(), transformName.getString(), detailIndex));
		UNREF(attachIssues);
	}

	//-- Take attached object out of the world so it doesn't get double altered.
	if (object->isInWorld())
		object->removeFromWorld();

	//-- Update extents.
	updateExtentWithLod(m_extent, m_displayLodIndex);
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::detach(const Object *object)
{
	//-- Ensure object is not NULL.
	if (!object)
	{
		DEBUG_WARNING(true, ("attempting to attach NULL object to an instance of [%s], ignoring.", getAppearanceTemplateName()));
		return;
	}

	//-- Ensure attached appearance container exists.
	if (!m_attachedAppearances)
	{
		WARNING_STRICT_FATAL(true, ("appearance [%s]: tried to detach non-attached [%s], but has no attached appearances", getAppearanceTemplateName(), object->getObjectTemplateName()));
		return;
	}

	//-- Find the AttachedAppearance corresponding to object.
	const AttachedAppearanceVector::iterator findIt = std::find_if(m_attachedAppearances->begin(), m_attachedAppearances->end(), AttachedAppearance::FindByObject(object));

	if (findIt == m_attachedAppearances->end())
	{
		// appearance was not attached
		WARNING_STRICT_FATAL(true, ("appearance [0x%08x][%s]: tried to detach non-attached [0x%08x][%s]", this, getAppearanceTemplateName(), object, object->getObjectTemplateName()));
		return;
	}

	//-- remove the attached appearance
	delete *findIt;
	IGNORE_RETURN(m_attachedAppearances->erase(findIt));

	//DEBUG_REPORT_LOG(true, ("appearance [0x%08x][%s]: removed object [0x%08x][%s]\n", this, getAppearanceTemplateName(), object, object->getObjectTemplateName()));

	//-- Update extents.
	updateExtentWithLod(m_extent, m_displayLodIndex);
}

// ----------------------------------------------------------------------

int SkeletalAppearance2::getAttachedAppearanceCount() const
{
	if (m_attachedAppearances)
		return static_cast<int>(m_attachedAppearances->size());
	else
		return 0;
}

// ----------------------------------------------------------------------

Appearance *SkeletalAppearance2::getAttachedAppearance(int index)
{
	NOT_NULL(m_attachedAppearances);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_attachedAppearances->size()));

	return (*m_attachedAppearances)[static_cast<size_t>(index)]->getAppearance();
}

// ----------------------------------------------------------------------

const Appearance *SkeletalAppearance2::getAttachedAppearance(int index) const
{
	NOT_NULL(m_attachedAppearances);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_attachedAppearances->size()));

	return (*m_attachedAppearances)[static_cast<size_t>(index)]->getAppearance();
}

// ----------------------------------------------------------------------

Object *SkeletalAppearance2::getAttachedObject(int index)
{
	NOT_NULL(m_attachedAppearances);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_attachedAppearances->size()));

	return (*m_attachedAppearances)[static_cast<size_t>(index)]->getObject();
}

// ----------------------------------------------------------------------

const Object *SkeletalAppearance2::getAttachedObject(int index) const
{
	NOT_NULL(m_attachedAppearances);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_attachedAppearances->size()));

	return (*m_attachedAppearances)[static_cast<size_t>(index)]->getObject();
}

// ----------------------------------------------------------------------

const CrcString &SkeletalAppearance2::getAttachedAppearanceTransformName(int index) const
{
	NOT_NULL(m_attachedAppearances);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_attachedAppearances->size()));

	return (*m_attachedAppearances)[static_cast<size_t>(index)]->getTransformName();
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::hideAttachment(int index, bool hide)
{
	NOT_NULL(m_attachedAppearances);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_attachedAppearances->size()));

	(*m_attachedAppearances)[static_cast<size_t>(index)]->setHidden(hide);
}

// ----------------------------------------------------------------------

bool SkeletalAppearance2::isAttachmentHidden(int index) const
{
	NOT_NULL(m_attachedAppearances);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_attachedAppearances->size()));

	return (*m_attachedAppearances)[static_cast<size_t>(index)]->isHidden();
}

// ----------------------------------------------------------------------

int SkeletalAppearance2::findAttachmentIndexByHardpoint(const CrcString &hardpointName) const
{
	const int attachmentCount = getAttachedAppearanceCount();

	for (int i = 0; i < attachmentCount; ++i)
	{
		if (getAttachedAppearanceTransformName(i) == hardpointName)
			return i;
	}

	//-- Didn't find it.
	return -1;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the Object instance attached to a specified hardpoint name.
 *
 * This is a helper function built on the existing interfaces.  It is
 * no faster than searching linearly through all attachments; however,
 * this could be sped up in the future if profiling warrants it.
 *
 * @param appearance     the skeletal appearance that will be checked.
 * @param hardpointName  the name of the hardpoint/transform to which
 *                       the desired object is attached.
 *
 * @return  the Object instance attatched to the specified hardpoint if
 *          one exists; otherwise, returns NULL.
 */

Object *SkeletalAppearance2::findAttachedObject(const CrcString &hardpointName)
{
	int const attachmentIndex = findAttachmentIndexByHardpoint(hardpointName);
	if (attachmentIndex < 0)
	{
		// No object attached to specified hardpoint.
		return 0;
	}
	else
	{
		// Found an attachment in the specified hardpoint name.  Return that object.
		return getAttachedObject(attachmentIndex);
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::wear(Object *object)
{
	//-- Check for NULL object.
	if (!object)
	{
		WARNING_STRICT_FATAL(true, ("attempted to wear a NULL object on SkeletalAppearance [%s].", getOwner()->getDebugName()));
		return;
	}

	//-- Check if wearables are locked.
	if (areWearablesLocked())
	{
		DEBUG_WARNING(true, ("wearables for wearer object id=[%s],template=[%s] are locked, ignoring ADD wearable id=[%s],template=[%s].",
			getOwner()->getNetworkId().getValueString().c_str(), getOwner()->getObjectTemplateName(),
			object->getNetworkId().getValueString().c_str(), object->getObjectTemplateName()));
		return;
	}

	//-- Ensure Object instance is a SkeletalAppearance2.
	Appearance *const baseAppearance      = object->getAppearance();
	SkeletalAppearance2 *const appearance = (baseAppearance != NULL) ? baseAppearance->asSkeletalAppearance2() : NULL;
	if (!appearance)
	{
		WARNING_STRICT_FATAL(true, ("attempted to wear object [%s] on SkeletalAppearance [%s] but wearable is null or non-skeletal.", object->getDebugName(), getOwner()->getDebugName()));
		return;
	}

	//-- Create the wearables container if it doesn't exist.
	if (!m_wornAppearanceObjects)
		m_wornAppearanceObjects = new WatcherObjectVectorVector;

	// Add appearance to list of worn appearances.
	m_wornAppearanceObjects->push_back();
	ObjectWatcherVectorPair & back = m_wornAppearanceObjects->back();
	back.first = object;

	{
		int const numWornChildObjects = object->getNumberOfChildObjects();
		for (int i = 0; i < numWornChildObjects; ++i)
		{
			Object * const child = object->getChildObject(i);
			if (NULL != child)
			{
				object->removeChildObject(child, Object::DF_none);
				getOwner()->addChildObject_p(child);
				back.second.push_back(ObjectWatcher(child));
			}
		}
	}

	//-- Remember that the appearance data (both skeleton and mesh) is now invalidated.
	// I'll rebuild later (lazy eval); this way, multiple wear/unwear calls can be executed
	// with a single appearance rebuild.
	markAsDirty();
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::stopWearing(const Object *object)
{
	Object * const ownerObject = getOwner();
	if (!ownerObject)
		return;

	//-- Check for NULL object.
	if (!object)
	{
		WARNING_STRICT_FATAL(true, ("unwear failed: object arg in NULL."));
		return;
	}

	//-- Check if wearables are locked.
	if (areWearablesLocked())
	{
		DEBUG_WARNING(true, ("wearables for wearer object id=[%s],template=[%s] are locked, ignoring REMOVE wearable id=[%s],template=[%s].",
			getOwner()->getNetworkId().getValueString().c_str(), getOwner()->getObjectTemplateName(),
			object->getNetworkId().getValueString().c_str(), object->getObjectTemplateName()));
		return;
	}


	//-- Check for no worn objects.
	if (!m_wornAppearanceObjects)
	{
		WARNING_STRICT_FATAL(true, ("unwear failed: no items worn."));
		return;
	}

	//-- Find the worn appearance object element corresponding to object.
	const WatcherObjectVectorVector::iterator endIt = m_wornAppearanceObjects->end();
	for (WatcherObjectVectorVector::iterator it = m_wornAppearanceObjects->begin(); it != endIt; ++it)
	{
		ObjectWatcher & watcher = (*it).first;

		if (watcher.getPointer() == object)
		{
			WatcherObjectVector & childObjects = (*it).second;

			for (WatcherObjectVector::iterator cit = childObjects.begin(); cit != childObjects.end(); ++cit)
			{
				Object * const child = (*cit);
				if (child)
				{
					ownerObject->removeChildObject(child, Object::DF_none);
					const_cast<Object *>(object)->addChildObject_p(child);
				}
			}

			// Found it, remove the worn object.
			IGNORE_RETURN(m_wornAppearanceObjects->erase(it));

			// Mark object as dirty.
			markAsDirty();

			//-- Return to caller.
			return;
		}
	}

	//-- Object was not worn.
	WARNING_STRICT_FATAL(true, ("unwear failed: object [%s] not worn.", object->getDebugName()));
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::lockWearables()
{
	m_wearablesLockedFrame = Graphics::getFrameNumber();
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::unlockWearables()
{
	m_wearablesLockedFrame = 0;
}

// ----------------------------------------------------------------------

bool SkeletalAppearance2::areWearablesLocked() const
{
	return m_wearablesLockedFrame > 0 && m_wearablesLockedFrame < Graphics::getFrameNumber();
}

// ----------------------------------------------------------------------

int SkeletalAppearance2::getWearableCount() const
{
	if (m_wornAppearanceObjects)
	{
		const_cast<SkeletalAppearance2*>(this)->removeDeletedWearables();
		return static_cast<int>(m_wornAppearanceObjects->size());
	}
	else
		return 0;
}

// ----------------------------------------------------------------------

const SkeletalAppearance2 *SkeletalAppearance2::getWearableAppearance(int index) const
{
	// make sure caller checked getWearableCount() first
	NOT_NULL(m_wornAppearanceObjects);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_wornAppearanceObjects->size()));

	const Object *const object = (*m_wornAppearanceObjects)[static_cast<size_t>(index)].first;
	Appearance const *baseAppearance = (object != NULL) ? object->getAppearance() : NULL;
	return (baseAppearance != NULL) ? baseAppearance->asSkeletalAppearance2() : NULL;
}

// ----------------------------------------------------------------------

SkeletalAppearance2 *SkeletalAppearance2::getWearableAppearance(int index)
{
	// make sure caller checked getWearableCount() first
	NOT_NULL(m_wornAppearanceObjects);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_wornAppearanceObjects->size()));

	ObjectWatcher & watcher = (*m_wornAppearanceObjects)[static_cast<size_t>(index)].first;
	Object *const object = watcher.getPointer();
	Appearance *baseAppearance = (object != NULL) ? object->getAppearance() : NULL;
	return (baseAppearance != NULL) ? baseAppearance->asSkeletalAppearance2() : NULL;
}

// ----------------------------------------------------------------------
/**
 * Return an approximation to the mesh extent.
 *
 * Warning: do not use this for typical appearance extent processing.
 * Use Appearance::getExtent() instead.
 *
 * If the mesh extent has never been calculated, or if new wearables
 * have been added or removed, the extent will be calculated 
 * during this call.  Otherwise, it will return the last calculated 
 * exact mesh extent for this instance.
 *
 * Use this routine if you need an extent for the mesh that needs to be
 * relatively close to the real mesh extent, but doesn't need to be exact.
 * This routine on average will be significantly less CPU intensive than
 * getExactMeshExtent().
 *
 * If you are looking for traditional Appearance extents, use that
 * mechanism as it will make use of the skeleton extents, far cheaper
 * to calculate than the true mesh extents.
 *
 * This function is non-const because to implement it correctly, it may
 * be forced to rebuild the mesh, most definitely not a const operation.
 *
 * @return  A box extent for the mesh data associated with this
 *          appearance instance.
 */

const BoxExtent &SkeletalAppearance2::getApproximateMeshExtent()
{
	bool calculateExtentNow;

	if (!m_meshExtent)
	{
		// extent doesn't exist
		calculateExtentNow = true;
	}
	else if (m_appearanceDirty[static_cast<BoolVector::size_type>(m_displayLodIndex)])
	{
		// mesh data has changed, time to recalculate mesh extents
		calculateExtentNow = true;
	}
	else
	{
		// extent doesn't need to be calculated
		calculateExtentNow = false;
	}

	if (calculateExtentNow)
		return getExactMeshExtent();
	else
	{
		if (m_extentDelegateObject.getPointer ())
			return getDelegateBoxExtent (true);

		NOT_NULL(m_meshExtent);
		return *m_meshExtent;
	}
}

// ----------------------------------------------------------------------
/**
 * Return the exact extents of the mesh data associated with this
 * appearance at the current transform settings.
 *
 * Warning: do not use this for typical appearance extent processing.
 * Use Appearance::getExtent() instead.
 *
 * Note: this routine accounts for wearables but does not account
 * for attachments.  If the caller needs attachment information, the caller
 * will need to handle that.
 *
 * If there are no wearables and no base meshes associated with this
 * appearance, it will return an extent centered on the local origin
 * with a 0.01 unit radius.
 *
 * This function is non-const because to implement it correctly, it may
 * be forced to rebuild the mesh, most definitely not a const operation.
 *
 * @return  A box extent for the mesh data associated with this
 *          appearance instance.
 */

const BoxExtent &SkeletalAppearance2::getExactMeshExtent()
{
	//-- Rebuild the mesh as necessary.
	bool const isReady = rebuildIfDirtyAndAvailable();

	//-- create mesh extent variable as necessary
	if (!m_meshExtent)
	{
		m_meshExtent = new BoxExtent();
		NOT_NULL(m_meshExtent); // lint needs this!
	}

	const ShaderPrimitiveVector *const shaderPrimitives = (isReady && !m_perLodShaderPrimitives->empty() ? &getDisplayLodShaderPrimitives() : 0);
	if (!shaderPrimitives || shaderPrimitives->empty())
	{
		if(m_displayLodIndex > 0)
		{
			m_displayLodIndex--;
			getExactMeshExtent();
			m_displayLodIndex++;
		}
		else
		{
			// no meshes, return an extent centered on local origin
			m_meshExtent->set(Vector::zero, Vector::zero, Vector::zero, 1.00f);
		}	
	}
	else
	{
		//-- initialize min and max vector
		Vector minVector(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		Vector maxVector(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

		Vector localMin;
		Vector localMax;

		//-- apply each blended shader primitive to min and max
		const ShaderPrimitiveVector::const_iterator endIt = shaderPrimitives->end();
		for (ShaderPrimitiveVector::const_iterator it = shaderPrimitives->begin(); it != endIt; ++it)
		{
			const ShaderPrimitive *baseShaderPrimitive = *it;
			const SoftwareBlendSkeletalShaderPrimitive *const shaderPrimitive = (baseShaderPrimitive != NULL) ? baseShaderPrimitive->asSoftwareBlendSkeletalShaderPrimitive() : NULL;

			if (!shaderPrimitive)
				continue;

			//-- calculate shader primitive extent
			shaderPrimitive->calculateBoundingBox(localMin, localMax);

			//-- update min and max
			minVector.set(std::min(minVector.x, localMin.x), std::min(minVector.y, localMin.y), std::min(minVector.z, localMin.z));
			maxVector.set(std::max(maxVector.x, localMax.x), std::max(maxVector.y, localMax.y), std::max(maxVector.z, localMax.z));
		}
		
		//-- set extent min and max
		m_meshExtent->setMin(minVector);
		m_meshExtent->setMax(maxVector);

		//-- calculate sphere center and radius
		m_meshExtent->calculateCenterAndRadius();
	}

	if (m_extentDelegateObject.getPointer ())
		return getDelegateBoxExtent (true);

	//-- return the extent
	return *m_meshExtent;
}

// ----------------------------------------------------------------------
/**
 * Override the transforms for the local skeleton only during the next render
 * with the transforms provided by the overriding skeleton.
 *
 * Overriding the skeleton associated with this instance will only
 * occur for the following addToCameraScene() call.  The override
 * will cease to take place thereafter.
 *
 * The intersection of identically-named transforms will be copied from
 * the source skeleton to the destination skeleton.  Transforms on this
 * instance falling outside of the intersection will receive the same
 * values they would have without this call.
 *
 * This is an expensive operation.  Do not perform this unless this is
 * both a rare occurrence and cannot be achieved any other way.  It is
 * cheaper to play the same animation files on two skeletons than to
 * try to copy over the skeleton transforms.
 *
 * @param overridingSkeleton  the skeleton providing the source transforms
 *                            that will be duplicated to the local skeleton.
 */

void SkeletalAppearance2::overrideSkeletonNextRender(const SkeletonVector &overridingSkeletons)
{
	if (overridingSkeletons.empty())
		return;

	m_overridingAnimationResolver = &(overridingSkeletons.front()->getAnimationResolver());
	NOT_NULL(m_overridingAnimationResolver);
}

// ----------------------------------------------------------------------
/**
 * Fetch a reference for and retrieve the CustomizationData associated
 * with this Appearance.
 *
 * @return  the CustomizationData instance associated with this
 *          Appearance instance.  It will return NULL if no
 *          CustomizationData instance is associated with the
 *          Appearance.
 */

CustomizationData *SkeletalAppearance2::fetchCustomizationData()
{
	if (m_customizationData)
		m_customizationData->fetch();

	return m_customizationData;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the const Skeleton associated with the current display LOD.
 *
 * @return  the const Skeleton associated with the current display LOD;
 *          NULL if the appearance is currently asynchronously loading
 *          the display LOD detail level.
 */

const Skeleton *SkeletalAppearance2::getDisplayLodSkeleton() const
{
	//-- Ensure display LOD index is valid.
	if (m_maxAvailableDetailLevelIndex < 0)
	{
		// There won't be a skeleton to return in this case.
		return 0;
	}

	if (m_displayLodIndex < 0 || m_displayLodIndex >= static_cast<int>(m_skeletons->size()))
	{
		DEBUG_WARNING(true, ("m_displayLodIndex out of range [0..%i] for %s", static_cast<int>(m_skeletons->size()) - 1, getAppearanceTemplate()->getName()));
		m_displayLodIndex = 0;
	}

	//-- Ensure the display LOD level is build.
	bool const isReady = const_cast<SkeletalAppearance2*>(this)->rebuildIfDirtyAndAvailable();
	if (!isReady)
		return 0;

	//-- Get the skeleton template.
	Skeleton *const skeleton = (*m_skeletons)[static_cast<size_t>(m_displayLodIndex)];
	NOT_NULL(skeleton);

	return skeleton;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the Skeleton associated with the current display LOD.
 *
 * @return  the Skeleton associated with the current display LOD;
 *          NULL if the appearance is currently asynchronously loading
 *          the display LOD detail level.
 */

Skeleton *SkeletalAppearance2::getDisplayLodSkeleton()
{
	//-- Ensure display LOD index is valid.
	if (m_maxAvailableDetailLevelIndex < 0)
	{
		// There won't be a skeleton to return in this case.
		return 0;
	}

	if (m_displayLodIndex < 0 || m_displayLodIndex >= static_cast<int>(m_skeletons->size()))
	{
		DEBUG_WARNING(true, ("m_displayLodIndex out of range [0..%i] for %s", static_cast<int>(m_skeletons->size()) - 1, getAppearanceTemplate()->getName()));
		m_displayLodIndex = 0;
	}

	//-- Ensure the display LOD level is build.
	bool const isReady = rebuildIfDirtyAndAvailable();
	if (!isReady)
		return 0;

	//-- Get the skeleton template.
	Skeleton *const skeleton = (*m_skeletons)[static_cast<size_t>(m_displayLodIndex)];
	NOT_NULL(skeleton);

	return skeleton;
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::setDesiredVelocity(const Vector &velocityRelativeToSelf)
{
#ifdef _DEBUG
	if (_isnan(static_cast<double>(velocityRelativeToSelf.x)) || _isnan(static_cast<double>(velocityRelativeToSelf.y)) || _isnan(static_cast<double>(velocityRelativeToSelf.z)))
	{
		DEBUG_WARNING(true, ("Velocity set is not-a-number, setting to zero [%g, %g, %g].", velocityRelativeToSelf.x, velocityRelativeToSelf.y, velocityRelativeToSelf.z));
		m_animationLocomotionVelocity = Vector::zero;
		return;
	}
#endif

	m_animationLocomotionVelocity = velocityRelativeToSelf;
}

// ----------------------------------------------------------------------

Vector const &SkeletalAppearance2::getDesiredVelocity() const
{
	return m_animationLocomotionVelocity;
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::getObjectLocomotion(Quaternion &rotation, Vector &translation, float elapsedTime) const
{
	m_animationResolver->getObjectLocomotion(rotation, translation, elapsedTime);
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::updateTargetPosition()
{
	Object *ownerObject = getOwner();
	if (!ownerObject)
	{
		return;
	}

	if (m_targetObject && (ownerObject != m_targetObject))
	{
		//-- Set target position to be at the object position (i.e. the pivot or master node position).
		//   In the past I used the center of targetObject's extents; however, this
		//   caused the pointing appearance to perceive a different target point when the target
		//   did any pure yaw movements.  Since this was undesired, I flipped it back to use the
		//   simpler Object pivot point.
		bool getTargetPosition_w = true;

		if (m_targetHardpoint != CrcLowerString::empty)
		{
			Appearance const * const          baseAppearance = m_targetObject->getAppearance();
			SkeletalAppearance2 const * const appearance     = (baseAppearance != NULL) ? baseAppearance->asSkeletalAppearance2() : NULL;
			
			if (appearance)
			{
				Transform hardpointTransform;
				if (appearance->findHardpoint(m_targetHardpoint, hardpointTransform))
				{
					m_targetPosition_w = m_targetObject->rotateTranslate_o2w(hardpointTransform.getPosition_p());
					getTargetPosition_w = false;
					m_targetPositionValid = true;
				}
			}
		}

		if (getTargetPosition_w)
		{
			m_targetPosition_w = m_targetObject->getAppearanceSphereCenter_w();
			m_targetPositionValid = true;
		}
	}

	if (m_targetPositionValid) 
	{
		//-- Get target position in object space.
		m_targetPosition_o = ownerObject->rotateTranslate_w2o(m_targetPosition_w);
		m_directionToTarget = m_targetPosition_o;
		
		//-- Normalize direction.
		IGNORE_RETURN(m_directionToTarget.normalize());
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::setTargetObjectAndHardpoint(const Object *targetObject, CrcLowerString const &hardpointName)
{
	m_targetObject = targetObject;
	m_targetHardpoint = hardpointName;
	m_targetPositionValid = false;

	updateTargetPosition();
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::setTargetObject(const Object *targetObject)
{
	m_targetObject = targetObject;
	m_targetHardpoint = CrcLowerString::empty;
	m_targetPositionValid = false;
	updateTargetPosition();
}

// ----------------------------------------------------------------------

Object const *SkeletalAppearance2::getTargetObject() const
{
	return m_targetObject;
}

// ----------------------------------------------------------------------
/**
 * Set the yaw direction for the object.
 *
 * This value gets set into the AnimationEnvironment and is possibly
 * used by the animation controller to do things such as playing
 * a shuffle left or shuffle right animation when rotating.
 *
 * @param direction  set to -1 if rotating in the negative direction around 
 *                   the object's local y axis (yaw left), 0 if not rotating, or +1 if
 *                   rotating in the positive y axis (yaw right).
 */

void SkeletalAppearance2::setYawDirection(int direction)
{
	if ((direction < -1) || (direction > 1))
	{
		DEBUG_WARNING(true, ("yaw direction [%d] outside valid range -1 .. 1, ignoring.", direction));
		return;
	}

	m_yawDirection = direction;
}

// ----------------------------------------------------------------------

bool SkeletalAppearance2::hasAsgController() const
{
	return getSkeletalAppearanceTemplate().hasAsgController();
}

// ----------------------------------------------------------------------

bool SkeletalAppearance2::hasAshController() const
{
	return getSkeletalAppearanceTemplate().hasAshController();
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::incrementDetailLevel()
{
	setDetailLevel(m_displayLodIndex + 1);
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::decrementDetailLevel()
{
	setDetailLevel(m_displayLodIndex - 1);
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::setDetailLevel(int const index)
{
	int const newIndex = std::min(std::max(index, 0), m_maxAvailableDetailLevelIndex); 

	if (newIndex != m_displayLodIndex)
	{
		m_displayLodIndex = newIndex;
		rebuildIfDirtyAndAvailable(m_displayLodIndex);
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::clearAllTransformModifiers()
{
	//-- Clear the list of attached modifiers.
	if (m_attachedTransformModifiers)
	{
		std::for_each(m_attachedTransformModifiers->begin(), m_attachedTransformModifiers->end(), PointerDeleter());
		m_attachedTransformModifiers->clear();
	}

	//-- Clear transform modifiers from all skeletons.
	SkeletonVector::iterator const endIt = m_skeletons->end();
	for (SkeletonVector::iterator it = m_skeletons->begin(); it != endIt; ++it)
	{
		Skeleton *const skeleton = *it;
		if (skeleton)
			skeleton->clearAllTransformModifiers();
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::addTransformModifierTakeOwnership(CrcString const &transformName, TransformModifier *modifier)
{
	NOT_NULL(modifier);

	//-- Create container as necessary.
	if (!m_attachedTransformModifiers)
		m_attachedTransformModifiers = new AttachedTransformModifierVector;

	//-- Track the attached modifier since we need to reattach when we create/rebuild skeletons.
	m_attachedTransformModifiers->push_back(new AttachedTransformModifier(transformName, modifier));

	//-- Attach modifier to all existing skeletons.
	SkeletonVector::iterator const endIt = m_skeletons->end();
	for (SkeletonVector::iterator it = m_skeletons->begin(); it != endIt; ++it)
	{
		Skeleton *const skeleton = *it;
		if (skeleton)
		{
			//-- Check if this LOD skeleton has the specified transform name.
			bool  foundIt        = false;
			int   transformIndex = -1;
			skeleton->findTransformIndex(transformName, &transformIndex, &foundIt);

			if (foundIt)
			{
				//-- Attach the modifier.
				skeleton->attachTransformModifier(transformIndex, modifier);
			}
		}
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::setPlannedLodIndex(int plannedLodIndex)
{
	if (m_maxAvailableDetailLevelIndex < 0)
	{
		DEBUG_WARNING(true, ("SkeletalAppearance2::setPlannedLodIndex(%d) called when max avialable detail level is [%d], ignoring.", plannedLodIndex, m_maxAvailableDetailLevelIndex));
		return;
	}

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, plannedLodIndex, getDetailLevelCount());
	m_plannedLodIndex          = plannedLodIndex;
	m_plannedLodSetFrameNumber = Os::getNumberOfUpdates();
}

// ----------------------------------------------------------------------

int SkeletalAppearance2::getPlannedLodIndex() const
{
	return m_plannedLodIndex;
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::setEveryOtherFrameSkinningEnabled(bool enabled)
{
	m_everyOtherFrameSkinningEnabled = enabled;
}

// ----------------------------------------------------------------------

bool SkeletalAppearance2::getEveryOtherFrameSkinningEnabled() const
{
	return m_everyOtherFrameSkinningEnabled;
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::setForceHardSkinningEnabled(bool enabled)
{
	m_forceHardSkinningEnabled = enabled;
}

// ----------------------------------------------------------------------

bool SkeletalAppearance2::getForceHardSkinningEnabled() const
{
	return m_forceHardSkinningEnabled;
}

// ======================================================================
// class SkeletalAppearance2: PRIVATE STATIC
// ======================================================================

void SkeletalAppearance2::onScaleModified(Vector const &oldScale, Vector const &newScale)
{
	//-- Chain down.
	Appearance::onScaleModified(oldScale, newScale);

	//-- Use the scale's y component as the uniform scaling component.
	if (newScale.y <= 0.0f)
	{
		DEBUG_WARNING(true, ("object id=[%s]: tried to scale skeletal appearance to invalid scale [%g].", (getOwner() != NULL) ? getOwner()->getNetworkId().getValueString().c_str() : "<null owner>", newScale.y));
		return;
	}

	//-- Save the newScale.
	m_scale = newScale.y;

	//-- Set each skeleton's newScale.
	std::for_each(m_skeletons->begin(), m_skeletons->end(), VoidBindSecond(VoidMemberFunction(&Skeleton::setScale), m_scale));
}

// ======================================================================
// class SkeletalAppearance2: PRIVATE STATIC
// ======================================================================

void SkeletalAppearance2::remove()
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	ms_installed = false;

#if PRODUCTION == 0
	//-- Unregister debug flags.
	DebugFlags::unregisterFlag(s_disableSkeletalAppearanceRendering);
	DebugFlags::unregisterFlag(s_reportRenderStatistics);
#ifdef _DEBUG
	DebugFlags::unregisterFlag(ms_renderTargetDirection);
#endif
	DebugFlags::unregisterFlag(s_logReadyIssues);
	DebugFlags::unregisterFlag(s_showSkeleton);
	DebugFlags::unregisterFlag(s_disableBatcher);
	DebugFlags::unregisterFlag(s_disableCollisionChecking);
	DebugFlags::unregisterFlag(s_disableSphereTreeUpdates);
	DebugFlags::unregisterFlag(s_reportAllocations);
	DebugFlags::unregisterFlag(s_reportRebuildMeshStatistics);
#endif

	AttachedAppearance::remove();

	delete ms_compositeMesh;
	ms_compositeMesh = 0;

	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::getAppearanceTemplateSkeletons(const SkeletalAppearanceTemplate &appearanceTemplate, SkeletonSegmentDescriptorSet &descriptorSet, SkeletonSegmentDescriptorVector &descriptorVector)
{
	const int skeletonTemplateCount = appearanceTemplate.getSkeletonTemplateCount();
	for (int i = 0; i < skeletonTemplateCount; ++i)
	{
		const SkeletalAppearanceTemplate::SkeletonTemplateInfo &sti = appearanceTemplate.getSkeletonTemplateInfo(i);
		const CrcLowerString &skeletonTemplateName                  = appearanceTemplate.getSkeletonTemplateName(sti);
		const CrcLowerString &attachmentTransformName               = appearanceTemplate.skeletonTemplateHasAttachmentTransform(sti) ? appearanceTemplate.getSkeletonTemplateAttachmentTransformName(sti) : CrcLowerString::empty;

		SkeletonSegmentDescriptorSet::value_type  newValue(skeletonTemplateName, attachmentTransformName);
		SkeletonSegmentDescriptorSet::iterator    lowerBound = descriptorSet.lower_bound(newValue);

		if ((lowerBound == descriptorSet.end()) || descriptorSet.key_comp()(newValue, *lowerBound))
		{
			// doesn't exist, add
			IGNORE_RETURN(descriptorSet.insert(lowerBound, newValue));
			descriptorVector.push_back(newValue);
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Fetch all the MeshGenerator instances owned by this instance for a given detail level.
 *
 * @param appearanceTemplate  the AppearanceTemplate for which owned MeshGenerators
 *                            are being collected.
 * @param lodIndex            the detail level index of the MeshGenerator instances to be retrieved.
 * @param meshGenerators      the newly created MeshGenerator instances are appended to this vector.
 */

void SkeletalAppearance2::fetchOwnedMeshGeneratorsForDetailLevel(const SkeletalAppearanceTemplate &appearanceTemplate, int lodIndex, MeshGeneratorVector &meshGenerators)
{
	//-- Get the # MeshGeneratorTemplate instances associated with this appearance.
	const int meshGeneratorCount = appearanceTemplate.getMeshGeneratorCount();

	//-- Load each MeshGeneratorTemplate instance and create the appropriate MeshGenerator if it exists at the specified LOD.
	for (int i = 0; i < meshGeneratorCount; ++i)
	{
		//-- Fetch the MeshGeneratorTemplate instance.
		const MeshGeneratorTemplate *const mgTemplate = MeshGeneratorTemplateList::fetch(appearanceTemplate.getMeshGeneratorName(i));
		if (!mgTemplate)
		{
			//-- Skip unavailable mesh generators.
			continue;
		}

		//-- Get # detail levels for this particular MeshGeneratorTemplate.
		const int mgLodCount = mgTemplate->getDetailCount();
		if (mgLodCount > lodIndex)
		{
			// Get the BasicMeshGeneratorTemplate for this LOD.
			const BasicMeshGeneratorTemplate *const bmgTemplate = mgTemplate->fetchBasicMeshGeneratorTemplate(lodIndex);
			if (bmgTemplate)
			{
				// Create the MeshGenerator for this LOD, keeping track of the last non-null MeshGenerator instance created from this template.
				MeshGenerator *meshGenerator = bmgTemplate->createMeshGenerator();
				if (meshGenerator)
				{
					if (m_customizationData)
						meshGenerator->setCustomizationData(m_customizationData);

					meshGenerator->registerModificationHandler(meshGeneratorModifiedCallback, this);
					meshGenerators.push_back(meshGenerator);
				}

				bmgTemplate->release();
			}
		}
	
		//-- Release local resources.
		mgTemplate->release();
	}
} //lint !e1762 // could be made const // yes, but only through slight of hand --- "this" context is later converted to non-const pointer

// ----------------------------------------------------------------------
/**
 * Return the cached mesh generators owned by this instance, filling the
 * cache if necessary.
 *
 * These should not be released, they are owned by this appearance instance.
 */

SkeletalAppearance2::MeshGeneratorVector const &SkeletalAppearance2::getOwnedMeshGeneratorsForDetailLevel(int lodIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, static_cast<int>(m_ownedMeshGenerators->size()));
	MeshGeneratorVector const &meshGenerators = (*m_ownedMeshGenerators)[static_cast<MeshGeneratorVector::size_type>(lodIndex)];

	//-- Check if mesh generators have been fetched already.
	if (!meshGenerators.empty())
	{
		// Yes, return them.
		return meshGenerators;
	}

	//-- Fetch the mesh generators for this LOD.
	SkeletalAppearanceTemplate const *const saTemplate = safe_cast<SkeletalAppearanceTemplate const*>(getAppearanceTemplate());
	if (!saTemplate)
	{
		// Nothing to cache.
		return meshGenerators;
	}

	const_cast<SkeletalAppearance2*>(this)->fetchOwnedMeshGeneratorsForDetailLevel(*saTemplate, lodIndex, const_cast<MeshGeneratorVector&>(meshGenerators));

	return meshGenerators;
}

// ----------------------------------------------------------------------
/**
 * Check if the mesh generators for the specified detail level are ready
 * on this appearance and all attached appearances.
 */

bool SkeletalAppearance2::areAllMeshGeneratorsReadyForDetailLevel(int lodIndex) const
{
	if ((lodIndex < 0) || (lodIndex >= static_cast<int>(m_ownedMeshGenerators->size())))
	{
		//-- Nothing to construct, so we're ready for this detail level.
		return true;
	}

	//-- Check this appearance's owned mesh generators first.
	MeshGeneratorVector const &meshGenerators = getOwnedMeshGeneratorsForDetailLevel(lodIndex);
	MeshGeneratorVector::const_iterator const endIt = meshGenerators.end();
	for (MeshGeneratorVector::const_iterator it = meshGenerators.begin(); it != endIt; ++it)
	{
		if ((*it != NULL) && !(*it)->isReadyForUse())
		{
			//-- Found a mesh generator that's not ready for use.
			return false;
		}
	}

	//-- Check worn items.
	if (m_wornAppearanceObjects)
	{
		for (WatcherObjectVectorVector::iterator objectIt = m_wornAppearanceObjects->begin(); objectIt != m_wornAppearanceObjects->end();)
		{
			ObjectWatcher & watcher = (*objectIt).first;

			//-- Get the skeletal appearance.
			const Object *const object = watcher.getPointer();
			if (!object)
				objectIt = m_wornAppearanceObjects->erase(objectIt);
			else
			{
				Appearance const          *const baseAppearance = object->getAppearance();
				SkeletalAppearance2 const *const appearance     = (baseAppearance != NULL) ? baseAppearance->asSkeletalAppearance2() : NULL;
				NOT_NULL(appearance);

				//-- Incorporate wearable appearance's mesh data.
				if (!appearance->areAllMeshGeneratorsReadyForDetailLevel(lodIndex))
				{
					//-- Found one worn item that isn't ready yet to construct, so we're not yet ready.
					return false;
				}

				++objectIt;
			}
		}
	}

	//-- All mesh generators are ready for use.
	return true;
}

// ----------------------------------------------------------------------

bool SkeletalAppearance2::areShaderPrimitivesReadyForDetailLevel(int lodIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, static_cast<int>(m_perLodShaderPrimitives->size()));
	ShaderPrimitiveVector &spVector = (*m_perLodShaderPrimitives)[static_cast<ShaderPrimitiveVectorVector::size_type>(lodIndex)];

	ShaderPrimitiveVector::iterator const endIt = spVector.end();
	for (ShaderPrimitiveVector::iterator it = spVector.begin(); it != endIt; ++it)
	{
		ShaderPrimitive const *const shaderPrimitive = *it;
		if (shaderPrimitive && !shaderPrimitive->isReady())
			return false;
	}

	//-- All existing shader primitives for this LOD are ready.
	return true;
}

// ----------------------------------------------------------------------
/**
 * Callback invoked when one of the MeshGenerator instances contributing
 * to this instance is modified.
 *
 * @param context        the associated SkeletalAppearance2 instance.
 * @param meshGenerator  the meshGenerator instance that is modified.
 */

void SkeletalAppearance2::meshGeneratorModifiedCallback(const void *context, MeshGenerator & /* meshGenerator */)
{
	NOT_NULL(context);

	//-- convert context to SkeletalAppearance instance
	SkeletalAppearance2 *const appearance = const_cast<SkeletalAppearance2*>(static_cast<const SkeletalAppearance2*>(context));

	//-- tell the appearance that a mesh generator was modified
	appearance->notifyMeshGeneratorModified();
}

// ======================================================================

void SkeletalAppearance2::lookupAttachmentTransformIndices(int detailLevel)
{
	if (!m_attachedAppearances)
		return;

	bool deletedAnyAttachments = false;

	for (AttachedAppearanceVector::iterator it = m_attachedAppearances->begin(); it != m_attachedAppearances->end();)
	{
		//-- get AttachedAppearance instance
		NOT_NULL(*it);
		AttachedAppearance *attachedAppearance = *it;

		//-- get detail level's skeleton
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, detailLevel, static_cast<int>(m_skeletons->size()));
		Skeleton *const skeleton = (*m_skeletons)[static_cast<size_t>(detailLevel)];

		if (attachedAppearance->lookupTransformIndex(detailLevel, *skeleton))
		{
			// lookup succeeded
			++it;
		}
		else
		{
			WARNING_STRICT_FATAL(true, ("appearance (0x%08x) attached to no-longer-existing transform [%s]\n", attachedAppearance->getAppearance(), attachedAppearance->getTransformName().getString()));
		
			// lookup failed, remove it
			delete attachedAppearance;
			it = m_attachedAppearances->erase(it);
			deletedAnyAttachments = true;
		}
	}

	if (deletedAnyAttachments && s_destroyedAttachmentWearableCallback)
	{
		Object *const object = getOwner();
		if (object)
			(*s_destroyedAttachmentWearableCallback)(*object);
	}
}

// ----------------------------------------------------------------------
/**
 * Populate the compositeMesh with all MeshGenerator objects from this
 * base appearance plus those from the worn appearances.
 */

void SkeletalAppearance2::buildCompositeMesh(CompositeMesh &compositeMesh, int lodIndex) const
{
	DEBUG_FATAL(lodIndex < 0, ("invalid LOD index [%d].", lodIndex));

	//-- add all locally owned mesh generators
	if (!m_ownedMeshGenerators->empty() && (lodIndex < static_cast<int>(m_ownedMeshGenerators->size())))
	{
		MeshGeneratorVector const &meshGenerators = getOwnedMeshGeneratorsForDetailLevel(lodIndex);

		const MeshGeneratorVector::const_iterator endIt = meshGenerators.end();
		for (MeshGeneratorVector::const_iterator it = meshGenerators.begin(); it != endIt; ++it)
		{
			// Add mesh generator owned by this appearance, using this appearance's customization data.
			compositeMesh.addMeshGenerator(*it, m_customizationData);
		}
	}

	//-- For each wearable, add its MeshGenerator instances using the wearable's customization data.
	if (m_wornAppearanceObjects)
	{
		for (WatcherObjectVectorVector::iterator objectIt = m_wornAppearanceObjects->begin(); objectIt != m_wornAppearanceObjects->end();)
		{
			//-- Get the skeletal appearance.
			ObjectWatcher & watcher = (*objectIt).first;
			const Object *const object = watcher.getPointer();
			if (!object)
			{
				objectIt = m_wornAppearanceObjects->erase(objectIt);
			}
			else
			{
				const SkeletalAppearance2 *const appearance = safe_cast<const SkeletalAppearance2*>(object->getAppearance());
				NOT_NULL(appearance);

				//-- Incorporate wearable appearance's mesh data.
				if(object->getShouldBakeIntoMesh())
					appearance->buildCompositeMesh(compositeMesh, lodIndex);
				
				++objectIt;
			}
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Add all skeleton segments required by this base appearance plus those
 * from the worn appearances.
 *
 * This function assumes it is called between a Skeleton::beginSkeletonModification() /
 * Skeleton::endSkeletonModification() pair.  It will fail if this restriction
 * is not obeyed.
 *
 * This function takes care of removing any existing segments from the current
 * skeleton.  It will do so after fetching each of the newly-required SkeletonTemplate
 * objects so as to prevent a SkeletonTemplate from being released from the system,
 * only to be reloaded immediately afterward.
 */

void SkeletalAppearance2::addSkeletonSegments(Skeleton &skeleton, int lodIndex)
{
	//-- build the unique set of <skeleton template, attachment transform name> entries.
	//   Find this information from the SkeletalAppearanceTemplate associated with this
	//   appearance and the SkeletalAppearanceTemplates associated with each wearable
	//   currently worn.
	//
	//   we use the set to guarantee uniqueness.  we use the vector to guarantee
	//   a safe order in which to attach skeleton segments to the skeleton.  We
	//   must attach in such an order that a segment's attachment point, if non-null,
	//   must exist before a segment attaches to it.

	SkeletonSegmentDescriptorVector  descriptorVector;
	SkeletonSegmentDescriptorSet     descriptorSet;

	// get skeleton segments from this appearance template
	{
		const SkeletalAppearanceTemplate *const appearanceTemplate = safe_cast<const SkeletalAppearanceTemplate*>(getAppearanceTemplate());
		if (!appearanceTemplate)
			return;

		getAppearanceTemplateSkeletons(*appearanceTemplate, descriptorSet, descriptorVector);
	}

	// get skeleton segments from wearables
	if (m_wornAppearanceObjects)
	{
		for (WatcherObjectVectorVector::iterator it = m_wornAppearanceObjects->begin(); it != m_wornAppearanceObjects->end();)
		{
			ObjectWatcher & watcher = (*it).first;
			const Object *const object = watcher.getPointer();
			if (!object)
				it = m_wornAppearanceObjects->erase(it);
			else
			{
				const SkeletalAppearance2 *appearance = safe_cast<const SkeletalAppearance2*>(object->getAppearance());
				NOT_NULL(appearance);

				const SkeletalAppearanceTemplate *const appearanceTemplate = safe_cast<const SkeletalAppearanceTemplate*>(appearance->getAppearanceTemplate());
				NOT_NULL(appearanceTemplate);

				getAppearanceTemplateSkeletons(*appearanceTemplate, descriptorSet, descriptorVector);

				++it;
			}
		}
	}

	//-- Optionally perform sanity checking on the descriptorVector.
	if (ConfigClientSkeletalAnimation::getSkeletonSegmentSanityCheckerEnabled())
	{
		SkeletonSegmentDescriptor::validate(descriptorVector, *this);
	}

	//-- fetch skeleton templates
	typedef std::vector<const SkeletonTemplate*>  SkeletonTemplateVector;

	SkeletonTemplateVector skeletonTemplates;

	skeletonTemplates.reserve(descriptorVector.size());

	{
		const SkeletonSegmentDescriptorVector::const_iterator endIt = descriptorVector.end();
		for (SkeletonSegmentDescriptorVector::const_iterator it = descriptorVector.begin(); it != endIt; ++it)
			skeletonTemplates.push_back(SkeletonTemplateList::fetch(it->getSkeletonTemplateName()));
	}
	DEBUG_FATAL(skeletonTemplates.size() != descriptorVector.size(), ("logic error"));

	//-- Tell animation resolver about the skeleton templates.  This is where the animation system
	//   finds out about which .skt files are new, stay, or go away.
	m_animationResolver->specifySkeletonTemplates(skeletonTemplates);

	//-- Remove all existing skeleton segments.
	skeleton.removeAllSegments();

	//-- Add all new segments.
	const size_t segmentCount = descriptorVector.size();
	for (size_t i = 0; i < segmentCount; ++i)
	{
		//-- get attachment skeleton template and attachment point
		const SkeletonSegmentDescriptor &ssd              = descriptorVector[i];
		const SkeletonTemplate          *skeletonTemplate = skeletonTemplates[i];
		NOT_NULL(skeletonTemplate);

		//-- Apply SkeletonTemplate segment to Skeleton if there is anything to apply at this LOD level.
		//   e.g. a human male face may drop off its entire SkeletonTemplate data after the first one or two LODs.
		if (lodIndex < skeletonTemplate->getDetailCount())
		{
			const BasicSkeletonTemplate *const basicSkeletonTemplate = skeletonTemplate->fetchBasicSkeletonTemplate(lodIndex);
			NOT_NULL(basicSkeletonTemplate);

			//-- Attach this LOD's skeleton segment data.
			IGNORE_RETURN(skeleton.attachSkeletonSegment(*basicSkeletonTemplate, ssd.getAttachmentTransformName()));

			//-- release local references
			basicSkeletonTemplate->release();
		}
	}

	// release local SkeletonTemplate references
	std::for_each(skeletonTemplates.begin(), skeletonTemplates.end(), VoidMemberFunction(&SkeletonTemplate::release));
}

// ----------------------------------------------------------------------
/**
 * Indicate that one of the MeshGenerator instances on which this
 * SkeletalAppearance2 instance depends has been modified.
 *
 * Invoking this function will result in (at least) the mesh data
 * being rebuilt for this appearance prior to the next render.
 */

void SkeletalAppearance2::notifyMeshGeneratorModified()
{
	markAsDirty();
}

// ----------------------------------------------------------------------

int SkeletalAppearance2::calculateDisplayLodIndex(float diameterScreenFraction, int previousLodIndex) const
{
	if (m_maxAvailableDetailLevelIndex < 0)
		return m_maxAvailableDetailLevelIndex;

	if (m_userControlledDetailLevel)
	{
		// Don't modify the display LOD index when the user is directly controlling detail levels.
		return m_displayLodIndex;
	}

	//-- Adjust diameter for LOD bias.
	if (ms_lodBias > 0.0f)
		diameterScreenFraction *= ms_lodBias;

	int tryLodIndex = ((previousLodIndex >= 0) && (previousLodIndex <= m_maxAvailableDetailLevelIndex)) ? previousLodIndex : m_maxAvailableDetailLevelIndex;

	if (tryLodIndex < 0)
		return 0;

	int const maxLodIndex = std::min(cs_maxLodCount - 1, m_maxAvailableDetailLevelIndex);
	tryLodIndex = std::min (maxLodIndex, tryLodIndex);

	//-- Start with try lod index.
	while ((tryLodIndex >= 0) && (tryLodIndex <= maxLodIndex))
	{
		VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0, tryLodIndex, maxLodIndex);

		// Check screen fraction fits within existing lod.
		if ((diameterScreenFraction <= s_lodInfoArray[tryLodIndex].maxScreenFraction) && (diameterScreenFraction >= s_lodInfoArray[tryLodIndex].minScreenFraction)) //lint !e676 // possible -1 array subscript // wrong.
		{
			// This lod will work.
			break;
		}
		else if (diameterScreenFraction <= s_lodInfoArray[tryLodIndex].minScreenFraction)  //lint !e676 // possible -1 array subscript // wrong.
		{
			// try next lower-detail lod index.
			++tryLodIndex;
		}
		else
		{
			// try next greater-detail lod index
			--tryLodIndex;
		}
	}

	//-- If global maximum LOD index is enabled, make sure we never _try_ to render at a lower-density (larger numbered) detail level index.
	if (s_maximumDesiredDetailLevelEnabled && (tryLodIndex > s_maximumDesiredDetailLevelIndex))
		tryLodIndex = s_maximumDesiredDetailLevelIndex;

	int const sizeBasedLodIndex = clamp(0, tryLodIndex, maxLodIndex);

	if (s_uiContextEnabled || !CharacterLodManager::isEnabled())
	{
		//-- Don't adjust the detail level beyond appropriate lod selection for screen size
		//   if we're in UI context or if the lod manager is disabled.
		return sizeBasedLodIndex;
	}
	else
	{
		//-- Add character to list of characters that should be planned for next frame.
		SkeletalAppearance2 *const nonConstAppearance = const_cast<SkeletalAppearance2*>(this);
		Object *const owner = nonConstAppearance->getOwner();

		if (owner)
			CharacterLodManager::addCharacter(owner);

		if (m_plannedLodSetFrameNumber + 2 < Os::getNumberOfUpdates())
		{
			//-- This appearance doesn't appear to have a planned lod set in the last frame.
			//   There's a +2 because the texture rendering can cause another frame to slip in between
			//   the two frames.
			return sizeBasedLodIndex;
		}

		//-- Start trying to render character at the LOD planned by the character LOD planner.
		//   Use the screen-based Lod index if it is a lower-density detail index.  In other words,
		//   don't draw something at a higher level just because we would let it happen by plan ---
		//   only do so if necessary.
		return std::max(m_plannedLodIndex, sizeBasedLodIndex);
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::removeDeletedWearables()
{
	if (m_wornAppearanceObjects)
	{
		//-- Remove any wearable Object instances if the Object instance has been deleted.
		bool  isDirty = false;

		for (WatcherObjectVectorVector::iterator it = m_wornAppearanceObjects->begin(); it != m_wornAppearanceObjects->end();)
		{
			//-- Get the Object watcher.
			ObjectWatcher & watcher = (*it).first;

			//-- If the Object instance is NULL, remove it and mark this appearance as dirty.
			if (NULL != watcher.getPointer())
				++it;
			else
			{
				// Delete the watcher.
				delete watcher;

				// Remove from container.
				it = m_wornAppearanceObjects->erase(it);

				// Mark appearance as dirty.
				isDirty = true;

				DEBUG_REPORT_LOG(true, ("removing unreported wearable deleted from [%s].\n", getOwner()->getDebugName()));
			}
		}

		//-- Mark this appearance as dirty since worn appearances have been removed.
		if (isDirty)
		{
			markAsDirty();

			Object *const owner = getOwner();
			if (s_destroyedAttachmentWearableCallback && owner)
				(*s_destroyedAttachmentWearableCallback)(*owner);
		}
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::markAsDirty()
{
	// Remember that the appearance (both skeleton and mesh) are now invalidated.
	// I'll rebuild later (lazy eval); this way, multiple wear/unwear calls can be executed
	// with a single appearance rebuild.
	const BoolVector::iterator endIt = m_appearanceDirty.end();
	for (BoolVector::iterator it = m_appearanceDirty.begin(); it != endIt; ++it)
		*it = true; //lint !e1058 // error 1058: (Error -- Initializing a non-const reference '_STL::_Bit_reference &' with a non-lvalue) // This is the class interface.
}

// ----------------------------------------------------------------------
/**
 * Calculates the delta between min and max bounding box of skeletally-deformed meshes
 * vs. skeleton bounding box.
 *
 * Assumes the specified lod has been baked and is not dirty.
 *
 * @param lodIndex  the lod for which we're calculating extent deltas.
 * @param minDelta  the delta between min mesh and min skeleton extent comes from here.
 * @param maxDelta  the delta between max mesh and max skeleton extent comes from here.
 */

void SkeletalAppearance2::calculateExtentDeltas(int lodIndex, Vector &minDelta, Vector &maxDelta) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, static_cast<int>(m_perLodShaderPrimitives->size()));
	ShaderPrimitiveVector const &shaderPrimitives = (*m_perLodShaderPrimitives)[static_cast<ShaderPrimitiveVectorVector::size_type>(lodIndex)];

	//-- Calculate combined min and max extent for all meshes.
	if (shaderPrimitives.empty())
	{
		// There is no mesh so there is no delta from skeleton extent.
		minDelta = Vector::zero;
		maxDelta = Vector::zero;
		return;
	}

	Vector  minMeshExtent;
	Vector  maxMeshExtent;

	Vector  minTemp;
	Vector  maxTemp;

	// Initialize mesh extent data.
	ShaderPrimitive const *baseShaderPrimitive = shaderPrimitives.front();
	SoftwareBlendSkeletalShaderPrimitive const *shaderPrimitive = (baseShaderPrimitive != NULL) ? baseShaderPrimitive->asSoftwareBlendSkeletalShaderPrimitive() : NULL;
	if (shaderPrimitive)
		shaderPrimitive->calculateBoundingBox(minMeshExtent, maxMeshExtent);

	// Process each shader primitive.
	int const shaderPrimitiveCount = static_cast<int>(shaderPrimitives.size());
	for (int i = 1; i < shaderPrimitiveCount; ++i)
	{
		baseShaderPrimitive = shaderPrimitives[static_cast<ShaderPrimitiveVector::size_type>(i)];
		shaderPrimitive    = (baseShaderPrimitive != NULL) ? baseShaderPrimitive->asSoftwareBlendSkeletalShaderPrimitive() : NULL;
		if (!shaderPrimitive)
			continue;

		shaderPrimitive->calculateBoundingBox(minTemp, maxTemp);

		// Incorporate min and max into exiting min and max.
		minMeshExtent.x = std::min(minMeshExtent.x, minTemp.x);
		minMeshExtent.y = std::min(minMeshExtent.y, minTemp.y);
		minMeshExtent.z = std::min(minMeshExtent.z, minTemp.z);

		maxMeshExtent.x = std::max(maxMeshExtent.x, maxTemp.x);
		maxMeshExtent.y = std::max(maxMeshExtent.y, maxTemp.y);
		maxMeshExtent.z = std::max(maxMeshExtent.z, maxTemp.z);
	}

	//-- Get skeleton extent.
	const BoxExtent *const skeletonExtent = getSkeleton(lodIndex).getExtent();
	NOT_NULL(skeletonExtent);

	//-- Calculate deltas.
	minDelta = minMeshExtent - skeletonExtent->getMin();
	maxDelta = maxMeshExtent - skeletonExtent->getMax();
}

// ----------------------------------------------------------------------
/** 
 * Return the extent of the clothed appearance based on skeleton extent
 * plus clothed mesh deltas.
 *
 * This extent does not consider any of the attached objects that get
 * rendered separately through their own appearance.
 *
 * NOTE: do not call this routine unless you have verified that the specified
 *       skeleton is done being asynchronously loaded.
 *
 * @param extent    the skeleton + deltas extent (approximation for true mesh
 *                  extent) is returned here.
 * @param lodIndex  the index of the LOD to return extents for.
 */

void SkeletalAppearance2::getNoAttachmentExtent(BoxExtent &extent, int lodIndex) const
{
	//-- Get skeleton extent.
	const BoxExtent *const skeletonExtent = getSkeleton(lodIndex).getExtent();
	NOT_NULL(skeletonExtent);

	//-- Incorporate min and max delta.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, static_cast<int>(m_perLodMeshExtentMinDelta.size()));
	extent.setMin(skeletonExtent->getMin() + m_perLodMeshExtentMinDelta[static_cast<VectorVector::size_type>(lodIndex)]);

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, static_cast<int>(m_perLodMeshExtentMaxDelta.size()));
	extent.setMax(skeletonExtent->getMax() + m_perLodMeshExtentMaxDelta[static_cast<VectorVector::size_type>(lodIndex)]);

	//-- Calculate new center and radius.
	extent.calculateCenterAndRadius();
}

// ----------------------------------------------------------------------
/**
 * Return the proper skleton extent for the given LOD level.
 *
 * The returned extents start with a shrink-wrapped box extent for the skeleton
 * and then incorporate extents from all attached objects.
 */

void SkeletalAppearance2::updateExtentWithLod(BoxExtent &extent, int lodIndex) const
{
	//-- Ensure mesh and skeleton data is up to date for specified LOD.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, static_cast<int>(m_appearanceDirty.size()));
	m_lodIsReady = const_cast<SkeletalAppearance2*>(this)->rebuildIfDirtyAndAvailable(lodIndex);

	if (!m_lodIsReady)
	{
		DEBUG_REPORT_LOG(s_logReadyIssues, ("updateExtentWithLod(LOD=%d,AT=[%s]): returning bogus extent due to asynchronous loading.\n", lodIndex, getAppearanceTemplateName() ? getAppearanceTemplateName() : "<NULL appearance template name>"));
		extent.set(-Vector::xyz111, Vector::xyz111, Vector::zero, 1.0f);
		updateDpvsTestObjectWithExtents();
#if PRODUCTION == 0
		if (!s_disableSphereTreeUpdates)
#endif
			extentChanged();
		return;
	}

	//-- Get the extent from the skeleton.  This will cause a skeleton update.
	BoxExtent  noAttachmentExtent;
	getNoAttachmentExtent(noAttachmentExtent, lodIndex);

	//-- Incorporate extents of attached appearances into the sphere calculation.
	if (!m_attachedAppearances || m_attachedAppearances->empty())
	{
		// No attached appearance, simply use the Skeleton extent sphere.
		extent.set(noAttachmentExtent.getMin(), noAttachmentExtent.getMax(), noAttachmentExtent.getCenter(), noAttachmentExtent.getRadius());
		updateDpvsTestObjectWithExtents();
#if PRODUCTION == 0
		if (!s_disableSphereTreeUpdates)
#endif
			extentChanged();
		return;
	}

	//-- Start accumulation box extent identical to skeleton box extent.
	extent.setMin(noAttachmentExtent.getMin());
	extent.setMax(noAttachmentExtent.getMax());

	Skeleton *const skeleton = (*m_skeletons)[static_cast<SkeletonVector::size_type>(lodIndex)];
	NOT_NULL(skeleton);

	//-- Walk through all attachments.  Prepare to detatch any stale attachments (attached Object instances that have been deleted) at this time.
	bool deletedAnyAttachments = false;

	for (AttachedAppearanceVector::iterator it = m_attachedAppearances->begin(); it != m_attachedAppearances->end();)
	{
		//-- Get the appearance.
		const Appearance *const appearance = (*it)->getAppearance();
		if (appearance)
		{
			if (!appearance->usesRenderEffectsFlag())
			{
			//-- Incorporate it's extent's min and max extents.
			const Sphere &attachedSphere = appearance->getSphere();
			const Vector &center         = attachedSphere.getCenter();
			const float   radius         = attachedSphere.getRadius();

			const int        transformIndex = (*it)->getTransformIndex(lodIndex);
			const Transform &transform      = skeleton->getJointToRootTransformArray()[transformIndex];

			// Note: this is a cheesed calculation.  Ideally I would deal directly with a box extent.
			extent.updateMinAndMax(transform.rotateTranslate_l2p(Vector(center.x - radius, center.y - radius, center.z - radius)));
			extent.updateMinAndMax(transform.rotateTranslate_l2p(Vector(center.x + radius, center.y + radius, center.z + radius)));
			}

			// Increment loop.
			++it;
		}
		else
		{
			//-- Delete entry, object must have been destroyed.
			DEBUG_REPORT_LOG(true, ("Detatching currently attached but deleted object from [%s].\n", getOwner()->getDebugName()));
			delete *it;
			it = m_attachedAppearances->erase(it);

			deletedAnyAttachments = true;
		}
	}

	//-- Recalculate box extent's sphere and center.
	extent.calculateCenterAndRadius();

	if (deletedAnyAttachments)
	{	
		Object *const owner = const_cast<Object*>(getOwner());
		if (s_destroyedAttachmentWearableCallback && owner)
			(*s_destroyedAttachmentWearableCallback)(*owner);
	}

	updateDpvsTestObjectWithExtents();
#if PRODUCTION == 0
	if (!s_disableSphereTreeUpdates)
#endif
		extentChanged();
}

// ----------------------------------------------------------------------

bool SkeletalAppearance2::rebuildIfDirtyAndAvailable(int lodIndex)
{
	//-- Can be less than zero in the viewer if only the skeleton.
	if ((lodIndex < 0) || (m_maxAvailableDetailLevelIndex < 0))
	{
		// Nothing to do, nothing to build.
		return true;
	}

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, getDetailLevelCount());

	//-- Check if display LOD is dirty.
	if (m_appearanceDirty[static_cast<BoolVector::size_type>(lodIndex)])
	{
		//-- Check if this detail level is available.
		bool const available = areAllMeshGeneratorsReadyForDetailLevel(lodIndex);
		if (!available)
			return false;

		//-- Rebuild the mesh.
		rebuildMesh(lodIndex);
		return true;
	}
	else
	{
		// Not dirty.  Nothing to do, everything's good.
		return true;
	}
}

// ----------------------------------------------------------------------
/**
 * If the current display LOD index is dirty, rebuild the current display
 * LOD index if all mesh generators are ready; otherwise, find the next
 * most appropriate display LOD to render.
 *
 * @return  true if a mesh is read
 */

bool SkeletalAppearance2::rebuildOrAdjustDisplayLodIndex()
{
	int  nextLessDetailIndex = m_displayLodIndex + 1;
	int  nextMoreDetailIndex = m_displayLodIndex - 1;
	bool useMoreDetailIndex  = false;
	bool generatorsAreReady  = false;
	bool meshIsDirty         = false;
	bool shaderPrimitivesAreReady = false;

	int  testDetailIndex = m_displayLodIndex;

	do
	{
		//-- Check if detail level being tested needs to be built.
		meshIsDirty = ((testDetailIndex >= 0) && (testDetailIndex <= m_maxAvailableDetailLevelIndex) && m_appearanceDirty[static_cast<BoolVector::size_type>(testDetailIndex)]);
		if (!meshIsDirty)
			shaderPrimitivesAreReady = areShaderPrimitivesReadyForDetailLevel(testDetailIndex);
		else
			shaderPrimitivesAreReady = false;

		if (!meshIsDirty && shaderPrimitivesAreReady)
		{
			// Found it, exit the loop.
			generatorsAreReady = true;
		}
		else
		{
			// Mesh is dirty.  Check if mesh generators are ready for use.  If not, they could be async loading.
			if (meshIsDirty)
				generatorsAreReady = areAllMeshGeneratorsReadyForDetailLevel(testDetailIndex);
			else
				generatorsAreReady = true;

			if (!shaderPrimitivesAreReady || !generatorsAreReady)
			{
				// Select next detail level to try.  First check if we're going in the more-detail direction.
				if ( ((nextMoreDetailIndex >= 0) && useMoreDetailIndex) || (nextLessDetailIndex > m_maxAvailableDetailLevelIndex) )
				{
					// Going in the more-detail direction, or we're out of less detail indices to try.
					testDetailIndex = nextMoreDetailIndex;
					--nextMoreDetailIndex;
				}
				else
				{
					// Going in the less-detail direction.
					testDetailIndex = nextLessDetailIndex;
					++nextLessDetailIndex;
				}

				// Alternate between using more and less detail until we find one that is loaded and available.
				useMoreDetailIndex = !useMoreDetailIndex;
			}
		}


	} while ((!generatorsAreReady || !shaderPrimitivesAreReady) && (testDetailIndex >= 0) && (testDetailIndex <= m_maxAvailableDetailLevelIndex));

	if (generatorsAreReady && shaderPrimitivesAreReady)
	{
		//-- Use the detail level we selected.
		m_displayLodIndex = testDetailIndex;
		
		//-- Rebuild if necesary.
		if (meshIsDirty)
			rebuildMesh(m_displayLodIndex);
	}
	else
	{
		//-- Don't alter the display lod index: we simply will not render.  There is not a single detail level ready for rendering.
	}

	return generatorsAreReady && shaderPrimitivesAreReady;
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::unloadUnusedResources()
{
	if (m_userControlledDetailLevel)
	{
		// Don't unload anything when LOD is user controlled.  This is
		// necessary for the Viewer.
		return;
	}

	//-- Free any shader primitives that haven't been used in a given number of frames.
	//   The lowest-detail LOD is exempt from this process.  Otherwise we would throw out
	//   and then rebuild the lowest-detail LOD every frame when that appearance was off-screen.
	int const freeShaderPrimFrameCount = ConfigClientSkeletalAnimation::getFreeShaderPrimitiveFrameCount();
	int const currentFrameNumber       = Os::getNumberOfUpdates();

	int lodIndex = 0;

	ShaderPrimitiveVectorVector::iterator const endIt = m_perLodShaderPrimitives->end();
	for (ShaderPrimitiveVectorVector::iterator it = m_perLodShaderPrimitives->begin(); (it != endIt) && (lodIndex < m_maxAvailableDetailLevelIndex); ++it, ++lodIndex)
	{
		ShaderPrimitiveVector &spVector = *it;
		if (!spVector.empty())
		{

			int const framesSinceLastUse = (currentFrameNumber - m_perLodMruFrameVector[static_cast<BoolVector::size_type>(lodIndex)]);
			if (framesSinceLastUse > freeShaderPrimFrameCount)
			{
				bool keepLod = false;

				//-- Do not throw out assets for this LOD if any of the mesh generators currently are asynchronously loading.
				//   Otherwise we could get into a thrash loop where we want an asset, it takes longer to load than the
				//   timeout interval, then we throw it away again.
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, static_cast<int>(m_ownedMeshGenerators->size()));
				MeshGeneratorVector &meshGenerators = (*m_ownedMeshGenerators)[static_cast<MeshGeneratorVectorVector::size_type>(lodIndex)];

				{
					MeshGeneratorVector::iterator const mgEndIt = meshGenerators.end();
					for (MeshGeneratorVector::iterator mgIt = meshGenerators.begin(); mgIt != mgEndIt; ++mgIt)
					{
						MeshGenerator const *meshGenerator = *mgIt;
						if (meshGenerator)
						{
							if (!meshGenerator->isReadyForUse())
							{
								//-- Keep this LOD since it is asynchronously loading.
								keepLod = true;

								//-- Adjust last frame used for this asset so we don't try to throw it out again for a while.
								m_perLodMruFrameVector[static_cast<BoolVector::size_type>(lodIndex)] = currentFrameNumber;
							}
						}
					}
				}

				if (keepLod)
				{
					// We're hanging on to this LOD, don't unload it.
					continue;
				}

				//-- Free the mesh generators.
				std::for_each(meshGenerators.begin(), meshGenerators.end(), VoidMemberFunction(&MeshGenerator::release));
				meshGenerators.clear();

				// Drop this LOD's shader primitives and mesh generators since they haven't been used in a while.
				//-- Free the shader primitives, empty vector so we know they aren't there.
				std::for_each(spVector.begin(), spVector.end(), PointerDeleter());
				spVector.clear();

				//-- Mark this lod as dirty so we know we have to rebuild the shader primitives when needed.
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, lodIndex, static_cast<int>(m_appearanceDirty.size()));
				m_appearanceDirty[static_cast<BoolVector::size_type>(lodIndex)] = true; //lint !e1058 // error 1058: (Error -- Initializing a non-const reference '_STL::_Bit_reference &' with a non-lvalue) // This is the class interface. 

				//-- If we're throwing out detail levels, set display lod index to something that exists
				if (m_displayLodIndex == lodIndex)
					m_displayLodIndex = m_maxAvailableDetailLevelIndex;
			}
		}
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::handleFade(float elapsedTime)
{
	switch (m_fadeState)
	{
		case FS_uninitialized:
			{
				//-- Do nothing.  We move to the FS_waitingForLodCompletion state the
				//   first time we try to render without having all mesh generators ready.
			}
			break;

		case FS_waitingForLoadCompletion:
			{
				//-- Check if display lod's mesh generators are all ready.  If so, start
				//   the fade in timer, transitioning to state FS_fadingIn.
				if ((m_displayLodIndex >= 0) && isDetailLevelAvailable(m_displayLodIndex))
				{
					// Transition to the fade-in state, start timer.
					setFadeState(FS_fadingIn, 0.0f);
				}
			}
			break;

		case FS_fadingIn:
			{
				//-- Continue fading in until the fade-in time expires.  Then transition
				//   to state FS_notFading.  Currently we fade in over one second.
				m_fadeFraction += elapsedTime;
				if (m_fadeFraction >= 1.0f)
				{
					// Fade-in timer has expired, transition to the "not fading" state.
					setFadeState(FS_notFading, 1.0f);
				}
			}
			break;

		case FS_fadeOutToRemove:
			{
				//-- Continue fading out until the fade-out time expires.  Then transition
				//   to state FS_removed.  Currently we fade out over one second.
				m_fadeFraction -= elapsedTime;
				if (m_fadeFraction <= 0.0f)
				{
					// Fade-out timer has expired, transition to the "not fading" state.
					setFadeState(FS_removed, 0.0f);
				}
			}
			break;

		case FS_fadeToHold:
			{
				// Check to see if the hold value is reached.
				if (WithinEpsilonInclusive(m_fadeHold, m_fadeFraction, elapsedTime))
				{
					setFadeState(FS_hold, m_fadeHold);
				}
				else if (m_fadeFraction > m_fadeHold)
				{
					m_fadeFraction -= elapsedTime;
				}
				else
				{
					m_fadeFraction += elapsedTime;
				}
			}
			break;

		case FS_removed:
		case FS_hold:
		case FS_notFading:
		default:
			{
				//-- Do nothing.
			}
			break;
	}
}

// ----------------------------------------------------------------------

void SkeletalAppearance2::updateDpvsTestObjectWithExtents() const
{
	//-- Make sure min and max elements are not reversed.
	fixBoxExtentMinMax(m_extent);

	{
		DPVS::OBBModel *const model = RenderWorld::fetchBoxModel(m_extent.getBox());
		m_dpvsObject->setTestModel(model);
		IGNORE_RETURN(model->release());
	}
}

//----------------------------------------------------------------------

void SkeletalAppearance2::setTargetPosition_w (const Vector & v)
{
	m_targetPositionValid = true;
	m_targetPosition_w = v;
}

//----------------------------------------------------------------------

void SkeletalAppearance2::setExtentDelegateObject (const Object * obj)
{
	m_extentDelegateObject = obj;
	extentChanged();
}

//----------------------------------------------------------------------

void SkeletalAppearance2::setFadeState(FadeState const fadeState, float const fadeFraction, float const fadeHold)
{
	if (m_fadeState != fadeState)
	{
		// call appearance's addToWorld logic.
		if (m_fadeState == FS_removed) 
			Appearance::addToWorld();

		m_fadeState = fadeState;

		// call appearance's removeFromWorld logic.
		if (m_fadeState == FS_removed)
			Appearance::removeFromWorld();
	}

	m_fadeFraction = fadeFraction;
	m_fadeHold = fadeHold;

}

//----------------------------------------------------------------------

SkeletalAppearance2::FadeState SkeletalAppearance2::getFadeState() const
{
	return m_fadeState;
}

//----------------------------------------------------------------------

float SkeletalAppearance2::getFadeFraction() const
{
	return m_fadeFraction;
}

//----------------------------------------------------------------------

void SkeletalAppearance2::addToWorld()
{
	if (m_fadeState != FS_removed)
	{
		Appearance::addToWorld();
	}
}

//----------------------------------------------------------------------

void SkeletalAppearance2::removeFromWorld()
{
	if (m_fadeState != FS_removed)
	{
		Appearance::removeFromWorld();
	}
}

//----------------------------------------------------------------------

void SkeletalAppearance2::setIsBlueGlowie(bool b)
{
	m_isBlueGlowie = b;
	if(b)
		m_isHolonet = false;
}

//----------------------------------------------------------------------

void SkeletalAppearance2::setHologramType(HologramType holoType)
{
	m_hologramType = holoType;
	switch(m_hologramType)
	{
	case HT_none:
		m_blackHologramFrame = 0;
		break;
	case HT_type1_quality1:
	case HT_type2_quality1:
	case HT_type3_quality1:
		m_blackHologramFrame = 30;
		break;
	case HT_type1_quality2:
	case HT_type2_quality2:
	case HT_type3_quality2:
		m_blackHologramFrame = 120;
		break;
	case HT_type1_quality3:
	case HT_type2_quality3:
	case HT_type3_quality3:
		m_blackHologramFrame = 480;
		break;
	case HT_type1_quality4:
	case HT_type2_quality4:
	case HT_type3_quality4:
		m_blackHologramFrame = 0;
		break;
	};
}

//----------------------------------------------------------------------

void SkeletalAppearance2::setIsHolonet(bool b)
{
	m_isHolonet = b;
	if(b)
		m_isBlueGlowie = false;
}

Object* SkeletalAppearance2::getWearableObject(int index)
{
	WatcherObjectVectorVector::iterator objectIt = m_wornAppearanceObjects->begin() + index;

	ObjectWatcher & watcher = (*objectIt).first;

	return watcher.getPointer();
}

bool SkeletalAppearance2::isWearing(const Object *object)
{
	if(!object || !m_wornAppearanceObjects)
		return false;
	
	WatcherObjectVectorVector::iterator wearIter = m_wornAppearanceObjects->begin();
	for(; wearIter != m_wornAppearanceObjects->end(); ++wearIter)
	{
		ObjectWatcher & watcher = (*wearIter).first;

		if(watcher.getPointer() == object)
			return true;
	}

	return false;
}

const Object* SkeletalAppearance2::getWearableObject(int index) const
{
	NOT_NULL(m_wornAppearanceObjects);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_wornAppearanceObjects->size()));

	const Object *const object = (*m_wornAppearanceObjects)[static_cast<size_t>(index)].first;

	return object;
}

// ======================================================================
