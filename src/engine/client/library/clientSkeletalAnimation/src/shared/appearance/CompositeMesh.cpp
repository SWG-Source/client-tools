// ==================================================================
//
// CompositeMesh.cpp
// copyright 2001 Sony Online Entertainment
// 
// ==================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/CompositeMesh.h"

#include "boost/smart_ptr.hpp"
#include "clientGraphics/Graphics.h"
#include "clientSkeletalAnimation/MeshConstructionHelper.h"
#include "clientSkeletalAnimation/MeshGenerator.h"
#include "clientSkeletalAnimation/OcclusionZoneSet.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"

#include <vector>

#define TRACK_SHADER_PRIMITIVE_GENERATION  0

#if TRACK_SHADER_PRIMITIVE_GENERATION
#include "clientSkeletalAnimation/MeshGeneratorTemplate.h"
#endif

// ======================================================================

#define ADD_ALWAYS_OCCLUSION_ZONE  1

// ======================================================================

namespace
{
	static bool                    ms_installed;
	static MeshConstructionHelper *ms_meshConstructionHelper;

#if ADD_ALWAYS_OCCLUSION_ZONE
	static int                     ms_alwaysOcclusionZoneId;
#endif
}

// ======================================================================

struct CompositeMesh::GeneratorContainer
{
public:

	typedef std::vector<GeneratorLayer> ContainerType;

public:

	GeneratorContainer();
	~GeneratorContainer();

	ContainerType::iterator  findFirstAtLayer(int layer);

public:

	ContainerType  m_container;
};

// ======================================================================

struct CompositeMesh::GeneratorLayer
{
public:

	GeneratorLayer(int layer, const MeshGenerator *meshGenerator, CustomizationData *customizationData);
	GeneratorLayer(const GeneratorLayer &rhs);
	~GeneratorLayer();

public:

	GeneratorLayer &operator =(const GeneratorLayer &rhs);
	bool            operator ==(const GeneratorLayer &rhs) const;

public:

	int                  m_layer;
	const MeshGenerator *m_meshGenerator;
	CustomizationData   *m_customizationData;

private:

	// disabled
	GeneratorLayer();

};

// ==================================================================
// class CompositeMesh::GeneratorLayer
// ======================================================================

inline CompositeMesh::GeneratorLayer::GeneratorLayer(int layer, const MeshGenerator *meshGenerator, CustomizationData *customizationData)
:	m_layer(layer),
	m_meshGenerator(meshGenerator),
	m_customizationData(customizationData)
{
	NOT_NULL(m_meshGenerator);
}

// ------------------------------------------------------------------

inline CompositeMesh::GeneratorLayer::GeneratorLayer(const GeneratorLayer &rhs) :
	m_layer(rhs.m_layer),
	m_meshGenerator(rhs.m_meshGenerator),
	m_customizationData(rhs.m_customizationData)
{
}

// ------------------------------------------------------------------

inline CompositeMesh::GeneratorLayer::~GeneratorLayer()
{
	// we don't own these
	m_meshGenerator     = 0;
	m_customizationData = 0;
}

// ------------------------------------------------------------------

inline CompositeMesh::GeneratorLayer &CompositeMesh::GeneratorLayer::operator =(const GeneratorLayer &rhs)
{
	if (&rhs == this)
		return *this;

	m_layer             = rhs.m_layer;
	m_meshGenerator     = rhs.m_meshGenerator;
	m_customizationData = rhs.m_customizationData;

	return *this;
}

// ------------------------------------------------------------------

inline bool CompositeMesh::GeneratorLayer::operator ==(const GeneratorLayer &rhs) const
{
	// note: we don't include customization data as part of the comparison
	return (m_layer == rhs.m_layer) && (m_meshGenerator == rhs.m_meshGenerator);
}

// ==================================================================
// class CompositeMesh::GeneratorContainer
// ======================================================================

inline CompositeMesh::GeneratorContainer::GeneratorContainer()
: m_container()
{
}

// ------------------------------------------------------------------

inline CompositeMesh::GeneratorContainer::~GeneratorContainer()
{
	
	const ContainerType::iterator endIt = m_container.end();
	for (ContainerType::iterator it = m_container.begin(); it != endIt; ++it)
	{
		it->m_meshGenerator->release();
		(*it).m_meshGenerator = 0;
	}
}

// ------------------------------------------------------------------

inline CompositeMesh::GeneratorContainer::ContainerType::iterator CompositeMesh::GeneratorContainer::findFirstAtLayer(int layer)
{
	// return iterator for first MeshGenerator at or below the given level

	ContainerType::iterator        it    = m_container.begin();
	ContainerType::const_iterator  itEnd = m_container.end();

	for (; it != itEnd; ++it)
	{
		if ((*it).m_layer <= layer)
			break;
	}

	return it;
}

// ======================================================================
// class CompositeMesh
// ======================================================================

