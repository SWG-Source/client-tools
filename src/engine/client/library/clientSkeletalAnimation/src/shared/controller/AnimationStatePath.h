// ======================================================================
//
// AnimationStatePath.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AnimationStatePath_H
#define INCLUDED_AnimationStatePath_H

// ======================================================================

#include "clientSkeletalAnimation/AnimationStateNameId.h"
#include <vector>

// ======================================================================

class AnimationStatePath
{
public:

	static int  getCommonPathLength(const AnimationStatePath &path1, const AnimationStatePath &path2);

public:

	AnimationStatePath();

	void                        clearPath();

	void                        appendState(const AnimationStateNameId &stateNameId);
	void                        removeLastState();
	bool                        hasState(const AnimationStateNameId &stateNameId) const;

	int                         getPathLength() const;
	const AnimationStateNameId &getState(int index) const;

	void                        copyTo(AnimationStatePath &destinationPath) const;

	bool                        operator ==(const AnimationStatePath &rhs) const;
	bool                        operator !=(const AnimationStatePath &rhs) const;
	bool                        isEqualTo(const AnimationStatePath &rhs, int rhsLength) const;

	std::string                 getPathString() const;
	std::string                 getPathString(int partialPathLength) const;

private:

	typedef stdvector<AnimationStateNameId>::fwd  AnimationStateNameIdVector;

private:

	// disabled
	AnimationStatePath(const AnimationStatePath&);
	AnimationStatePath &operator =(const AnimationStatePath&);

private:

	AnimationStateNameIdVector  m_path;

};

// ======================================================================

inline bool AnimationStatePath::operator !=(const AnimationStatePath &rhs) const
{
	return !(*this == rhs);
}

// ======================================================================

#endif
