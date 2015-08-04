// ======================================================================
//
// OccludedFaceMapGenerator.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "OccludedFaceMapGenerator.h"

#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "MayaCompoundString.h"
#include "Messenger.h"
#include "sharedUtility/UniqueNameList.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "boost/smart_ptr.hpp"

#include "maya/MDagPath.h"
#include "maya/MFnMesh.h"
#include "maya/MFnSet.h"
#include "maya/MFnSingleIndexedComponent.h"
#include "maya/MGlobal.h"
#include "maya/MObject.h"
#include "maya/MSelectionList.h"
#include "maya/MStatus.h"
#include "maya/MStringArray.h"

// ======================================================================

#define CONSTRUCTOR_ERROR(status, description) \
	if (!status) \
	{ \
		MESSENGER_LOG((description ": [%s]", status.errorString().asChar())); \
		return; \
	}

// ----------------------------------------------------------------------

#define FUNCTOR_STATUS_ERROR(status, description) \
	if (!status) \
	{ \
		MESSENGER_LOG((description ": [%s]", status.errorString().asChar())); \
		m_success = false; \
		return; \
	}

// ----------------------------------------------------------------------

#define FUNCTOR_ERROR(condition, description) \
	if (condition) \
	{ \
		MESSENGER_LOG(description); \
		m_success = false; \
		return; \
	}

// ======================================================================

namespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<std::vector<int> >  FaceParticipationVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	/**
	 * Builds the list of unique occlusion zones and the list of occlusion
	 * zones that each face belongs to.
	 */

	class FaceParticipationBuilder
	{
	public:

		explicit FaceParticipationBuilder(const MDagPath &meshDagPath, UniqueNameList *occlusionZoneNames, FaceParticipationVector &faceParticipationVector);

		void     operator()(MObject object);
		bool     getSuccess() const;

	private:

		const MDagPath          *m_meshDagPath;
		UniqueNameList          *m_occlusionZoneNames;
		FaceParticipationVector *m_faceParticipationVector;
		bool                     m_success;

	private:
		// disabled
		FaceParticipationBuilder();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	/**
	 * Builds the list of unique occlusion zone combinations.
	 */

	class UniqueIntVectorBuilder
	{
	public:

		UniqueIntVectorBuilder();

		int   submit(const std::vector<int> &submission);

		void  getUniqueVectors(std::vector<std::vector<int> > &uniqueVectors) const;

	private:

		struct VectorInfo;

		typedef std::map<const std::vector<int>*, boost::shared_ptr<VectorInfo>, LessPointerComparator>  VectorInfoMap;

	private:

		VectorInfoMap  m_vectorInfoMap;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class SetDumper
	{
	public:

		explicit SetDumper(const MDagPath &mesh);

		void     operator()(MObject object);
		bool     getSuccess() const;

	private:

		MDagPath   m_meshDagPath;
		bool       m_success;

	private:
		// disabled
		SetDumper();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Messenger *messenger;
}

// ======================================================================
// struct UniqueIntVectorBuilder::VectorInfo
// ======================================================================

struct UniqueIntVectorBuilder::VectorInfo
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct LessIndexComparator
	{
		bool operator ()(const boost::shared_ptr<VectorInfo> &lhs, const boost::shared_ptr<VectorInfo> &rhs)
		{
			return lhs->m_index < rhs->m_index;
		}
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	VectorInfo(const std::vector<int> &newVector, int newIndex);

public:

	std::vector<int>  m_vector;
	int               m_index;

private:
	// disabled
	VectorInfo();
};

// ======================================================================

UniqueIntVectorBuilder::VectorInfo::VectorInfo(const std::vector<int> &newVector, int newIndex)
:	m_vector(newVector),
	m_index(newIndex)
{
}

// ======================================================================
// class UniqueIntVectorBuilder
// ======================================================================

UniqueIntVectorBuilder::UniqueIntVectorBuilder()
:	m_vectorInfoMap()
{
}

// ----------------------------------------------------------------------

int UniqueIntVectorBuilder::submit(const std::vector<int> &submission)
{
	VectorInfoMap::iterator it = m_vectorInfoMap.find(&submission);
	if (it == m_vectorInfoMap.end())
	{
		// new vector
		const int                     newIndex = static_cast<int>(m_vectorInfoMap.size());
		boost::shared_ptr<VectorInfo> newVectorInfo(new VectorInfo(submission, newIndex));

		IGNORE_RETURN(m_vectorInfoMap.insert(VectorInfoMap::value_type(&(newVectorInfo->m_vector), newVectorInfo)));

		return newIndex;
	}
	else
	{
		// vector already exists
		return it->second->m_index;
	}
}

// ----------------------------------------------------------------------

void UniqueIntVectorBuilder::getUniqueVectors(std::vector<std::vector<int> > &uniqueVectors) const
{
	typedef std::vector<boost::shared_ptr<VectorInfo> >  SharedVectorInfoVector;

	SharedVectorInfoVector  uniqueVectorVector;

	//-- add all unique vectors to the vector of unique vectors
	{
		uniqueVectorVector.reserve(m_vectorInfoMap.size());

		const VectorInfoMap::const_iterator itEnd = m_vectorInfoMap.end();
		for (VectorInfoMap::const_iterator it = m_vectorInfoMap.begin(); it != itEnd; ++it)
			uniqueVectorVector.push_back(it->second);
	}

	//-- sort the list of unique vectors by index
	std::sort(uniqueVectorVector.begin(), uniqueVectorVector.end(), VectorInfo::LessIndexComparator());

	//-- copy sorted list of unique vectors to caller
	{
		uniqueVectors.clear();
		uniqueVectors.reserve(uniqueVectorVector.size());

		const SharedVectorInfoVector::const_iterator itEnd = uniqueVectorVector.end();
		for (SharedVectorInfoVector::const_iterator it = uniqueVectorVector.begin(); it != itEnd; ++it)
			uniqueVectors.push_back((*it)->m_vector);
	}
}

// ======================================================================
// class FaceParticipationBuilder
// ======================================================================

FaceParticipationBuilder::FaceParticipationBuilder(const MDagPath &meshDagPath, UniqueNameList *occlusionZoneNames, FaceParticipationVector &faceParticipationVector)
:	m_meshDagPath(&meshDagPath),
	m_occlusionZoneNames(occlusionZoneNames),
	m_faceParticipationVector(&faceParticipationVector),
	m_success(true)
{
}

// ----------------------------------------------------------------------

void FaceParticipationBuilder::operator()(MObject object)
{
	MESSENGER_INDENT;
	MStatus status;

	//-- get function set for set
	MFnSet fnSet(object, &status);
	FUNCTOR_STATUS_ERROR(status, "failed to get MFnSet for object");

	//-- get name of occlusion zone (second component of compound string name for set)
	const MString mayaNodeName = fnSet.name(&status);
	FUNCTOR_STATUS_ERROR(status, "name() failed for node");

	const MayaCompoundString  compoundNodeName(mayaNodeName);
	FUNCTOR_ERROR(compoundNodeName.getComponentCount() < 2, ("no occlusion zone name embedded in occlusion region set [%s]\n", mayaNodeName.asChar()));
	
	MString  mayaOcclusionZoneName = compoundNodeName.getComponentString(1);
	// @todo add to a flag: too much spam for typical case but is useful info.
	// MESSENGER_LOG(("Processing polygons for occlusion zone [%s]\n", mayaOcclusionZoneName.asChar()));

	//-- submit occlusion zone name, get index
	boost::shared_ptr<CrcLowerString> crcZoneName(new CrcLowerString(mayaOcclusionZoneName.asChar()));
	const int zoneIndex = m_occlusionZoneNames->submitName(crcZoneName);

	//-- get members
	MSelectionList  setMembers;

	status = fnSet.getMembers(setMembers, true);
	FUNCTOR_STATUS_ERROR(status, "getMembers() failed");

	{
		MESSENGER_INDENT;

		MDagPath                   dagPath;
		MObject                    component;
		MFnSingleIndexedComponent  fnComponent;
		MIntArray                  selectedPolygonIndices;

		int                        faceCount = 0;

		const unsigned int selectionCount = setMembers.length();
		for (unsigned int i = 0; i < selectionCount; ++i)
		{
			//-- we only deal with dag paths
			status = setMembers.getDagPath(i, dagPath, component);
			if (status && (dagPath == *m_meshDagPath))
			{
				// this set item refers to the target mesh.

				//-- find affected polygons
				// get component function set
				status = fnComponent.setObject(component);
				if (!status)
					continue;

				// get list of selected components
				status = fnComponent.getElements(selectedPolygonIndices);
				FUNCTOR_STATUS_ERROR(status, "getElements() failed");

				//-- add this occlusion zone to each position
				const unsigned int affectedFaceCount = selectedPolygonIndices.length();
				for (unsigned int selectionIndex = 0; selectionIndex < affectedFaceCount; ++selectionIndex)
				{
					const int polygonIndex = selectedPolygonIndices[selectionIndex];

					if (polygonIndex < static_cast<int>(m_faceParticipationVector->size()))
						(*m_faceParticipationVector)[static_cast<size_t>(polygonIndex)].push_back(zoneIndex);
					else
						MESSENGER_LOG_WARNING(("OccludedFaceMapGenerator: selectedPolygonIndex value (%d) is invalid. The max value is (%d).\n", polygonIndex, m_faceParticipationVector->size() - 1));
				}

				faceCount += static_cast<int>(affectedFaceCount);
			}
			else
			{
				// This spams us.
				// MESSENGER_LOG(("unknown set member type for index [%u]\n", i));
			}
		}

		// @todo add to a flag: too much spam for typical case but is useful info.
		// MESSENGER_LOG(("-- [%d] faces from set affect this mesh\n", faceCount));
	}
}

// ----------------------------------------------------------------------

inline bool FaceParticipationBuilder::getSuccess() const
{
	return m_success;
}

// ======================================================================
// class SetDumper
// ======================================================================

SetDumper::SetDumper(const MDagPath &mesh)
:	m_meshDagPath(mesh),
	m_success(true)
{
}

// ----------------------------------------------------------------------

void SetDumper::operator()(MObject object)
{
	MESSENGER_INDENT;
	MStatus status;

	//-- get function set for set
	MFnSet fnSet(object, &status);
	FUNCTOR_STATUS_ERROR(status, "failed to get MFnSet for object");

	//-- get members
	MSelectionList  setMembers;

	status = fnSet.getMembers(setMembers, true);
	FUNCTOR_STATUS_ERROR(status, "getMembers() failed");

	MESSENGER_LOG(("Examining set [%s]:\n", fnSet.name().asChar()));
	{
		MESSENGER_INDENT;

		//-- print member info
		MDagPath  dagPath;
		MObject   component;

		const unsigned int selectionCount = setMembers.length();
		for (unsigned int i = 0; i < selectionCount; ++i)
		{
			//-- first check for a dag path
			status = setMembers.getDagPath(i, dagPath, component);
			if (status)
			{
				MESSENGER_LOG(("set contains DAG [%s], component type [%s]\n", dagPath.partialPathName().asChar(), component.apiTypeStr()));

				//-- find compatible function set for components (if present)
				MStringArray  functionSetArray;

				status = MGlobal::getFunctionSetList(component, functionSetArray);
				if (status)
				{
					const unsigned int functionSetCount = functionSetArray.length();
					for (unsigned int fsIndex = 0; fsIndex < functionSetCount; ++fsIndex)
						MESSENGER_LOG(("-- function set [%s] available for component\n", functionSetArray[fsIndex].asChar()));
				}
			}
			else
			{
				// This spams us.
				// MESSENGER_LOG(("unknown set member type for index [%u]\n", i));
			}
		}
	}
}

// ----------------------------------------------------------------------

inline bool SetDumper::getSuccess() const
{
	return m_success;
}

// ======================================================================
// class OccludedFaceMapGenerator
// ======================================================================

void OccludedFaceMapGenerator::install(Messenger *newMessenger)
{
	ExitChain::add(remove, "OccludedFaceMapGenerator");
	messenger = newMessenger;
}

// ----------------------------------------------------------------------

void OccludedFaceMapGenerator::remove()
{
	messenger = 0;
}

// ======================================================================

OccludedFaceMapGenerator::OccludedFaceMapGenerator(const MObjectVector &occlusionSets, const MDagPath &meshDagPath, MStatus &status)
:	m_occlusionZoneNames(new UniqueNameList()),
	m_occlusionZoneCombinations(new IntVectorVector()),
	m_faceOcclusionZoneCombinations(new IntVector()),
	m_hasNonOccludableFace(false)
{
	MESSENGER_INDENT;

	//-- get # polygons in mesh
	MFnMesh  fnMesh(meshDagPath, &status);
	CONSTRUCTOR_ERROR(status, "MFnMesh() failed");

	const int polygonCount = fnMesh.numPolygons(&status);
	CONSTRUCTOR_ERROR(status, "numPolygons() failed");

	//-- populate face participation structure
	FaceParticipationVector  faceParticipationVector(static_cast<size_t>(polygonCount));

	FaceParticipationBuilder builder = std::for_each(occlusionSets.begin(), occlusionSets.end(), FaceParticipationBuilder(meshDagPath, m_occlusionZoneNames.get(), faceParticipationVector));
	if (builder.getSuccess())
		status = MStatus(MStatus::kSuccess);
	else
		status = MStatus(MStatus::kFailure);

	//-- generate unique occlusion zone combinations
	{
		UniqueIntVectorBuilder  uniqueOcclusionZoneBuilder;

		m_faceOcclusionZoneCombinations->reserve(static_cast<size_t>(polygonCount));

		for (int i = 0; i < polygonCount; ++i)
		{
			std::vector<int> &faceOcclusionZones = faceParticipationVector[static_cast<size_t>(i)];

			if (faceOcclusionZones.empty())
			{
				// mark polys that don't fit into occlusion zones with -1
				m_faceOcclusionZoneCombinations->push_back(-1);
				m_hasNonOccludableFace = true;
			}
			else
			{
				//-- make this polygon's occlusion zone participation vector sorted and unique
				std::sort(faceOcclusionZones.begin(), faceOcclusionZones.end());
				IGNORE_RETURN(faceOcclusionZones.erase(std::unique(faceOcclusionZones.begin(), faceOcclusionZones.end()), faceOcclusionZones.end()));

				//-- submit and return the index for this occlusion zone combination
				const int occlusionZoneCombinationIndex = uniqueOcclusionZoneBuilder.submit(faceOcclusionZones);

				//-- keep track of this combination index
				m_faceOcclusionZoneCombinations->push_back(occlusionZoneCombinationIndex);
			}
		}

		uniqueOcclusionZoneBuilder.getUniqueVectors(*(m_occlusionZoneCombinations.get()));
	}

#if 0
	//-- process each occlusion set
	SetDumper result = std::for_each(occlusionSets.begin(), occlusionSets.end(), SetDumper(meshDagPath));
	if (result.getSuccess())
		status = MStatus(MStatus::kSuccess);
	else
		status = MStatus(MStatus::kFailure);
#endif
}

// ----------------------------------------------------------------------

OccludedFaceMapGenerator::~OccludedFaceMapGenerator()
{
}

// ----------------------------------------------------------------------

void OccludedFaceMapGenerator::debugDump() const
{
	MESSENGER_INDENT;

	MESSENGER_LOG(("BEGIN: OccludedFaceMapGenerator debugDump\n"));

	//-- print out occlusion zone names
	{
		MESSENGER_INDENT;

		const int nameCount = m_occlusionZoneNames->getUniqueNameCount();
		MESSENGER_LOG(("Affected Occlusion Zone Names (%d):\n", nameCount));

		{
			MESSENGER_INDENT;

			for (int i = 0; i < nameCount; ++i)
			{
				const CrcLowerString &name = m_occlusionZoneNames->getName(i);
				MESSENGER_LOG((name.getString()));
			}
		}
	}

	//-- print out unique occlusion zone combinations
	{
		MESSENGER_INDENT;
		MESSENGER_LOG(("Unique Occlusion Zone Combinations (%u):\n", m_occlusionZoneCombinations->size()));

		const IntVectorVector::const_iterator itEnd = m_occlusionZoneCombinations->end();
		for (IntVectorVector::const_iterator it = m_occlusionZoneCombinations->begin(); it != itEnd; ++it)
		{
			MESSENGER_INDENT;

			//-- build the combinations string
			std::string  buffer("entries: ");

			IntVector::const_iterator zoneItEnd = it->end();
			for (IntVector::const_iterator zoneIt = it->begin(); zoneIt != zoneItEnd; ++zoneIt)
			{
				char conversionBuffer[32];

				IGNORE_RETURN(_itoa(*zoneIt, conversionBuffer, 10));
				buffer += conversionBuffer;
				buffer += "; ";
			}

			buffer += "\n";

			MESSENGER_LOG((buffer.c_str()));
		}
	}

	//-- print out each mapping
	{
		MESSENGER_INDENT;
		MESSENGER_LOG(("Per Face Occlusion Combination Mapping (%u):\n", m_faceOcclusionZoneCombinations->size()));

		const IntVector::const_iterator itEnd = m_faceOcclusionZoneCombinations->end();
		int                             index = 0;

		for (IntVector::const_iterator it = m_faceOcclusionZoneCombinations->begin(); it != itEnd; ++it, ++index)
		{
			MESSENGER_INDENT;

			MESSENGER_LOG(("item [%d]: %d\n", index, *it));
		}
	}

	MESSENGER_LOG(("END: OccludedFaceMapGenerator debugDump\n"));
}

// ----------------------------------------------------------------------

int OccludedFaceMapGenerator::getOcclusionZoneCount() const
{
	return m_occlusionZoneNames->getUniqueNameCount();
}

// ----------------------------------------------------------------------

const CrcLowerString &OccludedFaceMapGenerator::getOcclusionZoneName(int index) const
{
	return m_occlusionZoneNames->getName(index);
}

// ----------------------------------------------------------------------

int OccludedFaceMapGenerator::getOcclusionZoneCombinationCount() const
{
	return static_cast<int>(m_occlusionZoneCombinations->size());
}

// ----------------------------------------------------------------------

const OccludedFaceMapGenerator::IntVector &OccludedFaceMapGenerator::getOcclusionZoneCombination(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_occlusionZoneCombinations->size()));

	return (*m_occlusionZoneCombinations.get())[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

int OccludedFaceMapGenerator::getFaceCount() const
{
	return static_cast<int>(m_faceOcclusionZoneCombinations->size());
}

// ----------------------------------------------------------------------

int OccludedFaceMapGenerator::getFaceOcclusionZoneCombinationIndex(int faceIndex) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, faceIndex, static_cast<int>(m_faceOcclusionZoneCombinations->size()));

	return (*m_faceOcclusionZoneCombinations.get())[static_cast<size_t>(faceIndex)];
}

// ----------------------------------------------------------------------
/**
 * Return the list of occlusion zone indices that, if all occluded, would
 * fully occlude the mesh.
 *
 * If there are any polygons within the mesh that are not occluded, such
 * that there is no combination of occlusion zones that could fully occlude
 * the mesh, this function returns a zero-length vector.
 */

void OccludedFaceMapGenerator::constructFullyOccludedZoneCombination(IntVector &fullyOccludedZoneCombination) const
{
	fullyOccludedZoneCombination.clear();

	//-- test if any faces are non-occludable
	if (m_hasNonOccludableFace)
	{
		// there are faces that are non-occludable.  there can be no way to fully occlude this mesh.
		return;
	}

	// every occlusion zone referenced must be occluded for this mesh to be occluded

	//-- build return vector
	const int occlusionZoneCount = m_occlusionZoneNames->getUniqueNameCount();
	fullyOccludedZoneCombination.reserve(static_cast<size_t>(occlusionZoneCount));

	for (int i = 0; i < occlusionZoneCount; ++i)
		fullyOccludedZoneCombination.push_back(i);
}

// ======================================================================