void CompositeMesh::install()
{
	DEBUG_FATAL(ms_installed, ("CompositeMesh already installed"));

	ExitChain::add(remove, "CompositeMesh");
	ms_meshConstructionHelper = new MeshConstructionHelper();

#if ADD_ALWAYS_OCCLUSION_ZONE

	std::vector<boost::shared_ptr<CrcLowerString> >  occlusionZoneNames;
	std::vector<int>                                 occlusionZoneIds;

	occlusionZoneNames.push_back(boost::shared_ptr<CrcLowerString>(new ConstCharCrcLowerString("test_always")));
	OcclusionZoneSet::registerOcclusionZones(occlusionZoneNames, occlusionZoneIds);

	DEBUG_FATAL(occlusionZoneIds.empty(), ("expecting an occlusion zone id in return"));

	ms_alwaysOcclusionZoneId = occlusionZoneIds.front();

#endif

	ms_installed = true;
}

// ----------------------------------------------------------------------

void CompositeMesh::remove()
{
	DEBUG_FATAL(!ms_installed, ("CompositeMesh not installed"));

	delete ms_meshConstructionHelper;
	ms_meshConstructionHelper = 0;

	ms_installed = false;
}

// ======================================================================

CompositeMesh::CompositeMesh() :
	m_meshGenerators(0)
{
	DEBUG_FATAL(!ms_installed, ("CompositeMesh not installed"));
	m_meshGenerators = new GeneratorContainer;
}

// ----------------------------------------------------------------------

CompositeMesh::~CompositeMesh()
{
	delete m_meshGenerators;
}

// ----------------------------------------------------------------------

void CompositeMesh::addMeshGenerator(const MeshGenerator *meshGenerator, CustomizationData *customizationData)
{
	NOT_NULL(m_meshGenerators);
	NOT_NULL(meshGenerator);

	const int occlusionLayer = meshGenerator->getOcclusionLayer();

#ifdef _DEBUG
	// check if we're inserting the same mesh twice at the same level
	// -TRF- might want to check globally
	const GeneratorLayer  itemToInsert(occlusionLayer, meshGenerator, customizationData);

	GeneratorContainer::ContainerType::const_iterator itCopy = std::find(m_meshGenerators->m_container.begin(), m_meshGenerators->m_container.end(), itemToInsert);
	DEBUG_FATAL(itCopy != m_meshGenerators->m_container.end(), ("tried to insert duplicate mesh generator at same layer (0x%08, layer %d)", meshGenerator, occlusionLayer));
#endif

	// Insert this mesh as first at given layer.  We order from highest layer to lowest (outer-most to inner-most).
	GeneratorContainer::ContainerType::iterator itInsertionBefore = m_meshGenerators->findFirstAtLayer(occlusionLayer);
	IGNORE_RETURN(m_meshGenerators->m_container.insert(itInsertionBefore, GeneratorLayer(occlusionLayer, meshGenerator, customizationData)));

	//-- bump up reference count for mesh generator
	meshGenerator->fetch();
}

// ----------------------------------------------------------------------

void CompositeMesh::removeMeshGenerator(const MeshGenerator *meshGenerator)
{
	NOT_NULL(m_meshGenerators);

	// -TRF- do we want to be this friendly in release?
	if (!meshGenerator)
	{
		DEBUG_FATAL(true, ("null meshGenerator arg"));
		REPORT_LOG_PRINT(true, ("null meshGenerator arg, ignoring")); //lint !e527 // unreachable (not in release)
		return;
	}

	// find it in the list
	GeneratorContainer::ContainerType::iterator       it    = m_meshGenerators->m_container.begin();
	GeneratorContainer::ContainerType::const_iterator itEnd = m_meshGenerators->m_container.end();

	for (; it != itEnd; ++it)
	{
		if ((*it).m_meshGenerator == meshGenerator)
			break;
	}

	// erase it if we found it
	const bool foundIt = (it != m_meshGenerators->m_container.end());
	if (!foundIt)
		DEBUG_FATAL(true, ("failed to find mesh generator specified (0x%08x)", meshGenerator));
	else
		static_cast<void>(m_meshGenerators->m_container.erase(it));

#ifdef _DEBUG
	// make sure there was only one --- if we want to allow multiples, API must
	// change to specify layer as well

	it                                                         = m_meshGenerators->m_container.begin();
	GeneratorContainer::ContainerType::const_iterator itNewEnd = m_meshGenerators->m_container.end();

	for (; it != itNewEnd; ++it)
	{
		if ((*it).m_meshGenerator == meshGenerator)
			break;
	}

	// if this triggers, we had multiple references to the same meshGenerator.
	// we don't support that now.
	DEBUG_FATAL(it != itNewEnd, ("unexpected: multiple meshGenerators with same value"));

#endif

	//-- release reference to mesh generator
	meshGenerator->release();
}

// ----------------------------------------------------------------------

