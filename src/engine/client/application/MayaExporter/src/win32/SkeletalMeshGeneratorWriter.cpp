// ======================================================================
//
// SkeletalMeshGeneratorWriter.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#include "SkeletalMeshGeneratorWriter.h"

#include "boost/smart_ptr.hpp"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"

#include <algorithm>
#include <map>
#include <numeric>
#include <string>
#include <vector>

//-----------------------------------------------------------------

const Tag TAG_BLT  = TAG3(B,L,T); 
const Tag TAG_BLTS = TAG(B,L,T,S);
const Tag TAG_DOT3 = TAG(D,O,T,3);
const Tag TAG_DYN  = TAG3(D,Y,N);
const Tag TAG_FOZC = TAG(F,O,Z,C);
const Tag TAG_HPTS = TAG(H,P,T,S);
const Tag TAG_ITL  = TAG3(I,T,L);
const Tag TAG_NIDX = TAG(N,I,D,X);
const Tag TAG_NORM = TAG(N,O,R,M);
const Tag TAG_OITL = TAG(O,I,T,L);
const Tag TAG_OZC  = TAG3(O,Z,C);
const Tag TAG_OZN  = TAG3(O,Z,N);
const Tag TAG_PIDX = TAG(P,I,D,X);
const Tag TAG_POSN = TAG(P,O,S,N);
const Tag TAG_PRIM = TAG(P,R,I,M);
const Tag TAG_PSDT = TAG(P,S,D,T);
const Tag TAG_SKMG = TAG(S,K,M,G);
const Tag TAG_SKTM = TAG(S,K,T,M);
const Tag TAG_STAT = TAG(S,T,A,T);
const Tag TAG_TCSD = TAG(T,C,S,D);
const Tag TAG_TCSF = TAG(T,C,S,F);
const Tag TAG_TRT  = TAG3(T,R,T);
const Tag TAG_TRTS = TAG(T,R,T,S);
const Tag TAG_TXCI = TAG(T,X,C,I);
const Tag TAG_TWDT = TAG(T,W,D,T);
const Tag TAG_TWHD = TAG(T,W,H,D);
const Tag TAG_VDCL = TAG(V,D,C,L);
const Tag TAG_XFNM = TAG(X,F,N,M);
const Tag TAG_ZTO  = TAG3(Z,T,O);

// ======================================================================
// embedded struct declarations
// ======================================================================

class SkeletalMeshGeneratorWriter::DrawPrimitive
{
public:

	DrawPrimitive() {}
	virtual ~DrawPrimitive() {}

	virtual void  write(Iff &iff) const = 0;

};

// ======================================================================

class SkeletalMeshGeneratorWriter::Hardpoint
{
public:

	class FindByName
	{
	public:

		explicit FindByName(const CrcLowerString &targetName);

		bool operator ()(const Hardpoint *hardpoint) const;

	private:

		const CrcLowerString &m_targetName;

	private:
		// disabled
		FindByName();
	};

public:

	Hardpoint(const std::string &hardpointName, const std::string &parentName, const Vector &position, const Quaternion &rotation);

	const CrcLowerString &getHardpointName() const;
	const CrcLowerString &getParentName() const;

	const Vector         &getPosition() const;
	const Quaternion     &getRotation() const;

private:

	CrcLowerString  m_hardpointName;
	CrcLowerString  m_parentName;
	Vector          m_position;
	Quaternion      m_rotation;

private:
	// disabled
	Hardpoint();
};

// ======================================================================

class SkeletalMeshGeneratorWriter::IndexedTriListPrimitive: public SkeletalMeshGeneratorWriter::DrawPrimitive
{
public:

	IndexedTriListPrimitive();
	virtual ~IndexedTriListPrimitive();

	void          setTriCountHint(int triCountHint);
	void          addTriangle(int index0, int index1, int index2);

	virtual void  write(Iff &iff) const;

private:

	std::vector<int>  m_indices;

};

// ======================================================================

class SkeletalMeshGeneratorWriter::OccludableIndexedTriListPrimitive: public SkeletalMeshGeneratorWriter::DrawPrimitive
{
public:

	OccludableIndexedTriListPrimitive();
	virtual ~OccludableIndexedTriListPrimitive();

	void          setTriCountHint(int triCountHint);
	void          addTriangle(int occlusionZoneCombinationIndex, int index0, int index1, int index2);

	virtual void  write(Iff &iff) const;

private:

	std::vector<int>  m_indices;

};

// ======================================================================

struct SkeletalMeshGeneratorWriter::BlendVector
{
public:

	BlendVector(int index, const Vector &deltaVector);

public:

	int     m_index;
	Vector  m_deltaVector;

private:
	// disabled
	BlendVector();
}; 

// ======================================================================

