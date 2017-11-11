// ======================================================================
//
// AnimationStateNameId.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AnimationStateNameId_H
#define INCLUDED_AnimationStateNameId_H

// ======================================================================

class AnimationStateNameId
{
friend class AnimationStateNameIdManager;

public:

	AnimationStateNameId();

	bool  isValid() const;
	int   getId() const;

	bool operator ==(const AnimationStateNameId &rhs) const;
	bool operator !=(const AnimationStateNameId &rhs) const;

private:

	enum
	{
		INVALID_ID = -1
	};

private:

	explicit AnimationStateNameId(int id);

private:

	int  m_id;

};

// ======================================================================

inline AnimationStateNameId::AnimationStateNameId() :
	m_id(INVALID_ID)
{
}

// ----------------------------------------------------------------------

inline bool AnimationStateNameId::isValid() const
{
	return (m_id != INVALID_ID);
}

// ----------------------------------------------------------------------

inline int AnimationStateNameId::getId() const
{
	return m_id;
}

// ----------------------------------------------------------------------

inline bool AnimationStateNameId::operator ==(const AnimationStateNameId &rhs) const
{
	return getId() == rhs.getId();
}

// ----------------------------------------------------------------------

inline bool AnimationStateNameId::operator !=(const AnimationStateNameId &rhs) const
{
	return !(*this == rhs);
}

// ======================================================================

inline AnimationStateNameId::AnimationStateNameId(int id) :
	m_id(id)
{
	DEBUG_FATAL(id == INVALID_ID, ("explicitly creating an AnimationStateNameId with the magic invalid id value, bad idea."));
}

// ======================================================================

#endif