void CompositeMesh::removeAllMeshGenerators()
{
	//-- release the mesh generators
	const GeneratorContainer::ContainerType::const_iterator endIt = m_meshGenerators->m_container.end();
	for (GeneratorContainer::ContainerType::iterator it = m_meshGenerators->m_container.begin(); it != endIt; ++it)
	{
		const MeshGenerator *const meshGenerator = (*it).m_meshGenerator;
		NOT_NULL(meshGenerator);

		meshGenerator->release();
	}

	//-- remove the container entries
	m_meshGenerators->m_container.clear();
}

// ----------------------------------------------------------------------

int CompositeMesh::getMeshGeneratorCount() const
{
	NOT_NULL(m_meshGenerators);
	return static_cast<int>(m_meshGenerators->m_container.size());
}

// ----------------------------------------------------------------------

void CompositeMesh::getMeshGenerator(int index, int *layer, const MeshGenerator **meshGenerator) const
{
	// validate args
	NOT_NULL(m_meshGenerators);
	NOT_NULL(layer);
	NOT_NULL(meshGenerator);

	// make sure index is within bounds
	const size_t realIndex = static_cast<size_t>(index);
	DEBUG_FATAL(realIndex >= m_meshGenerators->m_container.size(), ("index value (%u) out of valid range [0..%u]", realIndex, m_meshGenerators->m_container.size()));

	// get element & return
	GeneratorLayer &generatorLayer = m_meshGenerators->m_container[realIndex];
	*layer                         = generatorLayer.m_layer;
	*meshGenerator                 = generatorLayer.m_meshGenerator;
}

// ----------------------------------------------------------------------

void CompositeMesh::applySkeletonModifications(Skeleton &skeleton) const
{
	if (m_meshGenerators->m_container.empty())
		return;

	const GeneratorContainer::ContainerType::const_iterator itEnd = m_meshGenerators->m_container.end();
	for (GeneratorContainer::ContainerType::const_iterator it = m_meshGenerators->m_container.begin(); it != itEnd; ++it)
		it->m_meshGenerator->applySkeletonModifications(skeleton);
}

// ----------------------------------------------------------------------

void CompositeMesh::addShaderPrimitives(Appearance &appearance, int lodIndex, const TransformNameMap &transformNameMap, ShaderPrimitiveVector &shaderPrimitives) const
{
	NOT_NULL(m_meshGenerators);

	if (m_meshGenerators->m_container.empty())
		return;

	// keep track of what zones are completely occluded from the perspective
	// of the current layer.  we'll work from outside in, so this set will
	// only grow, never shrink.
	OcclusionZoneSet  zonesCurrentlyOccluded;

#if ADD_ALWAYS_OCCLUSION_ZONE
	zonesCurrentlyOccluded.addZone(ms_alwaysOcclusionZoneId);
#endif

	// keep track of the zones fully occluded by items at this layer.  those
	// zones will be applied to all inner layers.
	OcclusionZoneSet  zonesOccludedByThisLayer;

	GeneratorContainer::ContainerType::iterator        it    = m_meshGenerators->m_container.begin();
	const GeneratorContainer::ContainerType::iterator  itEnd = m_meshGenerators->m_container.end();

#if TRACK_SHADER_PRIMITIVE_GENERATION
	int runCount = 0;
#endif

	while (it != itEnd)
	{
		const MeshGenerator *meshGenerator = it->m_meshGenerator;
		NOT_NULL(meshGenerator);

#if TRACK_SHADER_PRIMITIVE_GENERATION
		const int  startShaderPrimitiveCount = static_cast<int>(shaderPrimitives.size());
#endif

		meshGenerator->addShaderPrimitives(appearance, lodIndex, it->m_customizationData, transformNameMap, zonesCurrentlyOccluded, zonesOccludedByThisLayer, shaderPrimitives);

#if TRACK_SHADER_PRIMITIVE_GENERATION
		++runCount;
		const int endShaderPrimitiveCount = static_cast<int>(shaderPrimitives.size());
		DEBUG_REPORT_LOG(true, ("CM: loop [%2d]: mgn [%s] produced [%d] shader primitives.\n", runCount, meshGenerator->getMeshGeneratorTemplate().getName().getString(), endShaderPrimitiveCount - startShaderPrimitiveCount));
#endif

		// increment loop
		const int currentLayer = it->m_layer;
		++it;

		// handle when we're transitioning to process a new level
		if ((it != itEnd) && (it->m_layer != currentLayer))
		{
			// apply the zones fully occluded by this layer to the
			// set of occluded zones.  two meshes applied at the same
			// layer do not occlude one another.
			zonesCurrentlyOccluded.insertSet(zonesOccludedByThisLayer);
			zonesOccludedByThisLayer.clear();
		}
	}

#if TRACK_SHADER_PRIMITIVE_GENERATION
	DEBUG_REPORT_LOG(true, ("CM: total: [%d] runs, [%d] primitives produced.\n", runCount, static_cast<int>(shaderPrimitives.size())));
	DEBUG_REPORT_LOG(true, ("CM: =====\n"));
#endif
}

// ======================================================================
