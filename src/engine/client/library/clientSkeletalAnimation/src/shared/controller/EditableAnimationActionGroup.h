// ======================================================================
//
// EditableAnimationActionGroup.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_EditableAnimationActionGroup_H
#define INCLUDED_EditableAnimationActionGroup_H

// ======================================================================

#include "clientSkeletalAnimation/AnimationActionGroup.h"

class EditableAnimationAction;
class EditableAnimationStateHierarchyTemplate;
class Iff;

// ======================================================================

class EditableAnimationActionGroup: public AnimationActionGroup
{
public:

	EditableAnimationActionGroup(EditableAnimationStateHierarchyTemplate &hierarchyTemplate, Iff &iff, int versionNumber);
	explicit EditableAnimationActionGroup(EditableAnimationStateHierarchyTemplate &hierarchyTemplate);
	virtual ~EditableAnimationActionGroup();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// inherited interface
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	virtual const CrcLowerString  &getName() const;

	virtual int                    getActionCount() const;
	virtual const AnimationAction &getConstAction(int index) const;
	virtual const AnimationAction *findConstActionByName(const CrcLowerString &actionName) const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// new functionality
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	EditableAnimationStateHierarchyTemplate &getHierarchyTemplate();

	void                           setName(const CrcLowerString &name);

	EditableAnimationAction       &getAction(int index);
	void                           addAction(EditableAnimationAction *action);
	void                           removeAction(EditableAnimationAction *action);

	void                           write(Iff &iff) const;

private:

	typedef stdvector<EditableAnimationAction*>::fwd  EditableAnimationActionVector;

private:

	static std::string  getNewGroupName();

private:

	void  load_0000(Iff &iff, int versionNumber);
	void  load_0001(Iff &iff, int versionNumber);
	void  load_0002(Iff &iff, int versionNumber);

	// disabled
	EditableAnimationActionGroup(const EditableAnimationActionGroup&);
	EditableAnimationActionGroup &operator =(const EditableAnimationActionGroup&);

private:

	static const std::string  cms_newGroupNameBase;

	static int                ms_newGroupCount;

private:

	EditableAnimationStateHierarchyTemplate &m_hierarchyTemplate;

	CrcLowerString                      *m_name;
	EditableAnimationActionVector *const m_actions;

};

// ======================================================================

inline EditableAnimationStateHierarchyTemplate &EditableAnimationActionGroup::getHierarchyTemplate()
{
	return m_hierarchyTemplate;
}

// ======================================================================

#endif
