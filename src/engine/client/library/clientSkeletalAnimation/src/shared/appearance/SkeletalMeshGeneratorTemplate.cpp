// ======================================================================
//
// SkeletalMeshGeneratorTemplate.cpp
// Copyright 2001-2003 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalMeshGeneratorTemplate.h"

#define TRACK_FILL_CALLS 0

#include "boost/smart_ptr.hpp"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsOptionTags.h"
#include "clientGraphics/ShaderTemplate.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientSkeletalAnimation/MeshConstructionHelper.h"
#include "clientSkeletalAnimation/MeshGeneratorTemplateList.h"
#include "clientSkeletalAnimation/OcclusionZoneSet.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/SkeletalAppearanceTemplate.h"
#include "clientSkeletalAnimation/SkeletalMeshGenerator.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientSkeletalAnimation/SoftwareBlendSkeletalShaderPrimitive.h"
#include "clientSkeletalAnimation/TransformNameMap.h"
#include "clientTextureRenderer/TextureRenderer.h"
#include "clientTextureRenderer/TextureRendererList.h"
#include "clientTextureRenderer/TextureRendererShaderPrimitive.h"
#include "clientTextureRenderer/TextureRendererTemplate.h"
#include "clientTextureRenderer/TextureRendererList.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Transform.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/BasicRangedIntCustomizationVariable.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/Object.h"

#include <algorithm>
#include <string>

#if TRACK_FILL_CALLS
#include <map>
#endif

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(SkeletalMeshGeneratorTemplate, true, 0, 0, 0);

// ======================================================================

const int  SkeletalMeshGeneratorTemplate::ms_blendTargetNameSize = 64;

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

bool                                             SkeletalMeshGeneratorTemplate::ms_installed;

MeshConstructionHelper                          *SkeletalMeshGeneratorTemplate::ms_meshConstructionHelper;
SkeletalMeshGeneratorTemplate::IntVector         SkeletalMeshGeneratorTemplate::ms_localToOutputTransformIndices;

SkeletalMeshGeneratorTemplate::VectorVector      SkeletalMeshGeneratorTemplate::ms_dynamicHardpointPositions;
SkeletalMeshGeneratorTemplate::QuaternionVector  SkeletalMeshGeneratorTemplate::ms_dynamicHardpointRotations;

SkeletalMeshGeneratorTemplate::Dot3VectorVector *SkeletalMeshGeneratorTemplate::ms_blendedDot3Vectors;

// ======================================================================

namespace SkeletalMeshGeneratorTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<MeshConstructionHelper::PerShaderData*> MCHPSDContainer; 
	typedef std::vector<int>                                    IntVector;

