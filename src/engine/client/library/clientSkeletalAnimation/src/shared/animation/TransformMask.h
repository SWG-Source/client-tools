// ======================================================================
//
// TransformMask.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TransformMask_H
#define INCLUDED_TransformMask_H

// ======================================================================

class CrcLowerString;
class CrcString;
class Iff;
class PersistentCrcString;
class TransformMaskList;
class TransformNameMap;

// ======================================================================
/**
 * Represents a list of transform names that are divided into two groups:
 * those in the "in" group and those in the "out" group.
 *
 * This class stores the names of the transforms within the "in" group.
 * The user can query whether a transform name belongs to the "in" group.
 * Given an "in" group priority, an "out" group priority value and a
 * TransformNameMap instance, this class can fill a translation table
 * (int vector) where the index represents the transform index and the
 * value of the vector represents the priority for the given transform index.
 */

class TransformMask
{
friend class TransformMaskList;

public:

	typedef stdvector<int>::fwd  IntVector;

public:

	void                  fetch() const;
	void                  release() const;
	int                   getReferenceCount() const;

	const CrcLowerString &getName() const;

	int                   getInGroupTransformCount() const;
	CrcString const      &getInGroupTransformName(int index) const;
	bool                  isTransformWithinInGroup(CrcString const &transformName) const;

	void                  fillTransformPriorityVector(const TransformNameMap &transformNameMap, int inGroupPriority, int outGroupPriority, IntVector &priorityLookupVector) const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Tool interface for creation.
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	TransformMask();

	void                  addInGroupTransformName(const CrcLowerString &transformName);
	void                  write(Iff &iff) const;

private:

	typedef stdset<PersistentCrcString>::fwd  PersistentCrcStringSet;

private:

	explicit TransformMask(const char *name, Iff &iff);
	~TransformMask();

	void load_0000(Iff &iff);

	// Disabled.
	TransformMask(const TransformMask&);
	TransformMask &operator =(const TransformMask&);

private:

	mutable int                   m_referenceCount;
	CrcLowerString    *const      m_name;
	PersistentCrcStringSet *const m_inGroupTransformNames;

};

// ======================================================================

#endif