struct SkeletalMeshGeneratorWriter::BlendTarget
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class FindByVariableName
	{
	public:
	
		explicit FindByVariableName(const std::string &variableName);

		bool  operator ()(const BlendTarget &blendTarget) const;

	private:

		const std::string &m_variableName;

	private:
		// disabled
		FindByVariableName();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	void  addHardpointTarget(int dynamicHardpointIndex, const Vector &deltaPosition, const Quaternion &deltaRotation);

	void  write(Iff *iff) const;

private:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class HardpointTarget
	{
	public:

		HardpointTarget(int dynamicHardpointIndex, const Vector &deltaPosition, const Quaternion &deltaRotation);

		void write(Iff &iff) const;

	private:

		int         m_dynamicHardpointIndex;
		Vector      m_deltaPosition;
		Quaternion  m_deltaRotation;

	private:
		// disabled
		HardpointTarget();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<HardpointTarget>  HardpointTargetVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	std::string               m_variableName;
	std::vector<BlendVector>  m_positions;
	std::vector<BlendVector>  m_normals;
	std::vector<BlendVector>  m_dot3Vectors;

	HardpointTargetVector     m_hardpointTargets;
};

// ======================================================================

struct SkeletalMeshGeneratorWriter::BlendTargetContainer
{
public:

	typedef std::vector<BlendTarget> Container;

public:

	Container  m_container;

};

// ======================================================================

struct SkeletalMeshGeneratorWriter::PerShaderData
{
public:

	typedef std::vector<boost::shared_ptr<DrawPrimitive> > SharedDrawPrimitiveVector;

public:

	void write(Iff &iff) const;

public:

	std::string                m_shaderTemplateName;
	int                        m_index;

	int                        m_textureCoordinateSetCount;
	std::vector<int>           m_textureCoordinateSetDimensionality;
	std::vector<int>           m_textureCoordinateSetOffset;
	int                        m_textureCoordinatesPerVertex;

	std::vector<int>           m_positionIndices;
	std::vector<int>           m_normalIndices;
	std::vector<int>           m_dot3VectorIndices;
	std::vector<PackedArgb>    m_colors;	

	std::vector<real>          m_textureCoordinates;

	SharedDrawPrimitiveVector  m_primitives;
};

// ======================================================================

struct SkeletalMeshGeneratorWriter::PerShaderDataContainer
{
public:

	typedef std::vector<PerShaderData> Container;

public:

	Container  m_container;
	
};

// ======================================================================

struct SkeletalMeshGeneratorWriter::TransformWeight
{
	int  m_transformIndex;
	real m_transformWeight;
};

// ======================================================================

struct SkeletalMeshGeneratorWriter::TransformWeightContainer
{
public:

	typedef std::vector<TransformWeight>  Container;

public:

	Container  m_container;
};

// ======================================================================

struct SkeletalMeshGeneratorWriter::TransformWeightHeaderContainer
{
public:

	typedef std::vector<int>  Container;

public:

	// count of weights per position
	Container  m_container;

};

// ======================================================================

struct SkeletalMeshGeneratorWriter::VectorContainer
{
public:

	typedef std::vector<Vector>  Container;

public:

	Container  m_container;

};

// ======================================================================

struct SkeletalMeshGeneratorWriter::TextureRendererData
{
public:

	static int sumEntries(int initialValue, const TextureRendererData *textureRendererData);

public:

	explicit TextureRendererData(const char *textureRendererTemplateName);

	void removeDuplicateEntries();


public:

	typedef std::pair<int, Tag> Entry;
	typedef std::vector<Entry>  EntryContainer;

public:

	CrcLowerString  m_textureRendererTemplateName;
	EntryContainer  m_entries;

private:
	// disabled
	TextureRendererData();
};

// ======================================================================
// class SkeletalMeshGeneratorWriter::Hardpoint::FindByName
// ======================================================================

SkeletalMeshGeneratorWriter::Hardpoint::FindByName::FindByName(const CrcLowerString &targetName)
:	m_targetName(targetName)
{
}

// ----------------------------------------------------------------------

inline bool SkeletalMeshGeneratorWriter::Hardpoint::FindByName::operator ()(const Hardpoint *hardpoint) const
{
	NOT_NULL(hardpoint);

	return m_targetName == hardpoint->getHardpointName();
}

// ======================================================================
// class SkeletalMeshGeneratorWriter::Hardpoint
// ======================================================================

SkeletalMeshGeneratorWriter::Hardpoint::Hardpoint(const std::string &hardpointName, const std::string &parentName, const Vector &position, const Quaternion &rotation)
:	m_hardpointName(hardpointName.c_str()),
	m_parentName(parentName.c_str()),
	m_position(position),
	m_rotation(rotation)
{
}

// ----------------------------------------------------------------------

inline const CrcLowerString &SkeletalMeshGeneratorWriter::Hardpoint::getHardpointName() const
{
	return m_hardpointName;
}

// ----------------------------------------------------------------------

inline const CrcLowerString &SkeletalMeshGeneratorWriter::Hardpoint::getParentName() const
{
	return m_parentName;
}

// ----------------------------------------------------------------------

inline const Vector &SkeletalMeshGeneratorWriter::Hardpoint::getPosition() const
{
	return m_position;
}

// ----------------------------------------------------------------------

inline const Quaternion &SkeletalMeshGeneratorWriter::Hardpoint::getRotation() const
{
	return m_rotation;
}

// ======================================================================
// class SkeletalMeshGeneratorWriter::IndexedTriListPrimitive
// ======================================================================

SkeletalMeshGeneratorWriter::IndexedTriListPrimitive::IndexedTriListPrimitive()
:	DrawPrimitive(),
	m_indices()
{
}

// ----------------------------------------------------------------------

SkeletalMeshGeneratorWriter::IndexedTriListPrimitive::~IndexedTriListPrimitive()
{
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::IndexedTriListPrimitive::setTriCountHint(int triCountHint)
{
	m_indices.reserve(static_cast<size_t>(triCountHint) * 3);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::IndexedTriListPrimitive::addTriangle(int index0, int index1, int index2)
{
	m_indices.push_back(index0);
	m_indices.push_back(index1);
	m_indices.push_back(index2);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::IndexedTriListPrimitive::write(Iff &iff) const
{
	DEBUG_FATAL(m_indices.empty(), ("empty IndexedTriListPrimitive"));

	iff.insertChunk(TAG_ITL);

		const int triangleCount = static_cast<int>(m_indices.size()) / 3;
		DEBUG_FATAL(triangleCount * 3 != static_cast<int>(m_indices.size()), ("tri-list index count not divisible by 3"));

		iff.insertChunkData(static_cast<int>(triangleCount));

		const std::vector<int>::const_iterator itEnd = m_indices.end();
		for (std::vector<int>::const_iterator it = m_indices.begin(); it != itEnd; ++it)
			iff.insertChunkData(static_cast<int32>(*it));

	iff.exitChunk(TAG_ITL);
}

// ======================================================================
// class SkeletalMeshGeneratorWriter::OccludableIndexedTriListPrimitive
// ======================================================================

SkeletalMeshGeneratorWriter::OccludableIndexedTriListPrimitive::OccludableIndexedTriListPrimitive()
:	DrawPrimitive(),
	m_indices()
{
}

// ----------------------------------------------------------------------

SkeletalMeshGeneratorWriter::OccludableIndexedTriListPrimitive::~OccludableIndexedTriListPrimitive()
{
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::OccludableIndexedTriListPrimitive::setTriCountHint(int triCountHint)
{
	m_indices.reserve(static_cast<size_t>(triCountHint) * 4);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::OccludableIndexedTriListPrimitive::addTriangle(int occlusionZoneCombinationIndex, int index0, int index1, int index2)
{
	m_indices.push_back(occlusionZoneCombinationIndex);
	m_indices.push_back(index0);
	m_indices.push_back(index1);
	m_indices.push_back(index2);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::OccludableIndexedTriListPrimitive::write(Iff &iff) const
{
	DEBUG_FATAL(m_indices.empty(), ("empty OccludableIndexedTriListPrimitive"));

	iff.insertChunk(TAG_OITL);

		const size_t indexCount = m_indices.size();

		const int triangleCount = static_cast<int>(indexCount) / 4;
		DEBUG_FATAL(triangleCount * 4 != static_cast<int>(indexCount), ("tri-list index count not divisible by 4"));

		iff.insertChunkData(static_cast<int>(triangleCount));

		for (size_t i = 0; i < indexCount; i += 4)
		{
			iff.insertChunkData(static_cast<int16>(m_indices[i + 0]));

			iff.insertChunkData(static_cast<int32>(m_indices[i + 1]));
			iff.insertChunkData(static_cast<int32>(m_indices[i + 2]));
			iff.insertChunkData(static_cast<int32>(m_indices[i + 3]));
		}

	iff.exitChunk(TAG_OITL);
}

// ======================================================================
// struct SkeletalMeshGeneratorWriter::BlendVector
// ======================================================================

inline SkeletalMeshGeneratorWriter::BlendVector::BlendVector(int index, const Vector &deltaVector)
:
	m_index(index),
	m_deltaVector(deltaVector)
{
}

// ======================================================================
// struct SkeletalMeshGeneratorWriter::BlendTarget::HardpointTarget
// ======================================================================

SkeletalMeshGeneratorWriter::BlendTarget::HardpointTarget::HardpointTarget(int dynamicHardpointIndex, const Vector &deltaPosition, const Quaternion &deltaRotation)
:	m_dynamicHardpointIndex(dynamicHardpointIndex),
	m_deltaPosition(deltaPosition),
	m_deltaRotation(deltaRotation)
{
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::BlendTarget::HardpointTarget::write(Iff &iff) const
{
	iff.insertChunkData(static_cast<int16>(m_dynamicHardpointIndex));
	iff.insertChunkFloatVector(m_deltaPosition);
	iff.insertChunkFloatQuaternion(m_deltaRotation);
}

// ======================================================================
// class SkeletalMeshGeneratorWriter::BlendTarget::FindByVariableName
// ======================================================================

inline SkeletalMeshGeneratorWriter::BlendTarget::FindByVariableName::FindByVariableName(const std::string &variableName)
:	m_variableName(variableName)
{
}

// ----------------------------------------------------------------------

inline bool SkeletalMeshGeneratorWriter::BlendTarget::FindByVariableName::operator ()(const BlendTarget &blendTarget) const
{
	return m_variableName == blendTarget.m_variableName;
}

// ======================================================================
// struct SkeletalMeshGeneratorWriter::BlendTarget
// ======================================================================

inline void SkeletalMeshGeneratorWriter::BlendTarget::addHardpointTarget(int dynamicHardpointIndex, const Vector &deltaPosition, const Quaternion &deltaRotation)
{
	m_hardpointTargets.push_back(HardpointTarget(dynamicHardpointIndex, deltaPosition, deltaRotation));
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::BlendTarget::write(Iff *iff) const
{
	NOT_NULL(iff);

	iff->insertForm(TAG_BLT);
		const size_t positionCount = m_positions.size();
		const size_t normalCount   = m_normals.size();

		iff->insertChunk(TAG_INFO);
			iff->insertChunkData(static_cast<int32>(positionCount));
			iff->insertChunkData(static_cast<int32>(normalCount));
			iff->insertChunkString(m_variableName.c_str());
		iff->exitChunk(TAG_INFO);

		if (positionCount)
		{
			iff->insertChunk(TAG_POSN);
			{
				for (size_t i = 0; i < positionCount; ++i)
				{
					const BlendVector &blendVector = m_positions[i];
					iff->insertChunkData(static_cast<int32>(blendVector.m_index));
					iff->insertChunkFloatVector(blendVector.m_deltaVector);
				}
			}
			iff->exitChunk(TAG_POSN);
		}

		if (normalCount)
		{
			iff->insertChunk(TAG_NORM);
			{
				for (size_t i = 0; i < normalCount; ++i)
				{
					const BlendVector &blendVector = m_normals[i];
					iff->insertChunkData(static_cast<int32>(blendVector.m_index));
					iff->insertChunkFloatVector(blendVector.m_deltaVector);
				}
			}
			iff->exitChunk(TAG_NORM);
		}

		if (!m_dot3Vectors.empty())
		{
			iff->insertChunk(TAG_DOT3);

				// Write Dot3 count.
				iff->insertChunkData(static_cast<int32>(m_dot3Vectors.size()));

				const std::vector<BlendVector>::const_iterator endIt = m_dot3Vectors.end();
				for (std::vector<BlendVector>::const_iterator it = m_dot3Vectors.begin(); it != endIt; ++it)
				{
					const BlendVector &blendVector = *it;
					iff->insertChunkData(static_cast<int32>(blendVector.m_index));
					iff->insertChunkFloatVector(blendVector.m_deltaVector);
				}

			iff->exitChunk(TAG_DOT3);
		}

		//-- handle hardpoint targets
		if (!m_hardpointTargets.empty())
		{
			iff->insertChunk(TAG_HPTS);

				iff->insertChunkData(static_cast<int16>(m_hardpointTargets.size()));

				const HardpointTargetVector::const_iterator endIt = m_hardpointTargets.end();
				for (HardpointTargetVector::const_iterator it = m_hardpointTargets.begin(); it != endIt; ++it)
					it->write(*iff);

			iff->exitChunk(TAG_HPTS);
		}

	iff->exitForm(TAG_BLT);
}

// ======================================================================
// class SkeletalMeshGeneratorWriter::PerShaderData
// ======================================================================

void SkeletalMeshGeneratorWriter::PerShaderData::write(Iff &iff) const
{
	iff.insertForm(TAG_PSDT);

		//-- write shader template data
		// -TRF- later abstract the shader template data, let it write itself,
		// have a named (referenced) version and one that knows how to spit out
		// an inline ShaderTemplate
		iff.insertChunk(TAG_NAME);
			iff.insertChunkString(m_shaderTemplateName.c_str());
		iff.exitChunk(TAG_NAME);

		//-- write vertex position indices
		iff.insertChunk(TAG_PIDX);
		{
			// write # shader vertices
			iff.insertChunkData(static_cast<int32>(m_positionIndices.size()));

			// write position indices
			for (size_t i = 0; i < m_positionIndices.size(); ++i)
				iff.insertChunkData(static_cast<int32>(m_positionIndices[i]));
		}
		iff.exitChunk(TAG_PIDX);

		//-- write normal indices
		if (m_normalIndices.size())
		{
			iff.insertChunk(TAG_NIDX);
				for (size_t i = 0; i < m_normalIndices.size(); ++i)
					iff.insertChunkData(static_cast<int32>(m_normalIndices[i]));
			iff.exitChunk(TAG_NIDX);
		}

		//-- Write dot3 indices.
		if (m_dot3VectorIndices.size())
		{
			iff.insertChunk(TAG_DOT3);
				for (size_t i = 0; i < m_dot3VectorIndices.size(); ++i)
					iff.insertChunkData(static_cast<int32>(m_dot3VectorIndices[i]));
			iff.exitChunk(TAG_DOT3);
		}

		//-- write diffuse color info
		if (m_colors.size())
		{
			iff.insertChunk(TAG_VDCL);
				for (size_t i = 0; i < m_colors.size(); ++i)
				{
					const PackedArgb &color = m_colors[i];
					iff.insertChunkData(static_cast<uint8>(color.getA()));
					iff.insertChunkData(static_cast<uint8>(color.getR()));
					iff.insertChunkData(static_cast<uint8>(color.getG()));
					iff.insertChunkData(static_cast<uint8>(color.getB()));
				}
				iff.exitChunk(TAG_VDCL);
		}

		//-- write texture coordinate info
		if (m_textureCoordinateSetCount)
		{
			iff.insertChunk(TAG_TXCI);

				iff.insertChunkData(static_cast<int32>(m_textureCoordinateSetCount));

				for (size_t i = 0; i < static_cast<size_t>(m_textureCoordinateSetCount); ++i)
					iff.insertChunkData(static_cast<int32>(m_textureCoordinateSetDimensionality[i]));	

			iff.exitChunk(TAG_TXCI);
		}

		//-- write texture coordinate set data
		if (m_textureCoordinateSetCount)
		{
			iff.insertForm(TAG_TCSF);
				for (size_t i = 0; i < static_cast<size_t>(m_textureCoordinateSetCount); ++i)
				{
					iff.insertChunk(TAG_TCSD);

						const size_t dimensionality = static_cast<size_t>(m_textureCoordinateSetDimensionality[i]);
						const size_t vertexOffset   = static_cast<size_t>(m_textureCoordinateSetOffset[i]);

						// write texture data for each vertex
						for (size_t vertexIndex = 0; vertexIndex < m_positionIndices.size(); ++vertexIndex)
						{
							const size_t baseTcIndex = vertexIndex * static_cast<size_t>(m_textureCoordinatesPerVertex) + vertexOffset;
							for (size_t tcIndex = 0; tcIndex < dimensionality; ++tcIndex)
							{
								const real textureCoordinate = m_textureCoordinates[baseTcIndex + tcIndex];
								iff.insertChunkData(static_cast<float>(textureCoordinate));
							}
						}

					iff.exitChunk(TAG_TCSD);
				}
			iff.exitForm(TAG_TCSF);
		}

		//-- write primitive data
		iff.insertForm(TAG_PRIM);
		{
			//-- write out count of primitives
			iff.insertChunk(TAG_INFO);
				iff.insertChunkData(static_cast<int32>(m_primitives.size()));
			iff.exitChunk(TAG_INFO);

			//-- write primitives
			{
				const SharedDrawPrimitiveVector::const_iterator itEnd = m_primitives.end();

				for (SharedDrawPrimitiveVector::const_iterator it = m_primitives.begin(); it != itEnd; ++it)
					(*it)->write(iff);
			}
		}
		iff.exitForm(TAG_PRIM);

	iff.exitForm(TAG_PSDT);
}

// ======================================================================
// struct SkeletalMeshGeneratorWriter::TextureRendererData
// ======================================================================

inline int SkeletalMeshGeneratorWriter::TextureRendererData::sumEntries(int initialValue, const TextureRendererData *textureRendererData)
{
	NOT_NULL(textureRendererData);
	return initialValue + static_cast<int>(textureRendererData->m_entries.size());
}

// ======================================================================

SkeletalMeshGeneratorWriter::TextureRendererData::TextureRendererData(const char *textureRendererTemplateName)
: m_textureRendererTemplateName(textureRendererTemplateName),
	m_entries()
{
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::TextureRendererData::removeDuplicateEntries()
{
	std::sort(m_entries.begin(), m_entries.end());
	IGNORE_RETURN( m_entries.erase(std::unique(m_entries.begin(), m_entries.end()), m_entries.end()) );
}

// ======================================================================
// class SkeletalMeshGeneratorWriter
// ======================================================================

SkeletalMeshGeneratorWriter::SkeletalMeshGeneratorWriter()
:
	m_maxTransformsPerVertex(0),
	m_maxTransformsPerShader(0),
	m_skeletonTemplateNames(new CrcLowerStringVector()),
	m_transformNames(new CrcLowerStringVector()),
	m_positions(0),
	m_transformWeightHeaders(0),
	m_transformWeightData(0),
	m_normals(0),
	m_dot3Values(0),
	m_blendTargets(0),
	m_perShaderData(0),
	m_textureRenderers(new TextureRendererContainer()),
	m_currentIndexedTriList(0),
	m_currentOccludableIndexedTriList(0),
	m_occlusionZoneNames(new CrcLowerStringVector()),
	m_fullyOccludedZoneCombination(new IntVector()),
	m_occlusionZoneCombinations(new SharedIntVectorVector()),
	m_zonesThisOccludes(new IntVector()),
	m_occlusionLayer(-1),
	m_staticHardpoints(new HardpointVector()),
	m_dynamicHardpoints(new HardpointVector())
{
	m_positions              = new VectorContainer();
	m_transformWeightHeaders = new TransformWeightHeaderContainer();
	m_transformWeightData    = new TransformWeightContainer();
	m_normals                = new VectorContainer();
	m_blendTargets           = new BlendTargetContainer();
	m_perShaderData          = new PerShaderDataContainer();
}

// ----------------------------------------------------------------------

SkeletalMeshGeneratorWriter::~SkeletalMeshGeneratorWriter()
{
	std::for_each(m_dynamicHardpoints->begin(), m_dynamicHardpoints->end(), PointerDeleter());
	delete m_dynamicHardpoints;

	std::for_each(m_staticHardpoints->begin(), m_staticHardpoints->end(), PointerDeleter());
	delete m_staticHardpoints;

	m_currentOccludableIndexedTriList = 0;
	m_currentIndexedTriList           = 0;

	delete m_zonesThisOccludes;
	delete m_occlusionZoneCombinations;
	delete m_fullyOccludedZoneCombination;
	delete m_occlusionZoneNames;

	IGNORE_RETURN( std::for_each(m_textureRenderers->begin(), m_textureRenderers->end(), PointerDeleterPairSecond()) );
	delete m_textureRenderers;
	
	delete m_perShaderData;
	delete m_blendTargets;
	delete m_dot3Values;
	delete m_normals;
	delete m_transformWeightData;
	delete m_transformWeightHeaders;
	delete m_positions;
	delete m_skeletonTemplateNames;
	delete m_transformNames;

}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::write(Iff *iff)
{
	NOT_NULL(iff);

	NOT_NULL(m_positions);
	NOT_NULL(m_transformWeightHeaders);
	NOT_NULL(m_transformWeightData);
	NOT_NULL(m_normals);
	NOT_NULL(m_perShaderData);

	iff->insertForm(TAG_SKMG);

		iff->insertForm(TAG_0004);

			//-- write general info
			iff->insertChunk(TAG_INFO);

				iff->insertChunkData(static_cast<int32>(m_maxTransformsPerVertex));
				iff->insertChunkData(static_cast<int32>(m_maxTransformsPerShader));

				iff->insertChunkData(static_cast<int32>(m_skeletonTemplateNames->size()));
				iff->insertChunkData(static_cast<int32>(m_transformNames->size()));
				iff->insertChunkData(static_cast<int32>(m_positions->m_container.size()));
				iff->insertChunkData(static_cast<int32>(m_transformWeightData->m_container.size()));
				iff->insertChunkData(static_cast<int32>(m_normals->m_container.size()));
				iff->insertChunkData(static_cast<int32>(m_perShaderData->m_container.size()));

				iff->insertChunkData(static_cast<int32>(m_blendTargets->m_container.size()));

				iff->insertChunkData(static_cast<int16>(m_occlusionZoneNames->size()));
				iff->insertChunkData(static_cast<int16>(m_occlusionZoneCombinations->size()));

				iff->insertChunkData(static_cast<int16>(m_zonesThisOccludes->size()));

				iff->insertChunkData(static_cast<int16>(m_occlusionLayer));

			iff->exitChunk(TAG_INFO);

			//-- write skeleton template names referenced by this mesh
			iff->insertChunk(TAG_SKTM);
			{
				const CrcLowerStringVector::const_iterator itEnd = m_skeletonTemplateNames->end();
				for (CrcLowerStringVector::const_iterator it = m_skeletonTemplateNames->begin(); it != itEnd; ++it)
					iff->insertChunkString((*it)->getString());
			}
			iff->exitChunk(TAG_SKTM);

			//-- write transform names
			iff->insertChunk(TAG_XFNM);
			{
				const CrcLowerStringVector::const_iterator itEnd = m_transformNames->end();
				for (CrcLowerStringVector::const_iterator it = m_transformNames->begin(); it != itEnd; ++it)
					iff->insertChunkString((*it)->getString());
			}
			iff->exitChunk(TAG_XFNM);

			//-- write position vectors
			iff->insertChunk(TAG_POSN);
			{
				for (unsigned i = 0; i < m_positions->m_container.size(); ++i)
					iff->insertChunkFloatVector(m_positions->m_container[i]);
			}
			iff->exitChunk(TAG_POSN);

			//-- write transform weighting header information
			iff->insertChunk(TAG_TWHD);
			{
				for (unsigned i = 0; i < m_transformWeightHeaders->m_container.size(); ++i)
					iff->insertChunkData(static_cast<int32>(m_transformWeightHeaders->m_container[i]));
			}
			iff->exitChunk(TAG_TWHD);

			//-- write transform weight data
			iff->insertChunk(TAG_TWDT);
			{
				for (unsigned i = 0; i < m_transformWeightData->m_container.size(); ++i)
				{
					const TransformWeight &tw = m_transformWeightData->m_container[i];
					iff->insertChunkData(static_cast<int32>(tw.m_transformIndex));
					iff->insertChunkData(static_cast<float>(tw.m_transformWeight));
				}
			}
			iff->exitChunk(TAG_TWDT);

			//-- write normal vectors
			iff->insertChunk(TAG_NORM);
			{
				for (unsigned i = 0; i < m_normals->m_container.size(); ++i)
					iff->insertChunkFloatVector(m_normals->m_container[i]);
			}
			iff->exitChunk(TAG_NORM);

			//-- Write dot3 values.
			if (m_dot3Values && !m_dot3Values->empty())
			{
				const int dot3ValueCount = static_cast<int>(m_dot3Values->size());
				DEBUG_FATAL((dot3ValueCount % 4) != 0, ("dot3 per-pixel lighting entry count is not a multiple of 4 [%d].", dot3ValueCount));

				iff->insertChunk(TAG_DOT3);
					// Write # entries.
					iff->insertChunkData(static_cast<int32>(dot3ValueCount / 4));

					// Write dot3 value entries.
					const FloatVector::iterator endIt = m_dot3Values->end();
					for ( FloatVector::iterator it = m_dot3Values->begin(); it != endIt; ++it)
						iff->insertChunkData(*it);

				iff->exitChunk(TAG_DOT3);
			}

			//-- write hardpoint data
			if (!m_staticHardpoints->empty() || !m_dynamicHardpoints->empty())
			{
				iff->insertForm(TAG_HPTS);

				// handle static hardpoints
				if (!m_staticHardpoints->empty())
				{
					iff->insertChunk(TAG_STAT);

						writeHardpoints(*iff, *m_staticHardpoints);

					iff->exitChunk(TAG_STAT);
				}


				// handle dynamic hardpoints
				if (!m_dynamicHardpoints->empty())
				{
					iff->insertChunk(TAG_DYN);

						writeHardpoints(*iff, *m_dynamicHardpoints);

					iff->exitChunk(TAG_DYN);
				}

				iff->exitForm(TAG_HPTS);
			}

			//-- write blend target data
			const size_t blendTargetCount = m_blendTargets->m_container.size();
			if (blendTargetCount)
			{
				iff->insertForm(TAG_BLTS);
					for (size_t i = 0; i < blendTargetCount; ++i)
						m_blendTargets->m_container[i].write(iff);
				iff->exitForm(TAG_BLTS);
			}

			//-- write occlusion zone names
			if (!m_occlusionZoneNames->empty())
			{
				iff->insertChunk(TAG_OZN);

					const CrcLowerStringVector::const_iterator itEnd = m_occlusionZoneNames->end();
					for (CrcLowerStringVector::const_iterator it = m_occlusionZoneNames->begin(); it != itEnd; ++it)
						iff->insertChunkString((*it)->getString());

				iff->exitChunk(TAG_OZN);
			}

			//-- write fully occluded zone combination
			if (!m_fullyOccludedZoneCombination->empty())
			{
				iff->insertChunk(TAG_FOZC);

					iff->insertChunkData(static_cast<uint16>(m_fullyOccludedZoneCombination->size()));

					const IntVector::const_iterator itEnd = m_fullyOccludedZoneCombination->end();
					for (IntVector::const_iterator it = m_fullyOccludedZoneCombination->begin(); it != itEnd; ++it)
						iff->insertChunkData(static_cast<int16>(*it));

				iff->exitChunk(TAG_FOZC);
			}

			//-- write occlusion zone combinations
			if (!m_occlusionZoneCombinations->empty())
			{
				iff->insertChunk(TAG_OZC);

					const SharedIntVectorVector::const_iterator vectorItEnd = m_occlusionZoneCombinations->end();
					for (SharedIntVectorVector::const_iterator vectorIt = m_occlusionZoneCombinations->begin(); vectorIt != vectorItEnd; ++vectorIt)
					{
						iff->insertChunkData(static_cast<int16>((*vectorIt)->size()));

						const IntVector::const_iterator itEnd = (*vectorIt)->end();
						for (IntVector::const_iterator it = (*vectorIt)->begin(); it != itEnd; ++it)
							iff->insertChunkData(static_cast<int16>(*it));
					}

				iff->exitChunk(TAG_OZC);
			}

			//-- write zones this occludes
			if (!m_zonesThisOccludes->empty())
			{
				iff->insertChunk(TAG_ZTO);

					const IntVector::const_iterator itEnd = m_zonesThisOccludes->end();
					for (IntVector::const_iterator it = m_zonesThisOccludes->begin(); it != itEnd; ++it)
						iff->insertChunkData(static_cast<int16>(*it));

				iff->exitChunk(TAG_ZTO);
			}

			//-- write per shader data info
			{
				for (size_t i = 0; i < m_perShaderData->m_container.size(); ++i)
					m_perShaderData->m_container[i].write(*iff);
			}

			//-- write texture renderer info
			if (!m_textureRenderers->empty())
			{
				//-- copy texture renderers into a vector to make them easier to work with
				std::vector<TextureRendererData*> textureRenderers;
				textureRenderers.reserve(m_textureRenderers->size());
				{
					const TextureRendererContainer::const_iterator itEnd = m_textureRenderers->end();
					for (TextureRendererContainer::const_iterator it = m_textureRenderers->begin(); it != itEnd; ++it)
						textureRenderers.push_back(it->second);
				}

				// remove duplicate entries
				IGNORE_RETURN( std::for_each(textureRenderers.begin(), textureRenderers.end(), VoidMemberFunction(&TextureRendererData::removeDuplicateEntries)) );
			
				// count total number of entries across all texture renderers
				const int totalEntryCount = std::accumulate(textureRenderers.begin(), textureRenderers.end(), 0, TextureRendererData::sumEntries);

				iff->insertForm(TAG_TRTS);

					iff->insertChunk(TAG_INFO);

						iff->insertChunkData(static_cast<int32>(textureRenderers.size()));
						iff->insertChunkData(static_cast<int32>(totalEntryCount));

					iff->exitChunk(TAG_INFO);

					const std::vector<TextureRendererData*>::const_iterator itEnd = textureRenderers.end();
					for (std::vector<TextureRendererData*>::const_iterator it = textureRenderers.begin(); it != itEnd; ++it)
					{
						iff->insertChunk(TAG_TRT);

							const TextureRendererData &trd = *NON_NULL(*it);
							iff->insertChunkString(trd.m_textureRendererTemplateName.getString());
							iff->insertChunkData(static_cast<int32>(trd.m_entries.size()));

							const TextureRendererData::EntryContainer::const_iterator entryItEnd = trd.m_entries.end();
							for (TextureRendererData::EntryContainer::const_iterator entryIt = trd.m_entries.begin(); entryIt != entryItEnd; ++entryIt)
							{
								iff->insertChunkData(static_cast<int32>(entryIt->first));
								iff->insertChunkData(static_cast<uint32>(entryIt->second));
							}

						iff->exitChunk(TAG_TRT);
					}

				iff->exitForm(TAG_TRTS);
			}

		iff->exitForm(TAG_0004);

	iff->exitForm(TAG_SKMG);	
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::addReferencedSkeletonTemplateName(const char *name)
{
	DEBUG_FATAL(!name || !*name, ("bad name arg"));

	m_skeletonTemplateNames->push_back(boost::shared_ptr<CrcLowerString>(new CrcLowerString(name)));
}

// ----------------------------------------------------------------------

int SkeletalMeshGeneratorWriter::addTransformName(const char *name)
{
	NOT_NULL(name);

	const int transformIndex = static_cast<int>(m_transformNames->size());
	m_transformNames->push_back(boost::shared_ptr<CrcLowerString>(new CrcLowerString(name)));

	return transformIndex;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::addPosition(const Vector &position)
{
	NOT_NULL(m_positions);
	NOT_NULL(m_transformWeightHeaders);

	m_positions->m_container.push_back(position);
	m_transformWeightHeaders->m_container.resize(m_positions->m_container.size());
}

// ----------------------------------------------------------------------
/**
 *
 * Add transform weighting to position.
 *
 * You must add transform weightings in position-index order.  You must add
 * all transform weightings for this position before adding weightings to the
 * next position.
 */

void SkeletalMeshGeneratorWriter::addPositionWeight(int positionIndex, int transformIndex, real weight)
{
	NOT_NULL(m_transformWeightHeaders);
	NOT_NULL(m_transformWeightData);

	//-- add one more to this position's weighting count
	++(m_transformWeightHeaders->m_container[static_cast<size_t>(positionIndex)]);

	//-- add the weighting info
	const size_t weightDataIndex = m_transformWeightData->m_container.size();
	m_transformWeightData->m_container.resize(weightDataIndex + 1);

	TransformWeight&tw   = m_transformWeightData->m_container[weightDataIndex];
	tw.m_transformIndex  = transformIndex;
	tw.m_transformWeight = weight;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the number of positions specified for the SkeletalMeshGenerator
 * instance.
 *
 * @return  the number of positions specified for the SkeletalMeshGenerator
 *          instance.
 */

int SkeletalMeshGeneratorWriter::getPositionCount() const
{
	NOT_NULL(m_positions);
	return static_cast<int>(m_positions->m_container.size());
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::addNormal(const Vector &normal)
{
	NOT_NULL(m_normals);
	m_normals->m_container.push_back(normal);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::addDot3Value(float x, float y, float z, float w)
{
	// Create the vector as necessary.
	if (!m_dot3Values)
		m_dot3Values = new FloatVector;

	// Ensure there's adequate storage for the next 4 values.
	m_dot3Values->reserve(m_dot3Values->size() + 4);

	// Add values to vector.
	m_dot3Values->push_back(x);
	m_dot3Values->push_back(y);
	m_dot3Values->push_back(z);
	m_dot3Values->push_back(w);

	DEBUG_FATAL((m_dot3Values->size() % 4) != 0, ("dot3 per-pixel lighting entry count is not a multiple of 4 [%d].", static_cast<int>(m_dot3Values->size())));
}

// ----------------------------------------------------------------------
/**
 * Retrieve the number of normals specified for the SkeletalMeshGenerator
 * instance.
 *
 * @return  the number of normals specified for the SkeletalMeshGenerator
 *          instance.
 */

int SkeletalMeshGeneratorWriter::getNormalCount() const
{
	NOT_NULL(m_normals);
	return static_cast<int>(m_normals->m_container.size());
}

// ----------------------------------------------------------------------

int SkeletalMeshGeneratorWriter::addStaticHardpoint(const std::string &hardpointName, const std::string &parentName, const Vector &position, const Quaternion &rotation)
{
	return addHardpoint(*m_staticHardpoints, hardpointName, parentName, position, rotation);
}

// ----------------------------------------------------------------------

int SkeletalMeshGeneratorWriter::addDynamicHardpoint(const std::string &hardpointName, const std::string &parentName, const Vector &position, const Quaternion &rotation)
{
	return addHardpoint(*m_dynamicHardpoints, hardpointName, parentName, position, rotation);
}

// ----------------------------------------------------------------------

SkeletalMeshGeneratorWriter::PerShaderData &SkeletalMeshGeneratorWriter::addShaderTemplateByName(const char *name)
{
	NOT_NULL(m_perShaderData);

	const size_t perShaderDataIndex = m_perShaderData->m_container.size();
	m_perShaderData->m_container.resize(perShaderDataIndex + 1);

	PerShaderData &psd = m_perShaderData->m_container[perShaderDataIndex];
	IGNORE_RETURN( psd.m_shaderTemplateName.assign(name) );

	psd.m_index = static_cast<int>(perShaderDataIndex);

	return psd;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::setShaderTextureCoordinateSetCount(PerShaderData &perShaderData, int count) const
{
	perShaderData.m_textureCoordinateSetCount = count;
	
	perShaderData.m_textureCoordinateSetDimensionality.resize(static_cast<size_t>(count));
	perShaderData.m_textureCoordinateSetOffset.resize(static_cast<size_t>(count));

	// default dimensionalities to two
	for (size_t i = 0; i < static_cast<size_t>(count); ++i)
	{
		perShaderData.m_textureCoordinateSetDimensionality[i] = 2;
		perShaderData.m_textureCoordinateSetOffset[i]         = static_cast<int>(i * 2);
	}

	perShaderData.m_textureCoordinatesPerVertex = 2 * count;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::setShaderTextureCoordinateSetDimensionality(PerShaderData &perShaderData, int setIndex, int dimensionality) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, setIndex, perShaderData.m_textureCoordinateSetCount);
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0, dimensionality, 3);

	perShaderData.m_textureCoordinateSetDimensionality[static_cast<size_t>(setIndex)] = dimensionality;

	//-- recalculate offsets
	int currentOffset                           = perShaderData.m_textureCoordinateSetDimensionality[0];
	perShaderData.m_textureCoordinatesPerVertex = perShaderData.m_textureCoordinateSetDimensionality[0];

	for (size_t i = 1; i < static_cast<size_t>(perShaderData.m_textureCoordinateSetCount); ++i)
	{
		perShaderData.m_textureCoordinateSetOffset[i] = currentOffset;
		currentOffset += perShaderData.m_textureCoordinateSetDimensionality[i];

		perShaderData.m_textureCoordinatesPerVertex += perShaderData.m_textureCoordinateSetDimensionality[i];
	}
}

// ----------------------------------------------------------------------

int SkeletalMeshGeneratorWriter::addShaderVertex(PerShaderData &perShaderData, int positionIndex) const
{
	const int vertexIndex = static_cast<int>(perShaderData.m_positionIndices.size());
	
	perShaderData.m_positionIndices.resize(static_cast<size_t>(vertexIndex+1));
	perShaderData.m_positionIndices[static_cast<size_t>(vertexIndex)] = positionIndex;

	// make sure we've got enough texture storage allocated
	const size_t textureCoordinateCount = static_cast<size_t>((vertexIndex+1) * perShaderData.m_textureCoordinatesPerVertex);
	perShaderData.m_textureCoordinates.resize(textureCoordinateCount);

	return vertexIndex;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::setShaderVertexNormal(PerShaderData &perShaderData, int normalIndex) const
{
	perShaderData.m_normalIndices.push_back(normalIndex);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::setShaderVertexDot3ValueIndex(PerShaderData &perShaderData, int dot3VectorIndex) const
{
	perShaderData.m_dot3VectorIndices.push_back(dot3VectorIndex);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::setShaderVertexDiffuseColor(PerShaderData &perShaderData, const PackedArgb &color) const
{
	perShaderData.m_colors.push_back(color);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::setShaderVertexTextureCoordinate(PerShaderData &perShaderData, int vertexIndex, int setIndex, real tc0) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, vertexIndex, static_cast<int>(perShaderData.m_positionIndices.size()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, setIndex, perShaderData.m_textureCoordinateSetCount);
	DEBUG_FATAL(perShaderData.m_textureCoordinateSetDimensionality[static_cast<size_t>(setIndex)] != 1, ("texture coordinate dimensionality mismatch [1/%u]", perShaderData.m_textureCoordinateSetDimensionality[static_cast<size_t>(setIndex)]));

	const size_t baseIndex = static_cast<size_t>(vertexIndex * perShaderData.m_textureCoordinatesPerVertex + perShaderData.m_textureCoordinateSetOffset[static_cast<size_t>(setIndex)]);

	perShaderData.m_textureCoordinates[baseIndex] = tc0;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::setShaderVertexTextureCoordinate(PerShaderData &perShaderData, int vertexIndex, int setIndex, real tc0, real tc1) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, vertexIndex, static_cast<int>(perShaderData.m_positionIndices.size()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, setIndex, perShaderData.m_textureCoordinateSetCount);
	DEBUG_FATAL(perShaderData.m_textureCoordinateSetDimensionality[static_cast<size_t>(setIndex)] != 2, ("texture coordinate dimensionality mismatch [2/%u]", perShaderData.m_textureCoordinateSetDimensionality[static_cast<size_t>(setIndex)]));

	const size_t baseIndex = static_cast<size_t>(vertexIndex * perShaderData.m_textureCoordinatesPerVertex + perShaderData.m_textureCoordinateSetOffset[static_cast<size_t>(setIndex)]);

	perShaderData.m_textureCoordinates[baseIndex + 0] = tc0;
	perShaderData.m_textureCoordinates[baseIndex + 1] = tc1;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::setShaderVertexTextureCoordinate(PerShaderData &perShaderData, int vertexIndex, int setIndex, real tc0, real tc1, real tc2) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, vertexIndex, static_cast<int>(perShaderData.m_positionIndices.size()));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, setIndex, perShaderData.m_textureCoordinateSetCount);
	DEBUG_FATAL(perShaderData.m_textureCoordinateSetDimensionality[static_cast<size_t>(setIndex)] != 3, ("texture coordinate dimensionality mismatch [3/%u]", perShaderData.m_textureCoordinateSetDimensionality[static_cast<size_t>(setIndex)]));

	const size_t baseIndex = static_cast<size_t>(vertexIndex * perShaderData.m_textureCoordinatesPerVertex + perShaderData.m_textureCoordinateSetOffset[static_cast<size_t>(setIndex)]);

	perShaderData.m_textureCoordinates[baseIndex + 0] = tc0;
	perShaderData.m_textureCoordinates[baseIndex + 1] = tc1;
	perShaderData.m_textureCoordinates[baseIndex + 2] = tc2;
}

// ----------------------------------------------------------------------

SkeletalMeshGeneratorWriter::BlendTarget *SkeletalMeshGeneratorWriter::addBlendTarget(const char *variableName)
{
	NOT_NULL(m_blendTargets);
	DEBUG_FATAL(!variableName || !*variableName, ("bad variableName arg"));

	const size_t newBlendTargetIndex = m_blendTargets->m_container.size();
	m_blendTargets->m_container.resize(newBlendTargetIndex + 1);

	BlendTarget &blendTarget   = m_blendTargets->m_container[newBlendTargetIndex];
	blendTarget.m_variableName = variableName;

	return &blendTarget;
}

// ----------------------------------------------------------------------

SkeletalMeshGeneratorWriter::BlendTarget *SkeletalMeshGeneratorWriter::findBlendTarget(const char *variableName) const
{
	BlendTargetContainer::Container::iterator findIt = std::find_if(m_blendTargets->m_container.begin(), m_blendTargets->m_container.end(), BlendTarget::FindByVariableName(variableName));
	if (findIt == m_blendTargets->m_container.end())
		return 0;
	else
		return &(*findIt);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::debugDumpBlendTargets() const
{
#ifdef _DEBUG
	DEBUG_REPORT_LOG(true, ("====\nMGN contains [%d] blend targets:\n", static_cast<int>(m_blendTargets->m_container.size())));

	BlendTargetContainer::Container::iterator const endIt = m_blendTargets->m_container.end();
	for (BlendTargetContainer::Container::iterator it = m_blendTargets->m_container.begin(); it != endIt; ++it)
	{
		BlendTarget const &blendTarget = *it;
		DEBUG_REPORT_LOG(true, ("\t[%s]\n", blendTarget.m_variableName.c_str()));
	}

	DEBUG_REPORT_LOG(true, ("====\n"));
#endif
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::addBlendTargetPosition(BlendTarget *blendTarget, int positionIndex, const Vector &deltaVector) const
{
	NOT_NULL(blendTarget);
	blendTarget->m_positions.push_back(BlendVector(positionIndex, deltaVector));
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::addBlendTargetNormal(BlendTarget *blendTarget, int normalIndex, const Vector &deltaVector) const
{
	NOT_NULL(blendTarget);
	blendTarget->m_normals.push_back(BlendVector(normalIndex, deltaVector));
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::addBlendTargetDot3Value(BlendTarget *blendTarget, int dot3VectorIndex, float deltaX, float deltaY, float deltaZ, float deltaW) const
{
	// This should never happen.  It would indicate a programming error or a serious misunderstanding of the blend data and dot3 vector calculation.
	DEBUG_FATAL(deltaW != 0.0f, ("A blend target is trying to change the dot3 UV direction component."));
	UNREF(deltaW);

	NOT_NULL(blendTarget);
	blendTarget->m_dot3Vectors.push_back(BlendVector(dot3VectorIndex, Vector(deltaX, deltaY, deltaZ)));
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::addBlendTargetHardpointTarget(BlendTarget *blendTarget, int dynamicHardpointIndex, const Vector &deltaPosition, const Quaternion &deltaRotation) const
{
	NOT_NULL(blendTarget);
	blendTarget->addHardpointTarget(dynamicHardpointIndex, deltaPosition, deltaRotation);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::addTextureRenderer(const PerShaderData &perShaderData, const char *textureRendererTemplateName, Tag shaderTextureTag)
{
	CrcLowerString crcName(textureRendererTemplateName);

	//-- see if we already have info on this texture renderer template
	TextureRendererContainer::iterator it = m_textureRenderers->find(&crcName);
	if (it == m_textureRenderers->end())
	{
		TextureRendererData *const newTextureRendererData = new TextureRendererData(textureRendererTemplateName);

		std::pair<TextureRendererContainer::iterator, bool> insertResult = m_textureRenderers->insert(TextureRendererContainer::value_type(&newTextureRendererData->m_textureRendererTemplateName, newTextureRendererData));
		DEBUG_FATAL(!insertResult.second, ("both find and insert returned false"));
		it = insertResult.first;
	} //lint !e429 // newTextureRendererData neither freed nor returned // okay

	TextureRendererData &trd = *NON_NULL(it->second);

	//-- add this <shader, shaderTextureTag> entry.  we'll do a sort/unique on them before we write
	TextureRendererData::Entry  newEntry;

	newEntry.first  = perShaderData.m_index;
	newEntry.second = shaderTextureTag;
	trd.m_entries.push_back(newEntry);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::addOcclusionZone(const char *name)
{
	DEBUG_FATAL(!name || !*name, ("invalid name arg"));

	m_occlusionZoneNames->push_back(boost::shared_ptr<CrcLowerString>(new CrcLowerString(name)));
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::setFullyOccludedZoneCombination(const stdvector<int>::fwd &combination)
{
	DEBUG_FATAL(!m_fullyOccludedZoneCombination->empty(), ("setFullyOccludedZoneCombination() already called"));

	*m_fullyOccludedZoneCombination = combination;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::addOcclusionZoneCombination(const stdvector<int>::fwd &combination)
{
	m_occlusionZoneCombinations->push_back(boost::shared_ptr<IntVector>(new IntVector(combination)));
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::setZonesThisOccludes(const stdvector<int>::fwd &zonesThisOccludes)
{
	DEBUG_FATAL(!m_zonesThisOccludes->empty(), ("setZonesThisOccludes() already called"));

	*m_zonesThisOccludes = zonesThisOccludes;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::beginIndexedTriList(PerShaderData &perShaderData, int triCountHint)
{
	DEBUG_FATAL(m_currentIndexedTriList, ("already within a beginIndexedTriList()/endIndexedTriList() block"));

	//-- create the new primitive
	boost::shared_ptr<IndexedTriListPrimitive> newPrimitive(new IndexedTriListPrimitive());

	//-- add to PerShaderData's list of primitives
	perShaderData.m_primitives.push_back(newPrimitive);

	//-- remember the current primitive
	m_currentIndexedTriList = newPrimitive.get();

	//-- pass size hint along to the class
	newPrimitive->setTriCountHint(triCountHint);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::addIndexedTriListTri(int index0, int index1, int index2)
{
	DEBUG_FATAL(!m_currentIndexedTriList, ("not within a beginIndexedTriList()/endIndexedTriList() block"));
	NOT_NULL(m_currentIndexedTriList);

	m_currentIndexedTriList->addTriangle(index0, index1, index2);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::endIndexedTriList()
{
	DEBUG_FATAL(!m_currentIndexedTriList, ("not within a beginIndexedTriList()/endIndexedTriList() block"));

	m_currentIndexedTriList = 0;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::beginOccludableIndexedTriList(PerShaderData &perShaderData, int triCountHint)
{
	DEBUG_FATAL(m_currentOccludableIndexedTriList, ("already within a beginOccludableIndexedTriList()/endOccludableIndexedTriList() block"));

	//-- create the new primitive
	boost::shared_ptr<OccludableIndexedTriListPrimitive> newPrimitive(new OccludableIndexedTriListPrimitive());

	//-- add to PerShaderData's list of primitives
	perShaderData.m_primitives.push_back(newPrimitive);

	//-- remember the current primitive
	m_currentOccludableIndexedTriList = newPrimitive.get();

	//-- pass size hint along to the class
	newPrimitive->setTriCountHint(triCountHint);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::addOccludableIndexedTriListTri(int occlusionZoneCombinationIndex, int index0, int index1, int index2)
{
	DEBUG_FATAL(!m_currentOccludableIndexedTriList, ("not within a beginOccludableIndexedTriList()/endOccludableIndexedTriList() block"));
	NOT_NULL(m_currentOccludableIndexedTriList);

	m_currentOccludableIndexedTriList->addTriangle(occlusionZoneCombinationIndex, index0, index1, index2);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::endOccludableIndexedTriList()
{
	DEBUG_FATAL(!m_currentOccludableIndexedTriList, ("not within a beginOccludableIndexedTriList()/endOccludableIndexedTriList() block"));

	m_currentOccludableIndexedTriList = 0;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::setLayer(int meshGeneratorLayer)
{
	m_occlusionLayer = meshGeneratorLayer;
}

// ======================================================================

int SkeletalMeshGeneratorWriter::addHardpoint(HardpointVector &hardpoints, const std::string &hardpointName, const std::string &parentName, const Vector &position, const Quaternion &rotation)
{
	//-- check if hardpoint already exists
	CrcLowerString  crcHardpointName(hardpointName.c_str());

	HardpointVector::iterator findIt = std::find_if(hardpoints.begin(), hardpoints.end(), Hardpoint::FindByName(crcHardpointName));
	if (findIt != hardpoints.end())
	{
		// hardpoint did exist.
		WARNING(true, ("hardpoint [%s] already added\n", hardpointName.c_str()));
		return std::distance(hardpoints.begin(), findIt);
	}

	//-- create hardpoint
	const int hardpointIndex = static_cast<int>(hardpoints.size());

	hardpoints.push_back(new Hardpoint(hardpointName, parentName, position, rotation));

	return hardpointIndex;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorWriter::writeHardpoints(Iff &iff, const HardpointVector &hardpoints)
{
	iff.insertChunkData(static_cast<int16>(hardpoints.size()));

	const HardpointVector::const_iterator endIt = hardpoints.end();
	for (HardpointVector::const_iterator it = hardpoints.begin(); it != endIt; ++it)
	{
		const Hardpoint &hardpoint = *(*it);
		NOT_NULL(*it);

		iff.insertChunkString(hardpoint.getHardpointName().getString());
		iff.insertChunkString(hardpoint.getParentName().getString());
		iff.insertChunkFloatQuaternion(hardpoint.getRotation());
		iff.insertChunkFloatVector(hardpoint.getPosition());
	}
}

// ======================================================================