#if TRACK_FILL_CALLS
	typedef std::map<const SkeletalMeshGeneratorTemplate*, int> TemplateIntMap;
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class DrawPrimitive
	{
	public:

		typedef stdvector<Vector>::fwd  VectorVector;

	public:

		DrawPrimitive() {}
		virtual ~DrawPrimitive() = 0 {}

		virtual void  addPrimitiveData(MeshConstructionHelper &outputMesh, MeshConstructionHelper::PerShaderData *outputMeshPsd, int firstShaderVertexIndex, const std::vector<bool> &combinationOccluded) const = 0;
		virtual void  collectTriangleIndices(IntVector const &psdIndexMap, IntVector &triangleIndices) const = 0;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class IndexedTriListPrimitive: public DrawPrimitive
	{
	public:

		static void install();
		static void remove();

		static void *operator new(size_t size);
		static void  operator delete(void *pointer);

	public:

		IndexedTriListPrimitive();
		virtual ~IndexedTriListPrimitive();

		virtual void  addPrimitiveData(MeshConstructionHelper &outputMesh, MeshConstructionHelper::PerShaderData *outputMeshPsd, int firstShaderVertexIndex, const std::vector<bool> &combinationOccluded) const;
		virtual void  collectTriangleIndices(IntVector const &psdIndexMap, IntVector &triangleIndices) const;

		void          load_0002(Iff &iff, const VectorVector &positions, const std::vector<int> &indexLookup, int &zeroAreaTriCount);

	private:

		static MemoryBlockManager *ms_memoryBlockManager;

	private:

		int              m_triangleCount;
		std::vector<int> m_indices;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class OccludedIndexedTriListPrimitive: public DrawPrimitive
	{
	public:

		static void install();
		static void remove();

		static void *operator new(size_t size);
		static void  operator delete(void *pointer);

	public:

		OccludedIndexedTriListPrimitive();
		virtual ~OccludedIndexedTriListPrimitive();

		virtual void  addPrimitiveData(MeshConstructionHelper &outputMesh, MeshConstructionHelper::PerShaderData *outputMeshPsd, int firstShaderVertexIndex, const std::vector<bool> &combinationOccluded) const;
		virtual void  collectTriangleIndices(IntVector const &psdIndexMap, IntVector &triangleIndices) const;

		void          load_0002(Iff &iff, const VectorVector &positions, const std::vector<int> &indexLookup, int &zeroAreaTriCount);

	private:

		static MemoryBlockManager *ms_memoryBlockManager;

	private:

		int              m_triangleCount;
		std::vector<int> m_indices;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const float ms_applyWeightThreshold           = 0.009f;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


	// scratchpad data needed during construction.

	// -TRF- if we ever handle creating multiple fixed meshes concurrently (multiple creation threads),
	// this has to change.

	MCHPSDContainer   *ms_perShaderDataScratchpad;

	std::vector<bool> *ms_combinationsOccluded;

	char               ms_crashReportInfo[MAX_PATH * 2];

	Iff               *s_asynchronousLoadIff;
}

// ----------------------------------------------------------------------

using namespace SkeletalMeshGeneratorTemplateNamespace;

// ======================================================================
// embedded entity declarations
// ======================================================================

struct SkeletalMeshGeneratorTemplate::BlendVector
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

struct SkeletalMeshGeneratorTemplate::TextureRendererEntry
{
public:

	TextureRendererEntry(int shaderIndex, Tag shaderTextureTag)
		:	m_shaderIndex(shaderIndex),
			m_shaderTextureTag(shaderTextureTag)
		{
		}

public:

	int  m_shaderIndex;
	Tag  m_shaderTextureTag;

private:
	// disabled
	TextureRendererEntry();
};

// ======================================================================

class SkeletalMeshGeneratorTemplate::BlendTarget
{
public:

	static void  install();
	static void  remove();

	static void *operator new(size_t size);
	static void  operator delete(void *data, size_t size);

public:

	BlendTarget();
	~BlendTarget();

	void               load_0001(Iff &iff);
	void               load_0002(Iff &iff);
	void               load_0003(Iff &iff);
	void               load_0004(Iff &iff);

	void               applyPositionDeformation(real weight, Vector *destPositions, int destCount) const;
	void               applyNormalDeformation(real weight, Vector *destNormals, int destCount) const;
	void               applyDot3VectorDeformation(real weight, Dot3VectorVector &dot3Vectors) const;
	void               applyHardpointDeformation(float weight, VectorVector &hardpointPositions, QuaternionVector &hardpointRotations) const;

	const std::string &getCustomizationVariablePathName() const;

private:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class HardpointTarget
	{
	public:

		HardpointTarget(int dynamicHardpointIndex, const Vector &deltaPosition, const Quaternion &deltaRotation);

		void  applyHardpointDeformation(float weight, VectorVector &hardpointPositions, QuaternionVector &hardpointRotations) const;

	private:

		int         m_dynamicHardpointIndex;
		Vector      m_deltaPosition;
		Quaternion  m_deltaRotation;

	private:
		//disabled
		HardpointTarget();
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<BlendVector>     BlendVectorVector;
	typedef std::vector<HardpointTarget> HardpointTargetVector; 

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	static void applyVectorDeformation(real weight, const BlendVectorVector &blendVectors, Vector *destVectors, int destCount);

private:

	static MemoryBlockManager *ms_memoryBlockManager;

private:

	std::string            m_customizationVariablePathName;
	BlendVectorVector      m_positions;
	BlendVectorVector      m_normals;
	BlendVectorVector      m_dot3Vectors;
	HardpointTargetVector *m_hardpointTargets;
};

// ======================================================================

class SkeletalMeshGeneratorTemplate::Hardpoint
{
public:

	static void  install();
	static void  remove();

	static void *operator new(size_t size);
	static void  operator delete(void *data, size_t size);

public:

	Hardpoint(const char *hardpointName, const char *parentName, const Vector &position, const Quaternion &rotation);

	const CrcLowerString &getHardpointName() const;
	const CrcLowerString &getParentName() const;

	const Vector         &getPosition() const;
	const Quaternion     &getRotation() const;

private:

	static MemoryBlockManager *ms_memoryBlockManager;

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

class SkeletalMeshGeneratorTemplate::PerShaderData
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	PerShaderData();
	~PerShaderData();

	void  preloadAssets() const;
	void  garbageCollect() const;

	void  addPerShaderData(MeshConstructionHelper &outputMesh, int firstPositionIndex, int firstNormalIndex, const Dot3VectorVector *dot3Vectors, const std::vector<bool> &combinationOccluded, MeshConstructionHelper::PerShaderData *&outputMeshPsd);
	void  collectTriangleIndices(IntVector &triangleIndices) const;

	void  load_0002(Iff &iff, const VectorVector &positions, int &zeroAreaTriCount);
	void  load_0003(Iff &iff, const VectorVector &positions, int &zeroAreaTriCount);
	void  load_0004(Iff &iff, const VectorVector &positions, int &zeroAreaTriCount);

	bool  isCollidableShaderTemplate() const;

private:

	typedef std::vector<std::vector<float> >  FloatVectorVector;
	typedef std::vector<DrawPrimitive*>       SharedDrawPrimitiveVector;

private:

	std::string                   m_shaderTemplateName;
	mutable ShaderTemplate const *m_shaderTemplate; // used for preloading

	int                        m_vertexCount;
	std::vector<int>           m_positionIndices;
	std::vector<int>           m_normalIndices;
	std::vector<int>           m_dot3VectorIndices;
	std::vector<PackedArgb>    m_diffuseColors;

	std::vector<int>           m_textureCoordinateSetDimensionality;
	FloatVectorVector          m_textureCoordinateSetData;

	SharedDrawPrimitiveVector  m_drawPrimitives;
};

// ======================================================================

struct SkeletalMeshGeneratorTemplate::Dot3Vector
{
public:

	Dot3Vector(float x, float y, float z, float flipState);

private:

	// Disabled.
	Dot3Vector();

public:

	Vector  m_dot3Vector;
	float   m_flipState;
};

// ======================================================================

struct SkeletalMeshGeneratorTemplate::TransformWeightData
{
public:

	TransformWeightData(int transformIndex, float transformWeight);

public:

	int   m_transformIndex;
	float m_transformWeight;

private:
	// disabled
	TransformWeightData();
};

// ======================================================================

struct SkeletalMeshGeneratorTemplate::TextureRendererHeader
{
public:

	TextureRendererHeader(const char *textureRendererTemplateName, int entryCount, int firstEntryIndex);
	~TextureRendererHeader();

	void preloadAssets() const;
	void garbageCollect() const;

public:

	CrcLowerString                         m_textureRendererTemplateName;
	mutable TextureRendererTemplate const *m_textureRendererTemplate;

	int             m_entryCount;
	int             m_firstEntryIndex;	

private:
	// disabled
	TextureRendererHeader();
};

// ======================================================================

MemoryBlockManager *SkeletalMeshGeneratorTemplate::BlendTarget::ms_memoryBlockManager;
MemoryBlockManager *SkeletalMeshGeneratorTemplate::Hardpoint::ms_memoryBlockManager;

// ======================================================================
// class SkeletalMeshGeneratorTemplate::BlendVector
// ======================================================================

inline SkeletalMeshGeneratorTemplate::BlendVector::BlendVector(int index, const Vector &deltaVector)
:	m_index(index),
	m_deltaVector(deltaVector)
{
}

// ======================================================================
// class SkeletalMeshGeneratorTemplate::BlendTarget::HardpointTarget
// ======================================================================

SkeletalMeshGeneratorTemplate::BlendTarget::HardpointTarget::HardpointTarget(int dynamicHardpointIndex, const Vector &deltaPosition, const Quaternion &deltaRotation)
:	m_dynamicHardpointIndex(dynamicHardpointIndex),
	m_deltaPosition(deltaPosition),
	m_deltaRotation(deltaRotation)
{
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::BlendTarget::HardpointTarget::applyHardpointDeformation(float weight, VectorVector &hardpointPositions, QuaternionVector &hardpointRotations) const
{
	// apply the fractional delta position
	hardpointPositions[static_cast<size_t>(m_dynamicHardpointIndex)] += m_deltaPosition * weight;

	// calculate the fractional delta rotation
	//Quaternion  fractionalDeltaRotation;
	const Quaternion fractionalDeltaRotation = Quaternion::identity.slerp(m_deltaRotation, weight);

	// apply the fractional delta rotation
	Quaternion &rotation = hardpointRotations[static_cast<size_t>(m_dynamicHardpointIndex)];
	rotation = fractionalDeltaRotation * rotation;
}

// ======================================================================
// class SkeletalMeshGeneratorTemplate::BlendTarget
// ======================================================================

void SkeletalMeshGeneratorTemplate::BlendTarget::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("already installed"));

	ms_memoryBlockManager = new MemoryBlockManager("SkeletalMeshGeneratorTemplate::BlendTarget", true, sizeof(BlendTarget), 0, 0, 0);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::BlendTarget::remove()
{
	DEBUG_FATAL(!ms_memoryBlockManager, ("not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *SkeletalMeshGeneratorTemplate::BlendTarget::operator new(size_t size)
{
	DEBUG_FATAL(size != sizeof(BlendTarget), ("derivation from BlendTarget not supported"));
	UNREF(size);

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::BlendTarget::operator delete(void *data, size_t size)
{
	DEBUG_FATAL(size != sizeof(BlendTarget), ("derivation from BlendTarget not supported"));
	UNREF(size);

	ms_memoryBlockManager->free(data);
}

// ======================================================================

SkeletalMeshGeneratorTemplate::BlendTarget::BlendTarget() :
	m_customizationVariablePathName(),
	m_positions(),
	m_normals(),
	m_dot3Vectors(),
	m_hardpointTargets(0)
{
}

// ----------------------------------------------------------------------

SkeletalMeshGeneratorTemplate::BlendTarget::~BlendTarget()
{
	delete m_hardpointTargets;
}

// ----------------------------------------------------------------------

inline void SkeletalMeshGeneratorTemplate::BlendTarget::applyPositionDeformation(real weight, Vector *destPositions, int destCount) const
{
	applyVectorDeformation(weight, m_positions, destPositions, destCount);
}

// ----------------------------------------------------------------------

inline void SkeletalMeshGeneratorTemplate::BlendTarget::applyNormalDeformation(real weight, Vector *destNormals, int destCount) const 
{
	applyVectorDeformation(weight, m_normals, destNormals, destCount);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::BlendTarget::applyDot3VectorDeformation(real weight, Dot3VectorVector &dot3Vectors) const
{
	const int destCount = static_cast<int>(dot3Vectors.size());

	const BlendVectorVector::const_iterator itEnd = m_dot3Vectors.end();
	for (BlendVectorVector::const_iterator it = m_dot3Vectors.begin(); it != itEnd; ++it)
	{
		const BlendVector &blendVector = *it;

		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, blendVector.m_index, destCount);
		UNREF(destCount);

		dot3Vectors[static_cast<size_t>(blendVector.m_index)].m_dot3Vector += (blendVector.m_deltaVector * weight);
	}
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::BlendTarget::applyHardpointDeformation(float weight, VectorVector &hardpointPositions, QuaternionVector &hardpointRotations) const
{
	if (m_hardpointTargets)
	{
		const HardpointTargetVector::const_iterator itEnd = m_hardpointTargets->end();
		for (HardpointTargetVector::const_iterator it = m_hardpointTargets->begin(); it != itEnd; ++it)
			it->applyHardpointDeformation(weight, hardpointPositions, hardpointRotations);
	}
}

// ----------------------------------------------------------------------
/**
 * Retrieve the CustomizationData variable pathname for this BlendTarget
 * instance.
 *
 * @return  the CustomizationData variable pathname for this BlendTarget instance .
 */

inline const std::string &SkeletalMeshGeneratorTemplate::BlendTarget::getCustomizationVariablePathName() const
{
	return m_customizationVariablePathName;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::BlendTarget::applyVectorDeformation(real weight, const BlendVectorVector &blendVectors, Vector *destVectors, int destCount)
{
	NOT_NULL(destVectors);

	const BlendVectorVector::const_iterator itEnd = blendVectors.end();
	for (BlendVectorVector::const_iterator it = blendVectors.begin(); it != itEnd; ++it)
	{
		const BlendVector &blendVector = *it;

		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, blendVector.m_index, destCount);
		UNREF(destCount);

		Vector &destVector = destVectors[blendVector.m_index];

		destVector += (blendVector.m_deltaVector * weight);
	}
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::BlendTarget::load_0001(Iff &iff)
{
	iff.enterForm(TAG_BLT);

		//-- load blend target general info
		iff.enterChunk(TAG_INFO);

			char blendTargetName[ms_blendTargetNameSize];

			const int positionCount = iff.read_int32();
			const int normalCount   = iff.read_int32();

			//-- construct customization variable pathname
			//   note: at this point all customization variables are in the shared_owner directory.
			iff.read_string(blendTargetName, sizeof(blendTargetName));
			m_customizationVariablePathName  = "/shared_owner/";
			m_customizationVariablePathName += blendTargetName;

		iff.exitChunk(TAG_INFO);

		//-- load position data
		const bool enteredPosn = iff.enterChunk(TAG_POSN, true);
		DEBUG_FATAL(!enteredPosn && positionCount, ("expecting delta position data"));

		if (enteredPosn)
		{
			m_positions.reserve(static_cast<size_t>(positionCount));

			for (int i = 0; i < positionCount; ++i)
			{
				const int     index  = iff.read_int32();
				const Vector  vector = iff.read_floatVector();

				m_positions.push_back(BlendVector(index, vector));
			}

			iff.exitChunk(TAG_POSN);
		}

		//-- load normal data
		const bool enteredNorm = iff.enterChunk(TAG_NORM, true);
		DEBUG_FATAL(!enteredNorm && normalCount, ("expecting delta position data"));
		if (enteredNorm)
		{
			m_normals.reserve(static_cast<size_t>(normalCount));

			for (int i = 0; i < normalCount; ++i)
			{
				const int     index  = iff.read_int32();
				const Vector  vector = iff.read_floatVector();

				m_normals.push_back(BlendVector(index, vector));
			}

			iff.exitChunk(TAG_NORM);
		}

	iff.exitForm(TAG_BLT);
}

// ----------------------------------------------------------------------

inline void SkeletalMeshGeneratorTemplate::BlendTarget::load_0002(Iff &iff)
{
	// same as 0001 format
	load_0001(iff);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::BlendTarget::load_0003(Iff &iff)
{
	iff.enterForm(TAG_BLT);

		//-- load blend target general info
		iff.enterChunk(TAG_INFO);

			char blendTargetName[ms_blendTargetNameSize];

			const int positionCount = iff.read_int32();
			const int normalCount   = iff.read_int32();

			//-- construct customization variable pathname
			//   note: at this point all customization variables are in the shared_owner directory.
			iff.read_string(blendTargetName, sizeof(blendTargetName));
			m_customizationVariablePathName  = "/shared_owner/";
			m_customizationVariablePathName += blendTargetName;

		iff.exitChunk(TAG_INFO);

		//-- load position data
		const bool enteredPosn = iff.enterChunk(TAG_POSN, true);
		DEBUG_FATAL(!enteredPosn && positionCount, ("expecting delta position data"));

		if (enteredPosn)
		{
			m_positions.reserve(static_cast<size_t>(positionCount));

			for (int i = 0; i < positionCount; ++i)
			{
				const int     index  = iff.read_int32();
				const Vector  vector = iff.read_floatVector();

				m_positions.push_back(BlendVector(index, vector));
			}

			iff.exitChunk(TAG_POSN);
		}

		//-- load normal data
		const bool enteredNorm = iff.enterChunk(TAG_NORM, true);
		DEBUG_FATAL(!enteredNorm && normalCount, ("expecting delta position data"));
		if (enteredNorm)
		{
			m_normals.reserve(static_cast<size_t>(normalCount));

			for (int i = 0; i < normalCount; ++i)
			{
				const int     index  = iff.read_int32();
				const Vector  vector = iff.read_floatVector();

				m_normals.push_back(BlendVector(index, vector));
			}

			iff.exitChunk(TAG_NORM);
		}

		//-- load hardpoint target data (affected hardpoints)
		if (iff.enterChunk(TAG_HPTS, true))
		{
			const int hardpointCount = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(hardpointCount < 1, ("bad hardpoint count %d", hardpointCount));

			// create hardpoint target array
			m_hardpointTargets = new HardpointTargetVector();
			m_hardpointTargets->reserve(static_cast<size_t>(hardpointCount));

			for (int i = 0; i < hardpointCount; ++i)
			{
				const int         dynamicHardpointIndex = static_cast<int>(iff.read_int16());
				const Vector      deltaPosition         = iff.read_floatVector();
				const Quaternion  deltaRotation         = iff.read_floatQuaternion();

				m_hardpointTargets->push_back(HardpointTarget(dynamicHardpointIndex, deltaPosition, deltaRotation));
			}

			iff.exitChunk(TAG_HPTS, true);
		}

	iff.exitForm(TAG_BLT);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::BlendTarget::load_0004(Iff &iff)
{
	iff.enterForm(TAG_BLT);

		//-- load blend target general info
		iff.enterChunk(TAG_INFO);

			char blendTargetName[ms_blendTargetNameSize];

			const int positionCount = iff.read_int32();
			const int normalCount   = iff.read_int32();

			//-- construct customization variable pathname
			//   note: at this point all customization variables are in the shared_owner directory.
			iff.read_string(blendTargetName, sizeof(blendTargetName));
			m_customizationVariablePathName  = "/shared_owner/";
			m_customizationVariablePathName += blendTargetName;

		iff.exitChunk(TAG_INFO);

		//-- load position data
		const bool enteredPosn = iff.enterChunk(TAG_POSN, true);
		DEBUG_FATAL(!enteredPosn && positionCount, ("expecting delta position data"));

		if (enteredPosn)
		{
			m_positions.reserve(static_cast<size_t>(positionCount));

			for (int i = 0; i < positionCount; ++i)
			{
				const int     index  = iff.read_int32();
				const Vector  vector = iff.read_floatVector();

				m_positions.push_back(BlendVector(index, vector));
			}

			iff.exitChunk(TAG_POSN);
		}

		//-- load normal data
		const bool enteredNorm = iff.enterChunk(TAG_NORM, true);
		DEBUG_FATAL(!enteredNorm && normalCount, ("expecting delta position data"));
		if (enteredNorm)
		{
			m_normals.reserve(static_cast<size_t>(normalCount));

			for (int i = 0; i < normalCount; ++i)
			{
				const int     index  = iff.read_int32();
				const Vector  vector = iff.read_floatVector();

				m_normals.push_back(BlendVector(index, vector));
			}

			iff.exitChunk(TAG_NORM);
		}

		//-- Load DOT3 per-pixel-lighting normal map vector.
		if (iff.enterChunk(TAG_DOT3, true))
		{
			// Strip data loading if DOT3 is shut off.
			if (!GraphicsOptionTags::get(TAG_DOT3))
				iff.exitChunk(TAG_DOT3, true);
			else
			{
				const int dot3VectorCount = static_cast<int>(iff.read_int32());

				m_dot3Vectors.reserve(static_cast<size_t>(dot3VectorCount));

				for (int i = 0; i < dot3VectorCount; ++i)
				{
					const int     index  = iff.read_int32();
					const Vector  vector = iff.read_floatVector();

					m_dot3Vectors.push_back(BlendVector(index, vector));
				}

				// @todo change to non-optional end-of-chunk processing when implemented.
				iff.exitChunk(TAG_DOT3, true);
			}
		}

		//-- load hardpoint target data (affected hardpoints)
		if (iff.enterChunk(TAG_HPTS, true))
		{
			const int hardpointCount = static_cast<int>(iff.read_int16());
			DEBUG_FATAL(hardpointCount < 1, ("bad hardpoint count %d", hardpointCount));

			// create hardpoint target array
			IS_NULL(m_hardpointTargets);

			m_hardpointTargets = new HardpointTargetVector();
			m_hardpointTargets->reserve(static_cast<size_t>(hardpointCount));

			for (int i = 0; i < hardpointCount; ++i)
			{
				const int         dynamicHardpointIndex = static_cast<int>(iff.read_int16());
				const Vector      deltaPosition         = iff.read_floatVector();
				const Quaternion  deltaRotation         = iff.read_floatQuaternion();

				m_hardpointTargets->push_back(HardpointTarget(dynamicHardpointIndex, deltaPosition, deltaRotation));
			}

			iff.exitChunk(TAG_HPTS, true);
		}

	iff.exitForm(TAG_BLT);
}

// ======================================================================
// class SkeletalMeshGeneratorTemplate::Hardpoint
// ======================================================================

void SkeletalMeshGeneratorTemplate::Hardpoint::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("already installed"));

	ms_memoryBlockManager = new MemoryBlockManager("SkeletalMeshGeneratorTemplate::Hardpoint", true, sizeof(Hardpoint), 0, 0, 0);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::Hardpoint::remove()
{
	DEBUG_FATAL(!ms_memoryBlockManager, ("not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *SkeletalMeshGeneratorTemplate::Hardpoint::operator new(size_t size)
{
	DEBUG_FATAL(size != sizeof(Hardpoint), ("derivation from Hardpoint not supported"));
	UNREF(size);

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::Hardpoint::operator delete(void *data, size_t size)
{
	DEBUG_FATAL(size != sizeof(Hardpoint), ("derivation from Hardpoint not supported"));
	UNREF(size);

	ms_memoryBlockManager->free(data);
}

// ======================================================================

SkeletalMeshGeneratorTemplate::Hardpoint::Hardpoint(const char *hardpointName, const char *parentName, const Vector &position, const Quaternion &rotation)
:	m_hardpointName(hardpointName),
	m_parentName(parentName),
	m_position(position),
	m_rotation(rotation)
{
}

// ----------------------------------------------------------------------

inline const CrcLowerString &SkeletalMeshGeneratorTemplate::Hardpoint::getHardpointName() const
{
	return m_hardpointName;
}

// ----------------------------------------------------------------------

inline const CrcLowerString &SkeletalMeshGeneratorTemplate::Hardpoint::getParentName() const
{
	return m_parentName;
}

// ----------------------------------------------------------------------

inline const Vector &SkeletalMeshGeneratorTemplate::Hardpoint::getPosition() const
{
	return m_position;
}

// ----------------------------------------------------------------------

inline const Quaternion &SkeletalMeshGeneratorTemplate::Hardpoint::getRotation() const
{
	return m_rotation;
}

// ======================================================================
// class SkeletalMeshGeneratorTemplateNamespace::IndexedTriListPrimitive
// ======================================================================

MemoryBlockManager *SkeletalMeshGeneratorTemplateNamespace::IndexedTriListPrimitive::ms_memoryBlockManager;

// ======================================================================

void SkeletalMeshGeneratorTemplateNamespace::IndexedTriListPrimitive::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("IndexedTriListPrimitive already installed."));
	ms_memoryBlockManager = new MemoryBlockManager("IndexedTriListPrimitive", true, sizeof(IndexedTriListPrimitive), 0, 0, 0);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplateNamespace::IndexedTriListPrimitive::remove()
{
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *SkeletalMeshGeneratorTemplateNamespace::IndexedTriListPrimitive::operator new(size_t size)
{
	UNREF(size);

	DEBUG_FATAL(size != sizeof(IndexedTriListPrimitive), ("Incorrect allocation size for class IndexedTriListPrimitive: %d/%d", size, sizeof(IndexedTriListPrimitive)));
	DEBUG_FATAL(!ms_memoryBlockManager, ("IndexedTriListPrimitive not installed."));
	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplateNamespace::IndexedTriListPrimitive::operator delete(void *pointer)
{
	DEBUG_FATAL(!ms_memoryBlockManager, ("IndexedTriListPrimitive not installed."));
	if (pointer)
		ms_memoryBlockManager->free(pointer);	
}

// ======================================================================

SkeletalMeshGeneratorTemplateNamespace::IndexedTriListPrimitive::IndexedTriListPrimitive()
:	DrawPrimitive(),
	m_triangleCount(0),
	m_indices()
{
}

// ----------------------------------------------------------------------

SkeletalMeshGeneratorTemplateNamespace::IndexedTriListPrimitive::~IndexedTriListPrimitive()
{
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplateNamespace::IndexedTriListPrimitive::load_0002(Iff &iff, const VectorVector &positions, const std::vector<int> &indexLookup, int &zeroAreaTriCount)
{
	Vector  normal;
	int     localZeroAreaTriCount = 0;

	iff.enterChunk(TAG_ITL);

		m_triangleCount = static_cast<int>(iff.read_int32());
		m_indices.reserve(static_cast<size_t>(3 * m_triangleCount));

		for (int i = 0; i < m_triangleCount; ++i)
		{
			//-- Check for zero-area bind-pose triangles.  Note this cannot
			//   handle skinned or blended triangles that skin or blend into
			//   zero-area triangles.

			// Get the triangle coordinates.
			const int index0 = static_cast<int>(iff.read_int32());
			const int index1 = static_cast<int>(iff.read_int32());
			const int index2 = static_cast<int>(iff.read_int32());

			const Vector &v0 = positions[static_cast<VectorVector::size_type>(indexLookup[static_cast<std::vector<int>::size_type>(index0)])];
			const Vector &v1 = positions[static_cast<VectorVector::size_type>(indexLookup[static_cast<std::vector<int>::size_type>(index1)])];
			const Vector &v2 = positions[static_cast<VectorVector::size_type>(indexLookup[static_cast<std::vector<int>::size_type>(index2)])];

			// Compute normal.
			normal = (v0 - v2).cross(v1 - v0);

			if (normal.magnitudeSquared () == 0.0f)
			{
				//-- Do not keep this triangle.
				++localZeroAreaTriCount;
			}
			else
			{
				//-- Keep this triangle.
				m_indices.push_back(index0);
				m_indices.push_back(index1);
				m_indices.push_back(index2);
			}
		}

	iff.exitChunk(TAG_ITL);

	//-- Accumulate total zero are tri counts.
	zeroAreaTriCount += localZeroAreaTriCount;

	//-- Adjust  triangle count for primitive.
	m_triangleCount -= localZeroAreaTriCount;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplateNamespace::IndexedTriListPrimitive::addPrimitiveData(MeshConstructionHelper &outputMesh, MeshConstructionHelper::PerShaderData *outputMeshPsd, int firstShaderVertexIndex, const std::vector<bool> &combinationOccluded) const
{
	UNREF(combinationOccluded);

	MeshConstructionHelper::TriListHeader &tlh = outputMesh.addTriList(outputMeshPsd);

	for (int triIndex = 0; triIndex < m_triangleCount; ++triIndex)
	{
		const size_t baseIndex = static_cast<size_t>(3 * triIndex);

		// lookup our local shader vertex index, then adjust for shader's already-existing vertices
		const int v0 = m_indices[baseIndex + 0] + firstShaderVertexIndex;
		const int v1 = m_indices[baseIndex + 1] + firstShaderVertexIndex;
		const int v2 = m_indices[baseIndex + 2] + firstShaderVertexIndex;

		outputMesh.addTriListTri(outputMeshPsd, tlh, v0, v1, v2);
	}
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplateNamespace::IndexedTriListPrimitive::collectTriangleIndices(IntVector const &psdIndexMap, IntVector &triangleIndices) const
{
	triangleIndices.reserve(triangleIndices.size() + m_indices.size());

	IntVector::const_iterator const endIt = m_indices.end();
	for (IntVector::const_iterator it = m_indices.begin(); it != endIt; ++it)
	{
		int const psdIndex = *it;
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, psdIndex, static_cast<int>(psdIndexMap.size()));
		triangleIndices.push_back(psdIndexMap[static_cast<IntVector::size_type>(psdIndex)]);
	}
}

// ======================================================================
// class SkeletalMeshGeneratorTemplateNamespace::OccludedIndexedTriListPrimitive
// ======================================================================

MemoryBlockManager *SkeletalMeshGeneratorTemplateNamespace::OccludedIndexedTriListPrimitive::ms_memoryBlockManager;

// ======================================================================

void SkeletalMeshGeneratorTemplateNamespace::OccludedIndexedTriListPrimitive::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("OccludedIndexedTriListPrimitive already installed."));
	ms_memoryBlockManager = new MemoryBlockManager("OccludedIndexedTriListPrimitive", true, sizeof(OccludedIndexedTriListPrimitive), 0, 0, 0);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplateNamespace::OccludedIndexedTriListPrimitive::remove()
{
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *SkeletalMeshGeneratorTemplateNamespace::OccludedIndexedTriListPrimitive::operator new(size_t size)
{
	UNREF(size);

	DEBUG_FATAL(size != sizeof(OccludedIndexedTriListPrimitive), ("Incorrect allocation size for class OccludedIndexedTriListPrimitive: %d/%d", size, sizeof(OccludedIndexedTriListPrimitive)));
	DEBUG_FATAL(!ms_memoryBlockManager, ("OccludedIndexedTriListPrimitive not installed."));
	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplateNamespace::OccludedIndexedTriListPrimitive::operator delete(void *pointer)
{
	DEBUG_FATAL(!ms_memoryBlockManager, ("OccludedIndexedTriListPrimitive not installed."));
	if (pointer)
		ms_memoryBlockManager->free(pointer);	
}

// ======================================================================

SkeletalMeshGeneratorTemplateNamespace::OccludedIndexedTriListPrimitive::OccludedIndexedTriListPrimitive()
:	DrawPrimitive(),
	m_triangleCount(0),
	m_indices()
{
}

// ----------------------------------------------------------------------

SkeletalMeshGeneratorTemplateNamespace::OccludedIndexedTriListPrimitive::~OccludedIndexedTriListPrimitive()
{
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplateNamespace::OccludedIndexedTriListPrimitive::load_0002(Iff &iff, const VectorVector &positions, const std::vector<int> &indexLookup, int &zeroAreaTriCount)
{
	Vector  normal;
	int     localZeroAreaTriCount = 0;

	iff.enterChunk(TAG_OITL);

		m_triangleCount = static_cast<int>(iff.read_int32());
		m_indices.reserve(static_cast<size_t>(m_triangleCount) * 4);

		for (int i = 0; i < m_triangleCount; ++i)
		{
			//-- Read occlusion zone combination index.
			const int occlusionZoneCombinationIndex = static_cast<int>(iff.read_int16());

			//-- Check for zero-area bind-pose triangles.  Note this cannot
			//   handle skinned or blended triangles that skin or blend into
			//   zero-area triangles.

			// Get the triangle coordinates.
			const int index0 = static_cast<int>(iff.read_int32());
			const int index1 = static_cast<int>(iff.read_int32());
			const int index2 = static_cast<int>(iff.read_int32());

			const Vector &v0 = positions[static_cast<VectorVector::size_type>(indexLookup[static_cast<std::vector<int>::size_type>(index0)])];
			const Vector &v1 = positions[static_cast<VectorVector::size_type>(indexLookup[static_cast<std::vector<int>::size_type>(index1)])];
			const Vector &v2 = positions[static_cast<VectorVector::size_type>(indexLookup[static_cast<std::vector<int>::size_type>(index2)])];

			// Compute normal.
			normal = (v0 - v2).cross(v1 - v0);

			if (normal.magnitudeSquared () == 0.0f)
			{
				//-- Do not keep this triangle.
				++localZeroAreaTriCount;
			}
			else
			{
				//-- Keep this triangle.
				m_indices.push_back(occlusionZoneCombinationIndex);
				m_indices.push_back(index0);
				m_indices.push_back(index1);
				m_indices.push_back(index2);
			}

		}

	iff.exitChunk(TAG_OITL);

	//-- Accumulate total zero are tri counts.
	zeroAreaTriCount += localZeroAreaTriCount;

	//-- Adjust  triangle count for primitive.
	m_triangleCount -= localZeroAreaTriCount;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplateNamespace::OccludedIndexedTriListPrimitive::addPrimitiveData(MeshConstructionHelper &outputMesh, MeshConstructionHelper::PerShaderData *outputMeshPsd, int firstShaderVertexIndex, const std::vector<bool> &combinationOccluded) const
{
	MeshConstructionHelper::TriListHeader &tlh = outputMesh.addTriList(outputMeshPsd);

	for (int triIndex = 0; triIndex < m_triangleCount; ++triIndex)
	{
		const size_t baseIndex = static_cast<size_t>(4 * triIndex);

		//-- check if triangle is occluded
		const int ozCombinationIndex = m_indices[baseIndex + 0];
		if ((ozCombinationIndex >= 0) && combinationOccluded[static_cast<size_t>(ozCombinationIndex)])
		{
			// this triangle definitely is occluded by something worn over it, skip it
			continue;
		}

		//-- add this potentially visible triangle
		const int v0 = m_indices[baseIndex + 1] + firstShaderVertexIndex;
		const int v1 = m_indices[baseIndex + 2] + firstShaderVertexIndex;
		const int v2 = m_indices[baseIndex + 3] + firstShaderVertexIndex;

		outputMesh.addTriListTri(outputMeshPsd, tlh, v0, v1, v2);
	}
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplateNamespace::OccludedIndexedTriListPrimitive::collectTriangleIndices(IntVector const &psdIndexMap, IntVector &triangleIndices) const
{
	triangleIndices.reserve(triangleIndices.size() + m_indices.size());

	IntVector::const_iterator const endIt = m_indices.end();
	for (IntVector::const_iterator it = m_indices.begin(); it != endIt; ++it)
	{
		int const psdIndex = *it;
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, psdIndex, static_cast<int>(psdIndexMap.size()));
		triangleIndices.push_back(psdIndexMap[static_cast<IntVector::size_type>(psdIndex)]);
	}
}

// ======================================================================
// class SkeletalMeshGeneratorTemplate::PerShaderData
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(SkeletalMeshGeneratorTemplate::PerShaderData, true, 0, 0, 0);

// ======================================================================

SkeletalMeshGeneratorTemplate::PerShaderData::PerShaderData() :
	m_shaderTemplateName(),
	m_shaderTemplate(0),
	m_vertexCount(0),
	m_positionIndices(),
	m_normalIndices(),
	m_dot3VectorIndices(),
	m_diffuseColors(),
	m_textureCoordinateSetDimensionality(),
	m_textureCoordinateSetData(),
	m_drawPrimitives()
{
}

// ----------------------------------------------------------------------

SkeletalMeshGeneratorTemplate::PerShaderData::~PerShaderData()
{
	std::for_each(m_drawPrimitives.begin(), m_drawPrimitives.end(), PointerDeleter());

	if (m_shaderTemplate)
	{
		m_shaderTemplate->release();
		m_shaderTemplate = 0;
	}
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::PerShaderData::load_0002(Iff &iff, const VectorVector &positions, int &zeroAreaTriCount)
{
	int textureCoordinateSetCount = 0;

	iff.enterForm(TAG_PSDT);

		//-- load the shader template name
		iff.enterChunk(TAG_NAME);
		{
			char buffer[MAX_PATH];

			iff.read_string(buffer, sizeof(buffer)-1);
			m_shaderTemplateName = buffer;
		}
		iff.exitChunk(TAG_NAME);

		//-- load the shader vertex position indices (indexes into mesh's positions)
		iff.enterChunk(TAG_PIDX);
		{
			m_vertexCount = static_cast<int>(iff.read_int32());

			m_positionIndices.reserve(static_cast<size_t>(m_vertexCount));
			for (int i = 0; i < m_vertexCount; ++i)
				m_positionIndices.push_back(static_cast<int>(iff.read_int32()));
		}	
		iff.exitChunk(TAG_PIDX);

		//-- load the shader vertex normal indices
		if (iff.enterChunk(TAG_NIDX, true))
		{
			m_normalIndices.reserve(static_cast<size_t>(m_vertexCount));
			for (int i = 0; i < m_vertexCount; ++i)
				m_normalIndices.push_back(static_cast<int>(iff.read_int32()));

			iff.exitChunk(TAG_NIDX);
		}

		//-- load vertex diffuse color info
		if (iff.enterChunk(TAG_VDCL, true))
		{
			m_diffuseColors.reserve(static_cast<size_t>(m_vertexCount));

			for (int i = 0; i < m_vertexCount; ++i)
			{
				const uint8 a = iff.read_uint8();
				const uint8 r = iff.read_uint8();
				const uint8 g = iff.read_uint8();
				const uint8 b = iff.read_uint8();

				m_diffuseColors.push_back(PackedArgb(a,r,g,b));
			}

			iff.exitChunk(TAG_VDCL);
		}

		//-- load texture coordinate info
		if (iff.enterChunk(TAG_TXCI, true))
		{
			textureCoordinateSetCount = static_cast<int>(iff.read_int32());

			m_textureCoordinateSetDimensionality.reserve(static_cast<size_t>(textureCoordinateSetCount));
			for (int i = 0; i < textureCoordinateSetCount; ++i)
				m_textureCoordinateSetDimensionality.push_back(static_cast<int>(iff.read_int32()));

			iff.exitChunk(TAG_TXCI);
		}

		//-- load texture coordinate sets
		if (!iff.enterForm(TAG_TCSF, true))
			DEBUG_FATAL(textureCoordinateSetCount > 0, ("textureCoordinateSetCount > 0 [%d] but no texture coordinate set data form", textureCoordinateSetCount));
		else		
		{
			m_textureCoordinateSetData.resize(static_cast<size_t>(textureCoordinateSetCount));

			// load up each texture coordinate set's data
			for (int i = 0; i < textureCoordinateSetCount; ++i)
			{
				iff.enterChunk(TAG_TCSD);
				{
					//-- size texture coordinate set data's vector
					std::vector<float> &tcSetData = m_textureCoordinateSetData[static_cast<size_t>(i)];

					const size_t arraySize = static_cast<size_t>(m_vertexCount * m_textureCoordinateSetDimensionality[static_cast<size_t>(i)]);
					tcSetData.reserve(arraySize);

					for (size_t j = 0; j < arraySize; ++j)
						tcSetData.push_back(iff.read_float());
				}
				iff.exitChunk(TAG_TCSD);
			}

			iff.exitForm(TAG_TCSF);
		}

		//-- load up shader's draw primitive information
		iff.enterForm(TAG_PRIM);
		{
			iff.enterChunk(TAG_INFO);
				const int primitiveCount = static_cast<int>(iff.read_int32());
			iff.exitChunk(TAG_INFO);

			m_drawPrimitives.reserve(static_cast<size_t>(primitiveCount));

			for (int i = 0; i < primitiveCount; ++i)
			{
				switch (iff.getCurrentName())
				{
					case TAG_ITL:
						{
							IndexedTriListPrimitive *const drawPrimitive = new IndexedTriListPrimitive();
							drawPrimitive->load_0002(iff, positions, m_positionIndices, zeroAreaTriCount);

							m_drawPrimitives.push_back(drawPrimitive);
						}
						break;
					case TAG_OITL:
						{
							OccludedIndexedTriListPrimitive *const drawPrimitive = new OccludedIndexedTriListPrimitive();
							drawPrimitive->load_0002(iff, positions, m_positionIndices, zeroAreaTriCount);

							m_drawPrimitives.push_back(drawPrimitive);
						}
						break;
					default:
						{
							char buffer[5];

							ConvertTagToString(iff.getCurrentName(), buffer);
							FATAL(true, ("unsupported draw primitive [%s]", buffer));
						}
				}
			}
		}
		iff.exitForm(TAG_PRIM);

	iff.exitForm(TAG_PSDT);
}

// ----------------------------------------------------------------------

inline void SkeletalMeshGeneratorTemplate::PerShaderData::load_0003(Iff &iff, const VectorVector &positions, int &zeroAreaTriCount)
{
	// No change from version 2.
	load_0002(iff, positions, zeroAreaTriCount);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::PerShaderData::load_0004(Iff &iff, const VectorVector &positions, int &zeroAreaTriCount)
{
	int textureCoordinateSetCount = 0;

	iff.enterForm(TAG_PSDT);

		//-- load the shader template name
		iff.enterChunk(TAG_NAME);
		{
			char buffer[MAX_PATH];

			iff.read_string(buffer, sizeof(buffer)-1);
			m_shaderTemplateName = buffer;
		}
		iff.exitChunk(TAG_NAME);

		//-- load the shader vertex position indices (indexes into mesh's positions)
		iff.enterChunk(TAG_PIDX);
		{
			m_vertexCount = static_cast<int>(iff.read_int32());

			m_positionIndices.reserve(static_cast<size_t>(m_vertexCount));
			for (int i = 0; i < m_vertexCount; ++i)
				m_positionIndices.push_back(static_cast<int>(iff.read_int32()));
		}	
		iff.exitChunk(TAG_PIDX);

		//-- load the shader vertex normal indices
		if (iff.enterChunk(TAG_NIDX, true))
		{
			m_normalIndices.reserve(static_cast<size_t>(m_vertexCount));
			for (int i = 0; i < m_vertexCount; ++i)
				m_normalIndices.push_back(static_cast<int>(iff.read_int32()));

			iff.exitChunk(TAG_NIDX);
		}

		//-- Load dot3 per-pixel-lighting vectors.
		if (iff.enterChunk(TAG_DOT3, true))
		{
			if (!GraphicsOptionTags::get(TAG_DOT3))
				iff.exitChunk(TAG_DOT3, true);
			else
			{
				m_dot3VectorIndices.reserve(static_cast<size_t>(m_vertexCount));
				for (int i = 0; i < m_vertexCount; ++i)
					m_dot3VectorIndices.push_back(static_cast<int>(iff.read_int32()));

				iff.exitChunk(TAG_DOT3);
			}
		}

		//-- load vertex diffuse color info
		if (iff.enterChunk(TAG_VDCL, true))
		{
			m_diffuseColors.reserve(static_cast<size_t>(m_vertexCount));

			for (int i = 0; i < m_vertexCount; ++i)
			{
				const uint8 a = iff.read_uint8();
				const uint8 r = iff.read_uint8();
				const uint8 g = iff.read_uint8();
				const uint8 b = iff.read_uint8();

				m_diffuseColors.push_back(PackedArgb(a,r,g,b));
			}

			iff.exitChunk(TAG_VDCL);
		}

		//-- load texture coordinate info
		if (iff.enterChunk(TAG_TXCI, true))
		{
			textureCoordinateSetCount = static_cast<int>(iff.read_int32());

			m_textureCoordinateSetDimensionality.reserve(static_cast<size_t>(textureCoordinateSetCount));
			for (int i = 0; i < textureCoordinateSetCount; ++i)
				m_textureCoordinateSetDimensionality.push_back(static_cast<int>(iff.read_int32()));

			iff.exitChunk(TAG_TXCI);
		}

		//-- load texture coordinate sets
		if (!iff.enterForm(TAG_TCSF, true))
			DEBUG_FATAL(textureCoordinateSetCount > 0, ("textureCoordinateSetCount > 0 [%d] but no texture coordinate set data form", textureCoordinateSetCount));
		else		
		{
			m_textureCoordinateSetData.resize(static_cast<size_t>(textureCoordinateSetCount));

			// load up each texture coordinate set's data
			for (int i = 0; i < textureCoordinateSetCount; ++i)
			{
				iff.enterChunk(TAG_TCSD);
				{
					//-- size texture coordinate set data's vector
					std::vector<float> &tcSetData = m_textureCoordinateSetData[static_cast<size_t>(i)];

					const size_t arraySize = static_cast<size_t>(m_vertexCount * m_textureCoordinateSetDimensionality[static_cast<size_t>(i)]);
					tcSetData.reserve(arraySize);

					for (size_t j = 0; j < arraySize; ++j)
						tcSetData.push_back(iff.read_float());
				}
				iff.exitChunk(TAG_TCSD);
			}

			iff.exitForm(TAG_TCSF);
		}

		//-- load up shader's draw primitive information
		iff.enterForm(TAG_PRIM);
		{
			iff.enterChunk(TAG_INFO);
				const int primitiveCount = static_cast<int>(iff.read_int32());
			iff.exitChunk(TAG_INFO);

			m_drawPrimitives.reserve(static_cast<size_t>(primitiveCount));

			for (int i = 0; i < primitiveCount; ++i)
			{
				switch (iff.getCurrentName())
				{
					case TAG_ITL:
						{
							IndexedTriListPrimitive *const drawPrimitive = new IndexedTriListPrimitive();
							drawPrimitive->load_0002(iff, positions, m_positionIndices, zeroAreaTriCount);

							m_drawPrimitives.push_back(drawPrimitive);
						}
						break;
					case TAG_OITL:
						{
							OccludedIndexedTriListPrimitive *const drawPrimitive = new OccludedIndexedTriListPrimitive();
							drawPrimitive->load_0002(iff, positions, m_positionIndices, zeroAreaTriCount);

							m_drawPrimitives.push_back(drawPrimitive);
						}
						break;
					default:
						{
							char buffer[5];

							ConvertTagToString(iff.getCurrentName(), buffer);
							FATAL(true, ("unsupported draw primitive [%s]", buffer));
						}
				}
			}
		}
		iff.exitForm(TAG_PRIM);

	iff.exitForm(TAG_PSDT);
}

// ----------------------------------------------------------------------

bool SkeletalMeshGeneratorTemplate::PerShaderData::isCollidableShaderTemplate() const
{
	if (!m_shaderTemplate)
		preloadAssets();

	if (m_shaderTemplate)
		return m_shaderTemplate->isCollidable();
	else
	{
		WARNING(true, ("SkeletalMeshGeneratorTemplate::PerShaderData::isCollidableShaderTemplate(): could not load shader template [%s], reporting as collidable.", m_shaderTemplateName.c_str()));
		return false;
	}
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::PerShaderData::preloadAssets() const
{
	ShaderTemplate const *const lastShaderTemplate = m_shaderTemplate;

	m_shaderTemplate = ShaderTemplateList::fetch(m_shaderTemplateName.c_str());

	if (lastShaderTemplate)
		lastShaderTemplate->release();
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::PerShaderData::garbageCollect() const
{
	if (m_shaderTemplate)
	{
		m_shaderTemplate->release();
		m_shaderTemplate = NULL;
	}
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::PerShaderData::addPerShaderData(MeshConstructionHelper &outputMesh, int firstPositionIndex, int firstNormalIndex, const Dot3VectorVector *dot3Vectors, const std::vector<bool> &combinationOccluded, MeshConstructionHelper::PerShaderData *&outputMeshPsd)
{
	//-- add the shader template
	outputMeshPsd = outputMesh.addShaderTemplate(m_shaderTemplateName.c_str());

	//-- set shader template parameters
	const uint outputVertexFlags = 
		MeshConstructionHelper::SVF_xyz |
		(m_normalIndices.empty() ? 0 : MeshConstructionHelper::SVF_normal) |
		(m_diffuseColors.empty() ? 0 : MeshConstructionHelper::SVF_argb);

	outputMesh.setVertexFormat(outputMeshPsd, outputVertexFlags);

	//-- set shader texture coordinate info
	const int textureCoordinateSetCount = static_cast<int>(m_textureCoordinateSetDimensionality.size());

	//-- Add dot3 values as another texture coordinate set.
	const bool hasDot3      = !m_dot3VectorIndices.empty() && dot3Vectors;
	const int  dot3SetIndex = textureCoordinateSetCount;

	outputMesh.setTextureCoordinateSetCount(outputMeshPsd, textureCoordinateSetCount + (hasDot3 ? 1 : 0));

	for (int j = 0; j < textureCoordinateSetCount; ++j)
		outputMesh.setTextureCoordinateDimensionality(outputMeshPsd, j, m_textureCoordinateSetDimensionality[static_cast<size_t>(j)]);

	if (hasDot3)
		outputMesh.setTextureCoordinateDimensionality(outputMeshPsd, dot3SetIndex, 4);

	//-- construct shader vertices
	const int firstShaderVertexIndex = outputMesh.getNextNewVertexIndex(outputMeshPsd);
	{
		for (int localVertexIndex = 0; localVertexIndex < m_vertexCount; ++localVertexIndex)
		{
			const int outputVertexIndex = firstShaderVertexIndex + localVertexIndex;

			// positions
			// note: outputVertexIndex is not needed as a parameter because adding a vertex position always creates a new vertex
			//       with shader vertex index = one greater than last.
			outputMesh.addVertexPosition(outputMeshPsd, firstPositionIndex + m_positionIndices[static_cast<size_t>(localVertexIndex)]);

			// normals
			if (!m_normalIndices.empty())
				outputMesh.setVertexNormal(outputMeshPsd, outputVertexIndex, firstNormalIndex + m_normalIndices[static_cast<size_t>(localVertexIndex)]);

			// colors
			if (!m_diffuseColors.empty())
				outputMesh.setVertexColor(outputMeshPsd, outputVertexIndex, m_diffuseColors[static_cast<size_t>(localVertexIndex)]);

			// texture coordinate set data
			for (int tcSetIndex = 0; tcSetIndex < textureCoordinateSetCount; ++tcSetIndex)
			{
				std::vector<float> &tcsData = m_textureCoordinateSetData[static_cast<size_t>(tcSetIndex)];

				switch (m_textureCoordinateSetDimensionality[static_cast<size_t>(tcSetIndex)])
				{
					case 1:
						outputMesh.setTextureCoordinates(outputMeshPsd, outputVertexIndex, tcSetIndex, tcsData[static_cast<size_t>(localVertexIndex)]);
						break;
					
					case 2:
						outputMesh.setTextureCoordinates(outputMeshPsd, outputVertexIndex, tcSetIndex, tcsData[static_cast<size_t>(2*localVertexIndex + 0)], tcsData[static_cast<size_t>(2*localVertexIndex + 1)]);
						break;

					case 3:
						outputMesh.setTextureCoordinates(outputMeshPsd, outputVertexIndex, tcSetIndex, tcsData[static_cast<size_t>(3*localVertexIndex + 0)], tcsData[static_cast<size_t>(3*localVertexIndex + 1)], tcsData[static_cast<size_t>(3*localVertexIndex + 2)]);
						break;

					default:
						FATAL(true, ("unsupported texture coordinate set dimensionality [%d]", m_textureCoordinateSetDimensionality[static_cast<size_t>(tcSetIndex)]));
				}
			}

			// Set dot3 data.
			if (hasDot3)
			{
				const Dot3Vector &dot3Vector = (*dot3Vectors)[static_cast<Dot3VectorVector::size_type>(m_dot3VectorIndices[static_cast<size_t>(localVertexIndex)])];
				outputMesh.setTextureCoordinates(outputMeshPsd, outputVertexIndex, dot3SetIndex, dot3Vector.m_dot3Vector.x, dot3Vector.m_dot3Vector.y, dot3Vector.m_dot3Vector.z, dot3Vector.m_flipState);
			}
		}
	}

	//-- add primitives
	const SharedDrawPrimitiveVector::const_iterator itEnd = m_drawPrimitives.end();
	for (SharedDrawPrimitiveVector::const_iterator it = m_drawPrimitives.begin(); it != itEnd; ++it)
		(*it)->addPrimitiveData(outputMesh, outputMeshPsd, firstShaderVertexIndex, combinationOccluded);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::PerShaderData::collectTriangleIndices(IntVector &triangleIndices) const
{
	SharedDrawPrimitiveVector::const_iterator const itEnd = m_drawPrimitives.end();
	for (SharedDrawPrimitiveVector::const_iterator it = m_drawPrimitives.begin(); it != itEnd; ++it)
	{
		DrawPrimitive const *const drawPrimitive = *it;
		if (drawPrimitive)		
			drawPrimitive->collectTriangleIndices(m_positionIndices, triangleIndices);
	}
}

// ======================================================================
// struct SkeletalMeshGeneratorTemplate::Dot3Vector
// ======================================================================

inline SkeletalMeshGeneratorTemplate::Dot3Vector::Dot3Vector(float x, float y, float z, float flipState) :
	m_dot3Vector(x, y, z),
	m_flipState(flipState)
{
}

// ======================================================================
// struct SkeletalMeshGeneratorTemplate::TextureRendererHeader
// ======================================================================

inline SkeletalMeshGeneratorTemplate::TextureRendererHeader::TextureRendererHeader(const char *textureRendererTemplateName, int entryCount, int firstEntryIndex) :
	m_textureRendererTemplateName(textureRendererTemplateName),
	m_textureRendererTemplate(0),
	m_entryCount(entryCount),
	m_firstEntryIndex(firstEntryIndex)
{
}

// ----------------------------------------------------------------------

SkeletalMeshGeneratorTemplate::TextureRendererHeader::~TextureRendererHeader()
{
	if (m_textureRendererTemplate)
	{
		m_textureRendererTemplate->release();
		m_textureRendererTemplate = 0;
	}
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::TextureRendererHeader::preloadAssets() const
{
	TextureRendererTemplate const *const lastTemplate = m_textureRendererTemplate;

	m_textureRendererTemplate = TextureRendererList::fetch(m_textureRendererTemplateName);

	if (lastTemplate)
		lastTemplate->release();
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::TextureRendererHeader::garbageCollect() const
{
	if (m_textureRendererTemplate)
	{
		m_textureRendererTemplate->release();
		m_textureRendererTemplate = NULL;
	}
}

// ======================================================================
// struct SkeletalMeshGeneratorTemplate::TransformWeightData
// ======================================================================

SkeletalMeshGeneratorTemplate::TransformWeightData::TransformWeightData(int transformIndex, float transformWeight)
:	m_transformIndex(transformIndex),
	m_transformWeight(transformWeight)
{
}

// ======================================================================
// class SkeletalMeshGeneratorTemplate
// ======================================================================

void SkeletalMeshGeneratorTemplate::install()
{
	DEBUG_FATAL(ms_installed, ("SkeletalMeshGeneratorTemplate already installed"));

	installMemoryBlockManager();

	// register this mesh generator
	const bool success = MeshGeneratorTemplateList::registerMeshGeneratorTemplate(TAG_SKMG, create, true, TemporaryCrcString("mgn", false));
	DEBUG_FATAL(!success, ("failed to register SkeletalMeshGeneratorTemplate class"));
	UNREF(success);

	ms_perShaderDataScratchpad  = new MCHPSDContainer;
	ms_combinationsOccluded     = new std::vector<bool>;
	ms_meshConstructionHelper   = new MeshConstructionHelper();
	ms_blendedDot3Vectors       = new Dot3VectorVector();

	// install subsystems
	BlendTarget::install();

	Hardpoint::install();
	PerShaderData::install();
	IndexedTriListPrimitive::install();
	OccludedIndexedTriListPrimitive::install();

	ms_crashReportInfo[0] = '\0';
	CrashReportInformation::addDynamicText(ms_crashReportInfo);

	ms_installed = true;
	ExitChain::add(remove, "SkeletalMeshGeneratorTemplate");
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::remove()
{
	DEBUG_FATAL(!ms_installed, ("SkeletalMeshGeneratorTemplate not installed"));
	ms_installed = false;

	CrashReportInformation::removeDynamicText(ms_crashReportInfo);

	// remove subsystems
	OccludedIndexedTriListPrimitive::remove();
	IndexedTriListPrimitive::remove();
	Hardpoint::remove();
	BlendTarget::remove();

	delete ms_blendedDot3Vectors;
	ms_blendedDot3Vectors = 0;

	delete ms_meshConstructionHelper;
	ms_meshConstructionHelper = 0;

	delete ms_combinationsOccluded;
	ms_combinationsOccluded = 0;

	delete ms_perShaderDataScratchpad;
	ms_perShaderDataScratchpad = 0;

	const bool success = MeshGeneratorTemplateList::deregisterMeshGeneratorTemplate(TAG_SKMG);
	DEBUG_FATAL(!success, ("failed to deregister SkeletalMeshGeneratorTemplate class"));
	UNREF(success);

	removeMemoryBlockManager();
}

// ----------------------------------------------------------------------

MeshGeneratorTemplate *SkeletalMeshGeneratorTemplate::create(Iff *iff, CrcString const &name)
{
	return new SkeletalMeshGeneratorTemplate(iff, name);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::asynchronousLoadCallback(void *data)
{
	NOT_NULL(data);
	reinterpret_cast<SkeletalMeshGeneratorTemplate*>(data)->asynchronousLoadCallback();
}

// ======================================================================

MeshGenerator *SkeletalMeshGeneratorTemplate::createMeshGenerator() const
{
	//-- Check if we should start async loading.
	if (!m_isLoaded)
	{
		// Kick-off load if not already in progress.
		if (!m_asynchronousLoadInProgress)
		{
			if (AsynchronousLoader::isEnabled())
			{
				m_asynchronousLoadInProgress = true;
				AsynchronousLoader::add(getName().getString(), asynchronousLoadCallback, const_cast<SkeletalMeshGeneratorTemplate*>(this));
			}
			else
			{
				//-- Load directly: async loader is disabled.  This is an odd place to get to since
				//   the only way we get here is to be created without the IFF.
				const_cast<SkeletalMeshGeneratorTemplate*>(this)->asynchronousLoadCallback();
			}
		}
	}

	//-- create the mesh generator
	SkeletalMeshGenerator *const meshGenerator = new SkeletalMeshGenerator(this);
	NOT_NULL(meshGenerator);

	//-- Bump up reference count for caller.
	meshGenerator->fetch();

	
	//-- Enqueue the new mesh generator for fixup if we're not loaded.
	if (!m_isLoaded)
	{
		// Create container as needed.
		if (!m_uninitializedMeshGenerators)
			m_uninitializedMeshGenerators = new MeshGeneratorVector;

		// Add newly created mesh generator.
		m_uninitializedMeshGenerators->push_back(meshGenerator);
	}

	return meshGenerator;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::preloadAssets() const
{
	// Preload PerShaderData instance data.
	std::for_each(m_perShaderData.begin(), m_perShaderData.end(), VoidMemberFunction(&PerShaderData::preloadAssets));

	// Preload TextureRendererTemplate data.
	if (m_textureRendererHeaders)
		std::for_each(m_textureRendererHeaders->begin(), m_textureRendererHeaders->end(), VoidMemberFunction(&TextureRendererHeader::preloadAssets));
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::garbageCollect() const
{
	// Preload PerShaderData instance data.
	std::for_each(m_perShaderData.begin(), m_perShaderData.end(), VoidMemberFunction(&PerShaderData::garbageCollect));

	// Preload TextureRendererTemplate data.
	if (m_textureRendererHeaders)
		std::for_each(m_textureRendererHeaders->begin(), m_textureRendererHeaders->end(), VoidMemberFunction(&TextureRendererHeader::garbageCollect));
}

// ----------------------------------------------------------------------

bool SkeletalMeshGeneratorTemplate::hasOnlyNonCollidableShaderTemplates() const
{
	PerShaderDataVector::const_iterator const endIt = m_perShaderData.end();
	for (PerShaderDataVector::const_iterator it = m_perShaderData.begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);
		if ((*it)->isCollidableShaderTemplate())
		{
			// We can collide with at least one shader template: we are not only non-collidable.
			return false;
		}
	}

	//-- If we get here, all shaders are non-collidable or we have no shaders.
	return true;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the number of blend variables that influence the mesh data
 * constructed via this MeshGeneratorTemplate instance.
 *
 * @return  the number of blend variables that influence the mesh data
 *          constructed via this MeshGeneratorTemplate instance.
 */

int SkeletalMeshGeneratorTemplate::getBlendVariableCount() const
{
	if (!m_blendTargets)
		return 0;
	else
		return static_cast<int>(m_blendTargets->size());
}

// ----------------------------------------------------------------------
/**
 * Retrieve the CustomizationData variable name (full pathname) for
 * the given variable index.
 *
 * The variable index must be within the range [0..getBlendVariableCount()).
 *
 * @param index  the index of the blend variable to retrieve.
 *
 * @return       the CustomizationData variable name (full pathname) for
 *               the given variable index.
 */

const std::string &SkeletalMeshGeneratorTemplate::getBlendVariableName(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getBlendVariableCount());
	return (*m_blendTargets)[static_cast<size_t>(index)]->getCustomizationVariablePathName();
}

// ======================================================================

SkeletalMeshGeneratorTemplate::SkeletalMeshGeneratorTemplate(Iff *iff, CrcString const &name) :
	BasicMeshGeneratorTemplate(name),
	m_isLoaded(false),
	m_asynchronousLoadInProgress(false),
	m_uninitializedMeshGenerators(0),
	m_maxTransformsPerVertex(0),
	m_maxTransformsPerShader(0),
	m_skeletonTemplateNames(),
	m_transformNames(),
	m_positions(),
	m_transformWeightCounts(),
	m_transformWeightData(),
	m_normals(),
	m_blendTargets(0),
	m_perShaderData(),
	m_textureRendererHeaders(0),
	m_textureRendererEntries(0),
	m_fullyOccludedZoneCombination(0),
	m_occlusionZoneCombinations(),
	m_zonesThisOccludes(),
	m_occlusionLayer(-1),
	m_staticHardpoints(0),
	m_dynamicHardpoints(0),
	m_dot3Vectors(0)
{
	DEBUG_FATAL(!ms_installed, ("SkeletalMeshGeneratorTemplate not installed"));

	if (iff)
	{
		//-- Store the name of hte most recently loading SkeletalMeshGeneratorTemplate so we can track down live crashes if it fails somewhere in here.
		IGNORE_RETURN(snprintf(ms_crashReportInfo, sizeof(ms_crashReportInfo) - 1, "SkeletalMeshGeneratorTemplate: %s\n", getName().getString()));
		ms_crashReportInfo[sizeof(ms_crashReportInfo) - 1] = '\0';

		load(*iff);
	}
}

// ----------------------------------------------------------------------

SkeletalMeshGeneratorTemplate::~SkeletalMeshGeneratorTemplate()
{
	DEBUG_FATAL(m_uninitializedMeshGenerators && !m_uninitializedMeshGenerators->empty(), ("Destroying SkeletalMeshGeneratorTemplate with existing uninitialized SkeletalMeshGenerator instances."));
	delete m_uninitializedMeshGenerators;

	if (!m_isLoaded)
	{
		//-- Asynchronous load is in progress for this template.  Tell asynchronous loader to abort it.
		AsynchronousLoader::remove(asynchronousLoadCallback, this);
	}

	delete m_dot3Vectors;

	if (m_dynamicHardpoints)
	{
		std::for_each(m_dynamicHardpoints->begin(), m_dynamicHardpoints->end(), PointerDeleter());
		delete m_dynamicHardpoints;
	}

	if (m_staticHardpoints)
	{
		std::for_each(m_staticHardpoints->begin(), m_staticHardpoints->end(), PointerDeleter());
		delete m_staticHardpoints;
	}

	delete m_textureRendererEntries;

	if (m_textureRendererHeaders)
	{
		std::for_each(m_textureRendererHeaders->begin(), m_textureRendererHeaders->end(), PointerDeleter());
		delete m_textureRendererHeaders;
	}

	delete m_fullyOccludedZoneCombination;

	std::for_each(m_perShaderData.begin(), m_perShaderData.end(), PointerDeleter());

	if (m_blendTargets)
	{
		std::for_each(m_blendTargets->begin(), m_blendTargets->end(), PointerDeleter());
		delete m_blendTargets;
	}
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::load(Iff &iff)
{
	DEBUG_FATAL(m_isLoaded, ("attempted to reload SkeletalMeshGeneratorTemplate [%s].", getName().getString()));

	iff.enterForm(TAG_SKMG);

		switch (iff.getCurrentName())
		{
			case TAG_0002:
				load_0002(iff);
				break;
			case TAG_0003:
				load_0003(iff);
				break;
			case TAG_0004:
				load_0004(iff);
				break;
			default:
				{
					char formName[5];
					ConvertTagToString(iff.getCurrentName(), formName);
					DEBUG_FATAL(true, ("Unsupported SkeletalMeshGeneratorTemplate version [%s]", formName));
				}
		}

	iff.exitForm(TAG_SKMG);

	//-- update size of class-static scratch buffers
	{
		const size_t localTransformCount = m_transformNames.size();

		if (localTransformCount > ms_localToOutputTransformIndices.size())
			ms_localToOutputTransformIndices.resize(m_transformNames.size());
	}

	ms_combinationsOccluded->reserve(m_occlusionZoneCombinations.size());

	if (m_dynamicHardpoints)
	{
		const size_t dynamicHardpointCount = m_dynamicHardpoints->size();
		if (dynamicHardpointCount > ms_dynamicHardpointPositions.size())
		{
			ms_dynamicHardpointPositions.resize(dynamicHardpointCount);
			ms_dynamicHardpointRotations.resize(dynamicHardpointCount);
		}
	}

	// Ensure the dot3 vector buffer used during blending is large enough.
	if (m_dot3Vectors)
		if (m_blendTargets && !m_blendTargets->empty())
			ms_blendedDot3Vectors->reserve(m_dot3Vectors->size());

	//-- If asynchronous loading is enabled, we definitely want to load
	//   shader template and texture renderer template assets now.
	//   To generate async data (with async loading disabled) we need to
	//   preload these as well.  Just preload always for now.
	preloadAssets();

	// Remember that we've already loaded.
	m_isLoaded = true;

#if 0
//#ifdef _DEBUG
	if (DataLint::isEnabled())
	{
		ShadowVolumeStreamerDetector detector(getName().getString());

		IndexedTriangleList indexedTriangleList;
		fillIndexedTriangleList(indexedTriangleList);
		detector.addPrimitive(indexedTriangleList);

		detector.detectAndReport();
	}
#endif
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::load_0002(Iff &iff)
{
	int occlusionZoneCount            = 0;
	int occlusionZoneCombinationCount = 0;
	int zonesThisOccludesCount        = 0;

	iff.enterForm(TAG_0002);

		char nameBuffer[MAX_PATH];

		//-- load general info
		iff.enterChunk(TAG_INFO);

			m_maxTransformsPerVertex        = iff.read_int32();
			m_maxTransformsPerShader        = iff.read_int32();

			const int skeletonTemplateNameCount = iff.read_int32();
			const int transformNameCount        = iff.read_int32();
			const int positionCount             = iff.read_int32();
			const int transformWeightDataCount  = iff.read_int32();
			const int normalCount               = iff.read_int32();
			const int perShaderDataCount        = iff.read_int32();
			const int blendTargetCount          = iff.read_int32();

			occlusionZoneCount              = static_cast<int>(iff.read_int16());
			occlusionZoneCombinationCount   = static_cast<int>(iff.read_int16());

			zonesThisOccludesCount          = static_cast<int>(iff.read_int16());

			m_occlusionLayer                = static_cast<int>(iff.read_int16());

		iff.exitChunk(TAG_INFO);

		//-- make sure our PerShaderData scratchpad array has enough space
		NOT_NULL(ms_perShaderDataScratchpad);
		ms_perShaderDataScratchpad->reserve(static_cast<size_t>(perShaderDataCount));

		//-- load skeleton template names required on the skeleton to which this mesh is bound
		iff.enterChunk(TAG_SKTM);
		{
			m_skeletonTemplateNames.reserve(static_cast<size_t>(skeletonTemplateNameCount));

			for (int i = 0; i < skeletonTemplateNameCount; ++i)
			{
				iff.read_string(nameBuffer, sizeof(nameBuffer) - 1);
				m_skeletonTemplateNames.push_back(CrcLowerString(nameBuffer));
			}
		}
		iff.exitChunk(TAG_SKTM);

		//-- load transform names
		iff.enterChunk(TAG_XFNM);
		{
			m_transformNames.reserve(static_cast<size_t>(transformNameCount));

			for (int i = 0; i < transformNameCount; ++i)
			{
				iff.read_string(nameBuffer, sizeof(nameBuffer) - 1);
				m_transformNames.push_back(CrcLowerString(nameBuffer));
			}
		}
		iff.exitChunk(TAG_XFNM);

		//-- load position vectors
		iff.enterChunk(TAG_POSN);
		{
			m_positions.reserve(static_cast<size_t>(positionCount));

			for (int i = 0; i < positionCount; ++i)
				m_positions.push_back(iff.read_floatVector());
		}
		iff.exitChunk(TAG_POSN);

		//-- load transform weighting header information
		iff.enterChunk(TAG_TWHD);
		{
			m_transformWeightCounts.reserve(static_cast<size_t>(positionCount));

			for (int i = 0; i < positionCount; ++i)
				m_transformWeightCounts.push_back(iff.read_int32());
		}
		iff.exitChunk(TAG_TWHD);

		//-- load transform weighting data
		iff.enterChunk(TAG_TWDT);
		{
			m_transformWeightData.reserve(static_cast<size_t>(transformWeightDataCount));

			for (int i = 0; i < transformWeightDataCount; ++i)
			{
				const int   transformIndex  = static_cast<int>(iff.read_int32());
				const float transformWeight = iff.read_float();

				m_transformWeightData.push_back(TransformWeightData(transformIndex, transformWeight));
			}
		}
		iff.exitChunk(TAG_TWDT);

		//-- load normal vectors
		const bool enteredNormalChunk = iff.enterChunk(TAG_NORM, true);
		DEBUG_FATAL(!enteredNormalChunk && (normalCount > 0), ("no normal chunk but normal count set to [%d]", normalCount));

		if (enteredNormalChunk)
		{
			m_normals.reserve(static_cast<size_t>(normalCount));

			for (int i = 0; i < normalCount; ++i)
				m_normals.push_back(iff.read_floatVector());

			iff.exitChunk(TAG_NORM);
		}

		//-- load blend targets
		if (blendTargetCount)
		{
			iff.enterForm(TAG_BLTS);
			{
				m_blendTargets = new BlendTargetVector();
				m_blendTargets->reserve(static_cast<size_t>(blendTargetCount));

				for (int i = 0; i < blendTargetCount; ++i)
				{
					m_blendTargets->push_back(new BlendTarget());
					m_blendTargets->back()->load_0002(iff);
				}
			}
			iff.exitForm(TAG_BLTS);
		}

		//-- load occlusion zone names referenced by the mesh
		std::vector<int> occlusionZoneIds;

		if (occlusionZoneCount)
		{
			//-- load occlusion zone names
			std::vector<boost::shared_ptr<CrcLowerString> >  occlusionZoneNames;

			occlusionZoneNames.reserve(static_cast<size_t>(occlusionZoneCount));

			iff.enterChunk(TAG_OZN);
			{
				for (int i = 0; i < occlusionZoneCount; ++i)
				{
					char buffer[1024];
					iff.read_string(buffer, sizeof(buffer)-1);

					occlusionZoneNames.push_back(boost::shared_ptr<CrcLowerString>(new CrcLowerString(buffer)));
				}
			}
			iff.exitChunk(TAG_OZN);

			//-- get ids for occlusion zone names
			OcclusionZoneSet::registerOcclusionZones(occlusionZoneNames, occlusionZoneIds);
		}

		//-- load fully occluded zone combination
		if (iff.enterChunk(TAG_FOZC, true))
		{
			m_fullyOccludedZoneCombination = new IntVector();

			const size_t count = static_cast<size_t>(iff.read_uint16());
			m_fullyOccludedZoneCombination->reserve(count);

			for (size_t i = 0; i < count; ++i)
			{
				//-- get occlusion zone index relative to mesh's OZ name list
				const int localOzIndex  = static_cast<int>(iff.read_int16());
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, localOzIndex, static_cast<int>(occlusionZoneIds.size()));

				//-- convert local OZ index into system-wide OZ index
				const int systemOzIndex = occlusionZoneIds[static_cast<size_t>(localOzIndex)];
				m_fullyOccludedZoneCombination->push_back(systemOzIndex);
			}

			iff.exitChunk(TAG_FOZC);
		}

		//-- load occlusion zone combinations
		if (occlusionZoneCombinationCount)
		{
			m_occlusionZoneCombinations.resize(static_cast<size_t>(occlusionZoneCombinationCount));

			iff.enterChunk(TAG_OZC);
			{
				for (int i = 0; i < occlusionZoneCombinationCount; ++i)
				{
					//-- select the OZ combination array, size it properly
					const int         combinationZoneCount = static_cast<int>(iff.read_int16());
					std::vector<int> &combination          = m_occlusionZoneCombinations[static_cast<size_t>(i)];

					combination.reserve(static_cast<size_t>(combinationZoneCount));

					for (int ozIndex = 0; ozIndex < combinationZoneCount; ++ozIndex)
					{
						//-- get occlusion zone index relative to mesh's OZ name list
						const int localOzIndex  = static_cast<int>(iff.read_int16());
						VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, localOzIndex, static_cast<int>(occlusionZoneIds.size()));

						//-- convert local OZ index into system-wide OZ index
						const int systemOzIndex = occlusionZoneIds[static_cast<size_t>(localOzIndex)];
						combination.push_back(systemOzIndex);
					}
				}
			}
			iff.exitChunk(TAG_OZC);
		}

		//-- load occlusion zones that this mesh occludes
		if (zonesThisOccludesCount)
		{
			iff.enterChunk(TAG_ZTO);

				m_zonesThisOccludes.reserve(static_cast<size_t>(zonesThisOccludesCount));

				for (int i = 0; i < zonesThisOccludesCount; ++i)
				{
					//-- get occlusion zone index relative to mesh's OZ name list
					const int localOzIndex  = static_cast<int>(iff.read_int16());
					VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, localOzIndex, static_cast<int>(occlusionZoneIds.size()));

					//-- convert local OZ index into system-wide OZ index
					const int systemOzIndex = occlusionZoneIds[static_cast<size_t>(localOzIndex)];
					m_zonesThisOccludes.push_back(systemOzIndex);
				}

			iff.exitChunk(TAG_ZTO);
		}

		//-- load per shader data
		{
			int  zeroAreaTriCount = 0;

			m_perShaderData.reserve(static_cast<size_t>(perShaderDataCount));

			for (size_t i = 0; i < static_cast<size_t>(perShaderDataCount); ++i)
			{
				m_perShaderData.push_back(new PerShaderData());
				m_perShaderData.back()->load_0002(iff, m_positions, zeroAreaTriCount);
			}

			DEBUG_WARNING(zeroAreaTriCount > 0, ("Asset [%s] has [%d] zero-area bind-pose triangles.", getName().getString(), zeroAreaTriCount));
		}

		//-- load info about texture renderer templates
		if (iff.enterForm(TAG_TRTS, true))
		{
			iff.enterChunk(TAG_INFO);
				const int headerCount  = iff.read_int32();
				const int entryCount   = iff.read_int32();
			iff.exitChunk(TAG_INFO);

			DEBUG_FATAL(m_textureRendererHeaders, ("m_textureRendererHeaders not null, unexpected"));
			m_textureRendererHeaders = new TRHeaderContainer;
			m_textureRendererHeaders->reserve(static_cast<size_t>(headerCount));

			DEBUG_FATAL(m_textureRendererEntries, ("m_textureRendererEntries not null, unexpected"));
			m_textureRendererEntries = new TREntryContainer;
			m_textureRendererEntries->reserve(static_cast<size_t>(entryCount));

			int currentEntryIndex = 0;

			for (int i = 0; i < headerCount; ++i)
			{
				iff.enterChunk(TAG_TRT);
				
					//-- get the texture renderer name and # affected shaders
					char textureRendererTemplateName[MAX_PATH];
					
					iff.read_string(textureRendererTemplateName, sizeof(textureRendererTemplateName)-1);
					const int affectedShaderCount = iff.read_int32();

					//-- save this header info
					m_textureRendererHeaders->push_back(new TextureRendererHeader(textureRendererTemplateName, affectedShaderCount, currentEntryIndex));

					//-- load info on the affected shaders
					for (int affectedShaderIndex = 0; affectedShaderIndex < affectedShaderCount; ++affectedShaderIndex, ++currentEntryIndex)
					{
						const int shaderIndex      = iff.read_int32();
						const Tag shaderTextureTag = static_cast<Tag>(iff.read_uint32());
						m_textureRendererEntries->push_back(TextureRendererEntry(shaderIndex, shaderTextureTag));
					}

				iff.exitChunk(TAG_TRT);
			}

			iff.exitForm(TAG_TRTS);
		}

	iff.exitForm(TAG_0002);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::load_0003(Iff &iff)
{
	int occlusionZoneCount            = 0;
	int occlusionZoneCombinationCount = 0;
	int zonesThisOccludesCount        = 0;

	iff.enterForm(TAG_0003);

		char nameBuffer[MAX_PATH];

		//-- load general info
		iff.enterChunk(TAG_INFO);

			m_maxTransformsPerVertex        = iff.read_int32();
			m_maxTransformsPerShader        = iff.read_int32();

			const int skeletonTemplateNameCount = iff.read_int32();
			const int transformNameCount        = iff.read_int32();
			const int positionCount             = iff.read_int32();
			const int transformWeightDataCount  = iff.read_int32();
			const int normalCount               = iff.read_int32();
			const int perShaderDataCount        = iff.read_int32();
			const int blendTargetCount          = iff.read_int32();

			occlusionZoneCount              = static_cast<int>(iff.read_int16());
			occlusionZoneCombinationCount   = static_cast<int>(iff.read_int16());

			zonesThisOccludesCount          = static_cast<int>(iff.read_int16());

			m_occlusionLayer                = static_cast<int>(iff.read_int16());

		iff.exitChunk(TAG_INFO);

		//-- make sure our PerShaderData scratchpad array has enough space
		NOT_NULL(ms_perShaderDataScratchpad);
		ms_perShaderDataScratchpad->reserve(static_cast<size_t>(perShaderDataCount));

		//-- load skeleton template names required on the skeleton to which this mesh is bound
		iff.enterChunk(TAG_SKTM);
		{
			m_skeletonTemplateNames.reserve(static_cast<size_t>(skeletonTemplateNameCount));

			for (int i = 0; i < skeletonTemplateNameCount; ++i)
			{
				iff.read_string(nameBuffer, sizeof(nameBuffer) - 1);
				m_skeletonTemplateNames.push_back(CrcLowerString(nameBuffer));
			}
		}
		iff.exitChunk(TAG_SKTM);

		//-- load transform names
		iff.enterChunk(TAG_XFNM);
		{
			m_transformNames.reserve(static_cast<size_t>(transformNameCount));

			for (int i = 0; i < transformNameCount; ++i)
			{
				iff.read_string(nameBuffer, sizeof(nameBuffer) - 1);
				m_transformNames.push_back(CrcLowerString(nameBuffer));
			}
		}
		iff.exitChunk(TAG_XFNM);

		//-- load position vectors
		iff.enterChunk(TAG_POSN);
		{
			m_positions.reserve(static_cast<size_t>(positionCount));

			for (int i = 0; i < positionCount; ++i)
				m_positions.push_back(iff.read_floatVector());
		}
		iff.exitChunk(TAG_POSN);

		//-- load transform weighting header information
		iff.enterChunk(TAG_TWHD);
		{
			m_transformWeightCounts.reserve(static_cast<size_t>(positionCount));

			for (int i = 0; i < positionCount; ++i)
				m_transformWeightCounts.push_back(iff.read_int32());
		}
		iff.exitChunk(TAG_TWHD);

		//-- load transform weighting data
		iff.enterChunk(TAG_TWDT);
		{
			m_transformWeightData.reserve(static_cast<size_t>(transformWeightDataCount));

			for (int i = 0; i < transformWeightDataCount; ++i)
			{
				const int   transformIndex  = static_cast<int>(iff.read_int32());
				const float transformWeight = iff.read_float();

				m_transformWeightData.push_back(TransformWeightData(transformIndex, transformWeight));
			}
		}
		iff.exitChunk(TAG_TWDT);

		//-- load normal vectors
		const bool enteredNormalChunk = iff.enterChunk(TAG_NORM, true);
		DEBUG_FATAL(!enteredNormalChunk && (normalCount > 0), ("no normal chunk but normal count set to [%d]", normalCount));

		if (enteredNormalChunk)
		{
			m_normals.reserve(static_cast<size_t>(normalCount));

			for (int i = 0; i < normalCount; ++i)
				m_normals.push_back(iff.read_floatVector());

			iff.exitChunk(TAG_NORM);
		}

		//-- load hardpoints
		if (iff.enterForm(TAG_HPTS, true))
		{
			//-- handle static (non-morphable) hardpoints
			if (iff.enterChunk(TAG_STAT, true))
			{
				m_staticHardpoints = new HardpointVector();
				loadHardpoints(iff, *m_staticHardpoints);

				iff.exitChunk(TAG_STAT);
			}

			//-- handle dynamic (morphable) hardpoints
			if (iff.enterChunk(TAG_DYN, true))
			{
				m_dynamicHardpoints = new HardpointVector();
				loadHardpoints(iff, *m_dynamicHardpoints);

				iff.exitChunk(TAG_DYN);
			}

			iff.exitForm(TAG_HPTS);
		}

		//-- load blend targets
		if (blendTargetCount)
		{
			iff.enterForm(TAG_BLTS);
			{
				if (!m_blendTargets)
					m_blendTargets = new BlendTargetVector();
				else
				{
					std::for_each(m_blendTargets->begin(), m_blendTargets->end(), PointerDeleter());
					m_blendTargets->clear();
				}

				m_blendTargets->reserve(static_cast<size_t>(blendTargetCount));

				for (int i = 0; i < blendTargetCount; ++i)
				{
					m_blendTargets->push_back(new BlendTarget());
					m_blendTargets->back()->load_0003(iff);
				}
			}
			iff.exitForm(TAG_BLTS);
		}

		//-- load occlusion zone names referenced by the mesh
		std::vector<int> occlusionZoneIds;

		if (occlusionZoneCount)
		{
			//-- load occlusion zone names
			std::vector<boost::shared_ptr<CrcLowerString> >  occlusionZoneNames;

			occlusionZoneNames.reserve(static_cast<size_t>(occlusionZoneCount));

			iff.enterChunk(TAG_OZN);
			{
				for (int i = 0; i < occlusionZoneCount; ++i)
				{
					char buffer[1024];
					iff.read_string(buffer, sizeof(buffer)-1);

					occlusionZoneNames.push_back(boost::shared_ptr<CrcLowerString>(new CrcLowerString(buffer)));
				}
			}
			iff.exitChunk(TAG_OZN);

			//-- get ids for occlusion zone names
			OcclusionZoneSet::registerOcclusionZones(occlusionZoneNames, occlusionZoneIds);
		}

		//-- load fully occluded zone combination
		if (iff.enterChunk(TAG_FOZC, true))
		{
			if (!m_fullyOccludedZoneCombination)
				m_fullyOccludedZoneCombination = new IntVector();
			else
				m_fullyOccludedZoneCombination->clear();

			const size_t count = static_cast<size_t>(iff.read_uint16());
			m_fullyOccludedZoneCombination->reserve(count);

			for (size_t i = 0; i < count; ++i)
			{
				//-- get occlusion zone index relative to mesh's OZ name list
				const int localOzIndex  = static_cast<int>(iff.read_int16());
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, localOzIndex, static_cast<int>(occlusionZoneIds.size()));

				//-- convert local OZ index into system-wide OZ index
				const int systemOzIndex = occlusionZoneIds[static_cast<size_t>(localOzIndex)];
				m_fullyOccludedZoneCombination->push_back(systemOzIndex);
			}

			iff.exitChunk(TAG_FOZC);
		}

		//-- load occlusion zone combinations
		if (occlusionZoneCombinationCount)
		{
			m_occlusionZoneCombinations.resize(static_cast<size_t>(occlusionZoneCombinationCount));

			iff.enterChunk(TAG_OZC);
			{
				for (int i = 0; i < occlusionZoneCombinationCount; ++i)
				{
					//-- select the OZ combination array, size it properly
					const int         combinationZoneCount = static_cast<int>(iff.read_int16());
					std::vector<int> &combination          = m_occlusionZoneCombinations[static_cast<size_t>(i)];

					combination.reserve(static_cast<size_t>(combinationZoneCount));

					for (int ozIndex = 0; ozIndex < combinationZoneCount; ++ozIndex)
					{
						//-- get occlusion zone index relative to mesh's OZ name list
						const int localOzIndex  = static_cast<int>(iff.read_int16());
						VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, localOzIndex, static_cast<int>(occlusionZoneIds.size()));

						//-- convert local OZ index into system-wide OZ index
						const int systemOzIndex = occlusionZoneIds[static_cast<size_t>(localOzIndex)];
						combination.push_back(systemOzIndex);
					}
				}
			}
			iff.exitChunk(TAG_OZC);
		}

		//-- load occlusion zones that this mesh occludes
		if (zonesThisOccludesCount)
		{
			iff.enterChunk(TAG_ZTO);

				m_zonesThisOccludes.reserve(static_cast<size_t>(zonesThisOccludesCount));

				for (int i = 0; i < zonesThisOccludesCount; ++i)
				{
					//-- get occlusion zone index relative to mesh's OZ name list
					const int localOzIndex  = static_cast<int>(iff.read_int16());
					VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, localOzIndex, static_cast<int>(occlusionZoneIds.size()));

					//-- convert local OZ index into system-wide OZ index
					const int systemOzIndex = occlusionZoneIds[static_cast<size_t>(localOzIndex)];
					m_zonesThisOccludes.push_back(systemOzIndex);
				}

			iff.exitChunk(TAG_ZTO);
		}

		//-- load per shader data
		{
			int zeroAreaTriCount = 0;

			m_perShaderData.reserve(static_cast<size_t>(perShaderDataCount));

			for (size_t i = 0; i < static_cast<size_t>(perShaderDataCount); ++i)
			{
				m_perShaderData.push_back(new PerShaderData());
				m_perShaderData.back()->load_0003(iff, m_positions, zeroAreaTriCount);
			}

			DEBUG_WARNING(zeroAreaTriCount > 0, ("Asset [%s] has [%d] zero-area bind-pose triangles.", getName().getString(), zeroAreaTriCount));
		}

		//-- load info about texture renderer templates
		if (iff.enterForm(TAG_TRTS, true))
		{
			iff.enterChunk(TAG_INFO);
				const int headerCount  = iff.read_int32();
				const int entryCount   = iff.read_int32();
			iff.exitChunk(TAG_INFO);

			if (!m_textureRendererHeaders)
				m_textureRendererHeaders = new TRHeaderContainer;
			else
			{
				std::for_each(m_textureRendererHeaders->begin(), m_textureRendererHeaders->end(), PointerDeleter());
				m_textureRendererHeaders->clear();
			}

			m_textureRendererHeaders->reserve(static_cast<size_t>(headerCount));

			if (!m_textureRendererEntries)
				m_textureRendererEntries = new TREntryContainer;
			else
				m_textureRendererEntries->clear();

			m_textureRendererEntries->reserve(static_cast<size_t>(entryCount));

			int currentEntryIndex = 0;

			for (int i = 0; i < headerCount; ++i)
			{
				iff.enterChunk(TAG_TRT);
				
					//-- get the texture renderer name and # affected shaders
					char textureRendererTemplateName[MAX_PATH];
					
					iff.read_string(textureRendererTemplateName, sizeof(textureRendererTemplateName)-1);
					const int affectedShaderCount = iff.read_int32();

					//-- save this header info
					m_textureRendererHeaders->push_back(new TextureRendererHeader(textureRendererTemplateName, affectedShaderCount, currentEntryIndex));

					//-- load info on the affected shaders
					for (int affectedShaderIndex = 0; affectedShaderIndex < affectedShaderCount; ++affectedShaderIndex, ++currentEntryIndex)
					{
						const int shaderIndex      = iff.read_int32();
						const Tag shaderTextureTag = static_cast<Tag>(iff.read_uint32());
						m_textureRendererEntries->push_back(TextureRendererEntry(shaderIndex, shaderTextureTag));
					}

				iff.exitChunk(TAG_TRT);
			}

			iff.exitForm(TAG_TRTS);
		}

	iff.exitForm(TAG_0003);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::load_0004(Iff &iff)
{
	int occlusionZoneCount            = 0;
	int occlusionZoneCombinationCount = 0;
	int zonesThisOccludesCount        = 0;

	iff.enterForm(TAG_0004);

		char nameBuffer[MAX_PATH];

		//-- load general info
		iff.enterChunk(TAG_INFO);

			m_maxTransformsPerVertex        = iff.read_int32();
			m_maxTransformsPerShader        = iff.read_int32();

			const int skeletonTemplateNameCount = iff.read_int32();
			const int transformNameCount        = iff.read_int32();
			const int positionCount             = iff.read_int32();
			const int transformWeightDataCount  = iff.read_int32();
			const int normalCount               = iff.read_int32();
			const int perShaderDataCount        = iff.read_int32();
			const int blendTargetCount          = iff.read_int32();

			occlusionZoneCount              = static_cast<int>(iff.read_int16());
			occlusionZoneCombinationCount   = static_cast<int>(iff.read_int16());

			zonesThisOccludesCount          = static_cast<int>(iff.read_int16());

			m_occlusionLayer                = static_cast<int>(iff.read_int16());

		iff.exitChunk(TAG_INFO);

		//-- make sure our PerShaderData scratchpad array has enough space
		NOT_NULL(ms_perShaderDataScratchpad);
		ms_perShaderDataScratchpad->reserve(static_cast<size_t>(perShaderDataCount));

		//-- load skeleton template names required on the skeleton to which this mesh is bound
		iff.enterChunk(TAG_SKTM);
		{
			m_skeletonTemplateNames.reserve(static_cast<size_t>(skeletonTemplateNameCount));

			for (int i = 0; i < skeletonTemplateNameCount; ++i)
			{
				iff.read_string(nameBuffer, sizeof(nameBuffer) - 1);
				m_skeletonTemplateNames.push_back(CrcLowerString(nameBuffer));
			}
		}
		iff.exitChunk(TAG_SKTM);

		//-- load transform names
		iff.enterChunk(TAG_XFNM);
		{
			m_transformNames.reserve(static_cast<size_t>(transformNameCount));

			for (int i = 0; i < transformNameCount; ++i)
			{
				iff.read_string(nameBuffer, sizeof(nameBuffer) - 1);
				m_transformNames.push_back(CrcLowerString(nameBuffer));
			}
		}
		iff.exitChunk(TAG_XFNM);

		//-- load position vectors
		iff.enterChunk(TAG_POSN);
		{
			m_positions.reserve(static_cast<size_t>(positionCount));

			for (int i = 0; i < positionCount; ++i)
				m_positions.push_back(iff.read_floatVector());
		}
		iff.exitChunk(TAG_POSN);

		//-- load transform weighting header information
		iff.enterChunk(TAG_TWHD);
		{
			m_transformWeightCounts.reserve(static_cast<size_t>(positionCount));

			for (int i = 0; i < positionCount; ++i)
				m_transformWeightCounts.push_back(iff.read_int32());
		}
		iff.exitChunk(TAG_TWHD);

		//-- load transform weighting data
		iff.enterChunk(TAG_TWDT);
		{
			m_transformWeightData.reserve(static_cast<size_t>(transformWeightDataCount));

			for (int i = 0; i < transformWeightDataCount; ++i)
			{
				const int   transformIndex  = static_cast<int>(iff.read_int32());
				const float transformWeight = iff.read_float();

				m_transformWeightData.push_back(TransformWeightData(transformIndex, transformWeight));
			}
		}
		iff.exitChunk(TAG_TWDT);

		//-- load normal vectors
		const bool enteredNormalChunk = iff.enterChunk(TAG_NORM, true);
		DEBUG_FATAL(!enteredNormalChunk && (normalCount > 0), ("no normal chunk but normal count set to [%d]", normalCount));

		if (enteredNormalChunk)
		{
			m_normals.reserve(static_cast<size_t>(normalCount));

			for (int i = 0; i < normalCount; ++i)
				m_normals.push_back(iff.read_floatVector());

			iff.exitChunk(TAG_NORM);
		}

		//-- Load dot3 per-pixel-lighting vectors.
		if (iff.enterChunk(TAG_DOT3, true))
		{
			if (!GraphicsOptionTags::get(TAG_DOT3))
				iff.exitChunk(TAG_DOT3, true);
			else
			{
				// Get # dot3 vectors.
				const int count = static_cast<int>(iff.read_int32());
				DEBUG_FATAL(count < 0, ("Bad data, dot3 vertex count [%d].", count));

				// Create dot3 vector storage.
				IS_NULL(m_dot3Vectors);
				m_dot3Vectors = new Dot3VectorVector();

				m_dot3Vectors->reserve(static_cast<Dot3VectorVector::size_type>(count));

				// Load the vectors.
				float x;
				float y;
				float z;
				float flipState;

				for (int i = 0; i < count; ++i)
				{
					// Load components.
					x         = iff.read_float();
					y         = iff.read_float();
					z         = iff.read_float();
					flipState = iff.read_float();

					// Create entry.
					m_dot3Vectors->push_back(Dot3Vector(x, y, z, flipState));
				}

				iff.exitChunk(TAG_DOT3);
			}
		}

		//-- load hardpoints
		if (iff.enterForm(TAG_HPTS, true))
		{
			//-- handle static (non-morphable) hardpoints
			if (iff.enterChunk(TAG_STAT, true))
			{
				IS_NULL(m_staticHardpoints);

				m_staticHardpoints = new HardpointVector();
				loadHardpoints(iff, *m_staticHardpoints);

				iff.exitChunk(TAG_STAT);
			}

			//-- handle dynamic (morphable) hardpoints
			if (iff.enterChunk(TAG_DYN, true))
			{
				IS_NULL(m_dynamicHardpoints);

				m_dynamicHardpoints = new HardpointVector();
				loadHardpoints(iff, *m_dynamicHardpoints);

				iff.exitChunk(TAG_DYN);
			}

			iff.exitForm(TAG_HPTS);
		}

		//-- load blend targets
		if (blendTargetCount)
		{
			iff.enterForm(TAG_BLTS);
			{
				if (!m_blendTargets)
					m_blendTargets = new BlendTargetVector();
				else
				{
					std::for_each(m_blendTargets->begin(), m_blendTargets->end(), PointerDeleter());
					m_blendTargets->clear();
				}

				m_blendTargets->reserve(static_cast<size_t>(blendTargetCount));

				for (int i = 0; i < blendTargetCount; ++i)
				{
					m_blendTargets->push_back(new BlendTarget());
					m_blendTargets->back()->load_0004(iff);
				}
			}
			iff.exitForm(TAG_BLTS);
		}

		//-- load occlusion zone names referenced by the mesh
		std::vector<int> occlusionZoneIds;

		if (occlusionZoneCount)
		{
			//-- load occlusion zone names
			std::vector<boost::shared_ptr<CrcLowerString> >  occlusionZoneNames;

			occlusionZoneNames.reserve(static_cast<size_t>(occlusionZoneCount));

			iff.enterChunk(TAG_OZN);
			{
				for (int i = 0; i < occlusionZoneCount; ++i)
				{
					char buffer[1024];
					iff.read_string(buffer, sizeof(buffer)-1);

					occlusionZoneNames.push_back(boost::shared_ptr<CrcLowerString>(new CrcLowerString(buffer)));
				}
			}
			iff.exitChunk(TAG_OZN);

			//-- get ids for occlusion zone names
			OcclusionZoneSet::registerOcclusionZones(occlusionZoneNames, occlusionZoneIds);
		}

		//-- load fully occluded zone combination
		if (iff.enterChunk(TAG_FOZC, true))
		{
			if (!m_fullyOccludedZoneCombination)
				m_fullyOccludedZoneCombination = new IntVector();
			else
				m_fullyOccludedZoneCombination->clear();

			const size_t count = static_cast<size_t>(iff.read_uint16());
			m_fullyOccludedZoneCombination->reserve(count);

			for (size_t i = 0; i < count; ++i)
			{
				//-- get occlusion zone index relative to mesh's OZ name list
				const int localOzIndex  = static_cast<int>(iff.read_int16());
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, localOzIndex, static_cast<int>(occlusionZoneIds.size()));

				//-- convert local OZ index into system-wide OZ index
				const int systemOzIndex = occlusionZoneIds[static_cast<size_t>(localOzIndex)];
				m_fullyOccludedZoneCombination->push_back(systemOzIndex);
			}

			iff.exitChunk(TAG_FOZC);
		}

		//-- load occlusion zone combinations
		if (occlusionZoneCombinationCount)
		{
			m_occlusionZoneCombinations.resize(static_cast<size_t>(occlusionZoneCombinationCount));

			iff.enterChunk(TAG_OZC);
			{
				for (int i = 0; i < occlusionZoneCombinationCount; ++i)
				{
					//-- select the OZ combination array, size it properly
					const int         combinationZoneCount = static_cast<int>(iff.read_int16());
					std::vector<int> &combination          = m_occlusionZoneCombinations[static_cast<size_t>(i)];

					combination.reserve(static_cast<size_t>(combinationZoneCount));
	
					for (int ozIndex = 0; ozIndex < combinationZoneCount; ++ozIndex)
					{
						//-- get occlusion zone index relative to mesh's OZ name list
						const int localOzIndex  = static_cast<int>(iff.read_int16());
						VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, localOzIndex, static_cast<int>(occlusionZoneIds.size()));

						//-- convert local OZ index into system-wide OZ index
						const int systemOzIndex = occlusionZoneIds[static_cast<size_t>(localOzIndex)];
						combination.push_back(systemOzIndex);
					}
				}
			}
			iff.exitChunk(TAG_OZC);
		}

		//-- load occlusion zones that this mesh occludes
		if (zonesThisOccludesCount)
		{
			iff.enterChunk(TAG_ZTO);

				m_zonesThisOccludes.reserve(static_cast<size_t>(zonesThisOccludesCount));

				for (int i = 0; i < zonesThisOccludesCount; ++i)
				{
					//-- get occlusion zone index relative to mesh's OZ name list
					const int localOzIndex  = static_cast<int>(iff.read_int16());
					VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, localOzIndex, static_cast<int>(occlusionZoneIds.size()));

					//-- convert local OZ index into system-wide OZ index
					const int systemOzIndex = occlusionZoneIds[static_cast<size_t>(localOzIndex)];
					m_zonesThisOccludes.push_back(systemOzIndex);
				}

			iff.exitChunk(TAG_ZTO);
		}

		//-- load per shader data
		{
			int zeroAreaTriCount = 0;

			m_perShaderData.reserve(static_cast<size_t>(perShaderDataCount));
			for (int i = 0; i < perShaderDataCount; ++i)
			{
				PerShaderData *const psd = new PerShaderData();
				psd->load_0004(iff, m_positions, zeroAreaTriCount);

				m_perShaderData.push_back(psd);
			}

			DEBUG_WARNING(zeroAreaTriCount > 0, ("Asset [%s] has [%d] zero-area bind-pose triangles.", getName().getString(), zeroAreaTriCount));
		}

		//-- load info about texture renderer templates
		if (iff.enterForm(TAG_TRTS, true))
		{
			iff.enterChunk(TAG_INFO);
				const int headerCount  = iff.read_int32();
				const int entryCount   = iff.read_int32();
			iff.exitChunk(TAG_INFO);

			if (!m_textureRendererHeaders)
				m_textureRendererHeaders = new TRHeaderContainer;
			else
			{
				std::for_each(m_textureRendererHeaders->begin(), m_textureRendererHeaders->end(), PointerDeleter());
				m_textureRendererHeaders->clear();
			}

			m_textureRendererHeaders->reserve(static_cast<size_t>(headerCount));

			if (!m_textureRendererEntries)
				m_textureRendererEntries = new TREntryContainer;
			else
				m_textureRendererEntries->clear();

			m_textureRendererEntries->reserve(static_cast<size_t>(entryCount));

			int currentEntryIndex = 0;

			for (int i = 0; i < headerCount; ++i)
			{
				iff.enterChunk(TAG_TRT);
				
					//-- get the texture renderer name and # affected shaders
					char textureRendererTemplateName[MAX_PATH];
					
					iff.read_string(textureRendererTemplateName, sizeof(textureRendererTemplateName)-1);
					const int affectedShaderCount = iff.read_int32();

					//-- save this header info
					m_textureRendererHeaders->push_back(new TextureRendererHeader(textureRendererTemplateName, affectedShaderCount, currentEntryIndex));

					//-- load info on the affected shaders
					for (int affectedShaderIndex = 0; affectedShaderIndex < affectedShaderCount; ++affectedShaderIndex, ++currentEntryIndex)
					{
						const int shaderIndex      = iff.read_int32();
						const Tag shaderTextureTag = static_cast<Tag>(iff.read_uint32());
						m_textureRendererEntries->push_back(TextureRendererEntry(shaderIndex, shaderTextureTag));
					}

				iff.exitChunk(TAG_TRT);
			}

			iff.exitForm(TAG_TRTS);
		}

	iff.exitForm(TAG_0004);
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::asynchronousLoadCallback()
{
	//-- Load now.  If we really were async loaded, the async loaded has preloaded
	//   all files it thinks we need for this asset.
	{
		//-- Create the Iff instance for this load.
		FATAL(s_asynchronousLoadIff != NULL, ("SkeletalMeshGeneratorTemplate::asynchronousLoadCallback()'s iff pointer is non-NULL: this is an invalid state."));

		s_asynchronousLoadIff = new Iff();
		FATAL(s_asynchronousLoadIff == NULL, ("new Iff() returned NULL."));
		
		//-- Save name of most recently exported skeletal mesh generator template.
		IGNORE_RETURN(snprintf(ms_crashReportInfo, sizeof(ms_crashReportInfo) - 1, "SkeletalMeshGeneratorTemplateAsync: %s\n", getName().getString()));
		ms_crashReportInfo[sizeof(ms_crashReportInfo) - 1] = '\0';

		//-- Open the Iff file.
		bool const openSuccess = s_asynchronousLoadIff->open(getName().getString(), true);

		//-- Load the Iff.
		if (openSuccess)
			load(*s_asynchronousLoadIff);

		//-- Cleanup the Iff.
		delete s_asynchronousLoadIff;
		s_asynchronousLoadIff = NULL;

		//-- Exit if we failed to load the Iff.
		if (!openSuccess)
		{
			DEBUG_WARNING(true, ("Asynchronous loader called SkeletalMeshGeneratorTemplate callback but the async loaded file [%s] failed to load via iff.  How does that happen?  Leaving in unloaded state.", getName().getString()));
			return;
		} 
	}

	//-- Remember there's no async load in progress.
	m_asynchronousLoadInProgress = false;

	//-- Fixup uninitialized mesh generators.
	if (m_uninitializedMeshGenerators)
	{
		std::for_each(m_uninitializedMeshGenerators->begin(), m_uninitializedMeshGenerators->end(), VoidMemberFunction(&SkeletalMeshGenerator::create));
		delete m_uninitializedMeshGenerators;
		m_uninitializedMeshGenerators = 0;
	}
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::loadHardpoints(Iff &iff, HardpointVector &hardpoints)
{
	char  hardpointName[256];
	char  parentName[256];

	// get # hardpoints
	const int hardpointCount = static_cast<int>(iff.read_int16());
	DEBUG_FATAL(hardpointCount < 1, ("bad hardpoint count %d", hardpointCount));

	// create hardpoint array
	hardpoints.reserve(hardpoints.size() + static_cast<size_t>(hardpointCount));

	// create hardpoints
	for (int i = 0; i < hardpointCount; ++i)
	{
		// load hardpoint data
		iff.read_string(hardpointName, sizeof(hardpointName)-1);
		iff.read_string(parentName, sizeof(parentName)-1);

		const Quaternion rotation = iff.read_floatQuaternion();
		const Vector     position = iff.read_floatVector();

		hardpoints.push_back(new Hardpoint(hardpointName, parentName, position, rotation));
	}
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::applySkeletonModifications(const IntVector *blendValues, Skeleton &skeleton) const
{
	//-- add static hardpoints to skeleton
	Transform  hardpointToParent(Transform::IF_none);

	if (m_staticHardpoints)
	{
		const HardpointVector::const_iterator itEnd = m_staticHardpoints->end();
		for (HardpointVector::const_iterator it = m_staticHardpoints->begin(); it != itEnd; ++it)
		{
			// create the transform
			const Hardpoint &hardpoint = *(*it);
			NOT_NULL(*it);

			hardpoint.getRotation().getTransformPreserveTranslation(&hardpointToParent);
			hardpointToParent.setPosition_p(hardpoint.getPosition());

			// set it
			skeleton.addHardpoint(hardpoint.getHardpointName(), hardpoint.getParentName(), hardpointToParent);
		}
	}

	//-- handle dynamic hardpoints
	if (m_dynamicHardpoints)
	{
		//-- populate non-modified hardpoint data
		{
			const HardpointVector::const_iterator itEnd          = m_dynamicHardpoints->end();
			size_t                                hardpointIndex = 0;

			for (HardpointVector::const_iterator it = m_dynamicHardpoints->begin(); it != itEnd; ++it, ++hardpointIndex)
			{
				const Hardpoint &hardpoint = *(*it);
				NOT_NULL(*it);

				ms_dynamicHardpointPositions[hardpointIndex] = hardpoint.getPosition();
				ms_dynamicHardpointRotations[hardpointIndex] = hardpoint.getRotation();
			}
		}

		//-- retrieve morph target fractions from VariableSet

		// -TRF- this should simplify if we move to non-static (non-template)
		//       MeshGenerator objects.  At that point, we should no longer
		//       require the VariableSet to be passed in.

		// we should have blend targets if we have dynamic hardpoints
		if (!m_blendTargets)
			DEBUG_WARNING(true, ("MGN [%s]: has dynamic hardpoints without blend targets --- this shouldn't happen.", getName().getString()));
		else
		{
			// apply blend shape hardpoint morphs
			int blendTargetIndex = 0;

			const BlendTargetVector::const_iterator itEnd = m_blendTargets->end();
			for (BlendTargetVector::const_iterator it = m_blendTargets->begin(); it != itEnd; ++it, ++blendTargetIndex)
			{
				NOT_NULL(blendValues);
				//-- calculate blend weight as <blend value int> / 255.0f;
				const real weight = static_cast<float>((*blendValues)[static_cast<size_t>(blendTargetIndex)]) / 255.0f;

				if ((weight > ms_applyWeightThreshold) || (weight < -ms_applyWeightThreshold))
					(*it)->applyHardpointDeformation(weight, ms_dynamicHardpointPositions, ms_dynamicHardpointRotations);
			}
		}

		// add dynamic hardpoints to skeleton
		{
			const HardpointVector::const_iterator itEnd          = m_dynamicHardpoints->end();
			size_t                                hardpointIndex = 0;

			for (HardpointVector::const_iterator it = m_dynamicHardpoints->begin(); it != itEnd; ++it, ++hardpointIndex)
			{
				const Hardpoint &hardpoint = *(*it);
				NOT_NULL(*it);

				// build transform
				ms_dynamicHardpointRotations[hardpointIndex].getTransformPreserveTranslation(&hardpointToParent);
				hardpointToParent.setPosition_p(ms_dynamicHardpointPositions[hardpointIndex]);

				// add hardpoint
				skeleton.addHardpoint(hardpoint.getHardpointName(), hardpoint.getParentName(), hardpointToParent);
			}
		}
	}
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::addShaderPrimitives(
	Appearance             &appearance,
	int                     lodIndex,
	CustomizationData      *customizationData,
	const IntVector        *blendValues, 
	const TransformNameMap &transformNameMap, 
	const OcclusionZoneSet &zonesCurrentlyOccluded, 
	OcclusionZoneSet       &zonesOccludedByThisLayer,
	ShaderPrimitiveVector  &shaderPrimitives
	) const
{
	//-- -TRF- handle adding static mesh data here

	//-- fill mesh construction helper with dynamic data
	fillMeshConstructionHelper(*ms_meshConstructionHelper, blendValues, transformNameMap, zonesCurrentlyOccluded, zonesOccludedByThisLayer);

	//-- generate the shader primitives
	ms_meshConstructionHelper->prepareForReading();

	const size_t shaderPrimitiveBaseIndex = shaderPrimitives.size();

	{
		//-- make sure our appearance is a SkeletalAppearance
		SkeletalAppearance2 *skeletalAppearance = appearance.asSkeletalAppearance2();
		NOT_NULL(skeletalAppearance);

		const int shaderCount = ms_meshConstructionHelper->getShaderCount();
		for (int i = 0; i < shaderCount; ++i)
		{
			//-- create the shader primitive
			ShaderPrimitive *const newShaderPrimitive = new SoftwareBlendSkeletalShaderPrimitive(*skeletalAppearance, lodIndex, *ms_meshConstructionHelper, i);

			//-- set its customization data
			newShaderPrimitive->setCustomizationData(customizationData);

			//-- add to list
			shaderPrimitives.push_back(newShaderPrimitive);
		}
	}

	//-- properly handle texture renderers
	{
		const int textureRendererCount = ms_meshConstructionHelper->getTextureRendererCount();
		for (int i = 0; i < textureRendererCount; ++i)
		{
			//-- get the TextureRendererTemplate
			const MeshConstructionHelper::PerTextureRendererData *ptrd = ms_meshConstructionHelper->getPerTextureRendererData(i);
			const TextureRendererTemplate *textureRendererTemplate = TextureRendererList::fetch(ms_meshConstructionHelper->getTextureRendererTemplateName(ptrd));

			if (!textureRendererTemplate)
			{
				// bad data, skip it
				continue;
			}

			//-- create a new texture renderer and assign its texture for each shader to which the TR template is applied
			const int affectedShaderCount = ms_meshConstructionHelper->getAffectedShaderCount(ptrd);

			for (int affectedShaderIndex = 0; affectedShaderIndex < affectedShaderCount; ++affectedShaderIndex)
			{
				//-- get info on the affected shader
				int  shaderIndex      = -1;
				Tag  shaderTextureTag = TAG(N,O,N,E);

				ms_meshConstructionHelper->getAffectedShaderData(ptrd, affectedShaderIndex, &shaderIndex, &shaderTextureTag);
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, shaderIndex, ms_meshConstructionHelper->getShaderCount());

				//-- piggy-back the TextureRendererShaderPrimitive if it doesn't already exist
				const size_t  shaderPrimitiveIndex = shaderPrimitiveBaseIndex + static_cast<size_t>(shaderIndex);
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(shaderPrimitiveIndex), static_cast<int>(shaderPrimitives.size()));

				Shader *shader = 0;

				ShaderPrimitive *const initialShaderPrimitive = shaderPrimitives[shaderPrimitiveIndex];
				NOT_NULL(initialShaderPrimitive);

				TextureRendererShaderPrimitive *trShaderPrimitive = initialShaderPrimitive->asTextureRendererShaderPrimitive();
				if (trShaderPrimitive)
				{
					//-- retrieve the shader from the real ShaderPrimitive
					SoftwareBlendSkeletalShaderPrimitive *const sbsShaderPrimitive = trShaderPrimitive->asSoftwareBlendSkeletalShaderPrimitive();
					NOT_NULL(sbsShaderPrimitive);

					shader = &(sbsShaderPrimitive->getShader());
				}
				else
				{
					//-- get the shader
					SoftwareBlendSkeletalShaderPrimitive *const sbsShaderPrimitive = initialShaderPrimitive->asSoftwareBlendSkeletalShaderPrimitive();
					NOT_NULL(sbsShaderPrimitive);
					shader = &(sbsShaderPrimitive->getShader());

					//-- create the TextureRendererShaderPrimitive with no entries
					trShaderPrimitive = new TextureRendererShaderPrimitive(initialShaderPrimitive);

					//-- replace existing with the TR shader prim
					shaderPrimitives[shaderPrimitiveIndex] = trShaderPrimitive;
				}

				//-- add a new texture renderer to the tr shader primitive
				NOT_NULL(trShaderPrimitive);

				// create the texture renderer
				TextureRenderer *textureRenderer = textureRendererTemplate->createTextureRenderer();
				NOT_NULL(textureRenderer);

				// assign customization data
				textureRenderer->setCustomizationData(customizationData);
				
				// fetch texture from TextureRenderer
				const Texture *const texture = textureRenderer->fetchTexture();
				NOT_NULL(texture);

				// add texture renderer to list of texture renderers associated with the shader
				trShaderPrimitive->addTextureRenderer(*textureRenderer, shaderTextureTag);

				// release local texture renderer
				textureRenderer->release();
				textureRenderer = 0;

				//-- assign the texture to the shader
				NOT_NULL(shader);

				// @todo come up with better way (no dynamic casting) to get static shader.
				StaticShader *const staticShader = shader->getStaticShader();

				NOT_NULL(staticShader);
				const Texture *dummy = 0;
				if (staticShader->getTexture(shaderTextureTag, dummy))
					staticShader->setTexture(shaderTextureTag, *texture);
				else
				{
					char buffer[5];
					ConvertTagToString(shaderTextureTag, buffer);
					DEBUG_WARNING(true, ("could not set texture for tag [%s] appearance template [%s]", buffer, appearance.getAppearanceTemplateName () ? appearance.getAppearanceTemplateName () : ""));
				}

				//-- release local reference to texture
				texture->release();
			}
			//-- release the TextureRendererTemplate
			textureRendererTemplate->release();
		}
	}

	//-- Clean up MeshConstructionHelper for next caller usage.
	ms_meshConstructionHelper->clearAllData();
}

// ----------------------------------------------------------------------

int SkeletalMeshGeneratorTemplate::getReferencedSkeletonTemplateCount() const
{
	return static_cast<int>(m_skeletonTemplateNames.size());
}

// ----------------------------------------------------------------------
/**
 * Retrieve the number of TextureRendererTemplate instances referenced by
 * this SkeletalMeshGeneratorTemplate instance.
 *
 * @return  the number of TextureRendererTemplate instances referenced by
 *          this SkeletalMeshGeneratorTemplate instance.
 */

int SkeletalMeshGeneratorTemplate::getTextureRendererTemplateCount() const
{
	// @todo -TRF- all of this should be handled via a SkeletalMeshGeneratorTemplate::addCustomizationVariables() call.
	//       add that function and get rid of these.
	if (m_textureRendererHeaders)
		return static_cast<int>(m_textureRendererHeaders->size());
	else
		return 0;
}

// ----------------------------------------------------------------------

const TextureRendererTemplate *SkeletalMeshGeneratorTemplate::fetchTextureRendererTemplate(int index) const
{
	// @todo -TRF- all of this should be handled via a SkeletalMeshGeneratorTemplate::addCustomizationVariables() call.
	//       add that function and get rid of these.
	NOT_NULL(m_textureRendererHeaders);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getTextureRendererTemplateCount());

	// @todo -TRF- load these at MeshTemplate load time, don't just load the names
	return TextureRendererList::fetch((*m_textureRendererHeaders)[static_cast<size_t>(index)]->m_textureRendererTemplateName);
}

// ----------------------------------------------------------------------
/**
 * Add all CustomizationData variables that influence this MeshGeneratorTemplate.
 *
 * @param customizationData  the CustomizationData instance to which influential
 *                           variables will be added.
 */

void SkeletalMeshGeneratorTemplate::addCustomizationVariables(CustomizationData &customizationData) const
{
	//-- create a RangedIntCustomizationVariable for each blend variable
	{
		const int variableCount = getBlendVariableCount();
		for (int i = 0; i < variableCount; ++i)
		{
			// get variable name
			const std::string &variableName = getBlendVariableName(i);

			// check if it already exists in the customizationData
			const CustomizationVariable *const existingCustomizationVariable = customizationData.findConstVariable(variableName);
			if (existingCustomizationVariable)
				continue;

			// create a RangedIntCustomizationVariable for the variable
			customizationData.addVariableTakeOwnership(variableName, new BasicRangedIntCustomizationVariable(0, 0, 256));
		}
	}

	//-- add all TextureRendererTemplate variables
	// @todo load TR Templates at MG load time, not now!
	{
		const int trTemplateCount = getTextureRendererTemplateCount();
		for (int i = 0; i < trTemplateCount; ++i)
		{
			// get TextureRendererTemplate
			const TextureRendererTemplate *const trTemplate = fetchTextureRendererTemplate(i);
			if (!trTemplate)
			{
				// this happens when we fail to load a texture renderer template (i.e. bad data)
				continue;
			}

			const int trVariableCount = trTemplate->getCustomizationVariableCount();
			for (int j = 0; j < trVariableCount; ++j)
			{
				//-- Get short variable name.
				const std::string &shortVariableName = trTemplate->getCustomizationVariableName(j);

				//-- Construct full variable path.
				const std::string fullVariableName = (trTemplate->isCustomizationVariablePrivate(j) ? "/private/" : "/shared_owner/") + shortVariableName;

				// check if it already exists in the customizationData
				const CustomizationVariable *const existingCustomizationVariable = customizationData.findConstVariable(fullVariableName);
				if (existingCustomizationVariable)
					continue;

				// create a RangedIntCustomizationVariable for the variable
				customizationData.addVariableTakeOwnership(fullVariableName, trTemplate->createCustomizationVariable(j));
			}

			// release local reference
			trTemplate->release();
		}
	}
}

// ----------------------------------------------------------------------

bool SkeletalMeshGeneratorTemplate::isLoaded() const
{
	return m_isLoaded;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::fillIndexedTriangleList(IndexedTriangleList &list) const
{
	list.clear();

	//-- Collect draw primitive indices for indexed triangle lists.
	IntVector  triangleIndices;

	PerShaderDataVector::const_iterator const endIt = m_perShaderData.end();
	for (PerShaderDataVector::const_iterator it = m_perShaderData.begin(); it != endIt; ++it)
	{
		PerShaderData const *const perShaderData = *it;
		
		if (perShaderData)
			perShaderData->collectTriangleIndices(triangleIndices);
	}

	list.addIndexedTriangleList(&m_positions[0], static_cast<int>(m_positions.size()), &triangleIndices[0], static_cast<int>(triangleIndices.size()));
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::fillMeshConstructionHelper(
	MeshConstructionHelper &meshConstructionHelper,
	const IntVector        *blendValues,
	const TransformNameMap &transformNameMap,
	const OcclusionZoneSet &zonesCurrentlyOccluded,
	OcclusionZoneSet       &zonesOccludedByThisLayer
	) const
{
#if TRACK_FILL_CALLS
	//-- Test code to find how many times per frame this is getting called.
	{
		static TemplateIntMap  s_lastFrameNumberMap;
		static TemplateIntMap  s_frameCallCountMap;

		const int  currentFrameNumber  = Graphics::getFrameNumber();
		int       &thisLastFrameNumber = s_lastFrameNumberMap[this];
		int       &thisCallCount       = s_frameCallCountMap[this];

		if (thisLastFrameNumber != currentFrameNumber)
		{
			// Clear the frame count.
			thisCallCount       = 0;
			thisLastFrameNumber = currentFrameNumber;
		}
		
		// Bump up count for this frame.
		++thisCallCount;

		DEBUG_REPORT_LOG(true, ("fillMeshConstructionHelper: template [%s] frame [%d] call #[%d].\n", getName().getString(), currentFrameNumber, thisCallCount));
	}
#endif

	//-- test if we can trivially reject this entire mesh due to occluded faces
	if (m_fullyOccludedZoneCombination && !m_fullyOccludedZoneCombination->empty())
	{
		if (zonesCurrentlyOccluded.allZonesPresent(*m_fullyOccludedZoneCombination))
		{
			//-- fully occluded.
			// -TRF- might want to add all of the zones this mesh occludes to zonesThisOccludes; however, that should be redundant.
			return;
		}
	}

	//-- setup which occlusion zone combinations are occluded
	{
		size_t combinationIndex = 0;
		ms_combinationsOccluded->resize(m_occlusionZoneCombinations.size());

		const IntVectorVector::const_iterator vectorItEnd = m_occlusionZoneCombinations.end();
		for (IntVectorVector::const_iterator vectorIt = m_occlusionZoneCombinations.begin(); vectorIt != vectorItEnd; ++vectorIt, ++combinationIndex)
		{
			//-- test if this set of occlusion zones currently is occluded
			(*ms_combinationsOccluded)[combinationIndex] = zonesCurrentlyOccluded.allZonesPresent(*vectorIt); //lint !e1058 // (Error -- Initializing a non-const reference '_STL::_Bit_reference &' with a non-lvalue) // I think lint is confused
		}
	}

	//-- setup ms_localToOutputTransformIndexMap
	{
		// lookup transform names
		int i = 0;

		const CrcLowerStringVector::const_iterator itEnd = m_transformNames.end();
		for (CrcLowerStringVector::const_iterator it = m_transformNames.begin(); it != itEnd; ++it, ++i)
		{
			bool found          = false;
			int  transformIndex = -1;

			transformNameMap.findTransformIndex(*it, &transformIndex, &found);
			if (!found || (transformIndex < 0))
			{
				//-- yikes, didn't find a transform referenced by this mesh.  there's no way we can draw this properly,
				//   so drop the mesh.
				DEBUG_WARNING(true, ("skeletal mesh [%s] references non-existent joint [%s] -- is a required skeleton segment missing?", getName().getString(), (*it).getString()));
				return;
			}

			ms_localToOutputTransformIndices[static_cast<size_t>(i)] = transformIndex;
		}
	}

	//-- add position vectors
	const size_t  positionCount = m_positions.size();

	int     firstPositionIndex = -1;
	{
		Vector       *destPositions = 0;

		meshConstructionHelper.allocatePositionBuffer(static_cast<int>(positionCount), &firstPositionIndex, &destPositions);
		memcpy(destPositions, &m_positions[0], sizeof(Vector) * positionCount);

		//-- apply blend target position modifications
		if (m_blendTargets)
		{
			int blendTargetIndex = 0;

			const BlendTargetVector::const_iterator itEnd = m_blendTargets->end();
			for (BlendTargetVector::const_iterator it = m_blendTargets->begin(); it != itEnd; ++it, ++blendTargetIndex)
			{
				//-- calculate blend weight as <blend value int> / 255.0f;
				NOT_NULL(blendValues);
				const real weight = static_cast<float>((*blendValues)[static_cast<size_t>(blendTargetIndex)]) / 255.0f;

				if ((weight > ms_applyWeightThreshold) || (weight < -ms_applyWeightThreshold))
					(*it)->applyPositionDeformation(weight, destPositions, static_cast<int>(positionCount));
			}
		}
	}

	//-- set position weights
	{
		int currentTransformWeightIndex = 0;
		for (int i = 0; i < static_cast<int>(positionCount); ++i)
		{
			const int transformWeightCount = m_transformWeightCounts[static_cast<size_t>(i)];
			for (int j = 0; j < transformWeightCount; ++j)
			{
				const TransformWeightData &twd = m_transformWeightData[static_cast<size_t>(currentTransformWeightIndex + j)];
				meshConstructionHelper.addPositionWeight(firstPositionIndex + i, ms_localToOutputTransformIndices[static_cast<size_t>(twd.m_transformIndex)], twd.m_transformWeight);
			}

			currentTransformWeightIndex += transformWeightCount;
		}
	}

	//-- add normal vectors
	int firstNormalIndex = -1;
	if (!m_normals.empty())
	{
		Vector       *destNormals = 0;
		const size_t  normalCount = m_normals.size();

		meshConstructionHelper.allocateNormalBuffer(static_cast<int>(m_normals.size()), &firstNormalIndex, &destNormals);
		NOT_NULL(destNormals);
		memcpy(destNormals, &m_normals[0], sizeof(Vector) * normalCount);


		int appliedNormalCount = 0;

		//-- apply blend target normal modifications
		if (m_blendTargets)
		{
			int blendTargetIndex = 0;

			const BlendTargetVector::const_iterator itEnd = m_blendTargets->end();
			for (BlendTargetVector::const_iterator it = m_blendTargets->begin(); it != itEnd; ++it, ++blendTargetIndex)
			{
				//-- calculate blend weight as <blend value int> / 255.0f;
				NOT_NULL(blendValues);
				const real weight = static_cast<float>((*blendValues)[static_cast<size_t>(blendTargetIndex)]) / 255.0f;

				if ((weight > ms_applyWeightThreshold) || (weight < -ms_applyWeightThreshold))
				{
					(*it)->applyNormalDeformation(weight, destNormals, static_cast<int>(normalCount));
					++appliedNormalCount;
				}
			}

			// -TRF- do we really want to do this?
			//-- normalize normals if we applied any deformation
			if (appliedNormalCount)
			{
				for (size_t normalIndex = 0; normalIndex < normalCount; ++normalIndex)
				{
					IGNORE_RETURN(destNormals[normalIndex].normalize());
				}
			}
		}
	}

	//-- Prepare the dot3 vectors used by this mesh.
	const Dot3VectorVector *const preparedDot3Vectors = (m_dot3Vectors ? prepareDot3Vectors(blendValues) : 0);

	//-- add per shader data
	{
		int i = 0;

		const PerShaderDataVector::const_iterator itEnd = m_perShaderData.end();
		for (PerShaderDataVector::const_iterator it = m_perShaderData.begin(); it != itEnd; ++it, ++i)
			(*it)->addPerShaderData(meshConstructionHelper, firstPositionIndex, firstNormalIndex, preparedDot3Vectors, *ms_combinationsOccluded, (*ms_perShaderDataScratchpad)[static_cast<size_t>(i)]);
	}

	//-- add texture renderer info
	if (m_textureRendererHeaders)
	{
		NOT_NULL(m_textureRendererEntries);

		TRHeaderContainer::const_iterator itEnd = m_textureRendererHeaders->end();
		for (TRHeaderContainer::const_iterator it = m_textureRendererHeaders->begin(); it != itEnd; ++it)
		{
			const TextureRendererHeader &trh = *NON_NULL(*it);
			MeshConstructionHelper::PerTextureRendererData *ptrd = meshConstructionHelper.addTextureRendererTemplate(trh.m_textureRendererTemplateName);

			const int lastEntryIndex = trh.m_firstEntryIndex + trh.m_entryCount - 1;
			for (int entryIndex = trh.m_firstEntryIndex; entryIndex <= lastEntryIndex; ++entryIndex)
			{
				VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, entryIndex, static_cast<int>(m_textureRendererEntries->size()));
				const TextureRendererEntry &entry = (*m_textureRendererEntries)[static_cast<size_t>(entryIndex)];

				MeshConstructionHelper::PerShaderData *const outputPsd = (*ms_perShaderDataScratchpad)[static_cast<size_t>(entry.m_shaderIndex)];
				meshConstructionHelper.addAffectedShaderTemplate(ptrd, entry.m_shaderTextureTag, outputPsd);
			}
		}
	}

	//-- specify zones this mesh occludes
	{
		const IntVector::const_iterator itEnd = m_zonesThisOccludes.end();
		for (IntVector::const_iterator it = m_zonesThisOccludes.begin(); it != itEnd; ++it)
			zonesOccludedByThisLayer.addZone(*it);
	}
}

// ----------------------------------------------------------------------

const SkeletalMeshGeneratorTemplate::Dot3VectorVector *SkeletalMeshGeneratorTemplate::prepareDot3Vectors(const IntVector *blendValues) const
{
	if (!m_dot3Vectors)
		return 0;

	if (!m_blendTargets)
	{
		// Use the pristine dot3 vectors, no blending necessary.
		return m_dot3Vectors;
	}

	//-- Apply blend target morphing to the dot3 vectors.

	// Initialize dot3 vectors with pristine values.
	ms_blendedDot3Vectors->clear();
	*ms_blendedDot3Vectors = *m_dot3Vectors;

	int blendTargetIndex = 0;
	int appliedDot3Count = 0;

	{
		const BlendTargetVector::const_iterator itEnd = m_blendTargets->end();
		for (BlendTargetVector::const_iterator it = m_blendTargets->begin(); it != itEnd; ++it, ++blendTargetIndex)
		{
			//-- calculate blend weight as <blend value int> / 255.0f;
			NOT_NULL(blendValues);
			const real weight = static_cast<float>((*blendValues)[static_cast<size_t>(blendTargetIndex)]) * (1.0f / 255.0f);

			if ((weight > ms_applyWeightThreshold) || (weight < -ms_applyWeightThreshold))
			{
				(*it)->applyDot3VectorDeformation(weight, *ms_blendedDot3Vectors);
				++appliedDot3Count;
			}
		}
	}

	// -TRF- do we really want to do this?
	//-- normalize normals if we applied any deformation
	if (appliedDot3Count)
	{
		const Dot3VectorVector::iterator endIt = ms_blendedDot3Vectors->end();
		for (Dot3VectorVector::iterator it = ms_blendedDot3Vectors->begin(); it != endIt; ++it)
			IGNORE_RETURN(it->m_dot3Vector.normalize());
	}

	return ms_blendedDot3Vectors;
}

// ----------------------------------------------------------------------

void SkeletalMeshGeneratorTemplate::removeAsynchronouslyLoadedMeshGenerator(SkeletalMeshGenerator *meshGenerator) const
{
	NOT_NULL(meshGenerator);
	NOT_NULL(m_uninitializedMeshGenerators);

	MeshGeneratorVector::iterator findIt = std::find(m_uninitializedMeshGenerators->begin(), m_uninitializedMeshGenerators->end(), meshGenerator);
	if (findIt != m_uninitializedMeshGenerators->end())
	{
		IGNORE_RETURN(m_uninitializedMeshGenerators->erase(findIt));
		if (m_uninitializedMeshGenerators->empty())
		{
			delete m_uninitializedMeshGenerators;
			m_uninitializedMeshGenerators = 0;
		}
	}
	else
	{
		DEBUG_FATAL(true, ("removeAsynchronouslyLoadedMeshGenerator(): called on mesh generator not in list for [%s].", getName().getString()));
	}
}

// ----------------------------------------------------------------------

const CrcLowerString &SkeletalMeshGeneratorTemplate::getReferencedSkeletonTemplateName(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_skeletonTemplateNames.size()));
	return m_skeletonTemplateNames[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------
/**
 * NOTE: consider removing this routine for production builds.  It's
 *       here as a development helper.
 */

Appearance *SkeletalMeshGeneratorTemplate::createAppearance() const
{
	try
	{
		//-- get referenced skeleton template name from mesh generator
		if (!getReferencedSkeletonTemplateCount())
		{
			DEBUG_WARNING(true, ("no referenced skeleton templates\n"));
			return 0;
		}

		//-- build a new SkeletalAppearanceTemplate
		SkeletalAppearanceTemplate *appearanceTemplate = new SkeletalAppearanceTemplate();
		IGNORE_RETURN(AppearanceTemplateList::fetchNew(appearanceTemplate));

		IGNORE_RETURN(appearanceTemplate->addMeshGenerator(getName().getString()));
		IGNORE_RETURN(appearanceTemplate->addSkeletonTemplate(getReferencedSkeletonTemplateName(0).getString(), ""));

		//-- create a SkeletalAppearance2 from AppearanceTemplate
		Appearance *const appearance = appearanceTemplate->createAppearance();

		AppearanceTemplateList::release(appearanceTemplate);
		appearanceTemplate = 0; //lint !e423 // creation of memory leak // no, ref counted
		
		if (!appearance)
		{
			DEBUG_WARNING(true, ("appearance template failed to create appearance\n"));
			return 0;
		}
		SkeletalAppearance2 *const skeletalAppearance = dynamic_cast<SkeletalAppearance2*>(appearance);

		//-- build the mesh
		if (skeletalAppearance)
			IGNORE_RETURN(skeletalAppearance->rebuildIfDirtyAndAvailable());
		
		return appearance;
	}
	catch (const FatalException& /* e */)
	{
	}

	DEBUG_FATAL(true, ("try/catch/throw failed in SkeletalMeshGeneratorTemplate::createAppearance()\n"));
	return 0; //lint !e527 // Unreachable // Reachable in release.
}

// ----------------------------------------------------------------------

int SkeletalMeshGeneratorTemplate::getOcclusionLayer() const
{
	return m_occlusionLayer;
}

// ======================================================================
