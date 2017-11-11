// ======================================================================
//
// AnimationActionGroup.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_AnimationActionGroup_H
#define INCLUDED_AnimationActionGroup_H

// ======================================================================

class AnimationAction;
class CrcLowerString;

// ======================================================================

class AnimationActionGroup
{
public:

	struct LessNameComparator
	{
		bool operator ()(const AnimationActionGroup *lhs, const AnimationActionGroup *rhs) const;
		bool operator ()(const CrcLowerString &lhs, const AnimationActionGroup *rhs) const;
		bool operator ()(const AnimationActionGroup *lhs, const CrcLowerString &rhs) const;
	};

public:

	virtual ~AnimationActionGroup();

	virtual const CrcLowerString  &getName() const = 0;

	virtual int                    getActionCount() const = 0;
	virtual const AnimationAction &getConstAction(int index) const = 0;
	virtual const AnimationAction *findConstActionByName(const CrcLowerString &actionName) const = 0;

protected:

	AnimationActionGroup();

private:

	// disabled
	AnimationActionGroup(AnimationActionGroup&);
	AnimationActionGroup &operator =(const AnimationActionGroup&);

};

// ======================================================================

#endif
