// ======================================================================
//
// OccludedFaceMapGenerator.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_OccludedFaceMapGenerator_H
#define INCLUDED_OccludedFaceMapGenerator_H

// ======================================================================

#include <memory>

class CrcLowerString;
class Messenger;
class UniqueNameList;

class MDagPath;
class MObject;
class MStatus;

// ======================================================================

class OccludedFaceMapGenerator
{
public:

	typedef stdvector<MObject>::fwd  MObjectVector;
	typedef stdvector<int>::fwd      IntVector;


public:

	static void install(Messenger *newMessenger);

public:

	OccludedFaceMapGenerator(const MObjectVector &occlusionSets, const MDagPath &meshDagPath, MStatus &status);
	~OccludedFaceMapGenerator();

	int                   getOcclusionZoneCount() const;
	const CrcLowerString &getOcclusionZoneName(int index) const;

	int                   getOcclusionZoneCombinationCount() const;
	const IntVector      &getOcclusionZoneCombination(int index) const;

	int                   getFaceCount() const;
	int                   getFaceOcclusionZoneCombinationIndex(int faceIndex) const;

	void                  debugDump() const;

	void                  constructFullyOccludedZoneCombination(IntVector &fullyOccludedZoneCombination) const;

private:

	static void remove();

private:

	typedef stdvector<stdvector<int>::fwd>::fwd  IntVectorVector;

private:

	std::auto_ptr<UniqueNameList>   m_occlusionZoneNames;
	std::auto_ptr<IntVectorVector>  m_occlusionZoneCombinations;
	std::auto_ptr<IntVector>        m_faceOcclusionZoneCombinations;

	bool                            m_hasNonOccludableFace;

private:
	// disabled
	OccludedFaceMapGenerator();
	OccludedFaceMapGenerator(const OccludedFaceMapGenerator&);
	OccludedFaceMapGenerator &operator =(const OccludedFaceMapGenerator&);
};

// ======================================================================

#endif
